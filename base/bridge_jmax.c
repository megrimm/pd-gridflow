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

#include "../config.h"
#include "grid.h"
#include <ctype.h>
#include <stdarg.h>

#ifdef STANDALONE
typedef FObject fts_object_t;
typedef Symbol fts_symbol_t;
typedef Var fts_atom_t;
#define fts_is_symbol(a) Var_has_symbol(a)   
#define fts_is_int(a) Var_has_int(a)      
#define fts_get_symbol(a) Var_get_symbol(a)   
#define fts_get_int(a) Var_get_int(a)      
#define fts_get_ptr(a) Var_get_ptr(a)      
#define fts_set_symbol(a,b) Var_put_symbol(a,b) 
#define fts_set_int(a,b) Var_put_int(a,b)    
#define fts_set_ptr(a,b) Var_put_ptr(a,b)    
#define fts_new_symbol(x) Symbol_new(x)
#define fts_symbol_name(x) Symbol_name(x)
#define fts_status_get_description(x) "foobar"
#define fts_method_define_varargs(c,i,s,m) fts_method_define_optargs(c,i,s,m,0,0,0)
#define fts_object_get_outlets_number(o) o->head.cl->n_outlets
#define fts_outlet_send(a,b,c,d,e) Object_send_thru(a,b,c,d,e)
#else
#include "bridge_jmax.h"
#endif

VALUE FObject_class;
VALUE GridFlow_module;

/* **************************************************************** */
/* FObject */

/* for garbage collection */
void FObject_mark (VALUE *$) {}
void FObject_sweep (VALUE *$) {}

#define INTEGER_P(_value_) (FIXNUM_P(_value_) || TYPE(_value_)==T_BIGNUM)
#define FLOAT_P(_value_) (TYPE(_value_)==T_FLOAT)
#define EARG(_reason_) rb_raise(rb_eArgError, _reason_)

typedef struct BFObject {
	fts_object_t _o;
	VALUE peer;
} BFObject;

fts_object_t *FObject_peer(VALUE $) {
	fts_object_t *peer;
	Data_Get_Struct($,fts_object_t,peer);
	return peer;
}

VALUE BFObject_peer(fts_object_t *$) {
	return ((BFObject *)$)->peer;
}

void rj_convert(VALUE arg, fts_atom_t *at) {
	if (INTEGER_P(arg)) {
		fts_set_int(at,NUM2INT(arg));
	} else if (SYMBOL_P(arg)) {
		char *name = rb_id2name(SYM2ID(arg));
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
static struct {
	fts_object_t *$;
	int winlet;
	fts_symbol_t selector;
	int ac;
	const fts_atom_t *at;
} z2;

static VALUE BFObject_method_missing$1 (VALUE foo) {
	const char *s = fts_symbol_name(z2.selector);
	char buf[256];
	VALUE argv[z2.ac];
	VALUE $ = BFObject_peer(z2.$);
	ID sel;
	strcpy(buf+3,s);
	buf[0] = buf[2] = '_';
	buf[1] = '0' + z2.winlet;
	sel = rb_intern(buf);
	{ int i; for (i=0; i<z2.ac; i++) argv[i] = jr_convert(z2.at+i); }
	rb_funcall2($,sel,z2.ac,argv);
	return Qnil;
}

static VALUE BFObject_method_missing$2 (VALUE foo) {
	VALUE es = rb_eval_string("\"ruby #{$!.class}: #{$!}: #{$!.backtrace}\"");
//	VALUE $ = BFObject_peer(z2.$);
	whine("jmaxobject = %p", z2.$);
//	whine("rubyobject = %p", $);
	whine("%s",RSTRING(es)->ptr);
	fts_object_set_error(z2.$,"%s",RSTRING(es)->ptr);
	return Qnil;
}

typedef VALUE (*RFunc)();

static void BFObject_method_missing (fts_object_t *$,
int winlet, fts_symbol_t selector, int ac, const fts_atom_t *at) {
	z2.$ = $;
	z2.winlet = winlet;
	z2.selector = selector;
	z2.ac = ac;
	z2.at = at;
	rb_rescue2(
		(RFunc)BFObject_method_missing$1,Qnil,
		(RFunc)BFObject_method_missing$2,Qnil,
		rb_eException,0);
}

VALUE FObject_s_new(BFObject *peer, VALUE qlass, VALUE argc, VALUE *argv);

static VALUE BFObject_init$1 (VALUE foo) {
	VALUE argv[z2.ac];
	z2.ac--;
	z2.at++;
	{ int i; for (i=0; i<z2.ac; i++) argv[i] = jr_convert(z2.at+i); }
	{
		VALUE $ = FObject_s_new((BFObject *)z2.$,
			(VALUE)z2.$->head.cl->user_data,z2.ac,argv);
		((BFObject *)z2.$)->peer = $;
		return $;
	}
}

static void BFObject_init (fts_object_t *$,
int winlet, fts_symbol_t selector, int ac, const fts_atom_t *at) {
	int r;
	z2.$ = $;
	z2.winlet = winlet;
	z2.ac = ac;
	z2.at = at;


	r = rb_rescue2(
		(RFunc)BFObject_init$1,Qnil,
		(RFunc)BFObject_method_missing$2,Qnil,
		rb_eException,0);
	whine("rb_rescue2: %d",r);
}

static struct {
	int inlets;
	int outlets;
	VALUE qlass;
} kludge;

#define RETIFFAIL(name,r) \
	if (r) { \
		whine(name " failed: %s", fts_status_get_description(r)); \
		return r;}

#define RETIFFAIL2(name,r) \
	if (r) { \
		whine(name " failed: %s", fts_status_get_description(r)); \
		return Qnil;}


static fts_status_t BFObject_class_init (fts_class_t *qlass,
int ac, const fts_atom_t *at) {
	VALUE $ = kludge.qlass;
	VALUE methods = rb_funcall($, rb_intern("instance_methods"), 0);
	int n = RARRAY(methods)->len;
	int i;
	int objectsize = sizeof(BFObject);
	fts_status_t r;

	whine("objectsize=%d, inlets=%d, outlets=%d, rubyclass=%p",
		objectsize, kludge.inlets, kludge.outlets, kludge.qlass);

	r = fts_class_init(qlass, objectsize,
		kludge.inlets, kludge.outlets, (void *)$);

	RETIFFAIL("fts_class_init",r);
	
	r = fts_method_define_varargs(
		qlass,fts_SystemInlet,fts_s_init,BFObject_init);
	
	RETIFFAIL("define_varargs (for constructor)",r);

	for (i=0; i<n; i++) {
		const char *name = RSTRING(RARRAY(methods)->ptr[i])->ptr;
		whine("looking at #%s", name);
		/* max 10 inlets */
		if (strlen(name)>3 && isdigit(name[1]) &&
		 name[0]=='_' && name[2]=='_') {
			int inlet = name[1]-'0';
			if (inlet<0 || inlet>=kludge.inlets) {
				whine("inlet #%d does not exist, skipping");
				continue;
			}
			whine("will wrap that method");
			whine("it has arity %d",
				NUM2INT(rb_funcall(
					rb_funcall($, rb_intern("instance_method"), 1, RARRAY(methods)->ptr[i]),
					rb_intern("arity"), 0)));
			r = fts_method_define_varargs(
				qlass,inlet,fts_new_symbol(name+3),
				BFObject_method_missing);
			
			RETIFFAIL("define_varargs",r);
		}
	}

	return fts_Success;
}

VALUE FObject_s_install(VALUE $, VALUE name, VALUE inlets, VALUE outlets) {
	char *name2;
	if (SYMBOL_P(name)) {
		name2 = rb_id2name(SYM2ID(name));
	} else if (TYPE(name) == T_STRING) {
		name2 = strdup(RSTRING(name)->ptr);
	} else {
		EARG("expect symbol");
	}
	whine("class will be called: %s",name2);
	kludge.inlets = NUM2INT(inlets);
	if (kludge.inlets<0 || kludge.inlets>9) EARG("...");
	kludge.outlets = NUM2INT(outlets);
	if (kludge.outlets<0 || kludge.outlets>9) EARG("...");
	kludge.qlass = $;
	{
		fts_status_t r = 
			fts_class_install(fts_new_symbol(name2),BFObject_class_init);
		RETIFFAIL2("fts_class_install",r);
	}			
		
	return Qnil;
}

VALUE FObject_send_thru(int argc, VALUE *argv, VALUE $) {
	int outlet;
	fts_symbol_t selector = fts_s_bang;
	fts_atom_t at[argc+1];
	int ac=0;
	int i;
	if (argc<1) EARG("args are (outlet, ...)");
	if (TYPE(argv[0])!=T_FIXNUM) EARG("should be Fixnum");
	outlet = FIX2INT(argv[0]);
	if (outlet<0 || outlet>=fts_object_get_outlets_number(FObject_peer($))) {
		EARG("outlet does not exist");
	}
	argc--;
	argv++;
	if (argv==0) {
		selector = fts_s_bang;
	} else {
		if (SYMBOL_P(argv[0])) {
			rj_convert(argv[0],at);
			selector = fts_get_symbol(at);
			argc--;
			argv++;
		} else if (argc>1) {
			selector = fts_s_list;
		} else if (argc==1 && INTEGER_P(argv[0])) {
			selector = fts_s_int;
//		} else if (argc==1 && FLOAT_P(argv[0])) {
//			selector = fts_s_float;
		} else {
			/* will raise shortly after this */
		}
		ac=argc;
		for (i=0; i<ac; i++) rj_convert(argv[i],at+i);
	}
	fts_outlet_send(FObject_peer($),outlet,selector,ac,at);
	return Qnil;
}

void whinep(VALUE $) {
	rb_funcall(rb_eval_string("STDERR"),rb_intern("puts"),1,
		rb_funcall($,rb_intern("inspect"),0));
}

VALUE FObject_s_new(BFObject *peer, VALUE qlass, VALUE argc, VALUE *argv) {
	VALUE $ = Data_Wrap_Struct(qlass, FObject_mark, FObject_sweep, peer);
	VALUE keep = rb_ivar_get(GridFlow_module, rb_intern("@fobjects_set"));
	rb_hash_aset(keep,$,Qtrue); /* prevent sweeping */

	whinep($);
	{
		int i;
		for (i=0; i<argc; i++) whinep(argv[i]);
	}
	whine("argc = %d",argc);
	whine("argv = %p",argv);
	rb_funcall2($,rb_intern("initialize"),argc,argv);
	return $;
}

#define DEF(_class_,_name_,_argc_) \
	rb_define_method(_class_##_class,#_name_,_class_##_##_name_,_argc_)

#define SDEF(_class_,_name_,_argc_) \
	rb_define_singleton_method(_class_##_class,#_name_,_class_##_s_##_name_,_argc_)

VALUE gf_post_string (VALUE $, VALUE s) {
	if (TYPE(s) != T_STRING) rb_raise(rb_eArgError, "not a String");

	post("%s",RSTRING(s)->ptr);
//	fprintf(stderr,"%s",RSTRING(s)->ptr);
	return Qnil;
}

void gf_install_bridge (void) {
	GridFlow_module = rb_eval_string("GridFlow");
	FObject_class = rb_define_class_under(GridFlow_module, "FObject", rb_cObject);
	rb_ivar_set(GridFlow_module, rb_intern("@fobjects_set"), rb_hash_new());
	DEF(FObject, send_thru, -1);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, 3);
	rb_define_singleton_method(GridFlow_module,
		"post_string", gf_post_string, 1);
}

struct Timer {
	void (*f)(struct Timer *$, void *data);
	void *data;
	double time;
	int armed;
};

void gridflow_module_init (void) {
	gf_init();
}

fts_module_t gridflow_module = {
	"video",
	"GridFlow: n-dimensional array streaming, pictures, video, etc.",
	gridflow_module_init, 0, 0};

/*
	a slightly friendlier version of post(...)
	it removes redundant messages.
	it also ensures that a \n is added at the end.
*/
void whine(char *fmt, ...) {
	static char *last_format = 0;
	static int format_count = 0;

	if (last_format && strcmp(last_format,fmt)==0) {
		format_count++;
		if (format_count >= 64) {
			if (high_bit(format_count)-low_bit(format_count) < 3) {
				post("[too many similar posts. this is # %d]\n",format_count);
#ifdef MAKE_TMP_LOG
				fprintf(whine_f,"[too many similar posts. this is # %d]\n",format_count);
				fflush(whine_f);
#endif
			}
			return;
		}
	} else {
		last_format = strdup(fmt);
		format_count = 1;
	}

	/* do the real stuff now */
	{
		va_list args;
		char post_s[256*4];
		int length;
		va_start(args,fmt);
		length = vsnprintf(post_s,sizeof(post_s)-2,fmt,args);
		post_s[sizeof(post_s)-1]=0; /* safety */
		post("%s%s%.*s",whine_header,post_s,post_s[length-1]!='\n',"\n");
#ifdef MAKE_TMP_LOG
		fprintf(whine_f,"[whine] %s%.*s",post_s,post_s[length-1]!='\n',"\n");
		fflush(whine_f);
#endif
	}
}

