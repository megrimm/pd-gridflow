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

static symbol_entry_t *symbols;
static int symbols_n = 0;

/* **************************************************************** */
/* Symbol */

fts_symbol_t fts_new_symbol (const char *s) {
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

const char *fts_symbol_name(fts_symbol_t sym) { return symbols[sym].s; }

/* **************************************************************** */
/* Atom */

int fts_is_int(   const fts_atom_t *$) {return $->type == fts_t_int;}
int fts_is_float( const fts_atom_t *$) {return $->type == fts_t_float;}
int fts_is_symbol(const fts_atom_t *$) {return $->type == fts_t_symbol;}
int fts_is_ptr(   const fts_atom_t *$) {return $->type == fts_t_ptr;}

int fts_get_int(   const fts_atom_t *$) {assert(fts_is_int($)); return $->v.i;}
int fts_get_float( const fts_atom_t *$) {assert(fts_is_float($)); return $->v.f;}
int fts_get_symbol(const fts_atom_t *$) {assert(fts_is_symbol($)); return $->v.s;}
void *fts_get_ptr( const fts_atom_t *$) {assert(fts_is_ptr($)); return $->v.p;}

void fts_set_int(   fts_atom_t *$, int v) {
	$->type = fts_t_int;
	$->v.i = v;}
void fts_set_symbol(fts_atom_t *$, fts_symbol_t v) {
	$->type = fts_t_symbol;
	$->v.s = v;}
void fts_set_ptr(   fts_atom_t *$, void *v) {
	$->type = fts_t_ptr;
	$->v.p = v;}

void sprintf_atoms(char *buf, int ac, fts_atom_t *at) {
	int i;
	for (i=0; i<ac; i++) {
		if (fts_is_int(at+i)) {
			buf += sprintf(buf,"%d",fts_get_int(at+i));
		} else if (fts_is_symbol(at+i)) {
			buf += sprintf(buf,"%s",fts_symbol_name(fts_get_symbol(at+i)));
		} else {
			buf += sprintf(buf,"<%s>",fts_symbol_name(at[i].type));
		}
		*buf++ = i==ac-1 ? 0 : ' '; /* separate by space */
	}
}

/* **************************************************************** */
/* Class */

void fts_class_init(fts_class_t *class, int object_size, int n_inlets, int
n_outlets, int stuff) {
	int i;
	class->object_size = object_size;
	class->n_inlets = n_inlets;
	class->n_outlets = n_outlets;
	class->stuff = stuff;
	class->method_table = NEW(Dict *,n_inlets+1);
	for (i=0; i<n_inlets+1; i++) class->method_table[i] = Dict_new(0);
}

void fts_class_install(fts_symbol_t sym,
fts_status_t (*p)(fts_class_t *class, int ac, const fts_atom_t *at)) {
	fts_class_t *$ = NEW(fts_class_t,1);
	p($,0,0);
	symbols[sym].c = $;
	$->name = sym;
}

fts_symbol_t fts_get_class_name(fts_class_t *class) { return class->name; }

void fts_method_define_optargs(fts_class_t *$, int winlet, fts_symbol_t
selector, fts_method_t method, int n_args, fts_type_t *args, int min_args) {
	MethodDecl *md = NEW(MethodDecl,1);
	md->winlet = winlet;
	md->selector = selector;
	md->method = method;
	md->n_args = n_args;
	md->args = NEW(fts_type_t,n_args?n_args:1);
	memcpy(md->args,args,n_args*sizeof(fts_type_t));
	md->min_args = min_args;
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

fts_object_t *fts_object_new(int ac, fts_atom_t *at) {
	fts_symbol_t classname = fts_get_symbol(at);
	fts_class_t *class = symbols[classname].c;
	if (!class) {
		printf("ERROR: class not found: %s\n",fts_symbol_name(classname));
		exit(1);
	}
	return fts_object_new2(class,ac,at);
}

fts_object_t *fts_object_new2(fts_class_t *class, int ac, fts_atom_t *at) {
	int i;
	fts_object_t *$;
	assert(class->object_size > 0);
	$ = (fts_object_t *)NEW(char,class->object_size);
	$->head.cl = class;
	$->argc = ac;
	$->argv = NEW(fts_atom_t,ac?ac:1);
	memcpy($->argv,at,ac*sizeof(fts_atom_t));
	$->error = 0;
	$->outlets = NEW(List *, class->n_outlets);
	for (i=0; i<class->n_outlets; i++) {
		$->outlets[i] = List_new(0);
	}
	{
		fts_atom_t a[10];
		memcpy(a,at,ac*sizeof(fts_atom_t));
		fts_set_symbol(a+0,SYM(init));
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

void fts_send2(fts_object_t *o, int inlet, int ac, const fts_atom_t *at) {
	fts_symbol_t sel;
	assert(ac>0);
	if (fts_is_symbol(at)) {
		sel = fts_get_symbol(at);
		fts_send(o,inlet,sel,ac-1,at+1);
	} else {
		fts_send(o,inlet,SYM(list),ac,at);
	}
}

void fts_send(fts_object_t *o, int inlet, fts_symbol_t sel, int ac, const fts_atom_t *at) {
	Dict *d = o->head.cl->method_table[inlet+1];
	MethodDecl *md = (MethodDecl *) Dict_get(d,(void *)sel);
	if (md) {
/*		printf("object %08lx inlet %d selector %s argc
		%d\n",(uint32)o,inlet,fts_symbol_name(sel),ac); */
		md->method(o,inlet,sel,ac,at);
	} else {
		printf("unknown method `%s'\n",fts_symbol_name(sel));
	}
}

void fts_connect(fts_object_t *oo, int outlet, fts_object_t *oi, int inlet) {
	Dest *d = NEW(Dest,1);
	d->obj = oi;
	d->inlet = inlet;
	List_push(oo->outlets[outlet],d);
}

void fts_outlet_send(fts_object_t *o, int outlet, fts_symbol_t sel,
int ac, const fts_atom_t *at) {
	List *l = o->outlets[outlet];
	int i;
	for (i=0; i<List_size(l); i++) {
		Dest *d = (Dest *)List_get(l,i);
		fts_send(d->obj,d->inlet,sel,ac,at);
	}
}

/* **************************************************************** */
/* Clock, Alarm */

static fts_clock_t fts_main_clock = {};
static List *alarms = 0;

fts_clock_t *fts_sched_get_clock(void) {
	return &fts_main_clock;
}

fts_alarm_t *fts_alarm_new(fts_clock_t *clock,
void (*f)(fts_alarm_t *, void *), void *data) {
	fts_alarm_t *$ = NEW(fts_alarm_t,1);
	$->clock = clock;
	$->f = f;
	$->data = data;
	$->armed = false;
	return $;
}

void fts_alarm_set_delay(fts_alarm_t *$, float delay) {
	struct timeval tv;
	gettimeofday(&tv,0);
	$->time = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0 + delay;
}

void fts_alarm_arm(fts_alarm_t *$) {
	if (!$->armed) {
		$->armed = true;
		List_push(alarms,$);
	}
}

void fts_loop(void) {
	struct timeval tv;
	double time;
	int i;
	while (1) {
		int delay = 1000000;
		gettimeofday(&tv,0);
/*		printf("alarms size = %d\n",List_size(alarms)); */
		time = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
		for (i=0; i<List_size(alarms); i++) {
			fts_alarm_t *a = List_get(alarms,i);
			int delay2 = (a->time - time) * 1000;
			if (delay>delay2) delay=delay2;
		}
/*		printf("delay = %d\n",delay); */
		if (delay>0) usleep(delay);
		for (i=0; i<List_size(alarms);) {
			fts_alarm_t *a = List_get(alarms,i);
			if (a->time <= time) {
				a->armed = false;
				List_put(alarms,i,List_get(alarms,List_size(alarms)-1));
				List_pop(alarms);
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

int silly_parse(const char *s, fts_atom_t *a) {
	char *b = strdup(s);
	char *p[10];
	int i, n = strsplit(b,10,p);
	for (i=0; i<n; i++) {
		if (isnumber(p[i])) {
			fts_set_int(a+i,atoi(p[i]));
			//printf("%i: '%s' is int\n",i,p[i]);
		} else {
			fts_set_symbol(a+i,fts_new_symbol(p[i]));
			//printf("%i: '%s' is symbol\n",i,p[i]);
		}
	}
	FREE(b);
	return n;
}

fts_object_t *fts_object_new3(const char *foo) {
	fts_atom_t a[10];
	int n = silly_parse(foo,a);
	return fts_object_new(n,a);
}

void fts_send3(fts_object_t *o, int woutlet, const char *foo) {
	fts_atom_t a[10];
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
	alarms = List_new(0);
	fprintf(stdout,"< gridflow_init_standalone\n");
	fts_new_symbol("symbol");
	fts_new_symbol("int");
	fts_new_symbol("float");
	fts_new_symbol("list");
	fts_new_symbol("ptr");
	fts_new_symbol("init");
	fts_new_symbol("delete");
	fts_new_symbol("bang");
	fts_new_symbol("set");

	gridflow_module.foo3();
	printf("> gridflow_init_standalone\n");
	return true;
}
