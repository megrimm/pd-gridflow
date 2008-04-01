/*
	$Id$

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

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

#include <stdlib.h>
//#include <cstdlib>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "../config.h"
#include <limits.h>

/* for exception-handling in 0.9.0 */
#include <exception>
#include <execinfo.h>
//using namespace std;

#include "grid.h.fcs"

BuiltinSymbols bsym;
Ruby mGridFlow;
Ruby cFObject;

extern "C"{
void rb_raise0(
const char *file, int line, const char *func, VALUE exc, const char *fmt, ...) {
	va_list args;
	char buf[BUFSIZ];
	va_start(args,fmt);
	vsnprintf(buf, BUFSIZ, fmt, args);
	buf[BUFSIZ-1]=0;
	va_end(args);
	VALUE e = rb_exc_new2(exc, buf);
	char buf2[BUFSIZ];
	snprintf(buf2, BUFSIZ, "%s:%d:in `%s'", file, line, func);
	buf2[BUFSIZ-1]=0;
	VALUE ary = rb_funcall(e,SI(caller),0);
	rb_funcall(ary,SI(unshift),1,rb_str_new2(buf2));
	rb_funcall(e,SI(set_backtrace),1,ary);
	rb_exc_raise(e);
}};

Barf::Barf(const char *s, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap,s);
    vsnprintf(buf,1024,s,ap);
    buf[1023]=0;
    va_end(ap);
    text = strdup(buf);
}
Barf::Barf(const char *file, int line, const char *func, const char *s, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap,s);
    int n = vsnprintf(buf,1024,s,ap);
    if (n<1024) snprintf(buf+n, 1024-n, "\n%s:%d:in `%s'", file, line, func);
    buf[1023]=0;
    va_end(ap);
    text = strdup(buf);
}

Ruby rb_ary_fetch(Ruby rself, long i) {
	Ruby argv[] = { LONG2NUM(i) };
	return rb_ary_aref(COUNT(argv),argv,rself);
}

void pd_oprintf (std::ostream &o, const char *s, int argc, t_atom *argv) {
	int i=0;
	for (; *s; s++) {
		if (*s!='%') {o << (char)*s; continue;}
		s++; // skip the %
		if (*s=='f') {
			if (!argc) RAISE("not enough args");
			if (argv[i].a_type != A_FLOAT) RAISE("expected float");
			o << argv[i].a_w.w_float;
		}
		if (*s=='s') {
			if (!argc) RAISE("not enough args");
			if (argv[i].a_type != A_SYMBOL) RAISE("expected symbol");
			o << argv[i].a_w.w_symbol->s_name;
		}
		if (*s=='_') {
			if (!argc) RAISE("not enough args");
			char buf[MAXPDSTRING];
			atom_string(&argv[i],buf,MAXPDSTRING);
			o << buf;
		}
		if (*s=='%') {
			o << "%";
			continue;
		}
		RAISE("sorry, this format string is not supported yet");
	}
}

//----------------------------------------------------------------
// CObject

static void CObject_mark (void *z) {}
void CObject_free (void *foo) {
	CObject *self = (CObject *)foo;
	if (!self->rself) {
		fprintf(stderr,"attempt to free object that has no rself\n");
		abort();
	}
	self->rself = 0; /* paranoia */
	delete self;
}

//----------------------------------------------------------------
// Dim

void Dim::check() {
	if (n>MAX_DIM) RAISE("too many dimensions");
	for (int i=0; i<n; i++) if (v[i]<0) RAISE("Dim: negative dimension");
}

// !@#$ big leak machine?
// returns a string like "Dim[240,320,3]"
char *Dim::to_s() {
	// if you blow 256 chars it's your own fault
	char buf[256];
	char *s = buf;
	s += sprintf(s,"Dim[");
	for(int i=0; i<n; i++) s += sprintf(s,"%s%d", ","+!i, v[i]);
	s += sprintf(s,"]");
	return strdup(buf);
}

//----------------------------------------------------------------
\class FObject : CObject

static void FObject_prepare_message(int &argc, Ruby *&argv, Ruby &sym, FObject *foo=0) {
	if (argc<1) {
		sym = SYM(bang);
	} else if (argc>1 && !SYMBOL_P(*argv)) {
		sym = SYM(list);
	} else if (INTEGER_P(*argv)||FLOAT_P(*argv)) {
		sym = SYM(float);
	} else if (SYMBOL_P(*argv)) {
		sym = *argv;
		argc--, argv++;
	} else if (argc==1 && TYPE(*argv)==T_ARRAY) {
		RAISE("oh really? (in FObject_prepare_message)");
		sym = SYM(list);
		argc = rb_ary_len(*argv);
		argv = rb_ary_ptr(*argv);
	} else {
		RAISE("%s received bad message: argc=%d; argv[0]=%s",foo?ARGS(foo):"", argc,
			argc ? rb_str_ptr(rb_inspect(argv[0])) : "");
	}
}

struct Helper {
	int argc;
	Ruby *argv;
	FObject *self;
	Ruby rself;
};

static Ruby GridFlow_handle_braces(Ruby rself, Ruby argv);

// inlet #-2 is for inlet #0 messages that happen at start time
static void send_in_2 (Helper *h) {
	int argc = h->argc;
	Ruby *argv = h->argv;
	if (h->argc<1) RAISE("not enough args");
	int inlet = INT(argv[0]);
	argc--, argv++;
	Ruby foo;
	if (argc==1 && TYPE(argv[0])==T_STRING /* && argv[0] =~ / / */) {
		foo = rb_funcall(mGridFlow,SI(parse),1,argv[0]);
		argc = rb_ary_len(foo);
		argv = rb_ary_ptr(foo);
	}
	if (argc>1) {
		foo = rb_ary_new4(argc,argv);
		GridFlow_handle_braces(0,foo);
		argc = rb_ary_len(foo);
		argv = rb_ary_ptr(foo);
	}
	if (inlet==-2) {
		Ruby init_messages = rb_ivar_get(h->rself,SI(@init_messages));
		rb_ary_push(init_messages, rb_ary_new4(argc,argv));
		inlet=0;
	}
	if (inlet<0 || inlet>=64)
		if (inlet!=-3 && inlet!=-1) RAISE("invalid inlet number: %d", inlet);
	Ruby sym;
	FObject_prepare_message(argc,argv,sym,h->self);
	char buf[256];
	sprintf(buf,"_n_%s",rb_sym_name(sym));
	if (rb_obj_respond_to(h->rself,rb_intern(buf),0)) {
		argc++, argv--; // really
		argv[0] = inlet*2+1; // convert to Ruby Integer
	} else {
		sprintf(buf,"_%d_%s",inlet,rb_sym_name(sym));
	}
	rb_funcall2(h->rself,rb_intern(buf),argc,argv);
}

static void send_in_3 (Helper *h) {}
\def void send_in (...) {
	Helper h = {argc,argv,this,rself};
	rb_ensure(
		(RMethod)send_in_2,(Ruby)&h,
		(RMethod)send_in_3,(Ruby)&h);
}

\def void send_out (...) {
	int n=0;
	if (argc<1) RAISE("not enough args");
	int outlet = INT(*argv);
	if (outlet<0 || outlet>=64) RAISE("invalid outlet number: %d",outlet);
	argc--, argv++;
	Ruby sym;
	FObject_prepare_message(argc,argv,sym,this);
	Ruby noutlets2 = rb_ivar_get(rb_obj_class(rself),SYM2ID(SYM(@noutlets)));
	if (TYPE(noutlets2)!=T_FIXNUM) {
		IEVAL(rself,"STDERR.puts inspect");
		RAISE("don't know how many outlets this has");
	}
	//int noutlets = INT(noutlets2);
	//if (outlet<0 || outlet>=noutlets) RAISE("outlet %d does not exist",outlet);
	Ruby argv2[argc+2];
	for (int i=0; i<argc; i++) argv2[2+i] = argv[i];
	argv2[0] = INT2NUM(outlet);
	argv2[1] = sym;
	rb_funcall2(rself,SI(send_out2), argc+2, argv2);

	ID ol = SI(@outlets);
	Ruby ary = rb_ivar_defined(rself,ol) ? rb_ivar_get(rself,ol) : Qnil;
	if (ary==Qnil) goto end;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	ary = rb_ary_fetch(ary,outlet);
	if (ary==Qnil) goto end;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	n = rb_ary_len(ary);

	for (int i=0; i<n; i++) {
		Ruby conn = rb_ary_fetch(ary,i);
		Ruby rec = rb_ary_fetch(conn,0);
		int inl = INT(rb_ary_fetch(conn,1));
		argv2[0] = INT2NUM(inl);
		rb_funcall2(rec,SI(send_in),argc+2,argv2);
	}
end:;
}

Ruby FObject_s_new(Ruby argc, Ruby *argv, Ruby qlass) {
	Ruby allocator = rb_ivar_defined(qlass,SI(@allocator)) ?
		rb_ivar_get(qlass,SI(@allocator)) : Qnil;
	FObject *self;
	if (allocator==Qnil) {
		// this is a pure-ruby FObject/GridObject
		// !@#$ GridObject is in FObject constructor (ugly)
		self = new GridObject;
	} else {
		// this is a C++ FObject/GridObject
		void*(*alloc)() = (void*(*)())FIX2PTR(void,allocator);
		self = (FObject *)alloc();
	}
	Ruby keep = rb_ivar_get(mGridFlow, SI(@fobjects));
	self->bself = 0;
	Ruby rself = Data_Wrap_Struct(qlass, CObject_mark, CObject_free, self);
	self->rself = rself;
	rb_hash_aset(keep,rself,Qtrue); // prevent sweeping
	rb_funcall2(rself,SI(initialize),argc,argv);
	return rself;
}

Ruby FObject_s_install(Ruby rself, Ruby name, Ruby inlets2, Ruby outlets2) {
	int inlets, outlets;
	Ruby name2;
	if (SYMBOL_P(name)) {
		name2 = rb_funcall(name,SI(to_str),0);
	} else if (TYPE(name) == T_STRING) {
		name2 = rb_funcall(name,SI(dup),0);
	} else {
		RAISE("expect symbol or string");
	}
	inlets  = TYPE( inlets2)==T_ARRAY ? rb_ary_len( inlets2) : INT( inlets2);
	outlets = TYPE(outlets2)==T_ARRAY ? rb_ary_len(outlets2) : INT(outlets2);
	if ( inlets<0 ||  inlets>9) RAISE("...");
	if (outlets<0 || outlets>9) RAISE("...");
	rb_ivar_set(rself,SI(@ninlets),INT2NUM(inlets));
	rb_ivar_set(rself,SI(@noutlets),INT2NUM(outlets));
	rb_ivar_set(rself,SI(@foreign_name),name2);
	rb_hash_aset(rb_ivar_get(mGridFlow,SI(@fclasses)), name2, rself);
	rb_funcall(rself, SI(install2), 1, name2);
	return Qnil;
}

\def void delete_m () {
	rb_funcall(rb_ivar_get(mGridFlow, SI(@fobjects)),SI(delete),1,rself);
	DATA_PTR(rself) = 0; // really!
	delete this; // really!
}

\classinfo
\end class FObject

/* ---------------------------------------------------------------- */
/* C++<->Ruby bridge for classes/functions in base/number.c */

NumberTypeE NumberTypeE_find (string s) {
	if (number_type_dict.find(s)==number_type_dict.end()) RAISE("unknown number type \"%s\"", s.data());
	return number_type_dict[s]->index;
}

NumberTypeE NumberTypeE_find (Ruby sym) {
	if (TYPE(sym)!=T_STRING) sym=rb_funcall(sym,SI(to_s),0);
	return NumberTypeE_find(string(rb_str_ptr(sym)));
}

/* **************************************************************** */
void define_many_methods(Ruby rself, int n, MethodDecl *methods) {
	for (int i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		char *buf = strdup(md->selector);
		if (strlen(buf)>2 && strcmp(buf+strlen(buf)-2,"_m")==0)
			buf[strlen(buf)-2]=0;
		rb_define_method(rself,buf,(RMethod)md->method,-1);
		free(buf);
	}
}

static Ruby GridFlow_fclass_install(Ruby rself_, Ruby fc_, Ruby super) {
	FClass *fc = FIX2PTR(FClass,fc_);
	//fc = (FClass *)(NUM2ULONG(fc_)<<2); // wtf?
	Ruby rself = super!=Qnil ?
		rb_define_class_under(mGridFlow, fc->name, super) :
		rb_funcall(mGridFlow,SI(const_get),1,rb_str_new2(fc->name));
	define_many_methods(rself,fc->methodsn,fc->methods);
	rb_ivar_set(rself,SI(@allocator),PTR2FIX((void*)(fc->allocator)));
	if (fc->startup) fc->startup(rself);
	return Qnil;
}

//----------------------------------------------------------------
// GridFlow.class
//\class GridFlow_s < patate

typedef void (*Callback)(void*);
static Ruby GridFlow_exec (Ruby rself, Ruby data, Ruby func) {
	void *data2 = FIX2PTR(void,data);
	Callback func2 = (Callback) FIX2PTR(void,func);
	func2(data2);
	return Qnil;
}

static Ruby GridFlow_get_id (Ruby rself, Ruby arg) {
	fprintf(stderr,"%ld\n",arg);
	return INT2NUM((int)arg);
}

Ruby GridFlow_rdtsc (Ruby rself) { return R(rdtsc()).r; }

/* This code handles nested lists because PureData (all versions including 0.40) doesn't do it */
static Ruby GridFlow_handle_braces(Ruby rself, Ruby argv) {
    try {
	int stack[16];
	int stackn=0;
	Ruby *av = rb_ary_ptr(argv);
	int ac = rb_ary_len(argv);
	int j=0;
	for (int i=0; i<ac; ) {
		int close=0;
		if (SYMBOL_P(av[i])) {
			const char *s = rb_sym_name(av[i]);
			while (*s=='(' || *s=='{') {
				if (stackn==16) RAISE("too many nested lists (>16)");
				stack[stackn++]=j;
				s++;
			}
			const char *se = s+strlen(s);
			while (se>s && (se[-1]==')' || se[-1]=='}')) { se--; close++; }
			if (s!=se) {
				Ruby u = rb_str_new(s,se-s);
				av[j++] = rb_funcall(rself,SI(FloatOrSymbol),1,u);
			}
		} else {
			av[j++]=av[i];
		}
		i++;
		while (close--) {
			if (!stackn) RAISE("close-paren without open-paren",av[i]);
			Ruby a2 = rb_ary_new();
			int j2 = stack[--stackn];
			for (int k=j2; k<j; k++) rb_ary_push(a2,av[k]);
			j=j2;
			av[j++] = a2;
		}
	}
	if (stackn) RAISE("too many open-paren (%d)",stackn);
	while (rb_ary_len(argv)>j) rb_ary_pop(argv);
	return rself;
    } catch (Barf *oozy) {
        rb_raise(rb_eArgError,"%s",oozy->text);
    }
}

/* ---------------------------------------------------------------- */

// don't touch.
static void gfmemcopy32(int32 *as, int32 *bs, long n) {
	ptrdiff_t ba = bs-as;
#define FOO(I) as[I] = (as+ba)[I];
		UNROLL_8(FOO,n,as)
#undef FOO

}

void gfmemcopy(uint8 *out, const uint8 *in, long n) {
	for (; n>16; in+=16, out+=16, n-=16) {
		((int32*)out)[0] = ((int32*)in)[0];
		((int32*)out)[1] = ((int32*)in)[1];
		((int32*)out)[2] = ((int32*)in)[2];
		((int32*)out)[3] = ((int32*)in)[3];
	}
	for (; n>4; in+=4, out+=4, n-=4) { *(int32*)out = *(int32*)in; }
	for (; n; in++, out++, n--) { *out = *in; }
}

extern "C" {
void *gfmalloc(size_t n) {
	void *p = memalign(16,n);
	long align = (long)p & 15;
	if (align) fprintf(stderr,"malloc alignment = %ld mod 16\n",align);
	return p;
}
void gffree(void *p) {free(p);}
};

/* ---------------------------------------------------------------- */

void startup_number();
void startup_grid();
void startup_flow_objects();
void startup_format();

#define SDEF(_class_,_name_,_argc_) \
	rb_define_singleton_method(c##_class_,#_name_,(RMethod)_class_##_s_##_name_,_argc_)
#define SDEF2(_name1_,_name2_,_argc_) \
	rb_define_singleton_method(mGridFlow,_name1_,(RMethod)_name2_,_argc_)

STARTUP_LIST(void)

void blargh () {
  void *array[25];
  int nSize = backtrace(array, 25);
  char **symbols = backtrace_symbols(array, nSize);
  for (int i=0; i<nSize; i++) fprintf(stderr,"%d: %s\n",i,symbols[i]);
  free(symbols);
}

// Ruby's entrypoint.
void Init_gridflow () {
        srandom(rdtsc());
	//std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
	std::set_terminate(blargh);

#define FOO(_sym_,_name_) bsym._sym_ = gensym(_name_);
BUILTIN_SYMBOLS(FOO)
#undef FOO
	mGridFlow = EVAL("module GridFlow; CObject = ::Object; class<<self; end; def post_string(s) STDERR.puts s end; self end");
	SDEF2("exec",GridFlow_exec,2);
	SDEF2("get_id",GridFlow_get_id,1);
	SDEF2("rdtsc",GridFlow_rdtsc,0);
	SDEF2("handle_braces!",GridFlow_handle_braces,1);
	SDEF2("fclass_install",GridFlow_fclass_install,2);
	rb_ivar_set(mGridFlow, SI(@fobjects), rb_hash_new());
	rb_ivar_set(mGridFlow, SI(@fclasses), rb_hash_new());
	rb_define_const(mGridFlow, "GF_VERSION", rb_str_new2(GF_VERSION));
	rb_define_const(mGridFlow, "GF_COMPILE_TIME", rb_str_new2(__DATE__", "__TIME__));
	rb_define_const(mGridFlow, "GCC_VERSION", rb_str_new2(GCC_VERSION));
	cFObject = rb_define_class_under(mGridFlow, "FObject", rb_cObject);
	EVAL(
\ruby
	module GridFlow
		class FObject
		def send_out2(*) end
		def self.install2(*) end
		def self.add_creator(name)
			name=name.to_str.dup
			GridFlow.fclasses[name]=self
			GridFlow.add_creator_2 name end
		end
	end
\end ruby
);
	define_many_methods(cFObject,COUNT(FObject_methods),FObject_methods);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, -1);
	ID gbi = SI(gf_bridge_init);
	if (rb_respond_to(rb_cData,gbi)) rb_funcall(rb_cData,gbi,0);
	startup_number();
	startup_grid();
	startup_flow_objects();
	if (!EVAL("begin require 'base/main.rb'; true\n"
		"rescue Exception => e; "
		"STDERR.puts \"can't load: #{$!}\n"
		"backtrace: #{$!.backtrace.join\"\n\"}\n"
		"$: = #{$:.inspect}\"\n; false end")) return;
	startup_format();
	STARTUP_LIST()
	EVAL("h=GridFlow.fclasses; h['#io:window'] = h['#io:quartz']||h['#io:x11']||h['#io:sdl']");
	EVAL("GridFlow.load_user_config");
}

uint64 gf_timeofday () {
	timeval t;
	gettimeofday(&t,0);
	return t.tv_sec*1000000+t.tv_usec;
}

