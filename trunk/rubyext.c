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

/*
'bridge_puredata.c' becomes 'gridflow.pd_linux' which loads Ruby and tells
Ruby to load the other 95% of Gridflow. GridFlow creates the FObject class
and then subclasses it many times, each time calling FObject.install(),
which tells the bridge to register a class in puredata. Puredata
objects have proxies for each of the non-first inlets so that any inlet
may receive any distinguished message. All inlets are typed as 'anything',
and the 'anything' method translates the whole message to Ruby objects and
tries to call a Ruby method of the proper name.
*/

#include "base/grid.h.fcs"
/* resolving conflict: T_OBJECT will be PD's, not Ruby's */
#undef T_OBJECT
#undef T_DATA
//#undef EXTERN
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>

/* for exception-handling in 0.9.0... Linux-only */
#include <exception>
#include <execinfo.h>

#ifdef HAVE_GEM
#include "Base/GemPixDualObj.h"
#endif

/* I don't remember why we have to undefine Ruby's T_DATA;
   on Linux there is no conflict with any other library */
#if RUBY_VERSION_MINOR == 8
#define T_DATA   0x22
#else
#define T_DATA   0x12
#endif

// call f(x) and if fails call g(y)
#define RESCUE(f,x,g,y) rb_rescue2((VALUE (*)(...))(f),(Ruby)(x),(VALUE (*)(...))(g),(Ruby)(y),rb_eException,0);

std::map<string,FClass *> fclasses;
std::map<t_class *,FClass *> fclasses_pd;

/* **************************************************************** */
struct FMessage {
	BFObject *self;
	int winlet;
	t_symbol *selector;
	int ac;
	const t_atom *at;
	bool is_init;
};
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#define rb_sym_name rb_sym_name_r4j
void CObject_free (void *victim) {delete (CObject *)victim;}

/* **************************************************************** */

static RMethod funcall_lookup (BFObject *bself, const char *sel) {
	FClass *fclass = fclasses_pd[*(t_class **)bself];
	int n = fclass->methodsn;
	for (int i=0; i<n; i++) {
		if (strcmp(fclass->methods[i].selector,sel)==0) return fclass->methods[i].method;
	}
	return 0;
}

void call_super(int argc, t_atom *argv) {/* unimplemented */}

static void funcall (BFObject *bself, const char *sel, int argc, t_atom *argv, bool silent=false) {
	RMethod method = funcall_lookup(bself,sel);
	if (method) {method(bself->self,argc,(t_atom2 *)argv); return;}
	if (!silent) pd_error((t_pd *)bself, "method not found: '%s'",sel);
}

static void funcall_rescue(BFObject *bself, const char *sel, int argc, t_atom *argv) {
	try {funcall(bself,sel,argc,argv);} catch (Barf *oozy) {pd_error(bself,"%s",oozy->text);}
}

//****************************************************************
// BFObject

struct BFProxy : t_object {
	BFObject *parent;
	t_inlet *inlet;
	int id;
};

static t_class *find_bfclass (t_symbol *sym) {
	t_atom a[1];
	SETSYMBOL(a,sym);
	char buf[MAXPDSTRING];
	if (sym==&s_list) strcpy(buf,"list"); else atom_string(a,buf,sizeof(buf));
	string name = string(buf);
	if (fclasses.find(name)==fclasses.end()) {post("GF: class not found: '%s'",buf); return 0;}
	return fclasses[name]->bfclass;
}

static t_class *BFProxy_class;

static void BFObject_method_missing (BFObject *bself, int winlet, t_symbol *selector, int ac, t_atom *at) {
    try {
	t_atom argv[ac+1];
	for (int i=0; i<ac; i++) argv[i+1] = at[i];
	int argc = handle_braces(ac,argv+1);
	SETFLOAT(argv+0,winlet);
	char buf[256];
	sprintf(buf,"_n_%s",selector->s_name);
	if (funcall_lookup(bself,buf)) {
		funcall_rescue(bself,buf,argc+1,argv);
	} else {
		sprintf(buf,"_%d_%s",winlet,selector->s_name);
		if (funcall_lookup(bself,buf)) {
			funcall_rescue(bself,buf,argc,argv+1);
		} else {
			SETSYMBOL(argv+0,gensym(buf));
			funcall_rescue(bself,"method_missing",argc+1,argv);
		}
	}
    } catch (Barf *oozy) {pd_error(bself,"%s",oozy->text);}
}
static void BFObject_method_missing0 (BFObject *self, t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self,0,s,argc,argv);
}
static void BFProxy_method_missing   (BFProxy *self,  t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self->parent,self->id,s,argc,argv);
}

typedef void *(*t_constructor)(MESSAGE);
static void CObject_mark (void *z) {}

static void *BFObject_init (t_symbol *classsym, int ac, t_atom *at) {
	t_class *qlass = find_bfclass(classsym);
	if (!qlass) return 0;
	BFObject *bself = (BFObject *)pd_new(qlass);

	int argc = ac;
	t_atom argv[argc];
	for (int i=0; i<argc; i++) argv[i] = at[i];
	argc = handle_braces(argc,argv);
	pd_post(classsym->s_name,argc,argv);
#ifdef HAVE_GEM
	CPPExtern::m_holder = (t_object *)bself;
#ifdef HAVE_HOLDNAME
	CPPExtern::m_holdname = "keep_gem_happy";
#endif
#endif

	int j;
	for (j=0; j<argc; j++) if (argv[j].a_type==A_COMMA) break;
	t_constructor alloc = fclasses[string(classsym->s_name)]->allocator;
	FObject *self = (FObject *)alloc(0,j,(t_atom2 *)argv);
	self->bself = 0;

	self->bself = bself;
	bself->self = self;
	bself->mom = 0;
#ifdef HAVE_GEM
	bself->gemself = (CPPExtern *)((void **)self+11); /* not 64-bit-safe */
	CPPExtern::m_holder = 0;
#ifdef HAVE_HOLDNAME
	CPPExtern::m_holdname=0;
#endif
#endif
	bself->nin  = 1;
	bself->nout = 0;
	bself->in  = new  BFProxy*[1];
	bself->out = new t_outlet*[1];
	bself->ninlets_set( fclasses[classsym->s_name]->ninlets);
	bself->noutlets_set(fclasses[classsym->s_name]->noutlets);
	funcall(bself,"initialize2",0,0,true);
	bself->mom = (t_canvas *)canvas_getcurrent();
	while (j<argc) {
		j++;
		int k=j;
		for (; j<argc; j++) if (argv[j].a_type==A_COMMA) break;
		if (argv[k].a_type==A_SYMBOL) pd_typedmess((t_pd *)bself,argv[k].a_w.w_symbol,j-k-1,argv+k+1);
	}
	return bself;
}

static void BFObject_delete_1 (FMessage *fm) {}

static void BFObject_delete (BFObject *bself) {
	try {funcall(bself,"delete",0,0,true);} catch (Barf *oozy) {pd_error(bself,"%s",oozy->text);}
}

//****************************************************************

\class FObject

// from pd/src/g_canvas.c
struct _canvasenvironment {
    t_symbol *ce_dir;   /* directory patch lives in */
    int ce_argc;        /* number of "$" arguments */
    t_atom *ce_argv;    /* array of "$" arguments */
    int ce_dollarzero;  /* value of "$0" */
};

#include "bundled/pd/g_canvas.h"

static void BFObject_undrawio (BFObject *bself) {
#ifndef HAVE_DESIREDATA
	if (!bself->mom || !glist_isvisible(bself->mom)) return;
	t_rtext *rt = glist_findrtext(bself->mom,bself);
	if (!rt) return;
	glist_eraseiofor(bself->mom,bself,rtext_gettag(rt));
#endif
}

static void BFObject_redraw (BFObject *bself) {
#ifndef HAVE_DESIREDATA
	if (!bself->mom || !glist_isvisible(bself->mom)) return;
	t_rtext *rt = glist_findrtext(bself->mom,bself);
	if (!rt) return;
	gobj_vis((t_gobj *)bself,bself->mom,0);
	gobj_vis((t_gobj *)bself,bself->mom,1);
	canvas_fixlinesfor(bself->mom,(t_text *)bself);
#endif
}

/* warning: deleting inlets that are connected will cause pd to crash */
void BFObject::ninlets_set (int n) {
	if (!this) RAISE("there is no bself");
	if (n<1) RAISE("ninlets_set: n=%d must be at least 1",n);
	BFObject_undrawio(this);
	if (nin<n) {
		BFProxy **noo = new BFProxy*[n];
		memcpy(noo,in,nin*sizeof(BFProxy*));
		delete[] in;
		in = noo;
		while (nin<n) {
			BFProxy *p = in[nin] = (BFProxy *)pd_new(BFProxy_class);
			p->parent = this;
			p->id = nin;
			p->inlet = inlet_new(this, &p->ob_pd, 0,0);
			nin++;
		}
	} else {
		while (nin>n) {
			nin--;
			inlet_free(in[nin]->inlet);
			delete in[nin];
		}
	}
	BFObject_redraw(this);
}
/* warning: deleting outlets that are connected will cause pd to crash */
void BFObject::noutlets_set (int n) {
	if (!this) RAISE("there is no bself");
	if (n<0) RAISE("noutlets_set: n=%d must be at least 0",n);
	BFObject_undrawio(this);
	if (nout<n) {
		t_outlet **noo = new t_outlet*[n>0?n:1];
		memcpy(noo,out,nout*sizeof(t_outlet*));
		delete[] out;
		out = noo;
		while (nout<n) out[nout++] = outlet_new(this,&s_anything);
	} else {
		while (nout>n) outlet_free(out[--nout]);
	}
	BFObject_redraw(this);
}

void add_creator2(FClass *fclass, const char *name) {
	fclasses[string(name)] = fclass;
	class_addcreator((t_newmethod)BFObject_init,gensym((char *)name),A_GIMME,0);
}

//****************************************************************

struct t_namelist;
extern t_namelist *sys_searchpath, *sys_helppath;
extern "C" void namelist_append_files(t_namelist *, char *);
static void add_to_path(char *dir) {
	char bof[1024];
	sprintf(bof,"%s/abstractions",dir);        namelist_append_files(sys_searchpath,bof);
	sprintf(bof,"%s/deprecated",dir);          namelist_append_files(sys_searchpath,bof);
	sprintf(bof,"%s/doc/flow_classes",dir);    namelist_append_files(sys_helppath,  bof);
}

//----------------------------------------------------------------

void fclass_install(FClass *fclass, const char *super) {
	//fclass->super = fclasses[super];
	if (fclass->startup) fclass->startup(fclass);
}

void install2(FClass *fclass, const char *name, int inlets, int outlets) {
	fclass->ninlets = inlets;
	fclass->noutlets = outlets;
	fclass->name = string(name);
	fclass->bfclass = class_new(gensym((char *)name), (t_newmethod)BFObject_init, (t_method)BFObject_delete,
		sizeof(BFObject), CLASS_DEFAULT, A_GIMME,0);
	fclasses[string(name)] = fclass;
	fclasses_pd[fclass->bfclass] = fclass;
	class_addanything(fclass->bfclass,(t_method)BFObject_method_missing0);
}

/* This code handles nested lists because PureData (all versions including 0.40) doesn't do it */
int handle_braces(int ac, t_atom *av) {
	int stack[16];
	int stackn=0;
	int j=0;
	t_binbuf *buf = binbuf_new(); // leaks if RAISE
	for (int i=0; i<ac; ) {
		int close=0;
		if (av[i].a_type==A_SYMBOL) {
			const char *s = av[i].a_w.w_symbol->s_name;
			while (*s=='(') {
				if (stackn==16) RAISE("too many nested lists (>16)");
				stack[stackn++]=j;
				s++;
			}
			const char *se = s+strlen(s);
			while (se>s && se[-1]==')') {se--; close++;}
			if (s!=se) {
				binbuf_text(buf,(char *)s,se-s);
				av[j++] = binbuf_getvec(buf)[0];
			}
		} else av[j++]=av[i];
		i++;
		while (close--) {
			if (!stackn) RAISE("close-paren without open-paren",av[i]);
			t_binbuf *a2 = binbuf_new();
			int j2 = stack[--stackn];
			binbuf_add(a2,j-j2,av+j2);
			j=j2;
			SETLIST(av+j,a2);
			j++;
		}
	}
	//if (stackn) RAISE("too many open-paren (%d)",stackn);
	if (stackn) post("too many open-paren (%d)",stackn);
	return j;
}

\classinfo {}
\end class

void startup_number();
void startup_grid();
void startup_flow_objects();
void startup_format();
STARTUP_LIST(void)

void blargh () {
  void *array[25];
  int nSize = backtrace(array, 25);
  char **symbols = backtrace_symbols(array, nSize);
  for (int i=0; i<nSize; i++) fprintf(stderr,"%d: %s\n",i,symbols[i]);
  free(symbols);
}

#undef SDEF
#define SDEF(_class_,_name_,_argc_)   rb_define_singleton_method(c##_class_,#_name_,(RMethod)_class_##_s_##_name_,_argc_)
#define SDEF2(_name1_,_name2_,_argc_) rb_define_singleton_method(mGridFlow,_name1_,(RMethod)GridFlow_s_##_name2_,_argc_)

// note: contrary to what m_pd.h says, pd_getfilename() and pd_getdirname()
// don't exist; also, canvas_getcurrentdir() isn't available during setup
// (segfaults), in addition to libraries not being canvases ;-)
// AND ALSO, CONTRARY TO WHAT m_pd.h SAYS, open_via_path()'s args are reversed!!!
extern "C" void gridflow_setup () {
    post("GridFlow " GF_VERSION ", Copyright © 2001-2008 Mathieu Bouchard");
    //std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
    std::set_terminate(blargh);
    try {
	char *dirname   = new char[MAXPDSTRING];
	char *dirresult = new char[MAXPDSTRING];
	char *nameresult;
	if (getcwd(dirname,MAXPDSTRING)<0) {post("AAAARRRRGGGGHHHH!"); exit(69);}
	int       fd=open_via_path(dirname,"gridflow/gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd<0) fd=open_via_path(dirname,         "gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd>=0) close(fd); else post("%s was not found via the -path!","gridflow"PDSUF);
	/* nameresult is only a pointer in dirresult space so don't delete[] it. */
	add_to_path(dirresult);
	BFProxy_class = class_new(gensym("gf.proxy"),0,0,sizeof(BFProxy),CLASS_PD|CLASS_NOINLET, A_NULL);
	class_addanything(BFProxy_class,BFProxy_method_missing);
	gf_data_path.push_back(string(dirresult)+"/images");
        srandom(rdtsc());
#define FOO(_sym_,_name_) bsym._sym_ = gensym(_name_);
BUILTIN_SYMBOLS(FOO)
#undef FOO
	startup_number();
	startup_grid();
	startup_flow_objects();
	startup_format();
	STARTUP_LIST()
	delete[] dirresult;
	delete[] dirname;
	signal(SIGSEGV,SIG_DFL);
	signal(SIGABRT,SIG_DFL);
	signal(SIGBUS, SIG_DFL);
    } catch (Barf *oozy) {post("Init_gridflow error: %s",oozy->text);}
}
