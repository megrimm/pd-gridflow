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

/* this file is for allowing video4jmax to be compiled without jMax. */

#include "../grid.h"
#include <string.h>
#include <fcntl.h>

typedef struct symbol_entry_t {
	const char *s;
	fts_class_t *c;
} symbol_entry_t;

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
	if (symbols_n >= 1024) assert(0);
	symbols[symbols_n].s = strdup(s);
	symbols[symbols_n].c = 0;
	return symbols_n++;
}

const char *fts_symbol_name(fts_symbol_t sym) { return symbols[sym].s; }

/* **************************************************************** */
/* Atom */

int fts_is_int(   const fts_atom_t *$) { return $->type == fts_t_int; }
int fts_is_float( const fts_atom_t *$) { return $->type == fts_t_float; }
int fts_is_symbol(const fts_atom_t *$) { return $->type == fts_t_symbol; }

int fts_get_int(   const fts_atom_t *$) { return $->v.i; }
int fts_get_float( const fts_atom_t *$) { return $->v.f; }
int fts_get_symbol(const fts_atom_t *$) { return $->v.s; }
void *fts_get_ptr( const fts_atom_t *$) { return $->v.p; }

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
	sprintf(buf,"(can't sprintf atoms yet)");
}

/* **************************************************************** */
/* Class */

void fts_class_init(fts_class_t *class, int object_size, int n_inlets, int
n_outlets, int stuff) {
	class->object_size = object_size;
	class->n_inlets = n_inlets;
	class->n_outlets = n_outlets;
	class->stuff = stuff;
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
selector, fts_method_t method, int n_args, fts_type_t *args, int minargs) {
	/* nothing for now */
}

/* **************************************************************** */
/* Object */

void fts_object_set_error(fts_object_t *o, const char *s, ...) {
	if (o->error) FREE(o);
	o->error = strdup(s);
}

fts_object_t *fts_object_new(fts_class_t *class) {
	fts_object_t *$ = (fts_object_t *)qalloc(class->object_size);
	$->error = 0;
	return $;
}

void fts_outlet_send(fts_object_t *o, int woutlet, fts_symbol_t selector,
int ac, const fts_atom_t *at) {
	/* nothing for now */
}

/* **************************************************************** */
/* Clock, Alarm */

static fts_clock_t v4j_clock = {};

fts_clock_t *fts_sched_get_clock(void) {
	return &v4j_clock;
}

fts_alarm_t *fts_alarm_new(fts_clock_t *clock,
void (*f)(fts_alarm_t *, void *), void *data) {
	fts_alarm_t *$ = NEW(fts_alarm_t,1);
	$->clock = clock;
	$->f = f;
	return $;
}

void fts_alarm_set_delay(fts_alarm_t *$, float delay) {
	$->delay = delay;
}

void fts_alarm_arm(fts_alarm_t *$) {
	$->armed = true;
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

extern fts_module_t video4jmax_module;

int video4jmax_init_standalone (void) {
	fprintf(stdout,"< video4jmax_init_standalone\n");
	fts_new_symbol("symbol");
	fts_new_symbol("int");
	fts_new_symbol("list");
	fts_new_symbol("ptr");
	fts_new_symbol("init");
	fts_new_symbol("delete");
	fts_new_symbol("set");

	video4jmax_module.foo3();
	printf("> video4jmax_init_standalone\n");
	return true;
}
