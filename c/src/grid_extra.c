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

#include <stdlib.h>
#include <math.h>
#include "grid.h"

typedef struct GridScaleBy {
	GridObject_FIELDS;
	int rint;
} GridScaleBy;

GRID_BEGIN(GridScaleBy,0) {
	Dim *a = in->dim;
	if (Dim_count(a)!=3) RAISE("(height,width,chans) please");
	if (Dim_get(a,2)!=3) RAISE("3 chans please");
	{
		int v[3]={ Dim_get(a,0)*2, Dim_get(a,1)*2, Dim_get(a,2) };
		GridOutlet_begin($->out[0],Dim_new(3,v));
		GridInlet_set_factor(in,3*Dim_get(a,1));
	}
	return true;
}

GRID_FLOW(GridScaleBy,0) {
	int scale=2;//$->rint;
	int line = Dim_get(in->dim,1)*3;
	int i,j,k;
	while(n>0) {
		Number buf[line*scale];
		int p=0;
		for (i=0; i<line; i+=3) {
			for (k=0; k<scale; k++) {
				buf[p++]=data[i];
				buf[p++]=data[i+1];
				buf[p++]=data[i+2];
			}			
		}
		for (j=0; j<scale; j++) {
			GridOutlet_send($->out[0],line*scale,buf);
		}
		data+=line;
		n-=line;
	}
}

GRID_END(GridScaleBy,0) {
	GridOutlet_end($->out[0]);
}

METHOD(GridScaleBy,init) {
	$->rint = GET(1,int,2);
	GridObject_init((GridObject *)$);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

METHOD(GridScaleBy,delete) { GridObject_delete((GridObject *)$); }

GRCLASS(GridScaleBy,inlets:1,outlets:1,
LIST(GRINLET(GridScaleBy,0)),
	DECL(GridScaleBy,-1,init,  "s"),
	DECL(GridScaleBy,-1,delete,""))

/* **************************************************************** */

#define INSTALL(_sym_,_name_) \
	fts_class_install(Symbol_new(_sym_),_name_##_class_init)

void startup_grid_extra (void) {
	INSTALL("@scale_by",     GridScaleBy);
}
