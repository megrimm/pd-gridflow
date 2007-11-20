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

int ipl_eltype(NumberTypeE e) {
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

int cv_eltype(NumberTypeE e) {
  switch (e) {
    case uint8_e: return CV_8U;
    // CV_8S not supported
    // CV_16U not supported
    case int16_e: return CV_16S;
    case int32_e: return CV_32S;
    case float32_e: return CV_32F;
    case float64_e: return CV_64F;
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
	//post("mode=%d",(int)mode);
	if (mode==cv_mode_channels && g->dim->n==0) RAISE("channels dimension required for 'mode channels'");
	if (mode==cv_mode_auto && g->dim->n>=3 || mode==cv_mode_channels) channels=g->dim->v[--dims];
	if (channels>64) RAISE("too many channels. max 64, got %d",channels);
	//post("channels=%d dims=%d nt=%d",channels,dims,g->nt);
	//post("bits=%d",number_type_table[g->nt].size);
	//if (dims==2) return cvMat(g->dim->v[0],g->dim->v[1],cv_eltype(g->nt),g->data);
	if (dims==2) {
		CvMat *a = cvCreateMatHeader(g->dim->v[0],g->dim->v[1],CV_MAKETYPE(cv_eltype(g->nt),channels));
		cvSetData(a,g->data,g->dim->prod(1)*(number_type_table[g->nt].size/8));
		return a;
	}
	RAISE("unsupported number of dimensions (got %d)",g->dim->n);
	//return 0;
}

IplImage *cvImageGrid(PtrGrid g, CvMode mode) {
	P<Dim> d = g->dim;
	if (d->n!=3) RAISE("expected 3 dimensions, got %s",d->to_s());
	int channels=g->dim->v[2];
	if (channels>64) RAISE("too many channels. max 64, got %d",channels);
	CvSize size = {d->v[1],d->v[0]};
	IplImage *a = cvCreateImageHeader(size,ipl_eltype(g->nt),channels);
	cvSetData(a,g->data,g->dim->prod(1)*(number_type_table[g->nt].size/8));
	return a;
}

\class CvOp1 < GridObject
struct CvOp1 : GridObject {
	\attr CvMode mode;
	\decl void initialize ();
	/* has no default \grin 0 handler so far. */
};
\def void initialize () {
	rb_call_super(0,0);
	mode = cv_mode_auto;
}
\classinfo {}
\end class CvOp1

\class CvOp2 < CvOp1
struct CvOp2 : CvOp1 {
	PtrGrid r;
	\decl void initialize (Grid *r=0);
	virtual void func(CvArr *l, CvArr *r, CvArr *o) {/* rien */}
	\grin 0
	\grin 1
};
\def void initialize (Grid *r=0) {
	rb_call_super(0,0);
	this->r = r?r:new Grid(new Dim(),int32_e,true);
}
GRID_INLET(CvOp2,0) {
	SAME_TYPE(in,r);
	if (!in->dim->equal(r->dim)) RAISE("dimension mismatch: left:%s right:%s",in->dim->to_s(),r->dim->to_s());
	in->set_chunk(0);
} GRID_FLOW {
	//post("l=%p, r=%p", &*l, &*r);
	PtrGrid l = new Grid(in->dim,(T *)data);
	PtrGrid o = new Grid(in->dim,in->nt);
	CvArr *a = cvGrid(l,mode);
	CvArr *b = cvGrid(r,mode);
	CvArr *c = cvGrid(o,mode);
	//post("a=%p, b=%p", a, b);
	func(a,b,c);
	out = new GridOutlet(this,0,in->dim,in->nt);
	out->send(o->dim->prod(),(T *)o->data);
} GRID_END
GRID_INPUT2(CvOp2,1,r) {} GRID_END
\classinfo {}
\end class CvOp2

#define FUNC virtual void func(CvArr *l, CvArr *r, CvArr *o)

\class CvAdd < CvOp2
struct CvAdd : CvOp2 {FUNC {cvAdd(l,r,o,0);}};
\classinfo { install("cv.Add",2,1); }
\end class CvAdd
\class CvSub < CvOp2
struct CvSub : CvOp2 {FUNC {cvSub(l,r,o,0);}};
\classinfo { install("cv.Sub",2,1); }
\end class CvSub
\class CvMul < CvOp2
struct CvMul : CvOp2 {FUNC {cvMul(l,r,o,1);}};
\classinfo { install("cv.Mul",2,1); }
\end class CvMul
\class CvDiv < CvOp2
struct CvDiv : CvOp2 {FUNC {cvDiv(l,r,o,1);}};
\classinfo { install("cv.Div",2,1); }
\end class CvDiv

\class CvSplit < CvOp1
struct CvSplit : CvOp1 {
	int channels;
	\decl void initialize (int channels);
};
\def void initialize (int channels) {
	rb_call_super(0,0);
	if (channels<0 || channels>64) RAISE("channels=%d is not in 1..64",channels);
	this->channels = channels;
	bself->noutlets_set(channels);
}
\classinfo {}
\end class CvSplit

\class CvHaarDetectObjects < GridObject
struct CvHaarDetectObjects : GridObject {
	\attr double scale_factor; /*=1.1*/
	\attr int min_neighbors;   /*=3*/
	\attr int flags;           /*=0*/
	\decl void initialize ();
};
\def void initialize () {
	scale_factor=1.1;
	min_neighbors=3;
	flags=0;
}
/*GRID_INLET(CvHaarDetectObjects,0) {
} GRID_FLOW {
	IplImage *img = ;
	CvHidHaarClassifierCascade* cascade = ;
	CvMemStorage* storage = ;
	CvSeq *ret = cvHaarDetectObjects(img,cascade,storage,scale_factor,min_neighbors,flags);
	return ret;
}*/
\classinfo { install("cv.HaarDetectObjects",2,1); }
\end class CvHaarDetectObjects

static int erreur_handleur (int status, const char* func_name, const char* err_msg, const char* file_name, int line, void *userdata) {
	// we might be looking for trouble because we don't know whether OpenCV is longjmp-proof.
	RAISE("OpenCV error: status=%d func_name=%s err_msg=\"%s\" file_name=%s line=%d",status,func_name,err_msg,file_name,line);
	// if this breaks OpenCV, then we will have to use post() or a custom hybrid of post() and RAISE() that does not cause a
	// longjmp when any OpenCV functions are on the stack.
}

void startup_opencv() {
	CvErrorCallback z = cvRedirectError(erreur_handleur);
	\startall
}
