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

#include "video4jmax.h"
#include "grid.h"

//#if !defined(GRID_USE_INLINE) || (defined(GRID_USE_INLINE) && defined(PROC))
#ifndef PROC
#define PROC

Dim *Dim_new (int n, int *v) {
	Dim *$ = (Dim *) NEW(int,n+1);

	int i;
	assert_range(n,1,MAX_DIMENSIONS);
	assert(v);
	$->n = n;
	for (i=0; i<n; i++) {
		assert_range(v[i],1,MAX_INDICES);
		$->v[i] = v[i];
	}
	return $;
}

int Dim_count (Dim *$) {
	assert($);
	assert_range($->n,1,MAX_DIMENSIONS);
	return $->n;
}

int Dim_get (Dim *$, int i) {
	assert($);
	assert_range(i,0,$->n-1);
	assert_range($->v[i],1,MAX_INDICES);
	return $->v[i];
}

int Dim_prod(Dim *$) {
	int v=1;
	int i;
	assert($);
	for (i=0; i<$->n; i++) v *= $->v[i];
	return v;
}

int Dim_prod_start(Dim *$, int start) {
	int v=1;
	int i;
	assert($);
	for (i=start; i<$->n; i++) v *= $->v[i];
	return v;
}

/* returns a string like "Dim(240,320,3)" */
char *Dim_to_s(Dim *$) {
	/* if you blow 256 chars it's your own fault */
	char *bottom = NEW(char,256);
	char *s = bottom;

	int i=0;

	assert($);
	
	s += sprintf(s,"Dim");
	while(i<$->n) {
		s += sprintf(s,"%c%d", "(,"[!!i], $->v[i]);
		i++;
	}
	s += sprintf(s,")");
	return realloc(bottom,strlen(bottom)+1);
}

int Dim_equal_verbose(Dim *$, Dim *other) {
	int i;
	assert($);
	assert(other);
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
	assert($);
	assert(other);
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

/* ******************************************************** */

/*
  add n to the counter and return the number of the outermost index that
  wrapped around, or the number of dimensions if none.
*/
PROC int Dim_dex_add(Dim *$, int n, int *dex) {
	int k;
	int old = *dex;
	int new = *dex + n;
	assert($);
	assert(n>=0);
	k = $->n-1;
	*dex = new;
	if (*dex > Dim_prod($)) { *dex %= Dim_prod($); return 0; }
	while (old%$->v[k] != new%$->v[k]) {
		old /= $->v[k];
		new /= $->v[k];
		k--;
		if (k<0) return 0;
	}
	return k+1;
}

PROC int Dim_calc_dex(Dim *$, int *v) {
	int dex=0;
	int i;
	for (i=0; i<$->n; i++) {
		dex = dex * $->v[i] + v[i];
	}
	return dex;
}

#endif
#undef PROC
//#endif
