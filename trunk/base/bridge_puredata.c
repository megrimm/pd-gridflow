/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

/* matju to rfigura:
'bridge_puredata.c' becomes 'gridflow.pd_linux' which loads Ruby and tells
Ruby to load the other 90% of Gridflow. GridFlow creates the FObject class
and then subclasses it many times, each time calling FObject.install(),
which tells the bridge to register a class in puredata or jmax. Puredata
objects have proxies for each of the non-first inlets so that any inlet
may receive any distinguished message. All inlets are typed as 'anything',
and the 'anything' method translates the whole message to Ruby objects and
tries to call a Ruby method of the proper name.
*/

#include "grid.h"
/* resolving conflict: T_OBJECT will be PD's, not Ruby's */
#undef T_OBJECT
#undef EXTERN
#include <m_pd.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>

static const char *list_begin = "(";
static const char *list_end = ")";

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
#include "bridge.c"

static ID sym_lparen=0, sym_rparen=0;

static bool is_in_ruby = false;

/* **************************************************************** */
/* BFObject */

#define MAX_OUTLETS 4
struct BFObject : t_object {
	Ruby rself;
	t_outlet *out[MAX_OUTLETS];
};

static t_class *find_bfclass (t_symbol *sym) {
	Ruby v = rb_hash_aref(
		rb_ivar_get(mGridFlow2, SI(@bfclasses_set)),
		rb_str_new2(sym->s_name));
	if (v==Qnil) RAISE("class not found");
	return FIX2PTR(t_class,v);
}

static Ruby find_fclass (t_symbol *sym) {
	Ruby v = rb_hash_aref(
		rb_ivar_get(mGridFlow2, SI(@fclasses_set)),
		rb_str_new2(sym->s_name));
	if (v==Qnil) RAISE("class not found");
	return v;
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

/*
  This code handles nested lists because PureData doesn't do it
  and jMax only does it when it feels like it.
*/
int Bridge_import_list(int &ac, const t_atom *&at, Ruby *argv, int level=0) {
	int argc=0;
	while (ac) {
		t_atomtype t = at->a_type;
		if (t==A_SYMBOL) {
			const char *s=at->a_w.w_symbol->s_name;
			if (strcmp(s,list_begin)==0) {
				ac--; at++;
				Ruby ary = rb_ary_new();
				Ruby argv2[ac];
				int argc2 = Bridge_import_list(ac,at,argv2,level+1);
				for (int i=0; i<argc2; i++) rb_ary_push(ary,argv2[i]);
				argv[argc++] = ary;
				continue;
			}
			if (strcmp(s,list_end)==0) {
				ac--; at++;
				if (level>0) return argc;
				RAISE("closing list without opening");
			}
		}
		argv[argc++]=Bridge_import_value(at);
		ac--; at++;
	}
	if (level) RAISE("opening %d list(s) without closing",level);
	return argc;
}

static Ruby BFObject_method_missing_1 (FMessage *fm) {
	Ruby argv[fm->ac+2];
	argv[0] = INT2NUM(fm->winlet);
	argv[1] = ID2SYM(rb_intern(fm->selector->s_name));
	int argc = Bridge_import_list(fm->ac,fm->at,argv+2);
	rb_funcall2(fm->self->rself,SI(send_in),argc+2,argv);
	return Qnil;
}

static Ruby BFObject_rescue (FMessage *fm) {
	Ruby error_array = make_error_message();
//	for (int i=0; i<rb_ary_len(error_array); i++)
//		post("%s\n",rb_str_ptr(rb_ary_ptr(error_array)[i]));
	//!@#$leak
	if (fm->self) pd_error(fm->self,"%s",strdup(rb_str_ptr(
		rb_funcall(error_array,SI(join),1,rb_str_new2("\n")))));
	if (fm->self && fm->is_init) fm->self = 0;
	return Qnil;
}

static void BFObject_method_missing (BFObject *bself,
int winlet, t_symbol *selector, int ac, t_atom *at) {
//	post("%p %d %s %d\n",self,winlet,selector->s_name,ac);
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
//	post("BFProxy_method_missing: Hello World!");
	BFObject_method_missing(self->parent,self->inlet,s,argc,argv);
}
	
static Ruby BFObject_init_1 (FMessage *fm) {
	Ruby argv[fm->ac];
//	post("ac=%d at=%d\n",fm->ac,(int)fm->at);
	int argc = Bridge_import_list(fm->ac,fm->at,argv);
//	post("argc=%d\n",argc);

	Ruby qlass = find_fclass(fm->selector);
	Ruby rself = rb_funcall2(qlass,SI(new),argc,argv);
	DGS(FObject);
	self->bself = fm->self;
	self->bself->rself = rself;

	int ninlets = ninlets_of(rb_funcall(rself,SI(class),0));
	int noutlets = noutlets_of(rb_funcall(rself,SI(class),0));

/*
	post("%s in=%d out=%d\n",rb_str_ptr(rb_funcall(
		rb_funcall(rself,SI(class),0),SI(inspect),0)), ninlets, noutlets);
*/

	for (int i=1; i<ninlets; i++) {
		BFProxy *p = (BFProxy *)pd_new(BFProxy_class);
		p->parent = self->bself;
		p->inlet = i;
		inlet_new(self->bself, &p->ob_pd, 0,0);
	}

	for (int i=0; i<noutlets; i++) {
		assert(i<=MAX_OUTLETS); /* TROUBLE */
		self->bself->out[i] = outlet_new(self->bself,&s_anything);
	}
	return rself;
}

static void *BFObject_init (t_symbol *classsym, int ac, t_atom *at) {
	t_class *qlass = find_bfclass(classsym);
	BFObject *bself = (BFObject *)pd_new(qlass);
	FMessage fm = { self: bself, winlet:-1, selector: classsym,
		ac: ac, at: at, is_init: true };
	long r = rb_rescue2(
		(RMethod)BFObject_init_1,(Ruby)&fm,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
	if (r==Qnil) {
//		pd_error(bself,"hello");
		return 0; /* broken object */
	} else {
		return (void *)bself;
	}
}

static void BFObject_delete_1 (FMessage *fm) {
	if (fm->self->rself) {
		//post("BFObject_delete is handling object at %08x",(int)fm->self);
		rb_funcall(fm->self->rself,SI(delete),0);
	} else {
		post("BFObject_delete is NOT handling BROKEN object at %08x",(int)fm);
	}
}

static void BFObject_delete (BFObject *self) {
	FMessage fm = { self: self, winlet:-1, selector: gensym("delete"),
		ac: 0, at: 0, is_init: false };
	rb_rescue2(
		(RMethod)BFObject_delete_1,(Ruby)&fm,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
}

static void BFObject_class_init_1 (t_class *qlass) {
//	Ruby rself = BFObject::find_fclass(gensym(class_getname(qlass)));
	class_addanything(qlass,(t_method)BFObject_method_missing0);
}

static Ruby FObject_s_install_2(Ruby rself, char *name) {
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
		//fprintf(stderr,"HELP: %s\n", class_getname(qlass));
		class_sethelpsymbol(qlass,gensym(rb_str_ptr(path)));
		return Qnil;
	} else {
		RAISE("don't know how to do that");
	}
}

static t_clock *gf_alarm;

void gf_timer_handler (t_clock *alarm, void *obj) {
	if (is_in_ruby) {
		post("warning: ruby is not signal-reentrant (is this a signal?)\n");
		return;
	}
	is_in_ruby = true;
//	uint64 time = RtMetro_now2();
//	gfpost("tick");
	rb_funcall(mGridFlow2,SI(tick),0);
	clock_delay(gf_alarm,gf_bridge2->clock_tick/10);
//	gfpost("tick was: %lld\n",RtMetro_now2()-time);
	is_in_ruby = false;
	count_tick();
}       

Ruby gridflow_bridge_init (Ruby rself, Ruby p) {
	GFBridge *self = gf_bridge2 = FIX2PTR(GFBridge,p);
	self->name = "puredata";
	self->class_install = FObject_s_install_2;
	self->send_out = FObject_send_out_2;
	self->post = (void (*)(const char *, ...))post;
	self->post_does_ln = true;
	self->whatever = bridge_whatever;
	mGridFlow2 = EVAL("GridFlow");
	cPointer2 = EVAL("GridFlow::Pointer");
	rb_ivar_set(mGridFlow2, SI(@bfclasses_set), rb_hash_new());
	return Qnil;
}

extern "C" void gridflow_setup () {
	char *foo[] = {"Ruby-for-PureData","/dev/null"};
	post("setting up Ruby-for-PureData...");
	ruby_init();
	ruby_options(COUNT(foo),foo);
	bridge_common_init();
//	sym_lparen=ID2SYM(rb_intern("("));
//	sym_rparen=ID2SYM(rb_intern(")"));

	BFProxy_class = class_new(gensym("ruby_proxy"),
		NULL,NULL,sizeof(BFProxy),CLASS_PD|CLASS_NOINLET, A_NULL);

	class_addanything(BFProxy_class,BFProxy_method_missing);

	rb_define_singleton_method(EVAL("Data"),"gridflow_bridge_init",
		(RMethod)gridflow_bridge_init,1);

	post("(done)");

	if (!
	EVAL("begin require 'gridflow'; true; rescue Exception => e;\
		STDERR.puts \"ruby #{e.class}: #{e}: #{e.backtrace}\"; false; end"))
	{
		post("ERROR: Cannot load GridFlow-for-Ruby (gridflow.so)\n");
		return;
	}

//	rb_define_singleton_method(mGridFlow2,"find_file",gf_find_file,1);
	gf_alarm = clock_new(0,(void(*)())gf_timer_handler);
	gf_timer_handler(gf_alarm,0);
}
