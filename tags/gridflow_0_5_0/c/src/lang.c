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

/* this file is for classic objects, not dataflow objects. */
/* for dataflow objects, please see grid.c */
/* this file contains nothing fts-specific. */

#include "lang.h"
#include <assert.h>
#include <limits.h>

/* **************************************************************** */
/* Object/Class/Method */

Object *Object_new(Class *_class) {
	Object *$ = (Object *) NEW2(char,_class->objectsize);
	$->_class = _class;
	return $;
}

/* **************************************************************** */
/* List */

struct List {
	Object _o;
	int size;
	int capa;
	void **ptr;
};

#undef NEW
#define NEW(_type_,_count_) \
	((_type_ *)malloc(sizeof(_type_)*(_count_)))

List *List_new(int size) {
	List *$ = NEW(List,1);
	$->size = size;
	$->capa = size ? size : 1;
	$->ptr = NEW(void *,$->capa);
	{ int i; for (i=0; i<size; i++) $->ptr[i] = 0; }
	return $;
}

void List_push(List *$, void *v) {
	if ($->size >= $->capa) {
		void **ptr;
		$->capa = (7*$->capa+4)/5;
		ptr = NEW(void *,$->capa);
		memcpy(ptr,$->ptr,$->size*sizeof(void *));
		FREE($->ptr);
		$->ptr = ptr;
	}
	$->ptr[$->size]=v;
	$->size += 1;
}

void *List_get(List *$, int i) {
	return $->ptr[i];
}

void List_put(List *$, int i, void *v) {
	$->ptr[i] = v;
}

int List_size(List *$) {
	return $->size;
}

void *List_pop(List *$) {
	assert($->size>0);
	$->size--;
	return $->ptr[$->size];
}

void List_sort(List *$, int (*cmp)(void**,void**)) {
	qsort($->ptr, $->size, sizeof(void *), (int(*)(const void*,const void*))cmp);
}

/* **************************************************************** */
/* Dict */

struct DictEntry {
	Object _o;
	void *k;
	void *v;
	struct DictEntry *next;
};

struct Dict {
	Object _o;
	int capa;
	int size;
	DictEntry **table;
	CompFunc cf;
	HashFunc hf;
};

static int CompFunc_default(void *k1, void *k2) {
	return cmp((long)k1,(long)k2);
}

static int HashFunc_default(void *k) {
	return (int)k;
}

int HashFunc_string(void *k) {
	const char *s = k;
	int h=0;
	while (*s) h = (h<<13)^(h>>19)^(*s++);
	return h;
}

Dict *Dict_new(CompFunc cf, HashFunc hf) {
	int i;
	Dict *$ = NEW(Dict,1);
	$->cf = cf ? cf : CompFunc_default;
	$->hf = hf ? hf : HashFunc_default;
	$->capa = 7;
	$->size = 0;
	$->table = NEW(DictEntry *,$->capa);
	for (i=0; i<$->capa; i++) $->table[i] = 0;
	return $;
}

int Dict_size(Dict *$) {
	assert($->size>=0);
	return $->size;
}

long Dict_hash(Dict *$, void *k) {
	int k1 = $->hf(k);
	int k2 = (k1<<3)-k1;
	int k3 = (k2<<3)-k2;
	return INT_MAX & (k1^k2^k3);
}

DictEntry *Dict_has_key(Dict *$, void *k) {
	int h = Dict_hash($,k) % $->capa;
	DictEntry *de = $->table[h];
	while (de) {
		if ($->cf(de->k,k)==0) return de;
		de = de->next;
	}
	return 0;
}

void *Dict_get(Dict *$, void *k) {
	DictEntry *de = Dict_has_key($,k);
	if (de) return de->v; else return 0;
}

void Dict_put(Dict *$, void *k, void *v) {
	DictEntry *de = Dict_has_key($,k);
	assert(k!=(void *)0xdeadbeef);
//	printf("put %08x %08x\n",k,v);
	if ($->size >= $->capa) Dict_capa_is($,(7*$->capa+4)/5);

	if (de) {
		de->v = v;
	} else {
		int h = Dict_hash($,k) % $->capa;
		DictEntry *de = NEW(DictEntry,1);
		de->k = k;
		de->v = v;
		de->next = $->table[h];
		$->table[h] = de;
		$->size += 1;
	}
}

void Dict_del(Dict *$, void *k) {
	int h = Dict_hash($,k) % $->capa;
	DictEntry **dev = $->table+h;
	while (*dev) {
		DictEntry *de = *dev;
		if ($->cf(de->k,k)==0) {
			*dev = de->next;
			FREE(de);
			$->size -= 1;
			return;
		}
		dev = &(de->next);
	}
}

/* you must not modify the hash key set during this operation */
void Dict_each(Dict *$, void (*proc)(void*,void*,void*), void *data) {
	int i;
//	printf("hello.\n");
	for (i=0; i<$->capa; i++) {
		DictEntry *de = $->table[i];
		while (de) {
//			printf("%d %08x %08x %08x\n",i,de,de->k,de->v);
			proc(data,de->k,de->v);
			de = de->next;
		}
	}
}

void Dict_capa_is(Dict *$, int capa) {
	int i;
	DictEntry **l = $->table;
	int capa1 = $->capa;
	$->capa = capa;
	$->table = NEW(DictEntry *,$->capa);
	for (i=0; i<$->capa; i++) $->table[i] = 0;
	for (i=0; i<capa1; i++) {
		DictEntry *de = l[i];
		while (de) {
			DictEntry *wasnext = de->next;
			int h = Dict_hash($,de->k) % $->capa;
			de->next = $->table[h];
			$->table[h] = de;
			de = wasnext;
		}
	}
}
