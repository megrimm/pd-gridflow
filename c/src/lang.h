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

#ifndef __GRIDFLOW_LANG_H
#define __GRIDFLOW_LANG_H
#include <stdlib.h>

#define $ self

#define NEW(_type_,_count_) \
	((_type_ *)qalloc(sizeof(_type_)*(_count_),__FILE__,__LINE__))

#define NEW2(_type_,_count_) \
	((_type_ *)qalloc2(sizeof(_type_)*(_count_),__FILE__,__LINE__))

#define NEW3(_type_,_count_,_file_,_line_) \
	((_type_ *)qalloc2(sizeof(_type_)*(_count_),_file_,_line_))

#define FREE(_var_) \
	_var_ ? (qfree(_var_), _var_=0) : 0

#define REALLOC(_val_,_count_) \
	(qrealloc(_val_,_count_))

void *qalloc2(size_t n, const char *file, int line);
void *qalloc(size_t n, const char *file, int line); /*0xdeadbeef*/
void qfree2(void *data);
void qfree(void *data); /*0xfadedf00*/
void *qrealloc(void *data, int n);
void qdump(void);

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
typedef unsigned long long uint64;
typedef char  int8;
typedef short int16;
typedef long  int32;
typedef float  float32;
typedef double float64;

#ifndef __cplusplus
typedef enum { false, true } bool;
#endif

typedef struct Class Class;
typedef struct Object {
	Class *_class;
} Object;
struct Class {
	Object _o;
	Class *_super;
	int sig; /* 0x600df00d */
};

typedef struct List List; /* List ([0...n] -> void *) */
List *List_new(int size);
void List_push(List *$, void *v);
void *List_get(List *$, int i);
void List_put(List *$, int i, void *v);
void List_del(List *$, int i, void *v);
int List_size(List *$);
void *List_pop(List *$);
void List_sort(List *$, int (*cmp)(void**,void**));

typedef struct Dict Dict; /* Dictionary (void * -> void *) */
typedef struct DictEntry DictEntry;
typedef int (*HashFunc)(void *k);
Dict *Dict_new(HashFunc hf);
int Dict_size(Dict *$);
long Dict_hash(Dict *$, void *k);
DictEntry *Dict_has_key(Dict *$, void *k);
void *Dict_get(Dict *$, void *k);
void Dict_put(Dict *$, void *k, void *v);
void Dict_del(Dict *$, void *k);
void Dict_each(Dict *$, void (*proc)(void* data, void *k, void* v), void *data);

#endif /* __GRIDFLOW_LANG_H */
