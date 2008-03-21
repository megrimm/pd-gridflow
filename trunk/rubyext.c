/*
	$Id$

	GridFlow
	Copyright (c) 2001-2006 by Mathieu Bouchard

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

bool print_class_list;

#define IS_BRIDGE
#include "base/grid.h.fcs"
/* resolving conflict: T_OBJECT will be PD's, not Ruby's */
#undef T_OBJECT
#undef T_DATA
//#undef EXTERN
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>

#ifdef HAVE_DESIREDATA /* assuming a recent enough version */
/* desire.h ought to be fixed so that it can be used outside of desiredata... */
//#include <desire.h>
typedef struct _canvasenvironment t_canvasenvironment;
extern "C" {
EXTERN t_canvasenvironment *canvas_getenv(t_canvas *x);
};
#else
extern "C" {
#include "g_canvas.h"
};
#endif

#define CObject_free CObject_freeee

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
#define RESCUE(f,x,g,y) \
  rb_rescue2((RMethod)(f),(Ruby)(x),(RMethod)(g),(Ruby)(y),rb_eException,0);

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

static BuiltinSymbols *syms;

void CObject_free (void *victim) {
	CObject *self = (CObject *)victim;
	if (!self->rself) {
		_L_ fprintf(stderr,"attempt to free object that has no rself\n");
		abort();
	}
	self->rself = 0; /* paranoia */
	delete self;
}

/* can't even refer to the other mGridFlow because we don't link
   statically to the other gridflow.so */
static Ruby mGridFlow2=0;
Ruby cPointer=0;

Ruby Pointer_s_new (void *ptr) {
	Pointer *self = new Pointer(ptr);
	self->rself = Data_Wrap_Struct(cPointer, 0, CObject_free, self);
	//fprintf(stderr,"making rself=%p self=%p, a Pointer to %p\n",self->rself,self,ptr);
	return self->rself;
}
void *Pointer_get (Ruby rself) {
	DGS(Pointer);
	return self->p;
}

static Ruby make_error_message () {
	char buf[1000];
	sprintf(buf,"%s: %s",rb_class2name(rb_obj_class(ruby_errinfo)),
		rb_str_ptr(rb_funcall(ruby_errinfo,SI(to_s),0)));
	Ruby ary = rb_ary_new();
	Ruby backtrace = rb_funcall(ruby_errinfo,SI(backtrace),0);
	rb_ary_push(ary,rb_str_new2(buf));
	for (int i=0; i<2 && i<rb_ary_len(backtrace); i++)
		rb_ary_push(ary,rb_funcall(backtrace,SI([]),1,INT2NUM(i)));
//	rb_ary_push(ary,rb_funcall(rb_funcall(backtrace,SI(length),0),SI(to_s),0));
	return ary;
}

static int ninlets_of (Ruby qlass) {
	ID id = SYM2ID(syms->iv_ninlets);
	if (!rb_ivar_defined(qlass,id)) RAISE("no inlet count");
	return INT(rb_ivar_get(qlass,id));
}

static int noutlets_of (Ruby qlass) {
	ID id = SYM2ID(syms->iv_noutlets);
	if (!rb_ivar_defined(qlass,id)) RAISE("no outlet count");
	return INT(rb_ivar_get(qlass,id));
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
	char buf[4096];
	if (sym==&s_list) strcpy(buf,"list"); else atom_string(a,buf,sizeof(buf));
	Ruby v = rb_hash_aref(rb_ivar_get(mGridFlow2, SI(@fclasses)), rb_str_new2(buf));
	if (v==Qnil) {
		post("GF: class not found: '%s'",buf);
		return 0;
	}
	if (Qnil==rb_ivar_get(v,SI(@bfclass))) {
		post("@bfclass missing for '%s'",buf);
		return 0;
	}
	return FIX2PTR(t_class,rb_ivar_get(v,SI(@bfclass)));
}

static t_class *BFProxy_class;

static void Bridge_export_value(Ruby arg, t_atom *at) {
	if      (INTEGER_P(arg)) SETFLOAT(at,NUM2INT(arg));
	else if ( SYMBOL_P(arg)) SETSYMBOL(at,gensym((char *)rb_sym_name(arg)));
	else if (  FLOAT_P(arg)) SETFLOAT(at,RFLOAT(arg)->value);
	else if (rb_obj_class(arg)==cPointer) SETPOINTER(at,(t_gpointer*)Pointer_get(arg));
	else RAISE("cannot convert argument of class '%s'",
		rb_str_ptr(rb_funcall(rb_funcall(arg,SI(class),0),SI(inspect),0)));
}

static Ruby Bridge_import_value(const t_atom *at) {
	t_atomtype t = at->a_type;
	if (t==A_SYMBOL)  return ID2SYM(rb_intern(at->a_w.w_symbol->s_name));
	if (t==A_FLOAT )  return rb_float_new(at->a_w.w_float);
	if (t==A_POINTER) return Pointer_s_new(at->a_w.w_gpointer);
	return Qnil; /* unknown */
}

static Ruby BFObject_method_missing_1 (FMessage *fm) {
	Ruby argv[fm->ac+2];
	argv[0] = INT2NUM(fm->winlet);
	argv[1] = ID2SYM(rb_intern(fm->selector->s_name));
	for (int i=0; i<fm->ac; i++) argv[2+i] = Bridge_import_value(fm->at+i);
	rb_funcall2(fm->self->rself,SI(send_in),fm->ac+2,argv);
	return Qnil;
}

static Ruby BFObject_rescue (FMessage *fm) {
	Ruby error_array = make_error_message();
//	for (int i=0; i<rb_ary_len(error_array); i++)
//		post("%s\n",rb_str_ptr(rb_ary_ptr(error_array)[i]));
	if (fm->self) pd_error(fm->self,"%s",rb_str_ptr(
		rb_funcall(error_array,SI(join),1,rb_str_new2("\n"))));
	if (fm->self && fm->is_init) fm->self = 0;
	return Qnil;
}

static void BFObject_method_missing (BFObject *bself,
int winlet, t_symbol *selector, int ac, t_atom *at) {
	FMessage fm = { bself, winlet, selector, ac, at, false };
	if (!bself->rself) {
		pd_error(bself,"message to a dead object. (supposed to be impossible)");
		return;
	}
	RESCUE(BFObject_method_missing_1,&fm,BFObject_rescue,(Ruby)&fm);
}

static void BFObject_method_missing0 (BFObject *self, t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self,0,s,argc,argv);
}
static void BFProxy_method_missing   (BFProxy *self,  t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self->parent,self->id,s,argc,argv);
}

static Ruby BFObject_init_1 (FMessage *fm) {
	Ruby argv[fm->ac+1];
	for (int i=0; i<fm->ac; i++) argv[i+1] = Bridge_import_value(fm->at+i);
	// s_list is broken in some (?) versions of pd
	argv[0] = fm->selector==&s_list ?
		argv[0]=rb_str_new2("list") : rb_str_new2(fm->selector->s_name);
	BFObject *bself = fm->self;
#ifdef HAVE_GEM
	CPPExtern::m_holder = (t_object *)bself;
#ifdef HAVE_HOLDNAME
	CPPExtern::m_holdname = "keep_gem_happy";
#endif
#endif
	Ruby rself = rb_funcall2(rb_const_get(mGridFlow2,SI(FObject)),SI([]),fm->ac+1,argv);
	DGS(FObject);
	self->bself = bself;
	bself->rself = rself;
	bself->mom = 0;
#ifdef HAVE_GEM
	bself->gemself = (CPPExtern *)((void **)self+11); /* not 64-bit-safe */
	CPPExtern::m_holder = NULL;
#ifdef HAVE_HOLDNAME
	CPPExtern::m_holdname=NULL;
#endif
#endif
	int ninlets  =  ninlets_of(rb_funcall(rself,SI(class),0));
	int noutlets = noutlets_of(rb_funcall(rself,SI(class),0));
	bself->nin  = 1;
	bself->nout = 0;
	self->bself->in  = new  BFProxy*[1];
	self->bself->out = new t_outlet*[1];
	bself->ninlets_set(ninlets);
	bself->noutlets_set(noutlets);
/*
	bself->nin  =   ninlets;
	bself->nout =  noutlets;
	for (int i=1; i<ninlets; i++) {
		BFProxy *p = (BFProxy *)pd_new(BFProxy_class);
		p->parent = bself;
		p->inlet = i;
		inlet_new(bself, &p->ob_pd, 0,0);
	}
	self->bself->out = new t_outlet*[noutlets];
	for (int i=0; i<noutlets; i++) {
		bself->out[i] = outlet_new(bself,&s_anything);
	}
*/
	rb_funcall(rself,SI(initialize2),0);
	bself->mom = (t_canvas *)canvas_getcurrent();
	return rself;
}

static void *BFObject_init (t_symbol *classsym, int ac, t_atom *at) {
	t_class *qlass = find_bfclass(classsym);
	if (!qlass) return 0;
	BFObject *bself = (BFObject *)pd_new(qlass);
	FMessage fm = { self: bself, winlet:-1, selector: classsym,
		ac: ac, at: at, is_init: true };
	long r = RESCUE(BFObject_init_1,&fm,BFObject_rescue,&fm);
	return r==Qnil ? 0 : (void *)bself; // return NULL if broken object
}

static void BFObject_delete_1 (FMessage *fm) {
	if (fm->self->rself) {
		rb_funcall(fm->self->rself,SI(delete),0);
	} else {
		post("BFObject_delete is NOT handling BROKEN object at %*lx",2*sizeof(long),(long)fm);
	}
}

static void BFObject_delete (BFObject *bself) {
	FMessage fm = { self: bself, winlet:-1, selector: gensym("delete"),
		ac: 0, at: 0, is_init: false };
	RESCUE(BFObject_delete_1,&fm,BFObject_rescue,&fm);
}

/* **************************************************************** */

struct RMessage {
	VALUE rself;
	ID sel;
	int argc;
	VALUE *argv;
};

// this was called rb_funcall_rescue[...] but recently (ruby 1.8.2)
// got a conflict with a new function in ruby.

VALUE rb_funcall_myrescue_1(RMessage *rm) {
	return rb_funcall2(rm->rself,rm->sel,rm->argc,rm->argv);
}

static Ruby rb_funcall_myrescue_2 (RMessage *rm) {
	Ruby error_array = make_error_message();
//	for (int i=0; i<rb_ary_len(error_array); i++)
//		post("%s\n",rb_str_ptr(rb_ary_ptr(error_array)[i]));
	post("%s",rb_str_ptr(rb_funcall(error_array,SI(join),1,rb_str_new2("\n"))));
	return Qnil;
}

VALUE rb_funcall_myrescue(VALUE rself, ID sel, int argc, ...) {
	va_list foo;
	va_start(foo,argc);
	VALUE argv[argc];
	for (int i=0; i<argc; i++) argv[i] = va_arg(foo,VALUE);
	RMessage rm = { rself, sel, argc, argv };
	va_end(foo);
	return RESCUE(rb_funcall_myrescue_1,&rm,rb_funcall_myrescue_2,&rm);
}

#ifndef HAVE_DESIREDATA

/* Call this to get a gobj's bounding rectangle in pixels */
void bf_getrectfn(t_gobj *x, t_glist *glist,
int *x1, int *y1, int *x2, int *y2) {
	BFObject *bself = (BFObject*)x;
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	Ruby a = rb_funcall_myrescue(bself->rself,SI(pd_getrect),1,can);
	if (TYPE(a)!=T_ARRAY || rb_ary_len(a)<4) {
		post("bf_getrectfn: return value should be 4-element array");
		*x1=*y1=*x2=*y2=0;
		return;
	}
	Ruby *ap = rb_ary_ptr(a);
	*x1 = INT(ap[0]);
	*y1 = INT(ap[1]);
	*x2 = INT(ap[2]);
	*y2 = INT(ap[3]);
}

/* and this to displace a gobj: */
void bf_displacefn(t_gobj *x, t_glist *glist, int dx, int dy) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	BFObject *bself = (BFObject *)x;
	bself->te_xpix+=dx;
	bself->te_ypix+=dy;
	rb_funcall_myrescue(bself->rself,SI(pd_displace),3,can,INT2NUM(dx),INT2NUM(dy));
	canvas_fixlinesfor(glist, (t_text *)x);
}

/* change color to show selection: */
void bf_selectfn(t_gobj *x, t_glist *glist, int state) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_select),2,can,INT2NUM(state));
}

/* change appearance to show activation/deactivation: */
void bf_activatefn(t_gobj *x, t_glist *glist, int state) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_activate),2,can,INT2NUM(state));
}

/* warn a gobj it's about to be deleted */
void bf_deletefn(t_gobj *x, t_glist *glist) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_delete),1,can);
	canvas_deletelinesfor(glist, (t_text *)x);
}

/*  making visible or invisible */
void bf_visfn(t_gobj *x, t_glist *glist, int flag) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	Ruby rself = ((BFObject*)x)->rself;
	DGS(FObject);
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_vis),2,can,INT2NUM(flag));
}

/* field a mouse click (when not in "edit" mode) */
int bf_clickfn(t_gobj *x, t_glist *glist,
int xpix, int ypix, int shift, int alt, int dbl, int doit) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	Ruby ret = rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_click),7,can,
		INT2NUM(xpix),INT2NUM(ypix),
		INT2NUM(shift),INT2NUM(alt),
		INT2NUM(dbl),INT2NUM(doit));
	if (TYPE(ret) == T_FIXNUM) return INT(ret);
	post("bf_clickfn: expected Fixnum");
	return 0;
}

/* get keypresses during focus */
void bf_keyfn(void *x, t_floatarg fkey) {
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_key),1,INT2NUM((int)fkey));
}

/* get motion diff during focus */
void bf_motionfn(void *x, t_floatarg dx, t_floatarg dy) {
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_motion),2,
		INT2NUM((int)dx), INT2NUM((int)dy));
}

/* open properties dialog */
void bf_propertiesfn(t_gobj *x, struct _glist *glist) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_properties),1,can);
}

#endif /* HAVE_DESIREDATA */

/* save to a binbuf (FOR FUTURE USE) */
void bf_savefn(t_gobj *x, t_binbuf *b) {
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_save),1,Qnil);
}

/* **************************************************************** */

static void BFObject_class_init_1 (t_class *qlass) {
	class_addanything(qlass,(t_method)BFObject_method_missing0);
}

\class FObject

#define class_new(NAME,ARGS...) (print_class_list ? fprintf(stderr,"class_new %s\n",(NAME)->s_name) : 0, class_new(NAME,ARGS))

static Ruby FObject_s_install2(Ruby rself, Ruby name) {
	if (TYPE(name)!=T_STRING) RAISE("name must be String");
	//if (print_class_list) printf("class_new %s\n",rb_str_ptr(name));
	t_class *qlass = class_new(gensym(rb_str_ptr(name)),
		(t_newmethod)BFObject_init, (t_method)BFObject_delete,
		sizeof(BFObject), CLASS_DEFAULT, A_GIMME,0);
	rb_ivar_set(rself, SI(@bfclass), PTR2FIX(qlass));
	FMessage fm = {0, -1, 0, 0, 0, false};
	RESCUE(BFObject_class_init_1,qlass,BFObject_rescue,&fm);
	return Qnil;
}

static Ruby FObject_send_out2(int argc, Ruby *argv, Ruby rself) {
//\def Ruby send_out2(...) {
	DGS(FObject);
	BFObject *bself = self->bself;
	if (!bself) return Qnil;
	int outlet = INT(argv[0]);
	Ruby sym = argv[1];
	argc-=2;
	argv+=2;
	t_atom sel, at[argc];
	Bridge_export_value(sym,&sel);
	for (int i=0; i<argc; i++) Bridge_export_value(argv[i],at+i);
	outlet_anything(bself->out[outlet],atom_getsymbol(&sel),argc,at);
	return Qnil;
}

static Ruby FObject_bself(Ruby rself) {
	DGS(FObject);
	if (!self->bself) RAISE("there is no bself during #initialize. use #initialize2");
	return Pointer_s_new(self->bself);
}

static Ruby FObject_s_set_help (Ruby rself, Ruby path) {
	path = rb_funcall(path,SI(to_s),0);
	Ruby qlassid = rb_ivar_get(rself,SI(@bfclass));
	if (qlassid==Qnil) RAISE("@bfclass==nil ???");
	t_class *qlass = FIX2PTR(t_class,qlassid);
	class_sethelpsymbol(qlass,gensym(rb_str_ptr(path)));
	return Qnil;
}

static Ruby GridFlow_s_gui (int argc, Ruby *argv, Ruby rself) {
	if (argc!=1) RAISE("bad args");
	Ruby command = rb_funcall(argv[0],SI(to_s),0);
	sys_gui(rb_str_ptr(command));
	return Qnil;
}

static Ruby GridFlow_s_bind (Ruby rself, Ruby argv0, Ruby argv1) {
	if (TYPE(argv0)==T_STRING) {
#if PD_VERSION_INT < 37
	RAISE("requires Pd 0.37");
#else
		Ruby name = rb_funcall(argv0,SI(to_s),0);
		Ruby qlassid = rb_ivar_get(rb_hash_aref(rb_ivar_get(mGridFlow2,SI(@fclasses)),name),SI(@bfclass));
		if (qlassid==Qnil) RAISE("no such class: %s",rb_str_ptr(name));
		pd_typedmess(&pd_objectmaker,gensym(rb_str_ptr(name)),0,0);
		t_pd *o = pd_newest();
		pd_bind(o,gensym(rb_str_ptr(argv1)));
#endif
	} else {
		Ruby rself = argv0;
		DGS(FObject);
		t_symbol *s = gensym(rb_str_ptr(argv1));
		t_pd *o = (t_pd *)(self->bself);
		//fprintf(stderr,"binding %08x to: \"%s\" (%08x %s)\n",o,rb_str_ptr(argv[1]),s,s->s_name);
		pd_bind(o,s);
	}
	return Qnil;
}

static t_pd *rp_to_pd (Ruby pointer) {
       Pointer *foo;
       Data_Get_Struct(pointer,Pointer,foo);
       return (t_pd *)foo->p;
}

static Ruby GridFlow_s_objectmaker (int argc, Ruby *argv, Ruby rself) {
	if (argc<1) RAISE("not enough args (%d, need at least 1)",argc,0);
	Ruby sym = argv[0];
	argc--;
	argv++;
	t_atom sel, at[argc];
	Bridge_export_value(sym,&sel);
	for (int i=0; i<argc; i++) Bridge_export_value(argv[i],at+i);
	pd_typedmess(&pd_objectmaker,atom_getsymbol(&sel),argc,at);
	return Pointer_s_new((void *)pd_newest());
}

static Ruby GridFlow_s_send_in (int argc, Ruby *argv, Ruby rself) {
	if (argc<3) RAISE("not enough args (%d, need at least 3)",argc,0);
	Ruby ptr = argv[0];
	Ruby inn = argv[1];
	Ruby sym = argv[2];
	if (CLASS_OF(ptr)!=cPointer) RAISE("$1 is not a Pointer");
	int ini = INT(inn);
	if (ini<0) RAISE("negative inlet number?");
	t_pd *o = (t_pd *)rp_to_pd(ptr);
	if (ini>0) {
		RAISE("inlet numbers greater than 0 are not supported yet (ask matju)");
		/*inn--;
		t_inlet *inlet = ((t_object *)o)->ob_inlet;
		while (1) {
			if (!inlet) RAISE("nonexistent inlet");
			if (!inn) break;
			inlet = inlet->i_next;
			inn--;
		}
		o = inlet;*/
	}
	argc-=3;
	argv+=3;
	t_atom sel, at[argc];
	Bridge_export_value(sym,&sel);
	for (int i=0; i<argc; i++) Bridge_export_value(argv[i],at+i);
	pd_typedmess(o,atom_getsymbol(&sel),argc,at);
	return Qnil;
}

#ifndef HAVE_DESIREDATA
static Ruby FObject_s_gui_enable (Ruby rself) {
	Ruby qlassid = rb_ivar_get(rself,SI(@bfclass));
	if (qlassid==Qnil) RAISE("no class id ?");
	t_widgetbehavior *wb = new t_widgetbehavior;
	wb->w_getrectfn    = bf_getrectfn;
	wb->w_displacefn   = bf_displacefn;
	wb->w_selectfn     = bf_selectfn;
	wb->w_activatefn   = bf_activatefn;
	wb->w_deletefn     = bf_deletefn;
	wb->w_visfn        = bf_visfn;
	wb->w_clickfn      = bf_clickfn;
	//wb->w_savefn       = bf_savefn;
	//wb->w_propertiesfn = bf_propertiesfn;
	class_setwidget(FIX2PTR(t_class,qlassid),wb);
	return Qnil;
}

static Ruby FObject_s_properties_enable (Ruby rself) {
	Ruby qlassid = rb_ivar_get(rself,SI(@bfclass));
	if (qlassid==Qnil) RAISE("no class id ?");
	class_setpropertiesfn(FIX2PTR(t_class,qlassid), bf_propertiesfn);
	return Qnil;
}

#endif /* HAVE_DESIREDATA */

static Ruby FObject_s_save_enable (Ruby rself) {
	Ruby qlassid = rb_ivar_get(rself,SI(@bfclass));
	if (qlassid==Qnil) RAISE("no class id ?");
	class_setsavefn(FIX2PTR(t_class,qlassid), bf_savefn);
	return Qnil;
}

#ifndef HAVE_DESIREDATA
static Ruby FObject_focus (Ruby rself, Ruby canvas_, Ruby x_, Ruby y_) {
	DGS(FObject);
	t_glist *canvas = FIX2PTR(t_glist,canvas_);
	t_gobj  *bself = (t_gobj *)(self->bself);
	int x = INT(x_);
	int y = INT(y_);
	glist_grab(canvas,bself, bf_motionfn, bf_keyfn, x,y);
	return Qnil;
}

// doesn't use rself but still is aside FObject_focus for symmetry reasons.
static Ruby FObject_unfocus (Ruby rself, Ruby canvas_) {
	DGS(FObject);
	t_glist *canvas = FIX2PTR(t_glist,canvas_);
	glist_grab(canvas,0,0,0,0,0);
	return Qnil;
}
#endif /* HAVE_DESIREDATA */

// from pd/src/g_canvas.c
struct _canvasenvironment {
    t_symbol *ce_dir;   /* directory patch lives in */
    int ce_argc;        /* number of "$" arguments */
    t_atom *ce_argv;    /* array of "$" arguments */
    int ce_dollarzero;  /* value of "$0" */
};

static Ruby FObject_args (Ruby rself) {
	DGS(FObject);
	if (!self->bself) RAISE("can't use this in initialize");
	char *buf;
	int length;
	t_binbuf *b = self->bself->te_binbuf;
	if (!b) return Qnil;
	binbuf_gettext(b, &buf, &length);
	Ruby r = rb_str_new(buf,length);
	free(buf);
	return r;
}

static Ruby FObject_patcherargs (Ruby rself) {
	DGS(FObject);
	if (!self->bself) RAISE("can't use this in initialize");
	t_glist *canvas = self->bself->mom;
	_canvasenvironment *env = canvas_getenv(canvas);
	Ruby ar = rb_ary_new();
	for (int i=0; i<env->ce_argc; i++)
		rb_ary_push(ar, Bridge_import_value(env->ce_argv+i));
	return ar;
}

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
	//fprintf(stderr,"ninlets_set... nin=%d n=%d\n",nin,n);
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

static Ruby FObject_ninlets_set (Ruby rself, Ruby n_) {
	DGS(FObject); BFObject *bself = self->bself; int n = INT(n_); if (n<1) n=1;
	bself->ninlets_set(n);
	return Qnil;
};
static Ruby FObject_noutlets_set (Ruby rself, Ruby n_) {
	DGS(FObject); BFObject *bself = self->bself; int n = INT(n_); if (n<1) n=1;
	bself->noutlets_set(n);
	return Qnil;
};

static Ruby FObject_ninlets  (Ruby rself) {
	DGS(FObject); BFObject *bself = self->bself;
	if (!bself) return rb_ivar_get(rb_obj_class(rself),SI(@ninlets));
	return R(bself->nin ).r;
}
static Ruby FObject_noutlets (Ruby rself) {
	DGS(FObject); BFObject *bself = self->bself;
	if (!bself) return rb_ivar_get(rb_obj_class(rself),SI(@noutlets));
	return R(bself->nout).r;
}

static Ruby GridFlow_s_add_creator_2 (Ruby rself, Ruby name_) {
	t_symbol *name = gensym(rb_str_ptr(rb_funcall(name_,SI(to_s),0)));
	class_addcreator((t_newmethod)BFObject_init,name,A_GIMME,0);
	return Qnil;
}

#ifndef HAVE_DESIREDATA
static Ruby FObject_get_position (Ruby rself, Ruby canvas) {
	DGS(FObject);
	t_text *bself = (t_text *)(self->bself);
	t_glist *c = FIX2PTR(t_glist,canvas);
	float x0,y0;
	if (c->gl_havewindow || !c->gl_isgraph) {
		x0=bself->te_xpix;
		y0=bself->te_ypix;
	} else { // graph-on-parent: have to zoom
		float zx = float(c->gl_x2 - c->gl_x1) / (c->gl_screenx2 - c->gl_screenx1);
		float zy = float(c->gl_y2 - c->gl_y1) / (c->gl_screeny2 - c->gl_screeny1);
		x0 = glist_xtopixels(c, c->gl_x1 + bself->te_xpix*zx);
		y0 = glist_ytopixels(c, c->gl_y1 + bself->te_ypix*zy);
	}
	Ruby a = rb_ary_new();
	rb_ary_push(a,INT2NUM((int)x0));
	rb_ary_push(a,INT2NUM((int)y0));
	return a;
}
#endif

\classinfo {}
\end class FObject

//****************************************************************

\class Clock < CObject {
	t_clock *serf;
	Ruby owner; /* copy of ptr that serf already has, for marking */
	\decl void set  (double   systime);
	\decl void delay(double delaytime);
	\decl void unset();
};

void Clock_fn (Ruby rself) { rb_funcall_myrescue(rself,SI(call),0); }
void Clock_mark (Clock *self) { rb_gc_mark(self->owner); }
void Clock_free (Clock *self) { clock_free(self->serf); CObject_free(self); }

Ruby Clock_s_new (Ruby qlass, Ruby owner) {
	Clock *self = new Clock();
	self->rself = Data_Wrap_Struct(qlass, Clock_mark, Clock_free, self);
	self->serf = clock_new((void*)owner,(t_method)Clock_fn);
	self->owner = owner;
	return self->rself;
}

\def void set  (double   systime) {   clock_set(serf,  systime); }
\def void delay(double delaytime) { clock_delay(serf,delaytime); }
\def void unset() { clock_unset(serf); }

\classinfo {}
\end class Clock

//****************************************************************

\class Pointer < CObject
\def Ruby ptr () { return LONG2NUM(((long)p)); }
\classinfo {
	IEVAL(rself,
"self.module_eval{"
"def inspect; p=('%08x'%ptr).gsub(/^\\.\\.f/,''); \"#<Pointer:#{p}>\" % ptr; end;"
"alias to_s inspect }"
);}
\end class Pointer

//****************************************************************

Ruby GridFlow_s_post_string (Ruby rself, Ruby string) {
	if (TYPE(string)!=T_STRING) RAISE("not a string!");
	post("%s",rb_str_ptr(string));
	return Qnil;
}

#define SDEF(_name1_,_name2_,_argc_) \
	rb_define_singleton_method(mGridFlow2,_name1_,(RMethod)GridFlow_s_##_name2_,_argc_)

Ruby gf_bridge_init (Ruby rself) {
	Ruby ver = EVAL("GridFlow::GF_VERSION");
	if (strcmp(rb_str_ptr(ver), GF_VERSION) != 0) {
		RAISE("GridFlow version mismatch: "
			"main library is '%s'; bridge is '%s'",
			rb_str_ptr(ver), GF_VERSION);
	}
	syms = FIX2PTR(BuiltinSymbols,rb_ivar_get(mGridFlow2,SI(@bsym)));
	Ruby fo = EVAL("GridFlow::FObject");
	rb_define_singleton_method(fo,"install2",(RMethod)FObject_s_install2,1);

#ifndef HAVE_DESIREDATA
	rb_define_singleton_method(fo,"gui_enable",        (RMethod)FObject_s_gui_enable, 0);
	rb_define_singleton_method(fo,"properties_enable", (RMethod)FObject_s_properties_enable, 0);
#endif

	rb_define_singleton_method(fo,"set_help", (RMethod)FObject_s_set_help, 1);
	rb_define_singleton_method(fo,"save_enable",       (RMethod)FObject_s_save_enable, 0);

#ifndef HAVE_DESIREDATA
	rb_define_method(fo,"get_position",(RMethod)FObject_get_position,1);
	rb_define_method(fo,"unfocus",     (RMethod)FObject_unfocus, 1);
	rb_define_method(fo,  "focus",     (RMethod)FObject_focus,   3);
#endif
	rb_define_method(fo,"send_out2",   (RMethod)FObject_send_out2,-1);
	rb_define_method(fo,"ninlets",     (RMethod)FObject_ninlets,  0);
	rb_define_method(fo,"noutlets",    (RMethod)FObject_noutlets, 0);
	rb_define_method(fo,"ninlets=",    (RMethod)FObject_ninlets_set,  1);
	rb_define_method(fo,"noutlets=",   (RMethod)FObject_noutlets_set, 1);
	rb_define_method(fo,"patcherargs", (RMethod)FObject_patcherargs,0);
	rb_define_method(fo,"args",        (RMethod)FObject_args,0);
	rb_define_method(fo,"bself",       (RMethod)FObject_bself,0);

	SDEF("post_string",post_string,1);
	SDEF("add_creator_2",add_creator_2,1);
	SDEF("gui",gui,-1);
	SDEF("bind",bind,2);
	SDEF("objectmaker",objectmaker,-1);
	SDEF("send_in",send_in,-1);

	\startall
	rb_define_singleton_method(EVAL("GridFlow::Clock"  ),"new", (RMethod)Clock_s_new, 1);
	rb_define_singleton_method(EVAL("GridFlow::Pointer"),"new", (RMethod)Pointer_s_new, 1);
	cPointer = EVAL("GridFlow::Pointer");
	EVAL("class<<GridFlow;attr_accessor :config; end");
	return Qnil;
}

struct BFGridFlow : t_object {};

t_class *bindpatcher;
static void *bindpatcher_init (t_symbol *classsym, int ac, t_atom *at) {
	t_pd *bself = pd_new(bindpatcher);
	if (ac!=1 || at->a_type != A_SYMBOL) {
		post("bindpatcher: oops");
	} else {
		t_symbol *s = atom_getsymbol(at);
		post("binding patcher to: %s",s->s_name);
		pd_bind((t_pd *)canvas_getcurrent(),s);
	}
	return bself;
}

static t_clock *hack;

extern "C" void Init_stack(void*);

/* revival of the stack end crutch of 2003-2005... just in case */
static void ruby_stack_end_hack () {
	int bogus;
	Init_stack(&bogus);
//	post("hello from ruby_stack_end_hack");
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

static void boo (int boo) {
}

// note: contrary to what m_pd.h says, pd_getfilename() and pd_getdirname()
// don't exist; also, canvas_getcurrentdir() isn't available during setup
// (segfaults), in addition to libraries not being canvases ;-)
// AND ALSO, CONTRARY TO WHAT m_pd.h SAYS, open_via_path()'s args are reversed!!!
extern "C" void gridflow_setup () {
	char *foo[] = {"Ruby-for-PureData","-e",";"};
	post("setting up Ruby-for-PureData...");
	const char *pcl = getenv("PRINT_CLASS_LIST");
        if (pcl && strcmp(pcl,"yes")==0) print_class_list=true;
	char *dirname   = new char[MAXPDSTRING];
	char *dirresult = new char[MAXPDSTRING];
	char *nameresult;
	if (getcwd(dirname,MAXPDSTRING)<0) {post("AAAARRRRGGGGHHHH!"); exit(69);}
	int       fd=open_via_path(dirname,"gridflow/gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd<0) fd=open_via_path(dirname,         "gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd>=0) {
		//post("%s found itself in %s","gridflow"PDSUF,dirresult);
		close(fd);
	} else {
		post("%s was not found via the -path!","gridflow"PDSUF);
	}
	/* nameresult is only a pointer in dirresult space so don't delete[] it. */
	add_to_path(dirresult);
	ruby_init();
	ruby_options(COUNT(foo),foo);
	//post("we are using Ruby version %s",rb_str_ptr(EVAL("RUBY_VERSION")));
	Ruby cData = rb_const_get(rb_cObject,SI(Data));
	BFProxy_class = class_new(gensym("ruby_proxy"), NULL,NULL,sizeof(BFProxy),CLASS_PD|CLASS_NOINLET, A_NULL);
	class_addanything(BFProxy_class,BFProxy_method_missing);
	rb_define_singleton_method(cData,"gf_bridge_init",(RMethod)gf_bridge_init,0);
	mGridFlow2 = EVAL("module GridFlow; class<<self; end; Pd=GridFlow; self end");
	rb_const_set(mGridFlow2,SI(DIR),rb_str_new2(dirresult));
	post("DIR = %s",rb_str_ptr(EVAL("GridFlow::DIR.inspect")));
	EVAL("$:.unshift GridFlow::DIR+'/..', GridFlow::DIR, GridFlow::DIR+'/optional/rblti'");
	if (!EVAL("begin require 'gridflow'; true; rescue Exception => e; "
		"GridFlow.post \"[#{e.class}] [#{e.message}]:\n#{e.backtrace.join'\n'}\"; false; end"))
	{
		post("ERROR: Cannot load GridFlow-for-Ruby (gridflow.so)\n");
		return;
	}
	bindpatcher = class_new(gensym("bindpatcher"), (t_newmethod)bindpatcher_init, 0, sizeof(t_object),CLASS_DEFAULT,A_GIMME,0);
	delete[] dirresult;
	delete[] dirname;
	hack = clock_new((void*)0,(t_method)ruby_stack_end_hack);
	clock_delay(hack,0);
//	signal(SIGSEGV,boo);
	signal(SIGSEGV,SIG_DFL);
	signal(SIGABRT,SIG_DFL);
	signal(SIGBUS, SIG_DFL);
}
