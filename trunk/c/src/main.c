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
Dict *gf_alarm_set = 0;
Dict *gf_alloc_set = 0;
fts_alarm_t *gf_alarm = 0;

FILE *whine_f;

/* **************************************************************** */
/* The setup part */

#define DECL_SYM2(_sym_) \
	Symbol sym_##_sym_ = (Symbol) 0xDeadBeef;

DECL_SYM2(grid_begin)
DECL_SYM2(grid_flow)
DECL_SYM2(grid_flow2)
DECL_SYM2(grid_end)

#define STARTUP_LIST(_begin_,_end_) \
	_begin_## operator       _end_ \
	_begin_## grid_basic     _end_ \
	_begin_## grid_extra     _end_ \
	_begin_## io             _end_ \
	_begin_## gridflow       _end_

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

void gf_alarm_handler (fts_alarm_t *foo, void *obj);
void gridflow_module_init (void) {
	disable_signal_handlers();
	srandom(time(0));

#ifdef MAKE_TMP_LOG
	whine_f = fopen("/tmp/gridflow.log","w");
	if (!whine_f) whine_f = fopen("/dev/null","w");
#endif

	post("Welcome to GridFlow " GF_VERSION "\n");
	post("Compiled on: " GF_COMPILE_TIME "\n");
	post("--- GridFlow startup: begin ---\n");

	#define DEF_SYM(_sym_) \
		sym_##_sym_ = fts_new_symbol(#_sym_);

	DEF_SYM(grid_begin);
	DEF_SYM(grid_flow);
	DEF_SYM(grid_flow2);
	DEF_SYM(grid_end);

	gf_object_set = Dict_new(0);
	gf_alarm_set  = Dict_new(0);
	gf_alarm = fts_alarm_new(fts_sched_get_clock(), gf_alarm_handler, 0);

	/* run startup of every source file */
	STARTUP_LIST(startup_,();)

	post("--- GridFlow startup: end ---\n");

	gf_alarm_handler(0,0); /* bootstrap the event loop */

	gf_alloc_set  = Dict_new(0);
}

/* this is the entry point for all of the above */

fts_module_t gridflow_module = {
	"video",
	"GridFlow: streamed n-dimensional arrays; video; etc.",
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

/* to help find uninitialized values */
void *qalloc(size_t n, const char *file, int line) {
	long *data = (long *) qalloc2(n,file,line);
	int i;
	int nn = (int) n/4;
	#ifndef NO_DEADBEEF
		for (i=0; i<nn; i++) data[i] = 0xDEADBEEF;
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
		Dict_put(gf_alloc_set,data,al);
	}
#endif
	return data;
}

void *qrealloc(void *data, int n) {
	void *data2 = realloc(data,n);
#ifdef MAKE_LEAK_DUMP
	if (gf_alloc_set) {
		void *a = Dict_get(gf_alloc_set,data);
		Dict_del(gf_alloc_set,data);
		Dict_put(gf_alloc_set,data2,a);
	}
#endif
	return data2;
}

/* to help find dangling references */
void qfree(void *data) {
	assert(data);
#ifdef MAKE_LEAK_DUMP
	if (gf_alloc_set) {
		void *a = Dict_get(gf_alloc_set,data);
		if (a) free(a);
		Dict_del(gf_alloc_set,data);
	}
#endif
	{
		int n=8;
		data = realloc(data,n);
		{
			long *data2 = (long *) data;
			int i;
			int nn = (int) n/4;
#ifndef NO_DEADBEEF
			for (i=0; i<nn; i++) data2[i] = 0xFADEDF00;
#endif
		}
	}
	free(data);
}

void qdump$1(void *obj, void *k, void *v) {
	AllocTrace *al = (AllocTrace *)v;
	whine("warning: %d bytes leak allocated at file %s line %d",
		al->n,al->file,al->line);
}

void qdump(void) {
	whine("checking for memory leaks...");
	Dict_each(gf_alloc_set,qdump$1,0);
	if (Dict_size(gf_alloc_set)==0) {
		whine("no leaks (yet)");
	}
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
			fts_new_symbol(md->selector), md->method,
			n_args, args, min_args == -1 ? n_args : min_args);
	}
}

int gf_file_open(const char *name, int mode) {
	return fts_file_open(name,mode==4?"r":mode==2?"w":"");
/*	return open(name,mode); */
}

FILE *gf_file_fopen(const char *name, int mode) {
	int fd = gf_file_open(name,mode);
	if (fd<0) return 0;
	return fdopen(fd,mode==4?"r":mode==2?"w":"");
}

/* **************************************************************** */
/* [rtmetro] */

static fts_alarm_t *rtmetro_alarm = 0;

typedef struct RtMetro {
	GridObject_FIELDS; /* yes, i know, it doesn't do grids */
	int ms; /* millisecond time */
	int on;
} RtMetro;

METHOD(RtMetro,int) {
	$->on = !! GET(0,int,0);
	whine("on = %d",$->on);
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

CLASS(RtMetro,
	DECL2(RtMetro, 0,int,   int,   "i"),
	DECL2(RtMetro, 1,int,   rint,  "i"),
	DECL2(RtMetro,-1,init,  init,  "s"),
	DECL2(RtMetro,-1,delete,delete,""))
{
	fts_class_init(class, sizeof(RtMetro), 2, 1, 0);
	define_many_methods(class,ARRAY(RtMetro_methods));
	return fts_Success;
}

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
	int i;
	Dict_each(os,profiler_reset$1,0);
}

static int by_profiler_cumul(void **a, void **b) {
	uint64 apc = (*(const GridObject **)a)->profiler_cumul;
	uint64 bpc = (*(const GridObject **)b)->profiler_cumul;
	return apc>bpc ? -1 : apc<bpc ? +1 : 0;
}

static void profiler_dump$1(void*d,void*k,void*v) {
	List_push((List *)d,k);
}

METHOD(GFGlobal,profiler_dump) {
        /* if you blow 256 chars it's your own fault */
	char buf[256];

	Dict *os = gf_object_set;
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
		sprintf_atoms(buf,o->o.argc,o->o.argv);
		whine("%20lld %2d.%04d %08x [%s]\n",
			o->profiler_cumul,
			ppm/10000,
			ppm%10000,
			o,
			buf);
		/* fts_symbol_name(fts_get_class_name(o->o.head.cl)) */
	}
	whine("--------------------------------");
}

METHOD(GFGlobal,init) {
	GridObject_init((GridObject *)$);
}

METHOD(GFGlobal,delete) {
	GridObject_delete((GridObject *)$);
}

CLASS(GFGlobal,
	DECL(GFGlobal,-1,init,          "s"),
	DECL(GFGlobal,-1,delete,        ""),
	DECL(GFGlobal, 0,profiler_reset,""),
	DECL(GFGlobal, 0,profiler_dump, ""))
{
	fts_class_init(class, sizeof(GFGlobal), 1, 1, 0);
	define_many_methods(class,ARRAY(GFGlobal_methods));
	return fts_Success;
}

#define INSTALL(_sym_,_name_) \
	fts_class_install(fts_new_symbol(_sym_),_name_##_class_init)

void gf_alarm_handler$1 (void *foo, void *o, void(*callback)(void*o)) {
	callback(o);
}

void gf_alarm_handler (fts_alarm_t *foo, void *obj) {
	Dict_each(gf_alarm_set,
		(void(*)(void*,void*,void*))gf_alarm_handler$1,0);
	fts_alarm_set_delay(gf_alarm, 75.0);
	fts_alarm_arm(gf_alarm);
}

void startup_gridflow (void) {
	INSTALL("rtmetro", RtMetro);
	INSTALL("@global", GFGlobal);
}
