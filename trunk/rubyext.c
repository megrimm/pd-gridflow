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
#define RESCUE(f,x,g,y) rb_rescue2((RMethod)(f),(Ruby)(x),(RMethod)(g),(Ruby)(y),rb_eException,0);

std::map<string,FClass *> fclasses;
std::map<Ruby,FClass *> fclasses_ruby;

/* **************************************************************** */
struct BFObject;

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
static const char *rb_sym_name(Ruby sym) {return rb_id2name(SYM2ID(sym));}
void CObject_free (void *victim) {delete (CObject *)victim;}

Ruby cPointer=0;
Ruby Pointer_s_new (void *ptr) {
	Pointer *self = new Pointer(ptr);
	return Data_Wrap_Struct(cPointer, 0, CObject_free, self);
}
void *Pointer_get (Ruby rself) {
	DGS(Pointer);
	return self->p;
}

static Ruby make_error_message () {
	char buf[1000];
	sprintf(buf,"%s: %s",rb_class2name(rb_obj_class(ruby_errinfo)), rb_str_ptr(rb_funcall(ruby_errinfo,SI(to_s),0)));
	Ruby ary = rb_ary_new();
	Ruby backtrace = rb_funcall(ruby_errinfo,SI(backtrace),0);
	rb_ary_push(ary,rb_str_new2(buf));
	for (int i=0; i<2 && i<rb_ary_len(backtrace); i++) rb_ary_push(ary,rb_funcall(backtrace,SI([]),1,INT2NUM(i)));
//	rb_ary_push(ary,rb_funcall(rb_funcall(backtrace,SI(length),0),SI(to_s),0));
	return ary;
}

/* **************************************************************** */

struct RMessage {VALUE rself; ID sel; int argc; VALUE *argv;};
static VALUE rb_funcall_myrescue_1(RMessage *rm) {return rb_funcall2(rm->rself,rm->sel,rm->argc,rm->argv);}
static Ruby rb_funcall_myrescue_2 (RMessage *rm) {
	Ruby error_array = make_error_message();
//	for (int i=0; i<rb_ary_len(error_array); i++) post("%s\n",rb_str_ptr(rb_ary_ptr(error_array)[i]));
	post("%s",rb_str_ptr(rb_funcall(error_array,SI(join),1,rb_str_new2("\n"))));
	return Qnil;
}
static VALUE rb_funcall_myrescue(VALUE rself, ID sel, int argc, ...) {
	va_list foo;
	va_start(foo,argc);
	VALUE argv[argc];
	for (int i=0; i<argc; i++) argv[i] = va_arg(foo,VALUE);
	RMessage rm = { rself, sel, argc, argv };
	va_end(foo);
	return RESCUE(rb_funcall_myrescue_1,&rm,rb_funcall_myrescue_2,&rm);
}
static VALUE rb_funcall_myrescue2(VALUE rself, ID sel, int argc, Ruby *argv) {
	RMessage rm = { rself, sel, argc, argv };
	return RESCUE(rb_funcall_myrescue_1,&rm,rb_funcall_myrescue_2,&rm);
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

static void Bridge_export_value(Ruby arg, t_atom *at) {
	if      (INTEGER_P(arg)) SETFLOAT(at,NUM2INT(arg));
	else if ( SYMBOL_P(arg)) SETSYMBOL(at,gensym((char *)rb_sym_name(arg)));
	else if (  FLOAT_P(arg)) SETFLOAT(at,RFLOAT(arg)->value);
	else if (rb_obj_class(arg)==cPointer) SETPOINTER(at,(t_gpointer*)Pointer_get(arg));
	else if (TYPE(arg)==T_ARRAY) {
		t_binbuf *b = binbuf_new();
		t_atom a;
		int n=rb_ary_len(arg);
		Ruby *p=rb_ary_ptr(arg);
		for (int i=0; i<n; i++) {Bridge_export_value(p[i],&a); binbuf_add(b,1,&a);}
		SETLIST(at,b);
	} else RAISE("cannot convert argument of class '%s'",rb_str_ptr(rb_funcall(rb_funcall(arg,SI(class),0),SI(inspect),0)));
}

void ruby2pd (int argc, Ruby *argv, t_atom *at) {
	for (int i=0; i<argc; i++) Bridge_export_value(argv[i],&at[i]);
}

static Ruby Bridge_import_value(const t_atom *at) {
	t_atomtype t = at->a_type;
	if (t==A_SYMBOL)  return ID2SYM(rb_intern(at->a_w.w_symbol->s_name));
	if (t==A_FLOAT )  return rb_float_new(at->a_w.w_float);
	if (t==A_POINTER) return Pointer_s_new(at->a_w.w_gpointer);
	if (t==A_LIST) {
		Ruby a = rb_ary_new();
		t_binbuf *b = (t_binbuf *)at->a_w.w_gpointer;
		int n = binbuf_getnatom(b);
		for (int i=0; i<n; i++) rb_ary_push(a,Bridge_import_value(binbuf_getvec(b)+i));
		return a;
	}
	return Qnil; /* unknown */
}

void pd2ruby (int argc, Ruby *argv, t_atom *at) {
	for (int i=0; i<argc; i++) argv[i] = Bridge_import_value(&at[i]);
}

static Ruby BFObject_method_missing_1 (FMessage *fm) {
	t_atom at[fm->ac];
	for (int i=0; i<fm->ac; i++) at[i] = fm->at[i];
	int argc = handle_braces(fm->ac,at);
	Ruby argv[argc+1];
	argv[0] = fm->winlet*2+1; // convert to Ruby Integer
	for (int i=0; i<argc; i++) argv[1+i] = Bridge_import_value(at+i);
	Ruby rself = fm->self->rself;
	DGS(FObject);
	char buf[256];
	sprintf(buf,"_n_%s",fm->selector->s_name);
	if (rb_obj_respond_to(rself,rb_intern(buf),0)) {
		rb_funcall_myrescue2(rself,rb_intern(buf),argc+1,argv);
	} else {
		sprintf(buf,"_%d_%s",fm->winlet,fm->selector->s_name);
		rb_funcall_myrescue2(rself,rb_intern(buf),argc,argv+1);
	}
	return Qnil;
}

static Ruby BFObject_rescue (FMessage *fm) {
	Ruby error_array = make_error_message();
	if (fm->self) pd_error(fm->self,"%s",rb_str_ptr(rb_funcall(error_array,SI(join),1,rb_str_new2("\n"))));
	fprintf(stderr,"%s\n",rb_str_ptr(rb_funcall(error_array,SI(join),1,rb_str_new2("\n"))));
	if (fm->self && fm->is_init) fm->self = 0;
	return Qnil;
}

static void BFObject_method_missing (BFObject *bself, int winlet, t_symbol *selector, int ac, t_atom *at) {
	FMessage fm = { bself, winlet, selector, ac, at, false };
	if (!bself->rself) {pd_error(bself,"message to a dead object. (supposed to be impossible)"); return;}
	try {
		RESCUE(BFObject_method_missing_1,&fm,BFObject_rescue,(Ruby)&fm);
	} catch (Barf *oozy) {post("error: %s",oozy->text);}
}
static void BFObject_method_missing0 (BFObject *self, t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self,0,s,argc,argv);
}
static void BFProxy_method_missing   (BFProxy *self,  t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self->parent,self->id,s,argc,argv);
}

typedef void *(*t_constructor)(MESSAGE);
static void CObject_mark (void *z) {}

static Ruby FObject_s_new(Ruby argc, Ruby *argv, const char *name) {
	Ruby qlass = fclasses[string(name)]->rself;
	t_constructor alloc = (t_constructor)FIX2PTR(void,rb_ivar_get(qlass,SI(@allocator)));
	//t_constructor alloc2 = fclasses1[string(name)]->allocator;
	//fprintf(stderr,"allocator is %p or %p\n", alloc,alloc2);
	FObject *self;
	// this is a C++ FObject/GridObject
	t_atom2 argv2[argc];
	ruby2pd(argc,argv,argv2);
	self = (FObject *)alloc(0,argc,argv2);
	Ruby keep = rb_ivar_get(mGridFlow, SI(@fobjects));
	self->bself = 0;
	Ruby rself = Data_Wrap_Struct(qlass, CObject_mark, CObject_free, self);
	rb_hash_aset(keep,rself,Qtrue); // prevent sweeping
	rb_funcall2(rself,SI(initialize),argc,argv);
	return rself;
}

static Ruby BFObject_init_1 (FMessage *fm) {
	int argc = fm->ac;
	t_atom at[argc];
	for (int i=0; i<argc; i++) at[i] = fm->at[i];
	argc = handle_braces(argc,at);
	pd_post(fm->selector->s_name,argc,at);
	BFObject *bself = fm->self;
#ifdef HAVE_GEM
	CPPExtern::m_holder = (t_object *)bself;
#ifdef HAVE_HOLDNAME
	CPPExtern::m_holdname = "keep_gem_happy";
#endif
#endif

	int j;
	for (j=0; j<argc; j++) if (at[j].a_type==A_COMMA) break;
	Ruby argv[j];
	for (int i=0; i<j; i++) argv[i] = Bridge_import_value(at+i);
	Ruby rself = FObject_s_new(j,argv,fm->selector->s_name);
	DGS(FObject);
	self->bself = bself;
	bself->rself = rself;
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
	bself->ninlets_set( fclasses_ruby[rb_funcall(rself,SI(class),0)]->ninlets);
	bself->noutlets_set(fclasses_ruby[rb_funcall(rself,SI(class),0)]->noutlets);
	rb_funcall(rself,SI(initialize2),0);
	bself->mom = (t_canvas *)canvas_getcurrent();
	while (j<argc) {
		j++;
		int k=j;
		for (; j<argc; j++) if (at[j].a_type==A_COMMA) break;
		if (at[k].a_type==A_SYMBOL) pd_typedmess((t_pd *)bself,at[k].a_w.w_symbol,j-k-1,at+k+1);
	}
	return rself;
}

static void *BFObject_init (t_symbol *classsym, int ac, t_atom *at) {
	t_class *qlass = find_bfclass(classsym);
	if (!qlass) return 0;
	BFObject *bself = (BFObject *)pd_new(qlass);
	FMessage fm = {self:bself, winlet:-1, selector:classsym, ac:ac, at:at, is_init:true};
	long r = RESCUE(BFObject_init_1,&fm,BFObject_rescue,&fm);
	return r==Qnil ? 0 : (void *)bself; // return NULL if broken object
}

static void BFObject_delete_1 (FMessage *fm) {
	if (fm->self->rself) rb_funcall(fm->self->rself,SI(delete),0);
	else post("BFObject_delete is NOT handling BROKEN object at %*lx",2*sizeof(long),(long)fm);
}

static void BFObject_delete (BFObject *bself) {
	FMessage fm = {self:bself, winlet:-1, selector:gensym("delete"), ac:0, at:0, is_init:false};
	RESCUE(BFObject_delete_1,&fm,BFObject_rescue,&fm);
}

//****************************************************************

\class Pointer : CObject
\end class {}

static void BFObject_class_init_1 (t_class *qlass) {class_addanything(qlass,(t_method)BFObject_method_missing0);}
\class FObject

static t_pd *rp_to_pd (Ruby pointer) {
       Pointer *foo;
       Data_Get_Struct(pointer,Pointer,foo);
       return (t_pd *)foo->p;
}

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
	if ((Ruby)this==Qnil) RAISE("bself is nil");
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

/* revival of the stack end crutch of 2003-2005... just in case */
static t_clock *hack;
extern "C" void Init_stack(void*);
static void ruby_stack_end_hack () {
	int bogus;
	Init_stack(&bogus);
	clock_free(hack);
}

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

void define_many_methods(Ruby rself, int n, MethodDecl *methods) {
	for (int i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		char *buf = strdup(md->selector);
		if (strlen(buf)>2 && strcmp(buf+strlen(buf)-2,"_m")==0) buf[strlen(buf)-2]=0;
		rb_define_method(rself,buf,(RMethod)md->method,-1);
		free(buf);
	}
}

void fclass_install(FClass *fclass, const char *super) {
	fclass->rself = super ?
		rb_define_class_under(mGridFlow, fclass->rubyname, rb_funcall(mGridFlow,SI(const_get),1,rb_str_new2(super))) :
		rb_funcall(mGridFlow,SI(const_get),1,rb_str_new2(fclass->rubyname));
	define_many_methods(fclass->rself,fclass->methodsn,fclass->methods);
	rb_ivar_set(fclass->rself,SI(@allocator),PTR2FIX((void*)(fclass->allocator)));
	if (fclass->startup) fclass->startup(fclass);
}

void install2(FClass *fclass, const char *name, int inlets, int outlets) {
	fclasses_ruby[fclass->rself] = fclass;
	fclass->ninlets = inlets;
	fclass->noutlets = outlets;
	fclass->name = string(name);
	fclass->bfclass = class_new(gensym((char *)name), (t_newmethod)BFObject_init, (t_method)BFObject_delete,
		sizeof(BFObject), CLASS_DEFAULT, A_GIMME,0);
	fclasses[string(name)] = fclass;
	FMessage fm = {0, -1, 0, 0, 0, false};
	RESCUE(BFObject_class_init_1,fclass->bfclass,BFObject_rescue,&fm);
}

Ruby FObject_delete (Ruby rself) {
	DGS(FObject);
	rb_funcall(rb_ivar_get(mGridFlow, SI(@fobjects)),SI(delete),1,rself);
	DATA_PTR(rself) = 0; // really!
	delete self; // really!
	return Qnil;
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

Ruby FObject_dummy () {return Qnil;}

// note: contrary to what m_pd.h says, pd_getfilename() and pd_getdirname()
// don't exist; also, canvas_getcurrentdir() isn't available during setup
// (segfaults), in addition to libraries not being canvases ;-)
// AND ALSO, CONTRARY TO WHAT m_pd.h SAYS, open_via_path()'s args are reversed!!!
extern "C" void gridflow_setup () {
    //std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
    std::set_terminate(blargh);
    try {
	char *foo[] = {"Ruby-for-PureData","-e",";"};
	char *dirname   = new char[MAXPDSTRING];
	char *dirresult = new char[MAXPDSTRING];
	char *nameresult;
	if (getcwd(dirname,MAXPDSTRING)<0) {post("AAAARRRRGGGGHHHH!"); exit(69);}
	int       fd=open_via_path(dirname,"gridflow/gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd<0) fd=open_via_path(dirname,         "gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd>=0) close(fd); else post("%s was not found via the -path!","gridflow"PDSUF);
	/* nameresult is only a pointer in dirresult space so don't delete[] it. */
	add_to_path(dirresult);
	ruby_init();
	ruby_options(COUNT(foo),foo);
	BFProxy_class = class_new(gensym("gf.proxy"),0,0,sizeof(BFProxy),CLASS_PD|CLASS_NOINLET, A_NULL);
	class_addanything(BFProxy_class,BFProxy_method_missing);
	mGridFlow = EVAL("module GridFlow; class<<self; end; Pd=GridFlow; end");
	gf_data_path.push_back(string(dirresult)+"/images");
        srandom(rdtsc());
#define FOO(_sym_,_name_) bsym._sym_ = gensym(_name_);
BUILTIN_SYMBOLS(FOO)
#undef FOO
	mGridFlow = EVAL("module GridFlow; CObject = ::Object; self end");
	rb_ivar_set(mGridFlow, SI(@fobjects), rb_hash_new());
	cFObject = rb_define_class_under(mGridFlow, "FObject", rb_cObject);
	define_many_methods(cFObject,COUNT(FObject_methods),FObject_methods);
	Ruby fo = cFObject;
	rb_define_method(fo,"delete",      (RMethod)FObject_delete,0);
	rb_define_method(fo,"initialize",  (RMethod)FObject_dummy,-1);
	rb_define_method(fo,"initialize2", (RMethod)FObject_dummy,-1);
	\startall
	rb_define_singleton_method(EVAL("GridFlow::Pointer"),"new", (RMethod)Pointer_s_new, 1);
	cPointer = EVAL("GridFlow::Pointer");
	startup_number();
	startup_grid();
	startup_flow_objects();
	startup_format();
	STARTUP_LIST()
	delete[] dirresult;
	delete[] dirname;
	hack = clock_new((void*)0,(t_method)ruby_stack_end_hack);
	clock_delay(hack,0);
	signal(SIGSEGV,SIG_DFL);
	signal(SIGABRT,SIG_DFL);
	signal(SIGBUS, SIG_DFL);
    } catch (Barf *oozy) {post("Init_gridflow error: %s",oozy->text);}
}
