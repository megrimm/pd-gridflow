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

struct PlanEntry {
	int y,x; // offset
	bool neutral;
};

\class GridConvolve < GridObject
struct GridConvolve : GridObject {
	\attr Numop2 *op_para;
	\attr Numop2 *op_fold;
	\attr Grid seed;
	\attr Grid b;
	
	Grid a;
	int plann;
	PlanEntry *plan; //Pt?

	int margx,margy; /* margins */
	GridConvolve () { b.constrain(expect_convolution_matrix); plan=0; }
	\decl void initialize (Numop2 *op_para=op2_mul, Numop2 *op_fold=op2_add, Grid *seed=0, Grid *r=0);
	template <class T> void copy_row (Pt<T> buf, int sx, int y, int x);
	template <class T> void make_plan (T bogus);
	GRINLET3(0);
	GRINLET3(1);
};

template <class T> void GridConvolve::copy_row (Pt<T> buf, int sx, int y, int x) {
	int day = a.dim->get(0), dax = a.dim->get(1), dac = a.dim->prod(2);
	y=mod(y,day); x=mod(x,dax);
	Pt<T> ap = (Pt<T>)a + y*dax*dac;
	int u=(dax-x)*dac;
	int v=x*dac;
	while (sx) {
		int sx1 = min(sx,dax-x);
		COPY(buf,ap+x*dac,sx1*dac);
		x=0;
		buf += sx1*dac;
		sx -= sx1;
	}
}

static Numop2 *OP2(Ruby x) {return FIX2PTR(Numop2,rb_hash_aref(op2_dict,x));}

template <class T> void GridConvolve::make_plan (T bogus) {
	Dim *da = a.dim, *db = b.dim;
	int dby = db->get(0);
	int dbx = db->get(1);
	if (plan) delete[] plan;
	plan = new PlanEntry[dbx*dby];
	int i=0;
	for (int y=0; y<dby; y++) {
		for (int x=0; x<dbx; x++) {
			T rh = ((Pt<T>)b)[y*dbx+x];
			bool neutral = op_para->on(rh)->is_neutral(rh,at_right);
			bool absorbent = op_para->on(rh)->is_absorbent(rh,at_right);
			STACK_ARRAY(T,foo,1);
			static const char *boo[] = {"false","true"};
			if (absorbent) {
				foo[0] = 0;
				op_para->map(1,foo,rh);
				absorbent = op_fold->on(rh)->is_neutral(foo[0],at_right);
			}
			if (absorbent) continue;
			plan[i].y = y;
			plan[i].x = x;
			plan[i].neutral = neutral;
			i++;
		}
	}
	plann = i;
}

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
	margy = (db->get(0)-1)/2;
	margx = (db->get(1)-1)/2;
	a.init(in->dim->dup(),in->nt);
	out=new GridOutlet(this,0,da->dup(),in->nt);
} GRID_FLOW {
	COPY((Pt<T>)a+in->dex, data, n);
} GRID_FINISH {
	Numop2 *op2_put = OP2(SYM(put));
	make_plan((T)0);
	int dbx = b.dim->get(1);
	int day = a.dim->get(0);
	int n = a.dim->prod(1);
	int sx = a.dim->get(1)+dbx-1;
	int n2 = sx*a.dim->prod(2);
	STACK_ARRAY(T,buf,n);
	STACK_ARRAY(T,buf2,n2);
	T orh=0;
	for (int iy=0; iy<day; iy++) {
		op2_put->map(n,buf,*(T *)seed);
		for (int i=0; i<plann; i++) {
			int jy = plan[i].y;
			int jx = plan[i].x;
			T rh = ((Pt<T>)b)[jy*dbx+jx];
			if (i==0 || plan[i].y!=plan[i-1].y || orh!=rh) {
				copy_row(buf2,sx,iy+jy-margy,-margx);
				if (!plan[i].neutral) op_para->map(n2,buf2,rh);
			}
			op_fold->zip(n,buf,buf2+jx*a.dim->prod(2));
			orh=rh;
		}
		out->send(n,buf);
	}
	a.del();
	delete out; out=0;
} GRID_END

GRID_INPUT(GridConvolve,1,b) {} GRID_END

\def void initialize (Numop2 *op_para, Numop2 *op_fold, Grid *seed, Grid *r) {
	rb_call_super(argc,argv);
	this->op_para = op_para;
	this->op_fold = op_fold;
	if (seed) this->seed.swallow(seed); // this->seed = *seed;
	else this->seed.init_clear(new Dim(),int32_e);
	if (r) this->b.swallow(r);
}

GRCLASS(GridConvolve,LIST(GRINLET4(GridConvolve,0,4),GRINLET4(GridConvolve,1,4)),
	\grdecl
) { IEVAL(rself,"install '@convolve',2,1"); }

\end class GridConvolve

/* ---------------------------------------------------------------- */
/* "@scale_by" does quick scaling of pictures by integer factors */
/*{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }*/
\class GridScaleBy < GridObject
struct GridScaleBy : GridObject {
	\attr Grid scale; // integer scale factor
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

GRID_INLET(GridScaleBy,0) {
	Dim *a = in->dim;
	expect_picture(a);
	out=new GridOutlet(this,0,new Dim(a->get(0)*scaley,a->get(1)*scalex,a->get(2)),in->nt);
	in->set_factor(a->get(1)*a->get(2));
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	STACK_ARRAY(T,buf,rowsize*scalex);
	int chans = in->dim->get(2);
	// for every picture row in this packet...
	for (; n>0; data+=rowsize, n-=rowsize) {
		int p=0;
		switch (chans) {
		case 3:
			for (int i=0; i<rowsize; i+=3) {
				for (int k=0; k<scalex; k++, p+=3) {
					buf[p+0]=data[i+0];
					buf[p+1]=data[i+1];
					buf[p+2]=data[i+2];
				}
			}
		break;
		case 4:
			for (int i=0; i<rowsize; i+=4) {
				for (int k=0; k<scalex; k++, p+=4) {
					buf[p+0]=data[i+0];
					buf[p+1]=data[i+1];
					buf[p+2]=data[i+2];
					buf[p+3]=data[i+3];
				}
			}
		break;
		default:
			for (int i=0; i<rowsize; i+=chans) {
				for (int k=0; k<scalex; k++, p+=chans) {
					for (int c=0; c<chans; c++) buf[p+c]=data[i+c];
				}
			}
		}
		for (int j=0; j<scaley; j++) out->send(rowsize*scalex,buf);
	}
} GRID_FINISH {
	delete out; out=0;
} GRID_END

static void expect_scale_factor (Dim *dim) {
	if (dim->prod()!=1 && dim->prod()!=2)
		RAISE("expecting only one or two numbers");
}

GRID_INPUT(GridScaleBy,1,scale) { prepare_scale_factor(); } GRID_END

\def void initialize (Grid *factor) {
	scale.constrain(expect_scale_factor);
	rb_call_super(argc,argv);
	scale.init_from_ruby(INT2NUM(2));
	if (factor) scale.swallow(factor);
	prepare_scale_factor();
}

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
	out=new GridOutlet(this,0,new Dim(a->get(0)/scaley,a->get(1)/scalex,a->get(2)),in->nt);
	in->set_factor(a->get(1)*a->get(2));
//!@#$	in->set_factor(a->get(1)*a->get(2)*scaley); <- things could be easier with this
	temp.init(new Dim(2,in->dim->get(1)/scalex,in->dim->get(2)), in->nt);
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	int rowsize2 = temp.dim->prod();
	Pt<T> buf = (Pt<T>)temp; //!@#$ maybe should be something else than T ?

	int xinc = in->dim->get(2)*scalex;
	int y = in->dex / rowsize;

	int chans=in->dim->get(2);
	if (smoothly) {
		while (n>0) {
			if (y%scaley==0) CLEAR(buf,rowsize2);
			switch (chans) {
			// !@#$ how can i templatise all of those??? (cleanly)
			case 3:
			for (int i=0,p=0; p<rowsize2;) {
				for (int j=0; j<scalex; j++,i+=3) {
					buf[p+0]+=data[i+0];
					buf[p+1]+=data[i+1];
					buf[p+2]+=data[i+2];
				}
				p+=3;
			}break;
			case 4:
			for (int i=0,p=0; p<rowsize2;) {
				for (int j=0; j<scalex; j++,i+=4) {
					buf[p+0]+=data[i+0];
					buf[p+1]+=data[i+1];
					buf[p+2]+=data[i+2];
					buf[p+3]+=data[i+3];
				}
				p+=4;
			}break;
			default:
			for (int i=0,p=0; p<rowsize2;) {
				for (int j=0; j<scalex; j++,i+=chans) {
					for (int k=0; k<chans; k++)
						buf[p+k]+=data[i+k];
				}
				p+=chans;
			}break;
			}
			y++;
			if (y%scaley==0 && y/scaley<=in->dim->get(0)/scaley) {
				op2_div->map(rowsize2,buf,(T)(scalex*scaley));
				out->send(rowsize2,buf);
				CLEAR(buf,rowsize2);
			}
			data+=rowsize;
			n-=rowsize;
		}
	} else {
		for (; n>0; data+=rowsize, n-=rowsize,y++) {
			if (y%scaley || y/scaley>=in->dim->get(0)/scaley) continue;
			switch(in->dim->get(2)) {
			case 3:
			for (int i=0,p=0; p<rowsize2; i+=xinc, p+=3) {
				buf[p+0]=data[i+0];
				buf[p+1]=data[i+1];
				buf[p+2]=data[i+2];
			}break;
			case 4:
			for (int i=0,p=0; p<rowsize2; i+=xinc, p+=4) {
				buf[p+0]=data[i+0];
				buf[p+1]=data[i+1];
				buf[p+2]=data[i+2];
				buf[p+3]=data[i+3];
			}break;
			default:
			for (int i=0,p=0; p<rowsize2; i+=xinc, p+=chans) {
				for (int k=0; k<chans; k++)
					buf[p+k]=data[i+k];
			}break;
			}
			out->send(rowsize2,buf);
		}
	}
} GRID_FINISH {
	delete out; out=0;
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
	in->set_factor(a->prod(2));
	out=new GridOutlet(this,0,r.dim->dup());
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
#undef COMPUTE_ALPHA
	out->send(n*3/4,foo);
} GRID_FINISH {
	delete out; out=0;
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
	\attr Numop2 *op;
	\attr Grid color;
	\attr Grid polygon;
	Grid color2;
	Grid lines;
	int lines_start;
	int lines_stop;
	DrawPolygon() {
		color.constrain(expect_max_one_dim);
		polygon.constrain(expect_polygon);
	}
	\decl void initialize (Numop2 *op, Grid *color=0, Grid *polygon=0);
	GRINLET3(0);
	GRINLET3(1);
	GRINLET3(2);

	void init_lines();

};

void DrawPolygon::init_lines () {
	int nl = polygon.dim->get(0);
	lines.init(new Dim(nl,8), int32_e);
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
	out=new GridOutlet(this,0,in->dim->dup(),in->nt);
	lines_start = lines_stop = 0;
	in->set_factor(in->dim->get(1)*in->dim->get(2));
	int nl = polygon.dim->get(0);
	qsort((int32 *)lines,nl,sizeof(Line),order_by_starting_scanline);
	STACK_ARRAY(int32,v,1);
	int cn = color.dim->prod();
	color2.init(new Dim(cn*16), color.nt);
	for (int i=0; i<16; i++) COPY((Pt<T>)color2+cn*i,(Pt<T>)color,cn);
} GRID_FLOW {
	int nl = polygon.dim->get(0);
	Pt<Line> ld = Pt<Line>((Line *)(int32 *)lines,nl);

	int y = in->dex / in->factor;
	int cn = color.dim->prod();
	Pt<T> cd = (Pt<T>)color2;
	
	while (n) {
		while (lines_stop != nl && ld[lines_stop].y1<=y) lines_stop++;
		for (int i=lines_start; i<lines_stop; i++) {
			if (ld[i].y2<=y) {
				memswap(ld+i,ld+lines_start,1);
				lines_start++;
			}
		}
		if (lines_start == lines_stop) {
			out->send(in->factor,data);
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
				int xs = max(ld[i].x,(int32)0), xe = min(ld[i+1].x,xl);
				if (xs>=xe) continue; /* !@#$ WHAT? */
				while (xe-xs>=16) { op->zip(16*cn,data2+cn*xs,cd); xs+=16; }
				op->zip((xe-xs)*cn,data2+cn*xs,cd);
			}
			out->give(in->factor,data2);
		}
		n-=in->factor;
		data+=in->factor;
		y++;
	}
} GRID_FINISH {
	delete out; out=0;
} GRID_END

GRID_INPUT(DrawPolygon,1,color) {} GRID_END
GRID_INPUT(DrawPolygon,2,polygon) {init_lines();} GRID_END

\def void initialize (Numop2 *op, Grid *color, Grid *polygon) {
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
	if (d->n!=1) RAISE("position should have 1 dimension, not %d", d->n);
	if (d->v[0]!=2) RAISE("position dim 0 should have 2 elements, not %d", d->v[0]);
}

\class DrawImage < GridObject
struct DrawImage : GridObject {
	\attr Numop2 *op;
	\attr Grid image;
	\attr Grid position;
	\attr bool alpha;
	\attr bool tile;
	int count;
	
	DrawImage() : alpha(false), tile(false) {
		position.constrain(expect_position);
		image.constrain(expect_picture);
	}

	\decl void initialize (Numop2 *op, Grid *image=0, Grid *position=0);
	\decl void _0_alpha (bool v=true);
	\decl void _0_tile (bool v=true);
	GRINLET3(0);
	GRINLET3(1);
	GRINLET3(2);

	/* draw row # ry of right image in row buffer buf, starting at xs */
	/* overflow on both sides has to be handled automatically by this method */
	template <class T> void draw_segment(Pt<T> obuf, Pt<T> ibuf, int ry, int x0);
};

#define COMPUTE_ALPHA(c,a) obuf[j+(c)] = ibuf[j+(c)] + (rbuf[a])*(obuf[j+(c)]-ibuf[j+(c)])/256;
#define COMPUTE_ALPHA4(b) \
	COMPUTE_ALPHA(b+0,b+3); \
	COMPUTE_ALPHA(b+1,b+3); \
	COMPUTE_ALPHA(b+2,b+3); \
	obuf[b+3] = rbuf[b+3] + (255-rbuf[b+3])*(ibuf[j+b+3])/256;

template <class T> void DrawImage::draw_segment(Pt<T> obuf, Pt<T> ibuf, int ry, int x0) {
	count++;
	if (ry<0 || ry>=image.dim->get(0)) return; // outside of image
	int sx = in[0]->dim->get(1), rsx = image.dim->get(1);
	int sc = in[0]->dim->get(2), rsc = image.dim->get(2);
	Pt<T> rbuf = ((Pt<T>)image) + ry*rsx*rsc;
	if (x0>sx || x0<=-rsx) return; // outside of buffer
	int n=rsx;
	if (x0+n>sx) n=sx-x0;
	if (x0<0) { rbuf-=rsc*x0; n+=x0; x0=0; }
	if (alpha && rsc!=sc) { // RGB by RGBA //!@#$ optimise
		int j=sc*x0;
		for (; n; n--, rbuf+=rsc, j+=sc) {
			op->zip(sc,obuf+j,rbuf);
			COMPUTE_ALPHA(0,3); COMPUTE_ALPHA(1,3); COMPUTE_ALPHA(2,3);
		}
	} else if (alpha && rsc==4 && sc==4) { // RGBA by RGBA
		op->zip(n*rsc,obuf+x0*rsc,rbuf);
		int j=sc*x0;
		for (; n>=4; n-=4, rbuf+=16, j+=16) {
			COMPUTE_ALPHA4(0);COMPUTE_ALPHA4(4);
			COMPUTE_ALPHA4(8);COMPUTE_ALPHA4(12);
		}
		for (; n; n--, rbuf+=4, j+=4) {
			COMPUTE_ALPHA4(0);
		}
	} else { // RGB by RGB, etc
		op->zip(n*rsc,obuf+sc*x0,rbuf);
	}
}

GRID_INLET(DrawImage,0) {
	count=0;
	NOTEMPTY(image);
	NOTEMPTY(position);
	SAME_TYPE(*in,image);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	int lchan = in->dim->get(2);
	int rchan = image.dim->get(2);
	if (alpha && rchan!=4) {
		RAISE("alpha mode works only with 4 channels in right_hand");
	}
	if (lchan != rchan-(alpha?1:0) && lchan != rchan) {
		RAISE("right_hand has %d channels, alpha=%d, left_hand has %d, expecting %d or %d",
			rchan, alpha?1:0, lchan, rchan-(alpha?1:0), rchan);
	}
	out=new GridOutlet(this,0,in->dim->dup(),in->nt);
	in->set_factor(in->dim->get(1)*in->dim->get(2));
	int py = ((int32*)position)[0], rsy = image.dim->v[0], sy=in->dim->get(0);
	int px = ((int32*)position)[1], rsx = image.dim->v[1], sx=in->dim->get(1);
} GRID_FLOW {
	int y = in->dex/in->factor;
	if (position.nt != int32_e) RAISE("position has to be int32");
	int py = ((int32*)position)[0], rsy = image.dim->v[0], sy=in->dim->get(0);
	int px = ((int32*)position)[1], rsx = image.dim->v[1], sx=in->dim->get(1);
	for (; n; y++, n-=in->factor, data+=in->factor) {
		int ty = div2(y-py,rsy);
		if (tile || ty==0) {
			Pt<T> data2 = ARRAY_NEW(T,in->factor);
			COPY(data2,data,in->factor);
			if (tile) {
				for (int x=px-div2(px+rsx-1,rsx)*rsx; x<sx; x+=rsx) {
					draw_segment(data2,data,mod(y-py,rsy),x);
				}
			} else {
				draw_segment(data2,data,y-py,px);
			}
			out->give(in->factor,data2);
		} else {
			out->send(in->factor,data);
		}
	}
} GRID_FINISH {
	delete out; out=0;
} GRID_END

GRID_INPUT(DrawImage,1,image) {} GRID_END
GRID_INPUT(DrawImage,2,position) {} GRID_END

\def void _0_alpha (bool v=true) {
	alpha = v;
}

\def void _0_tile (bool v=true) {
	tile = v;
}

\def void initialize (Numop2 *op, Grid *image, Grid *position) {
	rb_call_super(argc,argv);
	this->op = op;
	if (image) this->image.swallow(image);
	if (position) this->position.swallow(position);
	else this->position.init_clear(new Dim(2), int32_e);
}

GRCLASS(DrawImage,LIST(GRINLET4(DrawImage,0,4),GRINLET4(DrawImage,1,4),GRINLET(DrawImage,2,4)),
	\grdecl
) { IEVAL(rself,"install '@draw_image',3,1"); }

\end class DrawImage

void startup_flow_objects_for_image () {
	\startup
}
