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

#include <stdlib.h>
#include <stdio.h>

#include "grid.h"

//#if !defined(GRID_USE_INLINE) || (defined(GRID_USE_INLINE) && defined(PROC))
#ifndef PROC
#define PROC

#define Dim_invariant(_self_) \
	assert(_self_); \
	assert_range(_self_->n,0,MAX_DIMENSIONS);

Dim *Dim_new(int n, int *v) {
	Dim *$ = (Dim *) NEW(int,n+1);

	int i;
	assert_range(n,0,MAX_DIMENSIONS);
	assert(v);
	$->n = n;
	for (i=0; i<n; i++) {
		assert_range(v[i],0,MAX_INDICES);
		$->v[i] = v[i];
	}
	return $;
}

Dim *Dim_dup(Dim *$) {
	return Dim_new($->n,$->v);
}

int Dim_count(Dim *$) {
	Dim_invariant($);
	return $->n;
}

int Dim_get(Dim *$, int i) {
	Dim_invariant($);
	assert_range(i,0,$->n-1);
	assert_range($->v[i],0,MAX_INDICES);
	return $->v[i];
}

int Dim_prod(Dim *$) {
	int v=1;
	int i;
	Dim_invariant($);
	for (i=0; i<$->n; i++) v *= $->v[i];
	return v;
}

int Dim_prod_start (Dim *$, int start) {
	int v=1;
	int i;
	Dim_invariant($);
	for (i=start; i<$->n; i++) v *= $->v[i];
	return v;
}

PROC int Dim_calc_dex(Dim *$, int *v) {
	int dex=0;
	int i;
	for (i=0; i<$->n; i++) {
		dex = dex * $->v[i] + v[i];
	}
	return dex;
}

/* ******************************************************** */

/* returns a string like "Dim(240,320,3)" */
char *Dim_to_s(Dim *$) {
	/* if you blow 256 chars it's your own fault */
	char *bottom = NEW(char,256);
	char *s = bottom;

	int i=0;

	Dim_invariant($);
	
	s += sprintf(s,"Dim(");
	while(i<$->n) {
		s += sprintf(s,"%s%d", ","+!i, $->v[i]);
		i++;
	}
	s += sprintf(s,")");
	return (char *)realloc(bottom,strlen(bottom)+1);
}

int Dim_equal_verbose(Dim *$, Dim *other) {
	int i;
	Dim_invariant($);
	Dim_invariant(other);
	if ($->n != other->n) {
		whine("Got %d dimensions, but should be %d, "
			"as in (height,width,channels)", $->n, other->n);
		return 0;
	}
	for(i=0; i<$->n; i++) {
		if ($->v[0] != other->v[0]) {
			whine("Dimension #%d mismatch: got %d, should be %d",
				$->v[0], other->v[0]);
			return 0;
		}
	}
	return 1;
}

/* here, "other" better be a (height,width,channels) with channels=3 (rgb) */
int Dim_equal_verbose_hwc(Dim *$, Dim *other) {
	Dim_invariant($);
	Dim_invariant(other);
	if ($->n != other->n) {
		whine("Got %d dimensions, but should be %d, "
			"as in (height,width,channels)", $->n, other->n);
	} else if ($->v[0] != other->v[0]) {
		whine("Height mismatch: got %d, should be %d",
			$->v[0], other->v[0]);
	} else if ($->v[1] != other->v[1]) {
		whine("Width mismatch: got %d, should be %d",
			$->v[1], other->v[1]);
	} else if ($->v[2] != 3) {
		whine("Channel mismatch: got %d, should be %d, as in (red,green,blue)",
			$->v[2], other->v[2]);
	} else {
		return 1;
	}
	return 0;
}

#endif
#undef PROC
//#endif
