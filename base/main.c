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
	return rb_id2name(SYM2ID(sym));
}

void FObject_send_out_3(int *argc, VALUE **argv, VALUE *sym, int *outlet) {
	if (*argc<1) RAISE("not enough args");
	{int i; for(i=0; i<(*argc); i++)
		fprintf(stderr,"%s\n",
			RSTRING(rb_funcall((*argv)[i],rb_intern("inspect"),0))->ptr);}
	*outlet = NUM2INT(**argv);
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
	FObject_send_out_2(argc,argv,$);
	FObject_send_out_3(&argc,&argv,&sym,&outlet);
//	whine("message: %s",rb_sym_name(sym));
	if (ary==Qnil) return Qnil;
	n = RARRAY(ary)->len;
	for (i=0; i<n; i++) {
		VALUE conn = rb_ary_fetch(ary,i);
		VALUE rec = rb_ary_fetch(conn,0);
		int inl = NUM2INT(rb_ary_fetch(conn,1));
		char buf[256];
		sprintf(buf,"_%d_%s",inl,rb_id2name(SYM2ID(sym)));
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

/* ---------------------------------------------------------------- */
/* The setup part */

#define DECL_SYM2(_sym_) \
	VALUE sym_##_sym_ = 0xDeadBeef;

DECL_SYM2(grid_begin)
DECL_SYM2(grid_flow)
DECL_SYM2(grid_end)
DECL_SYM2(bang)
DECL_SYM2(int)
DECL_SYM2(list)

static void disable_signal_handlers (void) {
	/*
		this is for debugging only and should be turned off in
		the normal distribution. this code ensures that crashes are
		not trapped by jmax itself so i can have something to inspect.
	*/
	static int signals[] = {
		SIGBUS, SIGSEGV, SIGTERM,
		SIGHUP, SIGINT, SIGQUIT, SIGABRT,
	};
	int i;
	for(i=0; i<COUNT(signals); i++) {
		signal(signals[i],SIG_DFL);
	}
}

static VALUE GridFlow_exec (VALUE $, VALUE data, VALUE func) {
	void *data2 = FIX2PTR(data);
	void (*func2)() = FIX2PTR(func);
	func2(data2);
	return Qnil;
}

VALUE gf_ruby_init$1 (void *foo) {
	gf_install_bridge();
	disable_signal_handlers(); /* paranoid; help me with gdb */
	rb_eval_string(
		"begin\n"
/*			"require '" GF_INSTALL_DIR "/ruby/main.rb'\n" */
			"require 'base/main.rb'\n"
/*
			"require '/home/projects/gridflow/extra/TkRubyListener.rb'\n"
			"$root = TkRoot.new { title 'GridFlow console' }\n"
			"def foo; STDERR.puts \"ruby-loop-tick\"; Tk.after(1000) { foo }; end; foo\n"
			"$listener = TkRubyListener.new($root,60,8)\n"
			"$listener.frame.pack\n"
			"Tk.mainloop\n"
*/
		"rescue Exception => e; STDERR.puts \"1: Ruby Exception: #{e.class}: #{e} #{e.backtrace}\"\n"
		"end\n"
	);
	return Qnil;
}

VALUE gf_ruby_init$2 (void *foo) {
	post("untrapped ruby exception occurred loading GridFlow\n");
	return Qnil;
}

#define DEF_SYM(_sym_) sym_##_sym_ = SYM(_sym_);

void showenv(const char *s) {
	post("%s = %s\n", s, getenv(s));
}

/* at this point, Ruby should already be started */
/* this part will load GridFlow _into_ Ruby */
void gf_init (void) {
	DEF_SYM(grid_begin);
	DEF_SYM(grid_flow);
	DEF_SYM(grid_end);
	DEF_SYM(bang);
	DEF_SYM(int);
	DEF_SYM(list);

	/* !@#$ mark */
	gf_alloc_set  = rb_hash_new();
	gf_object_set = rb_hash_new();

	GridFlow_module = rb_eval_string("module GridFlow; self; end");
	rb_define_singleton_method(GridFlow_module,"exec",GridFlow_exec,2);
	rb_ivar_set(GridFlow_module, rb_intern("@fobjects_set"), rb_hash_new());
	rb_ivar_set(GridFlow_module, rb_intern("@fclasses_set"), rb_hash_new());

	FObject_class = rb_define_class_under(GridFlow_module, "FObject", rb_cObject);
	DEF(FObject, send_out, -1);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, -1);

//	disable_signal_handlers();
	srandom(time(0));

	post("Welcome to GridFlow " GF_VERSION "\n");
	post("Compiled on: " GF_COMPILE_TIME "\n");
	post("--- GridFlow startup: begin ---\n");

	showenv("LD_LIBRARY_PATH");
	showenv("LIBRARY_PATH");
	showenv("PATH");
	showenv("DISPLAY");

	/* run startup of every source file */
	startup_number();
	startup_grid();
	startup_flow_objects();

	rb_rescue(gf_ruby_init$1,0,gf_ruby_init$2,0);

	post("--- GridFlow startup: end ---\n");
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
	for (i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		int j;
		int n_args=0;
		const char *s = md->signature;
		char buf[256];
		for (j=0; s[j]; j++) {
			switch(s[j]) {
			case 's': args[n_args++]=rb_cSymbol;  break;
			case 'i': args[n_args++]=rb_cInteger; break;
			case 'p': args[n_args++]=rb_cData;    break;
			case 'l': args[n_args++]=rb_cArray;   break;
			case '+':
				while(n_args<16) {
					args[n_args]=args[n_args-1];
					n_args++;}
				n_args = -1;
				break;
			case ';': n_args = -1; break;
			default: assert(0);
			}
		}
		n_args=-1; /* yes, really. sorry. */
		if (md->winlet>=0) {
			sprintf(buf,"_%d_%s",md->winlet,md->selector);
		} else {
			if (strcmp(md->selector,"init")==0) {
				sprintf(buf,"initialize");
			} else {
				sprintf(buf,"%s",md->selector);
			}
		}
		rb_define_method($,buf,(VALUE(*)())md->method,n_args);
	}
}
/*
char *FObject_to_s(FObject *$) {
	char *buf = NEW2(char,256);
	sprintf_vars(buf,
		$->argc,
		$->argv);
	return buf;
}
*/

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

/* ---------------------------------------------------------------- */

/* Ruby's entrypoint. */
void Init_gridflow (void) {
	gf_init();
}

