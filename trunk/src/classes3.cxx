/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
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
#include "gridflow.hxx.fcs"
static CONSTRAINT(expect_max_one_dim) {if (d.n>1)  RAISE("expecting Dim[] or Dim[n], got %s",d.to_s());}
static CONSTRAINT(expect_one_dim)     {if (d.n!=1) RAISE("expecting Dim[n], got %s",d.to_s());}
// BAD HACK: GCC complains: unimplemented (--debug mode only) (i don't remember which GCC this was)
#ifdef HAVE_DEBUG
#define SCOPY(a,b,n) COPY(a,b,n)
#else
#define SCOPY(a,b,n) SCopy<n>::f(a,b)
#endif
template <long n> struct SCopy {template <class T> static inline void __attribute__((always_inline)) f(T *a, T *b) {*a=*b; SCopy<n-1>::f(a+1,b+1);}};
template <>    struct SCopy<0> {template <class T> static inline void __attribute__((always_inline)) f(T *a, T *b) {}};
template <class T> T *DUP(T *m, size_t n) {T *r = (T *)malloc(sizeof(T)*n); memcpy(r,m,sizeof(T)*n); return r;}

//****************************************************************
\class GridToTilde {
	P<Grid> blah;
	t_outlet **sigout;
	int chans; /* number of channels */
	int start;
	int size;
	t_sample **sam;
	\constructor (int chans=1, int capacity=16384) {
		if (chans<0) RAISE("need nonnegative number of channels");
		if (capacity<1) RAISE("capacity must be at least 1");
		if ((capacity&-capacity)!=capacity) RAISE("capacity must be a power of two");
		sigout = new t_outlet *[chans];
		for (int i=0; i<chans; i++) sigout[i] = outlet_new((t_object *)bself,&s_signal);
		this->chans = chans;
		blah=new Grid(Dim(capacity,chans),float32_e);
		start=0; size=0; sam=0;
	}
	~GridToTilde () {delete[] sigout; if (sam) delete[] sam;}
	void perform (int n) {
		float32 *data = (float32 *)*blah;
		for (int i=0; i<n; i++) {
			if (size) {
				for (int j=0; j<chans; j++) sam[j][i]=data[start*chans+j];
				start=(start+1)&(blah->dim[0]-1);
				size--;
			} else for (int j=0; j<chans; j++) sam[j][i]=0;
		}
	}
	static t_int *perform_ (t_int *w) {((GridToTilde *)w[1])->perform(int(w[2])); return w+3;}
	void dsp (t_signal **sp) {
		if (sam) delete[] sam;
		sam = new t_sample *[chans];
		for (int i=0; i<chans; i++) sam[i] = sp[i]->s_vec;
		dsp_add(perform_,2,this,sp[0]->s_n);
	}
	static void dsp_ (BFObject *bself, t_signal **sp) {((GridToTilde *)bself->self)->dsp(sp);}
	\grin 0 float32
};
GRID_INLET(0) {
	if (in.dim.n!=2) RAISE("expecting two dimensions: signal samples, signal channels");
	if (in.dim[1]!=chans) RAISE("grid has %d channels, but this object has %d outlets",in.dim[1],chans);
	long samples = in.dim.prod()/chans;
	int capacity = blah->dim[0];
	if (samples+size>capacity) post("[#to~] buffer full: dropping %ld samples.",samples+size-capacity);
} GRID_FLOW {
	int capacity = blah->dim[0];
	while (n && size<capacity) {
		int i = ((start+size)&(capacity-1)) * chans;
		COPY((T *)*blah+i,data,chans);
		data+=chans; n-=chans; size++;
	}
} GRID_END
\end class {
	install("#to~",1,0); // actually it has outlets that are not registered with GF
	class_addmethod(fclass->bfclass,(t_method)GridToTilde::dsp_,gensym("dsp"),A_CANT,0);
}
//****************************************************************
\class GridFromTilde {
	P<Grid> blah;
	t_inlet **sigin;
	int chans; /* number of channels */
	t_sample **sam;
	t_clock *clock;
	int size;
	bool custom;
	\constructor (int chans=1, int capacity=0) {
		sigin = new t_inlet *[chans];
		for (int i=0; i<chans; i++) sigin[i] = inlet_new((t_object *)bself,(t_pd *)bself,&s_signal,&s_signal);
		this->chans = chans;
		sam=0;
		size=0;
		clock = clock_new(bself,(void(*)())clock_bang_);
		custom = !!capacity;
		if (custom) {
			if ((capacity&-capacity) != capacity) RAISE("capacity must be a power of two (or zero for auto)");
			blah=new Grid(Dim(capacity,chans),float32_e);
		}
	}
	void clock_bang () {
		GridOut go(this,0,blah->dim,blah->nt);
		go.send(blah->dim.prod(),(float32 *)*blah);
	}
	static void clock_bang_ (BFObject *bself) {((GridFromTilde *)bself->self)->clock_bang();}
	~GridFromTilde () {clock_free(clock); delete[] sigin; if (sam) delete[] sam;}
	void perform (int n) {
		float32 *data = (float32 *)*blah;
		for (int i=0; i<n; i++, size=(size+1)&(blah->dim[0]-1))
			for (int j=0; j<chans; j++) data[size*chans+j]=sam[j][i];
		if (!size) clock_delay(clock,0);
	}
	static t_int *perform_ (t_int *w) {((GridFromTilde *)w[1])->perform(int(w[2])); return w+3;}
	void dsp (t_signal **sp) {
		if (sam) delete[] sam;
		sam = new t_sample *[chans];
		for (int i=0; i<chans; i++) sam[i] = sp[i]->s_vec;
		dsp_add(perform_,2,this,sp[0]->s_n);
		if (!custom) blah=new Grid(Dim(sp[0]->s_n,chans),float32_e);
	}
	static void dsp_ (BFObject *bself, t_signal **sp) {((GridFromTilde *)bself->self)->dsp(sp);}
};
\end class {
	// note that the real inlet 0 is hidden, and the other inlets are not registered with GF
	install("#from~",1,1,CLASS_NOINLET);
	class_addmethod(fclass->bfclass,(t_method)GridFromTilde::dsp_,gensym("dsp"),A_CANT,0);
}

//****************************************************************
\class GridJoin {
	\attr int which_dim;
	P<Grid> r;
	\grin 0
	\grin 1
	\constructor (int which_dim=-1, Grid *r=0) {
		this->which_dim = which_dim;
		this->r=r?r:new Grid(Dim(),int32_e,true);
	}
};

GRID_INLET(0) {
	SAME_TYPE(in,r);
	Dim &d = in.dim;
	if (d.n != r->dim.n) RAISE("wrong number of dimensions");
	int w = which_dim;
	if (w<0) w+=d.n;
	if (w<0 || w>=d.n) RAISE("dim number %d does not exist %d-dimensional grids", which_dim,d.n);
	int32 v[d.n];
	for (int i=0; i<d.n; i++) {
		v[i] = d[i];
		if (i==w) {
			v[i]+=r->dim[i];
		} else {
			if (v[i]!=r->dim[i]) RAISE("dimensions mismatch: dim #%i, left is %d, right is %d",i,v[i],r->dim[i]);
		}
	}
	go=new GridOut(this,0,Dim(d.n,v),in.nt);
	in.set_chunk(w);
} GRID_FLOW {
	int w = which_dim; //! make macro NEGDIM
	if (w<0) w+=in.dim.n;
	long a = in.dim.prod(w);
	long b = r->dim.prod(w);
	T *data2 = (T *)*r + in.dex*b/a;
	if (a==3 && b==1) {
		int m = n+n*b/a;
		T data3[m];
		T *data4 = data3;
		while (n) {
			SCOPY(data4,data,3); SCOPY(data4+3,data2,1);
			n-=3; data+=3; data2+=1; data4+=4;
		}
		go->send(m,data3);
	} else if (a+b<=16) {
		int m = n+n*b/a;
		T data3[m];
		int i=0;
		while (n) {
			COPY(data3+i,data,a); data+=a; i+=a; n-=a;
			COPY(data3+i,data2,b); data2+=b; i+=b;
		}
		go->send(m,data3);
	} else {
		while (n) {
			go->send(a,data);
			go->send(b,data2);
			data+=a; data2+=b; n-=a;
		}
	}
} GRID_FINISH {
	if (in.dim.prod()==0) go->send(r->dim.prod(),(T *)*r);
} GRID_END
GRID_INPUT(1,r) {} GRID_END
\end class {install("#join",2,1); add_creator("@join");}

//****************************************************************
\class GridGrade {
	\constructor () {}
	\grin 0
};

typedef int (*comparator_t)(const void *, const void *);

template <class T> struct GradeFunction {
	static int comparator (T **a, T **b) {return **a-**b;}};
#define FOO(T) \
template <> struct GradeFunction<T> { \
	static int comparator (T **a, T **b) {T x = **a-**b; return x<0 ? -1 : x>0;}};
FOO(int64)
FOO(float32)
FOO(float64)
#undef FOO

GRID_INLET(0) {
	if (in.dim.n<1) RAISE("minimum 1 dimension");
	go=new GridOut(this,0,in.dim);
	in.set_chunk(in.dim.n-1);
} GRID_FLOW {
	long m = in.dim.prod(in.dim.n-1);
	T *foo[m];
	T  bar[m];
	if (m) for (; n; n-=m,data+=m) {
		for (int i=0; i<m; i++) foo[i] = &data[i];
		qsort(foo,m,sizeof(T *),(comparator_t)GradeFunction<T>::comparator);
		for (int i=0; i<m; i++) bar[i] = foo[i]-(T *)data;
		go->send(m,bar);
	}
} GRID_END

\end class {install("#grade",1,1); add_creator("@grade");}

//****************************************************************
//\class GridMedian : FObject
//****************************************************************

\class GridTranspose {
	\attr int dim1;
	\attr int dim2;
	int d1,d2,na,nb,nc,nd; // temporaries
	\constructor (int dim1=0, int dim2=1) {
		this->dim1 = dim1;
		this->dim2 = dim2;
	}
	\decl 1 float (int dim1) {this->dim1=dim1;}
	\decl 2 float (int dim2) {this->dim2=dim2;}
	\grin 0
};

GRID_INLET(0) {
	int32 v[in.dim.n];
	COPY(v,in.dim.v,in.dim.n);
	d1=dim1; d2=dim2;
	if (d1<0) d1+=in.dim.n;
	if (d2<0) d2+=in.dim.n;
	if (d1>=in.dim.n || d2>=in.dim.n || d1<0 || d2<0)
		RAISE("would swap dimensions %d and %d but this grid has only %d dimensions", dim1,dim2,in.dim.n);
	memswap(v+d1,v+d2,1);
	if (d1==d2) {
		go=new GridOut(this,0,Dim(in.dim.n,v),in.nt);
	} else {
		nd = in.dim.prod(1+max(d1,d2));
		nc = in.dim[max(d1,d2)];
		nb = nc&&nd ? in.dim.prod(1+min(d1,d2))/nc/nd : 0;
		na = in.dim[min(d1,d2)];
		go=new GridOut(this,0,Dim(in.dim.n,v),in.nt);
		in.set_chunk(min(d1,d2));
	}
	// Turns a Grid[*,na,*nb,nc,*nd] into a Grid[*,nc,*nb,na,*nd].
} GRID_FLOW {
	//T res[na*nb*nc*nd];
	T *res = NEWBUF(T,na*nb*nc*nd);
	if (dim1==dim2) {go->send(n,data); return;}
	int prod = na*nb*nc*nd;
	for (; n; n-=prod, data+=prod) {
		for (long a=0; a<na; a++)
			for (long b=0; b<nb; b++)
				for (long c=0; c<nc; c++)
					COPY(res +((c*nb+b)*na+a)*nd,
					     data+((a*nb+b)*nc+c)*nd,nd);
		go->send(na*nb*nc*nd,res);
	}
	DELBUF(res); //!@#$ if an exception was thrown by go->send, this never gets done
} GRID_END

\end class {install("#transpose",3,1); add_creator("@transpose");}

//****************************************************************
\class GridReverse {
	\attr int dim1; // dimension to act upon
	int d; // temporaries
	\constructor (int dim1=0) {this->dim1 = dim1;}
	\decl 1 float (int dim1) {this->dim1=dim1;}
	\grin 0
};
GRID_INLET(0) {
	d=dim1;
	if (d<0) d+=in.dim.n;
	if (d>=in.dim.n || d<0) RAISE("would reverse dimension %d but this grid has only %d dimensions",dim1,in.dim.n);
	go=new GridOut(this,0,in.dim,in.nt);
	in.set_chunk(d);
} GRID_FLOW {
	long f1=in.dim.prod(d), f2=in.dim.prod(d+1);
	while (n) {
		T *data2 = data+f1-f2;
		//long hf1=f1/2; for (long i=0; i<hf1; i+=f2) memswap(data+i,data2-i,f2);
		T tada[f1]; for (long i=0; i<f1; i+=f2) COPY(tada+i,data2-i,f2);
		go->send(f1,tada);
		data+=f1; n-=f1;
	}
} GRID_END

\end class {install("#reverse",2,1);}

//****************************************************************
\class GridCentroid {
	\constructor () {}
	\grin 0 int
	int sumx,sumy,sum,y; // temporaries
};

GRID_INLET(0) {
	if (in.dim.n != 3) RAISE("expecting 3 dims");
	if (in.dim[2] != 1) RAISE("expecting 1 channel");
	in.set_chunk(1);
	go=new GridOut(this,0,Dim(2), in.nt);
	sumx=0; sumy=0; sum=0; y=0;
} GRID_FLOW {
	int sx = in.dim[1];
	while (n) {
		for (int x=0; x<sx; x++) {
			sumx+=x*data[x];
			sumy+=y*data[x];
			sum +=  data[x];
		}
		n-=sx;
		data+=sx;
		y++;
	}
} GRID_FINISH {
	int32 blah[2];
	blah[0] = sum ? sumy/sum : 0;
	blah[1] = sum ? sumx/sum : 0;
	go->send(2,blah);
	out[1](blah[0]);
	out[2](blah[1]);
} GRID_END

\end class {install("#centroid",1,3);}

//****************************************************************
static CONSTRAINT(expect_pair) {if (d.prod()!=2) RAISE("expecting only two numbers. Dim(2)");}

\class GridMoment {
	\constructor (int order=1) {
		offset.but(expect_pair);
		t_atom2 a[2] = {t_atom2(0),t_atom2(0)};
		offset=new Grid(2,a,int32_e);
		if (order!=1 && order!=2) RAISE("supports only orders 1 and 2 for now");
		this->order=order;
	}
	\grin 0 int
	\grin 1 int
	\attr int order; // order
	\attr P<Grid> offset;
	int64 sumy,sumxy,sumx,sum,y; // temporaries
};

GRID_INLET(0) {
	if (in.dim.n != 3) RAISE("expecting 3 dims");
	if (in.dim[2] != 1) RAISE("expecting 1 channel");
	in.set_chunk(1);
	switch (order) {
	    case 1: go=new GridOut(this,0,Dim(2  ), in.nt); break;
	    case 2: go=new GridOut(this,0,Dim(2,2), in.nt); break;
	    default: RAISE("supports only orders 1 and 2 for now");
	}
	sumx=0; sumy=0; sumxy=0; sum=0; y=0;
} GRID_FLOW {
	int sx = in.dim[1];
	int oy = ((int*)*offset)[0];
	int ox = ((int*)*offset)[1];
	while (n) {
		switch (order) {
		    case 1:
			for (int x=0; x<sx; x++) {
				sumy+=y*data[x];
				sumx+=x*data[x];
				sum +=  data[x];
			}
		    break;
		    case 2:
			for (int x=0; x<sx; x++) {
				int ty=y-oy;
				int tx=x-ox;
				sumy +=ty*ty*data[x];
				sumxy+=tx*ty*data[x];
				sumx +=tx*tx*data[x];
				sum  +=      data[x];
			}
		}
		n-=sx;
		data+=sx;
		y++;
	}
} GRID_FINISH {
	int32 blah[4];
	switch (order) {
	    case 1: /* centroid vector */
		blah[0] = sum ? sumy/sum : 0;
		blah[1] = sum ? sumx/sum : 0;
		go->send(2,blah);
	    break;
	    case 2: /* covariance matrix */
		blah[0] = sum ? sumy/sum : 0;
		blah[1] = sum ? sumxy/sum : 0;
		blah[2] = sum ? sumxy/sum : 0;
		blah[3] = sum ? sumx/sum : 0;
		go->send(4,blah);
	    break;
	}
} GRID_END

GRID_INPUT(1,offset) {} GRID_END

\end class {install("#moment",2,1);}

//****************************************************************
\class GridLabelling {
	\grin 0
	\attr int form();
	\attr int form_val;
	\constructor (int form=0) {form_val=form; initialize3();}
	void initialize3() {noutlets_set(form_val ? 2 : 4);}
};
\def int form () {return form_val;}
struct Stats {
	int64 yy,yx,xx,y,x,area;
	int64 x1,x2;
	Stats() {yy=yx=xx=y=x=area=0;}
};
#define AT(y,x) dat[(y)*sx+(x)]
template <class T> void flood_fill(T *dat, int sy, int sx, int y, int x, Stats *stat, int label, int form) {
	/* find x1,x2 such that all the x of that horizontal segment are x1<=x<x2 */
	int x2; for (x2=x; x2<sx; x2++) if (AT(y,x2)!=1) break;
	int x1; for (x1=x; x1>=0; x1--) if (AT(y,x1)!=1) break;
	x1++;
	if (form==0) {
		for (x=x1; x<x2; x++) {
			AT(y,x)=label;
			stat->yy += y*y; stat->y += y;
			stat->yx += y*x; stat->area++;
			stat->xx += x*x; stat->x += x;
		}
		for (x=x1; x<x2; x++) {
			if (y>0    && AT(y-1,x)==1) flood_fill(dat,sy,sx,y-1,x,stat,label,form);
			if (y<sy-1 && AT(y+1,x)==1) flood_fill(dat,sy,sx,y+1,x,stat,label,form);
		}
	} else {
		for (x=x1; x<x2; x++) {
			AT(y,x)=label;
		}
		stat->y=y;
		stat->x1=x1;
		stat->x2=x2;
	}
}

GRID_INLET(0) {
	if (in.dim.n<2 || in.dim.prod(2)!=1) RAISE("requires dim (y,x) or (y,x,1)");
	in.set_chunk(0);
} GRID_FLOW {
	int sy=in.dim[0], sx=in.dim[1];
	T *dat = NEWBUF(T,n);
	for (int i=0; i<n; i++) dat[i]=data[i];
	int y,x=0,label=2;
	for (y=0; y<sy; y++) for (x=0; x<sx; x++) {
		if (dat[y*sx+x]!=1) continue;
		Stats s;
		flood_fill(dat,sy,sx,y,x,&s,label,form_val);
		if (form_val==0) {
			float32 cooked[6] = {
				(s.yy-s.y*s.y/s.area)/s.area,
				(s.yx-s.y*s.x/s.area)/s.area,
				(s.yx-s.y*s.x/s.area)/s.area,
				(s.xx-s.x*s.x/s.area)/s.area,
				s.y/s.area,
				s.x/s.area};
			float a[] = {s.area};
			send_out(3,1,a);
			GridOut o2(this,2,Dim(2));   o2.send(2,cooked+4);
			GridOut o1(this,1,Dim(2,2)); o1.send(4,cooked);
		} else {
			float32 cooked[4] = {s.y,s.x1,s.y,s.x2};
			GridOut o1(this,1,Dim(2,2)); o1.send(4,cooked);
		}
		label++;
	}
	go = new GridOut(this,0,Dim(sy,sx,1),in.nt);
	go->send(n,dat);
	DELBUF(dat);
} GRID_END

\def 0 form(int form) {
	if (form<0 || form>1) RAISE("form must be 0 or 1, not %d",form);
	form_val=form;
	initialize3();
}
\end class {install("#labelling",1,0); add_creator("#labeling");}

//****************************************************************
\class GridPerspective {
	\attr int32 z;
	\grin 0
	\constructor (int32 z=256) {this->z=z;}
};
GRID_INLET(0) {
	int n = in.dim.n;
	int32 v[n];
	COPY(v,in.dim.v,n);
	v[n-1]--;
	in.set_chunk(in.dim.n-1);
	go=new GridOut(this,0,Dim(n,v),in.nt);
} GRID_FLOW {
	int m = in.dim.prod(in.dim.n-1);
	for (; n; n-=m,data+=m) {
		op_mul->map(m-1,data,(T)z);
		op_div->map(m-1,data,data[m-1]);
		go->send(m-1,data);
	}	
} GRID_END
\end class {install("#perspective",1,1); add_creator("@perspective");}

//****************************************************************
\class GridBorder {
	/*\attr */ Dim diml;
	/*\attr */ Dim dimr;
	P<Grid> diml_grid;
	P<Grid> dimr_grid;
	\grin 0
	\grin 1 int
	\grin 2 int
	\constructor (Grid *dl=0, Grid *dr=0) {
		t_atom2 a[2] = {1,1};
		diml_grid=dl?dl:new Grid(2,a,int32_e);
		dimr_grid=dr?dr:new Grid(2,a,int32_e);
		diml = diml_grid->to_dim();
		dimr = dimr_grid->to_dim();
	}
};

GRID_INLET(0) {
	int n = in.dim.n;
	if (n<2 || n>3) RAISE("only 2 or 3 dims supported for now");
	if (diml.n<2 || diml.n>n) RAISE("diml mismatch");
	if (dimr.n<2 || dimr.n>n) RAISE("dimr mismatch");
	if ((diml.n>2 && diml[2]) || (diml.n>2 && dimr[2])) RAISE("can't augment channels (todo)");
	int32 v[n];
	for (int i=0; i<n; i++) v[i]=in.dim[i] + (i<diml.n?diml[i]:0) + (i<dimr.n?dimr[i]:0);
	in.set_chunk(0);
	go=new GridOut(this,0,Dim(n,v),in.nt);
} GRID_FLOW {
	int sy = in.dim[0];
	int sx = in.dim[1];      int zx = sx+diml[1]     +dimr[1];
	int sc = in.dim.prod(2); int zc = sc+(2<diml.n?diml[2]:0) + (2<dimr.n?dimr[2]:0);
	int sxc = sx*sc; int zxc = zx*zc;
	int32 duh[zxc];
	for (int x=0; x<zxc; x++) duh[x]=0;
	for (int y=0; y<diml[0]; y++) go->send(zxc,duh);
	for (int y=0; y<sy; y++) {
		go->send(diml[1]*sc,duh);
		go->send(sxc,data+y*sxc);
		go->send(dimr[1]*sc,duh);
	}	
	for (int i=0; i<dimr[0]; i++) go->send(zxc,duh);
} GRID_END

GRID_INPUT(1,diml_grid) {diml = diml_grid->to_dim();} GRID_END
GRID_INPUT(2,dimr_grid) {dimr = dimr_grid->to_dim();} GRID_END

\end class {install("#border",3,1);}

static CONSTRAINT(expect_picture) {if (d.n!=3) RAISE("(height,width,chans) dimensions please");}
static CONSTRAINT(expect_rgb_picture)  {expect_picture(d,nt); if (d[2]!=3) RAISE("(red,green,blue) channels please");}
static CONSTRAINT(expect_rgba_picture) {expect_picture(d,nt); if (d[2]!=4) RAISE("(red,green,blue,alpha) channels please");}

//****************************************************************
//{ Dim[A,B,*Cs]<T>,Dim[D,E]<T> -> Dim[A,B,*Cs]<T> }

static CONSTRAINT(expect_convolution_matrix) {
	if (d.n!=2) RAISE("only exactly two dimensions allowed for now (got %d)",d.n);
}


\class GridConvolve {
	// entry in a compiled convolution kernel
	struct  PlanEntry {long y; long x; long k; bool neutral;
		PlanEntry (long y, long x, long k, bool neutral) : y(y), x(x), k(k), neutral(neutral) {}
	};
	\attr Numop2 *op;
	\attr Numop2 *fold;
	\attr P<Grid> seed;
	\attr P<Grid> b;
	\attr bool wrap;
	\attr bool anti;
	vector<PlanEntry> plan;
	int margx,margy; // margins
	\constructor (Grid *r=0) {
		b.but(expect_convolution_matrix);
		this->op = op_mul;
		this->fold = op_add;
		this->seed = 0;//new Grid(Dim(),int32_e,true);
		this->b= r ? r : new Grid(Dim(1,1),int32_e,true);
		this->wrap = true;
		this->anti = true;
	}
	\grin 0
	\grin 1
	template <class T> void   copy_row (Dim &dim, T *data, T *buf, long sx, long y, long x);
	template <class T> void muladd_row (Dim &dim, T *data, T *buf, long sx, long y, long x, T m);
	template <class T> void    mul_row (Dim &dim, T *data, T *buf, long sx, long y, long x, T m);
	template <class T> void make_plan (T bogus);
};

template <class T> void GridConvolve::copy_row (Dim &dim, T *data, T *buf, long sx, long y, long x) {
	long day = dim[0], dax = dim[1], dac = dim.prod(2);
	y=mod(y,day); x=mod(x,dax); T *ap = data + y*dax*dac;
	while (sx) {
		long sx1 = min(sx,dax-x);
		COPY(buf,ap+x*dac,sx1*dac);
		x=0; buf += sx1*dac; sx -= sx1;
	}
}

#define UNROLL_8B(MACRO,LABEL,N,A,B,ARGS...) \
	int n__=(-N)&7; A-=n__; B-=n__; N+=n__; \
	switch (n__) { LABEL: \
		case 0:MACRO(0); case 1:MACRO(1); case 2:MACRO(2); case 3:MACRO(3); \
		case 4:MACRO(4); case 5:MACRO(5); case 6:MACRO(6); case 7:MACRO(7); \
		A+=8; B+=8; N-=8; ARGS; /*fprintf(stderr,"n=%d\n",N);*/ if (N) goto LABEL; }

// map * then zip +
// UNROLL_8 and UNROLL_8B seem to be pretty useless nowadays (gcc 4.x)
// and even the whole muladd,mul business seems to not have so much effect.
template <class T> void muladd (int n, T *as, T *bs, T c) {
	//fprintf(stderr,"muladd n=%d as=%p bs=%p c=%d\n",n,as,bs,c);
	if (!n) return;
	if (c==-1) {
		#define FOO(I) as[I] -= bs[I]
		UNROLL_8B(FOO,L1,n,as,bs)
		#undef FOO
	} else if (c==1) {
		#define FOO(I) as[I] += bs[I]
		UNROLL_8B(FOO,L2,n,as,bs)
		#undef FOO
	} else {
		#define FOO(I) as[I] += bs[I] * c
		UNROLL_8B(FOO,L3,n,as,bs)
		#undef FOO
	}
}
template <class T> void mul    (int n, T *as, T *bs, T c) {
	//fprintf(stderr,"mul    n=%d as=%p bs=%p c=%d\n",n,as,bs,c);
	if (!n) return;
	if (c==-1) {
		#define FOO(I) as[I] =-bs[I]
		UNROLL_8B(FOO,L1,n,as,bs)
		#undef FOO
	} else if (c==1) {
		#define FOO(I) as[I] = bs[I]
		UNROLL_8B(FOO,L2,n,as,bs)
		#undef FOO
	} else {
		#define FOO(I) as[I] = bs[I] * c
		UNROLL_8B(FOO,L3,n,as,bs)
		#undef FOO
	}
}

// map * then zip +
/*
template <class T> void muladd (int n, T *as, T *bs, T c) {
	if (c==-1)     while (n--) *as++ -= *bs++    ; else
	if (c==1)      while (n--) *as++ += *bs++    ; else
	else           while (n--) *as++ += *bs++ * c;
}
template <class T> void mul    (int n, T *as, T *bs, T c) {
	if (c==-1)     while (n--) *as++  = -*bs++   ; else
	if (c==1)      while (n--) *as++  = *bs++    ; else
	               while (n--) *as++  = *bs++ * c;
}
*/

template <class T> void GridConvolve::muladd_row (Dim &dim, T *data, T *buf, long sx, long y, long x, T m) {
	long day = dim[0], dax = dim[1], dac = dim.prod(2);
	y=mod(y,day); x=mod(x,dax); T *ap = data + y*dax*dac;
	while (sx) {
		long sx1 = min(sx,dax-x);
		muladd(sx1*dac,buf,ap+x*dac,m);
		x=0; buf += sx1*dac; sx -= sx1;
	}
}
template <class T> void GridConvolve::mul_row (Dim &dim, T *data, T *buf, long sx, long y, long x, T m) {
	long day = dim[0], dax = dim[1], dac = dim.prod(2);
	y=mod(y,day); x=mod(x,dax); T *ap = data + y*dax*dac;
	while (sx) {
		long sx1 = min(sx,dax-x);
		mul(sx1*dac,buf,ap+x*dac,m);
		x=0; buf += sx1*dac; sx -= sx1;
	}
}

template <class T> void GridConvolve::make_plan (T bogus) {
	Dim &db = b->dim;
	long dby = db[0];
	long dbx = db[1];
	plan.clear();
	for (long y=0; y<dby; y++) {
		for (long x=0; x<dbx; x++) {
			long k = anti ? y*dbx+x : (dby-1-y)*dbx+(dbx-1-x);
			T rh = ((T *)*b)[k];
			bool neutral   = op->on(rh)->is_neutral(  rh,at_right);
			bool absorbent = op->on(rh)->is_absorbent(rh,at_right);
			T foo[1]={0}; // how does this work, again ?
			if (absorbent) {
				op->map(1,foo,rh);
				absorbent = fold->on(rh)->is_neutral(foo[0],at_right);
			}
			if (absorbent) continue;
			plan.push_back(PlanEntry(y,x,k,neutral));
		}
	}
}

GRID_INLET(0) {
	SAME_TYPE(in,b);
	if (seed) SAME_TYPE(in,seed);
	Dim &da=in.dim, &db=b->dim;
	if (db.n != 2) RAISE("right grid must have two dimensions");
	if (da.n < 2) RAISE("left grid has less than two dimensions");
	if (seed && seed->dim.n) RAISE("seed must be scalar");
	if (da[0] < db[0]) RAISE("grid too small (y): %d < %d", da[0], db[0]);
	if (da[1] < db[1]) RAISE("grid too small (x): %d < %d", da[1], db[1]);
	margy = (db[0]-1)/2;
	margx = (db[1]-1)/2;
	int v[da.n]; COPY(v,da.v,da.n);
	if (!wrap) {v[0]-=db[0]-1; v[1]-=db[1]-1;}
	in.set_chunk(0);
	go=new GridOut(this,0,Dim(da.n,v),in.nt);
} GRID_FLOW {
	make_plan((T)0);
	long dbx = b->dim[1];
	long day = go->dim[0];
	long n   = go->dim.prod(1);
	long sx  = go->dim[1]+dbx-1;
	long sxc = go->dim.prod(2)*sx;
	T buf[n];
	T buf2[sxc];
	if (op==op_mul && fold==op_add && (!seed || *(T *)*seed == T(0))) {
		for (long ay=0; ay<day; ay++) {
			for (size_t i=0; i<plan.size(); i++) {
				long by = plan[i].y, bx = plan[i].x; T rh = ((T *)*b)[plan[i].k];
				if (i) {
					if (wrap) muladd_row(in.dim,data,buf,sx,ay+by-margy,bx-margx,rh);
					else      muladd_row(in.dim,data,buf,sx,ay+by      ,0       ,rh);
				} else {
					if (wrap)    mul_row(in.dim,data,buf,sx,ay+by-margy,bx-margx,rh);
					else         mul_row(in.dim,data,buf,sx,ay+by      ,0       ,rh);
				}
			}
			go->send(n,buf);
		}
	} else {
		T orh=0;
		for (long ay=0; ay<day; ay++) {
			op_put->map(n,buf, seed ? *(T *)*seed : T(0));
			for (size_t i=0; i<plan.size(); i++) {
				long by = plan[i].y, bx = plan[i].x; T rh = ((T *)*b)[plan[i].k];
				if (i==0 || by!=plan[i-1].y || orh!=rh) {
					if (wrap) copy_row(in.dim,data,buf2,sx,ay+by-margy,-margx);
					else      copy_row(in.dim,data,buf2,sx,ay+by      ,0     );
					if (!plan[i].neutral) op->map(sxc,buf2,rh);
				}
				fold->zip(n,buf,buf2+bx*go->dim.prod(2));
				orh=rh;
			}
			go->send(n,buf);
		}
	}
} GRID_END

GRID_INPUT(1,b) {} GRID_END

\end class {install("#convolve",2,1);}

/* ---------------------------------------------------------------- */
/* "#scale_by" does quick scaling of pictures by integer factors */
/*{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }*/

static CONSTRAINT(expect_scale_factor) {
	if (d.n>1) RAISE("expecting no more than one dimension");
	if (d.prod()!=1 && d.prod()!=2) RAISE("expecting only one or two numbers");
}

\class GridScaleBy {
	\attr P<Grid> scale; // integer scale factor
	int scaley;
	int scalex;
	\constructor (Grid *factor=0) {
		scale.but(expect_scale_factor);
		t_atom2 a[1] = {2};
		scale = factor?factor:new Grid(1,a);
		prepare_scale_factor();
	}
	\grin 0
	\grin 1
	void prepare_scale_factor () {
		scaley = max(1,((int32 *)*scale)[0]);
		scalex = max(1,((int32 *)*scale)[scale->dim.prod()-1]);
	}
};

GRID_INLET(0) {
	Dim &a = in.dim;
	expect_picture(a,in.nt);
	go=new GridOut(this,0,Dim(a[0]*scaley,a[1]*scalex,a[2]),in.nt);
	in.set_chunk(1);
} GRID_FLOW {
	int rowsize = in.dim.prod(1);
	T buf[rowsize*scalex];
	int chans = in.dim[2];
	#define Z(z) buf[p+z]=data[i+z]
	for (; n>0; data+=rowsize, n-=rowsize) {
		int p=0;
		#define LOOP(z) \
			for (int i=0; i<rowsize; i+=z) \
			for (int k=0; k<scalex; k++, p+=z)
		switch (chans) {
		case 3: LOOP(3) {Z(0);Z(1);Z(2);} break;
		case 4: LOOP(4) {Z(0);Z(1);Z(2);Z(3);} break;
		default: LOOP(chans) {for (int c=0; c<chans; c++) Z(c);}
		}
		#undef LOOP
		for (int j=0; j<scaley; j++) go->send(rowsize*scalex,buf);
	}
	#undef Z
} GRID_END

GRID_INPUT(1,scale) {prepare_scale_factor();} GRID_END

\end class {install("#scale_by",2,1); add_creator("@scale_by");}

// ----------------------------------------------------------------
//{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }
\class GridDownscaleBy {
	\attr P<Grid> scale;
	\attr bool smoothly;
	int scaley;
	int scalex;
	P<Grid> temp;
	\constructor (Grid *factor=0, t_symbol *option=0) {
		scale.but(expect_scale_factor);
		t_atom2 a[1] = {2};
		scale = factor?factor:new Grid(1,a,int32_e);
		prepare_scale_factor();
		smoothly = option==gensym("smoothly");
	}
	\grin 0
	\grin 1
	void prepare_scale_factor () {
		scaley = ((int32 *)*scale)[0];
		scalex = ((int32 *)*scale)[scale->dim.prod()==1 ? 0 : 1];
		if (scaley<1) scaley=2;
		if (scalex<1) scalex=2;
	}
};

GRID_INLET(0) {
	Dim &a = in.dim;
	if (a.n!=3) RAISE("(height,width,chans) please");
	go=new GridOut(this,0,Dim(a[0]/scaley,a[1]/scalex,a[2]),in.nt);
	in.set_chunk(1);
	// i don't remember why two rows instead of just one.
	temp=new Grid(Dim(2,in.dim[1]/scalex,in.dim[2]),in.nt);
} GRID_FLOW {
	int rowsize = in.dim.prod(1);
	int rowsize2 = temp->dim.prod(1);
	T *buf = (T *)*temp; //!@#$ maybe should be something else than T, to avoid overflow ?
	int xinc = in.dim[2]*scalex;
	int y = in.dex / rowsize;
	int chans=in.dim[2];
	#define Z(z) buf[p+z]+=data[i+z]
	if (smoothly) {
		while (n>0) {
			if (y%scaley==0) CLEAR(buf,rowsize2);
			#define LOOP(z) \
				for (int i=0,p=0; p<rowsize2; p+=z) \
				for (int j=0; j<scalex; j++,i+=z)
			switch (chans) {
			case 1: LOOP(1) {Z(0);} break;
			case 2: LOOP(2) {Z(0);Z(1);} break;
			case 3: LOOP(3) {Z(0);Z(1);Z(2);} break;
			case 4: LOOP(4) {Z(0);Z(1);Z(2);Z(3);} break;
			default:LOOP(chans) {for (int k=0; k<chans; k++) Z(k);} break;
			}
			#undef LOOP
			y++;
			if (y%scaley==0 && go->sender) {
				op_div->map(rowsize2,buf,(T)(scalex*scaley));
				go->send(rowsize2,buf);
				CLEAR(buf,rowsize2);
			}
			data+=rowsize;
			n-=rowsize;
		}
	#undef Z
	} else {
	#define Z(z) buf[p+z]=data[i+z]
		for (; n>0 && go->sender; data+=rowsize, n-=rowsize,y++) {
			if (y%scaley!=0) continue;
			#define LOOP(z) for (int i=0,p=0; p<rowsize2; i+=xinc, p+=z)
			switch(in.dim[2]) {
			case 1: LOOP(1) {Z(0);} break;
			case 2: LOOP(2) {Z(0);Z(1);} break;
			case 3: LOOP(3) {Z(0);Z(1);Z(2);} break;
			case 4: LOOP(4) {Z(0);Z(1);Z(2);Z(3);} break;
			default:LOOP(chans) {for (int k=0; k<chans; k++) Z(k);}break;
			}
			#undef LOOP
			go->send(rowsize2,buf);
		}
	}
	#undef Z
} GRID_END

GRID_INPUT(1,scale) {prepare_scale_factor();} GRID_END

\end class {install("#downscale_by",2,1); add_creator("@downscale_by");}

//****************************************************************
\class GridLayer {
	P<Grid> r;
	\constructor () {r.but(expect_rgb_picture); r=new Grid(Dim(1,1,3),int32_e,true);}
	\grin 0 int
	\grin 1 int
};

GRID_INLET(0) {
	SAME_TYPE(in,r);
	Dim &a = in.dim;
	expect_rgba_picture(a,in.nt);
	if (a[1]!=r->dim[1]) RAISE("same width please");
	if (a[0]!=r->dim[0]) RAISE("same height please");
	in.set_chunk(2);
	go=new GridOut(this,0,r->dim);
} GRID_FLOW {
	T *rr = ((T *)*r) + in.dex*3/4;
	T foo[n*3/4];
#define COMPUTE_ALPHA(c,a) \
	foo[j+c] = (data[i+c]*data[i+a] + rr[j+c]*(256-data[i+a])) >> 8
	for (int i=0,j=0; i<n; i+=4,j+=3) {
		COMPUTE_ALPHA(0,3);
		COMPUTE_ALPHA(1,3);
		COMPUTE_ALPHA(2,3);
	}
#undef COMPUTE_ALPHA
	go->send(n*3/4,foo);
} GRID_END

GRID_INPUT(1,r) {} GRID_END

\end class {install("#layer",2,1); add_creator("@layer");}

// ****************************************************************
// pad1,pad2 only are there for 32-byte alignment
struct Line {int32 y1,x1,y2,x2,x,m,ox,dir;};

static CONSTRAINT(expect_polygon) {if (d.n!=2 || d[1]!=2) RAISE("expecting Dim[n,2] polygon");}

enum DrawMode {DRAW_FILL,DRAW_LINE,DRAW_POINT};
enum OmitMode {OMIT_NONE,OMIT_LAST,OMIT_ODD};
enum Rule {RULE_ODDEVEN,RULE_WINDING,RULE_MULTI};
DrawMode convert(const t_atom2 &x, DrawMode *foo) {
	t_symbol *s = convert(x,(t_symbol **)0);
	if (s==gensym("fill")) return DRAW_FILL;
	if (s==gensym("line")) return DRAW_LINE;
	if (s==gensym("point")) return DRAW_POINT;
	RAISE("unknown DrawMode '%s' (want fill or line)",s->s_name);
}
OmitMode convert(const t_atom2 &x, OmitMode *foo) {
	t_symbol *s = convert(x,(t_symbol **)0);
	if (s==gensym("none")) return OMIT_NONE;
	if (s==gensym("last")) return OMIT_LAST;
	if (s==gensym("odd"))  return OMIT_ODD;
	RAISE("unknown OmitMode '%s' (want none or last or odd)",s->s_name);
}
Rule convert(const t_atom2 &x, Rule *foo) {
	t_symbol *s = convert(x,(t_symbol **)0);
	if (s==gensym("oddeven")) return RULE_ODDEVEN;
	if (s==gensym("winding")) return RULE_WINDING;
	if (s==gensym("multi"))    return RULE_MULTI;
	RAISE("unknown Rule '%s' (want oddeven or winding or multi)",s->s_name);
}
\class DrawPolygon {
	\attr Numop2 *op;
	\attr P<Grid> color;
	\attr P<Grid> polygon;
	\attr DrawMode draw;
	\attr OmitMode omit;
	\attr Rule rule; // unimplemented
	P<Grid> color2;
	P<Grid> lines;
	int lines_start;
	int lines_stop;
	\constructor (Numop2 *op=op_put, Grid *color=0, Grid *polygon=0) {
		draw=DRAW_FILL;
		omit=OMIT_NONE;
		rule=RULE_ODDEVEN;
		this->color  .but(expect_one_dim);
		this->polygon.but(expect_polygon);
		this->op = op;
		this->color   = color   ? color   : new Grid(Dim(1)  ,int32_e,true);
		this->polygon = polygon ? polygon : new Grid(Dim(0,2),int32_e,true);
		changed();
	}
	\grin 0
	\grin 1
	\grin 2 int32
	void init_lines();
	void changed(t_symbol *s=0) {init_lines();}
};
void DrawPolygon::init_lines () {
	if (!polygon) return;
	int tnl = polygon->dim[0]; // total number of vertices
	int nl = draw==DRAW_FILL ? tnl : omit==OMIT_LAST ? max(0,tnl-1) : omit==OMIT_ODD ? (tnl+1)/2 : tnl; // number of lines to draw
	lines=new Grid(Dim(nl,8), int32_e);
	Line *ld = (Line *)(int32 *)*lines;
	int32 *pd = *polygon;
	for (int i=0,j=0; i<nl; i++) {
		ld[i].y1 = pd[j+0];
		ld[i].x1 = pd[j+1];
		j=(j+2)%(2*tnl);
		ld[i].y2 = pd[j+0];
		ld[i].x2 = pd[j+1];
		ld[i].dir = cmp(ld[i].y1,ld[i].y2);
		if (omit==OMIT_ODD) j=(j+2)%(2*tnl);
		if (draw!=DRAW_POINT) if (ld[i].y1>ld[i].y2) memswap((int32 *)(ld+i)+0,(int32 *)(ld+i)+2,2);
		long dy = ld[i].y2-ld[i].y1;
		long dx = ld[i].x2-ld[i].x1;
		ld[i].m = dy ? (dx<<16)/dy : 0;
	}
}

static int order_by_starting_scanline (const void *a, const void *b) {return ((Line *)a)->y1 - ((Line *)b)->y1;}
static int order_by_column            (const void *a, const void *b) {return ((Line *)a)->x  - ((Line *)b)->x;}

GRID_INLET(0) {
	SAME_TYPE(in,color);
	if (in.dim.n!=3) RAISE("expecting 3 dimensions");
	if (in.dim[2]!=color->dim[0]) RAISE("image does not have same number of channels (%d) as stored color (%d)",
	    in.dim[2], color->dim[0]);
	go=new GridOut(this,0,in.dim,in.nt);
	lines_start = lines_stop = 0;
	in.set_chunk(1);
	int nl = lines->dim[0];
	qsort((int32 *)*lines,nl,sizeof(Line),order_by_starting_scanline);
	int cn = color->dim.prod();
	color2=new Grid(Dim(cn*16), color->nt);
	for (int i=0; i<16; i++) COPY((T *)*color2+cn*i,(T *)*color,cn);
} GRID_FLOW {
	int nl = lines->dim[0];
	Line *ld = (Line *)(int32 *)*lines;
	int f = in.dim.prod(1);
	int y = in.dex/f;
	int cn = color->dim.prod();
	T *cd = (T *)*color2;
	while (n) {
		while (lines_stop != nl && ld[lines_stop].y1<=y) {
			Line &l = ld[lines_stop];
			l.x = l.x1 + (((y-l.y1)*l.m + 0x8000)>>16);
			lines_stop++;
		}
		if (draw!=DRAW_POINT) {
			int fudge = draw==DRAW_FILL?0:1;
			for (int i=lines_start; i<lines_stop; i++) {
				if (ld[i].y2<=y-fudge) {memswap(ld+i,ld+lines_start,1); lines_start++;}
			}
		}
		if (lines_start == lines_stop) {
			go->send(f,data);
		} else {
			int32 xl = in.dim[1];
			T data2[f];
			COPY(data2,data,f);
			for (int i=lines_start; i<lines_stop; i++) {
				Line &l = ld[i];
				l.ox = l.x;
				l.x = l.x1 + (((y-l.y1)*l.m + 0x8000)>>16);
			}
			if (draw!=DRAW_POINT) qsort(ld+lines_start,lines_stop-lines_start,sizeof(Line),order_by_column);
			if (draw==DRAW_FILL) {
				if (rule==RULE_ODDEVEN) {
					for (int i=lines_start; i<lines_stop-1; i+=2) {
						int xs = max(ld[i].x,(int32)0);
						int xe = min(ld[i+1].x,xl);
						if (xs<xe) {
							while (xe-xs>=16) {op->zip(16*cn,data2+cn*xs,cd); xs+=16;}
							op->zip((xe-xs)*cn,data2+cn*xs,cd);
						}
					}
				} else {
					for (int k=0,i=lines_start; i<lines_stop-1; i++) {
						int xs = max(ld[i].x,(int32)0);
						int xe = min(ld[i+1].x,xl);
						k += ld[i].dir;
						if (xs<xe) {
							int q = abs(k);
							while (xe-xs>=16) {for (int w=0; w<q; w++) op->zip(16*cn,data2+cn*xs,cd); xs+=16;}
							for (int w=0; w<q; w++) op->zip((xe-xs)*cn,data2+cn*xs,cd);
						}
					}
				}
			} else if (draw==DRAW_LINE) {
				for (int i=lines_start; i<lines_stop; i++) {
					if (ld[i].y1==ld[i].y2) ld[i].ox=ld[i].x2;
					int xs = min(ld[i].x,ld[i].ox);
					int xe = max(ld[i].x,ld[i].ox);
					if (xs==xe) xe++;
					if ((xs<0 && xe<0) || (xs>=xl && xe>=xl)) continue;
					xs = max(0,xs);
					xe = min(xl,xe);
					while (xe-xs>=16) {op->zip(16*cn,data2+cn*xs,cd); xs+=16;}
					op->zip((xe-xs)*cn,data2+cn*xs,cd);
				}
			} else {
				for (int i=lines_start; i<lines_stop; i++) {
					if (y!=ld[i].y1) continue;
					int xs=ld[i].x1;
					int xe=xs+1;
					if (xs<0 || xs>=xl) continue;
					op->zip((xe-xs)*cn,data2+cn*xs,cd);
				}
				lines_start=lines_stop;
			}
			go->send(f,data2);
		}
		n-=f;
		data+=f;
		y++;
	}
} GRID_END
GRID_INPUT(1,color) {} GRID_END
GRID_INPUT(2,polygon) {init_lines();} GRID_END
\end class {install("#draw_polygon",3,1); add_creator("@draw_polygon");}

//****************************************************************
static CONSTRAINT(expect_position) {
	if (d.n!=1) RAISE("position should have 1 dimension, not %d", d.n);
	if (d[0]!=2) RAISE("position dim 0 should have 2 elements, not %d", d[0]);
}

\class DrawImage {
	\attr Numop2 *op;
	\attr P<Grid> image;
	\attr P<Grid> position;
	\attr bool alpha;
	\attr bool tile;
	\constructor (Numop2 *op=op_put, Grid *image=0, Grid *position=0) {
		alpha=false; tile=false;
		this->op = op;
		this->position.but(expect_position);
		this->image   .but(expect_picture);
		this->image    = image    ? image    : new Grid(Dim(1,1,3),int32_e,true);
		this->position = position ? position : new Grid(Dim(2),int32_e,true);
	}
	\grin 0
	\grin 1
	\grin 2 int32
	// draw row # ry of right image in row buffer buf, starting at xs
	// overflow on both sides has to be handled automatically by this method
	template <class T> void draw_segment(T *obuf, T *ibuf, int ry, int x0);
};

#define COMPUTE_ALPHA(c,a) obuf[j+(c)] = ibuf[j+(c)] + (rbuf[a])*(obuf[j+(c)]-ibuf[j+(c)])/256;
#define COMPUTE_ALPHA4(b) \
	COMPUTE_ALPHA(b+0,b+3); \
	COMPUTE_ALPHA(b+1,b+3); \
	COMPUTE_ALPHA(b+2,b+3); \
	obuf[b+3] = rbuf[b+3] + (255-rbuf[b+3])*(ibuf[j+b+3])/256;

template <class T> void DrawImage::draw_segment(T *obuf, T *ibuf, int ry, int x0) {
	if (ry<0 || ry>=image->dim[0]) return; // outside of image
	int sx = in[0]->dim[1], rsx = image->dim[1];
	int sc = in[0]->dim[2], rsc = image->dim[2];
	T *rbuf = (T *)*image + ry*rsx*rsc;
	if (x0>sx || x0<=-rsx) return; // outside of buffer
	int n=rsx;
	if (x0+n>sx) n=sx-x0;
	if (x0<0) { rbuf-=rsc*x0; n+=x0; x0=0; }
	if (alpha && rsc==4 && sc==3) { // RGB by RGBA //!@#$ optimise
		int j=sc*x0;
		for (; n; n--, rbuf+=4, j+=3) {
			op->zip(sc,obuf+j,rbuf); COMPUTE_ALPHA(0,3); COMPUTE_ALPHA(1,3); COMPUTE_ALPHA(2,3);
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

GRID_INLET(0) {
	SAME_TYPE(in,image);
	if (in.dim.n!=3) RAISE("expecting 3 dimensions");
	if (image->dim.n!=3) RAISE("expecting 3 dimensions in right_hand");
	int lchan = in.dim[2];
	int rchan = image->dim[2];
	if (alpha && rchan!=4) RAISE("alpha mode works only with 4 channels in right_hand");
	if (lchan != rchan-(alpha?1:0) && lchan != rchan) {
		RAISE("right_hand has %d channels, alpha=%d, left_hand has %d, expecting %d or %d",
			rchan, alpha?1:0, lchan, rchan-(alpha?1:0), rchan);
	}
	go=new GridOut(this,0,in.dim,in.nt);
	in.set_chunk(1);
} GRID_FLOW {
	int f = in.dim.prod(1);
	int y = in.dex/f;
	if (position->nt != int32_e) RAISE("position has to be int32");
	int py = ((int32*)*position)[0], rsy = image->dim[0];
	int px = ((int32*)*position)[1], rsx = image->dim[1], sx=in.dim[1];
	for (; n; y++, n-=f, data+=f) {
		int ty = div2(y-py,rsy);
		if (tile || ty==0) {
			T data2[f];
			COPY(data2,data,f);
			if (tile) {
				for (int x=px-div2(px+rsx-1,rsx)*rsx; x<sx; x+=rsx) {
					draw_segment(data2,data,mod(y-py,rsy),x);
				}
			} else {
				draw_segment(data2,data,y-py,px);
			}
			go->send(f,data2);
		} else {
			go->send(f,data);
		}
	}
} GRID_END

GRID_INPUT(1,image) {} GRID_END
GRID_INPUT(2,position) {} GRID_END

\end class {install("#draw_image",3,1); add_creator("@draw_image");}

//****************************************************************
// Dim[*A],Dim[*B],Dim[C,size(A)-size(B)] -> Dim[*A]

/* NOT FINISHED */
\class GridDrawPoints {
	\attr Numop2 *op;
	\attr P<Grid> color;
	\attr P<Grid> points;
	\grin 0
	\grin 1 int32
	\grin 2 int32
	\constructor (Numop2 *op=op_put, Grid *color=0, Grid *points=0) {
		this->op = op;
		this->color  = color  ? color  : new Grid(Dim(),int32_e,true);
		this->points = points ? points : new Grid(Dim(),int32_e,true);
	}
};

GRID_INPUT(1,color) {} GRID_END
GRID_INPUT(2,points) {} GRID_END

GRID_INLET(0) {
	SAME_TYPE(in,color);
	go=new GridOut(this,0,in.dim,in.nt);
	if (points->dim.n!=2) RAISE("points should be a 2-D grid");
	if (points->dim[1] != in.dim.n - color->dim.n) RAISE("wrong number of dimensions");
	in.set_chunk(0);
} GRID_FLOW {
	long m = points->dim[1];
	long cn = in.dim.prod(-color->dim.n); /* size of color (RGB=3, greyscale=1, ...) */
	int32 *pdata = (int32 *)points->data;
	T *cdata = (T *)color->data;
	for (long i=0; i<n; i++) {
		long off = 0;
		for (long k=0; k>m; k++) off = off*in.dim[k] + pdata[i*points->dim[1]+k];
		off *= cn;
		for (long j=0; j<cn; j++) data[off+j] = cdata[j];
	}
//	go->send(data);
} GRID_END
\end class {install("#draw_points",3,1);}

//****************************************************************
\class GridNoiseGateYuvs {
	\grin 0
	int thresh;
	\decl 1 float(int v) {thresh=v;}
	\constructor (int v=0) {thresh=v;}
};

GRID_INLET(0) {
	if (in.dim.n!=3) RAISE("requires 3 dimensions: dim(y,x,3)");
	if (in.dim[2]!=3) RAISE("requires 3 channels");
	go=new GridOut(this,0,in.dim,in.nt);
	in.set_chunk(2);
} GRID_FLOW {
	T tada[n];
	for (long i=0; i<n; i+=3) {
		if (data[i+0]<=thresh) {
			tada[i+0]=0;         tada[i+1]=0;         tada[i+2]=0;
		} else {
			tada[i+0]=data[i+0]; tada[i+1]=data[i+1]; tada[i+2]=data[i+2];
		}
	}
	go->send(n,tada);
} GRID_END

\end class {install("#noise_gate_yuvs",2,1);}

//****************************************************************

\class GridPack {
	int n;
	P<Grid> a;
	\attr NumberTypeE cast;
	\constructor (t_atom2 d=2, NumberTypeE nt=int32_e) {
		Dim dim;
		if (d.a_type==A_LIST) {
			int dn = binbuf_getnatom(d);
			t_atom2 *da = (t_atom2 *)binbuf_getvec(d);
			int dv[dn];
			for (int i=0; i<dn; i++) {dv[i]=da[i]; if (dv[i]<1) RAISE("size is not positive ??");}
			dim = Dim(dn,dv);
		} else dim = Dim(int(d));
		n = dim.prod();
		if (n>64) RAISE("don't you think that %d is too many inlets?",n);
		a = new Grid(dim,float32_e,true);
		ninlets_set(n);
		cast = nt;
	}
	\decl n set   (int inlet, float f) {
		if (inlet>=n) RAISE("what???");
		((float *)*a)[inlet] = f;
	}
	\decl n float (int inlet, float f) {_n_set(inlet,f); _0_bang();}
	\decl 0 bang () {
		go=new GridOut(this,0,a->dim,cast);
		go->send(n,(float *)*a);
	}
};
\end class {install("#pack",1,1); add_creator("@pack");}

\class GridUnpack {
	\constructor (int n=2) {
		if (n<1) RAISE("n=%d must be at least 1",n);
		if (n>32) RAISE("n=%d is too many?",n);
		noutlets_set(n);
	}
	\grin 0
};
GRID_INLET(0) {
	if (in.dim.prod()!=noutlets) RAISE("expect a grid containing a total of %d values (got %d)", noutlets, in.dim.prod());
	in.set_chunk(0);
} GRID_FLOW {
	for (int i=n-1; i>=0; i--) out[i](float(data[i]));
} GRID_END
\end class {install("#unpack",1,0);}

//****************************************************************

\class GridRotatificator {
	int angle;
	int from, to, n;
	\attr NumberTypeE cast;
	\constructor (int from=0, int to=1, int n=2) {cast=int32_e; angle=0; _0_axis(from,to,n);}
	\decl 0 float (int scale) {
		float32 rotator[n*n];
		for (int i=0; i<n; i++) for (int j=0; j<n; j++) rotator[i*n+j] = scale * (i==j);
		float th = angle * M_PI / 18000;
		for (int i=0; i<2; i++) for (int j=0; j<2; j++)
			rotator[(i?to:from)*n+(j?to:from)] = round(scale*cos(th+(j-i)*M_PI/2));
		GridOut go(this,0,Dim(n,n),cast);
		go.send(n*n,rotator);
	}
	\decl 0 axis (int from, int to, int n) {
		if (n<0) RAISE("n-axis number incorrect");
		if (from<0 || from>=n) RAISE("from-axis number incorrect");
		if (to  <0 || to  >=n) RAISE(  "to-axis number incorrect");
		this->from = from;
		this->  to =   to;
		this->   n =    n;
	}
	\decl 1 float(int angle) {this->angle = angle;}
};
\end class {install("#rotatificator",2,1);}

static CONSTRAINT(expect_min_one_dim) {
	if (d.n<1) RAISE("expecting at least one dimension, got %s",d.to_s());}

#define OP(x) dynamic_cast<Numop2 *>(op_dict[string(#x)])
\class GridClusterAvg {
	\attr int numClusters;
	\attr P<Grid> r;
	\attr P<Grid> sums;
	\attr P<Grid> counts;
	\constructor (int v) {
		_1_float(v);
		r.but(expect_min_one_dim);
		r = new Grid(Dim(0));
	}
	\decl 1 float (int v) {numClusters = v;}
	\grin 0 int32
	\grin 2
	template <class T> void make_stats (long n, int32 *ldata, T *rdata) {
		int32 chans = r->dim[r->dim.n-1];
		T     *sdata = (T     *)*sums;
		int32 *cdata = (int32 *)*counts;
		for (int i=0; i<n; i++, ldata++, rdata+=chans) {
			if (*ldata<0 || *ldata>=numClusters) RAISE("value out of range in left grid");
			OP(+)->zip(chans,sdata+(*ldata)*chans,rdata);
			cdata[*ldata]++;
		}
		for (int i=0; i<numClusters; i++) OP(/)->map(chans,sdata+i*chans,(T)cdata[i]);
		go = new GridOut(this,1,counts->dim,counts->nt); go->send(counts->dim.prod(),(int32 *)*counts);
		go = new GridOut(this,0,  sums->dim,  sums->nt); go->send(  sums->dim.prod(),(T     *)*  sums);
	}
};

GRID_INLET(0) {
	int32 v[r->dim.n];
	COPY(v,r->dim.v,r->dim.n-1);
	v[r->dim.n-1]=1;
	Dim t = Dim(r->dim.n,v);
	if (t != in.dim) RAISE("left %s must be equal to right %s except last dimension should be 1",in.dim.to_s(),r->dim.to_s());
	in.set_chunk(0);
	int32 w[2] = {numClusters,r->dim[r->dim.n-1]};
	sums   = new Grid(Dim(2,w),r->nt,  true);
	counts = new Grid(Dim(1,w),int32_e,true);
} GRID_FLOW {
	#define FOO(U) make_stats(n,data,(U *)*r);
	TYPESWITCH(r->nt,FOO,)
	#undef FOO
} GRID_END
GRID_INPUT(2,r) {
} GRID_END

\end class {install("#cluster_avg",3,2);}

//****************************************************************

static Numop *op_os8;
\class GridLopSpace {
	\attr bool reverse;
	\attr int which_dim;
	\attr P<Grid> r;
	\grin 0
	\grin 1
	P<Grid> r2;
	\constructor (int which_dim=0, Grid *r=0) {this->which_dim = which_dim; this->r=r; reverse=false;}
};
template <class T> inline T       shr8r (T       a) {return (a+128)>>8;}
template <>        inline float32 shr8r (float32 a) {return a/256.0;}
template <>        inline float64 shr8r (float64 a) {return a/256.0;}
GRID_INLET(0) {
	if (in.dim.n<2) RAISE("at least 2 dimensions");
	int w = which_dim; if (w<0) w+=in.dim.n;
	if (w<0 || w>=in.dim.n) RAISE("dim number %d does not exist on %d-dimensional grids", which_dim,in.dim.n);
	if (!r) {t_atom2 a[1] = {256.f}; r=new Grid(1,(t_atom *)a);}
	SAME_TYPE(in,r);
	go = new GridOut(this,0,in.dim,in.nt);
	in.set_chunk(w);
	int sxc = in.dim.prod(w);
	size_t rn = r->dim.prod();
	if (rn==1) sxc=1; // fudge
	r2 = new Grid(Dim(sxc),in.nt);
	T *rdata = (T *)*r2;
	for (int i=0; i<sxc; i++) rdata[i] = ((T *)*r)[mod(i,rn)];
} GRID_FLOW {
	int w = which_dim; if (w<0) w+=in.dim.n;
	int sc = in.dim.prod(w+1);
	int sxc = in.dim.prod(w);
	T *rdata = (T *)*r2;
	size_t rn = r->dim.prod();
	T rvalue = *(T *)*r;
	for (;n;n-=sxc, data+=sxc) {
	    T tada[sxc+sc];
	    if (reverse) {
		CLEAR(tada+sxc,sc);
		if (rn==1) for (int i=sxc-1; i>=0; i--) tada[i   ] = shr8r(tada[i+sc]*256 + (data[i]-tada[i+sc])*rvalue  );
		else       for (int i=sxc-1; i>=0; i--) tada[i   ] = shr8r(tada[i+sc]*256 + (data[i]-tada[i+sc])*rdata[i]);
		go->send(sxc,tada);
	    } else {
		CLEAR(tada,sc);
		if (rn==1) for (int i=0; i<sxc; i++)    tada[i+sc] = shr8r(tada[i   ]*256 + (data[i]-tada[i   ])*rvalue  );
		else       for (int i=0; i<sxc; i++)    tada[i+sc] = shr8r(tada[i   ]*256 + (data[i]-tada[i   ])*rdata[i]);
		go->send(sxc,tada+sc);
	    }
	}
} GRID_END
GRID_INPUT(1,r) {} GRID_END
\end class {install("#lop_space",2,1);}

//****************************************************************

\class GridTabread {
	t_symbol *t;
	\constructor (t_symbol *table=&s_) {
		t = table;
	}
	\decl 0 set (t_symbol *s) { t = s; }
	\grin 0
};

GRID_INLET(0) {	
	go = new GridOut(this,0,in.dim,float32_e);
} GRID_FLOW {
	t_garray *a = (t_garray *)pd_findbyclass(t, garray_class); if (!a) RAISE("%s: no such array", t->s_name);
	int npoints;
	t_word *vec;
	float32 tada[n];
	
	if (!garray_getfloatwords(a, &npoints, &vec)) RAISE("%s: bad template for tabread", t->s_name);
	for (int i=0; i<n; i++) {
		int index = clip(int(data[i]),0,npoints-1);
		tada[i] = (npoints ? vec[index].w_float : 0);
	}
	go->send(n,tada);
} GRID_END

\end class {install("#tabread",1,1);}

//****************************************************************

\class GridTabwrite {
	t_symbol *t;
	P<Grid> r;
	\constructor (t_symbol *table=&s_) {
		t = table;
	}
	\decl 0 set (t_symbol *s) { t = s; }
	\grin 0
	\grin 1 int32
};
GRID_INLET(0) {	
	if (!r) RAISE("need right-hand grid first !");
	in.set_chunk(0); // because for-loop is over nidx instead of n
} GRID_FLOW {
	int32 *rdata = (int32 *)*r;
	int npoints, nval=in.dim.prod(), nidx=r->dim.prod();
	t_garray *a = (t_garray *)pd_findbyclass(t, garray_class); if (!a) RAISE("%s: no such array", t->s_name);
	t_word *vec;

	if (!garray_getfloatwords(a, &npoints, &vec)) RAISE("%s: bad template for tabread", t->s_name);
	for (int i=0; i<nidx; i++) {
		int j = clip(int(rdata[i]),0,npoints-1);
		// if there are less values than there are indices, we loop over the value list
		// % is correct because i>=0 and it's faster than mod().
		vec[j].w_float = data[i%nval];
	}
} GRID_FINISH {
	t_garray *a = (t_garray *)pd_findbyclass(t, garray_class); if (!a) RAISE("%s: no such array", t->s_name);
	garray_redraw(a);
} GRID_END

GRID_INPUT(1,r) {} GRID_END

\end class {install("#tabwrite",2,0);}

//****************************************************************
\class GridCompress {
	void *tmp;
	P<Grid> r;
	\constructor (Grid *r=0) {
		this->r.but(expect_one_dim);
		this->r=r?r:new Grid(Dim(0),int32_e,true);
	}
	\grin 0
	\grin 1 int32
};
GRID_INLET(0) {
	if (in.dim.n!=1) RAISE("need 1 dimension");
	if (in.dim!=r->dim) RAISE("left dimension needs to be the same as right dimension");
	tmp = new vector<T>;
} GRID_FLOW {
	vector<T> &tmp2 = *(vector<T> *)tmp;
	int32 *data2 = (int32 *)*r+in.dex;
	for (typeof(n) i=0; i<n; i++) if (data2[i]) tmp2.push_back(data[i]);
} GRID_FINISH {
	vector<T> &tmp2 = *(vector<T> *)tmp;
	GridOut out(this,0,Dim(tmp2.size()),in.nt);
	out.send(tmp2.size(),&tmp2[0]);
	delete (vector<T> *)tmp;
} GRID_END
GRID_INPUT(1,r) {} GRID_END
\end class {install("#compress",2,1);}

//****************************************************************
void startup_classes3 () {
	op_os8 = OP(*>>8);
	\startall
}
