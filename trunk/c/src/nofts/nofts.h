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

#ifndef __STANDALONE_H
#define __STANDALONE_H

typedef int fts_status_t;
#define fts_Success 1

typedef enum fts_symbol_t {
	fts_t_symbol,
	fts_t_int, fts_s_int = fts_t_int,
	fts_t_float,
	fts_t_list, fts_s_list = fts_t_list,
	fts_t_ptr,
	fts_s_init,
	fts_s_delete,
	fts_s_bang,
	fts_s_set,
} fts_symbol_t;

typedef fts_symbol_t fts_type_t;

typedef struct fts_atom_t {
	fts_type_t type;
	union {
		int i;
		float f;
		fts_symbol_t s;
		void *p;
	} v;
} fts_atom_t;

typedef struct List List; /* List ([0...n] -> void *) */

List *List_new(int size);
void List_push(List *$, void *v);
void *List_get(List *$, int i);
void List_put(List *$, int i, void *v);
int List_size(List *$);
void *List_pop(List *$);

typedef struct Dict Dict; /* Dictionary (Symbol -> void *) */
typedef struct DictEntry DictEntry;

Dict *Dict_new(void);
long Dict_hash(Dict *$, fts_symbol_t k);
DictEntry *Dict_has_key(Dict *$, fts_symbol_t k);
void *Dict_get(Dict *$, fts_symbol_t k);
void Dict_put(Dict *$, fts_symbol_t k, void *v);
void Dict_each(Dict *$, void (*proc)(void* data,fts_symbol_t k,void* v), void *data);

typedef struct fts_object_t fts_object_t;

typedef void (*fts_method_t)(fts_object_t *,int,fts_symbol_t,int,const fts_atom_t *);

typedef struct fts_class_t {
	fts_symbol_t name;
	int object_size;
	int n_inlets;
	int n_outlets;
	int stuff;
	Dict **method_table; /* by inlet */
} fts_class_t;

struct fts_object_t {
	struct {
		fts_class_t *cl;
	} head;
	int argc;
	fts_atom_t *argv;
	char *error;
	List **outlets;
};

typedef struct fts_module_t {
	const char *foo1;
	const char *foo2;
	void (*foo3)(void);
	int foo4, foo5;
} fts_module_t;

typedef struct fts_clock_t {
} fts_clock_t;

typedef struct fts_alarm_t {
	fts_clock_t *clock;
	void (*f)(struct fts_alarm_t *$, void *data);
	void *data;
	double time;
	int armed;
} fts_alarm_t;

#define post printf
#define fts_SystemInlet (-1)

fts_symbol_t fts_new_symbol(const char *s);
const char *fts_symbol_name(fts_symbol_t sym);

void fts_method_define_optargs(fts_class_t *, int winlet, fts_symbol_t
selector, fts_method_t, int n_args, fts_type_t *args, int minargs);
int fts_file_open(const char *name, const char *mode);

int fts_is_int(const fts_atom_t *);
int fts_is_float(const fts_atom_t *);
int fts_is_symbol(const fts_atom_t *);

int fts_get_int(const fts_atom_t *);
int fts_get_float(const fts_atom_t *);
int fts_get_symbol(const fts_atom_t *);
void *fts_get_ptr(const fts_atom_t *);

void fts_set_int(fts_atom_t *, int);
void fts_set_symbol(fts_atom_t *, fts_symbol_t);
void fts_set_ptr(fts_atom_t *, void *);

#define fts_get_int_arg(AC, AT, N, DEF) \
((N) < (AC) ? (fts_is_int(&(AT)[N]) ? fts_get_int(&(AT)[N]) : \
              (fts_is_float(&(AT)[N]) ? (int) fts_get_float(&(AT)[N]) : (DEF))) : (DEF))

#define fts_get_symbol_arg(AC, AT, N, DEF) ((N) < (AC) ? fts_get_symbol(&(AT)[N]) : (DEF))
#define fts_get_ptr_arg(AC, AT, N, DEF)    ((N) < (AC) ? fts_get_ptr(&(AT)[N]) : (DEF))

void fts_class_init(fts_class_t *class, int object_size, int n_inlets, int n_outlets, int stuff);

fts_symbol_t fts_get_class_name(fts_class_t *class);

void sprintf_atoms(char *buf, int ac, fts_atom_t *at);

void fts_class_install(fts_symbol_t sym,
	fts_status_t (*p)(fts_class_t *class, int ac, const fts_atom_t *at));

void fts_outlet_send(fts_object_t *o, int woutlet, fts_symbol_t selector, int ac, const fts_atom_t *at);

void fts_object_set_error(fts_object_t *o, const char *s, ...);
fts_clock_t *fts_sched_get_clock(void);
fts_alarm_t *fts_alarm_new(fts_clock_t *foo,
	void (*f)(fts_alarm_t *foo, void *), void *);
void fts_alarm_set_delay(fts_alarm_t *, float);
void fts_alarm_arm(fts_alarm_t *);

int video4jmax_init_standalone(void);

/* **************************************************************** */

fts_object_t *fts_object_new(int ac, fts_atom_t *at);
fts_object_t *fts_object_new2(fts_class_t *class, int ac, fts_atom_t *at);
void fts_send2(fts_object_t *o, int winlet, int ac, const fts_atom_t *at);
void fts_send(fts_object_t *o, int winlet, fts_symbol_t sel, int ac, const fts_atom_t *at);
void fts_connect(fts_object_t *oo, int woutlet, fts_object_t *oi, int winlet);
void fts_loop(void);

int strsplit(char *victim, int max, char **witnesses);
int silly_parse(const char *s, fts_atom_t *a);
fts_object_t *fts_object_new3(const char *foo);
void fts_send3(fts_object_t *o, int woutlet, const char *foo);


#endif /* __STANDALONE_H */
