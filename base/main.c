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

#include "../config.h"
#include <assert.h>
#include <limits.h>

GFStack *gf_call_stack = 0;

Ruby GridFlow_module; /* not the same as jMax's gridflow_module */
Ruby FObject_class;
static Ruby sym_outlets=0;

static void default_post(const char *fmt, ...) {
	va_list args;
	va_start(args,fmt);
	vfprintf(stderr,fmt,args);
	va_end(args);
}

struct GFBridge gf_bridge = {
	send_out: 0,
	class_install: 0,
	post: default_post,
	post_does_ln: false,
	clock_tick: 10.0,
};

/* ---------------------------------------------------------------- */
/* GridFlow::FObject */

Ruby rb_ary_fetch(Ruby $, int i) {
	Ruby argv[] = { INT2NUM(i) };
	return rb_ary_aref(COUNT(argv),argv,$);
}

const char *rb_sym_name(Ruby sym) {return rb_id2name(SYM2ID(sym));}

void FObject_mark (void *z) {
/*
	GridObject *$ = (GridObject *)z;
//	fprintf(stderr,"marking FObject c++=%p ruby=%p\n",$,$->peer);
	$->mark();
*/
}

static int object_count=0;

static void FObject_free (void *$) {
//	fprintf(stderr,"Say farewell to %08x\n",(int)$);
	delete (GridObject *)$;
//	object_count -= 1; fprintf(stderr,"object_count=%d\n",object_count);
}

void FObject_send_out_3(int *argc, Ruby **argv, Ruby *sym, int *outlet) {
	if (*argc<1) RAISE("not enough args");
/*
	{int i; for(i=0; i<(*argc); i++)
		fprintf(stderr,"%s\n",
			RSTRING(rb_funcall((*argv)[i],SI(inspect),0))->ptr);}
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

Ruby FObject_send_out(int argc, Ruby *argv, Ruby $) {
	Ruby sym;
	int outlet;
	FObject_send_out_3(&argc,&argv,&sym,&outlet);
	if (gf_bridge.send_out)
		gf_bridge.send_out(argc,argv,sym,outlet,$);

	Ruby ary = rb_ivar_defined($,SYM2ID(sym_outlets)) ?
		rb_ivar_get($,SYM2ID(sym_outlets)) : Qnil;
//	printf("1: ");	rb_p(ary);
	if (ary==Qnil) return Qnil;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	ary = rb_ary_fetch(ary,outlet);
//	printf("2: ");	rb_p(ary);
	if (ary==Qnil) return Qnil;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	int n = RARRAY(ary)->len;
	for (int i=0; i<n; i++) {
		Ruby conn = rb_ary_fetch(ary,i);
		Ruby rec = rb_ary_fetch(conn,0);
		int inl = INT(rb_ary_fetch(conn,1));
//		printf("3: ");	rb_p(conn);
		char buf[256];
		sprintf(buf,"_%d_%s",inl,rb_sym_name(sym));
		/*
		fprintf(stderr,"%s:\n",buf);
		for (int i=0; i<argc; i++) rb_p(argv[i]);
		fprintf(stderr,".\n");
		*/
		rb_funcall2(rec,rb_intern(buf),argc,argv);
	}
	return Qnil;
}

Ruby FObject_delete(Ruby argc, Ruby *argv, Ruby $) {
	Ruby keep = rb_ivar_get(GridFlow_module, SI(@fobjects_set));
	rb_funcall(keep,SI(delete),1,$);
//	fprintf(stderr,"del: @fobjects_set.size: %ld\n",INT(rb_funcall(keep,SI(size),0)));
	return Qnil;
}

Ruby FObject_s_new(Ruby argc, Ruby *argv, Ruby qlass) {
	Ruby gc2 = rb_ivar_defined(qlass,SI(@grid_class)) ?
		rb_ivar_get(qlass,SI(@grid_class)) : Qnil;
/*
	if (gc2==Qnil) RAISE("@grid_class not found in %s",
		RSTRING(rb_funcall(qlass,SI(inspect),0))->ptr);
*/

	GridClass *gc = (GridClass *)(gc2==Qnil ? 0 : FIX2PTR(gc2));

	Ruby keep = rb_ivar_get(GridFlow_module, SI(@fobjects_set));
	GridObject *c_peer = gc ? (GridObject *)gc->allocate() : new GridObject();
	c_peer->foreign_peer = 0;
	Ruby $ = Data_Wrap_Struct(qlass, FObject_mark, FObject_free, c_peer);
	c_peer->peer = $;
	c_peer->grid_class = gc;
	rb_hash_aset(keep,$,Qtrue); /* prevent sweeping */
//	fprintf(stderr,"new: @fobjects_set.size: %ld\n",INT(rb_funcall(keep,SI(size),0)));
	rb_funcall2($,SI(initialize),argc,argv);
//	object_count += 1; fprintf(stderr,"object_count=%d\n",object_count);
	return $;
}

Ruby FObject_s_install(Ruby $, Ruby name, Ruby inlets2, Ruby outlets2) {
	int inlets, outlets;
	Ruby name2;
	if (SYMBOL_P(name)) {
		name2 = rb_funcall(name,SI(dup),0);
	} else if (TYPE(name) == T_STRING) {
		name2 = rb_funcall(name,SI(to_str),0);
	} else {
		EARG("expect symbol or string");
	}
	inlets = INT(inlets2);
	if (inlets<0 || inlets>9) EARG("...");
	outlets = INT(outlets2);
	if (outlets<0 || outlets>9) EARG("...");
	rb_ivar_set($,SI(@ninlets),INT2NUM(inlets));
	rb_ivar_set($,SI(@noutlets),INT2NUM(outlets));
	rb_ivar_set($,SI(@foreign_name),name2);
	rb_hash_aset(rb_ivar_get(GridFlow_module,SI(@fclasses_set)),
		name2, $);
	if (gf_bridge.class_install)
		gf_bridge.class_install($,RSTRING(name2)->ptr,inlets2,outlets2);
	return Qnil;
}

/* begin Ruby 1.6 compatibility */

static uint64 num2ull(Ruby val) {
    if (FIXNUM_P(val)) return (uint64)FIX2INT(val);
	if (TYPE(val)!=T_BIGNUM) RAISE("type error");
	uint64 v =
		(uint64)NUM2UINT(rb_funcall(val,SI(>>),1,INT2FIX(32))) << 32;
	return v + NUM2UINT(rb_funcall(val,SI(&),1,UINT2NUM(0xffffffff)));
}

static Ruby ull2num(uint64 val) {
	return rb_funcall(rb_funcall(UINT2NUM((uint32)val),SI(<<),1,INT2FIX(32)),
		SI(+),1,UINT2NUM(val>>32));
}

/* end */

Ruby FObject_profiler_cumul(Ruby rself) {
	DGS(GridObject);
	return ull2num($->profiler_cumul);
}

Ruby FObject_profiler_cumul_assign(Ruby rself, Ruby arg) {
	DGS(GridObject);
	$->profiler_cumul = num2ull(arg);
	return arg;
}

/* ---------------------------------------------------------------- */
/* The setup part */

#define DECL_SYM2(_sym_) Ruby sym_##_sym_ = 0xDeadBeef;
#define DEF_SYM(_sym_) sym_##_sym_ = SYM(_sym_);

DECL_SYM2(grid_begin)
DECL_SYM2(grid_flow)
DECL_SYM2(grid_end)
DECL_SYM2(bang)
DECL_SYM2(int)
DECL_SYM2(list)

static Ruby GridFlow_exec (Ruby $, Ruby data, Ruby func) {
	void *data2 = FIX2PTR(data);
	void (*func2)(void*) = (void(*)(void*))FIX2PTR(func);
	func2(data2);
	return Qnil;
}

/* this is the entry point for all of the above */

/* **************************************************************** */
/* Procs of somewhat general utility */

void define_many_methods(Ruby $, int n, MethodDecl *methods) {
/*	fprintf(stderr,"here are %d methods:\n",n); */
	for (int i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		const char *buf =
			strcmp(md->selector,"init")==0 ? "initialize" :
			strcmp(md->selector,"del")==0 ? "delete" :
			md->selector;
/*
		fprintf(stderr,"%s: adding method #%s\n",
			RSTRING(rb_funcall($,SI(inspect),0))->ptr,buf);
*/
		rb_define_method($,buf,(Ruby(*)())md->method,-1);
		rb_enable_super($,buf);
	}
}

Ruby GridFlow_clock_tick (Ruby $) {
	return rb_float_new(gf_bridge.clock_tick);
}

void GridFlow_clock_tick_set (Ruby $, Ruby tick) {
	if (TYPE(tick)!=T_FLOAT) RAISE("expecting Float");
	gf_bridge.clock_tick = RFLOAT(tick)->value;
}

void MainLoop_add(void *data, void (*func)(void*)) {
	rb_funcall(rb_eval_string("$tasks"),SI([]=), 2,
		PTR2FIX(data), PTR2FIX(func));
}

void MainLoop_remove(void *data) {
	rb_funcall(rb_eval_string("$tasks"),SI(delete), 1,
		PTR2FIX(data));
}

void gfpost(const char *fmt, ...) {
	va_list args;
	char post_s[256*4];
	int length;
	va_start(args,fmt);
	length = vsnprintf(post_s,sizeof(post_s)-2,fmt,args);
	post_s[sizeof(post_s)-1]=0; /* safety */
	va_end(args);
	if (rb_respond_to(GridFlow_module,SI(gfpost2))) {
		rb_funcall(GridFlow_module,SI(gfpost2),2,
			rb_str_new2(fmt),rb_str_new2(post_s));
	} else {
		default_post("%s",post_s);
	}
}

Ruby gf_post_string (Ruby $, Ruby s) {
	if (TYPE(s) != T_STRING) rb_raise(rb_eArgError, "not a String");
	char *p = rb_str_ptr(s);
//	bool has_ln = p[rb_str_len(p)-1]=='\n';
	gf_bridge.post(gf_bridge.post_does_ln?"%s":"%s\n",p);
	return Qnil;
}

Ruby ruby_c_install(GridClass *gc, Ruby super) {
	Ruby $ = rb_define_class_under(GridFlow_module, gc->name, super);
	rb_ivar_set($,SI(@grid_class),PTR2FIX(gc));
	define_many_methods($,gc->methodsn,gc->methods);
//remember to take care of delete
	rb_funcall($,SI(install),3,
		rb_str_new2(gc->jname),
		INT2NUM(gc->inlets),
		INT2NUM(gc->outlets));
	GridObject_conf_class($,gc);
	gc->rubyclass = $;
	if (gc->startup) gc->startup(gc);
	return Qnil;
}

/*
  I'm sorry to slow things down, but with PureData I can't do it
  the old way: pointers turned into integers turn into float32's
  and this is very evil and non-working.

  Actually: this does not work in PureData either, because I can't
  add datatypes to PureData. I am using float32's but splitting
  each pointer in two parts, which _is_ working.
*/
Ruby Pointer_class;

Ruby Pointer_new (void *ptr) {
	return Data_Wrap_Struct(rb_eval_string("GridFlow::Pointer"), 0, 0, ptr);
}

void *Pointer_get (Ruby self) {
	void *p;
	Data_Get_Struct(self,void *,p);
	return p;
}


/* ---------------------------------------------------------------- */

#include <signal.h>

void startup_number(void);
void startup_grid(void);
void startup_flow_objects(void);

/* Ruby's entrypoint. */
void Init_gridflow () {
	signal(11,SIG_DFL);

	DEF_SYM(grid_begin);
	DEF_SYM(grid_flow);
	DEF_SYM(grid_end);
	DEF_SYM(bang);
	DEF_SYM(int);
	DEF_SYM(list);
	sym_outlets=SYM(@outlets);

//	fprintf(stderr,"GridFlow_module=%p\n",(void*)GridFlow_module);
	GridFlow_module = rb_define_module("GridFlow");
//	fprintf(stderr,"GridFlow_module=%p\n",(void*)GridFlow_module);

	rb_define_singleton_method(GridFlow_module,"clock_tick",(RFunc)GridFlow_clock_tick,0);
	rb_define_singleton_method(GridFlow_module,"clock_tick=",(RFunc)GridFlow_clock_tick_set,1);
	rb_define_singleton_method(GridFlow_module,"exec",(RFunc)GridFlow_exec,2);
	rb_define_singleton_method(GridFlow_module, "post_string", (RFunc)gf_post_string, 1);
	rb_ivar_set(GridFlow_module, SI(@fobjects_set), rb_hash_new());
	rb_ivar_set(GridFlow_module, SI(@fclasses_set), rb_hash_new());

	Pointer_class = rb_define_class_under(GridFlow_module, "Pointer",
	rb_cObject);
	DEF(Pointer, get, 0);

	FObject_class = rb_define_class_under(GridFlow_module, "FObject", rb_cObject);
	DEF(FObject, send_out, -1);
	DEF(FObject, delete, -1);
	DEF(FObject, profiler_cumul, 0);
	DEF(FObject, profiler_cumul_assign, 1);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, -1);

	Ruby cdata = rb_eval_string("Data");
	ID bi = SI(gridflow_bridge_init);
	if (rb_respond_to(cdata,bi)) {
		//fprintf(stderr,"Setting up bridge...\n");
		rb_funcall(cdata,bi,1,PTR2FIX(&gf_bridge));
	}

	/* run startup of every source file */
	startup_number();
	startup_grid();
	startup_flow_objects();
	EVAL("STDERR.puts $:");
	EVAL("for f in %w(gridflow/base/main.rb gridflow/format/main.rb) do \
		require	f end");

	signal(11,SIG_DFL); /* paranoia */
}

