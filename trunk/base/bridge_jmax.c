/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002 by Mathieu Bouchard

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

#define new new_
#define template template_
#define this this_
#define operator operator_
#define class class_
#include "fts/fts.h"
#undef class
#undef operator
#undef this
#undef template
#undef new

#include "grid.h"
#include <ctype.h>
#include <stdarg.h>
#include <sys/time.h>

struct BFObject;
struct FMessage {
	BFObject *$;
	int winlet;
	fts_symbol_t selector;
	int ac;
	const fts_atom_t *at;
};

/* code that is common across all bridges. */
#include "bridge.c"

/* **************************************************************** */
/* BFObject */

struct BFObject : fts_object_t {
	Ruby peer;

	static fts_class_t *find_bfclass (fts_symbol_t sym) {
		Ruby v = rb_hash_aref(
			rb_ivar_get(mGridFlow2, SI(@bfclasses_set)),
			rb_str_new2(fts_symbol_name(sym)));
		if (v==Qnil) RAISE("class not found");
		return FIX2PTR(fts_class_t,v);
	}
	static Ruby find_fclass (fts_symbol_t sym) {
		Ruby v = rb_hash_aref(
			rb_ivar_get(mGridFlow2, SI(@fclasses_set)),
			rb_str_new2(fts_symbol_name(sym)));
		if (v==Qnil) RAISE("class not found");
		return v;
	}
};

static void Bridge_export_value(Ruby arg, fts_atom_t *at) {
	if (INTEGER_P(arg)) {
		fts_set_int(at,NUM2INT(arg));
	} else if (SYMBOL_P(arg)) {
		const char *name = rb_sym_name(arg);
		fts_set_symbol(at,fts_new_symbol_copy(name));
	} else if (FLOAT_P(arg)) {
		fts_set_float(at,RFLOAT(arg)->value);
/*
	} else if (rb_obj_class(arg)==cPointer2) {
		fts_set_ptr(at,Pointer_get(arg));
*/
	} else {
		/* can't use EARG here */
		rb_raise(rb_eArgError, "cannot convert argument of class '%s'",
			rb_str_ptr(rb_funcall(rb_funcall(arg,SI(class),0),SI(inspect),0)));
	}
}

static Ruby Bridge_import_value(const fts_atom_t *at) {
	if (fts_is_int(at)) {
		return INT2NUM(fts_get_int(at));
	} else if (fts_is_symbol(at)) {
		return ID2SYM(rb_intern(fts_symbol_name(fts_get_symbol(at))));
	} else if (fts_is_float(at)) {
		return rb_float_new(fts_get_float(at));
	} else if (fts_is_list(at)) {
		fts_list_t *l = fts_get_list(at);
		int n = fts_list_get_size(l);
		fts_atom_t *p = fts_list_get_ptr(l);
		Ruby a = rb_ary_new2(n);
		for (int i=0; i<n; i++) rb_ary_push(a,Bridge_import_value(p+i));
		return a;
//	} else if (fts_is_ptr(at)) {
//		return Pointer_new(fts_get_ptr(at));
	} else {
		post("warning: type \"%s\" not supported\n",fts_symbol_name(at->type));
		return Qnil; /* unknown */
	}
}

static Ruby BFObject_method_missing$1 (FMessage *fm) {
	const char *s = fts_symbol_name(fm->selector);
	char buf[256];
	Ruby argv[fm->ac];
	Ruby $ = fm->$->peer;
	strcpy(buf+3,s);
	buf[0] = buf[2] = '_';
	buf[1] = '0' + fm->winlet;
	ID sel = rb_intern(buf);
	for (int i=0; i<fm->ac; i++) argv[i] = Bridge_import_value(fm->at+i);
	rb_funcall2($,sel,fm->ac,argv);
	return Qnil;
}

static Ruby BFObject_rescue (FMessage *fm) {
//	EVAL("STDERR.puts $!.inspect");
//	Ruby error_array = EVAL("[\"ruby #{$!.class}: #{$!}\",*($!.backtrace)]");
	Ruby error_array = EVAL("[\"ruby #{$!.class}: #{$!}\"]");
	for (int i=0; i<rb_ary_len(error_array); i++)
		post("%s\n",rb_str_ptr(rb_ary_ptr(error_array)[i]));
	//!@#$leak
	if (fm->$) fts_object_set_error(fm->$,"%s",strdup(rb_str_ptr(
		rb_funcall(error_array,SI(join),0))));
	return Qnil;
}

static void BFObject_method_missing (BFObject *$,
int winlet, fts_symbol_t selector, int ac, const fts_atom_t *at) {
	FMessage fm = { $: $, winlet: winlet, selector: selector, ac: ac, at: at };
	rb_rescue2(
		(RFunc)BFObject_method_missing$1,(Ruby)&fm,
		(RFunc)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
}

static Ruby BFObject_init$1 (FMessage *fm) {
	Ruby argv[fm->ac];
	fm->ac--;
	fm->at++;
	for (int i=0; i<fm->ac; i++) argv[i] = Bridge_import_value(fm->at+i);
	Ruby qlass = (Ruby)fm->$->head.cl->user_data;
	Ruby rself = rb_funcall2(qlass,SI(new),fm->ac,argv);
	DGS(GridObject);
	$->foreign_peer = fm->$;
	$->foreign_peer->peer = rself;
	return rself;
}

static void BFObject_init (BFObject *$,
int winlet, fts_symbol_t selector, int ac, const fts_atom_t *at) {
	FMessage fm = { $: $, winlet: winlet, selector: selector, ac: ac, at: at };
	rb_rescue2(
		(RFunc)BFObject_init$1,(Ruby)&fm,
		(RFunc)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
}

static void BFObject_delete$1 (FMessage *fm) {
	post("BFObject_delete$1 says hello %08x\n",(int)fm->$);
	rb_funcall(fm->$->peer,SI(delete),0);
}

static void BFObject_delete (BFObject *$) {
	FMessage fm = { $: $, winlet:-1, selector: fts_new_symbol("delete"),
		ac: 0, at: 0 };
	rb_rescue2(
		(RFunc)BFObject_delete$1,(Ruby)&fm,
		(RFunc)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
}

#define RETIFFAIL(r,r2,fmt,args...) \
	if (r) { \
		post(fmt " failed: %s\n", args, fts_status_get_description(r)); \
		return r2;}

static fts_status_t BFObject_class_init$1 (fts_class_t *qlass) {
	Ruby $ = rb_hash_aref(rb_ivar_get(mGridFlow2,
		SI(@fclasses_set)), rb_str_new2(fts_symbol_name(qlass->mcl->name)));

	int ninlets = ninlets_of($);
	int noutlets = noutlets_of($);

	fts_status_t r;
	r = fts_class_init(qlass, sizeof(BFObject), ninlets, noutlets, (void *)$);
	RETIFFAIL(r,r,"fts_class_init%s","");
	
	r = fts_method_define_varargs(
		qlass,fts_SystemInlet,fts_s_init,(fts_method_t)BFObject_init);
	RETIFFAIL(r,r,"define_varargs (for %s)","#init");

	r = fts_method_define_varargs(
		qlass,fts_SystemInlet,fts_s_delete,(fts_method_t)BFObject_delete);
	RETIFFAIL(r,r,"define_varargs (for %s)","#delete");

	for (int i=0; i<ninlets; i++) {
		r = fts_method_define_varargs(qlass, i, fts_s_anything, (fts_method_t)BFObject_method_missing);
		RETIFFAIL(r,r,"define_varargs (for inlet %i)",i);
	}

	return fts_Success;
}

static fts_status_t BFObject_class_init (fts_class_t *qlass,
int ac, const fts_atom_t *at) {
	fts_status_t r;
	FMessage fm;
	fm.$ = 0;
	r = (fts_status_t) rb_rescue2(
		(RFunc)BFObject_class_init$1,(Ruby)qlass,
		(RFunc)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
	return (Ruby)r==Qnil ? 
		fts_new_status("Exception in BFObject_class_init$1") : r;
}

static Ruby FObject_s_install_2(Ruby $, char *name) {
	fts_status_t r = fts_class_install(
		fts_new_symbol_copy(name),BFObject_class_init);
	RETIFFAIL(r,Qnil,"fts_class_install%s","");
	return Qnil;
}

static Ruby FObject_send_out_2(int argc, Ruby *argv, Ruby sym, int outlet,
Ruby $) {
	fts_object_t *jo = FObject_peer($);
/*	post("%d\n",FObject_peer($));*/
	if (outlet<0 || outlet>=fts_object_get_outlets_number(jo)) {
		EARG("outlet %d does not exist",outlet);
	}
	fts_atom_t at[argc];
	fts_atom_t sel;
	Bridge_export_value(sym,&sel);
	for (int i=0; i<argc; i++) Bridge_export_value(argv[i],at+i);
	/*fprintf(stderr,"2: %s\n",fts_symbol_name(fts_get_symbol(&sel)));*/
	fts_outlet_send(jo,outlet,fts_get_symbol(&sel),argc,at);
	return Qnil;
}

static fts_alarm_t *gf_alarm;

static void gf_timer_handler (fts_alarm_t *alarm, void *obj) {
	long long time = RtMetro_now2();
//	post("tick\n");
	rb_funcall(mGridFlow2,SI(tick),0);
	fts_alarm_set_delay(gf_alarm,gf_bridge2->clock_tick);
	fts_alarm_arm(gf_alarm);
//	post("tick was: %lld\n",RtMetro_now2()-time);
	count_tick();
}       

static Ruby gridflow_bridge_init (Ruby rself, Ruby p) {
	GFBridge *$ = FIX2PTR(GFBridge,p);
	$->class_install = FObject_s_install_2;
	$->send_out = FObject_send_out_2;
	$->post = post;
	$->post_does_ln = false;
	gf_bridge2 = $;
	mGridFlow2 = EVAL("GridFlow");
	return Qnil;
}

static Ruby gf_find_file(Ruby $, Ruby s) {
	char buf[1024];
	if (TYPE(s)!=T_STRING) RAISE("expected string");
	/* unlikely buffer overflow ahead (blame jMax) */
	fts_file_get_read_path(RSTRING(s)->ptr,buf);
	return rb_str_new2(buf);
}

void gridflow_module_init () {
	char *foo[] = {"Ruby-for-jMax","/dev/null"};
	post("setting up Ruby-for-jMax...\n");
	ruby_init();
	ruby_options(COUNT(foo),foo);
	bridge_common_init();
	rb_define_singleton_method(EVAL("Data"),"gridflow_bridge_init",
		(RFunc)gridflow_bridge_init,1);
	post("(done)\n");

	if (!
	EVAL("begin require 'gridflow'; true; rescue Exception => e;\
		STDERR.puts \"ruby #{e.class}: #{e}: #{e.backtrace}\"; false; end"))
	{
		post("ERROR: Cannot load GridFlow-for-Ruby (gridflow.so)\n");
		return;
	}

	rb_define_singleton_method(mGridFlow2,"find_file",(RFunc)gf_find_file,1);
	/* if exception occurred above, will crash soon */
	gf_alarm = fts_alarm_new(fts_sched_get_clock(),gf_timer_handler,0);
	gf_timer_handler(gf_alarm,0);
}

fts_module_t gridflow_module = {
	"gridflow",
	"GridFlow: video, multidimensional arrays, Ruby scripting, etc.",
	gridflow_module_init, 0, 0};

