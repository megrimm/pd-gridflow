/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003,2004 by Mathieu Bouchard

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
which tells the bridge to register a class in puredata or jmax. Puredata
objects have proxies for each of the non-first inlets so that any inlet
may receive any distinguished message. All inlets are typed as 'anything',
and the 'anything' method translates the whole message to Ruby objects and
tries to call a Ruby method of the proper name.
*/

#define IS_BRIDGE
#include "../base/grid.h.fcs"
/* resolving conflict: T_OBJECT will be PD's, not Ruby's */
#undef T_OBJECT
#undef EXTERN
#include <m_pd.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include "g_canvas.h"

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

/* code that is common across all bridges. */
#include "common.c"

/* reentrancy check */
static bool is_in_ruby = false;

/* **************************************************************** */
/* BFObject */

struct BFObject : t_object {
	int32 magic; /* paranoia */
	Ruby rself;
	int nin;  /* per object settings (not class) */
	int nout; /* per object settings (not class) */
	t_outlet **out;

	void check_magic () {
		if (magic != OBJECT_MAGIC) {
			fprintf(stderr,"Object memory corruption! (ask the debugger)\n");
			for (int i=-1; i<=1; i++) {
				fprintf(stderr,"this[0]=0x%08x\n",((int*)this)[i]);
			}
			raise(11);
		}
	}
};

static t_class *find_bfclass (t_symbol *sym) {
	t_atom a[1];
	SETSYMBOL(a,sym);
	char buf[4096];
//	post("sym=0x%08x",(long)sym);
	//for (int i=0; i<16; i++) post("sym[%i]=0x%02x",i,((char*)sym)[i]);
	if (sym==&s_list) strcpy(buf,"list"); else atom_string(a,buf,sizeof(buf));
	Ruby v = rb_hash_aref(
		rb_ivar_get(mGridFlow2, SI(@bfclasses_set)),
		rb_str_new2(buf));
//	if (v==Qnil) RAISE("class not found: '%s'",buf);
	if (v==Qnil) {
		post("class not found: '%s'",buf);
		return 0;
	}
	return FIX2PTR(t_class,v);
}

static t_class *BFProxy_class;

struct BFProxy : t_object {
	BFObject *parent;
	int inlet;
};

static void Bridge_export_value(Ruby arg, t_atom *at) {
	if (INTEGER_P(arg)) {
		SETFLOAT(at,NUM2INT(arg));
	} else if (SYMBOL_P(arg)) {
		const char *name = rb_sym_name(arg);
		SETSYMBOL(at,gensym((char *)name));
	} else if (FLOAT_P(arg)) {
		SETFLOAT(at,RFLOAT(arg)->value);
//	} else if (rb_obj_class(arg)==Pointer_class2) {
//		SETPOINTER(at,Pointer_get(arg));
	} else {
		RAISE("cannot convert argument of class '%s'",
			rb_str_ptr(rb_funcall(rb_funcall(arg,SI(class),0),SI(inspect),0)));
	}
}

static Ruby Bridge_import_value(const t_atom *at) {
	t_atomtype t = at->a_type;
	if (t==A_SYMBOL) {
		return ID2SYM(rb_intern(at->a_w.w_symbol->s_name));
	} else if (t==A_FLOAT) {
		return rb_float_new(at->a_w.w_float);
//	} else if (t==A_POINTER) {
//		return Pointer_new(at->a_w.w_gpointer);
		} else {
		return Qnil; /* unknown */
	}
}

static Ruby BFObject_method_missing_1 (FMessage *fm) {
	Ruby argv[fm->ac+2];
	argv[0] = INT2NUM(fm->winlet);
	argv[1] = ID2SYM(rb_intern(fm->selector->s_name));
	for (int i=0; i<fm->ac; i++) argv[2+i] = Bridge_import_value(fm->at+i);
	fm->self->check_magic();
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
	FMessage fm = { self: bself, winlet: winlet, selector: selector, ac: ac, at: at,
		is_init: false };
	if (!bself->rself) {
		pd_error(bself,
		"message to a dead object. (supposed to be impossible)");
		return;
	}
	rb_rescue2(
		(RMethod)BFObject_method_missing_1,(Ruby)&fm,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
}

static void BFObject_method_missing0 (BFObject *self,
t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self,0,s,argc,argv);
}

static void BFProxy_method_missing(BFProxy *self,
t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self->parent,self->inlet,s,argc,argv);
}
	
static Ruby BFObject_init_1 (FMessage *fm) {
	Ruby argv[fm->ac+1];
	for (int i=0; i<fm->ac; i++) argv[i+1] = Bridge_import_value(fm->at+i);

	if (fm->selector==&s_list) {
		argv[0] = rb_str_new2("list"); // pd is slightly broken here
	} else {
		argv[0] = rb_str_new2(fm->selector->s_name);
	}

	Ruby rself = rb_funcall2(rb_const_get(mGridFlow2,SI(FObject)),SI([]),fm->ac+1,argv);
	DGS(FObject);
	self->bself = fm->self;
	self->bself->rself = rself;

	int ninlets  = self->bself->nin = ninlets_of(rb_funcall(rself,SI(class),0));
	int noutlets = self->bself->nout = noutlets_of(rb_funcall(rself,SI(class),0));

	for (int i=1; i<ninlets; i++) {
		BFProxy *p = (BFProxy *)pd_new(BFProxy_class);
		p->parent = self->bself;
		p->inlet = i;
		inlet_new(self->bself, &p->ob_pd, 0,0);
	}
	self->bself->out = new t_outlet*[noutlets];
	for (int i=0; i<noutlets; i++) {
		self->bself->out[i] = outlet_new(self->bself,&s_anything);
	}
	rb_funcall(rself,SI(initialize2),0);
	return rself;
}

static void *BFObject_init (t_symbol *classsym, int ac, t_atom *at) {
	t_class *qlass = find_bfclass(classsym);
	if (!qlass) return 0;
	BFObject *bself = (BFObject *)pd_new(qlass);
	bself->magic = OBJECT_MAGIC;
	bself->check_magic();
	FMessage fm = { self: bself, winlet:-1, selector: classsym,
		ac: ac, at: at, is_init: true };
	long r = rb_rescue2(
		(RMethod)BFObject_init_1,(Ruby)&fm,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
	/* return NULL if broken object */
	return r==Qnil ? 0 : (void *)bself;
}

static void BFObject_delete_1 (FMessage *fm) {
	fm->self->check_magic();
	if (fm->self->rself) {
		rb_funcall(fm->self->rself,SI(delete),0);
	} else {
		post("BFObject_delete is NOT handling BROKEN object at %08x",(int)fm);
	}
}

static void BFObject_delete (BFObject *bself) {
	bself->check_magic();
	FMessage fm = { self: bself, winlet:-1, selector: gensym("delete"),
		ac: 0, at: 0, is_init: false };
	rb_rescue2(
		(RMethod)BFObject_delete_1,(Ruby)&fm,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
	bself->magic = 0xDeadBeef;
}

/* **************************************************************** */

struct RMessage {
	VALUE rself;
	ID sel;
	int argc;
	VALUE *argv;
};

VALUE rb_funcall_rescue_1(RMessage *rm) {
	return rb_funcall2(rm->rself,rm->sel,rm->argc,rm->argv);
}

static Ruby rb_funcall_rescue_2 (RMessage *rm) {
	Ruby error_array = make_error_message();
//	for (int i=0; i<rb_ary_len(error_array); i++)
//		post("%s\n",rb_str_ptr(rb_ary_ptr(error_array)[i]));
	post("%s",rb_str_ptr(rb_funcall(error_array,SI(join),1,rb_str_new2("\n"))));
	return Qnil;
}

VALUE rb_funcall_rescue(VALUE rself, ID sel, int argc, ...) {
	va_list foo;
	va_start(foo,argc);
	VALUE argv[argc];
	for (int i=0; i<argc; i++) argv[i] = va_arg(foo,VALUE);
	RMessage rm = { rself, sel, argc, argv };
	va_end(foo);
	return rb_rescue2(
		(RMethod)rb_funcall_rescue_1,(Ruby)&rm,
		(RMethod)rb_funcall_rescue_2,(Ruby)&rm,
		rb_eException,0);
}

/* Call this to get a gobj's bounding rectangle in pixels */
void bf_getrectfn(t_gobj *x, struct _glist *glist,
int *x1, int *y1, int *x2, int *y2) {
	BFObject *bself = (BFObject*)x;
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	t_atom at[5];
	float x0,y0;
	if (glist->gl_havewindow || !glist->gl_isgraph) {
		x0=bself->te_xpix;
		y0=bself->te_ypix;
	}
	else {
		x0 = glist_xtopixels(glist, glist->gl_x1 + (glist->gl_x2 - glist->gl_x1) * 
            	bself->te_xpix / (glist->gl_screenx2 - glist->gl_screenx1));
		y0 = glist_ytopixels(glist, glist->gl_y1 + (glist->gl_y2 - glist->gl_y1) * 
            	bself->te_ypix / (glist->gl_screeny2 - glist->gl_screeny1));
	}
	fprintf(stderr,"real x=%f; y=%f;\n",x0,y0);

	Ruby a = rb_funcall_rescue(bself->rself,SI(pd_getrect),1,can);
	if (TYPE(a)!=T_ARRAY || rb_ary_len(a)<4) {
		post("bf_getrectfn: return value should be 4-element array");
		*x1=*y1=*x2=*y2=0;
		return;
	}
	*x1 = rb_ary_ptr(a)[0];
	*y1 = rb_ary_ptr(a)[1];
	*x2 = rb_ary_ptr(a)[2];
	*y2 = rb_ary_ptr(a)[3];
}

/* and this to displace a gobj: */
void bf_displacefn(t_gobj *x, struct _glist *glist, int dx, int dy) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_rescue(((BFObject*)x)->rself,SI(pd_displace),3,can,INT2NUM(dx),INT2NUM(dy));
}

/* change color to show selection: */
void bf_selectfn(t_gobj *x, struct _glist *glist, int state) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_rescue(((BFObject*)x)->rself,SI(pd_select),2,can,INT2NUM(state));
}

/* change appearance to show activation/deactivation: */
void bf_activatefn(t_gobj *x, struct _glist *glist, int state) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_rescue(((BFObject*)x)->rself,SI(pd_activate),2,can,INT2NUM(state));
}

/* warn a gobj it's about to be deleted */
void bf_deletefn(t_gobj *x, struct _glist *glist) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_rescue(((BFObject*)x)->rself,SI(pd_delete),1,can);
}

/*  making visible or invisible */
void bf_visfn(t_gobj *x, struct _glist *glist, int flag) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	Ruby rself = ((BFObject*)x)->rself;
	DGS(FObject);
	self->check_magic();
	rb_funcall_rescue(((BFObject*)x)->rself,SI(pd_vis),2,can,INT2NUM(flag));
}

/* field a mouse click (when not in "edit" mode) */
int bf_clickfn(t_gobj *x, struct _glist *glist,
int xpix, int ypix, int shift, int alt, int dbl, int doit) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	Ruby ret = rb_funcall_rescue(((BFObject*)x)->rself,SI(pd_click),7,can,
		INT2NUM(xpix),INT2NUM(ypix),
		INT2NUM(shift),INT2NUM(alt),
		INT2NUM(dbl),INT2NUM(doit));
	if (TYPE(ret) == T_FIXNUM) return INT(ret);
	post("bf_clickfn: expected Fixnum");
	return 0;
}
/*  save to a binbuf */

void bf_savefn(t_gobj *x, t_binbuf *b) {
	rb_funcall_rescue(((BFObject*)x)->rself,SI(pd_save),1,Qnil);
}
/*  open properties dialog */

void bf_propertiesfn(t_gobj *x, struct _glist *glist) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_rescue(((BFObject*)x)->rself,SI(pd_properties),1,can);
}
/* ... and later, resizing; getting/setting font or color... */

/* **************************************************************** */

static void BFObject_class_init_1 (t_class *qlass) {
	class_addanything(qlass,(t_method)BFObject_method_missing0);
}

static Ruby FObject_s_install_2(Ruby rself, char *name) {
//	post("install: %s",name);
	t_class *qlass = class_new(gensym(name),
		(t_newmethod)BFObject_init, (t_method)BFObject_delete,
		sizeof(BFObject),
		CLASS_DEFAULT,
		A_GIMME,0);

	rb_hash_aset(rb_ivar_get(mGridFlow2,SI(@bfclasses_set)),
		rb_str_new2(name), PTR2FIX(qlass));

	FMessage fm = {0, -1, 0, 0, 0, false};
	rb_rescue2(
		(RMethod)BFObject_class_init_1,(Ruby)qlass,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
	return Qnil;
}

static Ruby FObject_send_out_2(int argc, Ruby *argv, Ruby sym, int outlet,
Ruby rself) {
	DGS(FObject);
	BFObject *bself = self->bself;
	bself->check_magic();
	Ruby qlass = rb_funcall(rself,SI(class),0);
	t_atom sel, at[argc];
	Bridge_export_value(sym,&sel);
	for (int i=0; i<argc; i++) Bridge_export_value(argv[i],at+i);
	t_outlet *out = bself->te_outlet;
	outlet_anything(bself->out[outlet],atom_getsymbol(&sel),argc,at);
	return Qnil;
}

/* additional bridge-specific functionality */
Ruby bridge_whatever (int argc, Ruby *argv, Ruby rself) {
	if (argc==0) RAISE("BLEH");
	if (argv[0] == SYM(help)) {
		if (argc!=3) RAISE("bad args");
		Ruby name = rb_funcall(argv[1],SI(to_s),0);
		Ruby path = rb_funcall(argv[2],SI(to_s),0);
		Ruby qlassid =
			rb_hash_aref(rb_ivar_get(mGridFlow2,SI(@bfclasses_set)),name);
		if (qlassid==Qnil) RAISE("no such class: %s",
			rb_str_ptr(name));
		t_class *qlass = FIX2PTR(t_class,qlassid);
		class_sethelpsymbol(qlass,gensym(rb_str_ptr(path)));
		return Qnil;
	} else if (argv[0] == SYM(gui)) {
		if (argc!=2) RAISE("bad args");
		Ruby command = rb_funcall(argv[1],SI(to_s),0);
		sys_gui(rb_str_ptr(command));
	} else if (argv[0] == SYM(bind)) {
		if (argc!=3) RAISE("bad args");
		if (TYPE(argv[1])==T_STRING) {
			Ruby name = rb_funcall(argv[1],SI(to_s),0);
			Ruby qlassid =
				rb_hash_aref(rb_ivar_get(mGridFlow2,SI(@bfclasses_set)),name);
			if (qlassid==Qnil) RAISE("no such class: %s",rb_str_ptr(name));
			pd_typedmess(&pd_objectmaker,gensym(rb_str_ptr(name)),0,0);
			t_pd *o = pd_newest();
			pd_bind(o,gensym(rb_str_ptr(argv[2])));
		} else {
			Ruby rself = argv[1];
			DGS(FObject);
			pd_bind(&self->bself->te_g.g_pd,gensym(rb_str_ptr(argv[2])));
		}
	} else if (argv[0] == SYM(setwidget)) {
		if (argc!=2) RAISE("bad args");
		Ruby name = rb_funcall(argv[1],SI(to_s),0);
		Ruby qlassid =
			rb_hash_aref(rb_ivar_get(mGridFlow2,SI(@bfclasses_set)),name);
		if (qlassid==Qnil) RAISE("no such class: %s",rb_str_ptr(name));
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
	} else if (argv[0] == SYM(addinlets)) {
		if (argc!=3) RAISE("bad args");
		FObject *self;
		Data_Get_Struct(argv[1],FObject,self);
		self->check_magic();
		if (!self->bself) RAISE("there is no bself");
		int n = INT(argv[2]);
		for (int i=self->bself->nin; i<self->bself->nin+n; i++) {
			BFProxy *p = (BFProxy *)pd_new(BFProxy_class);
			p->parent = self->bself;
			p->inlet = i;
			inlet_new(self->bself, &p->ob_pd, 0,0);
		}
		self->bself->nin+=n;
	} else if (argv[0] == SYM(addoutlets)) {
		if (argc!=3) RAISE("bad args");
		FObject *self;
		Data_Get_Struct(argv[1],FObject,self);
		self->check_magic();
		if (!self->bself) RAISE("there is no bself");
		int n = INT(argv[2]);
		t_outlet **oldouts = self->bself->out;
		self->bself->out = new t_outlet*[self->bself->nout+n];
		memcpy(self->bself->out,oldouts,self->bself->nout*sizeof(t_outlet*));
		for (int i=self->bself->nout; i<self->bself->nout+n; i++) {
			self->bself->out[i] = outlet_new(self->bself,&s_anything);
		}
		self->bself->nout+=n;
	} else if (argv[0] == SYM(addtomenu)) {
		if (argc!=2) RAISE("bad args");
		Ruby name = rb_funcall(argv[1],SI(to_s),0);
		Ruby qlassid =
			rb_hash_aref(rb_ivar_get(mGridFlow2,SI(@bfclasses_set)),name);
		if (qlassid==Qnil) RAISE("no such class: %s",rb_str_ptr(name));
		//!@#$
	} else {
		RAISE("don't know how to do that");
	}
	return Qundef; /* what? */
}

static t_clock *gf_alarm;

static void gf_timer_handler_1 (VALUE blah) {
	rb_funcall(mGridFlow2,SI(tick),0);
}

void gf_timer_handler (t_clock *alarm, void *obj) {
	if (is_in_ruby) {
		post("warning: ruby is not signal-reentrant (is this a signal?)\n");
		return;
	}
	is_in_ruby = true;
//	rb_funcall(mGridFlow2,SI(tick),0);
	FMessage fm; fm.self = 0;
	rb_rescue2(
		(RMethod)gf_timer_handler_1,(Ruby)&fm,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
	clock_delay(gf_alarm,gf_bridge2->clock_tick);
	is_in_ruby = false;
	count_tick();
}       

Ruby gf_bridge_init (Ruby rself) {
	gf_same_version();
	mGridFlow2 = rb_const_get(rb_cObject,SI(GridFlow));
	rb_ivar_set(mGridFlow2, SI(@bfclasses_set), rb_hash_new());
	syms = FIX2PTR(BuiltinSymbols,rb_ivar_get(mGridFlow2,SI(@bsym)));
	return Qnil;
}

static GFBridge gf_bridge3 = {
	name: "puredata",
	send_out: FObject_send_out_2,
	class_install: FObject_s_install_2,
	post: (void (*)(const char *, ...))post,
	post_does_ln: true,
	clock_tick: 10.0,
	whatever: bridge_whatever
};

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

extern "C" void gridflow_setup () {
	gf_bridge2 = &gf_bridge3;
	char *foo[] = {"Ruby-for-PureData","/dev/null"};
	post("setting up Ruby-for-PureData...");

	ruby_init();
	Init_stack(localize_sysstack());
	ruby_options(COUNT(foo),foo);

post("we are using Ruby version %s",rb_str_ptr(EVAL("RUBY_VERSION")));
	bridge_common_init();
	Ruby cData = rb_const_get(rb_cObject,SI(Data));
	rb_ivar_set(cData,SI(@gf_bridge),PTR2FIX(gf_bridge2));

	BFProxy_class = class_new(gensym("ruby_proxy"),
		NULL,NULL,sizeof(BFProxy),CLASS_PD|CLASS_NOINLET, A_NULL);

	class_addanything(BFProxy_class,BFProxy_method_missing);

	rb_define_singleton_method(cData,"gf_bridge_init",
		(RMethod)gf_bridge_init,0);

	post("(done)");

	if (!
	EVAL("begin require 'gridflow'; true; rescue Exception => e;\
		STDERR.puts \"#{e.class}: #{e}: #{e.backtrace}\"; false; end"))
	{
		post("ERROR: Cannot load GridFlow-for-Ruby (gridflow.so)\n");
		return;
	}

	gf_alarm = clock_new(0,(void(*)())gf_timer_handler);
	gf_timer_handler(gf_alarm,0);

	bindpatcher = class_new(gensym("bindpatcher"),
		(t_newmethod)bindpatcher_init, 0, sizeof(t_object),CLASS_DEFAULT,A_GIMME,0);
}
