/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

#include <math.h>
#include "grid.h.fcs"

static void expect_picture (Dim *d) {
	if (d->n!=3) RAISE("(height,width,chans) dimensions please");
}

static void expect_rgb_picture (Dim *d) {
	expect_picture(d);
	if (d->get(2)!=3) RAISE("(red,green,blue) channels please");
}

static void expect_rgba_picture (Dim *d) {
	expect_picture(d);
	if (d->get(2)!=4) RAISE("(red,green,blue,alpha) channels please");
}

static void expect_max_one_dim (Dim *d) {
	if (d->n>1) { RAISE("expecting Dim[] or Dim[n], got %s",d->to_s()); }
}

/* **************************************************************** */
/* the incoming grid is stored as "c" with a margin on the four sides
   of it. the margin is the size of the "b" grid minus one then split in two.
*/   

/*{ Dim[A,B,*Cs]<T>,Dim[D,E]<T> -> Dim[A,B,*Cs]<T> }*/

static void expect_convolution_matrix (Dim *d) {
	if (d->n != 2) RAISE("only exactly two dimensions allowed for now (got %d)",
		d->n);
}

\class GridConvolve < GridObject
struct GridConvolve : GridObject {
	\attr Operator2 *op_para;
	\attr Operator2 *op_fold;
	\attr Grid seed;

	Grid c,b;
	bool mode;
	int margx,margy,margx2,margy2; /* margins */
	GridConvolve () { b.constrain(expect_convolution_matrix); }	
	\decl void initialize (Operator2 *op_para=op2_mul, Operator2 *op_fold=op2_add, Grid *seed=0, Grid *r=0);
	\decl void mode_m (int m);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridConvolve,0) {
	SAME_TYPE(*in,b);
	SAME_TYPE(*in,seed);
	Dim *da = in->dim, *db = b.dim;
	if (!db) RAISE("right inlet has no grid");
	if (seed.is_empty()) RAISE("seed missing");
	if (db->n != 2) RAISE("right grid must have two dimensions");
	if (da->n < 2) RAISE("left grid has less than two dimensions");
	if (seed.dim->n != 0) RAISE("seed must be scalar");
	if (da->get(0) < db->get(0)) RAISE("grid too small (y): %d < %d",
		da->get(0), db->get(0));
	if (da->get(1) < db->get(1)) RAISE("grid too small (x): %d < %d",
		da->get(1), db->get(1));
	STACK_ARRAY(int32,v,da->n);
	COPY(v,da->v,da->n);
	margy = (db->get(0)-1)/2;
	margx = (db->get(1)-1)/2;
	margy2 = db->get(0)-1-margy;
	margx2 = db->get(1)-1-margx;
	v[0] += db->get(0)-1;
	v[1] += db->get(1)-1;
	c.init(new Dim(da->n,v),in->nt);
	out[0]->begin(da->dup(),in->nt);
	in->set_factor(da->prod(1));
} GRID_FLOW {
	int factor = in->factor; /* line length of a */
	Dim *da = in->dim, *dc = c.dim, *db = b.dim;
	int my = db->get(0), ny = da->get(0) - my/2*2;
	int mx = db->get(1);
	int ll = dc->prod(1); /* line length of c */
	int l  = dc->prod(2); /* "pixel" length of a,c */
	int oy = ll*(my/2), ox = l*(mx/2);
	int i = in->dex / factor; /* line number of a */
	Pt<T> base = ((Pt<T>)c)+(i+margy)*ll+margx*l;
	
	/* building c from a */
	while (n) {
		COPY(base,data,factor);
		COPY(base+factor,data,margx2*l);
		COPY(base-margx*l,data+factor-margx*l,margx*l);
		base += ll; data += factor; n -= factor; i++;
	}
} GRID_FINISH {
	Dim *da = in->dim, *dc = c.dim, *db = b.dim;
	int dby = db->get(0), day = da->get(0);
	int dbx = db->get(1), dax = da->get(1);
	int l  = dc->prod(2);
	int ll = dc->prod(1);
	Pt<T> cp = (Pt<T>)c;

	/* finishing building c from a */
	COPY(cp                      ,cp+da->get(0)*ll,margy*ll);
	COPY(cp+(margy+da->get(0))*ll,cp+margy*ll,    margy2*ll);
	switch (mode) {
	case 0:{
		
	STACK_ARRAY(T,buf3,l);
	STACK_ARRAY(T,buf2,l*dbx*dby);
	STACK_ARRAY(T,buf ,l*dbx*dby);
	Pt<T> q=buf2;
	for (int i=0; i<dbx*dby; i++) for (int j=0; j<l; j++) *q++=((Pt<T>)b)[i];
	for (int iy=0; iy<day; iy++) {
		for (int ix=0; ix<dax; ix++) {
			Pt<T> p = ((Pt<T>)c) + iy*ll + ix*l;
			Pt<T> r = buf;
			for (int jy=dby; jy; jy--,p+=ll,r+=dbx*l) COPY(r,p,dbx*l);
			for (int i=l-1; i>=0; i--) buf3[i]=*(T *)seed;
			op_para->zip(l*dbx*dby,buf,buf2);
			op_fold->fold(l,dbx*dby,buf3,buf);
			out[0]->send(l,buf3);
		}
	}
	
	}break;
	case 1:{

	int n = da->prod(1);
	STACK_ARRAY(T,buf,n);
	STACK_ARRAY(T,buf2,n);
	for (int iy=0; iy<day; iy++) {
		for (int i=0; i<n; i++) buf[i]=*(T *)seed;
		for (int jy=0; jy<dby; jy++) {
			for (int jx=0; jx<dbx; jx++) {
				Pt<T> p = ((Pt<T>)c) + (iy+jy)*ll + jx*l;
				COPY(buf2,p,n);
				op_para->map(n,buf2,((Pt<T>)b)[jy*dbx+jx]);
				op_fold->zip(n,buf,buf2);
			}
		}
		out[0]->send(n,buf);
	}
	
	}break;
	}
	
	c.del();
} GRID_END

\def void mode_m (int m) {
	if (m<0) m=0;
	if (m>1) m=1;
	gfpost("convolution mode = %d",m);
	mode = m;
}
	
GRID_INPUT(GridConvolve,1,b) {} GRID_END

\def void initialize (Operator2 *op_para, Operator2 *op_fold, Grid *seed, Grid *r) {
	rb_call_super(argc,argv);
	this->op_para = op_para;
	this->op_fold = op_fold;
	this->seed = *seed;
	this->mode = 1;
	if (r) this->b.swallow(r);
}

GRCLASS(GridConvolve,LIST(GRINLET4(GridConvolve,0,4),GRINLET4(GridConvolve,1,4)),
	\grdecl
) { IEVAL(rself,"install '@convolve',2,1"); }

\end class GridConvolve

/* ---------------------------------------------------------------- */
/* "@scale_by" does quick scaling of pictures by integer factors */

/*{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }*/
/*!@#$ should support N channels */

\class GridScaleBy < GridObject
struct GridScaleBy : GridObject {
	\attr Grid scale; /* integer scale factor */
	int scaley;
	int scalex;
	\decl void initialize (Grid *factor=0);
	GRINLET3(0);
	GRINLET3(1);
	void prepare_scale_factor () {
		scaley = ((Pt<int32>)scale)[0];
		scalex = ((Pt<int32>)scale)[scale.dim->prod()==1 ? 0 : 1];
		if (scaley<1) scaley=2;
		if (scalex<1) scalex=2;
	}
};

/* processing a grid coming from inlet 0 */
GRID_INLET(GridScaleBy,0) {
/* this section processes the header and accepts or rejects the grid */
	Dim *a = in->dim;
	expect_picture(a);
	/* computing the output's size */
	int32 v[3]={ a->get(0)*scaley, a->get(1)*scalex, a->get(2) };
	out[0]->begin(new Dim(3,v),in->nt);

	/* configuring the input format */
	in->set_factor(a->get(1)*a->get(2));
} GRID_FLOW {
/* this section processes one packet of grid content */
	int rowsize = in->dim->prod(1);
	STACK_ARRAY(T,buf,rowsize*scalex);

	int chans = in->dim->get(2);
	
	/* for every picture row in this packet... */
	for (; n>0; data+=rowsize, n-=rowsize) {

		/* scale the line x-wise */
		int p=0;
		if (chans==3) {
			for (int i=0; i<rowsize; i+=3) {
				for (int k=0; k<scalex; k++, p+=3) {
					buf[p+0]=data[i+0];
					buf[p+1]=data[i+1];
					buf[p+2]=data[i+2];
				}
			}
		} else {
			for (int i=0; i<rowsize; i+=chans) {
				for (int k=0; k<scalex; k++, p+=chans) {
					for (int c=0; c<chans; c++) buf[p+c]=data[i+c];
				}
			}
		}
		/* send the x-scaled line several times (thus y-scaling) */
		for (int j=0; j<scaley; j++) out[0]->send(rowsize*scalex,buf);
	}
} GRID_FINISH {
/* last section: things to do after the transmission. */
/* nothing to do here */
} GRID_END

static void expect_scale_factor (Dim *dim) {
	if (dim->prod()!=1 && dim->prod()!=2)
		RAISE("expecting only one or two numbers");
}

GRID_INPUT(GridScaleBy,1,scale) { prepare_scale_factor(); } GRID_END

/* the constructor accepts a scale factor as an argument */
/* that argument is not modifiable through an inlet yet (that would be the right inlet) */
\def void initialize (Grid *factor) {
	scale.constrain(expect_scale_factor);
	rb_call_super(argc,argv);
	scale.init_from_ruby(INT2NUM(2));
	if (factor) scale.swallow(factor);
	prepare_scale_factor();
}

/* there's one inlet, one outlet, and two system methods (inlet #-1) */
GRCLASS(GridScaleBy,LIST(GRINLET4(GridScaleBy,0,4),GRINLET(GridScaleBy,1,4)),
	\grdecl
) { IEVAL(rself,"install '@scale_by',2,1"); }

\end class GridScaleBy

/* ---------------------------------------------------------------- */
/* "@downscale_by" does quick downscaling of pictures by integer factors */

/*{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }*/

\class GridDownscaleBy < GridObject
struct GridDownscaleBy : GridObject {
	\attr Grid scale;
	\attr bool smoothly;
	int scaley;
	int scalex;
	Grid temp;
	\decl void initialize (Grid *factor=0, Symbol option=Qnil);
	GRINLET3(0);
	GRINLET3(1);
	void prepare_scale_factor () {
		scaley = ((Pt<int32>)scale)[0];
		scalex = ((Pt<int32>)scale)[scale.dim->prod()==1 ? 0 : 1];
		if (scaley<1) scaley=2;
		if (scalex<1) scalex=2;
	}
};

GRID_INLET(GridDownscaleBy,0) {
	Dim *a = in->dim;
	if (a->n!=3) RAISE("(height,width,chans) please");
	if (a->get(2)!=3) RAISE("3 chans please");
	int32 v[3]={ a->get(0)/scaley, a->get(1)/scalex, a->get(2) };
	out[0]->begin(new Dim(3,v),in->nt);
	in->set_factor(a->get(1)*a->get(2));
	int32 w[]={in->dim->get(1)/scalex,in->dim->get(2)};
	temp.init(new Dim(2,w));
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	int rowsize2 = temp.dim->prod();
	Pt<T> buf = (Pt<T>)temp;

	int xinc = in->dim->get(2)*scalex;
	int y = in->dex / rowsize;

	if (smoothly) {
		while (n>0) {
			if (y%scaley==0) CLEAR(buf,rowsize2);
			for (int i=0,p=0; p<rowsize2;) {
				for (int j=0; j<scalex; j++,i+=3) {
					buf[p+0]+=data[i+0];
					buf[p+1]+=data[i+1];
					buf[p+2]+=data[i+2];
				}
				p+=3;
			}
			y++;
			if (y%scaley==0 && y/scaley<=in->dim->get(0)/scaley) {
				op2_div->map(rowsize2,buf,(T)(scalex*scaley));
				out[0]->send(rowsize2,buf);
				CLEAR(buf,rowsize2);
			}
			data+=rowsize;
			n-=rowsize;
		}
	} else {
		for (; n>0; data+=rowsize, n-=rowsize,y++) {
			if (y%scaley || y/scaley>=in->dim->get(0)/scaley) continue;
			for (int i=0,p=0; p<rowsize2; i+=xinc) {
				buf[p+0]=data[i+0];
				buf[p+1]=data[i+1];
				buf[p+2]=data[i+2];
				p+=3;
			}
			out[0]->send(rowsize2,buf);
		}
	}
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridDownscaleBy,1,scale) { prepare_scale_factor(); } GRID_END

\def void initialize (Grid *factor, Symbol option) {
	scale.constrain(expect_scale_factor);
	rb_call_super(argc,argv);
	scale.init_from_ruby(INT2NUM(2));
	if (factor) scale.swallow(factor);
	prepare_scale_factor();
	smoothly = option==SYM(smoothly);
}

GRCLASS(GridDownscaleBy,LIST(GRINLET4(GridDownscaleBy,0,4),GRINLET(GridDownscaleBy,1,4)),
	\grdecl
) { IEVAL(rself,"install '@downscale_by',2,1"); }

\end class GridDownscaleBy

/* **************************************************************** */

\class GridLayer < GridObject
struct GridLayer : GridObject {
	Grid r;
	GridLayer() { r.constrain(expect_rgb_picture); }
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridLayer,0) {
	NOTEMPTY(r);
	SAME_TYPE(*in,r);
	Dim *a = in->dim;
	expect_rgba_picture(a);
	if (a->get(1)!=r.dim->get(1)) RAISE("same width please");
	if (a->get(0)!=r.dim->get(0)) RAISE("same height please");
	in->set_factor(a->prod(1));
	out[0]->begin(r.dim->dup());
} GRID_FLOW {
	Pt<T> rr = ((Pt<T>)r) + in->dex*3/4;
	STACK_ARRAY(T,foo,n*3/4);
#define COMPUTE_ALPHA(c,a) \
	foo[j+c] = (data[i+c]*data[i+a] + rr[j+c]*(256-data[i+a])) >> 8
	for (int i=0,j=0; i<n; i+=4,j+=3) {
		COMPUTE_ALPHA(0,3);
		COMPUTE_ALPHA(1,3);
		COMPUTE_ALPHA(2,3);
	}
	out[0]->send(n*3/4,foo);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridLayer,1,r) {} GRID_END

GRCLASS(GridLayer,LIST(GRINLET2(GridLayer,0,4),GRINLET2(GridLayer,1,4)),
	\grdecl
) { IEVAL(rself,"install '@layer',2,1"); }

\end class GridLayer

/* **************************************************************** */
//template <class T>
struct Line {
	int32 y1,x1,y2,x2,x,m,pad1,pad2;
};

static void expect_polygon (Dim *d) {
	if (d->n!=2 || d->get(1)!=2) RAISE("expecting Dim[n,2] polygon");
}

\class DrawPolygon < GridObject
struct DrawPolygon : GridObject {
	\attr Operator2 *op;
	\attr Grid color;
	\attr Grid polygon;
	Grid lines;
	int lines_start;
	int lines_stop;
	Pt<int32> lines_current;
	int lines_current_n;
	DrawPolygon() {
		color.constrain(expect_max_one_dim);
		polygon.constrain(expect_polygon);
	}
	\decl void initialize (Operator2 *op, Grid *color=0, Grid *polygon=0);
	GRINLET3(0);
	GRINLET3(1);
	GRINLET3(2);

	void init_lines();

};

void DrawPolygon::init_lines () {
	int nl = polygon.dim->get(0);
	int32 v[] = {nl,8};
	lines.init(new Dim(2,v));
	Pt<Line> ld = Pt<Line>((Line *)(int32 *)lines,nl);
	Pt<int32> pd = (Pt<int32>)polygon;
	for (int i=0,j=0; i<nl; i++) {
		ld[i].y1 = pd[j+0];
		ld[i].x1 = pd[j+1];
		j=(j+2)%(2*nl);
		ld[i].y2 = pd[j+0];
		ld[i].x2 = pd[j+1];
		if (ld[i].y1>ld[i].y2) memswap(Pt<int32>(ld+i)+0,Pt<int32>(ld+i)+2,2);
	}
}

static int order_by_starting_scanline (const void *a, const void *b) {
	return ((Line *)a)->y1 - ((Line *)b)->y1;
}

static int order_by_column (const void *a, const void *b) {
	return ((Line *)a)->x - ((Line *)b)->x;
}

GRID_INLET(DrawPolygon,0) {
	NOTEMPTY(color);
	NOTEMPTY(polygon);
	NOTEMPTY(lines);
	SAME_TYPE(*in,color);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	if (in->dim->get(2)!=color.dim->get(0))
		RAISE("image does not have same number of channels as stored color");
	out[0]->begin(in->dim->dup(),in->nt);
	lines_start = lines_stop = 0;
	in->set_factor(in->dim->get(1)*in->dim->get(2));
	int nl = polygon.dim->get(0);
	qsort((int32 *)lines,nl,sizeof(Line),order_by_starting_scanline);
} GRID_FLOW {
	int nl = polygon.dim->get(0);
	Pt<Line> ld = Pt<Line>((Line *)(int32 *)lines,nl);

	int y = in->dex / in->factor;
	while (n) {
		while (lines_stop != nl && ld[lines_stop].y1<=y) lines_stop++;
		for (int i=lines_start; i<lines_stop; i++) {
			if (ld[i].y2<=y) {
				memswap(ld+i,ld+lines_start,1);
				lines_start++;
			}
		}
		Pt<T> cd = (Pt<T>)color;
		int cn = color.dim->prod();
		if (lines_start == lines_stop) {
			out[0]->send(in->factor,data);
		} else {
			int32 xl = in->dim->get(1);
			Pt<T> data2 = ARRAY_NEW(T,in->factor);
			COPY(data2,data,in->factor);
			for (int i=lines_start; i<lines_stop; i++) {
				Line &l = ld[i];
				l.x = l.x1 + (y-l.y1)*(l.x2-l.x1+1)/(l.y2-l.y1+1);
			}
			qsort(ld+lines_start,lines_stop-lines_start,
				sizeof(Line),order_by_column);
			for (int i=lines_start; i<lines_stop-1; i+=2) {
				int xs = max(ld[i].x,(int32)0), xe = min(ld[i+1].x,xl-1);
				//int xn = xe-xs;
				/* !@#$ could be faster! */
				while (xs<xe) op->zip(cn,data2+cn*xs++,cd);
			}
			out[0]->give(in->factor,data2);
		}
		n-=in->factor;
		data+=in->factor;
		y++;
	}
} GRID_FINISH {
} GRID_END

GRID_INPUT(DrawPolygon,1,color) {} GRID_END
GRID_INPUT(DrawPolygon,2,polygon) {init_lines();} GRID_END

\def void initialize (Operator2 *op, Grid *color, Grid *polygon) {
	rb_call_super(argc,argv);
	this->op = op;
	if (color) this->color.swallow(color);
	if (polygon) { this->color.swallow(polygon); init_lines(); }
}

GRCLASS(DrawPolygon,LIST(GRINLET4(DrawPolygon,0,4),GRINLET4(DrawPolygon,1,4),GRINLET(DrawPolygon,2,4)),
	\grdecl
) { IEVAL(rself,"install '@draw_polygon',3,1"); }

\end class DrawPolygon

/* **************************************************************** */

static void expect_position(Dim *d) {
	if (d->n!=1) RAISE("huh?");
	if (d->v[0]!=2) RAISE("huh?");
}

\class DrawImage < GridObject
struct DrawImage : GridObject {
	\attr Operator2 *op;
	\attr Grid image;
	\attr Grid position;

	DrawImage() {
		position.constrain(expect_position);
		image.constrain(expect_picture);
	}

	\decl void initialize (Operator2 *op, Grid *image=0, Grid *position=0);
	GRINLET3(0);
	GRINLET3(1);
	GRINLET3(2);
};

GRID_INLET(DrawImage,0) {
	NOTEMPTY(image);
	NOTEMPTY(position);
	SAME_TYPE(*in,image);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	if (in->dim->get(2)!=image.dim->get(2))
		RAISE("incoming image does not have same number of channels as stored image");
	out[0]->begin(in->dim->dup(),in->nt);
	in->set_factor(in->dim->get(1)*in->dim->get(2));
} GRID_FLOW {
	int y = in->dex/in->factor;
	int py = ((int32*)position)[0];
	int px = ((int32*)position)[1];
	int sy = image.dim->v[0];
	int sx = image.dim->v[1];
	for (; n; y++, n-=in->factor, data+=in->factor) {
		if (y>=py && y<py+sy) {
			Pt<T> data2 = ARRAY_NEW(T,in->factor);
			COPY(data2,data,in->factor);
			int xs = max(0,px);
			int xe = min(in->dim->get(1),px+sx);
			int cn = image.dim->prod(2);
			Pt<T> cd = (Pt<T>)image + image.dim->prod(1)*(y-py) + cn*(xs-px);
			//!@#$ optimise
			for (; xs<xe; cd+=cn) op->zip(cn,data2+in->dim->get(2)*xs++,cd);
			out[0]->give(in->factor,data2);
		} else {
			out[0]->send(in->factor,data);
		}
	}
} GRID_FINISH {
} GRID_END

GRID_INPUT(DrawImage,1,image) {} GRID_END
GRID_INPUT(DrawImage,2,position) {} GRID_END

\def void initialize (Operator2 *op, Grid *image, Grid *position) {
	rb_call_super(argc,argv);
	this->op = op;
	if (image) this->image.swallow(image);
	if (position) this->position.swallow(position);
}

GRCLASS(DrawImage,LIST(GRINLET4(DrawImage,0,4),GRINLET4(DrawImage,1,4),GRINLET(DrawImage,2,4)),
	\grdecl
) { IEVAL(rself,"install '@draw_image',3,1"); }

\end class DrawImage

void startup_flow_objects_for_image () {
	\startup
}
