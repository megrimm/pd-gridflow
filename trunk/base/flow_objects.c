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

#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include "grid.h.fcs"

template <int n>
class SCOPY {
public:
	template <class T> static inline void __attribute__((always_inline)) f(Pt<T> a, Pt<T> b) {
		*a=*b;
		SCOPY<n-1>::f(a+1,b+1);
	}
};

template <>
class SCOPY<0> {
public:
	template <class T> static inline void __attribute__((always_inline)) f(Pt<T> a, Pt<T> b) {}
};

/*
template <>
class SCOPY<4> {
public:
	template <class T>
	static inline void __attribute__((always_inline)) f(Pt<T> a, Pt<T> b) {
		*a=*b;
		SCOPY<3>::f(a+1,b+1);
	}
	// wouldn't gcc 2.95 complain here?
	static inline void __attribute__((always_inline)) f(Pt<uint8> a, Pt<uint8> b)
	{ *(int32 *)a=*(int32 *)b; }
};
*/

Numop2 *op2_add, *op2_sub, *op2_mul, *op2_div, *op2_mod;
Numop2 *op2_shl, *op2_and;

/* **************************************************************** */

static void expect_dim_dim_list (Dim *d) {
	if (d->n!=1) RAISE("dimension list should be Dim[n], not %s",d->to_s());
	int n = d->get(0);
	if (n>MAX_DIMENSIONS) RAISE("too many dimensions");
}

static void expect_min_one_dim (Dim *d) {
	if (d->n<1) RAISE("minimum 1 dimension");
}

static void expect_max_one_dim (Dim *d) {
	if (d->n>1) { RAISE("expecting Dim[] or Dim[n], got %s",d->to_s()); }
}

static void expect_exactly_one_dim (Dim *d) {
	if (d->n!=1) { RAISE("expecting Dim[n], got %s",d->to_s()); }
}

/* **************************************************************** */

\class GridCast < GridObject
struct GridCast : GridObject {
	\attr NumberTypeE nt;
	\decl void initialize (NumberTypeE nt);
	GRINLET3(0);
};

GRID_INLET(GridCast,0) {
	out[0]->begin(in->dim->dup(),nt);
} GRID_FLOW {
	out[0]->send(n,data);
} GRID_FINISH {
} GRID_END

\def void initialize (NumberTypeE nt) {
	rb_call_super(argc,argv);
	this->nt = nt;
}

GRCLASS(GridCast,LIST(GRINLET4(GridCast,0,4)),
	\grdecl
) { IEVAL(rself,"install '@cast',1,1"); }

\end class GridCast

/* **************************************************************** */
/*
  GridImport ("@import") is the class for converting a old-style stream
  of integers to a streamed grid as used now.
*/

/*{ ?,Dim[B] -> Dim[*Cs] }*/
/* out0 nt to be specified explicitly */
\class GridImport < GridObject
struct GridImport : GridObject {
	\attr NumberTypeE nt;
	\attr Dim *dim; /* size of grids to send */
	Grid dim_grid;
	GridImport() { dim_grid.constrain(expect_dim_dim_list); }
	~GridImport() { if (dim) delete dim; }

	\decl void initialize(Ruby x, NumberTypeE nt=int32_type_i);
	\decl void _0_reset();
	\decl void _0_symbol(Symbol x);
	\decl void _1_per_message();
	GRINLET3(0);
	GRINLET3(1);

	template <class T> void process (int n, Pt<T> data) {
		GridOutlet *o = out[0];
		while (n) {
			if (dim && !o->is_busy()) o->begin(dim->dup(),nt);
			int n2 = min((long)n,o->dim->prod()-o->dex);
			o->send(n2,data);
			n -= n2;
			data += n2;
		}
	}
};

GRID_INLET(GridImport,0) {
	if (!dim) out[0]->begin(in->dim->dup(),nt);
} GRID_FLOW {
	process(n,data);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridImport,1,dim_grid) {
	if (dim) delete dim;
	dim = dim_grid.to_dim();
} GRID_END

\def void _0_symbol(Symbol x) {
	const char *name = rb_sym_name(argv[0]);
	int32 v[] = { strlen(name) };
	if (!dim) out[0]->begin(new Dim(1,v));
	process(v[0],Pt<uint8>((uint8 *)name,v[0]));
}

\def void _1_per_message() {
	dim_grid.del();
	if (dim) delete dim;
	dim = 0;
}

\def void initialize(Ruby x, NumberTypeE nt) {
	rb_call_super(argc,argv);
	this->nt = nt;
	if (argv[0]!=SYM(per_message)) {
		dim_grid.init_from_ruby(argv[0]);
		dim = dim_grid.to_dim();
	} else {
		dim = 0;
	}
}

\def void _0_reset() {
	if (out[0]->is_busy()) out[0]->abort();
}

GRCLASS(GridImport,LIST(GRINLET4(GridImport,0,4),GRINLET(GridImport,1,4)),
	\grdecl
){ IEVAL(rself,"install '@import',2,1"); }

\end class GridImport

/* **************************************************************** */
/*
  GridExport ("@export") is the class for converting from streamed grids
  to old-style integer stream.
*/

/*{ Dim[*As] -> ? }*/
/* in0: integer nt */
\class GridExport < GridObject
struct GridExport : GridObject {
	GRINLET3(0);
};

template <class T>
static Ruby INTORFLOAT2NUM(T value) {return INT2NUM(value);}

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

/* outlet 0 not used for grids */
GRCLASS(GridExport,LIST(GRINLET4(GridExport,0,4)))
{ IEVAL(rself,"install '@export',1,1"); }
\end class GridExport

/* **************************************************************** */

/*{ Dim[*As] -> ? }*/
/* in0: integer nt */
\class GridExportList < GridObject
struct GridExportList : GridObject {
	Ruby /*Array*/ list;
	int n;

	GRINLET3(0);
};

GRID_INLET(GridExportList,0) {
	int n = in->dim->prod();
	if (n>250000) RAISE("list too big (%d elements)", n);
	list = rb_ary_new2(n+2);
	this->n = n;
	rb_ivar_set(rself,SI(@list),list); /* keep */
	rb_ary_store(list,0,INT2NUM(0));
	rb_ary_store(list,1,bsym._list);
} GRID_FLOW {
	for (int i=0; i<n; i++, data++)
		rb_ary_store(list,in->dex+i+2,INTORFLOAT2NUM(*data));
} GRID_FINISH {
	send_out(rb_ary_len(list),rb_ary_ptr(list));
	list = 0;
	rb_ivar_set(rself,SI(@list),Qnil); /* unkeep */
} GRID_END

GRCLASS(GridExportList,LIST(GRINLET4(GridExportList,0,4)))
/* outlet 0 not used for grids */
{ IEVAL(rself,"install '@export_list',1,1"); }
\end class GridExportList

/* **************************************************************** */
/*
  GridStore ("@store") is the class for storing a grid and restituting
  it on demand. The right inlet receives the grid. The left inlet receives
  either a bang (which forwards the whole image) or a grid describing what to
  send.
*/

/*{ Dim[*As,B],Dim[*Cs,*Ds] -> Dim[*As,*Ds] }*/
/* in0: integer nt */
/* in1: whatever nt */
/* out0: same nt as in1 */
\class GridStore < GridObject
struct GridStore : GridObject {
	Grid r; //\attr
	\decl void initialize (Grid *r=0);
	\decl void _0_bang ();
	GRINLET3(0);
	GRINLET3(1);
};

/* takes the backstore of a grid and puts it back into place.
   a backstore is a grid that is filled while the grid it would
   replace has not finished being used.
*/
static void snap_backstore (Grid &r) {
	if (r.next) { r.swallow(r.next); r.next = 0; }
}

/*!@#$ i should ensure that n is not exceedingly large */
/*!@#$ worse: the size of the foo buffer may still be too large */
GRID_INLET(GridStore,0) {
	/* snap_backstore must be done before *anything* else */
	snap_backstore(r);

	NOTEMPTY(r);

	int na = in->dim->n;
	int nb = r.dim->n;
	int nc = in->dim->get(na-1);
	STACK_ARRAY(int32,v,MAX_DIMENSIONS);

	if (na<1) RAISE("must have at least 1 dimension.",na,1,1+nb);

	int lastindexable = r.dim->prod()/r.dim->prod(nc) - 1;
	int ngreatest = nt_greatest((T *)0);
	if (lastindexable > ngreatest) {
		RAISE("lastindexable=%d > ngreatest=%d (ask matju)",lastindexable,ngreatest);
	}
	
	if (nc > nb)
		RAISE("wrong number of elements in last dimension: "
			"got %d, expecting <= %d", nc, nb);
	int nd = nb - nc + na - 1;
	if (nd > MAX_DIMENSIONS) RAISE("too many dimensions!");

	COPY(v,in->dim->v,na-1);
	COPY(v+na-1,r.dim->v+nc,nb-nc);
	out[0]->begin(new Dim(nd,v),r.nt);
	if (nc>0) in->set_factor(nc);
} GRID_FLOW {
	int na = in->dim->n;
	int nc = in->dim->get(na-1);
	int size = r.dim->prod(nc);
	assert((n % nc) == 0);
	int nd = n/nc;
	STACK_ARRAY(T,w,n);
	Pt<T> v=w;
	switch (nc) {
	//case 1:  COPY(v,data,n); break;
	case 1:  v=data; break;
	default: for (int k=0,i=0; i<nc; i++) for (int j=0; j<n; j+=nc) v[k++] = data[i+j]; break;
	}
	if (sizeof(T)==1 && nc==1 && r.dim->v[0]<=256) goto skip;
	for (int i=0; i<nc; i++) {
		int32 wrap = r.dim->v[i];
		bool is_power_of_two = lowest_bit(wrap)==highest_bit(wrap);
		if (i) {
			if (is_power_of_two) {
				op2_shl->map(nd,v,(T)highest_bit(wrap));
			} else {
				op2_mul->map(nd,v,(T)wrap);
			}
		}
		if (is_power_of_two) {
			op2_and->map(nd,v+nd*i,(T)(wrap-1));
		} else {
			op2_mod->map(nd,v+nd*i,(T)(wrap));
		}
		if (i) op2_add->zip(nd,v,v+nd*i);
	}
	skip:;
/*
#define FOO(type) { \
	Pt<type> p = (Pt<type>)r; \
	if (size<=16) { \
		Pt<type> foo = ARRAY_NEW(type,nd*size); \
		switch (size) { \
		case 1: for (int i=0; i<nd; i++, foo+=size) SCOPY<1>::f(foo,p+1*v[i]); break; \
		case 2: for (int i=0; i<nd; i++, foo+=size) SCOPY<2>::f(foo,p+2*v[i]); break; \
		case 3: for (int i=0; i<nd; i++, foo+=size) SCOPY<3>::f(foo,p+3*v[i]); break; \
		case 4: for (int i=0; i<nd; i++, foo+=size) SCOPY<4>::f(foo,p+4*v[i]); break; \
		default: for (int i=0; i<nd; i++, foo+=size) COPY(foo,p+size*v[i],size); \
		}; \
		out[0]->give(size*nd,foo-size*nd); \
	} else { \
		for (int i=0; i<nd; i++) out[0]->send(size,p+size*v[i]); \
	} \
}
*/

#define FOO(type) { \
	Pt<type> p = (Pt<type>)r; \
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
		out[0]->give(size*nd,foo-size*nd); \
	} else { \
		for (int i=0; i<nd; i++) out[0]->send(size,p+size*v[i]); \
	} \
}

/*
#define FOO(type) { \
	Pt<type> p = (Pt<type>)r; \
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
		case 2: for (; i<nd&-4; i+=4, foo+=4*2) { \
			SCOPY<2>::f(foo+0,p+2*v[i+0]); \
			SCOPY<2>::f(foo+2,p+2*v[i+1]); \
			SCOPY<2>::f(foo+4,p+2*v[i+2]); \
			SCOPY<2>::f(foo+6,p+2*v[i+3]); \
		} break; \
		case 3: for (; i<nd&-4; i+=4, foo+=4*3) { \
			SCOPY<3>::f(foo+0,p+3*v[i+0]); \
			SCOPY<3>::f(foo+3,p+3*v[i+1]); \
			SCOPY<3>::f(foo+6,p+3*v[i+2]); \
			SCOPY<3>::f(foo+9,p+3*v[i+3]); \
		} break; \
		case 4: for (; i<nd&-4; i+=4, foo+=4*4) { \
			SCOPY<4>::f(foo+0,p+4*v[i+0]); \
			SCOPY<4>::f(foo+4,p+4*v[i+1]); \
			SCOPY<4>::f(foo+8,p+4*v[i+2]); \
			SCOPY<4>::f(foo+12,p+4*v[i+3]); \
		} break; \
		default:; }; \
		for (; i<nd; i++, foo+=size) COPY(foo,p+size*v[i],size); \
		out[0]->give(size*nd,foo-size*nd); \
	} else { \
		for (int i=0; i<nd; i++) out[0]->send(size,p+size*v[i]); \
	} \
}
*/

	TYPESWITCH(r.nt,FOO,)
#undef FOO
} GRID_FINISH {
	GridOutlet *o = out[0];
	if (in->dim->prod()==0) {
		int n = in->dim->prod(0,-2);
		int size = r.dim->prod();
#define FOO(T) while (n--) o->send(size,(Pt<T>)r);
		TYPESWITCH(r.nt,FOO,)
#undef FOO
	}
} GRID_END

GRID_INPUT2(GridStore,1,r) {} GRID_END

\def void initialize (Grid *r) {
	rb_call_super(argc,argv);
	if (r) this->r.swallow(r);
}

\def void _0_bang () {
	rb_funcall(rself,SI(_0_list),3,INT2NUM(0),SYM(#),INT2NUM(0));
}

GRCLASS(GridStore,LIST(GRINLET2(GridStore,0,4),GRINLET4(GridStore,1,4)),
	\grdecl
) { IEVAL(rself,"install '@store',2,1"); }

\end class GridStore

/* **************************************************************** */

/*{ Dim[*As]<T> -> Dim[*As]<T> }*/

\class GridOp1 < GridObject
struct GridOp1 : GridObject {
	\attr Numop1 *op;
	\decl void initialize (Numop1 *op);
	GRINLET3(0);
};

GRID_INLET(GridOp1,0) {
	out[0]->begin(in->dim->dup(),in->nt);
} GRID_FLOW {
	op->map(n,data);
	out[0]->give(n,data);
} GRID_FINISH {
} GRID_END

\def void initialize (Numop1 *op) {
	rb_call_super(argc,argv);
	this->op = op;
}

GRCLASS(GridOp1,LIST(GRINLET4(GridOp1,0,6)),
	\grdecl
){ IEVAL(rself,"install '@!',1,1"); }

\end class GridOp1

/* **************************************************************** */
/*
  GridOp2 ("@") is the class of objects for parallel operation ("pairwise") on the
  values of the left grid with values of a (stored) right grid or (stored)
  single value.
*/

\class GridOp2 < GridObject
struct GridOp2 : GridObject {
	\attr Numop2 *op;
	Grid r;
	\decl void initialize(Numop2 *op, Grid *r=0);
	GRINLET3(0);
	GRINLET3(1);
};

/*{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As]<T> }*/

GRID_INLET(GridOp2,0) {
	snap_backstore(r);

	SAME_TYPE(*in,r);
	out[0]->begin(in->dim->dup(),in->nt);
} GRID_FLOW {
	NOTEMPTY(r);
	Pt<T> rdata = (Pt<T>)r;
	int loop = r.dim->prod();
	if (loop>1) {
		if (in->dex+n <= loop) {
			op->zip(n,data,rdata+in->dex);
		} else {
			/* !@#$ should prebuild and reuse this array when "loop" is small */
			STACK_ARRAY(T,data2,n);
			int ii = mod(in->dex,loop);
			int m = min(loop-ii,n);
			COPY(data2,rdata+ii,m);
			int nn = m+((n-m)/loop)*loop;
			for (int i=m; i<nn; i+=loop) COPY(data2+i,rdata,loop);
			if (n>nn) COPY(data2+nn,rdata,n-nn);
			op->zip(n,data,data2);
		}
	} else {
		op->map(n,data,*rdata);
	}
	out[0]->give(n,data);
} GRID_FINISH {
} GRID_END

GRID_INPUT2(GridOp2,1,r) {} GRID_END

\def void initialize(Numop2 *op, Grid *r=0) {
	rb_call_super(argc,argv);
	this->op = op;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0), int32_type_i);
}

GRCLASS(GridOp2,LIST(GRINLET4(GridOp2,0,6),GRINLET4(GridOp2,1,4)),
	\grdecl
) { IEVAL(rself,"install '@',2,1"); }

\end class GridOp2

/* **************************************************************** */
/*
  GridFold ("@fold") is the class of objects for removing the last dimension
  by cascading an operation on all those values. There is a start value. When
  doing [@fold + 42] each new value is computed like 42+a+b+c+...
*/

/*{ Dim[*As,*Bs]<T>,Dim[*Bs]<T> -> Dim[*As]<T> }*/

\class GridFold < GridObject
struct GridFold : GridObject {
	\attr Numop2 *op;
	\attr Grid seed;
	\decl void initialize (Numop2 *op, Grid *seed=0);
	GRINLET3(0);
	GRINLET3(1);
};

/* fold: dim(*X,Y,*Z) x dim(*Z) -> dim(*X,*Z) */
GRID_INLET(GridFold,0) {
	NOTEMPTY(seed);
	SAME_TYPE(*in,seed);
	int an = in->dim->n;
	int bn = seed.dim->n;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand "
		"(%d vs %d)",an,bn);
	STACK_ARRAY(int32,v,an-1);
	int yi = an-bn-1;
	COPY(v,in->dim->v,yi);
	COPY(v+yi,in->dim->v+an-bn,bn);
	SAME_DIM(an-(yi+1),in->dim,(yi+1),seed.dim,0);
	out[0]->begin(new Dim(an-1,v),in->nt);
	in->set_factor(in->dim->get(yi)*seed.dim->prod());
} GRID_FLOW {
	int an = in->dim->n;
	int bn = seed.dim->n;
	int yn = in->dim->v[an-bn-1];
	int zn = in->dim->prod(an-bn);
	STACK_ARRAY(T,buf,n/yn);
	int nn=n;
	int yzn=yn*zn;
	for (int i=0; n; i+=zn, data+=yzn, n-=yzn) {
		COPY(buf+i,((Pt<T>)seed),zn);
		op->fold(zn,yn,buf+i,data);
	}
	out[0]->send(nn/yn,buf);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridFold,1,r) {} GRID_END

\def void initialize (Numop2 *op, Grid *seed=0) {
	rb_call_super(argc,argv);
	this->op = op;
	if (seed) this->seed.swallow(seed); else this->seed.init_clear(new Dim(0,0), int32_type_i);
}

GRCLASS(GridFold,LIST(GRINLET4(GridFold,0,4)),
	\grdecl
) { IEVAL(rself,"install '@fold',2,1"); }

\end class GridFold

/* **************************************************************** */
/*
  GridScan ("@scan") is similar to @fold except that it gives back all
  the partial results thereof; therefore the output is of the same
  size as the input (unlike @fold).
*/

/*{ Dim[*As,*Bs]<T>,Dim[*Bs]<T> -> Dim[*As,*Bs]<T> }*/

\class GridScan < GridFold
struct GridScan : GridFold {
	\decl void initialize (Numop2 *op, Grid *seed=0);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridScan,0) {
	NOTEMPTY(seed);
	SAME_TYPE(*in,seed);
	int an = in->dim->n;
	int bn = seed.dim->n;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand");
	SAME_DIM(bn,in->dim,an-bn,seed.dim,0);
	out[0]->begin(in->dim->dup(),in->nt);
	in->set_factor(in->dim->prod(an-bn-1));
} GRID_FLOW {
	int an = in->dim->n;
	int bn = seed.dim->n;
	int yn = in->dim->v[an-bn-1];
	int zn = in->dim->prod(an-bn);
	int factor = in->factor;
	STACK_ARRAY(T,buf,n);
	COPY(buf,data,n);
	for (int i=0; i<n; i+=factor) op->scan(zn,yn,(Pt<T>)seed,buf+i);
	out[0]->send(n,buf);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridScan,1,r) {} GRID_END

\def void initialize (Numop2 *op, Grid *seed=0) {
	rb_call_super(argc,argv);
	this->op = op;
	if (seed) this->seed.swallow(seed); else this->seed.init_clear(new Dim(0,0), int32_type_i);
}

GRCLASS(GridScan,LIST(GRINLET4(GridScan,0,4)),
	\grdecl
) { IEVAL(rself,"install '@scan',2,1"); }

\end class GridScan

/* **************************************************************** */
/* inner: (op_para,op_fold,seed,A in dim(*As,A0), B in dim(B0,*Bs))
          -> c in dim(*As,*Bs)
   c = map((*As,*Bs),fold(op_fold,seed,zip(op_para,...... whatever
*/

/* transpose=false: */
/*{ Dim[*As,C]<T>,Dim[C,*Bs]<T> -> Dim[*As,*Bs]<T> }*/

/* transpose=true: */
/*{ Dim[*As,C]<T>,Dim[*Bs,C]<T> -> Dim[*As,*Bs]<T> }*/

\class GridInner < GridObject
struct GridInner : GridObject {
	\attr Numop2 *op_para;
	\attr Numop2 *op_fold;
	\attr Grid seed;
	Grid r;

	bool transpose;
	Grid r2;
	
	GridInner() { transpose=false; }
	\decl void initialize (Numop2 *op_para=op2_mul, Numop2 *op_fold=op2_add, Grid *seed=0, Grid *r=0);
	GRINLET3(0);
	GRINLET3(2);
	template <class T> void process_right(T bogus);
};

#define MAX_PACKET_SIZE (1<<11)

GRID_INLET(GridInner,0) {
	NOTEMPTY(r);
	NOTEMPTY(seed);
	SAME_TYPE(*in,r);
	SAME_TYPE(*in,seed);
	Dim *a = in->dim;
	Dim *b = r.dim;
	if (a->n<1) RAISE("a: minimum 1 dimension");
	if (b->n<1) RAISE("b: minimum 1 dimension");
	if (seed.dim->n != 0) RAISE("seed must be a scalar");
	int a_last = a->get(a->n-1);
	int b_first = b->get(0);
	int n = a->n+b->n-2;
	SAME_DIM(1,a,a->n-1,b,0);
	STACK_ARRAY(int32,v,n);
	COPY(v,a->v,a->n-1);
	COPY(v+a->n-1,b->v+1,b->n-1);
	out[0]->begin(new Dim(n,v),in->nt);
	in->set_factor(a_last);

	int rrows = in->factor;
	int rsize = r.dim->prod();
	int rcols = rsize/rrows;
	Pt<T> rdata = (Pt<T>)r;
	int chunk = MAX_PACKET_SIZE/rsize;
	v[0] = chunk*rsize;
	r2.init(new Dim(1,v),r.nt);
	Pt<T> buf3 = (Pt<T>)r2;
	for (int i=0; i<rrows; i++)
		for (int j=0; j<chunk; j++)
			COPY(buf3+(j+i*chunk)*rcols,rdata+i*rcols,rcols);
} GRID_FLOW {
	Numop2 *op2_put = FIX2PTR(Numop2,rb_hash_aref(op2_dict,SYM(put)));
	int rrows = in->factor;
	int rsize = r.dim->prod();
	int rcols = rsize/rrows;
	Pt<T> rdata = (Pt<T>)r;
	int chunk = MAX_PACKET_SIZE/rsize;
	STACK_ARRAY(T,buf ,chunk*rcols);
	STACK_ARRAY(T,buf2,chunk*rcols);
	int off = chunk;
	while (n) {
		if (chunk*rrows>n) chunk=n/rrows;
		op2_put->map(chunk*rcols,buf2,*(T *)seed);
		for (int i=0; i<rrows; i++) {
			for (int j=0; j<chunk; j++)
				op2_put->map(rcols,buf+j*rcols,data[i+j*rrows]);
			op_para->zip(chunk*rcols,buf,(Pt<T>)r2+i*off*rcols);
			op_fold->zip(chunk*rcols,buf2,buf);
		}
		out[0]->send(chunk*rcols,buf2);
		n-=chunk*rrows;
		data+=chunk*rrows;
	}
} GRID_FINISH {
	r2.del();
} GRID_END

template <class T> void GridInner::process_right(T bogus) {
	if (!transpose) return;
	int n = r.dim->n;
	int rrows = r.dim->get(n-1);
	int rsize = r.dim->prod();
	if (rrows==0) RAISE("transpose: rows=0 ???");
	int rcols = rsize/rrows;
	STACK_ARRAY(int32,v,n);
	for (int i=0; i<n-1; i++) v[i+1]=r.dim->v[i];
	v[0]=r.dim->v[n-1];
	Pt<T> rdata = (Pt<T>)r;
	STACK_ARRAY(T,r2data,r.dim->prod());
	for (int i=0; i<rrows; i++) {
		for (int j=0; j<rcols; j++) r2data[i*rcols+j] = rdata[j*rrows+i];
	}
	r.init(new Dim(n,v),r.nt);
	COPY((Pt<T>)r,r2data,rsize);
}

GRID_INPUT(GridInner,2,r) {
	process_right((T)0);
} GRID_END

\def void initialize (Numop2 *op_para, Numop2 *op_fold, Grid *seed, Grid *r) {
	rb_call_super(argc,argv);
	this->op_para = op_para;
	this->op_fold = op_fold;
	this->seed.swallow(seed); // this->seed = *seed;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0), int32_type_i);
#define FOO(T) process_right((T)0);
		TYPESWITCH(this->r.nt,FOO,)
#undef FOO
}

GRCLASS(GridInner,LIST(GRINLET4(GridInner,0,4),GRINLET4(GridInner,2,4)),
	\grdecl
) { IEVAL(rself,"install '@inner',3,1"); }

\end class GridInner

/* **************************************************************** */

\class GridInner2 < GridInner
struct GridInner2 : GridInner {
	GridInner2() { transpose=true; }
	\decl void initialize (Numop2 *op_para=op2_mul, Numop2 *op_fold=op2_add, Grid *seed=0, Grid *r=0);
};

\def void initialize (Numop2 *op_para, Numop2 *op_fold, Grid *seed, Grid *r) {
	rb_call_super(argc,argv);
	this->op_para = op_para;
	this->op_fold = op_fold;
	this->seed.swallow(seed); // this->seed = *seed;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0), int32_type_i);
#define FOO(T) process_right((T)0);
		TYPESWITCH(this->r.nt,FOO,)
#undef FOO
}

GRCLASS(GridInner2,LIST(GRINLET4(GridInner,0,4),GRINLET4(GridInner,2,4)),
	\grdecl
) { IEVAL(rself,"install '@inner2',3,1"); }

\end class GridInner2

/* **************************************************************** */

/*{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As,*Bs]<T> }*/

\class GridOuter < GridObject
struct GridOuter : GridObject {
	\attr Numop2 *op;
	Grid r;

	\decl void initialize (Numop2 *op, Grid *r=0);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridOuter,0) {
	NOTEMPTY(r);
	SAME_TYPE(*in,r);
	Dim *a = in->dim;
	Dim *b = r.dim;
	int n = a->n+b->n;
	STACK_ARRAY(int32,v,n);
	COPY(v,a->v,a->n);
	COPY(v+a->n,b->v,b->n);
	out[0]->begin(new Dim(n,v),in->nt);
} GRID_FLOW {
	int b_prod = r.dim->prod();
	if (b_prod > 4) {
		STACK_ARRAY(T,buf,b_prod);
		while (n) {
			for (int j=0; j<b_prod; j++) buf[j] = *data;
			op->zip(b_prod,buf,(Pt<T>)r);
			out[0]->send(b_prod,buf);
			data++; n--;
		}
		return;
	}
	n*=b_prod;
	Pt<T> buf = ARRAY_NEW(T,n);
	STACK_ARRAY(T,buf2,b_prod*64);
	for (int i=0; i<64; i++) COPY(buf2+i*b_prod,(Pt<T>)r,b_prod);
	switch (b_prod) {
	case 1:	for (int i=0,k=0; k<n; i++) {buf[k++]=data[i];}
	break;
	case 2: for (int i=0,k=0; k<n; i++) {buf[k++]=data[i];buf[k++]=data[i];}
	break;
	case 3:	for (int i=0,k=0; k<n; i++)
		{buf[k++]=data[i];buf[k++]=data[i];buf[k++]=data[i];}
	break;
	case 4:	for (int i=0,k=0; k<n; i++)
		{buf[k++]=data[i];buf[k++]=data[i];buf[k++]=data[i];buf[k++]=data[i];}
	break;
	default:
	for (int i=0,k=0; k<n; i++) for (int j=0; j<b_prod; j++, k++) buf[k]=data[i];
	}
	int nn=n/(64*b_prod)*(64*b_prod);
	for (int j=0; j<nn; j+=64*b_prod) op->zip(64*b_prod,buf+j,buf2);
	op->zip(n-nn,buf+nn,buf2);
	out[0]->give(n,buf);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridOuter,1,r) {} GRID_END

\def void initialize (Numop2 *op, Grid *r) {
	rb_call_super(argc,argv);
	this->op = op;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0), int32_type_i);
}

GRCLASS(GridOuter,LIST(GRINLET4(GridOuter,0,4),GRINLET4(GridOuter,1,4)),
	\grdecl
) { IEVAL(rself,"install '@outer',2,1"); }

\end class GridOuter

/* **************************************************************** */

\class GridFor < GridObject
struct GridFor : GridObject {
	\attr Grid from;
	\attr Grid to;
	\attr Grid step;

	GridFor () {
		from.constrain(expect_max_one_dim);
		to  .constrain(expect_max_one_dim);
		step.constrain(expect_max_one_dim);
	}
	\decl void initialize (Grid *from, Grid *to, Grid *step);
	\decl void _0_set (Grid *r=0);
	\decl void _0_bang ();
	GRINLET3(0);
	GRINLET3(1);
	GRINLET3(2);
	template <class T> void trigger (T bogus);
};

/*{ Dim[]<T>,Dim[]<T>,Dim[]<T> -> Dim[A]<T> }
  or
  { Dim[B]<T>,Dim[B]<T>,Dim[B]<T> -> Dim[*As,B]<T> }*/

\def void initialize (Grid *from, Grid *to, Grid *step) {
	rb_call_super(argc,argv);
	this->from.swallow(from);
	this->to  .swallow(to  );
	this->step.swallow(step);
}

template <class T>
void GridFor::trigger (T bogus) {
	int n = from.dim->prod();
	int32 nn[n+1];
	STACK_ARRAY(T,x,64*n);
	Pt<T> fromb = ((Pt<T>)from);
	Pt<T>   tob = ((Pt<T>)to  );
	Pt<T> stepb = ((Pt<T>)step);
	STACK_ARRAY(T,to2,n);
	
	for (int i=step.dim->prod()-1; i>=0; i--)
		if (!stepb[i]) RAISE("step must not contain zeroes");
	for (int i=0; i<n; i++) {
		nn[i] = (tob[i] - fromb[i] + stepb[i] - cmp(stepb[i],(T)0)) / stepb[i];
		if (nn[i]<0) nn[i]=0;
		to2[i] = fromb[i]+stepb[i]*nn[i];
	}
	Dim *d;
	if (from.dim->n==0) {
		d = new Dim(1,(int32 *)nn);
	} else {
		nn[n]=n;
		d = new Dim(n+1, (int32 *)nn);
	}
	int total = d->prod();
	out[0]->begin(d,from.nt);
	if (total==0) return;
	int k=0;
	for(int d=0;;d++) {
		/* here d is the dim# to reset; d=n for none */
		for(;d<n;d++) x[k+d]=fromb[d];
		k+=n;
		if (k==64*n) {
			out[0]->send(k,x);
			k=0;
			COPY(x,x+63*n,n);
		} else {
			COPY(x+k,x+k-n,n);
		}
		d--;
		/* here d is the dim# to increment */
		for(;;d--) {
			if (d<0) goto end;
			x[k+d]+=stepb[d];
			if (x[k+d]!=to2[d]) break;
		}
	}
	end: if (k) out[0]->send(k,x);
}

\def void _0_bang () {
	SAME_TYPE(from,to);
	SAME_TYPE(from,step);
	if (!from.dim->equal(to.dim) || !to.dim->equal(step.dim))
		RAISE("dimension mismatch");
#define FOO(T) trigger((T)0);
	TYPESWITCH_NOFLOAT(from.nt,FOO,);
#undef FOO
}

\def void _0_set (Grid *r) {
	from.init_from_ruby(argv[0]);
}

GRID_INPUT(GridFor,2,step) {} GRID_END
GRID_INPUT(GridFor,1,to) {} GRID_END
GRID_INPUT(GridFor,0,from) {_0_bang(0,0);} GRID_END

GRCLASS(GridFor,LIST(GRINLET4(GridFor,0,4),GRINLET4(GridFor,1,4),GRINLET4(GridFor,2,4)),
	\grdecl
) { IEVAL(rself,"install '@for',3,1"); }

\end class GridFor

/* **************************************************************** */

\class GridFinished < GridObject
struct GridFinished : GridObject {
	GRINLET3(0);
};

/* nt N/A */
GRID_INLET(GridFinished,0) {
} GRID_FLOW {
} GRID_FINISH {
	Ruby a[] = { INT2NUM(0), bsym._bang };
	send_out(COUNT(a),a);
} GRID_END

GRCLASS(GridFinished,LIST(GRINLET4(GridFinished,0,0)),
	\grdecl
) { IEVAL(rself,"install '@finished',1,1"); }

\end class GridFinished

/* **************************************************************** */
/*{ Dim[*As] -> Dim[B] }*/

\class GridDim < GridObject
struct GridDim : GridObject {
	GRINLET3(0);
};

GRID_INLET(GridDim,0) {
	int32 v[1] = {in->dim->n};
	out[0]->begin(new Dim(1,v));
	out[0]->send(v[0],Pt<int32>((int32 *)in->dim->v,in->dim->n));
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

GRCLASS(GridDim,LIST(GRINLET4(GridDim,0,0)))
{ IEVAL(rself,"install '@dim',1,1"); }
\end class GridDim

/* **************************************************************** */

\class GridType < GridObject
struct GridType : GridObject {
	GRINLET3(0);
};

GRID_INLET(GridType,0) {
	Ruby a[] = { INT2NUM(0), SYM(symbol), number_type_table[in->nt].sym };
	send_out(COUNT(a),a);
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

GRCLASS(GridType,LIST(GRINLET4(GridType,0,0)))
{ IEVAL(rself,"install '@type',1,1"); }
\end class GridType

/* **************************************************************** */

/*{ Dim[*As]<T>,Dim[B] -> Dim[*Cs]<T> }*/

\class GridRedim < GridObject
struct GridRedim : GridObject {
	\attr Dim *dim;
	Grid dim_grid;

	Grid temp; /* temp.dim is not of the same shape as dim */

	~GridRedim() { if (dim) delete dim; }
	\decl void initialize (Grid *d);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridRedim,0) {
	int a = in->dim->prod(), b = dim->prod();
	if (a<b) {int32 v[1]={a}; temp.init(new Dim(1,v),in->nt);}
	out[0]->begin(dim->dup(),in->nt);
} GRID_FLOW {
	int i = in->dex;
	if (temp.is_empty()) {
		int b = dim->prod();
		int n2 = min(n,b-i);
		if (n2>0) out[0]->send(n2,data);
		/* discard other values if any */
	} else {
		int a = in->dim->prod();
		int n2 = min(n,a-i);
		COPY(((Pt<T>)temp)+i,data,n2);
		if (n2>0) out[0]->send(n2,data);
	}
} GRID_FINISH {
	if (!temp.is_empty()) {
		int a = in->dim->prod(), b = dim->prod();
		for (int i=a; i<b; i+=a) out[0]->send(min(a,b-i),(Pt<T>)temp);
	}
	temp.del();
} GRID_END

GRID_INPUT(GridRedim,1,dim_grid) { dim = dim_grid.to_dim(); } GRID_END

\def void initialize (Grid *d) {
	rb_call_super(argc,argv);
	dim_grid.swallow(d);
	expect_dim_dim_list(dim_grid.dim);
	dim = dim_grid.to_dim();
}

GRCLASS(GridRedim,LIST(GRINLET4(GridRedim,0,4),GRINLET(GridRedim,1,4)),
	\grdecl
) { IEVAL(rself,"install '@redim',2,1"); }

\end class GridRedim

/* **************************************************************** */

\class GridJoin < GridObject
struct GridJoin : GridObject {
	\attr int which_dim;

	Grid r;
	GRINLET3(0);
	GRINLET3(1);
	\decl void initialize (int which_dim=-1, Grid *r=0);
};

GRID_INLET(GridJoin,0) {
	NOTEMPTY(r);
	SAME_TYPE(*in,r);
	Dim *d = in->dim;
	if (d->n != r.dim->n) RAISE("wrong number of dimensions");
	int w = which_dim;
	if (w<0) w+=d->n;
	if (w<0 || w>=d->n)
		RAISE("can't join on dim number %d on %d-dimensional grids",
			which_dim,d->n);
	STACK_ARRAY(int32,v,d->n);
	for (int i=0; i<d->n; i++) {
		v[i] = d->get(i);
		if (i==w) {
			v[i]+=r.dim->v[i];
		} else {
			if (v[i]!=r.dim->v[i]) RAISE("dimensions mismatch: dim #%i, left is %d, right is %d",i,v[i],r.dim->v[i]);
		}
	}
	out[0]->begin(new Dim(d->n,v),in->nt);
	if (d->prod(w)) in->set_factor(d->prod(w));
} GRID_FLOW {
	int w = which_dim;
	if (w<0) w+=in->dim->n;
	int a = in->factor;
	int b = r.dim->prod(w);
	Pt<T> data2 = (Pt<T>)r + in->dex*b/a;
	if (a==3 && b==1) {
		int m = n+n*b/a;
		STACK_ARRAY(T,data3,m);
		Pt<T> data4 = data3;
/*		if (sizeof(T)==1 && is_le()) {
			while (n) {
				*(int32*)data4 = (0x00ffffff & data) | (*(uint8*)data2 << 24);
				n-=3; data+=3; data2+=1; data4+=4;
			}
		}
*/		while (n) {
			SCOPY<3>::f(data4,data); SCOPY<1>::f(data4+3,data2); n-=3; data+=3; data2+=1; data4+=4;
		}
		out[0]->send(m,data3);
	} else if (a+b<=16) {
		int m = n+n*b/a;
		STACK_ARRAY(T,data3,m);
		int i=0;
		while (n) {
			COPY(data3+i,data,a); data+=a; i+=a; n-=a;
			COPY(data3+i,data2,b); data2+=b; i+=b;
		}
		out[0]->send(m,data3);
	} else {
		while (n) {
			out[0]->send(a,data);
			out[0]->send(b,data2);
			data+=a; data2+=b; n-=a;
		}
	}
} GRID_FINISH {
	if (in->dim->prod()==0) out[0]->send(r.dim->prod(),(Pt<T>)r);
} GRID_END

GRID_INPUT(GridJoin,1,r) {} GRID_END

\def void initialize (int which_dim, Grid *r) {
	rb_call_super(argc,argv);
	this->which_dim = which_dim;
	if (r) this->r.swallow(r);
}

GRCLASS(GridJoin,LIST(GRINLET4(GridJoin,0,4),GRINLET4(GridJoin,1,4)),
	\grdecl
) { IEVAL(rself,"install '@join',2,1"); }

\end class GridJoin

/* **************************************************************** */

\class GridGrade < GridObject
struct GridGrade : GridObject {
	GRINLET3(0);
};

template <class T>
struct GradeFunction {
	static int comparator (const void *a, const void *b) {
		return **(T**)a - **(T**)b;
	}
};

#define FOO(S) \
struct GradeFunction<S> { \
	static int comparator (const void *a, const void *b) { \
		S x = **(S**)a - **(S**)b; \
		return x<0 ? -1 : x>0;}};
FOO(float32)
FOO(float64)
#undef FOO

GRID_INLET(GridGrade,0) {
	out[0]->begin(in->dim->dup(),in->nt);
	in->set_factor(in->dim->get(in->dim->n-1));
} GRID_FLOW {
	int m = in->factor;
	STACK_ARRAY(T*,foo,m);
	STACK_ARRAY(T,bar,m);
	for (; n; n-=m,data+=m) {
		for (int i=0; i<m; i++) foo[i] = &data[i];
		qsort(foo,m,sizeof(T),GradeFunction<T>::comparator);
		for (int i=0; i<m; i++) bar[i] = foo[i]-(T *)data;
		out[0]->send(m,bar);
	}
} GRID_FINISH {
} GRID_END

GRCLASS(GridGrade,LIST(GRINLET4(GridGrade,0,4)),
	\grdecl
) { IEVAL(rself,"install '@grade',1,1"); }

\end class GridGrade

/* **************************************************************** */

\class GridPerspective < GridObject
struct GridPerspective : GridObject {
	\attr int32 z;

	GRINLET3(0);
	\decl void initialize (int32 z=256);
};

GRID_INLET(GridPerspective,0) {
	int n = in->dim->n;
	STACK_ARRAY(int32,v,n);
	COPY(v,in->dim->v,n);
	v[n-1]--;
	in->set_factor(in->dim->get(in->dim->n-1));
	out[0]->begin(new Dim(n,v),in->nt);
} GRID_FLOW {
	int m = in->factor;
	STACK_ARRAY(T,foo,m);
	for (; n; n-=m,data+=m) {
		op2_mul->map(m-1,data,(T)z);
		op2_div->map(m-1,data,data[m-1]);
		out[0]->send(m-1,data);
	}	
} GRID_FINISH {
} GRID_END

\def void initialize (int32 z) {
	rb_call_super(argc,argv);
	this->z = z;
}

GRCLASS(GridPerspective,LIST(GRINLET4(GridPerspective,0,4)),
	\grdecl
) { IEVAL(rself,"install '@perspective',1,1"); }

\end class GridPerspective

/* **************************************************************** */
/* [rtmetro] */

\class RtMetro < GridObject
struct RtMetro : GridObject {
	\attr int ms; /* millisecond time interval */
	\attr bool on;
	\attr int mode; /* 0=equal; 1=geiger */

	uint64 next_time; /* microseconds since epoch: next time an event occurs */
	uint64 last;      /* microseconds since epoch: last time we checked */

	uint64 delay();

	\decl void initialize (int ms=1000, Symbol mode=Qnil);
	\decl void _0_int (int x);
	\decl void _1_int (int x);
	\decl void delete_m();

	~RtMetro() {
		if (on) {
			gfpost("~RtMetro: deleting rtmetro alarm for self=%08x rself=%08x",
				(long)this,(long)rself);
			MainLoop_remove((void *)rself);
		} else {
			gfpost("~RtMetro: nothing to do");
		}
	}
};

uint64 RtMetro_now() {
	struct timeval nowtv;
	gettimeofday(&nowtv,0);
	return nowtv.tv_sec * 1000000LL + nowtv.tv_usec;
}

uint64 RtMetro::delay() {
	switch (mode) {
		case 0:
			return 1000LL*ms;
		case 1:
			/* Bangs in a geiger counter have a l*dt probability of occuring
			in a dt time interval. Therefore the time till the next event
			follows a Exp(l) law, for which:
			f(t) = l*exp(-l*t)
			F(t) = integral of l*exp(-l*t)*dt = 1-exp(-l*t)
			F^-1(p) = -log(1-p)/l
			*/
			return (uint64)(1000LL*ms*-log(1-drand()));
		default:
			abort();
	}
}

static void RtMetro_alarm(Ruby rself) {
	uint64 now = RtMetro_now();
	DGS(RtMetro);
	if (now >= self->next_time) {
		Ruby a[] = { INT2NUM(0), bsym._bang };
		self->send_out(COUNT(a),a);
		/* self->next_time = now; */ /* jmax style, less realtime */
		self->next_time += self->delay();
	}
	self->last = now;
}

\def void _0_int (int x) {
	int oon = on;
	on = !! x;
	gfpost("on = %d",on);
	if (oon && !on) {
		gfpost("_0_int: deleting rtmetro alarm for self=%08x rself=%08x",
			(long)this,(long)rself);
		MainLoop_remove((void *)rself);
	} else if (!oon && on) {
		gfpost("_0_int: creating rtmetro alarm for self=%08x rself=%08x",
			(long)this,(long)rself);
		MainLoop_add((void *)rself,(void(*)(void*))RtMetro_alarm);
		next_time = RtMetro_now();
	}
}

\def void _1_int (int x) {
	ms = x;
	gfpost("ms = %d",ms);
}

\def void initialize (int ms, Symbol mode) {
	rb_call_super(argc,argv);
	this->ms = ms;
	on = 0;
	mode = 0;
	if (argc>=2) {
		if (mode==Qnil || mode==SYM(equal)) this->mode=0;
		else if (mode==SYM(geiger)) this->mode=1;
		else RAISE("this is not a known mode");
	}		
	gfpost("ms = %d",ms);
	gfpost("on = %d",on);
	gfpost("mode = %d",mode);
}

\def void delete_m () {
	gfpost("RtMetro#del");
	rb_funcall(rself,SI(_0_int),1,INT2NUM(0));
}

GRCLASS(RtMetro,LIST(),
	\grdecl
) { IEVAL(rself,"install 'rtmetro',2,1"); }

\end class RtMetro

/* **************************************************************** */

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
	\startup
}
