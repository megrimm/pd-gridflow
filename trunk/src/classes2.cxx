/*
	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "gridflow.hxx.fcs"
extern "C" {
#include "bundled/g_canvas.h"
#include "bundled/m_imp.h"
extern t_class *text_class;
};
#include <algorithm>
#include <errno.h>
#include <sys/time.h>
#include <string>
#include <fcntl.h>

typedef int (*comparator_t)(const void *, const void *);

struct _outconnect {
    struct _outconnect *next;
    t_pd *to;
};
struct _outlet {
    t_object *owner;
    struct _outlet *next;
    t_outconnect *connections;
    t_symbol *sym;
};

#define foreach(ITER,COLL) for(typeof(COLL.begin()) ITER = COLL.begin(); ITER != (COLL).end(); ITER++)

string join (int argc, t_atom *argv, string sep=" ", string term="") {
	ostringstream os;
	for (int i=0; i<argc; i++) os << argv[i] << (i==argc-1 ? term : sep);
	return os.str();
}

/* get the owner of the result of canvas_getenv */
static t_canvas *canvas_getabstop(t_canvas *x) {
    while (!x->gl_env) if (!(x = x->gl_owner)) bug("t_canvasenvironment %p", x);
    return x;
} 
void outlet_anything2 (PtrOutlet o, int argc, t_atom *argv) {
	if (!argc) o();
	else if (argv[0].a_type==A_SYMBOL)           o(argv[0].a_symbol,argc-1,argv+1);
	else if (argv[0].a_type==A_FLOAT && argc==1) o(argv[0].a_float);
	else o(argc,argv);
}
void pd_anything2 (t_pd *o, int argc, t_atom *argv) {
	if (!argc) pd_bang(o);
	else if (argv[0].a_type==A_SYMBOL)       pd_typedmess(o,argv[0].a_symbol,argc-1,argv+1);
	else if (argv[0].a_type==A_FLOAT && argc==1) pd_float(o,argv[0].a_float);
	else pd_typedmess(o,&s_list,argc,argv);
}

//****************************************************************

struct ArgSpec {
	t_symbol *name;
	t_symbol *type;
	t_atom2 defaultv;
};

\class Args : FObject {
	vector<ArgSpec> sargs;
	\constructor (...) {
		sargs.resize(argc);
		for (int i=0; i<argc; i++) {
			if (argv[i].a_type==A_LIST) {
				t_binbuf *b = argv[i];
				int bac = binbuf_getnatom(b);
				t_atom *bat = binbuf_getvec(b);
				sargs[i].name = atom_getsymbolarg(0,bac,bat);
				sargs[i].type = atom_getsymbolarg(1,bac,bat);
				if (bac<3) set_atom(&sargs[i].defaultv); else sargs[i].defaultv = bat[2];
			} else if (argv[i].a_type==A_SYMBOL) {
				sargs[i].name = argv[i];
				sargs[i].type = gensym("a");
				set_atom(&sargs[i].defaultv);
			} else RAISE("expected symbol or nested list");
		}
		noutlets_set(argc);
	}
	\decl 0 bang () {_0_loadbang();}
	\decl 0 loadbang ();
	void process_args (int argc, t_atom *argv);
};
\def 0 loadbang () {
	t_canvasenvironment *env = canvas_getenv(mom);
	int ac = env->ce_argc;
	t_atom2 av[ac];
	for (int i=0; i<ac; i++) av[i] = env->ce_argv[i];
	//ac = handle_parens(ac,av);
	t_symbol *comma = gensym(",");
	int j;
	for (j=0; j<ac; j++) if (av[j].a_type==A_SYMBOL && av[j]==comma) break;
	int jj = handle_parens(j,av);
	process_args(jj,av);
	while (j<ac) {
		j++;
		int k=j;
		for (; j<ac; j++) if (av[j].a_type==A_SYMBOL && av[j]==comma) break;
		t_text *t = (t_text *)canvas_getabstop(mom);
		if (!t->te_inlet) RAISE("can't send init-messages, because object has no [inlet]");
		if (j-k) pd_anything2((t_pd *)t->te_inlet,j-k,av+k);
	}
}
void Args::process_args (int argc, t_atom *argv) {
	t_canvas *canvas = canvas_getrootfor(mom);
	t_symbol *wildcard = gensym("*");
	for (int i=sargs.size()-1; i>=0; i--) {
		t_atom2 *v;
		if (i>=argc) {
			if (sargs[i].defaultv.a_type != A_NULL) {
				v = &sargs[i].defaultv;
			} else if (sargs[i].name!=wildcard) {
				pd_error(canvas,"missing argument $%d named \"%s\"", i+1,sargs[i].name->s_name);
				continue;
			}
		} else v = (t_atom2 *)&argv[i];
		if (sargs[i].name==wildcard) {
			if (argc-i>0) out[i](argc-i,argv+i); else out[i]();
		} else {
			if      (v->a_type==A_LIST)   {t_binbuf *b = *v; out[i](binbuf_getnatom(b),binbuf_getvec(b));}
			else if (v->a_type==A_SYMBOL) out[i](*v);
			else                          outlet_anything2(out[i],1,v);
		}
	}
	if (argc>int(sargs.size()) && sargs[sargs.size()-1].name!=wildcard)
		pd_error(canvas,"warning: too many args (got %d, want %d)", argc, int(sargs.size()));
}
\end class {install("args",1,1);}

#define Z(T) case T: return #T; break;
const char *atomtype_to_s (t_atomtype t) {
  switch (t) {Z(A_FLOAT)Z(A_SYMBOL)Z(A_POINTER)Z(A_DOLLAR)Z(A_DOLLSYM)Z(A_COMMA)Z(A_SEMI)Z(A_LIST)Z(A_GRID)Z(A_GRIDOUT)
  default: return "unknown type";}}
#undef Z
#include "hack.hxx"
#define binbuf_addv(B,S,A...) binbuf_addv(B,const_cast<char *>(S),A)
#define BOF t_binbuf *b = ((t_object *)canvas)->te_binbuf; if (!b) RAISE("no parent for canvas containing [setargs]");
\class GFSetArgs : FObject {
	t_canvas *canvas;
	\constructor () {canvas = canvas_getrootfor(mom);}
	void mom_changed () {
		BOF;
		t_binbuf *d = binbuf_new();
		binbuf_addv(d,"s",gensym("args"));
		binbuf_add(d,max(int(binbuf_getnatom(b))-1,0),binbuf_getvec(b)+1);
		t_canvasenvironment *pce = canvas_getenv(canvas->gl_owner);
		if (!pce) RAISE("no canvas environment for canvas containing canvas containing [setargs]");
		pd_pushsym((t_pd *)canvas);
		binbuf_eval(d,(t_pd *)bself,pce->ce_argc,pce->ce_argv);
		pd_popsym((t_pd *)canvas);
		binbuf_free(d);
		glist_retext(canvas->gl_owner,(t_object *)canvas);
	}
	\decl 0 args (...) {
		t_canvasenvironment *ce = canvas_getenv(canvas);
		free(ce->ce_argv);
		ce->ce_argv = (t_atom *)malloc(argc*sizeof(t_atom));
		ce->ce_argc = argc;
		for (int i=0; i<argc; i++) ce->ce_argv[i]=argv[i];
		if (glist_isvisible(canvas)) canvas_reflecttitle(canvas);
	}
	\decl 0 set      (...) {BOF; binbuf_clear(b); binbuf_add(b,argc,argv);                     mom_changed();}
	\decl 0 add2     (...) {BOF;                  binbuf_add(b,argc,argv);                     mom_changed();}
	\decl 0 add      (...) {BOF;                  binbuf_add(b,argc,argv); binbuf_addsemi(b);  mom_changed();}
	\decl 0 addsemi  (...) {BOF;                                           binbuf_addsemi(b);  mom_changed();}
	\decl 0 addcomma (...) {BOF;                  t_atom a; SETCOMMA(&a); binbuf_add(b,1,&a);  mom_changed();}
	\decl 0 adddollar(float f)      {BOF; t_atom a; SETDOLLAR(&a, max(int(f),0));              binbuf_add(b,1,&a); mom_changed();}
	\decl 0 adddollsym(t_symbol *s) {BOF; t_atom a; SETDOLLSYM(&a,symprintf("$%s",s->s_name)); binbuf_add(b,1,&a); mom_changed();}
	\decl 0 adddollsym2(t_symbol *s, t_symbol *t) {
		BOF;
		if (strlen(t->s_name)!=1) RAISE("substitution character must have a single letter");
		char tt = *t->s_name;
		t_atom a;
		char *d = strdup(s->s_name);
		for (char *e=d; *e; e++) if (*e==tt) *e='$';
		SETDOLLSYM(&a,gensym(d));
		free(d);
		binbuf_add(b,1,&a); mom_changed();
	}
};
\end class {install("setargs",1,1);}

\class GFAttr : FObject {
	typedef map<t_symbol *,vector<t_atom2> > t_table;
	t_table table;
	\constructor () {}
	//void outlet_entry(const typeof(table.begin()) &f) // can't use this in gcc<4.4
	void outlet_entry(const t_table::iterator &f) {out[0](f->first,f->second.size(),&*f->second.begin());}
	\decl 0 get (t_symbol *s=0) {
		if (s) {
			typeof(table.begin()) f = table.find(s);
			if (f!=table.end()) outlet_entry(f);
		} else {
			foreach(it,table) outlet_entry(it);
		}
	}
	\decl 0 remove (t_symbol *s=0) {if (s) table.erase(s); else table.clear();}
	\decl void anything (...) {
		t_symbol *sel = argv[1];
		table[sel].resize(argc-2);
		for (int i=2; i<argc; i++) table[sel][i-2]=argv[i];
	}
};
\end class {install("attr",1,1);}

//****************************************************************

namespace {
template <class T> void swap (T &a, T &b) {T c; c=a; a=b; b=c;}
};

\class ListReverse : FObject {
	\constructor () {}
	\decl 0 list(...) {
		for (int i=(argc-1)/2; i>=0; i--) swap(argv[i],argv[argc-i-1]);
		out[0](argc,argv);
	}
};
\end class {install("listreverse",1,1);}

\class ListFlatten : FObject {
	vector<t_atom2> contents;
	\constructor () {}
	void traverse (int argc, t_atom2 *argv) {
		for (int i=0; i<argc; i++) {
			if (argv[i].a_type==A_LIST) traverse(binbuf_getnatom(argv[i]),(t_atom2 *)binbuf_getvec(argv[i]));
			else contents.push_back(argv[i]);
		}
	}
	\decl 0 list(...) {
		traverse(argc,argv);
		out[0](contents.size(),&contents[0]);
		contents.clear();
	}
};
\end class {install("listflatten",1,1);}

\class ListFind : FObject {
	vector<t_atom2> r;
	\constructor (...) {_1_list(argc,argv);}
	\decl 0 list(t_atom a) {
		for (size_t i; i<r.size(); i++) if (r[i]==a) {out[0](i); return;}
		out[0](-1);
	}
	\decl 1 list(...) {r.resize(argc); for (int i=0; i<argc; i++) r[i]=argv[i];}
};
\end class {install("listfind",2,1);}

\class ListRead : FObject { /* sounds like tabread */
	vector<t_atom2> r;
	\constructor (...) {_1_list(argc,argv);}
	\decl 0 float(float f) {
		int i = int(f);
		if (i<0) i+=r.size();
		if (i<0 || i>=int(r.size())) {out[0](); return;} /* out-of-range */
		out[0](r[i]);
	}
	\decl 1 list(...) {r.resize(argc); for (int i=0; i<argc; i++) r[i]=argv[i];}
};
\end class {install("listread",2,1);}

\class Range : FObject {
	vector<float> mosusses; // use vector<float> ?
	\constructor (...) {
		mosusses.resize(argc);
		for (int i=0; i<argc; i++) if (argv[i].a_type!=A_FLOAT) RAISE("$%d: expected float",i+1);
		for (int i=0; i<argc; i++) mosusses[i]=argv[i];
		 ninlets_set(1+argc);
		noutlets_set(1+argc);
	}
	\decl 0 float (float f) {
		size_t i;
		for (i=0; i<mosusses.size(); i++) if (f<mosusses[i]) break;
		out[i](f);
	}
	\decl n float(int i, float f) {mosusses[i-1] = f;}
};
\end class {install("range",1,1); add_creator("gf/range");}

//****************************************************************

static inline const t_atom *convert (const t_atom &r, const t_atom **bogus) {return &r;}

\class GFPrint : FObject {
	string prefix;
	t_pd *gp;
	\constructor (const t_atom *s=0) {
		ostringstream text;
		if (s) text << *s; else text << "print";
		prefix = text.str();
		t_atom2 a[1]; a[0]=gensym(prefix.data());
		pd_typedmess(&pd_objectmaker,gensym("#print"),1,a);
		gp = pd_newest();
	}
	~GFPrint () {pd_free(gp);}
	\decl 0 grid(...) {pd_typedmess(gp,s_grid,argc,argv);}
	\decl void anything (...) {
		ostringstream text;
		text << prefix << ":";
		if      (argv[1]==&s_float && argc==3 && argv[2].a_type==A_FLOAT  ) {/* don't show the selector. */}
		else if (argv[1]==&s_list  && argc>=3 && argv[2].a_type==A_FLOAT  ) {/* don't show the selector. */}
		else if (argv[1]==&s_list  && argc==3 && argv[2].a_type==A_SYMBOL ) {text << " symbol" ;}
		else if (argv[1]==&s_list  && argc==3 && argv[2].a_type==A_POINTER) {text << " pointer";}
		else if (argv[1]==&s_list  && argc==2) {text << " bang";}
		else {text << " " << argv[1];}
		for (int i=2; i<argc; i++) {text << " " << argv[i];}
		post("%s",text.str().data());
	}
};
\end class {install("gf/print",1,0); add_creator2(fclass,"gf.print"); add_creator3(fclass,"print");}

//****************************************************************

\class UnixTime : FObject {
	\constructor () {}
	\decl 0 bang () {
		timeval tv;
		gettimeofday(&tv,0);
		time_t t = time(0);
		struct tm *tmp = localtime(&t);
		if (!tmp) RAISE("localtime: %s",strerror(errno));
		char tt[MAXPDSTRING];
		strftime(tt,MAXPDSTRING,"%a %b %d %H:%M:%S %Z %Y",tmp);
		t_atom2 a[6] = {tmp->tm_year+1900,tmp->tm_mon-1,tmp->tm_mday,tmp->tm_hour,tmp->tm_min,tmp->tm_sec};
		t_atom2 b[3] = {tv.tv_sec/86400,mod(tv.tv_sec,86400),tv.tv_usec};
		out[2](6,a);
		out[1](3,b);
		send_out(0,strlen(tt),tt);
	}
};
\end class UnixTime {install("unix_time",1,3);}


//****************************************************************

/* if using a DB-25 female connector as found on a PC, then the pin numbering is like:
  13 _____ 1
  25 \___/ 14
  1 = STROBE = the clock line is a square wave, often at 9600 Hz,
      which determines the data rate in usual circumstances.
  2..9 = D0..D7 = the eight ordinary data bits
  10 = -ACK (status bit 6 ?)
  11 = BUSY (status bit 7)
  12 = PAPER_END (status bit 5)
  13 = SELECT (status bit 4 ?)
  14 = -AUTOFD
  15 = -ERROR (status bit 3 ?)
  16 = -INIT
  17 = -SELECT_IN
  18..25 = GROUND
*/

//#include <linux/parport.h>
#define LPCHAR 0x0601
#define LPCAREFUL 0x0609 /* obsoleted??? wtf? */
#define LPGETSTATUS 0x060b /* return LP_S(minor) */
#define LPGETFLAGS 0x060e /* get status flags */

#ifndef __WIN32__
#include <sys/ioctl.h>
#endif

struct ParallelPort;
void ParallelPort_call(ParallelPort *self);
\class ParallelPort : FObject {
	FILE *f;
	int fd;
	int status;
	int flags;
	bool manually;
	t_clock *clock;
	~ParallelPort () {if (clock) clock_free(clock); if (f) fclose(f);}
	\constructor (string port, bool manually=0) {
	  #ifdef __WIN32__
	    RAISE("doesn't work on win32");
	  #endif
		f = fopen(port.data(),"r+");
		if (!f) RAISE("open %s: %s",port.data(),strerror(errno));
		fd = fileno(f);
		status = 0xdeadbeef;
		flags  = 0xdeadbeef;
		this->manually = manually;
		clock = manually ? 0 : clock_new(this,(void(*)())ParallelPort_call);
		clock_delay(clock,0);
	}
	void call ();
	#define FRAISE(funk,f) RAISE("can't "#funk": %s",ferror(f));
	\decl 0 float (float x) {
		uint8 foo = (uint8) x;
		if (fwrite(&foo,1,1,f)<1) FRAISE(fwrite,f);
		if (!fflush(f)<0)         FRAISE(fflush,f);
	}
	\decl 0 bang () {status = flags = 0xdeadbeef; call();}
};
void ParallelPort_call(ParallelPort *self) {self->call();}
void ParallelPort::call() {
#ifndef __WIN32__
	int flags;
	if (ioctl(fd,LPGETFLAGS,&flags)<0) post("ioctl: %s",strerror(errno));
	if (this->flags!=flags) out[2](flags);
	this->flags = flags;
	int status;
	if (ioctl(fd,LPGETSTATUS,&status)<0) post("ioctl: %s",strerror(errno));
	if (this->status!=status) out[1](status);
	this->status = status;
	if (clock) clock_delay(clock,2000);
#endif
}
//outlet 0 reserved (future use)
\end class {install("parallel_port",1,3);}

//****************************************************************

void setsels (vector<t_symbol *> &sels, int argc, t_atom2 *argv) {
	for (int i=0; i<argc; i++) if (argv[i].a_type!=A_SYMBOL) RAISE("$%d: expected symbol",i+1);
	sels.resize(argc); for (int i=0; i<argc; i++) sels[i]=argv[i];
}

\class Route2 : FObject {
	vector<t_symbol *> sels;
	\constructor (...) {_1_list(argc,argv); noutlets_set(1+argc);}
	\decl void anything(...) {
		t_symbol *sel = argv[1];
		int i=0; for (i=0; i<int(sels.size()); i++) if (sel==sels[i]) break;
		out[i](sel,argc-2,argv+2);
	}
	\decl 1 list(...) {setsels(sels,argc,argv);}
};
\end class {install("route2",1,1);}

\class Route3 : FObject {
	vector<t_symbol *> sels;
	\constructor (...) {_1_list(argc,argv); noutlets_set(1+argc);}
	\decl void anything(...) {
		t_symbol *sel = argv[1];
		int i=0; for (i=0; i<int(sels.size()); i++) if (sel==sels[i]) break;
		if (sel!=&s_bang && sel!=&s_float && sel!=&s_symbol && sel!=&s_pointer) sel=&s_list; // what about grid,blob ?
		out[i](sel,argc-2,argv+2);
	}
	\decl 1 list(...) {setsels(sels,argc,argv);}
};
\end class {install("route3",1,1);}

template <class T> int sgn(T a, T b=0) {return a<b?-1:a>b;}

\class Shunt : FObject {
	int n;
	\attr int index;
	\attr int mode;
	\attr int lo;
	\attr int hi;
	\constructor (int n=2, int i=0) {this->n=n; index=i; mode=0; lo=0; hi=n-1; noutlets_set(n);}
	\decl 1 float(int i) {index = mod(i,n);}
	\decl void anything(...) {
		out[index](argv[1],argc-2,argv+2);
		if (mode) {
			index += sgn(mode);
			if (index<lo || index>hi) {
				if (gf_abs(mode)==1) index = mod(index-lo,max(hi-lo+1,0))+lo; else {mode=-mode; index+=mode;}
			}
		}
	}
};
\end class {install("shunt",2,0);}

struct Receives;
struct ReceivesProxy {
	t_pd x_pd;
	Receives *parent;
	t_symbol *suffix;
};
t_class *ReceivesProxy_class;

\class Receives : FObject {
	vector<ReceivesProxy> rp;
	t_symbol *prefix;
	t_symbol *local (t_symbol *suffix) {return symprintf("%s%s",prefix->s_name,suffix->s_name);}
	\constructor (t_symbol *prefix=&s_, ...) {
		this->prefix = prefix==gensym("empty") ? &s_ : prefix;
		int n = min(1,argc);
		do_bind(argc-n,argv+n);
	}
	\decl 0 symbol (t_symbol *s) {t_atom2 a[1] = {s}; _0_list(1,a);}
	\decl 0 list (...) {do_unbind(); do_bind(argc,argv);}
	void do_bind (int argc, t_atom2 *argv) {
		rp.resize(argc);
		for (int i=0; i<argc; i++) {
			rp[i].x_pd = ReceivesProxy_class;
			rp[i].parent = this;
			rp[i].suffix = argv[i];
			pd_bind(  (t_pd *)&rp[i],local(rp[i].suffix));
		}
	}
	void do_unbind () {
		for (int i=0; i<int(rp.size()); i++) {
			pd_unbind((t_pd *)&rp[i],local(rp[i].suffix));
		}
	}
	~Receives () {do_unbind();}
};
void ReceivesProxy_anything (ReceivesProxy *self, t_symbol *s, int argc, t_atom *argv) {
	self->parent->out[1](self->suffix);
	self->parent->out[0](s,argc,argv);
}
\end class {
	install("receives",1,2);
	ReceivesProxy_class = class_new(gensym("receives.proxy"),0,0,sizeof(ReceivesProxy),CLASS_PD|CLASS_NOINLET, A_NULL);
	class_addanything(ReceivesProxy_class,(t_method)ReceivesProxy_anything);
}

/* this can't report on bang,float,symbol,pointer,blob,list because zgetfn can't either */
\class ClassExists : FObject {
	\constructor () {}
	\decl 0 symbol(t_symbol *s) {out[0](!!zgetfn(&pd_objectmaker,s));}
};
\end class {install("class_exists",1,1);}

\class ListEqual : FObject {
	vector<t_atom2> list;
	\constructor (...) { _1_list(argc,argv);}
	\decl 1 list (...) {list.resize(argc); for (int i=0; i<argc; i++) list[i]=argv[i];}
	\decl 0 list (...)  {
		if (int(list.size()) != argc) {out[0](0); return;}
		for (int i=0; i<argc; i++) if (list[i]!=argv[i]) {out[0](0); return;}
		out[0](1);
	}
};
\end class {install("list.==",2,1);}

//****************************************************************
//#ifdef UNISTD
#include <sys/types.h>
#include <sys/time.h>
#ifdef __WIN32__
	struct tms {long tms_utime,tms_stime;}; static void times(struct tms *) {} // dummy
	#define HZ 31337
	#define NOWIN RAISE("win32 unsupported");
#else
	#include <sys/times.h>
	#define NOWIN
#endif
#include <sys/param.h>
#include <unistd.h>
//#endif
#if defined (__APPLE__) || defined (__FreeBSD__)
#define HZ CLK_TCK
#endif

uint64 cpu_hertz;
int uint64_compare(uint64 &a, uint64 &b) {return a<b?-1:a>b;}

\class UserTime : FObject {
	clock_t time;
	\constructor () {_0_bang();}
	\decl 0 bang () {NOWIN; struct tms t; times(&t); time = t.tms_utime;}
	\decl 1 bang () {NOWIN; struct tms t; times(&t); out[0]((t.tms_utime-time)*1000/HZ);}
};
\end class {install("usertime",2,1);}
\class SystemTime : FObject {
	clock_t time;
	\constructor () {_0_bang();}
	\decl 0 bang () {NOWIN; struct tms t; times(&t); time = t.tms_stime;}
	\decl 1 bang () {NOWIN; struct tms t; times(&t); out[0]((t.tms_stime-time)*1000/HZ);}
};
\end class {install("systemtime",2,1);}
\class TSCTime : FObject {
	uint64 time;
	\constructor () {_0_bang();}
	\decl 0 bang () {time=rdtsc();}
	\decl 1 bang () {out[0]((rdtsc()-time)*1000.0/cpu_hertz);}
};
\end class {install("tsctime",2,1);
	struct timeval t0,t1;
	uint64 u0,u1;
	uint64 estimates[3];
	for (int i=0; i<3; i++) {
		u0=rdtsc(); gettimeofday(&t0,0); usleep(10000);
		u1=rdtsc(); gettimeofday(&t1,0);
		uint64 t = (t1.tv_sec-t0.tv_sec)*1000000+(t1.tv_usec-t0.tv_usec);
		estimates[i] = (u1-u0)*1000000/t;
	}
	qsort(estimates,3,sizeof(uint64),(comparator_t)uint64_compare);
	cpu_hertz = estimates[1];
}

\class GFError : FObject {
	string format;
	\constructor (...) {format = join(argc,argv);}
	\decl 0 list (...) {
		t_binbuf *b = mom->gl_obj.te_binbuf;
		t_canvasenvironment *ce = canvas_getenv(canvas_getabstop(mom));
		ostringstream o;
		o << "[";
		if (b && binbuf_getnatom(b)) o<<*binbuf_getvec(b); else o<<"???";
		if (ce) for (int i=0; i<ce->ce_argc; i++) o << " " << ce->ce_argv[i];
		o << "]: ";
		pd_oprintf(o,format.data(),argc,argv);
		t_canvas *canvas = canvas_getrootfor(mom);
		string s = o.str();
		pd_error(canvas,"%s",s.data());
	}
};
\end class {install("gf/error",1,0);}

\class GFSprintf : FObject {
	string format;
	\constructor (...) {format = join(argc,argv);}
	\decl 1 list (...) {format = join(argc,argv);}
	\decl 0 list (...) {
		ostringstream o; pd_oprintf(o,format.data(),argc,argv); string s = o.str();
		out[0](gensym(s.data()));
	}
};
\end class {install("gf/sprintf",2,1);}
\class GridSprintf : FObject {
	string format;
	\attr NumberTypeE cast;
	\constructor (...) {format = join(argc,argv); cast = int32_e;}
	\decl 1 list (...) {format = join(argc,argv);}
	\decl 0 list (...) {
		ostringstream o; pd_oprintf(o,format.data(),argc,argv); string s = o.str();
		GridOut out(this,0,Dim(s.size()),cast); out.send(s.size(),(uint8 *)s.data());
	}
};
\end class {install("#sprintf",2,1);}

\class ForEach : FObject {
	\constructor () {}
	\decl 0 list (...) {for (int i=0; i<argc; i++) out[0](argv[i]);}
};
\end class {install("foreach",1,1);}

//****************************************************************

#define MOM t_canvas *m = mom; for (int i=0; i<n; i++) {m=m->gl_owner; if (!m) RAISE("no such canvas");}

\class GFCanvasFileName : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 bang () {MOM; out[0](m->gl_name ? m->gl_name : gensym("empty"));}
};
\end class {install("gf/canvas_filename",1,1);}
\class GFCanvasDollarZero : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 bang () {MOM; out[0](canvas_getenv(m)->ce_dollarzero);}
};
\end class {install("gf/canvas_dollarzero",1,1);}
\class GFCanvasGetPos : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 bang () {MOM; t_atom2 a[2] = {m->gl_obj.te_xpix, m->gl_obj.te_ypix}; out[0](2,a);}
};
\end class {install("gf/canvas_getpos",1,1);}
\class GFCanvasSetPos : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 list (...) {MOM;
		if (argc!=2) RAISE("wrong number of args");
		((t_text *)m)->te_xpix = argv[0];
		((t_text *)m)->te_ypix = argv[1];
		t_canvas *granny = m->gl_owner;
		if (!granny) RAISE("chosen canvas is not in any canvas");
		gobj_vis((t_gobj *)m,granny,0);
		gobj_vis((t_gobj *)m,granny,1);
		canvas_fixlinesfor(glist_getcanvas(granny), (t_text *)m);
	}
};
\end class {install("gf/canvas_setpos",1,0);}
\class GFCanvasEditMode : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 bang () {MOM; out[0](int(m->gl_edit));}
};
\end class {install("gf/canvas_edit_mode",1,1);}
\class GFCanvasIsSelected : FObject {
	/* contributed by "rumence" of Slovakia, on IRC */
	/* bugfix by matju */
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 bang () {MOM;
		if (!m->gl_owner) RAISE("chosen canvas is not in any canvas");
		out[0](glist_isselected(m->gl_owner,(t_gobj *)m));
	}
};
\end class {install("gf/canvas_isselected",1,1);}
extern "C" void canvas_setgraph(t_glist *x, int flag, int nogoprect);
\class GFCanvasSetGOP : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 float (float gop) {MOM; canvas_setgraph(m,int(gop),0);}
};
\end class {install("gf/canvas_setgop",1,0);}
\class GFCanvasXID : FObject {
	int n;
	t_symbol *name;
	\constructor (int n_=0) {
		n=n_;
		name=symprintf("gf/canvas_xid:%lx",bself);
		pd_bind((t_pd *)bself,name);
	}
	~GFCanvasXID () {pd_unbind((t_pd *)bself,name);}
	\decl 0 bang () {MOM; sys_vgui("pd %s xid [winfo id .x%lx.c] [winfo id .x%lx]\\;\n",name->s_name,long(m),long(m));}
	\decl 0 xid (t_symbol *t, t_symbol *u) {MOM
		out[2](symprintf(".x%lx",m));
		out[1](u);
		out[0](t);
	}
};
\end class {install("gf/canvas_xid",1,3);}

\class GFCanvasHeHeHe : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 float (float y) {MOM;
		int osx = int(m->gl_screenx2-m->gl_screenx1); // old size x
		int osy = int(m->gl_screeny2-m->gl_screeny1); // old size y
		m->gl_screenx2 = m->gl_screenx1 + 632;
		if (m->gl_screeny2-m->gl_screeny1 < int(y)) m->gl_screeny2 = m->gl_screeny1+int(y);
		int  sx = int(m->gl_screenx2-m->gl_screenx1); // new size x
		int  sy = int(m->gl_screeny2-m->gl_screeny1); // new size y
		if (osx!=sx || osy!=sy) sys_vgui("wm geometry .x%lx %dx%d\n",long(m),sx,sy);
	}
};
\end class {install("gf/canvas_hehehe",1,1);}

#define DASHRECT "-outline #80d4b2 -dash {2 6 2 6}"

\class GFCanvasHoHoHo : FObject {
	int n;
	t_canvas *last;
	\constructor (int n) {this->n=n; last=0;}
	~GFCanvasHoHoHo () {_0_hide();}
	\decl 0 hide () {if (last) sys_vgui(".x%lx.c delete %lxRECT\n",long(last),long(bself));}
	\decl 0 list (int x1, int y1, int x2, int y2) {MOM;
		_0_hide();
		last = m;
		sys_vgui(".x%lx.c create rectangle %d %d %d %d "DASHRECT" -tags %lxRECT\n",long(m),x1,y1,x2,y2,long(bself));
	}
};
\end class {install("gf/canvas_hohoho",1,0);}

#define canvas_each(y,x) for (t_gobj *y=x->gl_list; y; y=y->g_next)
\class GFCanvasCount : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 bang () {MOM; int k=0; canvas_each(y,m) k++; out[0](k);}
};
\end class {install("gf/canvas_count",1,1);}
\class GFCanvasIndex : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 bang () {
		MOM;
		t_canvas *mm=m->gl_owner;
		if (!mm) RAISE("chosen canvas is not in any canvas");
		int k=0;
		canvas_each(y,mm) {if (y==(t_gobj *)m) break; else k++;}
		out[0](k);
	}
};
\end class {install("gf/canvas_index",1,1);}

\class GFCanvasLoadbang : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 float (float z) {MOM;
		int k=0;
		canvas_each(y,m) {
			if (k>=z && pd_class((t_pd *)y)==canvas_class) canvas_loadbang((t_canvas *)y);
			k++;
		}
	}
};
\end class {install("gf/canvas_loadbang",1,0);};

#define BEGIN \
	t_outlet *ouch = ((t_object *)mom)->te_outlet; \
	t_canvas *can = mom->gl_owner; \
	if (!can) RAISE("no such canvas"); \
	for (int i=0; i<n; i++) {ouch = ouch->next; if (!ouch) {RAISE("no such outlet");}}

#define wire_each(wire,ouchlet) for (t_outconnect *wire = ouchlet->connections; wire; wire=wire->next)

\class GFLOL : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 wire_dotted  (int r, int g, int b) {BEGIN
		wire_each(wire,ouch) {
			sys_vgui(".x%lx.c itemconfigure l%lx -fill #%02x%02x%02x -dash {3 3 3 3}\n",long(can),long(wire),r,g,b);
		}
	}
	\decl 0 wire_bracket (int r, int g, int b) {BEGIN
		t_outconnect *wire; t_linetraverser lt; linetraverser_start(&lt,can);
		// lt.tr_ob = (t_object *)mom; lt.tr_nextoc = wire; // would like this shortcut but can't use it
		while ((wire = linetraverser_next(&lt))) if (lt.tr_outlet==ouch) {
			int x1=lt.tr_lx1, y1=lt.tr_ly1, x2=lt.tr_lx2, y2=lt.tr_ly2;
			sys_vgui(".x%lx.c itemconfigure l%lx -fill #%02x%02x%02x -dash {}\n; "
				".x%lx.c coords l%lx %d %d %d %d %d %d %d %d %d %d\n",
				long(can),long(wire),r,g,b,
				long(can),long(wire), x1,y1, x1,y1+3, x1+7,y1+3, x1+7,y2+8, x2-2,y2+8);
		}
	}
	\decl 0 wire_hide () {BEGIN
		wire_each(wire,ouch) sys_vgui(".x%lx.c delete l%lx\n",long(can),long(wire));
	}
	\decl 0  box_dotted (int r, int g, int b) {BEGIN
		wire_each(wire,ouch) {
			t_object *t = (t_object *)wire->to;
			int x1,y1,x2,y2;
			gobj_getrect((t_gobj *)wire->to,can,&x1,&y1,&x2,&y2);
			// was #00aa66 {3 5 3 5}
			sys_vgui(".x%lx.c delete %lxRECT; .x%lx.c create rectangle %d %d %d %d "DASHRECT" -tags %lxRECT\n",
				long(can),long(t),long(can),x1,y1,x2,y2,long(t));
			}
	}
	\decl 0  box_align (t_symbol *s, int x_start, int y_start, int incr);
};
bool comment_sort_y_lt(t_object * const &a, t_object * const &b) /* is a StrictWeakOrdering */ {
	return a->te_ypix < b->te_ypix;
}
static t_class *inlet_class, *floatinlet_class, *symbolinlet_class, *pointerinlet_class;
static bool ISINLET(t_pd *o) {
  t_class *c=pd_class(o);
  return c==inlet_class || c==floatinlet_class || c==symbolinlet_class || c==pointerinlet_class;
}
struct _inlet {
    t_pd pd;
    struct _inlet *next;
    t_object *owner;
    t_pd *dest;
    t_symbol *symfrom;
    //union inletunion un;
};
\def 0 box_align (t_symbol *dir, int x_start, int y_start, int incr) {
	int x=x_start, y=y_start;
	bool horiz;
	if (dir==&s_x) horiz=false; else
	if (dir==&s_y) horiz=true;  else RAISE("$1 must be x or y");
	vector<t_object *> v;
	BEGIN
	wire_each(wire,ouch) {
		//post("wire to object of class %s ISINLET=%d",pd_class(wire->to)->c_name->s_name,ISINLET(wire->to));
		t_object *to = ISINLET(wire->to) ? ((t_inlet *)wire->to)->owner : (t_object *)wire->to;
		v.push_back(to);
	}
	sort(v.begin(),v.end(),comment_sort_y_lt);
	foreach(tt,v) {
		t_object *t = *tt;
		if (t->te_xpix!=x || t->te_ypix!=y) {
			gobj_vis((t_gobj *)t,can,0);
			t->te_xpix=x;
			t->te_ypix=y;
			gobj_vis((t_gobj *)t,can,1);
			canvas_fixlinesfor(can,t);
		}
		int x1,y1,x2,y2;
		gobj_getrect((t_gobj *)t,can,&x1,&y1,&x2,&y2);
		if (horiz) x += x2-x1+incr;
		else       y += y2-y1+incr;
	}
	if (horiz) out[0](x-x_start);
	else       out[0](y-y_start);
}

extern t_widgetbehavior text_widgetbehavior;
t_widgetbehavior text_widgetbehavi0r;

/* i was gonna use gobj_shouldvis but it's only for >= 0.42 */

static int text_chou_de_vis(t_text *x, t_glist *glist) {
    return (glist->gl_havewindow ||
        (x->te_pd != canvas_class && x->te_pd->c_wb != &text_widgetbehavior) ||
        (x->te_pd == canvas_class && (((t_glist *)x)->gl_isgraph)) ||
        (glist->gl_goprect && (x->te_type == T_TEXT)));
}

static void text_visfn_hax0r (t_gobj *o, t_canvas *can, int vis) {
	text_widgetbehavior.w_visfn(o,can,vis);
	//if (vis) return; // if you want to see #X text inlets uncomment this line
	t_rtext *y = glist_findrtext(can,(t_text *)o);
	if (text_chou_de_vis((t_text *)o,can)) glist_eraseiofor(can,(t_object *)o,rtext_gettag(y));
}

size_t properties_offset;

\end class {
	install("gf/lol",1,1);
	class_setpropertiesfn(text_class,(t_propertiesfn)0xDECAFFED);
	unsigned long *lol = (unsigned long *)text_class;
	int i=0;
	while (lol[i]!=0xDECAFFED) i++;
	*((char *)(lol+i+1) + 6) = 1;
	properties_offset = i;
	class_setpropertiesfn(text_class,0);
	t_object *bogus = (t_object *)pd_new(text_class);
	       inlet_class = pd_class((t_pd *)       inlet_new(bogus,0,0,0));
	  floatinlet_class = pd_class((t_pd *)  floatinlet_new(bogus,0));
	 symbolinlet_class = pd_class((t_pd *) symbolinlet_new(bogus,0));
	pointerinlet_class = pd_class((t_pd *)pointerinlet_new(bogus,0));
	memcpy(&text_widgetbehavi0r,&text_widgetbehavior,sizeof(t_widgetbehavior));
	text_widgetbehavi0r.w_visfn = text_visfn_hax0r;
	class_setwidget(text_class,&text_widgetbehavi0r);
}

bool canvas_contains (t_canvas *x, t_gobj *y) {for (t_gobj *g=x->gl_list; g; g=g->g_next) if (g==y) return true; return false;}
t_widgetbehavior *class_getwidget (t_class *x) {return (t_widgetbehavior *)((long *)x)[properties_offset-3];}
\class GFObjectBBox : FObject {
	\constructor () {}
	\decl 0 symbol (t_symbol *s) {
		if (!s->s_thing) RAISE("no such object");
		t_class *c = pd_class(s->s_thing);
		if (c->c_name==gensym("bindlist")) RAISE("multiple such objects");
		t_widgetbehavior *wb = class_getwidget(c);
		if (!wb) RAISE("not a patchable object");
		if (!canvas_contains(mom,(t_gobj *)s->s_thing)) RAISE("object is not in this canvas");
		int x1,y1,x2,y2;
		wb->w_getrectfn((t_gobj *)s->s_thing,mom,&x1,&y1,&x2,&y2);
		t_atom2 a[4] = {x1,y1,x2,y2}; out[0](4,a);
	}
};

\end class {install("gf/object_bbox",1,1);}

// pas vite vite
string string_replace (string victim, string from, string to) {
	for (size_t i=0;;) {
	  i = victim.find(from,i);
	  if (i==string::npos) break;
	  victim = victim.replace(i,from.length(),to);
	  i += to.length();
	}
	return victim;
}

\class GFStringReplace : FObject {
	t_symbol *from;
	t_symbol *to;
	\constructor (t_symbol *from=&s_, t_symbol *to=&s_) {this->from=from; this->to=to;}
	\decl 0 symbol (t_symbol *victim) {
		string a = string_replace(victim->s_name,from->s_name,to->s_name);
		out[0](gensym(a.c_str()));
	}
};
\end class {install("gf/string_replace",1,1);}

\class GFStringLessThan : FObject {
	t_symbol *than;
	\constructor (t_symbol *than=&s_) {this->than=than;}
	\decl 0 symbol (t_symbol *it) {out[0](strcmp(it->s_name,than->s_name)<0);}
	\decl 1 symbol (t_symbol *than) {this->than=than;}
};
\end class {install("gf/string_<",2,1); class_sethelpsymbol(fclass->bfclass,gensym("gf/string_0x3c"));}

\class GFStringLength : FObject {
	\constructor () {}
	\decl 0 symbol (t_symbol *it) {out[0](strlen(it->s_name));}
};
\end class {install("gf/string_length",1,1);}

\class GFGetPid : FObject {
	static t_symbol *sym;
	\constructor () {}
	\decl 0 bang () {
	    #ifdef __WIN32__
		RAISE("getppid unsupported on win32");
	    #else
		out[1](getppid());
	    #endif
	    out[0](getpid());
	}
};
\end class {install("gf/getpid",1,2);}
\class GFGetCwd : FObject {
	static t_symbol *sym;
	\constructor () {}
	\decl 0 bang () {
	    char bof[PATH_MAX];
	    if (!getcwd(bof,sizeof(bof))) RAISE("getcwd: %s",strerror(errno));
	    out[0](gensym(bof));
	}
};
\end class {install("gf/getcwd",1,1);}

\class GFSelector : FObject {
	\constructor () {}
	\decl void anything (...) {out[0](argv[1]);}
};
\end class {install("gf/selector",1,1);}

\class FindFile : FObject {
	int n;
	\constructor (int n=0) {this->n=n;}
	\decl 0 symbol (t_symbol *s) {MOM
	    int fd;
	    char bof[MAXPDSTRING], *bofp;
	    fd=open_via_path(canvas_getdir(m)->s_name,s->s_name,"",bof,&bofp,MAXPDSTRING,1);
	    if (fd>=0) {close(fd); out[0](symprintf("%s/%s",bof,bofp)); return;}
	    canvas_makefilename(m,s->s_name,bof,MAXPDSTRING);
	    fd = open(bof,0,O_RDONLY);
	    //post("(2) fd=%d for %s",fd,bof);
	    if (fd>=0) {close(fd); out[0](gensym(bof)); return;}

	    string b = gf_find_file(string(s->s_name));
	    fd = open(b.data(),0,O_RDONLY);
	    //post("(3) fd=%d for %s",fd,b.data());
	    if (fd>=0) {close(fd); out[0](gensym(b.data())); return;}

	    fd = open(s->s_name,0,O_RDONLY);
	    //post("(4) fd=%d for %s",fd,s->s_name);
	    if (fd>=0) {close(fd); out[0](s); return;}

	    out[1]();
	}
};
\end class {install("gf/find_file",1,2);}

\class GFL2S : FObject {
	t_symbol *sep;
	\constructor (t_symbol *sep=0) {this->sep=sep?sep:gensym(" ");}
	\decl 0 list (...) {out[0](gensym(join(argc,argv,sep->s_name).data()));}
	\decl 1 symbol (t_symbol *sep) {this->sep=sep;}
};
\end class {install("gf/l2s",2,1);}

\class GFS2L : FObject {
	t_symbol *sep;
	\constructor (t_symbol *sep=0) {this->sep=sep?sep:gensym(" ");}
	\decl 0 symbol (t_symbol *s) {
		const char *p = s->s_name;
		const char *q=p;
		char sepc = *sep->s_name;
		t_atom2 a[64]; int n=0;
		for (;;q++) {
			if (!*q)      {if (q!=s->s_name) a[n++]=symprintf("%.*s",q-p,p); break;}
			if (*q==sepc) {                  a[n++]=symprintf("%.*s",q-p,p); q++; p=q;}
			if (n==64) break;
		}
		out[0](n,a);
	}
	\decl 1 symbol (t_symbol *sep) {this->sep=sep;}
};
\end class {install("gf/s2l",2,1);}

\class GFSysGui : FObject {
	\constructor () {}
	\decl 0 list (...) {sys_gui(join(argc,argv," ","\n").data());}
};
\end class {install("gf/sys_gui",1,0);}

\class GFWrap : FObject {
	float b;
	\constructor (float f=1) {b=f;}
	\decl 1 float (float f)  {b=f;}
	\decl 0 float (float f) {out[0](f-floor(f/b)*b);}
};
\end class {install("gf/wrap",1,1);}

/* hack because hexloader is a myth */
\class InvPlus : FObject {
	float b;
	\constructor (float b=1) {this->b=b;}
	\decl 0 float (float a) {out[0](b-a);}
	\decl 1 float (float b) {this->b=b;}};
\end class {install("inv+",2,1); class_sethelpsymbol(fclass->bfclass,gensym("inv0x2b"));}
\class InvTimes : FObject {
	float b;
	\constructor (float b=1) {this->b=b;}
	\decl 0 float (float a) {out[0](b/a);}
	\decl 1 float (float b) {this->b=b;}};
\end class {install("inv*",2,1); class_sethelpsymbol(fclass->bfclass,gensym("inv0x2a"));}

extern t_symbol *gridflow_folder; 
\class GridFlowClass : FObject {
	\constructor () {}
	\decl 0 get (t_symbol *s=0) {
		if (!s) {_0_get(gensym("version"));_0_get(gensym("folder"));_0_get(gensym("os"));}
		if (s==gensym("version")) {t_atom2 a[3] = {GF_VERSION_A,GF_VERSION_B,GF_VERSION_C}; out[0](s,3,a);}
		if (s==gensym("folder"))  {t_atom2 a[1] = {gridflow_folder}; out[0](s,1,a);}
		#ifdef __WIN32__
		if (s==gensym("os")) {t_atom2 a[1] = {"win32"};  out[0](s,1,a);}
		#else
		#ifdef MACOSX
		if (s==gensym("os")) {t_atom2 a[1] = {"darwin"}; out[0](s,1,a);}
		#else
		if (s==gensym("os")) {t_atom2 a[1] = {"linux"}; out[0](s,1,a);} // or other
		#endif
		#endif
	}
};
\end class {install("gridflow",1,1);}

map<t_canvas *, map<t_gobj *, int> > propertybang_map;
\class PropertyBang : FObject {
	\constructor () {propertybang_map[canvas_getabstop(mom)][(t_gobj *)bself]=1;}
	~PropertyBang () {propertybang_map[canvas_getabstop(mom)].erase((t_gobj *)bself);}
	void properties () {out[0]();}
};
extern "C" void canvas_properties(t_gobj *z, t_glist *owner);
void canvas_properties2(t_gobj *z, t_glist *owner) {
	typeof(propertybang_map.end()) it = propertybang_map.find((t_canvas *)z);
	if (it == propertybang_map.end()) canvas_properties(z,owner); // fallback
	else {
		foreach(it2,it->second) {
			BFObject *bf = (BFObject *)it2->first;
			((PropertyBang *)bf->self)->properties();
		}
	}
}
\end class {
	install("gf/propertybang",1,1);
	class_setpropertiesfn(canvas_class,canvas_properties2);
}

\class GFClassInfo : FObject {
	\constructor () {}
	\decl 0 symbol (t_symbol *s) {
		if (fclasses.find(s)==fclasses.end()) {out[0](); return;}
		FClass *c = fclasses[s];
		{t_atom2 a[] = {c->name       }; out[0](gensym("name"   ),1,a);}
//		{t_atom2 a[] = {c->super->name}; out[0](gensym("super"  ),1,a);}
		{t_atom2 a[] = {c->flags      }; out[0](gensym("flags"  ),1,a);}
		{t_atom2 a[] = {c->ninlets    }; out[0](gensym("inlets" ),1,a);}
		{t_atom2 a[] = {c->noutlets   }; out[0](gensym("outlets"),1,a);}
		foreach(m,c->methods) {t_atom2 a[] = {m->first.first,m->first.second}; out[0](gensym("method"),2,a);}
		foreach(m,c->attrs)   {t_atom2 a[] = {m->first                      }; out[0](gensym("attr"  ),1,a);}
		//t_class *qlass = c->bfclass;	
	}
};
\end class {install("gf/class_info",1,1);}

void startup_classes2 () {
	\startall
}
