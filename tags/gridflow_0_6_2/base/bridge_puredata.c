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

#include "grid.h"
#include <ctype.h>
#include <stdarg.h>
#include <sys/time.h>

/* sorry. memory leaks must be found by hand in this module */
#undef strdup

/* resolving conflict: T_OBJECT will be PD's, not Ruby's */
#undef T_OBJECT
#include <m_pd.h>

/* can't even refer to the other GridFlow_module before ruby loads gridflow */
static VALUE GridFlow_module2=0;
static VALUE Pointer_class2=0;
static VALUE sym_ninlets=0, sym_noutlets=0;
static VALUE sym_lparen=0, sym_rparen=0;

static bool is_in_ruby = false;

#define rb_sym_name rb_sym_name_r4j
static const char *rb_sym_name(VALUE sym) {return rb_id2name(SYM2ID(sym));}

static VALUE Pointer_new (void *ptr) {
	return Data_Wrap_Struct(rb_eval_string("GridFlow::Pointer"), 0, 0, ptr);
}

static void *Pointer_get (VALUE self) {
	void *p;
	Data_Get_Struct(self,void *,p);
	return p;
}

/* **************************************************************** */
/* BFObject */

struct BFObject : t_object {
	VALUE peer;
	t_outlet *out[MAX_OUTLETS];

	static t_class *find_bfclass (t_symbol *sym) {
		VALUE v = rb_hash_aref(
			rb_ivar_get(GridFlow_module2, SI(@bfclasses_set)),
			rb_str_new2(sym->s_name));
		if (v==Qnil) RAISE("class not found");
		return (t_class *) FIX2PTR(v);
	}
	static VALUE find_fclass (t_symbol *sym) {
		VALUE v = rb_hash_aref(
			rb_ivar_get(GridFlow_module2, SI(@fclasses_set)),
			rb_str_new2(sym->s_name));
		if (v==Qnil) RAISE("class not found");
		return v;
	}
};

int ninlets_of (VALUE qlass) {
	if (!rb_ivar_defined(qlass,SYM2ID(sym_ninlets))) RAISE("no inlet count");
	return INT(rb_ivar_get(qlass,SYM2ID(sym_ninlets)));
}

int noutlets_of (VALUE qlass) {
	if (!rb_ivar_defined(qlass,SYM2ID(sym_noutlets))) RAISE("no outlet count");
	return INT(rb_ivar_get(qlass,SYM2ID(sym_noutlets)));
}

static t_class *BFProxy_class;

struct BFProxy : t_object {
	t_object *parent;
	int inlet;
};

BFObject *FObject_peer(VALUE rself) {
	DGS(GridObject);
	return (BFObject *) $->foreign_peer;
}

void Bridge_export_value(VALUE arg, t_atom *at) {
	if (INTEGER_P(arg)) {
		SETFLOAT(at,NUM2INT(arg));
	} else if (SYMBOL_P(arg)) {
		const char *name = rb_sym_name(arg);
		/*fprintf(stderr,"1: %s\n",name);*/
		SETSYMBOL(at,gensym((char *)name));
	} else if (FLOAT_P(arg)) {
		SETFLOAT(at,RFLOAT(arg)->value);
//	} else if (CLASS_OF(arg)==Pointer_class2) {
//		SETPOINTER(at,Pointer_get(arg));
	} else {
		/* can't use EARG here */
		rb_raise(rb_eArgError, "cannot convert argument of class '%s'",
			rb_str_ptr(rb_funcall(rb_funcall(arg,SI(class),0),SI(inspect),0)));
	}
}

VALUE Bridge_import_value(const t_atom *at) {
	t_atomtype t = at->a_type;
	if (t==A_SYMBOL) {
		return ID2SYM(rb_intern(at->a_w.w_symbol->s_name));
	} else if (t==A_FLOAT) {
		return rb_float_new(at->a_w.w_float);
	} else if (t==A_POINTER) {
		return Pointer_new(at->a_w.w_gpointer);
	} else {
		return Qnil; /* unknown */
	}
}

/*
  Yes, I really have to do that myself.
  PureData is not very sophisticated: it does not have a parser.
*/
#include<unistd.h>
int Bridge_import_list(int &ac, const t_atom *&at, VALUE *argv,
int level=0) {
	int argc=0;
	while (ac) {
		t_atomtype t = at->a_type;
//		fprintf(stderr,"ac=%d at=%08x t=%08x\n",ac,(long)at,(long)t);
		if (t==A_SYMBOL) {
			const char *s=at->a_w.w_symbol->s_name;
			if (strcmp(s,"(")==0) {
				ac--; at++;
				VALUE ary = rb_ary_new();
				VALUE argv2[ac];
				int argc2 = Bridge_import_list(ac,at,argv2,level+1);
				for (int i=0; i<argc2; i++) rb_ary_push(ary,argv2[i]);
				argv[argc++] = ary;
				continue;
			}
			if (strcmp(s,")")==0) {
				ac--; at++;
				if (level>0) return argc;
				RAISE("closing parenthese without opening");
			}
		}
		argv[argc++]=Bridge_import_value(at);
		ac--; at++;
	}
	if (level) RAISE("opening %d parenthese(s) without closing",level);
	return argc;
}

// kludge
typedef struct {
	BFObject *$;
	int winlet;
	t_symbol *selector;
	int ac;
	const t_atom *at;
	bool is_init;
} kludge;

static VALUE BFObject_method_missing$1 (kludge *k) {
	const char *s = k->selector->s_name;
	char buf[256];
	VALUE $ = k->$->peer;
	strcpy(buf+3,s);
	buf[0] = buf[2] = '_';
	buf[1] = '0' + k->winlet;
	ID sel = rb_intern(buf);

	VALUE argv[k->ac];
	int argc = Bridge_import_list(k->ac,k->at,argv);

	rb_funcall2($,sel,argc,argv);
	return Qnil;
}

static VALUE BFObject_rescue (kludge *k) {
//	rb_eval_string("STDERR.puts $!.inspect");
//	VALUE error_array = rb_eval_string(
//		"[\"ruby #{$!.class}: #{$!}\",*($!.backtrace)]");
	VALUE error_array = rb_eval_string(
		"[\"ruby #{$!.class}: #{$!}\"]");
//	for (int i=0; i<rb_ary_len(error_array); i++)
//		post("%s\n",rb_str_ptr(rb_ary_ptr(error_array)[i]));
	//!@#$leak
	if (k->$) pd_error(k->$,"%s",strdup(rb_str_ptr(
		rb_funcall(error_array,SI(join),0))));
	if (k->$ && k->is_init) {
		k->$ = 0;
	}
	return Qnil;
}

static void BFObject_method_missing (BFObject *$,
int winlet, t_symbol *selector, int ac, t_atom *at) {
//	fprintf(stderr,"%p %d %s %d\n",$,winlet,selector->s_name,ac);
	kludge k;
	k.$ = $;
	k.winlet = winlet;
	k.selector = selector;
	k.ac = ac;
	k.at = at;
	k.is_init = false;
	if (!$->peer) {
		pd_error($,"you are trying to send a message to a dead object. \
			the precise reason was given when you last tried to create \
			the object. fix the contents of that box and try again.");
		return;
	}
	rb_rescue2(
		(RFunc)BFObject_method_missing$1,(VALUE)&k,
		(RFunc)BFObject_rescue,(VALUE)&k,
		rb_eException,0);
}

static void BFObject_method_missing0 (BFObject *$,
t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing($,0,s,argc,argv);
}

static void BFProxy_method_missing(BFProxy *$,
t_symbol *s, int argc, t_atom *argv) {
//	post("BFProxy_method_missing: Hello World!");
	BFObject_method_missing((BFObject *)$->parent,$->inlet,s,argc,argv);
}
	

#define $R rself

static VALUE BFObject_init$1 (kludge *k) {
	VALUE argv[k->ac];
//	fprintf(stderr,"k->ac=%d k->at=%d\n",k->ac,(int)k->at);
	int argc = Bridge_import_list(k->ac,k->at,argv);
//	fprintf(stderr,"argc=%d\n",argc);

	BFObject *j_peer = (BFObject *)k->$;
	VALUE qlass = BFObject::find_fclass(k->selector);
	VALUE rself = rb_funcall2(qlass,SI(new),argc,argv);
	j_peer->peer = rself;
	DGS(GridObject);
	$->foreign_peer = (void *)j_peer;

	int ninlets = ninlets_of(rb_funcall(rself,SI(class),0));
	int noutlets = noutlets_of(rb_funcall(rself,SI(class),0));

/*
	fprintf(stderr,"%s in=%d out=%d\n",
		rb_str_ptr(rb_funcall(rb_funcall(rself,SI(class),0),SI(inspect),0)),
		ninlets,noutlets);
*/

//	if (ninlets>0) inlet_new(j_peer,&j_peer->ob_pd,0,0);

	for (int i=1; i<ninlets; i++) {
		BFProxy *p = (BFProxy *)pd_new(BFProxy_class);
		p->parent = j_peer;
		p->inlet = i;
		inlet_new(j_peer, &p->ob_pd, 0,0);
	}

	for (int i=0; i<noutlets; i++) {
		j_peer->out[i] = outlet_new(j_peer,&s_anything);
	}
	return rself;
}

static void *BFObject_init (t_symbol *classsym, int ac, t_atom *at) {
	t_class *qlass = BFObject::find_bfclass(classsym);
	BFObject *self = (BFObject *)pd_new(qlass);

	kludge k;
	k.is_init = true;
	k.$ = (BFObject *)$;
	k.selector = classsym;
	k.ac = ac;
	k.at = at;

	rb_rescue2(
		(RFunc)BFObject_init$1,(VALUE)&k,
		(RFunc)BFObject_rescue,(VALUE)&k,
		rb_eException,0);

	/* now what do i do with constructors that raised exception? */

	return (void *)self;
}

static void BFObject_delete$1 (kludge *k) {
	fprintf(stderr,"BFObject_delete$1 says hello %08x\n",(int)k->$);
	rb_funcall(k->$->peer,SI(delete),0);
}

static void BFObject_delete (BFObject *$) {
	kludge k;
	k.$ = $;
	k.is_init = false;
	k.selector = gensym("delete");
	k.ac = 0;
	k.at = 0;

	rb_rescue2(
		(RFunc)BFObject_delete$1,(VALUE)&k,
		(RFunc)BFObject_rescue,(VALUE)&k,
		rb_eException,0);
}

static void BFObject_class_init$1 (t_class *qlass) {
	VALUE $ = BFObject::find_fclass(gensym(class_getname(qlass)));
	class_addanything(qlass,(t_method)BFObject_method_missing0);
}

VALUE FObject_s_install_2(VALUE $, char *name2, VALUE inlets2, VALUE outlets2) {
	t_class *qlass = class_new(gensym(name2),
		(t_newmethod)BFObject_init, (t_method)BFObject_delete,
		sizeof(BFObject),
		CLASS_DEFAULT,
		A_GIMME,0);

	rb_hash_aset(rb_ivar_get(GridFlow_module2,SI(@bfclasses_set)),
		rb_str_new2(name2), PTR2FIX(qlass));

	kludge k;
	k.$ = 0;
	k.is_init = false;
	rb_rescue2(
		(RFunc)BFObject_class_init$1,(VALUE)qlass,
		(RFunc)BFObject_rescue,(VALUE)&k,
		rb_eException,0);
	return Qnil;
}

VALUE FObject_send_out_2(int argc, VALUE *argv, VALUE sym, int outlet, VALUE $) {
	BFObject *jo = (BFObject *) FObject_peer($);
	if (!jo) return Qnil; /* Ruby-only object */
	VALUE qlass = rb_funcall($,SI(class),0);
	if (outlet<0 || outlet>=noutlets_of(qlass)) {
		EARG("outlet %d does not exist",outlet);
	}
	t_atom at[argc];
	t_atom sel;
	Bridge_export_value(sym,&sel);
	for (int i=0; i<argc; i++) Bridge_export_value(argv[i],at+i);
	t_outlet *out = jo->te_outlet;
	outlet_anything(jo->out[outlet],atom_getsymbol(&sel),argc,at);
	return Qnil;
}

static t_clock *gf_alarm;

/* copy-paste */
static uint64 RtMetro_now2(void) {
	struct timeval nowtv;
	gettimeofday(&nowtv,0);
	return nowtv.tv_sec * 1000000LL + nowtv.tv_usec;
}

void gf_timer_handler (t_clock *alarm, void *obj) {
	static int count = 0;
	if (count%1000==0) fprintf(stderr,"survived to %d ticks\n",count);
	if (is_in_ruby) {
		fprintf(stderr,"warning: ruby is not reentrant\n");
		return;
	}
	is_in_ruby = true;
	long long time = RtMetro_now2();
//	gfpost("tick");
	rb_funcall(GridFlow_module2,SI(tick),0);
	clock_delay(gf_alarm,GF_TIMER_GRANULARITY);
//	gfpost("tick was: %lld\n",RtMetro_now2()-time);
	is_in_ruby = false;
	count++;
}       

VALUE gridflow_bridge_init (VALUE rself, VALUE p) {
	GFBridge *$ = (GFBridge *) FIX2PTR(p);
	$->class_install = FObject_s_install_2;
	$->send_out = FObject_send_out_2;
	$->post = (void (*)(const char *, ...))post;
	$->post_does_ln = true;
	GridFlow_module2 = rb_eval_string("GridFlow");
	Pointer_class2 = rb_eval_string("GridFlow::Pointer");
	rb_ivar_set(GridFlow_module2, SI(@bfclasses_set), rb_hash_new());
	return Qnil;
}

/*
static VALUE gf_find_file(VALUE $, VALUE s) {
	char buf[1024];
	if (TYPE(s)!=T_STRING) RAISE("expected string");
	// unlikely buffer overflow ahead
	fts_file_get_read_path(RSTRING(s)->ptr,buf);
	return rb_str_new2(buf);
}
*/

extern "C" void gridflow_setup () {
	char *foo[] = {"Ruby-for-PureData","/dev/null"};
	post("setting up Ruby-for-PureData...");
	ruby_init();
	ruby_options(COUNT(foo),foo);
	sym_ninlets =SYM(@ninlets);
	sym_noutlets=SYM(@noutlets);
//	sym_lparen=ID2SYM(rb_intern("("));
//	sym_rparen=ID2SYM(rb_intern(")"));

	BFProxy_class = class_new(gensym("ruby_proxy"),
		NULL,NULL,sizeof(BFProxy),CLASS_PD|CLASS_NOINLET, A_NULL);

	class_addanything(BFProxy_class,BFProxy_method_missing);

	rb_define_singleton_method(rb_eval_string("Data"),"gridflow_bridge_init",
		(RFunc)gridflow_bridge_init,1);

	post("(done)");
	rb_eval_string("begin require 'gridflow'; rescue Exception => e;\
		STDERR.puts \"ruby #{e.class}: #{e}: #{e.backtrace}\"; end");
//	rb_define_singleton_method(GridFlow_module2,"find_file",gf_find_file,1);
	/* if exception occurred above, will crash soon */
	gf_alarm = clock_new(0,(void(*)())gf_timer_handler);
	gf_timer_handler(gf_alarm,0);
}
