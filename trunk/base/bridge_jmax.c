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

/* **************************************************************** */
/* FObject */

/* for garbage collection */
void FObject_mark (VALUE *$) {}
void FObject_sweep (VALUE *$) {fprintf(stderr,"sweeping FObject %p\n",$);}

struct BFObject {
	fts_object_t _o;
	VALUE peer;
};

fts_object_t *FObject_peer(VALUE rself) {
	DGS(GridObject);
	assert($->foreign_peer);
	return $->foreign_peer;
}

VALUE BFObject_peer(fts_object_t *$) {
	return ((BFObject *)$)->peer;
}

void rj_convert(VALUE arg, fts_atom_t *at) {
	if (INTEGER_P(arg)) {
		fts_set_int(at,NUM2INT(arg));
	} else if (SYMBOL_P(arg)) {
		char *name = rb_id2name(SYM2ID(arg));
		fprintf(stderr,"1: %s\n",name);
		fts_set_symbol(at,fts_new_symbol(name));
		free(name);
//	} else if (FLOAT_P(arg)) {
//		fts_set_float(at,RFLOAT(arg)->value);
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
//	} else if (fts_is_float(at)) {
//		return rb_float_new(fts_get_float(at));
//	} else if (fts_is_ptr(at)) {
//		return Qnil; /* not supported */
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

static VALUE BFObject_method_missing$1 (kludge *z2) {
	const char *s = fts_symbol_name(z2->selector);
	char buf[256];
	VALUE argv[z2->ac];
	VALUE $ = BFObject_peer(z2->$);
	ID sel;
	strcpy(buf+3,s);
	buf[0] = buf[2] = '_';
	buf[1] = '0' + z2->winlet;
	sel = rb_intern(buf);
	{ int i; for (i=0; i<z2->ac; i++) argv[i] = jr_convert(z2->at+i); }
	rb_funcall2($,sel,z2->ac,argv);
	return Qnil;
}

static VALUE BFObject_rescue (kludge *z2) {
	VALUE es = rb_eval_string("\"ruby #{$!.class}: #{$!}: #{$!.backtrace}\"");
//	VALUE $ = BFObject_peer(z2->$);
	post("jmaxobject = %p\n", z2->$);
//	post("rubyobject = %p\n", $);
	post("%s\n",RSTRING(es)->ptr);
	if (z2->$) fts_object_set_error(z2->$,"%s",RSTRING(es)->ptr);
	return Qnil;
}

typedef VALUE (*RFunc)();

static void BFObject_method_missing (fts_object_t *$,
int winlet, fts_symbol_t selector, int ac, const fts_atom_t *at) {
	kludge z2;
	z2.$ = $;
	z2.winlet = winlet;
	z2.selector = selector;
	z2.ac = ac;
	z2.at = at;
	rb_rescue2(
		(RFunc)BFObject_method_missing$1,&z2,
		(RFunc)BFObject_rescue,&z2,
		rb_eException,0);
}

static VALUE BFObject_init$1 (kludge *z2) {
	VALUE argv[z2->ac];
	z2->ac--;
	z2->at++;
	{ int i; for (i=0; i<z2->ac; i++) argv[i] = jr_convert(z2->at+i); }
	{
		BFObject *j_peer = (BFObject *)z2->$;
		VALUE qlass = (VALUE)z2->$->head.cl->user_data;
		VALUE rself = FObject_s_new(z2->ac,argv,qlass);
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
	kludge z2;
	z2.$ = $;
	z2.winlet = winlet;
	z2.ac = ac;
	z2.at = at;

	r = rb_rescue2(
		(RFunc)BFObject_init$1,&z2,
		(RFunc)BFObject_rescue,&z2,rb_eException,0);
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
	int objectsize = sizeof(BFObject);
	fts_status_t r;

	post("name=%s\n",fts_symbol_name(qlass->mcl->name));

	$ = rb_hash_aref(rb_ivar_get(GridFlow_module,rb_intern("@fclasses_set")),
		rb_str_new2(fts_symbol_name(qlass->mcl->name)));

	methods = rb_funcall($, rb_intern("instance_methods"), 0);
	n = RARRAY(methods)->len;

	inlets = rb_ivar_get($,rb_intern("@inlets"));
	outlets = rb_ivar_get($,rb_intern("@outlets"));

	post("objectsize=%d, inlets=%d, outlets=%d, rubyclass=%p\n",
		objectsize, inlets, outlets, qlass);

	r = fts_class_init(qlass, objectsize, inlets, outlets, (void *)$);

	RETIFFAIL("fts_class_init",r);
	
	r = fts_method_define_varargs(
		qlass,fts_SystemInlet,fts_s_init,BFObject_init);
	
	RETIFFAIL("define_varargs (for constructor)",r);

	for (i=0; i<n; i++) {
		const char *name = RSTRING(RARRAY(methods)->ptr[i])->ptr;
		post("looking at #%s\n", name);
		/* max 10 inlets */
		if (strlen(name)>3 && isdigit(name[1]) &&
		 name[0]=='_' && name[2]=='_') {
			int inlet = name[1]-'0';
			if (inlet<0 || inlet>=inlets) {
				post("inlet #%d does not exist, skipping\n");
				continue;
			}
			//post("will wrap that method\n");
			r = fts_method_define_varargs(
				qlass,inlet,fts_new_symbol(name+3),
				BFObject_method_missing);
			
			RETIFFAIL("define_varargs",r);
		}
	}

	return fts_Success;
}

static fts_status_t BFObject_class_init (fts_class_t *qlass,
int ac, const fts_atom_t *at) {
	VALUE r;
	kludge z2;
	z2.$ = 0;
	r = rb_rescue2(
		(RFunc)BFObject_class_init$1,qlass,
		(RFunc)BFObject_rescue,&z2,
		rb_eException,0);
	return r==Qnil;
}

VALUE FObject_s_install(VALUE $, VALUE name, VALUE inlets2, VALUE outlets2) {
	int inlets;
	int outlets;

	VALUE name2;
	if (SYMBOL_P(name)) {
		name2 = rb_funcall(name,rb_intern("dup"),0);
	} else if (TYPE(name) == T_STRING) {
		name2 = rb_funcall(name,rb_intern("to_s"),0);
	} else {
		EARG("expect symbol or string");
	}
	inlets = NUM2INT(inlets2);
	if (inlets<0 || inlets>9) EARG("...");
	outlets = NUM2INT(outlets2);
	if (outlets<0 || outlets>9) EARG("...");
	post("class will be called: %s\n",RSTRING(name2)->ptr);
	rb_ivar_set($,rb_intern("@inlets"),inlets);
	rb_ivar_set($,rb_intern("@outlets"),outlets);
	rb_ivar_set($,rb_intern("@foreign_name"),name2);
	rb_hash_aset(rb_ivar_get(GridFlow_module,rb_intern("@fclasses_set")),
		name2, $);
	{
		fts_status_t r = fts_class_install(
			fts_new_symbol(RSTRING(name2)->ptr),BFObject_class_init);
		RETIFFAIL2("fts_class_install",r);
	}			
		
	return Qnil;
}

VALUE FObject_send_out_2(int argc, VALUE *argv, VALUE $) {
	VALUE sym;
	int outlet;
	FObject_send_out_3(&argc,&argv,&sym,&outlet);
	post("%d\n",FObject_peer($));
	if (outlet<0 || outlet>=fts_object_get_outlets_number(FObject_peer($))) {
		EARG("outlet %d does not exist",outlet);
	}
	{
		int i;
		fts_atom_t at[argc];
		fts_atom_t sel;
		rj_convert(sym,&sel);
		for (i=0; i<argc; i++) rj_convert(argv[i],at+i);
		fprintf(stderr,"2: %s\n",fts_symbol_name(fts_get_symbol(&sel)));
		fts_outlet_send(FObject_peer($),outlet,fts_get_symbol(&sel),argc,at);
	}
	return Qnil;
}

void gf_install_bridge (void) {
	FObject_class = rb_define_class_under(GridFlow_module, "FObject", rb_cObject);
	rb_ivar_set(GridFlow_module, rb_intern("@fobjects_set"), rb_hash_new());
	DEF(FObject, send_out, -1);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, 3);
	rb_define_singleton_method(GridFlow_module, "post_string", gf_post_string, 1);
}

static fts_alarm_t *gf_alarm;

void gf_timer_handler (fts_alarm_t *alarm, void *obj) {
	long long time = RtMetro_now();
//	whine("tick");
	rb_eval_string("begin $mainloop.one(0); rescue Exception => e;\
		GridFlow.whine \"ruby #{e.class}: #{e}: #{e.backtrace}\"; end");
	fts_alarm_set_delay(gf_alarm,500.0);
	fts_alarm_arm(gf_alarm);
//	whine("tick was: %lld\n",RtMetro_now()-time);
}       

void gridflow_module_init (void) {
	char *foo[] = {"jmax","/dev/null"};
	post("setting up Ruby-for-jMax...\n");
	ruby_init();
	ruby_options(COUNT(foo),foo);
	post("(done)\n");
	gf_init();
	{
		gf_alarm = fts_alarm_new(fts_sched_get_clock(),gf_timer_handler,0);
		gf_timer_handler(gf_alarm,0);
	}
}

fts_module_t gridflow_module = {
	"video",
	"GridFlow: n-dimensional array streaming, pictures, video, etc.",
	gridflow_module_init, 0, 0};

