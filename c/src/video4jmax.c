/*
	$Id$

	Video4jmax
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
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>

#include "grid.h"

/* **************************************************************** */
/* The setup part */

#define DECL_SYM2(_sym_) \
	fts_symbol_t sym_##_sym_ = (fts_symbol_t) 0xDeadBeef;

DECL_SYM2(open)
DECL_SYM2(close)
DECL_SYM2(reset)
DECL_SYM2(autodraw)
DECL_SYM2(grid_begin)
DECL_SYM2(grid_flow)
DECL_SYM2(grid_end)

void grid_basic_config (void);
void VideoOut_config (void);
void VideoOutFile_config (void);
void VideoInFile_config (void);

void video4jmax_module_init (void) {
	post("Welcome to Video4jmax !\n");
	post("Version: " VIDEO4JMAX_VERSION "\n");
	post("Compiled on: " VIDEO4JMAX_COMPILE_TIME "\n");
	post("--- Video4jmax startup: begin ---\n");

	#define DEF_SYM(_sym_) \
		sym_##_sym_ = fts_new_symbol(#_sym_);

	DEF_SYM(open);
	DEF_SYM(close);
	DEF_SYM(reset);
	DEF_SYM(autodraw);
	DEF_SYM(grid_begin);
	DEF_SYM(grid_flow);
	DEF_SYM(grid_end);

	grid_basic_config();
	VideoOut_config();
	VideoOutFile_config();
	VideoInFile_config();
	post("--- Video4jmax startup: end ---\n");

	/*
		this is for debugging only and should be turned off in
		the normal distribution. this code ensures that crashes are
		not trapped by jmax itself so i can have something to inspect.
	*/
	if (1) {
		static int signals[] = {
			SIGBUS, SIGSEGV, SIGTERM,
			SIGHUP, SIGINT, SIGQUIT, SIGABRT,
		};
		int i;
		for(i=0; i<COUNT(signals); i++) {
			signal(signals[i],SIG_DFL);
		}
	}
}

/* this is the entry point for all of the above */

fts_module_t video4jmax_module = {
	"video",
	"Video4jmax: video / streamed array classes",
	video4jmax_module_init, 0, 0};

/* **************************************************************** */
/* Procs of somewhat general utility */

// returns the highest bit set in a word, or -1 if none
int high_bit(unsigned long n) {
	int i;
	for (i=31; ((n & 0x80000000) == 0) && i>=0; i--) n <<= 1;
	return i;
}

// returns the lowest bit set in a word, or -1 if none
int low_bit(unsigned long n) {
	int i=0;
	n = (~n+1)&n;
	return high_bit(n);
}

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
			if (high_bit(format_count)-low_bit(format_count) < 3)
			post("[too many similar posts. this is # %d]\n",format_count);
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
		post("[whine] %s%.*s",post_s,post_s[length-1]!='\n',"\n");
	}
}

/* to help find uninitialized values */
void *qalloc(size_t n) {
	long *data = (long *) malloc(n);
	int i;
	int nn = (int) n/4;
	#ifndef NO_DEADBEEF
		for (i=0; i<nn; i++) data[i] = 0xDEADBEEF;
	#endif
	return data;	
}

/* to help find dangling references */
void qfree(void *data, size_t n) {
	long *data2 = (long *) data;
	int i;
	int nn = (int) n/4;
	#ifndef NO_DEADBEEF
		for (i=0; i<nn; i++) data2[i] = 0xFADEDF00;
	#endif
}

void define_many_methods(fts_class_t *class, int n, MethodDecl *methods) {
	int i;
	for (i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		fts_method_define_optargs(class,
			md->winlet == -1 ? fts_SystemInlet : md->winlet,
			md->selector, md->method,
			md->n_args, md->args,
			md->min_args == -1 ? md->n_args : md->min_args);
	}
}

int v4j_file_open(const char *name, int mode) {
	return fts_file_open(name,mode==4?"r":mode==2?"w":"");
/*	return open(name,mode); */
}

FILE *v4j_file_fopen(const char *name, int mode) {
	int fd = v4j_file_open(name,mode);
	if (fd<0) return 0;
	return fdopen(fd,mode==4?"r":mode==2?"w":"");
}

