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
	VALUE sym_##_sym_ = 0xDeadBeef;

DECL_SYM2(grid_begin)
DECL_SYM2(grid_flow)
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
	gf_install_bridge();
	disable_signal_handlers(); /* paranoid; help me with gdb */
	rb_eval_string(
		"begin\n"
			"require '" GF_INSTALL_DIR "/ruby/main.rb'\n"
			"require '/home/projects/gridflow/extra/eval_server.rb'\n"
			"$esm = EvalServerManager.new\n"
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
	whine("untrapped ruby exception occurred loading GridFlow");
	return Qnil;
}

void gf_ruby_init (void) {
	char *foo[] = {"/bin/false","/dev/null"};
	whine("starting Ruby...");
	ruby_init();

	rb_eval_string("module GridFlow; end");
	ruby_options(COUNT(foo),foo);
	rb_rescue(gf_ruby_init$1,0,gf_ruby_init$2,0);
}

void gf_init (void) {

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

	gf_ruby_init();

#define DEF_SYM(_sym_) \
	sym_##_sym_ = SYM(_sym_);

	DEF_SYM(grid_begin);
	DEF_SYM(grid_flow);
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

void gf_timer_handler$1 (void *foo, void *o, void(*callback)(void*o)) {
	callback(o);
}

void gf_timer_handler$2 (void *foo) {
//	rb_eval_string("STDERR.puts \"ruby-tick\"");
//	rb_eval_string("protect{$esm.tick}");
}

void gf_timer_handler (Timer *foo, void *obj) {
	uint64 now = RtMetro_now();
//	fprintf(stderr,"tick-start @ %llu\n",now);
	Dict_each(gf_timer_set,
		(void(*)(void*,void*,void*))gf_timer_handler$1,0);
	gf_timer_handler$2(0);
//	fprintf(stderr,"tick-end (%llu)\n", RtMetro_now()-now);
	Timer_set_delay(gf_timer, 100.0);
	Timer_arm(gf_timer);
}

void startup_gridflow (void) {
}
