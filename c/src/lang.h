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

void gf_lang_init (void);

/* three-way comparison */
static inline int cmp(int a, int b) { return a < b ? -1 : a > b; }

/* **************************************************************** */
/* other general purpose stuff */

/*
  a remainder function such that floor(a/b)*b+mod(a,b) = a
  in contrast to C-language builtin a%b,
  this one has uniform behaviour around zero.
*/
static inline int mod(int a, int b) { if (a<0) a += b * (1-(a/b)); return a%b; }

/* integer powers in log(b) time */
static inline int ipow(int a, int b) {
	int r=1;
	while(1) {
		if (b&1) r*=a;
		b>>=1;
		if (!b) return r;
		a*=a;
	}
}	

#undef min
#undef max

/* minimum/maximum functions */

static inline int min(int a, int b) { return a<b?a:b; }
static inline int max(int a, int b) { return a>b?a:b; }
/*
static inline int min(int a, int b) { int c = -(a<b); return (a&c)|(b&~c); }
static inline int max(int a, int b) { int c = -(a>b); return (a&c)|(b&~c); }
*/

/* **************************************************************** */
/* Object/Class */

typedef struct Class Class;
typedef struct Object {
	Class *_class; /* or fts_class_t */
} Object;
struct Class {
	Object _o;
	Class *_super;
	int objectsize;
	int magic; /* 0x600df00d */
};

// void *Object_class(Object *$);

/* **************************************************************** */
/* List/Dict */

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
typedef int (*CompFunc)(void *k1, void *k2);
Dict *Dict_new(CompFunc cf, HashFunc hf);
int Dict_size(Dict *$);
long Dict_hash(Dict *$, void *k);
DictEntry *Dict_has_key(Dict *$, void *k);
void *Dict_get(Dict *$, void *k);
void Dict_put(Dict *$, void *k, void *v);
void Dict_del(Dict *$, void *k);
void Dict_each(Dict *$, void (*proc)(void *data, void *k, void *v), void *data);
void Dict_capa_is(Dict *$, int capa);

/* **************************************************************** */
/* FObject: flow object */

//typedef struct FObject FObject;

/* **************************************************************** */
/* Bridge */

/* **************************************************************** */

#endif /* __GRIDFLOW_LANG_H */
