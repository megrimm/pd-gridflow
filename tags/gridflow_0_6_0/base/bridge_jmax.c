/*
	$Id$

	GridFlow
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <fts/fts.h>
#include "grid.h"
#include <ctype.h>
#include <stdarg.h>
#include <sys/time.h>
#undef post

static VALUE GridFlow_module;

#define rb_sym_name rb_sym_name_r4j
static char *rb_sym_name(VALUE sym) {
	return strdup(rb_id2name(SYM2ID(sym)));
}

/* **************************************************************** */
/* BFObject */

struct BFObject {
	fts_object_t _o;
	VALUE peer;
};

fts_object_t *FObject_peer(VALUE rself) {
	DGS(GridObject);
	return $->foreign_peer;
}

VALUE BFObject_peer(fts_object_t *$) {
	return ((BFObject *)$)->peer;
}

void rj_convert(VALUE arg, fts_atom_t *at) {
	if (INTEGER_P(arg)) {
		fts_set_int(at,NUM2INT(arg));
	} else if (SYMBOL_P(arg)) {
		char *name = rb_sym_name(arg);
		/*fprintf(stderr,"1: %s\n",name);*/
		fts_set_symbol(at,fts_new_symbol_copy(name));
	} else if (FLOAT_P(arg)) {
		fts_set_float(at,RFLOAT(arg)->value);
	} else {
		/* can't use EARG here */
		rb_raise(rb_eArgError, "cannot convert argument of class %s", CLASS_OF(arg));
	}
}

VALUE jr_convert(const fts_atom_t *at) {
	if (fts_is_int(at)) {
		return INT2NUM(fts_get_int(at));
	} else if (fts_is_symbol(at)) {
		return ID2SYM(rb_intern(fts_symbol_name(fts_get_symbol(at))));
	} else if (fts_is_float(at)) {
		return rb_float_new(fts_get_float(at));
	} else if (fts_is_ptr(at)) {
		return Qnil; /* not supported */
	} else {
		return Qnil; /* unknown */
	}
}

// kludge
typedef struct {
	fts_object_t *$;
	int winlet;
	fts_symbol_t selector;
	int ac;
	const fts_atom_t *at;
} kludge;

static VALUE BFObject_method_missing$1 (kludge *k) {
	const char *s = fts_symbol_name(k->selector);
	char buf[256];
	VALUE argv[k->ac];
	VALUE $ = BFObject_peer(k->$);
	ID sel;
	strcpy(buf+3,s);
	buf[0] = buf[2] = '_';
	buf[1] = '0' + k->winlet;
	sel = rb_intern(buf);
	{ int i; for (i=0; i<k->ac; i++) argv[i] = jr_convert(k->at+i); }
	rb_funcall2($,sel,k->ac,argv);
	return Qnil;
}

static VALUE BFObject_rescue (kludge *k) {
	VALUE es = rb_eval_string("\"ruby #{$!.class}: #{$!}: #{$!.backtrace}\"");
//	VALUE $ = BFObject_peer(k->$);
	post("jmaxobject = %p\n", k->$);
//	post("rubyobject = %p\n", $);
	post("%s\n",RSTRING(es)->ptr);
	if (k->$) fts_object_set_error(k->$,"%s",RSTRING(es)->ptr);
	return Qnil;
}

typedef VALUE (*RFunc)();

static void BFObject_method_missing (fts_object_t *$,
int winlet, fts_symbol_t selector, int ac, const fts_atom_t *at) {
	kludge k;
	k.$ = $;
	k.winlet = winlet;
	k.selector = selector;
	k.ac = ac;
	k.at = at;
	rb_rescue2(
		(RFunc)BFObject_method_missing$1,(VALUE)&k,
		(RFunc)BFObject_rescue,(VALUE)&k,
		rb_eException,0);
}

static VALUE BFObject_init$1 (kludge *k) {
	VALUE argv[k->ac];
	k->ac--;
	k->at++;
	{ int i; for (i=0; i<k->ac; i++) argv[i] = jr_convert(k->at+i); }
	{
		BFObject *j_peer = (BFObject *)k->$;
		VALUE qlass = (VALUE)k->$->head.cl->user_data;
		VALUE rself = rb_funcall2(qlass,rb_intern("new"),k->ac,argv);
		j_peer->peer = rself;
		{
			DGS(GridObject);
			$->foreign_peer = (void *)j_peer;
		}
		return rself;
	}
}

static void BFObject_init (fts_object_t *$,
int winlet, fts_symbol_t selector, int ac, const fts_atom_t *at) {
	int r;
	kludge k;
	k.$ = $;
	k.winlet = winlet;
	k.ac = ac;
	k.at = at;

	r = rb_rescue2(
		(RFunc)BFObject_init$1,(VALUE)&k,
		(RFunc)BFObject_rescue,(VALUE)&k,
		rb_eException,0);
}

#define RETIFFAIL(name,r) \
	if (r) { \
		post(name " failed: %s\n", fts_status_get_description(r)); \
		return r;}

#define RETIFFAIL2(name,r) \
	if (r) { \
		post(name " failed: %s\n", fts_status_get_description(r)); \
		return Qnil;}

static fts_status_t BFObject_class_init$1 (fts_class_t *qlass) {
	int inlets, outlets;
	VALUE $;
	VALUE methods;
	int i,n;
	fts_status_t r;

	post("name=%s\n",fts_symbol_name(qlass->mcl->name));

	$ = rb_hash_aref(rb_ivar_get(GridFlow_module,rb_intern("@fclasses_set")),
		rb_str_new2(fts_symbol_name(qlass->mcl->name)));

	post("$=%p\n",$);
/*  post("$=%s\n",RSTRING(rb_funcall($, rb_intern("inspect"), 0))->ptr); */

	methods = rb_funcall($, rb_intern("instance_methods"), 1, Qtrue);
	n = RARRAY(methods)->len;

/* post("instance_methods(true)=%s\n",
RSTRING(rb_funcall(methods,rb_intern("inspect"),0))->ptr); */

	inlets = rb_ivar_get($,rb_intern("@inlets"));
	outlets = rb_ivar_get($,rb_intern("@outlets"));

	post("inlets=%d, outlets=%d, rubyclass=%p\n",
		inlets, outlets, qlass);

	r = fts_class_init(qlass, sizeof(BFObject), inlets, outlets, (void *)$);

	RETIFFAIL("fts_class_init",r);
	
	r = fts_method_define_varargs(
		qlass,fts_SystemInlet,fts_s_init,BFObject_init);
	
	RETIFFAIL("define_varargs (for constructor)",r);

	for (i=0; i<n; i++) {
		const char *name = RSTRING(RARRAY(methods)->ptr[i])->ptr;
		/* max 10 inlets */
		if (strlen(name)>3 && isdigit(name[1]) &&
		 name[0]=='_' && name[2]=='_') {
			int inlet = name[1]-'0';
			if (inlet<0 || inlet>=inlets) {
				post("inlet #%d does not exist, skipping\n");
				continue;
			}
			post("will wrap method #%s\n", name);
			r = fts_method_define_varargs(
				qlass,inlet,fts_new_symbol_copy(name+3),
				BFObject_method_missing);
			
			RETIFFAIL("define_varargs",r);
		}
	}

	return fts_Success;
}

static fts_status_t BFObject_class_init (fts_class_t *qlass,
int ac, const fts_atom_t *at) {
	fts_status_t r;
	kludge k;
	k.$ = 0;
	r = (fts_status_t) rb_rescue2(
		(RFunc)BFObject_class_init$1,(VALUE)qlass,
		(RFunc)BFObject_rescue,(VALUE)&k,
		rb_eException,0);
	return (VALUE)r==Qnil ? 
		fts_new_status("Exception in BF_Object_class_init") : r;
}

VALUE FObject_s_install_2(VALUE $, char *name2, VALUE inlets2, VALUE outlets2) {
	fts_status_t r = fts_class_install(
		fts_new_symbol_copy(name2),BFObject_class_init);
	RETIFFAIL2("fts_class_install",r);
	return Qnil;
}

VALUE FObject_send_out_2(int argc, VALUE *argv, VALUE sym, int outlet, VALUE $) {
	fts_object_t *jo = FObject_peer($);
/*	post("%d\n",FObject_peer($));*/

	if (!jo) return Qnil; /* Ruby-only object */

	if (outlet<0 || outlet>=fts_object_get_outlets_number(jo)) {
		EARG("outlet %d does not exist",outlet);
	}
	{
		int i;
		fts_atom_t at[argc];
		fts_atom_t sel;
		rj_convert(sym,&sel);
		for (i=0; i<argc; i++) rj_convert(argv[i],at+i);
		/*fprintf(stderr,"2: %s\n",fts_symbol_name(fts_get_symbol(&sel)));*/
		fts_outlet_send(jo,outlet,fts_get_symbol(&sel),argc,at);
	}
	return Qnil;
}

static fts_alarm_t *gf_alarm;

/* copy-paste */
static uint64 RtMetro_now2(void) {
	struct timeval nowtv;
	gettimeofday(&nowtv,0);
	return nowtv.tv_sec * 1000000LL + nowtv.tv_usec;
}

void gf_timer_handler (fts_alarm_t *alarm, void *obj) {
	long long time = RtMetro_now2();
//	whine("tick");
	rb_eval_string("begin $mainloop.one(0); rescue Exception => e;\
		GridFlow.whine \"ruby #{e.class}: #{e}: #{e.backtrace}\"; end");
	fts_alarm_set_delay(gf_alarm,500.0);
	fts_alarm_arm(gf_alarm);
//	whine("tick was: %lld\n",RtMetro_now2()-time);
}       

VALUE gridflow_bridge_init (VALUE rself, VALUE p) {
	GFBridge *$ = FIX2PTR(p);
	$->class_install = FObject_s_install_2;
	$->send_out = FObject_send_out_2;
	$->post = post;
	GridFlow_module = rb_eval_string("GridFlow");
	return Qnil;
}

static VALUE gf_find_file(VALUE $, VALUE s) {
	char buf[1024];
	if (TYPE(s)!=T_STRING) RAISE("expected string");
	/* unlikely buffer overflow ahead */
	fts_file_get_read_path(RSTRING(s)->ptr,buf);
	return rb_str_new2(buf);
}

void gridflow_module_init (void) {
	char *foo[] = {"Ruby-for-jMax","/dev/null"};
	post("setting up Ruby-for-jMax...\n");
	ruby_init();
	ruby_options(COUNT(foo),foo);
	rb_define_singleton_method(rb_eval_string("Data"),"gridflow_bridge_init",
		gridflow_bridge_init,1);
	post("(done)\n");
	rb_eval_string("begin require 'gridflow'; rescue Exception => e;\
		STDERR.puts \"ruby #{e.class}: #{e}: #{e.backtrace}\"; end");
	rb_define_singleton_method(GridFlow_module,"find_file",gf_find_file,1);
	/* if exception occurred above, will crash soon */
	gf_alarm = fts_alarm_new(fts_sched_get_clock(),gf_timer_handler,0);
	gf_timer_handler(gf_alarm,0);
}

fts_module_t gridflow_module = {
	"gridflow",
	"GridFlow: video, multidimensional arrays, Ruby scripting, etc.",
	gridflow_module_init, 0, 0};

