/*
	Video4jmax
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file LICENSE for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __VIDEO_H
#define __VIDEO_H

#include "config.h"
#include "fts_redirect.h"
#include "macros.h"

#include <assert.h>
#ifdef NO_ASSERT
	/* disabling assertion checking */
	#undef assert
	#define assert(_foo_)
#endif

/* current version number as string literal */
#define VIDEO4JMAX_VERSION "0.2.0"

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
typedef char  int8;
typedef short int16;
typedef long  int32;
typedef float float32;
typedef double float64;

int high_bit(unsigned long n);
int low_bit(unsigned long n);
void whine(char *fmt, ...);
void *qalloc(size_t n);

/* **************************************************************** */

typedef struct MethodDecl MethodDecl;
struct MethodDecl {
	int winlet;
	fts_symbol_t selector;
	void (*method)(METHOD_ARGS(fts_object_t));
	int n_args;
	fts_type_t *args;
	int min_args;
};

void define_many_methods(fts_class_t *class, int n, MethodDecl *methods);

#define DECL_SYM(_sym_) \
	extern fts_symbol_t sym_##_sym_;

DECL_SYM(open)
DECL_SYM(close)
DECL_SYM(reset)
DECL_SYM(autodraw)
DECL_SYM(grid_begin)
DECL_SYM(grid_packet)
DECL_SYM(grid_end)

/* not used for now
typedef enum {
	t_uint8,
	t_uint16,
	t_uint32,
unsupported:
	t_int8,
	t_int16,
	t_int32,
	t_float32,
	t_float64,
	t_complex64,
	t_complex128,
} ArrayType;

typedef struct {
	int type;
	void *buffer;
} Array;
*/

#endif /* __VIDEO_H */
