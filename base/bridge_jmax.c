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

#include "bridge_jmax.h"
#include "../config.h"
#include "grid.h"
#include <ctype.h>

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

typedef struct Kludge {
	int inlets;
	int outlets;
	VALUE qlass;
} Kludge;

Kludge kludge;

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

static fts_status_t BFObject_class_init (fts_class_t *qlass,
int ac, const fts_atom_t *at) {
	fts_class_init(qlass, sizeof(BFObject),
		kludge.inlets,
		kludge.outlets,
		(void *) kludge.qlass);
	return fts_Success;
}

typedef struct Kludge2 {
	fts_object_t *peer;
	VALUE $;
	VALUE sel;
	int argc;
	VALUE *argv;
} Kludge2;

static Kludge2 closure;

static void BFObject_method_missing$1 (VALUE foo) {
	rb_funcall(closure.$,closure.sel,closure.argc,closure.argv);
}

static void BFObject_method_missing$2 (VALUE foo) {
	VALUE es = rb_eval_string("\"ruby #{$!.class}: #{$!}\"");
	fts_object_set_error(FObject_peer(closure.$),"%s",es);
}

typedef VALUE (*RFunc)();

static void BFObject_method_missing (fts_object_t *$,
int winlet, fts_symbol_t selector, int ac, const fts_atom_t *at) {
	const char *s = fts_symbol_name(selector);
	char buf[256];
	VALUE argv[ac];
/*	Kludge2 closure; */
	strcpy(buf+3,s);
	buf[0] = buf[2] = '_';
	buf[1] = '0' + winlet;
	closure.$ = BFObject_peer($);
	closure.sel = rb_intern(buf);
	closure.argc = ac;
	closure.argv = argv;
	{ int i; for (i=0; i<ac; i++) argv[i] = jr_convert(at+i); }
	rb_rescue(
		(RFunc)BFObject_method_missing$1,Qnil,
		(RFunc)BFObject_method_missing$2,Qnil);
}

VALUE FObject_s_new(FObject *peer, VALUE argc, VALUE *argv);

static void BFObject_init$1 (VALUE foo) {
	FObject_s_new(closure.peer,closure.argc,closure.argv);
	rb_funcall(closure.$,closure.sel,closure.argc,closure.argv);
}

static void BFObject_init (fts_object_t *$,
int winlet, fts_symbol_t selector, int ac, const fts_atom_t *at) {
	const char *s = fts_symbol_name(selector);
	char buf[256];
	VALUE argv[ac];
	closure.peer = $;
	closure.sel = rb_intern(s);
	ac--; at++;
	closure.argc = ac;
	closure.argv = argv;
	{ int i; for (i=0; i<ac; i++) argv[i] = jr_convert(at+i); }
	rb_rescue(
		(RFunc)BFObject_init$1,Qnil,
		(RFunc)BFObject_method_missing$2,Qnil);
}

VALUE FObject_s_install(VALUE $, VALUE name, VALUE inlets, VALUE outlets) {
	VALUE methods = rb_funcall($, rb_intern("instance_methods"), 0);
	fts_class_t *peer;
	int n = RARRAY(methods)->len;
	int i;
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
		fts_status_t foo = 
			fts_class_install(fts_new_symbol(name2),BFObject_class_init);
		if (!foo) raise(rb_eStandardError, "jMax class not installed");
	}			
		
	peer = fts_class_get_by_name(fts_new_symbol(name2));
	free(name2);

	fts_method_define_varargs(
		peer,fts_SystemInlet,fts_s_init,BFObject_init);

	for (i=0; i<n; i++) {
		const char *name = RSTRING(RARRAY(methods)->ptr[i])->ptr;
		whine("looking at #%s", name);
		/* max 10 inlets */
		if (strlen(name)>3 && isdigit(name[1]) &&
		 name[0]=='_' && name[2]=='_') {
			whine("will wrap that method");
			whine("it has arity %d",
				NUM2INT(rb_funcall(
					rb_funcall($, rb_intern("instance_method"), 1, RARRAY(methods)->ptr[i]),
					rb_intern("arity"), 0)));
			fts_method_define_varargs(
				peer,name[1]-'0',fts_new_symbol(name+3),
				BFObject_method_missing);
		}
	}
	return Qnil;
}

VALUE FObject_send_thru(VALUE argc, VALUE *argv, VALUE $) {
	int outlet;
	fts_symbol_t selector = fts_s_bang;
	fts_atom_t at[argc+1];
	int ac=0;
	int i;
	if (argc<1) EARG("args are (outlet, ...)");
	if (TYPE(argv[0])!=T_FIXNUM) EARG("should be Fixnum");
	outlet = FIX2INT(argv[0]);
	if (outlet<0 || outlet>fts_object_get_outlets_number(FObject_peer($))) {
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
		} else if (argc==1 && FLOAT_P(argv[0])) {
			selector = fts_s_float;
		} else {
			/* will raise shortly after this */
		}
		ac=argc;
		for (i=0; i<ac; i++) rj_convert(argv[i],at+i);
	}
	fts_outlet_send(FObject_peer($),outlet,selector,ac,at);
	return Qnil;
}

VALUE FObject_s_new(FObject *peer, VALUE argc, VALUE *argv) {
	VALUE $ = Data_Wrap_Struct(FObject_class, FObject_mark, FObject_sweep, peer);
	VALUE set = rb_ivar_get(GridFlow_module, rb_intern("@fobjects_set"));
	rb_hash_aset(set,$,Qtrue); /* prevent sweeping */
	rb_funcall($,rb_intern("initialize"),argc,argv);
	return $;
}

#define DEF(_class_,_name_,_argc_) \
	rb_define_method(_class_##_class,#_name_,_class_##_##_name_,_argc_)

#define SDEF(_class_,_name_,_argc_) \
	rb_define_singleton_method(_class_##_class,#_name_,_class_##_s_##_name_,_argc_)

void gf_install_bridge (void) {
	GridFlow_module = rb_eval_string("GridFlow");
	FObject_class = rb_define_class_under(GridFlow_module,
		"FObject", rb_cObject);
	rb_ivar_set(GridFlow_module, rb_intern("@fobjects_set"), rb_hash_new());
	DEF(FObject, send_thru, -1);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, 3);
}

