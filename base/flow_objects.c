/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003,2004 by Mathieu Bouchard

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

template <int n> class SCOPY {
public: template <class T> static inline void __attribute__((always_inline)) f(Pt<T> a, Pt<T> b) {
		*a=*b; SCOPY<n-1>::f(a+1,b+1);}};
template <> class SCOPY<0> {
public: template <class T> static inline void __attribute__((always_inline)) f(Pt<T> a, Pt<T> b) {}};

/*template <> class SCOPY<4> {
public: template <class T>
	static inline void __attribute__((always_inline)) f(Pt<T> a, Pt<T> b) {
		*a=*b; SCOPY<3>::f(a+1,b+1);}
	// wouldn't gcc 2.95 complain here?
	static inline void __attribute__((always_inline)) f(Pt<uint8> a, Pt<uint8> b)
	{ *(int32 *)a=*(int32 *)b; }
};*/

Numop2 *op2_add, *op2_sub, *op2_mul, *op2_div, *op2_mod, *op2_shl, *op2_and;

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

\classinfo { IEVAL(rself,"install '@cast',1,1"); }
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
	\decl void _0_cast(NumberTypeE cast);
	\decl void _0_reset();
	\decl void _0_symbol(Symbol x);
	\decl void _0_list(...);
	\decl void _1_per_message();
	\grin 0
	\grin 1 int32
	template <class T> void process (int n, Pt<T> data) {
		while (n) {
			if (!out || !out->is_busy())
				out = new GridOutlet(this,0,dim?dim:in[0]->dim,cast);
			int n2 = min((long)n,out->dim->prod()-out->dex);
			out->send(n2,data);
			n-=n2; data+=n2;
		}
	}
};

GRID_INLET(GridImport,0) {} GRID_FLOW { process(n,data); } GRID_END
GRID_INPUT(GridImport,1,dim_grid) { dim = dim_grid->to_dim(); } GRID_END

\def void _0_symbol(Symbol x) {
	const char *name = rb_sym_name(argv[0]);
	int n = strlen(name);
	if (!dim) out=new GridOutlet(this,0,new Dim(n));
	process(n,Pt<uint8>((uint8 *)name,n));
}

\def void _0_cast(NumberTypeE cast) { this->cast = cast; }

\def void _0_list(...) {
	if (!in[0]) _0_grid(0,0); //HACK: enable grid inlet...
	in[0]->from_ruby_list(argc,argv,cast);
}

\def void _1_per_message() { dim=0; dim_grid=0; }

\def void initialize(Ruby x, NumberTypeE cast) {
	rb_call_super(argc,argv);
	this->cast = cast;
	if (argv[0]!=SYM(per_message)) {
		dim_grid=new Grid(argv[0]);
		dim = dim_grid->to_dim();
	} else {
		dim = 0;
	}
}

\def void _0_reset() {
	STACK_ARRAY(int32,foo,1); *foo=0;
	while (out->is_busy()) out->send(1,foo);
}

\classinfo { IEVAL(rself,"install '@import',2,1"); }
\end class GridImport

//****************************************************************
/*{ Dim[*As] -> ? }*/
/* in0: integer nt */
\class GridExport < GridObject
struct GridExport : GridObject {
	\grin 0
};

template <class T>
static Ruby INTORFLOAT2NUM(T       value) {return      INT2NUM(value);}
static Ruby INTORFLOAT2NUM(int64   value) {return    gf_ll2num(value);}
static Ruby INTORFLOAT2NUM(float32 value) {return rb_float_new(value);}
static Ruby INTORFLOAT2NUM(float64 value) {return rb_float_new(value);}

GRID_INLET(GridExport,0) {
} GRID_FLOW {
	for (int i=0; i<n; i++) {
		Ruby a[] = { INT2NUM(0), INTORFLOAT2NUM(data[i]) };
		send_out(COUNT(a),a);
	}
} GRID_FINISH {
} GRID_END
\classinfo { IEVAL(rself,"install '@export',1,1"); }
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
	int n = in->dim->prod();
	if (n>250000) RAISE("list too big (%d elements)", n);
	list = rb_ary_new2(n+2);
	this->n = n;
	rb_ivar_set(rself,SI(@list),list); // keep
	rb_ary_store(list,0,INT2NUM(0));
	rb_ary_store(list,1,bsym._list);
} GRID_FLOW {
	for (int i=0; i<n; i++, data++)
		rb_ary_store(list,in->dex+i+2,INTORFLOAT2NUM(*data));
} GRID_FINISH {
	send_out(rb_ary_len(list),rb_ary_ptr(list));
	list = 0;
	rb_ivar_set(rself,SI(@list),Qnil); // unkeep
} GRID_END

\classinfo { IEVAL(rself,"install '@export_list',1,1"); }
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
	PtrGrid r; //\attr
	PtrGrid put_at; //\attr
	int32 wdex [MAX_DIMENSIONS]; // temporary buffer, copy of put_at
	int32 fromb[MAX_DIMENSIONS];
	int32 to2  [MAX_DIMENSIONS];
	int lsd; // lsd = Last Same Dimension (for put_at)
	int d; // goes with wdex
	\decl void initialize (Grid *r=0);
	\decl void _0_bang ();
	\decl void _1_reassign ();
	\decl void _1_put_at (Grid *index);
	\grin 0 int
	\grin 1
	GridStore() { put_at.constrain(expect_max_one_dim); }
	template <class T> void compute_indices(Pt<T> v, int nc, int nd);
};

// takes the backstore of a grid and puts it back into place. a backstore
// is a grid that is filled while the grid it would replace has not
// finished being used.
static void snap_backstore (PtrGrid &r) {
	if (r.next) {r=r.next.p; r.next=0;}
}

template <class T> void GridStore::compute_indices(Pt<T> v, int nc, int nd) {
	for (int i=0; i<nc; i++) {
		int32 wrap = r->dim->v[i];
		bool fast = lowest_bit(wrap)==highest_bit(wrap); // is power of two?
		if (i) {
			if (fast) op2_shl->map(nd,v,(T)highest_bit(wrap));
			else      op2_mul->map(nd,v,(T)wrap);
		}
		if (fast) op2_and->map(nd,v+nd*i,(T)(wrap-1));
		else      op2_mod->map(nd,v+nd*i,(T)(wrap));
		if (i) op2_add->zip(nd,v,v+nd*i);
	}
}

// !@#$ i should ensure that n is not exceedingly large
// !@#$ worse: the size of the foo buffer may still be too large
GRID_INLET(GridStore,0) {
	// snap_backstore must be done before *anything* else
	snap_backstore(r);
	NOTEMPTY(r);
	int na = in->dim->n;
	int nb = r->dim->n;
	int nc = in->dim->get(na-1);
	STACK_ARRAY(int32,v,MAX_DIMENSIONS);
	if (na<1) RAISE("must have at least 1 dimension.",na,1,1+nb);
	int lastindexable = r->dim->prod()/r->dim->prod(nc) - 1;
	int ngreatest = nt_greatest((T *)0);
	if (lastindexable > ngreatest) {
		RAISE("lastindexable=%d > ngreatest=%d (ask matju)",lastindexable,ngreatest);
	}
	if (nc > nb)
		RAISE("wrong number of elements in last dimension: "
			"got %d, expecting <= %d", nc, nb);
	int nd = nb - nc + na - 1;
	COPY(v,in->dim->v,na-1);
	COPY(v+na-1,r->dim->v+nc,nb-nc);
	out=new GridOutlet(this,0,new Dim(nd,v),r->nt);
	if (nc>0) in->set_factor(nc);
} GRID_FLOW {
	int na = in->dim->n;
	int nc = in->dim->get(na-1);
	int size = r->dim->prod(nc);
	assert((n % nc) == 0);
	int nd = n/nc;
	STACK_ARRAY(T,w,n);
	Pt<T> v=w;
	if (sizeof(T)==1 && nc==1 && r->dim->v[0]<=256) {
		// bug? shouldn't modulo be done here?
		v=data;
	} else {
		COPY(v,data,n);
		for (int k=0,i=0; i<nc; i++) for (int j=0; j<n; j+=nc) v[k++] = data[i+j];
		compute_indices(v,nc,nd);
	}
#define FOO(type) { \
	Pt<type> p = (Pt<type>)*r; \
	if (size<=16) { \
		Pt<type> foo = ARRAY_NEW(type,nd*size); \
		int i=0; \
		switch (size) { \
		case 1: for (; i<nd&-4; i+=4, foo+=4) { \
			foo[0] = p[v[i+0]]; \
			foo[1] = p[v[i+1]]; \
			foo[2] = p[v[i+2]]; \
			foo[3] = p[v[i+3]]; \
		} break; \
		case 2: for (; i<nd; i++, foo+=2) SCOPY<2>::f(foo,p+2*v[i]); break; \
		case 3: for (; i<nd; i++, foo+=3) SCOPY<3>::f(foo,p+3*v[i]); break; \
		case 4: for (; i<nd; i++, foo+=4) SCOPY<4>::f(foo,p+4*v[i]); break; \
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
		int n = in->dim->prod(0,-2);
		int size = r->dim->prod();
#define FOO(T) while (n--) out->send(size,(Pt<T>)*r);
		TYPESWITCH(r->nt,FOO,)
#undef FOO
	}
} GRID_END

GRID_INLET(GridStore,1) {
	NumberTypeE nt = NumberTypeE_type_of(*data);
	if (!put_at) { // reassign
		if (in[0].is_busy())
			r.next = new Grid(in->dim,nt);
		else
			r = new Grid(in->dim,nt);
		return;
	}
	// put_at ( ... )
	//!@#$ should check types. if (r->nt!=in->nt) RAISE("shoo");
	int nn=r->dim->n, na=put_at->dim->v[0], nb=in->dim->n;
	STACK_ARRAY(int32,sizeb,nn);
	for (int i=0; i<nn; i++) { fromb[i]=0; sizeb[i]=1; }
	COPY(Pt<int32>(wdex,nn)       ,(Pt<int32>)*put_at   ,put_at->dim->prod());
	COPY(Pt<int32>(fromb,nn)+nn-na,(Pt<int32>)*put_at   ,na);
	COPY(Pt<int32>(sizeb,nn)+nn-nb,(Pt<int32>)in->dim->v,nb);
	for (int i=0; i<nn; i++) to2[i] = fromb[i]+sizeb[i];
	d=0;
	// find out when we can skip computing indices
	//!@#$ should actually also stop before blowing up packet size
#define MAX_PACKET_SIZE (1<<11) /* sorry! */
	lsd=nn;
	while (lsd>=nn-in->dim->n) {
		lsd--;
		int cs = in->dim->prod(lsd-nn+in->dim->n);
		if (cs>MAX_PACKET_SIZE || fromb[lsd]!=0 || sizeb[lsd]!=r->dim->v[lsd]) break;
	}
	lsd++;
	int cs = in->dim->prod(lsd-nn+in->dim->n);
	in->set_factor(cs);
} GRID_FLOW {
	if (!put_at) { // reassign
		COPY(((Pt<T>)*(r.next ? r.next : &*r))+in->dex, data, n);
		return;
	}
	// put_at ( ... )
	int nn=r->dim->n;
	int cs = in->factor(); // chunksize
	STACK_ARRAY(int32,v,lsd);
	Pt<int32> x = Pt<int32>(wdex,nn);
	while (n) {
		// here d is the dim# to reset; d=n for none
		for(;d<lsd;d++) x[d]=fromb[d];
		// do stuff here
		COPY(v,x,lsd);
		compute_indices(v,lsd,1);
		COPY((Pt<T>)*r+v[0]*cs,data,cs);
		data+=cs;
		n-=cs;
		// find next set of indices; here d is the dim# to increment
		for(;;) {
			d--;
			if (d<0) goto end;
			x[d]++;
			if (x[d]<to2[d]) break;
		}
		end:; // why here ??? or why at all?
		d++;
	}
	//end:; // why not here ???
} GRID_END

\def void _0_bang () {
	rb_funcall(rself,SI(_0_list),3,INT2NUM(0),SYM(#),INT2NUM(0));
}
\def void _1_reassign () { put_at=0; }
\def void _1_put_at (Grid *index) { put_at=index; }
\def void initialize (Grid *r) {rb_call_super(argc,argv); if (r) this->r=r;}
\classinfo { IEVAL(rself,"install '@store',2,1"); }
\end class GridStore

//****************************************************************
//{ Dim[*As]<T> -> Dim[*As]<T> }
\class GridOp1 < GridObject
struct GridOp1 : GridObject {
	\attr Numop1 *op;
	\decl void initialize (Numop1 *op);
	\grin 0
};

GRID_INLET(GridOp1,0) {
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_mode(6);
} GRID_FLOW {
	op->map(n,data);
	out->give(n,data);
} GRID_END

\def void initialize (Numop1 *op) { rb_call_super(argc,argv); this->op=op;}
\classinfo { IEVAL(rself,"install '@!',1,1"); }
\end class GridOp1

//****************************************************************
//{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As]<T> }
\class GridOp2 < GridObject
struct GridOp2 : GridObject {
	\attr Numop2 *op;
	PtrGrid r;
	\decl void initialize(Numop2 *op, Grid *r=0);
	\grin 0
	\grin 1
};

GRID_INLET(GridOp2,0) {
	snap_backstore(r);
	SAME_TYPE(in,r);
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_mode(6);
} GRID_FLOW {
	NOTEMPTY(r);
	Pt<T> rdata = (Pt<T>)*r;
	int loop = r->dim->prod();
	if (sizeof(T)==8) {
		fprintf(stderr,"1: data=%p rdata=%p\n",data.p,rdata.p);
		WATCH(n,data);
	}
	if (loop>1) {
		if (in->dex+n <= loop) {
			op->zip(n,data,rdata+in->dex);
		} else {
			// !@#$ should prebuild and reuse this array when "loop" is small
			STACK_ARRAY(T,data2,n);
			int ii = mod(in->dex,loop);
			int m = min(loop-ii,n);
			COPY(data2,rdata+ii,m);
			int nn = m+((n-m)/loop)*loop;
			for (int i=m; i<nn; i+=loop) COPY(data2+i,rdata,loop);
			if (n>nn) COPY(data2+nn,rdata,n-nn);
			if (sizeof(T)==8) {
				fprintf(stderr,"2: data=%p data2=%p\n",data.p,data2.p);
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

GRID_INPUT2(GridOp2,1,r) {} GRID_END

\def void initialize(Numop2 *op, Grid *r=0) {
	rb_call_super(argc,argv);
	this->op = op;
	this->r = r ? r : new Grid(new Dim(),int32_e,true);
}

\classinfo { IEVAL(rself,"install '@',2,1"); }
\end class GridOp2

//****************************************************************
//{ Dim[*As,B,*Cs]<T>,Dim[*Cs]<T> -> Dim[*As,*Cs]<T> }
\class GridFold < GridObject
struct GridFold : GridObject {
	\attr Numop2 *op;
	\attr PtrGrid seed;
	\decl void initialize (Numop2 *op, Grid *seed=0);
	\grin 0
	\grin 1
};

GRID_INLET(GridFold,0) {
	NOTEMPTY(seed);
	SAME_TYPE(in,seed);
	int an = in->dim->n;
	int bn = seed->dim->n;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand "
		"(%d vs %d)",an,bn);
	STACK_ARRAY(int32,v,an-1);
	int yi = an-bn-1;
	COPY(v,in->dim->v,yi);
	COPY(v+yi,in->dim->v+an-bn,bn);
	SAME_DIM(an-(yi+1),in->dim,(yi+1),seed->dim,0);
	out=new GridOutlet(this,0,new Dim(an-1,v),in->nt);
	in->set_factor(in->dim->get(yi)*seed->dim->prod());
} GRID_FLOW {
	int an = in->dim->n;
	int bn = seed->dim->n;
	int yn = in->dim->v[an-bn-1];
	int zn = in->dim->prod(an-bn);
	STACK_ARRAY(T,buf,n/yn);
	int nn=n;
	int yzn=yn*zn;
	for (int i=0; n; i+=zn, data+=yzn, n-=yzn) {
		COPY(buf+i,((Pt<T>)*seed),zn);
		op->fold(zn,yn,buf+i,data);
	}
	out->send(nn/yn,buf);
} GRID_END

GRID_INPUT(GridFold,1,seed) {} GRID_END

\def void initialize (Numop2 *op, Grid *seed=0) {
	rb_call_super(argc,argv);
	this->op = op;
	this->seed = seed ? seed : new Grid(new Dim(),int32_e,true);
}

\classinfo { IEVAL(rself,"install '@fold',2,1"); }
\end class GridFold

//****************************************************************
//{ Dim[*As,B,*Cs]<T>,Dim[*Cs]<T> -> Dim[*As,B,*Cs]<T> }
\class GridScan < GridObject
struct GridScan : GridObject {
	\attr Numop2 *op;
	\attr PtrGrid seed;
	\decl void initialize (Numop2 *op, Grid *seed=0);
	\grin 0
	\grin 1
};

GRID_INLET(GridScan,0) {
	NOTEMPTY(seed);
	SAME_TYPE(in,seed);
	int an = in->dim->n;
	int bn = seed->dim->n;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand");
	SAME_DIM(bn,in->dim,an-bn,seed->dim,0);
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_factor(in->dim->prod(an-bn-1));
} GRID_FLOW {
	int an = in->dim->n;
	int bn = seed->dim->n;
	int yn = in->dim->v[an-bn-1];
	int zn = in->dim->prod(an-bn);
	int factor = in->factor();
	STACK_ARRAY(T,buf,n);
	COPY(buf,data,n);
	for (int i=0; i<n; i+=factor) op->scan(zn,yn,(Pt<T>)*seed,buf+i);
	out->send(n,buf);
} GRID_END

GRID_INPUT(GridScan,1,seed) {} GRID_END

\def void initialize (Numop2 *op, Grid *seed=0) {
	rb_call_super(argc,argv);
	this->op = op;
	this->seed = seed ? seed : new Grid(new Dim(),int32_e,true);
}

\classinfo { IEVAL(rself,"install '@scan',2,1"); }
\end class GridScan

//****************************************************************
//{ Dim[*As,C]<T>,Dim[C,*Bs]<T> -> Dim[*As,*Bs]<T> }
\class GridInner < GridObject
struct GridInner : GridObject {
	\attr Numop2 *op_para;
	\attr Numop2 *op_fold;
	\attr PtrGrid seed;
	PtrGrid r;
	PtrGrid r2;
	GridInner() {}
	\decl void initialize (Numop2 *op_para=op2_mul, Numop2 *op_fold=op2_add, Grid *seed=0, Grid *r=0);
	\grin 0
	\grin 2
};

#define MAX_PACKET_SIZE (1<<11)

GRID_INLET(GridInner,0) {
	NOTEMPTY(r);    SAME_TYPE(in,r);
	NOTEMPTY(seed); SAME_TYPE(in,seed);
	P<Dim> a = in->dim;
	P<Dim> b = r->dim;
	if (a->n<1) RAISE("a: minimum 1 dimension");
	if (b->n<1) RAISE("b: minimum 1 dimension");
	if (seed->dim->n != 0) RAISE("seed must be a scalar");
	int a_last = a->get(a->n-1);
	int n = a->n+b->n-2;
	SAME_DIM(1,a,a->n-1,b,0);
	STACK_ARRAY(int32,v,n);
	COPY(v,a->v,a->n-1);
	COPY(v+a->n-1,b->v+1,b->n-1);
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
	in->set_factor(a_last);

	int rrows = in->factor();
	int rsize = r->dim->prod();
	int rcols = rsize/rrows;
	Pt<T> rdata = (Pt<T>)*r;
	int chunk = MAX_PACKET_SIZE/rsize;
	r2=new Grid(new Dim(chunk*rsize),r->nt);
	Pt<T> buf3 = (Pt<T>)*r2;
	for (int i=0; i<rrows; i++)
		for (int j=0; j<chunk; j++)
			COPY(buf3+(j+i*chunk)*rcols,rdata+i*rcols,rcols);
} GRID_FLOW {
	Numop2 *op2_put = FIX2PTR(Numop2,rb_hash_aref(op2_dict,SYM(put)));
	int rrows = in->factor();
	int rsize = r->dim->prod();
	int rcols = rsize/rrows;
	int chunk = MAX_PACKET_SIZE/rsize;
	STACK_ARRAY(T,buf ,chunk*rcols);
	STACK_ARRAY(T,buf2,chunk*rcols);
	int off = chunk;
	while (n) {
		if (chunk*rrows>n) chunk=n/rrows;
		op2_put->map(chunk*rcols,buf2,*(T *)*seed);
		for (int i=0; i<rrows; i++) {
			for (int j=0; j<chunk; j++)
				op2_put->map(rcols,buf+j*rcols,data[i+j*rrows]);
			op_para->zip(chunk*rcols,buf,(Pt<T>)*r2+i*off*rcols);
			op_fold->zip(chunk*rcols,buf2,buf);
		}
		out->send(chunk*rcols,buf2);
		n-=chunk*rrows;
		data+=chunk*rrows;
	}
} GRID_FINISH {
	r2=0;
} GRID_END

GRID_INPUT(GridInner,2,r) {} GRID_END

\def void initialize (Numop2 *op_para, Numop2 *op_fold, Grid *seed, Grid *r) {
	rb_call_super(argc,argv);
	this->op_para = op_para;
	this->op_fold = op_fold;
	this->seed = seed ? seed : new Grid(new Dim(),int32_e,true);
	this->r    = r    ? r    : new Grid(new Dim(),int32_e,true);
}

\classinfo { IEVAL(rself,"install '@inner',3,1"); }
\end class GridInner

/* **************************************************************** */
/*{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As,*Bs]<T> }*/
\class GridOuter < GridObject
struct GridOuter : GridObject {
	\attr Numop2 *op;
	PtrGrid r;
	\decl void initialize (Numop2 *op, Grid *r=0);
	\grin 0
	\grin 1
};

GRID_INLET(GridOuter,0) {
	NOTEMPTY(r);
	SAME_TYPE(in,r);
	P<Dim> a = in->dim;
	P<Dim> b = r->dim;
	int n = a->n+b->n;
	STACK_ARRAY(int32,v,n);
	COPY(v,a->v,a->n);
	COPY(v+a->n,b->v,b->n);
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
} GRID_FLOW {
	int b_prod = r->dim->prod();
	if (b_prod > 4) {
		STACK_ARRAY(T,buf,b_prod);
		while (n) {
			for (int j=0; j<b_prod; j++) buf[j] = *data;
			op->zip(b_prod,buf,(Pt<T>)*r);
			out->send(b_prod,buf);
			data++; n--;
		}
		return;
	}
	n*=b_prod;
	Pt<T> buf = ARRAY_NEW(T,n);
	STACK_ARRAY(T,buf2,b_prod*64);
	for (int i=0; i<64; i++) COPY(buf2+i*b_prod,(Pt<T>)*r,b_prod);
	switch (b_prod) {
	#define Z buf[k++]=data[i]
	case 1:	for (int i=0,k=0; k<n; i++) {Z;} break;
	case 2: for (int i=0,k=0; k<n; i++) {Z;Z;} break;
	case 3:	for (int i=0,k=0; k<n; i++) {Z;Z;Z;} break;
	case 4:	for (int i=0,k=0; k<n; i++) {Z;Z;Z;Z;} break;
	default:for (int i=0,k=0; k<n; i++) for (int j=0; j<b_prod; j++, k++) Z;
	}
	#undef Z
	int ch=64*b_prod;
	int nn=(n/ch)*ch;
	for (int j=0; j<nn; j+=ch) op->zip(ch,buf+j,buf2);
	op->zip(n-nn,buf+nn,buf2);
	out->give(n,buf);
} GRID_END

GRID_INPUT(GridOuter,1,r) {} GRID_END

\def void initialize (Numop2 *op, Grid *r) {
	rb_call_super(argc,argv);
	this->op = op;
	this->r = r ? r : new Grid(new Dim(),int32_e,true);
}

\classinfo { IEVAL(rself,"install '@outer',2,1"); }
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
	STACK_ARRAY(T,x,64*n);
	Pt<T> fromb = (Pt<T>)*from;
	Pt<T>   tob = (Pt<T>)*to  ;
	Pt<T> stepb = (Pt<T>)*step;
	STACK_ARRAY(T,to2,n);
	
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
	TYPESWITCH_NOFLOAT(from->nt,FOO,);
#undef FOO
}

\def void _0_set (Grid *r) { from=new Grid(argv[0]); }
GRID_INPUT(GridFor,2,step) {} GRID_END
GRID_INPUT(GridFor,1,to) {} GRID_END
GRID_INPUT(GridFor,0,from) {_0_bang(0,0);} GRID_END
\classinfo { IEVAL(rself,"install '@for',3,1"); }
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

\classinfo { IEVAL(rself,"install '@finished',1,1"); }
\end class GridFinished

\class GridDim < GridObject
struct GridDim : GridObject {
	\grin 0
};
GRID_INLET(GridDim,0) {
	GridOutlet out(this,0,new Dim(in->dim->n));
	out.send(in->dim->n,Pt<int32>(in->dim->v,in->dim->n));
	in->set_mode(0);
} GRID_END
\classinfo { IEVAL(rself,"install '@dim',1,1"); }
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
\classinfo { IEVAL(rself,"install '@type',1,1"); }
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
	int a = in->dim->prod(), b = dim->prod();
	if (a<b) temp=new Grid(new Dim(a),in->nt);
	out=new GridOutlet(this,0,dim,in->nt);
} GRID_FLOW {
	int i = in->dex;
	if (!temp) {
		int b = dim->prod();
		int n2 = min(n,b-i);
		if (n2>0) out->send(n2,data);
		// discard other values if any
	} else {
		int a = in->dim->prod();
		int n2 = min(n,a-i);
		COPY((Pt<T>)*temp+i,data,n2);
		if (n2>0) out->send(n2,data);
	}
} GRID_FINISH {
	if (!!temp) {
		int a = in->dim->prod(), b = dim->prod();
		if (a) {
			for (int i=a; i<b; i+=a) out->send(min(a,b-i),(Pt<T>)*temp);
		} else {
			STACK_ARRAY(T,foo,1);
			foo[0]=0;
			for (int i=0; i<b; i++) out->send(1,foo);
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

\classinfo { IEVAL(rself,"install '@redim',2,1"); }
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
	L
	NOTEMPTY(r);
	SAME_TYPE(in,r);
	P<Dim> d = in->dim;
	if (d->n != r->dim->n) RAISE("wrong number of dimensions");
	int w = which_dim;
	if (w<0) w+=d->n;
	if (w<0 || w>=d->n)
		RAISE("can't join on dim number %d on %d-dimensional grids",
			which_dim,d->n);
	STACK_ARRAY(int32,v,d->n);
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
	L
	int w = which_dim;
	if (w<0) w+=in->dim->n;
	int a = in->factor();
	int b = r->dim->prod(w);
	Pt<T> data2 = (Pt<T>)*r + in->dex*b/a;
	if (a==3 && b==1) {
		int m = n+n*b/a;
		STACK_ARRAY(T,data3,m);
		Pt<T> data4 = data3;
		while (n) {
			SCOPY<3>::f(data4,data); SCOPY<1>::f(data4+3,data2);
			n-=3; data+=3; data2+=1; data4+=4;
		}
		out->send(m,data3);
	} else if (a+b<=16) {
		int m = n+n*b/a;
		STACK_ARRAY(T,data3,m);
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
	L
	if (in->dim->prod()==0) out->send(r->dim->prod(),(Pt<T>)*r);
} GRID_END

GRID_INPUT(GridJoin,1,r) {L} GRID_END

\def void initialize (int which_dim, Grid *r) {
	rb_call_super(argc,argv);
	this->which_dim = which_dim;
	if (r) this->r=r;
}

\classinfo { IEVAL(rself,"install '@join',2,1"); }
\end class GridJoin

//****************************************************************
\class GridGrade < GridObject
struct GridGrade : GridObject {
	\grin 0
};

template <class T> struct GradeFunction {
	static int comparator (const void *a, const void *b) {
		return **(T**)a - **(T**)b;}};
#define FOO(S) \
struct GradeFunction<S> { \
	static int comparator (const void *a, const void *b) { \
		S x = **(S**)a - **(S**)b; \
		return x<0 ? -1 : x>0;}};
FOO(int64)
FOO(float32)
FOO(float64)
#undef FOO

GRID_INLET(GridGrade,0) {
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_factor(in->dim->get(in->dim->n-1));
} GRID_FLOW {
	int m = in->factor();
	STACK_ARRAY(T*,foo,m);
	STACK_ARRAY(T,bar,m);
	for (; n; n-=m,data+=m) {
		for (int i=0; i<m; i++) foo[i] = &data[i];
		qsort(foo,m,sizeof(T),GradeFunction<T>::comparator);
		for (int i=0; i<m; i++) bar[i] = foo[i]-(T *)data;
		out->send(m,bar);
	}
} GRID_END

\classinfo { IEVAL(rself,"install '@grade',1,1"); }
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
	\grin 0
};

GRID_INLET(GridTranspose,0) {
	STACK_ARRAY(int32,v,in->dim->n);
	COPY(v,in->dim->v,in->dim->n);
	d1=dim1; d2=dim2;
	if (d1>=in->dim->n || d2>=in->dim->n)
		RAISE("not enough dimensions: dim1=%d dim2=%d",dim1,dim2);
	if (d1<0) d1+=in->dim->n;
	if (d2<0) d2+=in->dim->n;
	if (d1<0 || d2<0)
		RAISE("not enough dimensions: dim1=%d dim2=%d",dim1,dim2);
	memswap(v+d1,v+d2,1);
	if (d1==d2) {
		out=new GridOutlet(this,0,new Dim(in->dim->n,v), in->nt);
	} else {
		nd = in->dim->prod(1+max(d1,d2));
		nc = in->dim->v[max(d1,d2)];
		nb = in->dim->prod(1+min(d1,d2))/nc/nd;
		na = in->dim->v[min(d1,d2)];
		out->begin(new Dim(in->dim->n,v), in->nt);
		in->set_factor(na*nb*nc*nd);
	}
	// Turns a Grid[*,na,*nb,nc,*nd] into a Grid[*,nc,*nb,na,*nd].
} GRID_FLOW {
	STACK_ARRAY(T,res,na*nb*nc*nd);
	if (dim1==dim2) { out->send(n,data); return; }
	for (; n; n-=na*nb*nc*nd, data+=na*nb*nc*nd) {
		for (int a=0; a<na; a++)
			for (int b=0; b<nb; b++)
				for (int c=0; c<nc; c++)
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

\classinfo { IEVAL(rself,"install '@transpose',3,1"); }
\end class GridTranspose

//****************************************************************
\class GridPerspective < GridObject
struct GridPerspective : GridObject {
	\attr int32 z;
	\grin 0
	\decl void initialize (int32 z=256);
};

GRID_INLET(GridPerspective,0) {
	int n = in->dim->n;
	STACK_ARRAY(int32,v,n);
	COPY(v,in->dim->v,n);
	v[n-1]--;
	in->set_factor(in->dim->get(in->dim->n-1));
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
} GRID_FLOW {
	int m = in->factor();
	for (; n; n-=m,data+=m) {
		op2_mul->map(m-1,data,(T)z);
		op2_div->map(m-1,data,data[m-1]);
		out->send(m-1,data);
	}	
} GRID_END

\def void initialize (int32 z) {rb_call_super(argc,argv); this->z=z; }
\classinfo { IEVAL(rself,"install '@perspective',1,1"); }
\end class GridPerspective

static Numop2 *OP2(Ruby x) {
	return FIX2PTR(Numop2,rb_hash_aref(op2_dict,x));
}

void startup_flow_objects () {
	op2_add = OP2(SYM(+));
	op2_sub = OP2(SYM(-));
	op2_mul = OP2(SYM(*));
	op2_shl = OP2(SYM(<<));
	op2_mod = OP2(SYM(%));
	op2_and = OP2(SYM(&));
	op2_div = OP2(SYM(/));
	\startall
}
