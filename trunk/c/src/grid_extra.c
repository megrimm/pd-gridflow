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
	int scale = $->rint;
	Dim *a = in->dim;
	if (Dim_count(a)!=3) RAISE("(height,width,chans) please");
	if (Dim_get(a,2)!=3) RAISE("3 chans please");
	{
		int v[3]={ Dim_get(a,0)*scale, Dim_get(a,1)*scale, Dim_get(a,2) };
		GridOutlet_begin($->out[0],Dim_new(3,v));
		GridInlet_set_factor(in,3*Dim_get(a,1));
	}
	return true;
}

GRID_FLOW(GridScaleBy,0) {
	int scale = $->rint;
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
	DECL(GridScaleBy,-1,init,  "si"),
	DECL(GridScaleBy,-1,delete,""))

/* **************************************************************** */

typedef struct GridRGBtoHSV {
	GridObject_FIELDS;
} GridRGBtoHSV;

GRID_BEGIN(GridRGBtoHSV,0) {
	if (Dim_count(in->dim)<1) RAISE("at least 1 dim please");
	if (Dim_get(in->dim,Dim_count(in->dim)-1)!=3) RAISE("3 chans please");
	GridOutlet_begin($->out[0],Dim_dup(in->dim));
	GridInlet_set_factor(in,3);
	return true;
}

/*
  h=42*0: red
  h=42*1: yellow
  h=42*2: green
  h=42*3: cyan
  h=42*4: blue
  h=42*5: magenta
  h=42*6: crap
*/
GRID_FLOW(GridRGBtoHSV,0) {
	GridOutlet *out = $->out[0];
	Number *buf = NEW(Number,n), *buf2=buf;
	int i;
	for (i=0; i<n; i+=3, buf+=3, data+=3) {
		int r=data[0], g=data[1], b=data[2];
		int v=buf[2]=max(max(r,g),b);
		int m=min(min(r,g),b);
		int d=(v-m)?(v-m):1;
		buf[1]=255*(v-m)/(v?v:1);
		buf[0] = 
			b==m ? 42*1+(g-r)*42/d :
			r==m ? 42*3+(b-g)*42/d :
			g==m ? 42*5+(r-b)*42/d : 0;
	}
	in->dex += n;
	GridOutlet_give(out,n,buf2);
}

GRID_END(GridRGBtoHSV,0) {
	GridOutlet_end($->out[0]);
}

METHOD(GridRGBtoHSV,init) {
	GridObject_init((GridObject *)$);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

METHOD(GridRGBtoHSV,delete) { GridObject_delete((GridObject *)$); }

GRCLASS(GridRGBtoHSV,inlets:1,outlets:1,
LIST(GRINLET(GridRGBtoHSV,0)),
	DECL(GridRGBtoHSV,-1,init,  "s"),
	DECL(GridRGBtoHSV,-1,delete,""))

/* **************************************************************** */

typedef struct GridHSVtoRGB {
	GridObject_FIELDS;
} GridHSVtoRGB;

GRID_BEGIN(GridHSVtoRGB,0) {
	if (Dim_count(in->dim)<1) RAISE("at least 1 dim please");
	if (Dim_get(in->dim,Dim_count(in->dim)-1)!=3) RAISE("3 chans please");
	GridOutlet_begin($->out[0],Dim_dup(in->dim));
	GridInlet_set_factor(in,3);
	return true;
}

GRID_FLOW(GridHSVtoRGB,0) {
	GridOutlet *out = $->out[0];
	Number *buf = NEW(Number,n), *buf2 = buf;
	int i;
	for (i=0; i<n; i+=3, buf+=3, data+=3) {
		int h=mod(data[0],252), s=data[1], v=data[2];
		int j=h%42;
		int k=h/42;
		int m=(255-s)*v/255;
		int d=s*v/255;
		buf[0]=(k==4?j:k==5||k==0?42:k==1?42-j:0)*d/42+m;
		buf[1]=(k==0?j:k==1||k==2?42:k==3?42-j:0)*d/42+m;
		buf[2]=(k==2?j:k==3||k==4?42:k==5?42-j:0)*d/42+m;
	}
	in->dex += n;
	GridOutlet_give(out,n,buf2);
}

GRID_END(GridHSVtoRGB,0) {
	GridOutlet_end($->out[0]);
}

METHOD(GridHSVtoRGB,init) {
	GridObject_init((GridObject *)$);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

METHOD(GridHSVtoRGB,delete) { GridObject_delete((GridObject *)$); }

GRCLASS(GridHSVtoRGB,inlets:1,outlets:1,
LIST(GRINLET(GridHSVtoRGB,0)),
	DECL(GridHSVtoRGB,-1,init,  "s"),
	DECL(GridHSVtoRGB,-1,delete,""))

/* **************************************************************** */

#define INSTALL(_sym_,_name_) \
	fts_class_install(Symbol_new(_sym_),_name_##_class_init)

void startup_grid_extra (void) {
	INSTALL("@scale_by",     GridScaleBy);
	INSTALL("@rgb_to_hsv",   GridRGBtoHSV);
	INSTALL("@hsv_to_rgb",   GridHSVtoRGB);
}
