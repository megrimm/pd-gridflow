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

#ifndef __MACROS_H
#define __MACROS_H
#include <assert.h>

/*
  This file defines a few macros of general use. The goal of these is
  to work more at the level of jmax's ideas instead of copy+pasting
  boilerplate code (and having to read that afterwards). In short, they
  should not hinder understanding of the code, but exactly the reverse.

  The set of macros should remain small so that it stays easy to
  use and understand. They should map to concepts commonly used in the
  jMax extensions and/or user interface.

  Please point out any inappropriate definitions and possible
  replacements for those.
*/

/* ************************************ macros that were in 0.1.0 */

/*
  lists the arguments suitable for any method of a given class.
  this includes a class-specific class name so you don't have to
  write inane stuff like
  
  SomeClass* self = (SomeClass *) o;
  
  at the beginning of every method body. This is a trade-off because
  it means you have to cast to (fts_object_t *) sometimes.
  see METHOD_PTR, METHOD, OBJ

*/
#define METHOD_ARGS(_class_) \
	_class_ *self, int winlet, fts_symbol_t selector, \
	int ac, const fts_atom_t *at

/*
  a header for a given method in a given class.
*/
#define METHOD(_class_,_name_) \
	void _class_##_##_name_(METHOD_ARGS(_class_))

/*
  returns a pointer to a method of a class as an fts_object_t* instead
  of a pointer to an object of your class. This is because in C, structs
  cannot be inherited from each other, and so a pointer of one type cannot be
  considered as a pointer to a similar, more elementary type.
*/
#define METHOD_PTR(_class_,_name_) \
	((void(*)(METHOD_ARGS(fts_object_t))) _class_##_##_name_)

/* a header for the class constructor */
#define CLASS(_name_) \
	fts_status_t _name_ ## _class_init \
	(fts_class_t *class, int ac, const fts_atom_t *at)

/* returns the size of a statically defined array */
#define COUNT(_array_) \
	((int)(sizeof(_array_) / sizeof((_array_)[0])))

/*
  casts a pointer to any object, into a fts_object_t*.
*/
#define OBJ(_object_) \
	((fts_object_t *)(_object_))

/*
  get arg or default value
  this works with int, float, symbol (fts_symbol_t),
  string (const char *), ptr (void *)
*/
#define GET(_i_,_type_,_default_) \
	fts_get_##_type_##_arg (ac,at,_i_,_default_)

/*
  "COUNT(array), array", a shortcut because often in jmax you pass
  first the size of an array followed by a pointer to that array.
*/
#define ARRAY(_array_) \
	COUNT(_array_), _array_

/* ************************************ new macros for 0.1.1 */

/*
  make sure an int is not too small nor too big;
  warn and adjust outside values
*/
#define COERCE_INT_INTO_RANGE(_var_,_lower_,_upper_) \
	if ((_var_) < (_lower_)) { \
		post("warning: %s=%d was smaller than %d\n", #_var_, (_var_), (_lower_)); \
		(_var_) = (_lower_); \
	} else if ((_var_) > (_upper_)) { \
		post("warning: %s=%d was bigger than %d\n", #_var_, (_var_), (_upper_)); \
		(_var_) = (_upper_); \
	}

#endif /* __MACROS_H */

/* ************************************ new macros for 0.2.0 */

#define $ self

#define assert_range(_var_,_lower_,_upper_) \
	if ((_var_) < (_lower_) || (_var_) > (_upper_)) { \
		fprintf(stderr, "%s:%d: assertion failed: %s=%d not in (%d..%d)\n", \
			__FILE__, __LINE__, #_var_, (_var_), (_lower_), (_upper_)); \
		abort(); }

#define NEW(_type_,_count_) \
	((_type_ *)qalloc(sizeof(_type_)*(_count_)))

#define NEW2(_type_,_count_) \
	((_type_ *)malloc(sizeof(_type_)*(_count_)))

/* ************************************ new macros for 0.2.2 */

#undef assert
#define assert(_expr_) \
	if (!(_expr_)) { \
		fprintf(stderr, "%s:%d: assertion failed: %s is false\n", \
			__FILE__, __LINE__, #_expr_); \
		abort(); }

#define SYM(_sym_) fts_new_symbol(#_sym_)

#define FREE(_var_) \
	_var_ ? (qfree(_var_), _var_=0) : 0

/* ************************************ junk */

/*
fts_object_set_error(OBJ(self), "width: %d is out of range", self->width);
*/
