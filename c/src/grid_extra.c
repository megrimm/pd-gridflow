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

#define DECL(_cl_,_inlet_,_sym_,args...) \
	{_inlet_,SYM(_sym_),METHOD_PTR(_cl_,_sym_),args}

typedef struct GridScaleBy {
	GridObject_FIELDS;
	int rint;
} GridScaleBy;

GRID_BEGIN(GridScaleBy,0) {
	Dim *a = in->dim;
	int v[3];
	int i;
	if (Dim_count(a)!=3) {whine("(height,width,chans) please"); return false;}
	if (Dim_get(a,2)!=3) {whine("3 chans please"); return false;}
	v[0]=Dim_get(a,0)*2;
	v[1]=Dim_get(a,1)*2;
	v[2]=Dim_get(a,2);
	GridOutlet_begin($->out[0],Dim_new(3,v));
	GridInlet_set_factor(in,3*Dim_get(a,1));
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
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridScaleBy,0);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

METHOD(GridScaleBy,delete) { GridObject_delete((GridObject *)$); }

CLASS(GridScaleBy) {
	int i;
	fts_type_t int_alone[]  = {fts_t_int};
	fts_type_t init_args[] = {fts_t_symbol};
	MethodDecl methods[] = { 
		DECL(GridScaleBy,-1,init,  ARRAY(init_args),-1),
		DECL(GridScaleBy,-1,delete,0,0,0),
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridScaleBy), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);
	return fts_Success;
}


/* **************************************************************** */

#define INSTALL(_sym_,_name_) \
	fts_class_install(fts_new_symbol(_sym_),_name_##_class_init)

void startup_grid_extra (void) {
	INSTALL("@scale_by",     GridScaleBy);
}
