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

#define MAKE_TMP_LOG
#define MAKE_LEAK_DUMP

const char *whine_header = "[whine] ";
Dict *gf_object_set = 0;
Dict *gf_timer_set = 0;
Timer *gf_timer = 0;

FILE *whine_f;

/* **************************************************************** */
/* The setup part */

#define DECL_SYM2(_sym_) \
	Symbol sym_##_sym_ = (Symbol) 0xDeadBeef;

DECL_SYM2(grid_begin)
DECL_SYM2(grid_flow)
DECL_SYM2(grid_flow2)
DECL_SYM2(grid_end)
DECL_SYM2(bang)
DECL_SYM2(int)
DECL_SYM2(list)

#define STARTUP_LIST(_begin_,_end_) \
	_begin_## number        _end_ \
	_begin_## flow_objects  _end_ \
	_begin_## grid          _end_ \
	_begin_## gridflow      _end_

/* declare startup of every source file */
STARTUP_LIST(void startup_,(void);)

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

void showenv(const char *s) {
	whine("%s = %s\n", s, getenv(s));
}

void gf_timer_handler (Timer *foo, void *obj);

VALUE gf_ruby_init$1 (void *foo) {
	rb_eval_string(
		"begin\n"
			"Thread.abort_on_exception = true\n"
			"require '/home/projects/gridflow/extra/DRbServer.rb\n"
/*
			"require '/home/projects/gridflow/extra/TkRubyListener.rb'\n"
			"$root = TkRoot.new { title 'GridFlow console' }\n"
			"def foo; STDERR.puts \"ruby-loop-tick\"; Tk.after(1000) { foo }; end; foo\n"
			"Thread.new {STDERR.puts \"TK-HELLO\"; Tk.mainloop}\n"
			"$listener = TkRubyListener.new($root,60,8)\n"
			"$listener.frame.pack\n"
*/
		"rescue Exception => e; STDERR.puts \"1: Ruby Exception: #{e} #{e.backtrace}\"\n"
		"end\n"
	);
	return Qnil;
}

VALUE gf_ruby_init$2 (void *foo) {
	whine("ruby exception occurred loading gridflow-console");
	return Qnil;
}

void gf_ruby_init (void) {
	char *foo[] = {"/bin/false","/dev/null"};
	whine("starting Ruby...");
	ruby_init();
	ruby_options(ARRAY(foo));
//	ruby_run();
	{
		VALUE foo = rb_eval_string("VERSION");
		whine("Ruby VERSION = %s",RSTRING(foo)->ptr);
	}
	rb_rescue(gf_ruby_init$1,0,gf_ruby_init$2,0);
}

void gridflow_module_init (void) {

	disable_signal_handlers();
	srandom(time(0));

#ifdef MAKE_TMP_LOG
	whine_f = fopen("/tmp/gridflow.log","w");
	if (!whine_f) whine_f = fopen("/dev/null","w");
#endif

	whine("Welcome to GridFlow " GF_VERSION);
	whine("Compiled on: " GF_COMPILE_TIME);
	whine("--- GridFlow startup: begin ---");

	showenv("LD_LIBRARY_PATH");
	showenv("LIBRARY_PATH");
	showenv("PATH");
	showenv("DISPLAY");

//	gf_ruby_init();

	#define DEF_SYM(_sym_) \
		sym_##_sym_ = Symbol_new(#_sym_);

	DEF_SYM(grid_begin);
	DEF_SYM(grid_flow);
	DEF_SYM(grid_flow2);
	DEF_SYM(grid_end);
	DEF_SYM(bang);
	DEF_SYM(int);
	DEF_SYM(list);

	gf_alloc_set  = Dict_new(0,0);
	gf_object_set = Dict_new(0,0);
	gf_timer_set  = Dict_new(0,0);

	/* run startup of every source file */
	STARTUP_LIST(startup_,();)

	gf_timer = Timer_new(gf_timer_handler, 0);
//	Dict_put(gf_timer_set,0,RtMetro_alarm);
	gf_timer_handler(0,0); /* bootstrap the event loop */

	whine("--- GridFlow startup: end ---");
}

/* this is the entry point for all of the above */

fts_module_t gridflow_module = {
	"video",
	"GridFlow: n-dimensional array streaming, pictures, video, etc.",
	gridflow_module_init, 0, 0};

/* **************************************************************** */
/* Procs of somewhat general utility */

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
void define_many_methods(fts_class_t *class, int n, MethodDecl *methods) {
	fts_type_t args[16];
	int i;
	for (i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		int j;
		int min_args=-1;
		int n_args=0;
		const char *s = md->signature;
		for (j=0; s[j]; j++) {
			switch(s[j]) {
			case 's': args[n_args++]=fts_t_symbol; break;
			case 'i': args[n_args++]=fts_t_int;    break;
			case 'p': args[n_args++]=fts_t_ptr;    break;
			case 'l': args[n_args++]=fts_t_list;   break;
			case '+': min_args = n_args;
				while(n_args<16) {
					args[n_args]=args[n_args-1];
					n_args++;}
				break;
			case ';': min_args = n_args; break;
			default: assert(0);
			}
		}
		fts_method_define_optargs(class,
			md->winlet == -1 ? fts_SystemInlet : md->winlet,
			Symbol_new(md->selector), md->method,
			n_args, args, min_args == -1 ? n_args : min_args);
	}
}

char *FObject_to_s(FObject *$) {
	char *buf = NEW2(char,256);
	sprintf_vars(buf,
/*		((fts_object_t)$->twin)->o.argc,
		((fts_object_t)$->twin)->o.argv);
*/
		$->argc,
		$->argv);
	return buf;
}

/* **************************************************************** */
/* [rtmetro] */

typedef struct RtMetro {
	GridObject_FIELDS; /* yes, i know, it doesn't do grids */
	int ms; /* millisecond time interval */
	int on;
	uint64 next_time; /* next time an event occurred */
	uint64 last;
} RtMetro;

static long long RtMetro_now(void) {
	struct timeval nowtv;
	gettimeofday(&nowtv,0);
	return nowtv.tv_sec * 1000000LL + nowtv.tv_usec;
}

static void RtMetro_alarm(RtMetro *$) {
	uint64 now = RtMetro_now();
	//whine("rtmetro alarm tick: %lld; next_time: %lld; now-last: %lld",now,$->next_time,now-$->last);
	if (now >= $->next_time) {
		//whine("rtmetro sending bang");
		Object_send_thru(OBJ($),0,sym_bang,0,0);
		/* $->next_time = now; */ /* jmax style, less realtime */
		$->next_time += 1000*$->ms;
	}
	$->last = now;
}

METHOD(RtMetro,int) {
	int oon = $->on;
	$->on = !! GET(0,int,0);
	whine("on = %d",$->on);
	if (oon && !$->on) {
		whine("deleting rtmetro alarm...");
		Dict_del(gf_timer_set,$);
	} else if (!oon && $->on) {
		whine("creating rtmetro alarm...");
		Dict_put(gf_timer_set,$,RtMetro_alarm);
		$->next_time = RtMetro_now();
	}
}

METHOD(RtMetro,rint) {
	$->ms = GET(0,int,0);
	whine("ms = %d",$->ms);
}

METHOD(RtMetro,init) {
	GridObject_init((GridObject *)$);
	$->ms = GET(1,int,0);
	$->on = 0;
	whine("ms = %d",$->ms);
	whine("on = %d",$->on);
}

METHOD(RtMetro,delete) {
	GridObject_delete((GridObject *)$);
}

GRCLASS(RtMetro,inlets:2,outlets:1,
LIST(),
/* outlet 0 not used for grids */
	DECL2(RtMetro, 0,int,   int,   "i"),
	DECL2(RtMetro, 1,int,   rint,  "i"),
	DECL2(RtMetro,-1,init,  init,  "si"),
	DECL2(RtMetro,-1,delete,delete,""))

/* **************************************************************** */
/* [@global] */

/* a dummy object that gives access to any stuff global to
   GridFlow.
*/
typedef struct GFGlobal {
	GridObject_FIELDS; /* yes, i know, it doesn't do grids */
} GFGlobal;

static void profiler_reset$1(void*d,void*k,void*v) {
	((GridObject *)k)->profiler_cumul = 0;
}

METHOD(GFGlobal,profiler_reset) {
	Dict *os = gf_object_set;
	Dict_each(os,profiler_reset$1,0);
}

static int by_profiler_cumul(void **a, void **b) {
	uint64 apc = (*(const GridObject **)a)->profiler_cumul;
	uint64 bpc = (*(const GridObject **)b)->profiler_cumul;
	return apc>bpc ? -1 : apc<bpc ? +1 : 0;
}

static void profiler_dump$1(void *d,void *k,void *v) {
	List_push((List *)d,k);
}

METHOD(GFGlobal,profiler_dump) {
	/* if you blow 256 chars it's your own fault */
	List *ol = List_new(0);

	uint64 total=0;
	int i;
	whine("--------------------------------");
	whine("         clock-ticks percent pointer  constructor");
	Dict_each(gf_object_set,profiler_dump$1,ol);
	List_sort(ol,by_profiler_cumul);
	for(i=0;i<List_size(ol);i++) {
		GridObject *o = List_get(ol,i);
		total += o->profiler_cumul;
	}
	if (total<1) total=1;
	for(i=0;i<List_size(ol);i++) {
		GridObject *o = List_get(ol,i);
		int ppm = o->profiler_cumul * 1000000 / total;
		char *buf = FObject_to_s(OBJ(o));
		whine("%20lld %2d.%04d %08x [%s]\n",
			o->profiler_cumul,
			ppm/10000,
			ppm%10000,
			o,
			buf);
		/* Symbol_new(fts_get_class_name(o->o.head.cl)) */
		FREE(buf);
	}
	whine("--------------------------------");
}

METHOD(GFGlobal,init) {
	GridObject_init((GridObject *)$);
}

METHOD(GFGlobal,delete) {
	GridObject_delete((GridObject *)$);
}

GRCLASS(GFGlobal,inlets:1,outlets:1,
LIST(),
/* outlet 0 not used for grids */
	DECL(GFGlobal,-1,init,          "s"),
	DECL(GFGlobal,-1,delete,        ""),
	DECL(GFGlobal, 0,profiler_reset,""),
	DECL(GFGlobal, 0,profiler_dump, ""))

#define INSTALL(_sym_,_name_) \
	fts_class_install(Symbol_new(_sym_),_name_##_class_init)

void gf_timer_handler$1 (void *foo, void *o, void(*callback)(void*o)) {
	callback(o);
}

void gf_timer_handler$2 (void *foo) {
//	rb_eval_string("STDERR.puts \"ruby-tick\"");
	rb_eval_string("begin Thread.pass; "
		"rescue Exception => e; STDERR.puts \"2: Ruby Exception: #{e} #{e.backtrace}\" end");
}

void gf_timer_handler (Timer *foo, void *obj) {
//	whine("tick-start");
	Dict_each(gf_timer_set,
		(void(*)(void*,void*,void*))gf_timer_handler$1,0);
//	gf_timer_handler$2(0);
//	whine("tick-end");
	Timer_set_delay(gf_timer, 100.0);
	Timer_arm(gf_timer);
}

void startup_gridflow (void) {
	INSTALL("rtmetro", RtMetro);
	INSTALL("@global", GFGlobal);
}
