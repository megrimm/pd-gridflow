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

#ifndef __BRIDGE_NONE_H
#define __BRIDGE_NONE_H
#include "lang.h"

typedef int fts_status_t;
#define fts_Success 0

typedef long Symbol;

/* hack */
static Symbol
	fts_t_symbol=0,
	fts_t_int=1, fts_s_int=1,
	fts_t_float=2,
	fts_t_list=3, fts_s_list=3,
	fts_t_ptr=4,
	fts_s_init=5,
	fts_s_delete=6,
	fts_s_bang=7,
	fts_s_set=8;

typedef Symbol fts_type_t;

typedef struct Var {
	fts_type_t type;
	union {
		int i;
		float f;
		Symbol s;
		void *p;
	} v;
} Var;

typedef struct FObject FObject;

typedef void (*fts_method_t)(FObject *,int,Symbol,int,const Var *);

typedef struct fts_class_t {
	Symbol name;
	int object_size;
	int n_inlets;
	int n_outlets;
	Dict **method_table; /* by inlet */
	void *user_data;
} fts_class_t;

struct FObject {
	struct {
		fts_class_t *cl;
	} head;
	int argc;
	Var *argv;
	char *error;
	List **outlets;
};

typedef struct fts_module_t {
	const char *foo1;
	const char *foo2;
	void (*foo3)(void);
	int foo4, foo5;
} fts_module_t;

typedef struct Timer {
	void (*f)(struct Timer *$, void *data);
	void *data;
	double time;
	int armed;
} Timer;

#define post printf
#define fts_SystemInlet (-1)

fts_status_t fts_method_define_optargs(fts_class_t *, int winlet, Symbol
selector, fts_method_t, int n_args, fts_type_t *args, int minargs);
int fts_file_open(const char *name, const char *mode);

bool Var_has_int(const Var *);
bool Var_has_float(const Var *);
bool Var_has_symbol(const Var *);
bool Var_has_ptr(const Var *);

int Var_get_int(const Var *);
float Var_get_float(const Var *);
Symbol Var_get_symbol(const Var *);
void *Var_get_ptr(const Var *);

void Var_put_int(Var *, int);
void Var_put_float(Var *, float);
void Var_put_symbol(Var *, Symbol);
void Var_put_ptr(Var *, void *);

/*
#define fts_get_int_arg(AC, AT, N, DEF) \
((N) < (AC) ? (fts_is_int(&(AT)[N]) ? fts_get_int(&(AT)[N]) : \
              (fts_is_float(&(AT)[N]) ? (int) fts_get_float(&(AT)[N]) : (DEF))) : (DEF))

#define fts_get_symbol_arg(AC, AT, N, DEF) ((N) < (AC) ? fts_get_symbol(&(AT)[N]) : (DEF))
#define fts_get_ptr_arg(AC, AT, N, DEF)    ((N) < (AC) ? fts_get_ptr(&(AT)[N]) : (DEF))
*/

fts_status_t fts_class_init(fts_class_t *class, int object_size, int n_inlets, int n_outlets, void *user_data);

Symbol fts_get_class_name(fts_class_t *class);

void sprintf_vars(char *buf, int ac, Var *at);

fts_status_t fts_class_install(Symbol sym,
	fts_status_t (*p)(fts_class_t *class, int ac, const Var *at));

void Object_send_thru(FObject *o, int woutlet, Symbol selector, int ac, const Var *at);

void fts_object_set_error(FObject *o, const char *s, ...);
Timer *Timer_new(void (*f)(Timer *foo, void *), void *);
void Timer_set_delay(Timer *, float);
void Timer_arm(Timer *);
void Timer_loop(void);

int gf_init_standalone(void);

/* **************************************************************** */

FObject *fts_object_new(int ac, Var *at);
FObject *fts_object_new2(fts_class_t *class, int ac, Var *at);
void fts_object_delete(FObject *$);
void fts_send2(FObject *o, int winlet, int ac, const Var *at);
void fts_send(FObject *o, int winlet, Symbol sel, int ac, const Var *at);
void fts_connect(FObject *oo, int woutlet, FObject *oi, int winlet);

int strsplit(char *victim, int max, char **witnesses);
int silly_parse(const char *s, Var *a);
FObject *fts_object_new3(const char *foo);
void fts_send3(FObject *o, int woutlet, const char *foo);

Symbol Symbol_new(const char *s);
const char *Symbol_name(Symbol sym);

#endif /* __BRIDGE_NONE_H */
