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

/* this file is for allowing GridFlow to be compiled without jMax. */

#include "grid.h"
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>

typedef struct symbol_entry_t {
	const char *s;
	fts_class_t *c;
} symbol_entry_t;

typedef struct Dest {
	fts_object_t *obj;
	int inlet;
} Dest;

typedef struct MethodDecl2 {
	int winlet;
	Symbol selector;
	FTSMethod method;
	const char *signature;
} MethodDecl2;

static symbol_entry_t *symbols;
static int symbols_n = 0;

/* **************************************************************** */
/* Symbol */

Symbol Symbol_new (const char *s) {
	int i;
	if (!symbols) symbols = NEW(symbol_entry_t,1024);
	for (i=0; i<symbols_n; i++) {
		if (strcmp(symbols[i].s,s)==0) return i;
	}
	if (symbols_n >= 1024) { assert(0); }
	symbols[symbols_n].s = strdup(s);
	symbols[symbols_n].c = 0;
	return symbols_n++;
}

const char *Symbol_name(Symbol sym) { return symbols[sym].s; }

/* **************************************************************** */
/* Var */

bool Var_has_int(   const Var *$) {return $->type == fts_t_int;}
bool Var_has_float( const Var *$) {return $->type == fts_t_float;}
bool Var_has_symbol(const Var *$) {return $->type == fts_t_symbol;}
bool Var_has_ptr(   const Var *$) {return $->type == fts_t_ptr;}

int Var_get_int(   const Var *$) {assert(Var_has_int($)); return $->v.i;}
float Var_get_float( const Var *$) {assert(Var_has_float($)); return $->v.f;}
Symbol Var_get_symbol(const Var *$) {assert(Var_has_symbol($)); return $->v.s;}
void *Var_get_ptr( const Var *$) {assert(Var_has_ptr($)); return $->v.p;}

void Var_put_int(   Var *$, int v) {    $->type = fts_t_int;    $->v.i = v;}
void Var_put_symbol(Var *$, Symbol v) { $->type = fts_t_symbol; $->v.s = v;}
void Var_put_ptr(   Var *$, void *v) {  $->type = fts_t_ptr;    $->v.p = v;}

void sprintf_vars(char *buf, int ac, Var *at) {
	int i;
	for (i=0; i<ac; i++) {
		if (Var_has_int(at+i)) {
			buf += sprintf(buf,"%d",Var_get_int(at+i));
		} else if (Var_has_symbol(at+i)) {
			buf += sprintf(buf,"%s",Symbol_name(Var_get_symbol(at+i)));
		} else {
			buf += sprintf(buf,"<%s>",Symbol_name(at[i].type));
		}
		*buf++ = i==ac-1 ? 0 : ' '; /* separate by space */
	}
}

/* **************************************************************** */
/* Class */

void fts_class_init(fts_class_t *class, int object_size, int n_inlets, int
n_outlets, void *user_data) {
	int i;
	class->object_size = object_size;
	class->n_inlets = n_inlets;
	class->n_outlets = n_outlets;
	class->method_table = NEW(Dict *,n_inlets+1);
	for (i=0; i<n_inlets+1; i++) class->method_table[i] = Dict_new(0);
	class->user_data = user_data;
}

void fts_class_install(Symbol sym,
fts_status_t (*p)(fts_class_t *class, int ac, const Var *at)) {
	fts_class_t *$ = NEW(fts_class_t,1);
	p($,0,0);
	symbols[sym].c = $;
	$->name = sym;
}

Symbol fts_get_class_name(fts_class_t *class) { return class->name; }

void fts_method_define_optargs(fts_class_t *$, int winlet, Symbol
selector, fts_method_t method, int n_args, fts_type_t *args, int min_args) {
	MethodDecl2 *md = NEW(MethodDecl2,1);
	md->winlet = winlet;
	md->selector = selector;
	md->method = method;
	md->signature = "?";
/*
	md->n_args = n_args;
	md->args = NEW(fts_type_t,n_args?n_args:1);
	memcpy(md->args,args,n_args*sizeof(fts_type_t));
	md->min_args = min_args;
*/
	Dict_put($->method_table[winlet+1],(void *)selector,md);
}

/* **************************************************************** */
/* Object */

void fts_object_set_error(fts_object_t *o, const char *s, ...) {
	char buf[256];
	va_list rest;
	va_start(rest,s);
	vsnprintf(buf,sizeof(buf),s,rest);
	if (o->error) FREE(o->error);
	o->error = strdup(buf);
	va_end(rest);
}

fts_object_t *fts_object_new(int ac, Var *at) {
	Symbol classname = Var_get_symbol(at);
	fts_class_t *class = symbols[classname].c;
	if (!class) {
		printf("ERROR: class not found: %s\n",Symbol_name(classname));
		exit(1);
	}
	return fts_object_new2(class,ac,at);
}

fts_object_t *fts_object_new2(fts_class_t *class, int ac, Var *at) {
	int i;
	fts_object_t *$;
	assert(class->object_size > 0);
	$ = (fts_object_t *)NEW(char,class->object_size);
	$->head.cl = class;
	$->argc = ac;
	$->argv = NEW(Var,ac?ac:1);
	memcpy($->argv,at,ac*sizeof(Var));
	$->error = 0;
	$->outlets = NEW(List *, class->n_outlets);
	for (i=0; i<class->n_outlets; i++) {
		$->outlets[i] = List_new(0);
	}
	{
		Var a[10];
		memcpy(a,at,ac*sizeof(Var));
		Var_put_symbol(a+0,SYM(init));
		fts_send($,-1,SYM(init),ac,a);
		if ($->error) {
			printf("ERROR: %s\n",$->error);
			exit(1);
		}
	}
	return $;
}

void fts_object_delete(fts_object_t *$) {
	int i,j;
	fts_send3($,-1,"delete");
	for (i=0; i<$->head.cl->n_outlets; i++) {
		List *l = $->outlets[i];
		for (j=0; j<List_size(l); j++) {
			Dest *d = List_get(l,j);
			FREE(d);
		}
		FREE(l);
	}
	FREE($->outlets);
	FREE($->error);
	FREE($->argv);
	FREE($);
}

void fts_send2(fts_object_t *o, int inlet, int ac, const Var *at) {
	Symbol sel;
	assert(ac>0);
	if (Var_has_symbol(at)) {
		sel = Var_get_symbol(at);
		fts_send(o,inlet,sel,ac-1,at+1);
	} else {
		fts_send(o,inlet,SYM(list),ac,at);
	}
}

void fts_send(fts_object_t *o, int inlet, Symbol sel, int ac, const Var *at) {
	Dict *d = o->head.cl->method_table[inlet+1];
	MethodDecl *md = (MethodDecl *) Dict_get(d,(void *)sel);
	if (md) {
/*		printf("object %08lx inlet %d selector %s argc
		%d\n",(uint32)o,inlet,Symbol_name(sel),ac); */
		md->method(o,inlet,sel,ac,at);
	} else {
		printf("unknown method `%s'\n",Symbol_name(sel));
	}
}

void fts_connect(fts_object_t *oo, int outlet, fts_object_t *oi, int inlet) {
	Dest *d = NEW(Dest,1);
	d->obj = oi;
	d->inlet = inlet;
	List_push(oo->outlets[outlet],d);
}

void Object_send_thru(fts_object_t *o, int outlet, Symbol sel,
int ac, const Var *at) {
	List *l = o->outlets[outlet];
	int i;
	for (i=0; i<List_size(l); i++) {
		Dest *d = (Dest *)List_get(l,i);
		fts_send(d->obj,d->inlet,sel,ac,at);
	}
}

/* **************************************************************** */
/* Clock, Timer */

static fts_clock_t fts_main_clock = {};
static List *timers = 0;

fts_clock_t *fts_sched_get_clock(void) {
	return &fts_main_clock;
}

Timer *Timer_new(fts_clock_t *clock,
void (*f)(Timer *, void *), void *data) {
	Timer *$ = NEW(Timer,1);
	$->clock = clock;
	$->f = f;
	$->data = data;
	$->armed = false;
	return $;
}

void Timer_set_delay(Timer *$, float delay) {
	struct timeval tv;
	gettimeofday(&tv,0);
	$->time = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0 + delay;
}

void Timer_arm(Timer *$) {
	if (!$->armed) {
		$->armed = true;
		List_push(timers,$);
	}
}

void Timer_loop(void) {
	struct timeval tv;
	double time;
	int i;
	while (1) {
		int delay = 1000000;
		gettimeofday(&tv,0);
/*		printf("timers size = %d\n",List_size(timers)); */
		time = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
		for (i=0; i<List_size(timers); i++) {
			Timer *a = List_get(timers,i);
			int delay2 = (a->time - time) * 1000;
			if (delay>delay2) delay=delay2;
		}
/*		printf("delay = %d\n",delay); */
		if (delay>0) usleep(delay);
		for (i=0; i<List_size(timers);) {
			Timer *a = List_get(timers,i);
			if (a->time <= time) {
				a->armed = false;
				List_put(timers,i,List_get(timers,List_size(timers)-1));
				List_pop(timers);
				a->f(a,a->data);
			} else i++;
		}
	}	
}

/* **************************************************************** */

int strsplit(char *victim, int max, char **witnesses) {
	int n=0;
	for (;;) {
		*witnesses++ = victim;
		n++;   
		if (n==max) return n;
		while (*victim != ' ') {
			if (!*victim) return n;
			victim++;
		}
		*victim++ = 0;
	}
}

static bool isnumber(const char *s) {
	return isdigit(*s) ||
		((*s=='+' || *s=='-') && isdigit(s[1]));}

int silly_parse(const char *s, Var *a) {
	char *b = strdup(s);
	char *p[10];
	int i, n = strsplit(b,10,p);
	for (i=0; i<n; i++) {
		if (isnumber(p[i])) {
			Var_put_int(a+i,atoi(p[i]));
			//printf("%i: '%s' is int\n",i,p[i]);
		} else {
			Var_put_symbol(a+i,Symbol_new(p[i]));
			//printf("%i: '%s' is symbol\n",i,p[i]);
		}
	}
	FREE(b);
	return n;
}

fts_object_t *fts_object_new3(const char *foo) {
	Var a[10];
	int n = silly_parse(foo,a);
	return fts_object_new(n,a);
}

void fts_send3(fts_object_t *o, int woutlet, const char *foo) {
	Var a[10];
	int n = silly_parse(foo,a);
	return fts_send2(o,woutlet,n,a);
}

/* **************************************************************** */

int fts_file_open(const char *name, const char *mode) {
	if (strcmp(mode,"r")==0) {
		return open(name,0,O_RDONLY);
	} else if (strcmp(mode,"w")==0) {
		return open(name,0,O_CREAT|O_TRUNC|O_WRONLY);
	} else {
		return -1;
	}
}

extern fts_module_t gridflow_module;

int gridflow_init_standalone (void) {
	timers = List_new(0);
	fprintf(stdout,"< gridflow_init_standalone\n");

	Symbol_new("symbol");
	Symbol_new("int");
	Symbol_new("float");
	Symbol_new("list");
	Symbol_new("ptr");
	Symbol_new("init");
	Symbol_new("delete");
	Symbol_new("bang");
	Symbol_new("set");

	gridflow_module.foo3();
	printf("> gridflow_init_standalone\n");
	return true;
}
