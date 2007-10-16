/*
	$Id$

	GridFlow
	Copyright (c) 2001-2007 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "../base/grid.h.fcs"
#include <opencv/cv.h>

#define install(name,ins,outs) rb_funcall(rself,SI(install),3, \
	rb_str_new2(name),INT2NUM(ins),INT2NUM(outs))

int cv_eltype(NumberTypeE e) {
  switch (e) {
    case uint8_e: return IPL_DEPTH_8U;
    // IPL_DEPTH_8S not supported
    // IPL_DEPTH_16U not supported
    case int16_e: return IPL_DEPTH_16S;
    case int32_e: return IPL_DEPTH_32S;
    case float32_e: return IPL_DEPTH_32F;
    case float64_e: return IPL_DEPTH_64F;
    default: RAISE("unsupported type %s",number_type_table[e].name);
  }
}

enum CvMode {
	cv_mode_auto,
	cv_mode_channels,
	cv_mode_nochannels,
};

CvMode convert (Ruby x, CvMode *foo) {
	if (x==SYM(auto))       return cv_mode_auto;
	if (x==SYM(channels))   return cv_mode_channels;
	if (x==SYM(nochannels)) return cv_mode_nochannels;
	RAISE("invalid CvMode");
}

CvArr *cvGrid(PtrGrid g, CvMode mode) {
	P<Dim> d = g->dim;
	int channels=1;
	int dims=g->dim->n;
	post("mode=%d",(int)mode);
	if (mode==cv_mode_channels && g->dim->n==0) RAISE("channels dimension required for 'mode channels'");
	if (mode==cv_mode_auto && g->dim->n>=3 || mode==cv_mode_channels) channels=g->dim->v[--dims];
	post("channels=%d dims=%d",channels,dims);
	return 0;
}

\class CvAdd < GridObject
struct CvAdd : GridObject {
	\attr CvMode mode;
	PtrGrid l;
	PtrGrid r;
	\decl void initialize ();
	\grin 0
	\grin 1
};
\def void initialize () {
	rb_call_super(argc,argv);
	this->r = r?r:new Grid(new Dim(),int32_e,true);
	mode = cv_mode_auto;
}
GRID_INPUT2(CvAdd,0,l) {
	post("l=%p, r=%p", &*l, &*r);
	CvArr *a = cvGrid(l,mode);
	CvArr *b = cvGrid(r,mode);
	post("a=%p, b=%p", a, b);
} GRID_END
GRID_INPUT2(CvAdd,1,r) {} GRID_END

\classinfo { install("cv.add",2,1); }
\end class CvAdd

void startup_opencv() {
	\startall
}
