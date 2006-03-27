/*
	$Id$

	GridFlow
	Copyright (c) 2001-2006 by Mathieu Bouchard

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

#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include "grid.h.fcs"

// BAD HACK: GCC complains: unimplemented (--debug mode only)
#ifdef HAVE_DEBUG
#define SCOPY(a,b,n) COPY(a,b,n)
#else
#define SCOPY(a,b,n) SCopy<n>::f(a,b)
#endif

template <long n> class SCopy {
public: template <class T> static inline void __attribute__((always_inline)) f(T * a, T * b) {
		*a=*b; SCopy<n-1>::f(a+1,b+1);}};
template <> class SCopy<0> {
public: template <class T> static inline void __attribute__((always_inline)) f(T * a, T * b) {}};

/*template <> class SCopy<4> {
public: template <class T>
	static inline void __attribute__((always_inline)) f(T * a, T * b) {
		*a=*b; SCopy<3>::f(a+1,b+1);}
	static inline void __attribute__((always_inline)) f(uint8 * a, uint8 * b)
	{ *(int32 *)a=*(int32 *)b; }
};*/

Numop *op_add, *op_sub, *op_mul, *op_div, *op_mod, *op_shl, *op_and, *op_put;

static void expect_dim_dim_list (P<Dim> d) {
	if (d->n!=1) RAISE("dimension list should be Dim[n], not %s",d->to_s());}
//static void expect_min_one_dim (P<Dim> d) {
//	if (d->n<1) RAISE("minimum 1 dimension");}
static void expect_max_one_dim (P<Dim> d) {
	if (d->n>1) { RAISE("expecting Dim[] or Dim[n], got %s",d->to_s()); }}
//static void expect_exactly_one_dim (P<Dim> d) {
//	if (d->n!=1) { RAISE("expecting Dim[n], got %s",d->to_s()); }}

//****************************************************************
\class GridCast < GridObject
struct GridCast : GridObject {
	\attr NumberTypeE nt;
	\decl void initialize (NumberTypeE nt);
	\grin 0
};

GRID_INLET(GridCast,0) {
	out = new GridOutlet(this,0,in->dim,nt);
} GRID_FLOW {
	out->send(n,data);
} GRID_END

\def void initialize (NumberTypeE nt) {
	rb_call_super(argc,argv);
	this->nt = nt;
}

\classinfo { IEVAL(rself,"install '#cast',1,1"); }
\end class GridCast

//****************************************************************
//{ ?,Dim[B] -> Dim[*Cs] }
// out0 nt to be specified explicitly
\class GridImport < GridObject
struct GridImport : GridObject {
	\attr NumberTypeE cast;
	\attr P<Dim> dim; // size of grids to send
	PtrGrid dim_grid;
	GridImport() { dim_grid.constrain(expect_dim_dim_list); }
	~GridImport() {}
	\decl void initialize(Ruby x, NumberTypeE cast=int32_e);
	\decl void _0_reset();
	\decl void _0_symbol(Symbol x);
	\decl void _0_list(...);
	\decl void _1_per_message();
	\grin 0
	\grin 1 int32
	template <class T> void process (long n, T *data) {
		while (n) {
			if (!out || !out->dim) out = new GridOutlet(this,0,dim?dim:in[0]->dim,cast);
			long n2 = min((long)n,out->dim->prod()-out->dex);
			out->send(n2,data);
			n-=n2; data+=n2;
		}
	}
};

GRID_INLET(GridImport,0) {} GRID_FLOW { process(n,data); } GRID_END
GRID_INPUT(GridImport,1,dim_grid) { dim = dim_grid->to_dim(); } GRID_END

\def void _0_symbol(Symbol x) {
	const char *name = rb_sym_name(x);
	long n = strlen(name);
	if (!dim) out=new GridOutlet(this,0,new Dim(n));
	process(n,(uint8 *)name);
}

\def void _0_list(...) {
	if (in.size()<1 || !in[0]) _0_grid(0,0); //HACK: enable grid inlet...
	in[0]->from_ruby_list(argc,argv,cast);
}

\def void _1_per_message() { dim=0; dim_grid=0; }

\def void initialize(Ruby x, NumberTypeE cast) {
	rb_call_super(argc,argv);
	this->cast = cast;
	if (argv[0]!=SYM(per_message)) {
		dim_grid=new Grid(argv[0]);
		dim = dim_grid->to_dim();
	}
}

\def void _0_reset() {int32 foo[1]={0}; while (out->dim) out->send(1,foo);}
\classinfo { IEVAL(rself,"install '#import',2,1"); }
\end class GridImport

//****************************************************************
/*{ Dim[*As] -> ? }*/
/* in0: integer nt */
\class GridExport < GridObject
struct GridExport : GridObject {
	\grin 0
};

GRID_INLET(GridExport,0) {
} GRID_FLOW {
	Ruby a[] = { INT2NUM(0), INT2NUM(0) };
	for (int i=0; i<n; i++) {a[1]=R(data[i]).r; send_out(COUNT(a),a);}
} GRID_END
\classinfo { IEVAL(rself,"install '#export',1,1"); }
\end class GridExport

/* **************************************************************** */
/*{ Dim[*As] -> ? }*/
/* in0: integer nt */
\class GridExportList < GridObject
struct GridExportList : GridObject {
	Ruby /*Array*/ list;
	int n;
	\grin 0
};

GRID_INLET(GridExportList,0) {
	long n = in->dim->prod();
	if (n>1000000) RAISE("list too big (%ld elements, max 1000000)", n);
	list = rb_ary_new2(n+2);
	this->n = n;
	rb_ivar_set(rself,SI(@list),list); // keep
	rb_ary_store(list,0,INT2NUM(0));
	rb_ary_store(list,1,bsym._list);
} GRID_FLOW {
	for (int i=0; i<n; i++, data++)
		rb_ary_store(list,in->dex+i+2,R(*data).r);
} GRID_FINISH {
	send_out(rb_ary_len(list),rb_ary_ptr(list));
	list = 0;
	rb_ivar_set(rself,SI(@list),Qnil); // unkeep
} GRID_END

\classinfo { IEVAL(rself,"install '#export_list',1,1"); }
\end class GridExportList

/* **************************************************************** */
// GridStore ("@store") is the class for storing a grid and restituting
// it on demand. The right inlet receives the grid. The left inlet receives
// either a bang (which forwards the whole image) or a grid describing what
// to send.
//{ Dim[*As,B],Dim[*Cs,*Ds] -> Dim[*As,*Ds] }
// in0: integer nt
// in1: whatever nt
// out0: same nt as in1
\class GridStore < GridObject
struct GridStore : GridObject {
	PtrGrid r; // can't be \attr
	PtrGrid put_at; // can't be //\attr
	\attr Numop *op;
	int32 wdex [Dim::MAX_DIM]; // temporary buffer, copy of put_at
	int32 fromb[Dim::MAX_DIM];
	int32 to2  [Dim::MAX_DIM];
	int lsd; // lsd = Last Same Dimension (for put_at)
	int d; // goes with wdex
	\decl void initialize (Grid *r=0);
	\decl void _0_bang ();
	\decl void _1_reassign ();
	\decl void _1_put_at (Grid *index);
	\grin 0 int
	\grin 1
	GridStore() { put_at.constrain(expect_max_one_dim); }
	template <class T> void compute_indices(T *v, long nc, long nd);
};

// takes the backstore of a grid and puts it back into place. a backstore
// is a grid that is filled while the grid it would replace has not
// finished being used.
static void snap_backstore (PtrGrid &r) {
	if (r.next) {r=r.next.p; r.next=0;}
}

template <class T> void GridStore::compute_indices(T * v, long nc, long nd) {
	for (int i=0; i<nc; i++) {
		uint32 wrap = r->dim->v[i];
		bool fast = lowest_bit(wrap)==highest_bit(wrap); // is power of two?
		if (i) {
			if (fast) op_shl->map(nd,v,(T)highest_bit(wrap));
			else      op_mul->map(nd,v,(T)wrap);
		}
		if (fast) op_and->map(nd,v+nd*i,(T)(wrap-1));
		else      op_mod->map(nd,v+nd*i,(T)(wrap));
		if (i) op_add->zip(nd,v,v+nd*i);
	}
}

// !@#$ i should ensure that n is not exceedingly large
// !@#$ worse: the size of the foo buffer may still be too large
GRID_INLET(GridStore,0) {
	// snap_backstore must be done before *anything* else
	snap_backstore(r);
	int na = in->dim->n;
	int nb = r->dim->n;
	int32 v[Dim::MAX_DIM];
	if (na<1) RAISE("must have at least 1 dimension.",na,1,1+nb);
	long nc = in->dim->get(na-1);
	if (nc > nb)
		RAISE("got %d elements in last dimension, expecting <= %d", nc, nb);
	int lastindexable = r->dim->prod()/r->dim->prod(nc) - 1;
	int ngreatest = nt_greatest((T *)0);
	if (lastindexable > ngreatest) {
		RAISE("lastindexable=%d > ngreatest=%d (ask matju)",lastindexable,ngreatest);
	}
	int nd = nb-nc+na-1;
	COPY(v,in->dim->v,na-1);
	COPY(v+na-1,r->dim->v+nc,nb-nc);
	out=new GridOutlet(this,0,new Dim(nd,v),r->nt);
	if (nc>0) in->set_factor(nc);
} GRID_FLOW {
	int na = in->dim->n;
	int nc = in->dim->get(na-1);
	long size = r->dim->prod(nc);
	long nd = n/nc;
	T w[n];
	T *v=w;
	if (sizeof(T)==1 && nc==1 && r->dim->v[0]<=256) {
		// bug? shouldn't modulo be done here?
		v=data;
	} else {
		COPY(v,data,n);
		for (long k=0,i=0; i<nc; i++) for (long j=0; j<n; j+=nc) v[k++] = data[i+j];
		compute_indices(v,nc,nd);
	}
#define FOO(type) { \
	type *p = (type *)*r; \
	if (size<=16) { \
		type *foo = new type[nd*size]; \
		long i=0; \
		switch (size) { \
		case 1: for (; i<nd&-4; i+=4, foo+=4) { \
			foo[0] = p[v[i+0]]; \
			foo[1] = p[v[i+1]]; \
			foo[2] = p[v[i+2]]; \
			foo[3] = p[v[i+3]]; \
		} break; \
		case 2: for (; i<nd; i++, foo+=2) SCOPY(foo,p+2*v[i],2); break; \
		case 3: for (; i<nd; i++, foo+=3) SCOPY(foo,p+3*v[i],3); break; \
		case 4: for (; i<nd; i++, foo+=4) SCOPY(foo,p+4*v[i],4); break; \
		default:; }; \
		for (; i<nd; i++, foo+=size) COPY(foo,p+size*v[i],size); \
		out->give(size*nd,foo-size*nd); \
	} else { \
		for (int i=0; i<nd; i++) out->send(size,p+size*v[i]); \
	} \
}
	TYPESWITCH(r->nt,FOO,)
#undef FOO
} GRID_FINISH {
	if (in->dim->prod()==0) {
		long n = in->dim->prod(0,-2);
		long size = r->dim->prod();
#define FOO(T) while (n--) out->send(size,(T *)*r);
		TYPESWITCH(r->nt,FOO,)
#undef FOO
	}
} GRID_END

GRID_INLET(GridStore,1) {
	NumberTypeE nt = NumberTypeE_type_of(*data);
	if (!put_at) { // reassign
		if (in[0].dim)
			r.next = new Grid(in->dim,nt);
		else
			r = new Grid(in->dim,nt);
		return;
	}
	// put_at ( ... )
	//!@#$ should check types. if (r->nt!=in->nt) RAISE("shoo");
	long nn=r->dim->n, na=put_at->dim->v[0], nb=in->dim->n;
	int32 sizeb[nn];
	for (int i=0; i<nn; i++) { fromb[i]=0; sizeb[i]=1; }
	COPY(wdex       ,(int32 *)*put_at   ,put_at->dim->prod());
	COPY(fromb+nn-na,(int32 *)*put_at   ,na);
	COPY(sizeb+nn-nb,(int32 *)in->dim->v,nb);
	for (int i=0; i<nn; i++) to2[i] = fromb[i]+sizeb[i];
	d=0;
	// find out when we can skip computing indices
	//!@#$ should actually also stop before blowing up packet size
	lsd=nn;
	while (lsd>=nn-in->dim->n) {
		lsd--;
		int cs = in->dim->prod(lsd-nn+in->dim->n);
		if (cs>GridOutlet::MAX_PACKET_SIZE || fromb[lsd]!=0 || sizeb[lsd]!=r->dim->v[lsd]) break;
	}
	lsd++;
	long cs = in->dim->prod(lsd-nn+in->dim->n);
	in->set_factor(cs);
} GRID_FLOW {
	if (!put_at) { // reassign
		COPY(((T *)*(r.next ? r.next.p : &*r.p))+in->dex, data, n);
		return;
	}
	// put_at (...)
	long cs = in->factor(); // chunksize
	int32 v[lsd];
	int32 *x = wdex;
	while (n) {
		// here d is the dim# to reset; d=n for none
		for(;d<lsd;d++) x[d]=fromb[d];
		COPY(v,x,lsd);
		compute_indices(v,lsd,1);
		op->zip(cs,(T *)*r+v[0]*cs,data);
		data+=cs;
		n-=cs;
		// find next set of indices; here d is the dim# to increment
		for(;;d++) {d--; if (d<0) return; x[d]++; if (x[d]<to2[d]) break;}
	}
} GRID_END
\def void _0_bang () { rb_funcall(rself,SI(_0_list),3,INT2NUM(0),SYM(#),INT2NUM(0)); }
\def void _1_reassign () { put_at=0; }
\def void _1_put_at (Grid *index) { put_at=index; }
\def void initialize (Grid *r) {
	rb_call_super(argc,argv);
	this->r = r?r:new Grid(new Dim(),int32_e,true);
	op = op_put;
}
\classinfo { IEVAL(rself,"install '#store',2,1"); }
\end class GridStore

//****************************************************************
//{ Dim[*As]<T> -> Dim[*As]<T> } or
//{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As]<T> }
\class GridOp < GridObject
struct GridOp : GridObject {
	\attr Numop *op;
	PtrGrid r;
	\decl void initialize(Numop *op, Grid *r=0);
	\grin 0
	\grin 1
};

GRID_INLET(GridOp,0) {
	snap_backstore(r);
	SAME_TYPE(in,r);
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_mode(6);
} GRID_ALLOC {
	//out->ask(in->allocn,(T * &)in->alloc,in->allocfactor,in->allocmin,in->allocmax);
} GRID_FLOW {
	T *rdata = (T *)*r;
	long loop = r->dim->prod();
	if (sizeof(T)==8) {
		fprintf(stderr,"1: data=%p rdata=%p\n",data,rdata);
		WATCH(n,data);
	}
	if (loop>1) {
		if (in->dex+n <= loop) {
			op->zip(n,data,rdata+in->dex);
		} else {
			// !@#$ should prebuild and reuse this array when "loop" is small
			T data2[n];
			long ii = mod(in->dex,loop);
			long m = min(loop-ii,n);
			COPY(data2,rdata+ii,m);
			long nn = m+((n-m)/loop)*loop;
			for (long i=m; i<nn; i+=loop) COPY(data2+i,rdata,loop);
			if (n>nn) COPY(data2+nn,rdata,n-nn);
			if (sizeof(T)==8) {
				fprintf(stderr,"2: data=%p data2=%p\n",data,data2);
				WATCH(n,data); WATCH(n,data2);
			}
			op->zip(n,data,data2);
			if (sizeof(T)==8) {WATCH(n,data); WATCH(n,data2);}
		}
	} else {
		op->map(n,data,*rdata);
	}
	out->give(n,data);
} GRID_END

GRID_INPUT2(GridOp,1,r) {} GRID_END
\def void initialize(Numop *op, Grid *r=0) {
  rb_call_super(argc,argv); this->op=op;
  this->r=r?r:new Grid(new Dim(),int32_e,true);
}
\classinfo { IEVAL(rself,"install '#',2,1"); }
\end class GridOp

//****************************************************************
\class GridFold < GridObject
struct GridFold : GridObject {
	\attr Numop *op;
	\attr PtrGrid seed;
	\decl void initialize (Numop *op);
	\grin 0
};

GRID_INLET(GridFold,0) {
	//{ Dim[*As,B,*Cs]<T>,Dim[*Cs]<T> -> Dim[*As,*Cs]<T> }
	if (seed) SAME_TYPE(in,seed);
	int an = in->dim->n;
	int bn = seed?seed->dim->n:0;
	if (an<=bn) RAISE("minimum 1 more dimension than the seed (%d vs %d)",an,bn);
	int32 v[an-1];
	int yi = an-bn-1;
	COPY(v,in->dim->v,yi);
	COPY(v+yi,in->dim->v+an-bn,bn);
	if (seed) SAME_DIM(an-(yi+1),in->dim,(yi+1),seed->dim,0);
	out=new GridOutlet(this,0,new Dim(an-1,v),in->nt);
	long k = seed ? seed->dim->prod() : 1;
	in->set_factor(in->dim->get(yi)*k);
} GRID_FLOW {
	int an = in->dim->n;
	int bn = seed?seed->dim->n:0;
	long yn = in->dim->v[an-bn-1];
	long zn = in->dim->prod(an-bn);
	T buf[n/yn];
	long nn=n;
	long yzn=yn*zn;
	for (long i=0; n; i+=zn, data+=yzn, n-=yzn) {
		if (seed) COPY(buf+i,((T *)*seed),zn);
		else CLEAR(buf+i,zn);
		op->fold(zn,yn,buf+i,data);
	}
	out->send(nn/yn,buf);
} GRID_END

\def void initialize (Numop *op) { rb_call_super(argc,argv); this->op=op; }
\classinfo { IEVAL(rself,"install '#fold',1,1"); }
\end class GridFold

\class GridScan < GridObject
struct GridScan : GridObject {
	\attr Numop *op;
	\attr PtrGrid seed;
	\decl void initialize (Numop *op);
	\grin 0
};

GRID_INLET(GridScan,0) {
	//{ Dim[*As,B,*Cs]<T>,Dim[*Cs]<T> -> Dim[*As,B,*Cs]<T> }
	if (seed) SAME_TYPE(in,seed);
	int an = in->dim->n;
	int bn = seed?seed->dim->n:0;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand");
	if (seed) SAME_DIM(bn,in->dim,an-bn,seed->dim,0);
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_factor(in->dim->prod(an-bn-1));
} GRID_FLOW {
	int an = in->dim->n;
	int bn = seed?seed->dim->n:0;
	long yn = in->dim->v[an-bn-1];
	long zn = in->dim->prod(an-bn);
	long factor = in->factor();
	T buf[n];
	COPY(buf,data,n);
	if (seed) {
		for (long i=0; i<n; i+=factor) op->scan(zn,yn,(T *)*seed,buf+i);
	} else {
		T seed[zn]; CLEAR(seed,zn);
		for (long i=0; i<n; i+=factor) op->scan(zn,yn,      seed,buf+i);
	}
	out->send(n,buf);
} GRID_END

\def void initialize (Numop *op) { rb_call_super(argc,argv); this->op = op; }
\classinfo { IEVAL(rself,"install '#scan',1,1"); }
\end class GridScan

//****************************************************************
// L      is a Dim[*si,sj,    *ss]<T>
// R      is a Dim[    sj,*sk,*ss]<T>
// Seed   is a Dim[           *ss]<T>
// result is a Dim[*si,   *sk,*ss]<T>
// Currently *ss can only be = Dim[]
\class GridInner < GridObject
struct GridInner : GridObject {
	\attr Numop *op;
	\attr Numop *fold;
	\attr PtrGrid seed;
	PtrGrid r;
	PtrGrid r2; // temporary
	bool use_dot;
	GridInner() {}
	\decl void initialize (Grid *r=0);
	\grin 0
	\grin 1
};

// let's see this as a matrix product like L[i,j]*R[j,k] in Einstein notation
//   L: matrix of size si by sj
//   R: matrix of size sj by sk
//  LR: matrix of size si by sk
template <class T> void inner_child_a (T *as, T *bs, int sj, int sk, int chunk) {
	for (int j=0; j<chunk; j++, as+=sk, bs+=sj) op_put->map(sk,as,*bs);}
template <class T, int sk> void inner_child_b (T *as, T *bs, int sj, int chunk) {
	for (int j=0; j<chunk; j++, as+=sk, bs+=sj) op_put->map(sk,as,*bs);}

// Inner product in a Module on the (+,*) Ring
//      | BBBBB
//      j BBBBB
//      | BBBBB
// --j--*---k---
// AAAAA  CCCCC
template <class T> void dot_add_mul (long sk, long sj, T *cs, T *as, T *bs) {
	for (int k=0; k<sk; k++) {
		T c = 0; for (int j=0; j<sj; j++) c+=as[j]*bs[j*sk+k];
		*cs++=c;
	}
}
template <class T, int sj> void dot_add_mul (long sk, T *cs, T *as, T *bs) {
	for (int k=0; k<sk; k++) {
		T c = 0; for (int j=0; j<sj; j++) c+=as[j]*bs[j*sk+k];
		*cs++=c;
	}
}

GRID_INLET(GridInner,0) {
	SAME_TYPE(in,r);
	SAME_TYPE(in,seed);
	P<Dim> a=in->dim, b=r->dim;
	if (a->n<1) RAISE("a: minimum 1 dimension");
	if (b->n<1) RAISE("b: minimum 1 dimension");
	if (seed->dim->n != 0) RAISE("seed must be a scalar");
	int n = a->n+b->n-2;
	SAME_DIM(1,a,a->n-1,b,0);
	int32 v[n];
	COPY(v,a->v,a->n-1);
	COPY(v+a->n-1,b->v+1,b->n-1);
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
	in->set_factor(a->get(a->n-1));
	long sjk=r->dim->prod(), sj=in->factor(), sk=sjk/sj;
	long chunk = GridOutlet::MAX_PACKET_SIZE/sjk;
	T *rdata = (T *)*r;
	r2=new Grid(new Dim(chunk*sjk),r->nt);
	T *buf3 = (T *)*r2;
	for (long i=0; i<sj; i++)
		for (long j=0; j<chunk; j++)
			COPY(buf3+(j+i*chunk)*sk,rdata+i*sk,sk);
	use_dot = op==op_mul && fold==op_add && seed->dim->n==0 && *(T *)*seed==0;
	//gfpost("use_dot=%d",use_dot);
} GRID_FLOW {
    long sjk=r->dim->prod(), sj=in->factor(), sk=sjk/sj;
    long chunk = GridOutlet::MAX_PACKET_SIZE/sjk;
    T buf [chunk*sk];
    T buf2[chunk*sk];
    long off = chunk;
    if (use_dot) {
	while (n) {
		if (chunk*sj>n) chunk=n/sj;
		switch (sj) {
		case 1: for (int i=0; i<chunk; i++) dot_add_mul<T,1>(sk,buf2+sk*i,data+sj*i,(T *)*r); break;
		case 2: for (int i=0; i<chunk; i++) dot_add_mul<T,2>(sk,buf2+sk*i,data+sj*i,(T *)*r); break;
		case 3: for (int i=0; i<chunk; i++) dot_add_mul<T,3>(sk,buf2+sk*i,data+sj*i,(T *)*r); break;
		case 4: for (int i=0; i<chunk; i++) dot_add_mul<T,4>(sk,buf2+sk*i,data+sj*i,(T *)*r); break;
		default:for (int i=0; i<chunk; i++) dot_add_mul(sk,sj,buf2+sk*i,data+sj*i,(T *)*r);
		}
		out->send(chunk*sk,buf2);
		n-=chunk*sj;
		data+=chunk*sj;
	}
    } else {
	while (n) {
		if (chunk*sj>n) chunk=n/sj;
		op_put->map(chunk*sk,buf2,*(T *)*seed);
		for (long i=0; i<sj; i++) {
			switch (sk) {
			case 1:  inner_child_b<T,1>(buf,data+i,sj,chunk); break;
			case 2:  inner_child_b<T,2>(buf,data+i,sj,chunk); break;
			case 3:  inner_child_b<T,3>(buf,data+i,sj,chunk); break;
			case 4:  inner_child_b<T,4>(buf,data+i,sj,chunk); break;
			default: inner_child_a(buf,data+i,sj,sk,chunk);
			}
			op->zip(chunk*sk,buf,(T *)*r2+i*off*sk);
			fold->zip(chunk*sk,buf2,buf);
		}
		out->send(chunk*sk,buf2);
		n-=chunk*sj;
		data+=chunk*sj;
	}
    }
} GRID_FINISH {
	r2=0;
} GRID_END

GRID_INPUT(GridInner,1,r) {} GRID_END

\def void initialize (Grid *r) {
	rb_call_super(argc,argv);
	this->op = op_mul;
	this->fold = op_add;
	this->seed = new Grid(new Dim(),int32_e,true);
	this->r    = r ? r : new Grid(new Dim(),int32_e,true);
}

\classinfo { IEVAL(rself,"install '#inner',2,1"); }
\end class GridInner

/* **************************************************************** */
/*{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As,*Bs]<T> }*/
\class GridOuter < GridObject
struct GridOuter : GridObject {
	\attr Numop *op;
	PtrGrid r;
	\decl void initialize (Numop *op, Grid *r=0);
	\grin 0
	\grin 1
};

GRID_INLET(GridOuter,0) {
	SAME_TYPE(in,r);
	P<Dim> a = in->dim;
	P<Dim> b = r->dim;
	int n = a->n+b->n;
	int32 v[n];
	COPY(v,a->v,a->n);
	COPY(v+a->n,b->v,b->n);
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
} GRID_FLOW {
	long b_prod = r->dim->prod();
	if (b_prod > 4) {
		T buf[b_prod];
		while (n) {
			for (long j=0; j<b_prod; j++) buf[j] = *data;
			op->zip(b_prod,buf,(T *)*r);
			out->send(b_prod,buf);
			data++; n--;
		}
		return;
	}
	n*=b_prod;
	T * buf = new T[n];
	T buf2[b_prod*64];
	for (int i=0; i<64; i++) COPY(buf2+i*b_prod,(T *)*r,b_prod);
	switch (b_prod) {
	#define Z buf[k++]=data[i]
	case 1:	for (long i=0,k=0; k<n; i++) {Z;} break;
	case 2: for (long i=0,k=0; k<n; i++) {Z;Z;} break;
	case 3:	for (long i=0,k=0; k<n; i++) {Z;Z;Z;} break;
	case 4:	for (long i=0,k=0; k<n; i++) {Z;Z;Z;Z;} break;
	default:for (long i=0,k=0; k<n; i++) for (int j=0; j<b_prod; j++, k++) Z;
	}
	#undef Z
	int ch=64*b_prod;
	int nn=(n/ch)*ch;
	for (int j=0; j<nn; j+=ch) op->zip(ch,buf+j,buf2);
	op->zip(n-nn,buf+nn,buf2);
	out->give(n,buf);
} GRID_END

GRID_INPUT(GridOuter,1,r) {} GRID_END

\def void initialize (Numop *op, Grid *r) {
	rb_call_super(argc,argv);
	this->op = op;
	this->r = r ? r : new Grid(new Dim(),int32_e,true);
}

\classinfo { IEVAL(rself,"install '#outer',2,1"); }
\end class GridOuter

//****************************************************************
//{ Dim[]<T>,Dim[]<T>,Dim[]<T> -> Dim[A]<T> } or
//{ Dim[B]<T>,Dim[B]<T>,Dim[B]<T> -> Dim[*As,B]<T> }
\class GridFor < GridObject
struct GridFor : GridObject {
	\attr PtrGrid from;
	\attr PtrGrid to;
	\attr PtrGrid step;
	GridFor () {
		from.constrain(expect_max_one_dim);
		to  .constrain(expect_max_one_dim);
		step.constrain(expect_max_one_dim);
	}
	\decl void initialize (Grid *from, Grid *to, Grid *step);
	\decl void _0_set (Grid *r=0);
	\decl void _0_bang ();
	\grin 0 int
	\grin 1 int
	\grin 2 int
	template <class T> void trigger (T bogus);
};

\def void initialize (Grid *from, Grid *to, Grid *step) {
	rb_call_super(argc,argv);
	this->from=from;
	this->to  =to;
	this->step=step;
}

template <class T>
void GridFor::trigger (T bogus) {
	int n = from->dim->prod();
	int32 nn[n+1];
	T x[64*n];
	T * fromb = (T *)*from;
	T *   tob = (T *)*to  ;
	T * stepb = (T *)*step;
	T to2[n];
	
	for (int i=step->dim->prod()-1; i>=0; i--)
		if (!stepb[i]) RAISE("step must not contain zeroes");
	for (int i=0; i<n; i++) {
		nn[i] = (tob[i] - fromb[i] + stepb[i] - cmp(stepb[i],(T)0)) / stepb[i];
		if (nn[i]<0) nn[i]=0;
		to2[i] = fromb[i]+stepb[i]*nn[i];
	}
	P<Dim> d;
	if (from->dim->n==0) { d = new Dim(*nn); }
	else { nn[n]=n;        d = new Dim(n+1,nn); }
	int total = d->prod();
	out=new GridOutlet(this,0,d,from->nt);
	if (total==0) return;
	int k=0;
	for(int d=0;;d++) {
		// here d is the dim# to reset; d=n for none
		for(;d<n;d++) x[k+d]=fromb[d];
		k+=n;
		if (k==64*n) {out->send(k,x); k=0; COPY(x,x+63*n,n);}
		else {                             COPY(x+k,x+k-n,n);}
		d--;
		// here d is the dim# to increment
		for(;;d--) {
			if (d<0) goto end;
			x[k+d]+=stepb[d];
			if (x[k+d]!=to2[d]) break;
		}
	}
	end: if (k) out->send(k,x);
}

\def void _0_bang () {
	SAME_TYPE(from,to);
	SAME_TYPE(from,step);
	if (!from->dim->equal(to->dim) || !to->dim->equal(step->dim))
		RAISE("dimension mismatch");
#define FOO(T) trigger((T)0);
	TYPESWITCH_JUSTINT(from->nt,FOO,);
#undef FOO
}

\def void _0_set (Grid *r) { from=new Grid(argv[0]); }
GRID_INPUT(GridFor,2,step) {} GRID_END
GRID_INPUT(GridFor,1,to) {} GRID_END
GRID_INPUT(GridFor,0,from) {_0_bang(0,0);} GRID_END
\classinfo { IEVAL(rself,"install '#for',3,1"); }
\end class GridFor

//****************************************************************
\class GridFinished < GridObject
struct GridFinished : GridObject {
	\grin 0
};
GRID_INLET(GridFinished,0) {
	in->set_mode(0);
} GRID_FINISH {
	Ruby a[] = { INT2NUM(0), bsym._bang };
	send_out(COUNT(a),a);
} GRID_END
\classinfo { IEVAL(rself,"install '#finished',1,1"); }
\end class GridFinished

\class GridDim < GridObject
struct GridDim : GridObject {
	\grin 0
};
GRID_INLET(GridDim,0) {
	GridOutlet out(this,0,new Dim(in->dim->n));
	out.send(in->dim->n,in->dim->v);
	in->set_mode(0);
} GRID_END
\classinfo { IEVAL(rself,"install '#dim',1,1"); }
\end class GridDim

\class GridType < GridObject
struct GridType : GridObject {
	\grin 0
};
GRID_INLET(GridType,0) {
	Ruby a[] = { INT2NUM(0), SYM(symbol), number_type_table[in->nt].sym };
	send_out(COUNT(a),a);
	in->set_mode(0);
} GRID_END
\classinfo { IEVAL(rself,"install '#type',1,1"); }
\end class GridType

//****************************************************************
//{ Dim[*As]<T>,Dim[B] -> Dim[*Cs]<T> }
\class GridRedim < GridObject
struct GridRedim : GridObject {
	\attr P<Dim> dim;
	PtrGrid dim_grid;
	PtrGrid temp; // temp->dim is not of the same shape as dim
	GridRedim() { dim_grid.constrain(expect_dim_dim_list); }
	~GridRedim() {}
	\decl void initialize (Grid *d);
	\grin 0
	\grin 1 int32
};

GRID_INLET(GridRedim,0) {
	long a=in->dim->prod(), b=dim->prod();
	if (a<b) temp=new Grid(new Dim(a),in->nt);
	out=new GridOutlet(this,0,dim,in->nt);
} GRID_FLOW {
	long i = in->dex;
	if (!temp) {
		long n2 = min(n,dim->prod()-i);
		if (n2>0) out->send(n2,data);
		// discard other values if any
	} else {
		long n2 = min(n,in->dim->prod()-i);
		COPY((T *)*temp+i,data,n2);
		if (n2>0) out->send(n2,data);
	}
} GRID_FINISH {
	if (!!temp) {
		long a = in->dim->prod(), b = dim->prod();
		if (a) {
			for (long i=a; i<b; i+=a) out->send(min(a,b-i),(T *)*temp);
		} else {
			T foo[1]={0}; for (long i=0; i<b; i++) out->send(1,foo);
		}
	}
	temp=0;
} GRID_END

GRID_INPUT(GridRedim,1,dim_grid) { dim = dim_grid->to_dim(); } GRID_END

\def void initialize (Grid *d) {
	rb_call_super(argc,argv);
	dim_grid=d;
	dim = dim_grid->to_dim();
}

\classinfo { IEVAL(rself,"install '#redim',2,1"); }
\end class GridRedim

//****************************************************************
\class GridJoin < GridObject
struct GridJoin : GridObject {
	\attr int which_dim;
	PtrGrid r;
	\grin 0
	\grin 1
	\decl void initialize (int which_dim=-1, Grid *r=0);
};

GRID_INLET(GridJoin,0) {
	NOTEMPTY(r);
	SAME_TYPE(in,r);
	P<Dim> d = in->dim;
	if (d->n != r->dim->n) RAISE("wrong number of dimensions");
	int w = which_dim;
	if (w<0) w+=d->n;
	if (w<0 || w>=d->n)
		RAISE("can't join on dim number %d on %d-dimensional grids",
			which_dim,d->n);
	int32 v[d->n];
	for (int i=0; i<d->n; i++) {
		v[i] = d->get(i);
		if (i==w) {
			v[i]+=r->dim->v[i];
		} else {
			if (v[i]!=r->dim->v[i]) RAISE("dimensions mismatch: dim #%i, left is %d, right is %d",i,v[i],r->dim->v[i]);
		}
	}
	out=new GridOutlet(this,0,new Dim(d->n,v),in->nt);
	if (d->prod(w)) in->set_factor(d->prod(w));
} GRID_FLOW {
	int w = which_dim;
	if (w<0) w+=in->dim->n;
	long a = in->factor();
	long b = r->dim->prod(w);
	T * data2 = (T *)*r + in->dex*b/a;
	if (a==3 && b==1) {
		int m = n+n*b/a;
		T data3[m];
		T * data4 = data3;
		while (n) {
			SCOPY(data4,data,3); SCOPY(data4+3,data2,1);
			n-=3; data+=3; data2+=1; data4+=4;
		}
		out->send(m,data3);
	} else if (a+b<=16) {
		int m = n+n*b/a;
		T data3[m];
		int i=0;
		while (n) {
			COPY(data3+i,data,a); data+=a; i+=a; n-=a;
			COPY(data3+i,data2,b); data2+=b; i+=b;
		}
		out->send(m,data3);
	} else {
		while (n) {
			out->send(a,data);
			out->send(b,data2);
			data+=a; data2+=b; n-=a;
		}
	}
} GRID_FINISH {
	if (in->dim->prod()==0) out->send(r->dim->prod(),(T *)*r);
} GRID_END

GRID_INPUT(GridJoin,1,r) {} GRID_END

\def void initialize (int which_dim, Grid *r) {
	rb_call_super(argc,argv);
	this->which_dim = which_dim;
	this->r=r;
}

\classinfo { IEVAL(rself,"install '@join',2,1"); }
\end class GridJoin

//****************************************************************
\class GridGrade < GridObject
struct GridGrade : GridObject {
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

GRID_INLET(GridGrade,0) {
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_factor(in->dim->get(in->dim->n-1));
} GRID_FLOW {
	long m = in->factor();
	T* foo[m];
	T  bar[m];
	for (; n; n-=m,data+=m) {
		for (int i=0; i<m; i++) foo[i] = &data[i];
		qsort(foo,m,sizeof(T),(comparator_t)GradeFunction<T>::comparator);
		for (int i=0; i<m; i++) bar[i] = foo[i]-(T *)data;
		out->send(m,bar);
	}
} GRID_END

\classinfo { IEVAL(rself,"install '#grade',1,1"); }
\end class GridGrade

//****************************************************************
//\class GridMedian < GridObject
//****************************************************************

\class GridTranspose < GridObject
struct GridTranspose : GridObject {
	\attr int dim1;
	\attr int dim2;
	int d1,d2,na,nb,nc,nd; // temporaries
	\decl void initialize (int dim1=0, int dim2=1);
	\decl void _1_float (int dim1);
	\decl void _2_float (int dim2);
	\grin 0
};

\def void _1_float (int dim1) { this->dim1=dim1; }
\def void _2_float (int dim2) { this->dim2=dim2; }

GRID_INLET(GridTranspose,0) {
	int32 v[in->dim->n];
	COPY(v,in->dim->v,in->dim->n);
	d1=dim1; d2=dim2;
	if (d1<0) d1+=in->dim->n;
	if (d2<0) d2+=in->dim->n;
	if (d1>=in->dim->n || d2>=in->dim->n || d1<0 || d2<0)
		RAISE("would swap dimensions %d and %d but this grid has only %d dimensions",
			dim1,dim2,in->dim->n);
	memswap(v+d1,v+d2,1);
	if (d1==d2) {
		out=new GridOutlet(this,0,new Dim(in->dim->n,v), in->nt);
	} else {
		nd = in->dim->prod(1+max(d1,d2));
		nc = in->dim->v[max(d1,d2)];
		nb = in->dim->prod(1+min(d1,d2))/nc/nd;
		na = in->dim->v[min(d1,d2)];
		out=new GridOutlet(this,0,new Dim(in->dim->n,v), in->nt);
		in->set_factor(na*nb*nc*nd);
	}
	// Turns a Grid[*,na,*nb,nc,*nd] into a Grid[*,nc,*nb,na,*nd].
} GRID_FLOW {
	T res[na*nb*nc*nd];
	if (dim1==dim2) { out->send(n,data); return; }
	int prod = na*nb*nc*nd;
	for (; n; n-=prod, data+=prod) {
		for (long a=0; a<na; a++)
			for (long b=0; b<nb; b++)
				for (long c=0; c<nc; c++)
					COPY(res +((c*nb+b)*na+a)*nd,
					     data+((a*nb+b)*nc+c)*nd,nd);
		out->send(na*nb*nc*nd,res);
	}
} GRID_END

\def void initialize (int dim1=0, int dim2=1) {
	rb_call_super(argc,argv);
	this->dim1 = dim1;
	this->dim2 = dim2;
}

\classinfo { IEVAL(rself,"install '#transpose',3,1"); }
\end class GridTranspose

//****************************************************************
\class GridReverse < GridObject
struct GridReverse : GridObject {
	\attr int dim1; // dimension to act upon
	int d; // temporaries
	\decl void initialize (int dim1=0);
	\decl void _1_float (int dim1);
	\grin 0
};

\def void _1_float (int dim1) { this->dim1=dim1; }

GRID_INLET(GridReverse,0) {
	d=dim1;
	if (d<0) d+=in->dim->n;
	if (d>=in->dim->n || d<0)
		RAISE("would reverse dimension %d but this grid has only %d dimensions",
			dim1,in->dim->n);
	out=new GridOutlet(this,0,new Dim(in->dim->n,in->dim->v), in->nt);
	in->set_factor(in->dim->prod(d));
} GRID_FLOW {
	long f1=in->factor(), f2=in->dim->prod(d+1);
	while (n) {
		long hf1=f1/2;
		T * data2 = data+f1-f2;
		for (long i=0; i<hf1; i+=f2) memswap(data+i,data2-i,f2);
		out->send(f1,data);
		data+=f1; n-=f1;
	}
} GRID_END

\def void initialize (int dim1=0) {
	rb_call_super(argc,argv);
	this->dim1 = dim1;
}

\classinfo { IEVAL(rself,"install '#reverse',2,1"); }
\end class GridReverse

//****************************************************************
\class GridCentroid < GridObject
struct GridCentroid : GridObject {
	\decl void initialize ();
	\grin 0 int
	int sumx,sumy,sum,y; // temporaries
};

GRID_INLET(GridCentroid,0) {
	if (in->dim->n != 3) RAISE("expecting 3 dims");
	if (in->dim->v[2] != 1) RAISE("expecting 1 channel");
	in->set_factor(in->dim->prod(1));
	out=new GridOutlet(this,0,new Dim(2), in->nt);
	sumx=0; sumy=0; sum=0; y=0;
} GRID_FLOW {
	int sx = in->dim->v[1];
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
	out->send(2,blah);
	rb_funcall(rself,SI(send_out),2,INT2NUM(1),INT2NUM(blah[0]));
	rb_funcall(rself,SI(send_out),2,INT2NUM(2),INT2NUM(blah[1]));
} GRID_END

\def void initialize () {
	rb_call_super(argc,argv);
}

\classinfo { IEVAL(rself,"install '#centroid',1,3"); }
\end class GridCentroid

//****************************************************************
\class GridPerspective < GridObject
struct GridPerspective : GridObject {
	\attr int32 z;
	\grin 0
	\decl void initialize (int32 z=256);
};

GRID_INLET(GridPerspective,0) {
	int n = in->dim->n;
	int32 v[n];
	COPY(v,in->dim->v,n);
	v[n-1]--;
	in->set_factor(in->dim->get(in->dim->n-1));
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
} GRID_FLOW {
	int m = in->factor();
	for (; n; n-=m,data+=m) {
		op_mul->map(m-1,data,(T)z);
		op_div->map(m-1,data,data[m-1]);
		out->send(m-1,data);
	}	
} GRID_END

\def void initialize (int32 z) {rb_call_super(argc,argv); this->z=z; }

\classinfo { IEVAL(rself,"install '#perspective',1,1"); }
\end class GridPerspective

//****************************************************************
\class GridBorder < GridObject
struct GridBorder : GridObject {
	\attr P<Dim> diml;
	\attr P<Dim> dimr;
	PtrGrid diml_grid;
	PtrGrid dimr_grid;
	\grin 0
	\grin 1 int
	\grin 2 int
	\decl void initialize (Grid *dl, Grid *dr);
};

GRID_INLET(GridBorder,0) {
	int n = in->dim->n;
	if (n!=3) RAISE("only 3 dims supported for now");
	if (diml->n != n) RAISE("diml mismatch");
	if (dimr->n != n) RAISE("dimr mismatch");
	if (diml->v[2] || dimr->v[2]) RAISE("can't augment channels (todo)");
	int32 v[n];
	for (int i=0; i<n; i++) v[i]=in->dim->v[i]+diml->v[i]+dimr->v[i];
	in->set_factor(in->dim->prod());
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
} GRID_FLOW {
	int sy = in->dim->v[0];
	int sx = in->dim->v[1]; int zx = sx+diml->v[1]+dimr->v[1];
	int sc = in->dim->v[2]; int zc = sc+diml->v[2]+dimr->v[2];
	int sxc = sx*sc; int zxc = zx*zc;
	int32 duh[zxc];
	for (int x=0; x<zxc; x++) duh[x]=0;
	for (int y=0; y<diml->v[0]; y++) out->send(zxc,duh);
	for (int y=0; y<sy; y++) {
		out->send(diml->v[1]*sc,duh);
		out->send(sxc,data+y*sxc);
		out->send(dimr->v[1]*sc,duh);
	}	
	for (int i=0; i<dimr->v[0]; i++) out->send(zxc,duh);
} GRID_END

GRID_INPUT(GridBorder,1,diml_grid) { diml = diml_grid->to_dim(); } GRID_END
GRID_INPUT(GridBorder,2,dimr_grid) { dimr = dimr_grid->to_dim(); } GRID_END

\def void initialize (Grid *dl, Grid *dr) {
	rb_call_super(argc,argv);
	diml_grid=dl; diml = diml_grid->to_dim();
	dimr_grid=dr; dimr = dimr_grid->to_dim();
}

\classinfo { IEVAL(rself,"install '#border',3,1"); }
\end class GridBorder

static void expect_picture (P<Dim> d) {
	if (d->n!=3) RAISE("(height,width,chans) dimensions please");}
static void expect_rgb_picture (P<Dim> d) {
	expect_picture(d);
	if (d->get(2)!=3) RAISE("(red,green,blue) channels please");}
static void expect_rgba_picture (P<Dim> d) {
	expect_picture(d);
	if (d->get(2)!=4) RAISE("(red,green,blue,alpha) channels please");}

//****************************************************************
//{ Dim[A,B,*Cs]<T>,Dim[D,E]<T> -> Dim[A,B,*Cs]<T> }

static void expect_convolution_matrix (P<Dim> d) {
	if (d->n != 2) RAISE("only exactly two dimensions allowed for now (got %d)",
		d->n);
}

// entry in a compiled convolution kernel
struct PlanEntry { long y,x; bool neutral; };

\class GridConvolve < GridObject
struct GridConvolve : GridObject {
	\attr Numop *op;
	\attr Numop *fold;
	\attr PtrGrid seed;
	\attr PtrGrid b;
	\attr bool wrap;
	\attr bool anti;
	PtrGrid a;
	int plann;
	PlanEntry *plan;
	int margx,margy; // margins
	GridConvolve () : plan(0) { b.constrain(expect_convolution_matrix); plan=0; }
	\decl void initialize (Grid *r=0);
	\grin 0
	\grin 1
	template <class T> void copy_row (T *buf, long sx, long y, long x);
	template <class T> void make_plan (T bogus);
	~GridConvolve () {if (plan) delete[] plan;}
};

template <class T> void GridConvolve::copy_row (T *buf, long sx, long y, long x) {
	long day = a->dim->get(0), dax = a->dim->get(1), dac = a->dim->prod(2);
	y=mod(y,day); x=mod(x,dax);
	T *ap = (T *)*a + y*dax*dac;
	while (sx) {
		long sx1 = min(sx,dax-x);
		COPY(buf,ap+x*dac,sx1*dac);
		x=0;
		buf += sx1*dac;
		sx -= sx1;
	}
}

static Numop *OP(Ruby x) {return FIX2PTR(Numop,rb_hash_aref(op_dict,x));}

template <class T> void GridConvolve::make_plan (T bogus) {
	P<Dim> da = a->dim, db = b->dim;
	long dby = db->get(0);
	long dbx = db->get(1);
	if (plan) delete[] plan;
	plan = new PlanEntry[dbx*dby];
	long i=0;
	for (long y=0; y<dby; y++) {
		for (long x=0; x<dbx; x++) {
			long k = anti ? y*dbx+x : (dby-1-y)*dbx+(dbx-1-x);
			T rh = ((T *)*b)[k];
			bool neutral   = op->on(rh)->is_neutral(  rh,at_right);
			bool absorbent = op->on(rh)->is_absorbent(rh,at_right);
			T foo[1]={0};
			if (absorbent) {
				op->map(1,foo,rh);
				absorbent = fold->on(rh)->is_neutral(foo[0],at_right);
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
	SAME_TYPE(in,b);
	SAME_TYPE(in,seed);
	P<Dim> da=in->dim, db=b->dim;
	if (!db) RAISE("right inlet has no grid");
	if (!seed) RAISE("seed missing");
	if (db->n != 2) RAISE("right grid must have two dimensions");
	if (da->n < 2) RAISE("left grid has less than two dimensions");
	if (seed->dim->n != 0) RAISE("seed must be scalar");
	if (da->get(0) < db->get(0)) RAISE("grid too small (y): %d < %d", da->get(0), db->get(0));
	if (da->get(1) < db->get(1)) RAISE("grid too small (x): %d < %d", da->get(1), db->get(1));
	margy = (db->get(0)-1)/2;
	margx = (db->get(1)-1)/2;
	a=new Grid(da,in->nt);
	int v[da->n]; COPY(v,da->v,da->n);
	if (!wrap) {v[0]-=db->v[0]-1; v[1]-=db->v[1]-1;}
	out=new GridOutlet(this,0,new Dim(da->n,v),in->nt);
} GRID_FLOW {
	COPY((T *)*a+in->dex, data, n);
} GRID_FINISH {
	Numop *op_put = OP(SYM(put));
	make_plan((T)0);
	long dbx = b->dim->get(1);
	long dby = b->dim->get(0);
	long day = out->dim->get(0);
	long n =   out->dim->prod(1);
	long sx =  out->dim->get(1)+dbx-1;
	long sxc = out->dim->prod(2)*sx;
	T buf[n];
	T buf2[sxc];
	T orh=0;
	for (long ay=0; ay<day; ay++) {
		op_put->map(n,buf,*(T *)*seed);
		for (long i=0; i<plann; i++) {
			long by = plan[i].y;
			long bx = plan[i].x;
			long k = anti ? by*dbx+bx : (dby-1-by)*dbx+(dbx-1-bx);
			T rh = ((T *)*b)[k];
			if (i==0 || by!=plan[i-1].y || orh!=rh) {
				if (wrap) copy_row(buf2,sx,ay+by-margy,-margx);
				else      copy_row(buf2,sx,ay+by,0);
				if (!plan[i].neutral) op->map(sxc,buf2,rh);
			}
			fold->zip(n,buf,buf2+bx*out->dim->prod(2));
			orh=rh;
		}
		out->send(n,buf);
	}
	a=0;
} GRID_END

GRID_INPUT(GridConvolve,1,b) {} GRID_END

\def void initialize (Grid *r) {
	rb_call_super(argc,argv);
	this->op = op_mul;
	this->fold = op_add;
	this->seed = new Grid(new Dim(),int32_e,true);
	this->b= r ? r : new Grid(new Dim(1,1),int32_e,true);
	this->wrap = true;
	this->anti = true;
}

\classinfo { IEVAL(rself,"install '#convolve',2,1"); }
\end class GridConvolve

/* ---------------------------------------------------------------- */
/* "#scale_by" does quick scaling of pictures by integer factors */
/*{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }*/
\class GridScaleBy < GridObject
struct GridScaleBy : GridObject {
	\attr PtrGrid scale; // integer scale factor
	int scaley;
	int scalex;
	\decl void initialize (Grid *factor=0);
	\grin 0
	\grin 1
	void prepare_scale_factor () {
		scaley = ((int32 *)*scale)[0];
		scalex = ((int32 *)*scale)[scale->dim->prod()==1 ? 0 : 1];
		if (scaley<1) scaley=2;
		if (scalex<1) scalex=2;
	}
};

GRID_INLET(GridScaleBy,0) {
	P<Dim> a = in->dim;
	expect_picture(a);
	out=new GridOutlet(this,0,new Dim(a->get(0)*scaley,a->get(1)*scalex,a->get(2)),in->nt);
	in->set_factor(a->get(1)*a->get(2));
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	T buf[rowsize*scalex];
	int chans = in->dim->get(2);
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
		for (int j=0; j<scaley; j++) out->send(rowsize*scalex,buf);
	}
	#undef Z
} GRID_END

static void expect_scale_factor (P<Dim> dim) {
	if (dim->prod()!=1 && dim->prod()!=2)
		RAISE("expecting only one or two numbers");
}

GRID_INPUT(GridScaleBy,1,scale) { prepare_scale_factor(); } GRID_END

\def void initialize (Grid *factor) {
	scale.constrain(expect_scale_factor);
	rb_call_super(argc,argv);
	scale=new Grid(INT2NUM(2));
	if (factor) scale=factor;
	prepare_scale_factor();
}

\classinfo { IEVAL(rself,"install '#scale_by',2,1"); }
\end class GridScaleBy

// ----------------------------------------------------------------
//{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }
\class GridDownscaleBy < GridObject
struct GridDownscaleBy : GridObject {
	\attr PtrGrid scale;
	\attr bool smoothly;
	int scaley;
	int scalex;
	PtrGrid temp;
	\decl void initialize (Grid *factor=0, Symbol option=Qnil);
	\grin 0
	\grin 1
	void prepare_scale_factor () {
		scaley = ((int32 *)*scale)[0];
		scalex = ((int32 *)*scale)[scale->dim->prod()==1 ? 0 : 1];
		if (scaley<1) scaley=2;
		if (scalex<1) scalex=2;
	}
};

GRID_INLET(GridDownscaleBy,0) {

	P<Dim> a = in->dim;
	if (a->n!=3) RAISE("(height,width,chans) please");
	out=new GridOutlet(this,0,new Dim(a->get(0)/scaley,a->get(1)/scalex,a->get(2)),in->nt);
	in->set_factor(a->get(1)*a->get(2));
	// i don't remember why two rows instead of just one.
	temp=new Grid(new Dim(2,in->dim->get(1)/scalex,in->dim->get(2)),in->nt);
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	int rowsize2 = temp->dim->prod(1);
	T * buf = (T *)*temp; //!@#$ maybe should be something else than T ?
	int xinc = in->dim->get(2)*scalex;
	int y = in->dex / rowsize;
	int chans=in->dim->get(2);
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
			if (y%scaley==0 && out->dim) {
				op_div->map(rowsize2,buf,(T)(scalex*scaley));
				out->send(rowsize2,buf);
				CLEAR(buf,rowsize2);
			}
			data+=rowsize;
			n-=rowsize;
		}
	#undef Z
	} else {
	#define Z(z) buf[p+z]=data[i+z]
		for (; n>0 && out->dim; data+=rowsize, n-=rowsize,y++) {
			if (y%scaley!=0) continue;
			#define LOOP(z) for (int i=0,p=0; p<rowsize2; i+=xinc, p+=z)
			switch(in->dim->get(2)) {
			case 1: LOOP(1) {Z(0);} break;
			case 2: LOOP(2) {Z(0);Z(1);} break;
			case 3: LOOP(3) {Z(0);Z(1);Z(2);} break;
			case 4: LOOP(4) {Z(0);Z(1);Z(2);Z(3);} break;
			default:LOOP(chans) {for (int k=0; k<chans; k++) Z(k);}break;
			}
			#undef LOOP
			out->send(rowsize2,buf);
		}
	}
	#undef Z
} GRID_END

GRID_INPUT(GridDownscaleBy,1,scale) { prepare_scale_factor(); } GRID_END

\def void initialize (Grid *factor, Symbol option) {
	scale.constrain(expect_scale_factor);
	rb_call_super(argc,argv);
	scale=new Grid(INT2NUM(2));
	if (factor) scale=factor;
	prepare_scale_factor();
	smoothly = option==SYM(smoothly);
}

\classinfo { IEVAL(rself,"install '#downscale_by',2,1"); }
\end class GridDownscaleBy

//****************************************************************
\class GridLayer < GridObject
struct GridLayer : GridObject {
	PtrGrid r;
	GridLayer() { r.constrain(expect_rgb_picture); }
	\grin 0 int
	\grin 1 int
};

GRID_INLET(GridLayer,0) {
	NOTEMPTY(r);
	SAME_TYPE(in,r);
	P<Dim> a = in->dim;
	expect_rgba_picture(a);
	if (a->get(1)!=r->dim->get(1)) RAISE("same width please");
	if (a->get(0)!=r->dim->get(0)) RAISE("same height please");
	in->set_factor(a->prod(2));
	out=new GridOutlet(this,0,r->dim);
} GRID_FLOW {
	T * rr = ((T *)*r) + in->dex*3/4;
	T foo[n*3/4];
#define COMPUTE_ALPHA(c,a) \
	foo[j+c] = (data[i+c]*data[i+a] + rr[j+c]*(256-data[i+a])) >> 8
	for (int i=0,j=0; i<n; i+=4,j+=3) {
		COMPUTE_ALPHA(0,3);
		COMPUTE_ALPHA(1,3);
		COMPUTE_ALPHA(2,3);
	}
#undef COMPUTE_ALPHA
	out->send(n*3/4,foo);
} GRID_END

GRID_INPUT(GridLayer,1,r) {} GRID_END

\classinfo { IEVAL(rself,"install '#layer',2,1"); }
\end class GridLayer

// ****************************************************************
// pad1,pad2 only are there for 32-byte alignment
struct Line { int32 y1,x1,y2,x2,x,m,pad1,pad2; };

static void expect_polygon (P<Dim> d) {
	if (d->n!=2 || d->get(1)!=2) RAISE("expecting Dim[n,2] polygon");
}

\class DrawPolygon < GridObject
struct DrawPolygon : GridObject {
	\attr Numop *op;
	\attr PtrGrid color;
	\attr PtrGrid polygon;
	PtrGrid color2;
	PtrGrid lines;
	int lines_start;
	int lines_stop;
	DrawPolygon() {
		color.constrain(expect_max_one_dim);
		polygon.constrain(expect_polygon);
	}
	\decl void initialize (Numop *op, Grid *color=0, Grid *polygon=0);
	\grin 0
	\grin 1
	\grin 2 int32
	void init_lines();

};

void DrawPolygon::init_lines () {
	int nl = polygon->dim->get(0);
	lines=new Grid(new Dim(nl,8), int32_e);
	Line *ld = (Line *)(int32 *)*lines;
	int32 *pd = *polygon;
	for (int i=0,j=0; i<nl; i++) {
		ld[i].y1 = pd[j+0];
		ld[i].x1 = pd[j+1];
		j=(j+2)%(2*nl);
		ld[i].y2 = pd[j+0];
		ld[i].x2 = pd[j+1];
		if (ld[i].y1>ld[i].y2) memswap((int32 *)(ld+i)+0,(int32 *)(ld+i)+2,2);
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
	SAME_TYPE(in,color);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	if (in->dim->get(2)!=color->dim->get(0))
		RAISE("image does not have same number of channels as stored color");
	out=new GridOutlet(this,0,in->dim,in->nt);
	lines_start = lines_stop = 0;
	in->set_factor(in->dim->get(1)*in->dim->get(2));
	int nl = polygon->dim->get(0);
	qsort((int32 *)*lines,nl,sizeof(Line),order_by_starting_scanline);
	int cn = color->dim->prod();
	color2=new Grid(new Dim(cn*16), color->nt);
	for (int i=0; i<16; i++) COPY((T *)*color2+cn*i,(T *)*color,cn);
} GRID_FLOW {
	int nl = polygon->dim->get(0);
	Line * ld = (Line *)(int32 *)*lines;
	int f = in->factor();
	int y = in->dex/f;
	int cn = color->dim->prod();
	T * cd = (T *)*color2;
	
	while (n) {
		while (lines_stop != nl && ld[lines_stop].y1<=y) lines_stop++;
		for (int i=lines_start; i<lines_stop; i++) {
			if (ld[i].y2<=y) {
				memswap(ld+i,ld+lines_start,1);
				lines_start++;
			}
		}
		if (lines_start == lines_stop) {
			out->send(f,data);
		} else {
			int32 xl = in->dim->get(1);
			T * data2 = new T[f];
			COPY(data2,data,f);
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
			out->give(f,data2);
		}
		n-=f;
		data+=f;
		y++;
	}
} GRID_END


GRID_INPUT(DrawPolygon,1,color) {} GRID_END
GRID_INPUT(DrawPolygon,2,polygon) {init_lines();} GRID_END

\def void initialize (Numop *op, Grid *color, Grid *polygon) {
	rb_call_super(argc,argv);
	this->op = op;
	if (color) this->color=color;
	if (polygon) { this->polygon=polygon; init_lines(); }
}

\classinfo { IEVAL(rself,"install '#draw_polygon',3,1"); }
\end class DrawPolygon

//****************************************************************
static void expect_position(P<Dim> d) {
	if (d->n!=1) RAISE("position should have 1 dimension, not %d", d->n);
	if (d->v[0]!=2) RAISE("position dim 0 should have 2 elements, not %d", d->v[0]);
}

\class DrawImage < GridObject
struct DrawImage : GridObject {
	\attr Numop *op;
	\attr PtrGrid image;
	\attr PtrGrid position;
	\attr bool alpha;
	\attr bool tile;
	
	DrawImage() : alpha(false), tile(false) {
		position.constrain(expect_position);
		image.constrain(expect_picture);
	}

	\decl void initialize (Numop *op, Grid *image=0, Grid *position=0);
	\grin 0
	\grin 1
	\grin 2 int32
	// draw row # ry of right image in row buffer buf, starting at xs
	// overflow on both sides has to be handled automatically by this method
	template <class T> void draw_segment(T * obuf, T * ibuf, int ry, int x0);
};

#define COMPUTE_ALPHA(c,a) obuf[j+(c)] = ibuf[j+(c)] + (rbuf[a])*(obuf[j+(c)]-ibuf[j+(c)])/256;
#define COMPUTE_ALPHA4(b) \
	COMPUTE_ALPHA(b+0,b+3); \
	COMPUTE_ALPHA(b+1,b+3); \
	COMPUTE_ALPHA(b+2,b+3); \
	obuf[b+3] = rbuf[b+3] + (255-rbuf[b+3])*(ibuf[j+b+3])/256;

template <class T> void DrawImage::draw_segment(T * obuf, T * ibuf, int ry, int x0) {
	if (ry<0 || ry>=image->dim->get(0)) return; // outside of image
	int sx = in[0]->dim->get(1), rsx = image->dim->get(1);
	int sc = in[0]->dim->get(2), rsc = image->dim->get(2);
	T * rbuf = (T *)*image + ry*rsx*rsc;
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

GRID_INLET(DrawImage,0) {
	NOTEMPTY(image);
	NOTEMPTY(position);
	SAME_TYPE(in,image);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	if (image->dim->n!=3) RAISE("expecting 3 dimensions in right_hand");
	int lchan = in->dim->get(2);
	int rchan = image->dim->get(2);
	if (alpha && rchan!=4) {
		RAISE("alpha mode works only with 4 channels in right_hand");
	}
	if (lchan != rchan-(alpha?1:0) && lchan != rchan) {
		RAISE("right_hand has %d channels, alpha=%d, left_hand has %d, expecting %d or %d",
			rchan, alpha?1:0, lchan, rchan-(alpha?1:0), rchan);
	}
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_factor(in->dim->get(1)*in->dim->get(2));
} GRID_FLOW {
	int f = in->factor();
	int y = in->dex/f;
	if (position->nt != int32_e) RAISE("position has to be int32");
	int py = ((int32*)*position)[0], rsy = image->dim->v[0];
	int px = ((int32*)*position)[1], rsx = image->dim->v[1], sx=in->dim->get(1);
	for (; n; y++, n-=f, data+=f) {
		int ty = div2(y-py,rsy);
		if (tile || ty==0) {
			T * data2 = new T[f];
			COPY(data2,data,f);
			if (tile) {
				for (int x=px-div2(px+rsx-1,rsx)*rsx; x<sx; x+=rsx) {
					draw_segment(data2,data,mod(y-py,rsy),x);
				}
			} else {
				draw_segment(data2,data,y-py,px);
			}
			out->give(f,data2);
		} else {
			out->send(f,data);
		}
	}
} GRID_END

GRID_INPUT(DrawImage,1,image) {} GRID_END
GRID_INPUT(DrawImage,2,position) {} GRID_END

\def void initialize (Numop *op, Grid *image, Grid *position) {
	rb_call_super(argc,argv);
	this->op = op;
	if (image) this->image=image;
	if (position) this->position=position;
	else this->position=new Grid(new Dim(2),int32_e,true);
}

\classinfo { IEVAL(rself,"install '#draw_image',3,1"); }
\end class DrawImage

//****************************************************************

void startup_flow_objects () {
	op_add = OP(SYM(+));
	op_sub = OP(SYM(-));
	op_mul = OP(SYM(*));
	op_shl = OP(SYM(<<));
	op_mod = OP(SYM(%));
	op_and = OP(SYM(&));
	op_div = OP(SYM(/));
	op_put = OP(SYM(put));
	\startall
}
