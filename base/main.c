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

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "grid.h"

#define MAKE_LEAK_DUMP

#include "../config.h"
#include <assert.h>
#include <limits.h>

VALUE GridFlow_module; /* not the same as jMax's gridflow_module */
VALUE FObject_class;

struct GFBridge gf_bridge = {
	send_out: 0,
	class_install: 0,
	post: (void(*)(const char *, ...))printf,
};

void startup_number(void);
void startup_grid(void);
void startup_flow_objects(void);

/* **************************************************************** */
/* Allocation */

VALUE gf_alloc_set = Qnil;

/* to help find uninitialized values */
void *qalloc(size_t n, const char *file, int line) {
	long *data = (long *) qalloc2(n,file,line);
	#ifndef NO_DEADBEEF
	{
		int i;
		int nn = (int) n/4;
		for (i=0; i<nn; i++) data[i] = 0xDEADBEEF;
	}
	#endif
	return data;	

}

typedef struct AllocTrace {
	size_t n;
	const char *file;
	int line;
} AllocTrace;

void *qalloc2(size_t n, const char *file, int line) {
	void *data = malloc(n);
	assert(data);
#ifdef MAKE_LEAK_DUMP
	if (gf_alloc_set) {
		AllocTrace *al = (AllocTrace *) malloc(sizeof(AllocTrace));
		al->n    = n   ;
		al->file = file;
		al->line = line;
/*		Dict_put(gf_alloc_set,data,al); */
	}
#endif
	return data;
}

void *qrealloc(void *data, int n) {
	void *data2 = realloc(data,n);
#ifdef MAKE_LEAK_DUMP
	if (gf_alloc_set) {
/*
		void *a = Dict_get(gf_alloc_set,data);
		Dict_del(gf_alloc_set,data);
		Dict_put(gf_alloc_set,data2,a);
*/
	}
#endif
	return data2;
}

/* to help find dangling references */
void qfree(void *data) {
	assert(data);
#ifdef MAKE_LEAK_DUMP
	if (gf_alloc_set) {
/*
		void *a = Dict_get(gf_alloc_set,data);
		if (a) free(a);
		Dict_del(gf_alloc_set,data);
*/
	}
#endif
	{
		int n=8;
		data = realloc(data,n);
#ifndef NO_DEADBEEF
		{
			long *data2 = (long *) data;
			int i;
			int nn = (int) n/4;
			for (i=0; i<nn; i++) data2[i] = 0xFADEDF00;
		}
#endif
	}
	free(data);
}

static void qdump$1(void *obj, void *k, void *v) {
	AllocTrace *al = (AllocTrace *)v;
	post("warning: %d bytes leak allocated at file %s line %d",
		al->n,al->file,al->line);
}

void qdump(void) {
	post("(leak detection disabled)");
/*
	post("checking for memory leaks...");
	VALUE ary;
	ary = rb_funcall(gf_alloc_set,rb_intern("
	Dict_each(gf_alloc_set,qdump$1,0);
	if (Dict_size(gf_alloc_set)==0) {
		post("no leaks (yet)");
	}
*/
}

VALUE gf_object_set = 0;

FILE *whine_f;

/* ---------------------------------------------------------------- */
/* FObject */

VALUE rb_ary_fetch(VALUE $, int i) {
	VALUE argv[] = { INT2NUM(i) };
	return rb_ary_aref(COUNT(argv),argv,$);
}

char *rb_sym_name(VALUE sym) {
	return strdup(rb_id2name(SYM2ID(sym)));
}

void FObject_mark (VALUE *$) {}
void FObject_sweep (VALUE *$) {fprintf(stderr,"sweeping FObject %p\n",$);}

void FObject_send_out_3(int *argc, VALUE **argv, VALUE *sym, int *outlet) {
	if (*argc<1) RAISE("not enough args");
/*
	{int i; for(i=0; i<(*argc); i++)
		fprintf(stderr,"%s\n",
			RSTRING(rb_funcall((*argv)[i],rb_intern("inspect"),0))->ptr);}
*/
	*outlet = INT(**argv);
	if (*outlet<0 || *outlet>9 /*|| *outlet>real_outlet_max*/)
		RAISE("invalid outlet number");
	(*argc)--, (*argv)++;
	if (*argc<1) {
		*sym = SYM(bang);
	} else if (*argc>1 && !SYMBOL_P(**argv)) {
		*sym = SYM(list);
	} else if (INTEGER_P(**argv)) {
		*sym = SYM(int);
	} else if (FLOAT_P(**argv)) {
		*sym = SYM(float);
	} else if (SYMBOL_P(**argv)) {
		*sym = **argv;
		(*argc)--, (*argv)++;
	} else {
		RAISE("bad message");
	}
}

VALUE FObject_send_out(int argc, VALUE *argv, VALUE $) {
	VALUE ary = rb_ivar_get($,rb_intern("@outlets"));
	VALUE sym;
	int outlet;
	int i, n;
	FObject_send_out_3(&argc,&argv,&sym,&outlet);
	if (gf_bridge.send_out)
		gf_bridge.send_out(argc,argv,sym,outlet,$);
	if (ary==Qnil) return Qnil;
	n = RARRAY(ary)->len;
	for (i=0; i<n; i++) {
		VALUE conn = rb_ary_fetch(ary,i);
		VALUE rec = rb_ary_fetch(conn,0);
		int inl = INT(rb_ary_fetch(conn,1));
		char buf[256];
		sprintf(buf,"_%d_%s",inl,rb_sym_name(sym));
		rb_funcall2(rec,rb_intern(buf),argc,argv);
	}
	return Qnil;
}

VALUE FObject_s_new(VALUE argc, VALUE *argv, VALUE qlass) {
	BFObject *foreign_peer = 0; /* for now */
	VALUE keep = rb_ivar_get(GridFlow_module, rb_intern("@fobjects_set"));
	GridObject *c_peer = NEW(GridObject,10); /* !@#$ allocate correct length */
	VALUE $; /* ruby_peer */
	c_peer->foreign_peer = foreign_peer;
	$ = Data_Wrap_Struct(qlass, FObject_mark, FObject_sweep, c_peer);
	c_peer->peer = $;
	{
		VALUE gc2 = rb_ivar_get(qlass,rb_intern("@grid_class"));
		if (gc2==Qnil) RAISE("@grid_class not found in %s",
			RSTRING(rb_funcall(qlass,rb_intern("inspect"),0))->ptr);
		c_peer->grid_class = gc2==Qnil ? 0 : FIX2PTR(gc2);
	}
	rb_hash_aset(keep,$,Qtrue); /* prevent sweeping */

	rb_funcall2($,rb_intern("initialize"),argc,argv);
	return $;
}

VALUE FObject_s_install(VALUE $, VALUE name, VALUE inlets2, VALUE outlets2) {
	int inlets, outlets;
	VALUE name2;
	if (SYMBOL_P(name)) {
		name2 = rb_funcall(name,rb_intern("dup"),0);
	} else if (TYPE(name) == T_STRING) {
		name2 = rb_funcall(name,rb_intern("to_str"),0);
	} else {
		EARG("expect symbol or string");
	}
	inlets = INT(inlets2);
	if (inlets<0 || inlets>9) EARG("...");
	outlets = INT(outlets2);
	if (outlets<0 || outlets>9) EARG("...");
	rb_ivar_set($,rb_intern("@inlets"),inlets);
	rb_ivar_set($,rb_intern("@outlets"),outlets);
	rb_ivar_set($,rb_intern("@foreign_name"),name2);
	rb_hash_aset(rb_ivar_get(GridFlow_module,rb_intern("@fclasses_set")),
		name2, $);
	if (gf_bridge.class_install)
		gf_bridge.class_install($,RSTRING(name2)->ptr,inlets2,outlets2);
	return Qnil;
}

/* ---------------------------------------------------------------- */
/* The setup part */

#define DECL_SYM2(_sym_) VALUE sym_##_sym_ = 0xDeadBeef;
#define DEF_SYM(_sym_) sym_##_sym_ = SYM(_sym_);

DECL_SYM2(grid_begin)
DECL_SYM2(grid_flow)
DECL_SYM2(grid_end)
DECL_SYM2(bang)
DECL_SYM2(int)
DECL_SYM2(list)

static VALUE GridFlow_exec (VALUE $, VALUE data, VALUE func) {
	void *data2 = FIX2PTR(data);
	void (*func2)() = FIX2PTR(func);
	func2(data2);
	return Qnil;
}

/* this is the entry point for all of the above */

/* **************************************************************** */
/* Procs of somewhat general utility */

void whine_time(const char *s) {
	struct timeval t;
	gettimeofday(&t,0);
	whine("%s: %d.%06d\n",s,t.tv_sec,t.tv_usec);
}

/* Key for method signature codes:
	s Symbol
	i Fixnum (int)
	l List (???)
	p void *
	+ more of the same
	; begin optional section
*/
void define_many_methods(VALUE $, int n, MethodDecl *methods) {
	VALUE args[16]; /* not really used anymore */
	int i;
	fprintf(stderr,"here are %d methods:\n",n);
	for (i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		int j;
		const char *s = md->signature;
		char buf[256];
		int n_args=-1; /* yes, really. sorry. */
		if (md->winlet>=0) {
			sprintf(buf,"_%d_%s",md->winlet,md->selector);
		} else {
			if (strcmp(md->selector,"init")==0) {
				sprintf(buf,"initialize");
			} else {
				sprintf(buf,"%s",md->selector);
			}
		}
		fprintf(stderr,"%s: adding method #%s\n",
			RSTRING(rb_funcall($,rb_intern("inspect"),0))->ptr,buf);
		rb_define_method($,buf,(VALUE(*)())md->method,n_args);
		fprintf(stderr,"(hello)\n");
	}
}

void MainLoop_add(void *data, void (*func)(void)) {
	rb_funcall(rb_eval_string("$tasks"),rb_intern("[]="), 2,
		PTR2FIX(data), PTR2FIX(func));
}

void MainLoop_remove(void *data) {
	rb_funcall(rb_eval_string("$tasks"),rb_intern("delete"), 1,
		PTR2FIX(data));
}

void whine(const char *fmt, ...) {
	va_list args;
	char post_s[256*4];
	int length;
	va_start(args,fmt);
	length = vsnprintf(post_s,sizeof(post_s)-2,fmt,args);
	post_s[sizeof(post_s)-1]=0; /* safety */
	rb_funcall(GridFlow_module,rb_intern("whine2"),2,
		rb_str_new2(fmt),rb_str_new2(post_s));
}

VALUE gf_post_string (VALUE $, VALUE s) {
	if (TYPE(s) != T_STRING) rb_raise(rb_eArgError, "not a String");

	post("%s",RSTRING(s)->ptr);
	return Qnil;
}

/*
void post(const char *fmt, ...) {
	va_list args;
	va_start(args,fmt);
	vfprintf(stderr,fmt,args);
}
*/

/* ---------------------------------------------------------------- */

/* Ruby's entrypoint. */
void Init_gridflow (void) /*throws Exception*/ {
	DEF_SYM(grid_begin);
	DEF_SYM(grid_flow);
	DEF_SYM(grid_end);
	DEF_SYM(bang);
	DEF_SYM(int);
	DEF_SYM(list);

	puts("Hello World!");

	/* !@#$ mark */
	gf_alloc_set  = rb_hash_new();
	gf_object_set = rb_hash_new();

	GridFlow_module = rb_define_module("GridFlow");
	rb_define_singleton_method(GridFlow_module,"exec",GridFlow_exec,2);
	rb_define_singleton_method(GridFlow_module, "post_string", gf_post_string, 1);
	rb_ivar_set(GridFlow_module, rb_intern("@fobjects_set"), rb_hash_new());
	rb_ivar_set(GridFlow_module, rb_intern("@fclasses_set"), rb_hash_new());


	FObject_class = rb_define_class_under(GridFlow_module, "FObject", rb_cObject);
	DEF(FObject, send_out, -1);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, -1);

	post("GridFlow "GF_VERSION", as compiled on "GF_COMPILE_TIME"\n");

	{
		VALUE cdata = rb_eval_string("Data");
		if (rb_respond_to(cdata,rb_intern("gridflow_bridge_init"))) {
			post("Setting up bridge...\n");
			rb_funcall(cdata,rb_intern("gridflow_bridge_init"),
				1,PTR2FIX(&gf_bridge));
			post("Done setting up bridge.\n");
		} else {
			post("bridge not found\n");
		}
	}

	/* run startup of every source file */
	startup_number();
	startup_grid();
	startup_flow_objects();

	rb_require("gridflow/base/main.rb");
}

