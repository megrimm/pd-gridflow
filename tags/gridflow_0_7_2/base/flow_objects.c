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
#include "grid.h"

Operator2 *op2_add;
Operator2 *op2_mul;
Operator2 *op2_shl;
Operator2 *op2_mod;
Operator2 *op2_and;
Operator2 *op2_div;

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

static void expect_rgb_picture (Dim *d) {
	if (d->n!=3) RAISE("(height,width,chans) dimensions please");
	if (d->get(2)!=3) RAISE("(red,green,blue) channels please");
}

static void expect_rgba_picture (Dim *d) {
	if (d->n!=3) RAISE("(height,width,chans) dimensions please");
	if (d->get(2)!=4) RAISE("(red,green,blue,alpha) channels please");
}

/* a variant on GRID_INPUT */
/* there's an inlet 0 hardcoded here, sorry */
/* should be replaced by GRID_INPUT eventually ! */
#define DIM_INPUT(_class_,_inlet_,_member_) \
	GRID_INLET(_class_,1) { \
		expect_dim_dim_list(in->dim); \
		if (in->dim->n) in->set_factor(in->dim->n); \
	} \
	GRID_FLOW { \
		if (dim) { delete _member_; _member_=0; } \
		_member_ = new Dim(n,(int *)(T *)data); \
		this->in[0]->abort(); \
		out[0]->abort(); \
	} \
	GRID_FINISH {}

#define SAME_TYPE(_a_,_b_) \
	if ((_a_).nt != (_b_).nt) RAISE("%s(%s): same type please (%s versus %s)", \
		INFO(this), __PRETTY_FUNCTION__, \
		number_type_table[(_a_).nt].name, \
		number_type_table[(_b_).nt].name);

/* result should be printed immediately as the GC may discard it anytime */
static const char *INFO(GridObject *foo) {
	if (!foo) return "(nil GridObject!?)";
	Ruby z = rb_funcall(foo->rself,SI(args),0);
/*	if (TYPE(z)==T_ARRAY) z = rb_funcall(z,SI(inspect),0); */
	if (z==Qnil) return "(nil args!?)";
	return rb_str_ptr(z);
}

static void SAME_DIM(int n, Dim *a, int ai, Dim *b, int bi) {
	if (ai+n > a->n) RAISE("left hand: not enough dimensions");
	if (bi+n > b->n) RAISE("right hand: not enough dimensions");
	for (int i=0; i<n; i++) {
		if (a->v[ai+i] != b->v[bi+i]) {
			RAISE("mismatch: left dim #%d is %d, right dim #%d is %d",
				ai+i, a->v[ai+i],
				bi+i, b->v[bi+i]);
		}
	}
}

/* **************************************************************** */

\class GridCast < GridObject
struct GridCast : GridObject {
	NumberTypeE nt;
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
	Grid dim_grid;
	Dim *dim; /* size of grids to send */
	NumberTypeE nt;
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
  grid_store ("@store") is the class for storing a grid and restituting
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
	Grid r;
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
	if (r.next != &r) { r.swallow(r.next); r.next = &r; }
}

/*!@#$ i should ensure that n is not exceedingly large */
/*!@#$ worse: the size of the foo buffer may still be too large */
GRID_INLET(GridStore,0) {
	/* snap_backstore must be done before *anything* else */
	snap_backstore(r);

	if (r.is_empty()) RAISE("empty buffer, better luck next time.");

	int na = in->dim->n;
	int nb = r.dim->n;
	int nc = in->dim->get(na-1);
	int32 v[MAX_DIMENSIONS];

	if (na<1) RAISE("must have at least 1 dimension.",na,1,1+nb);

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
	STACK_ARRAY(T,v,n);
	for (int k=0,i=0; i<nc; i++) for (int j=0; j<n; j+=nc) v[k++] = data[i+j];
	for (int i=0; i<nc; i++) {
		int32 wrap = r.dim->v[i];
		bool is_power_of_two = lowest_bit(wrap)==highest_bit(wrap);
		if (i) {
			if (is_power_of_two) {
				op2_shl->map(nd,v,(int32)highest_bit(wrap));
			} else {
				op2_mul->map(nd,v,wrap);
			}
		}
		if (is_power_of_two) {
			op2_and->map(nd,v+nd*i,wrap-1);
		} else {
			op2_mod->map(nd,v+nd*i,wrap);
		}
		if (i) op2_add->zip(nd,v,v+nd*i);
	}

#define FOO(type) { \
		Pt<type> p = (Pt<type>)r; \
		if (size<=16) { \
			Pt<type> foo = ARRAY_NEW(type,nd*size); \
			for (int i=0; i<nd; i++) COPY(foo+size*i,p+size*v[i],size); \
			out[0]->give(size*nd,foo); \
		} else { \
			for (int i=0; i<nd; i++) out[0]->send(size,p+size*v[i]); \
		} \
}

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

GRCLASS(GridStore,LIST(GRINLET(GridStore,0,4),GRINLET4(GridStore,1,4)),
	\grdecl
) { IEVAL(rself,"install '@store',2,1"); }

\end class GridStore

/* **************************************************************** */

/*{ Dim[*As]<T> -> Dim[*As]<T> }*/

\class GridOp1 < GridObject
struct GridOp1 : GridObject {
	Operator1 *op;
	\decl void initialize (Operator1 *op);
	GRINLET3(0);
};

GRID_INLET(GridOp1,0) {
	out[0]->begin(in->dim->dup(),in->nt);
} GRID_FLOW {
	op->map(n,data);
	out[0]->give(n,data);
} GRID_FINISH {
} GRID_END

\def void initialize (Operator1 *op) {
	rb_call_super(argc,argv);
	this->op = op;
}

GRCLASS(GridOp1,LIST(GRINLET4(GridOp1,0,6)),
	\grdecl
){ IEVAL(rself,"install '@!',1,1"); }

\end class GridOp1

/* **************************************************************** */
/*
  GridOp2 ("@") is the class of objects for parallel operation on the
  Rubys of the left grid with Rubys of a (stored) right grid or (stored)
  single Ruby.
*/

\class GridOp2 < GridObject
struct GridOp2 : GridObject {
	Operator2 *op;
	Grid r;
	\decl void initialize(Operator2 *op, Grid *r=0);
	GRINLET3(0);
	GRINLET3(1);
};

/*{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As]<T> }*/

GRID_INLET(GridOp2,0) {
	snap_backstore(r);

	SAME_TYPE(*in,r);
	out[0]->begin(in->dim->dup(),in->nt);
} GRID_FLOW {
	if (r.is_empty()) RAISE("ARGH");
	Pt<T> rdata = (Pt<T>)r;
	int loop = r.dim->prod();
	if (loop>1) {
		if (in->dex+n <= loop) {
			op->zip(n,data,rdata+in->dex);
		} else {
			/* !@#$ should prebuild and reuse this array when n is small */
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

\def void initialize(Operator2 *op, Grid *r=0) {
	rb_call_super(argc,argv);
	this->op = op;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0));
}

GRCLASS(GridOp2,LIST(GRINLET4(GridOp2,0,6),GRINLET4(GridOp2,1,4)),
	\grdecl
) { IEVAL(rself,"install '@',2,1"); }

\end class GridOp2

/* **************************************************************** */
/*
  GridFold ("@fold") is the class of objects for removing the last dimension
  by cascading an operation on all those Rubys. There is a start Ruby. When
  doing [@fold + 42] each new Ruby is computed like 42+a+b+c+...
*/

/*{ Dim[*As,*Bs]<T>,Dim[*Bs]<T> -> Dim[*As]<T> }*/

\class GridFold < GridObject
struct GridFold : GridObject {
	Operator2 *op;
	Grid r;
	\decl void initialize (Operator2 *op, Grid *r=0);
	GRINLET3(0);
	GRINLET3(1);
};

/* fold: dim(*X,Y,*Z) x dim(*Z) -> dim(*X,*Z) */
GRID_INLET(GridFold,0) {
	SAME_TYPE(*in,r);
	int an = in->dim->n;
	int bn = r.dim->n;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand "
		"(%d vs %d)",an,bn);
	int32 v[an-1];
	int yi = an-bn-1;
	COPY(v,in->dim->v,yi);
	COPY(v+yi,in->dim->v+an-bn,bn);
	SAME_DIM(an-(yi+1),in->dim,(yi+1),r.dim,0);
	out[0]->begin(new Dim(COUNT(v),v),in->nt);
	in->set_factor(in->dim->get(yi)*r.dim->prod());
} GRID_FLOW {
	int an = in->dim->n;
	int bn = r.dim->n;
	int yn = in->dim->v[an-bn-1];
	int zn = in->dim->prod(an-bn);
	int factor = in->factor;
	STACK_ARRAY(T,buf,n/yn);
	int i=0;
	int nn=n;
	for (int i=0; n; i+=zn, data+=yn*zn, n-=yn*zn) {
		COPY(buf+i,((Pt<T>)r),zn);
		op->fold(zn,yn,buf+i,data);
	}
	out[0]->send(nn/yn,buf);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridFold,1,r) {} GRID_END

\def void initialize (Operator2 *op, Grid *r=0) {
	rb_call_super(argc,argv);
	this->op = op;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0));
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
	\decl void initialize (Operator2 *op, Grid *r=0);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridScan,0) {
	SAME_TYPE(*in,r);
	int an = in->dim->n;
	int bn = r.dim->n;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand");
	SAME_DIM(bn,in->dim,an-bn,r.dim,0);
	out[0]->begin(in->dim->dup(),in->nt);
	in->set_factor(in->dim->prod(an-bn-1));
} GRID_FLOW {
	int an = in->dim->n;
	int bn = r.dim->n;
	int yn = in->dim->v[an-bn-1];
	int zn = in->dim->prod(an-bn);
	int factor = in->factor;
	STACK_ARRAY(T,buf,n);
	COPY(buf,data,n);
	for (int i=0; i<n; i+=factor) op->scan(zn,yn,(Pt<T>)r,buf+i);
	out[0]->send(n,buf);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridScan,1,r) {} GRID_END

\def void initialize (Operator2 *op, Grid *r=0) {
	rb_call_super(argc,argv);
	this->op = op;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0));
}

GRCLASS(GridScan,LIST(GRINLET4(GridScan,0,4)),
	\grdecl
) { IEVAL(rself,"install '@scan',2,1"); }

\end class GridScan

/* **************************************************************** */
/* inner: (op_para,op_fold,rint,A in dim(*As,A0), B in dim(B0,*Bs))
          -> c in dim(*As,*Bs)
   c = map((*As,*Bs),fold(op_fold,rint,zip(op_para,...... whatever
*/

/* transpose=false: */
/*{ Dim[*As,C]<T>,Dim[C,*Bs]<T> -> Dim[*As,*Bs]<T> }*/

/* transpose=true: */
/*{ Dim[*As,C]<T>,Dim[*Bs,C]<T> -> Dim[*As,*Bs]<T> }*/

\class GridInner < GridObject
struct GridInner : GridObject {
	bool transpose;
	Operator2 *op_para;
	Operator2 *op_fold;
	int32 rint;
	Grid r;

	GridInner() { transpose=false; }
	\decl void initialize (Operator2 *op_para=op2_mul, Operator2 *op_fold=op2_add, int32 rint=0, Grid *r=0);
	GRINLET3(0);
	GRINLET3(2);
	template <class T> void process_right(T bogus);
};

GRID_INLET(GridInner,0) {
	SAME_TYPE(*in,r);
	Dim *a = in->dim;
	Dim *b = r.dim;
	if (!b) RAISE("right inlet has no grid");
	if (a->n<1) RAISE("a: minimum 1 dimension");
	if (b->n<1) RAISE("b: minimum 1 dimension");
	int a_last = a->get(a->n-1);
	int b_first = b->get(0);
	int n = a->n+b->n-2;
	SAME_DIM(1,a,a->n-1,b,0);
	int32 v[n];
	COPY(v,a->v,a->n-1);
	COPY(v+a->n-1,b->v+1,b->n-1);
	out[0]->begin(new Dim(n,v),in->nt);
	in->set_factor(a_last);
} GRID_FLOW {
	int rrows = in->factor;
	int rsize = r.dim->prod();
	int rcols = rsize/rrows;
	Pt<T> rdata = (Pt<T>)r;
	STACK_ARRAY(T,buf2,rcols);
	STACK_ARRAY(T,buf,rsize);
	while (n) {
		for (int i=0,k=0; i<rrows; i++)
			for (int j=0; j<rcols; j++) buf[k++]=data[i];
		for (int j=0; j<rcols; j++) buf2[j] = (T)rint;
		op_para->zip(rsize,buf,rdata);
		op_fold->fold(rcols,rrows,buf2,buf);
		out[0]->send(rcols,buf2);
		n-=rrows;
		data+=rrows;
	}
} GRID_FINISH {
} GRID_END

template <class T> void GridInner::process_right(T bogus) {
	if (!transpose) return;
	int n = r.dim->n;
	int rrows = r.dim->get(n-1);
	int rsize = r.dim->prod();
	int rcols = rsize/rrows;
	int32 v[n];
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

\def void initialize (Operator2 *op_para, Operator2 *op_fold, int32 rint, Grid *r) {
	rb_call_super(argc,argv);
	this->op_para = op_para;
	this->op_fold = op_fold;
	this->rint = rint;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0));
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
	\decl void initialize (Operator2 *op_para=op2_mul, Operator2 *op_fold=op2_add, int32 rint=0, Grid *r=0);
};

\def void initialize (Operator2 *op_para, Operator2 *op_fold, int32 rint, Grid *r) {
	rb_call_super(argc,argv);
	this->op_para = op_para;
	this->op_fold = op_fold;
	this->rint = rint;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0));
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
	Grid r;
	Operator2 *op;
	\decl void initialize (Operator2 *op, Grid *r=0);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridOuter,0) {
	Dim *a = in->dim;
	Dim *b = r.dim;
	if (!b) RAISE("right inlet has no grid");
	SAME_TYPE(*in,r);
	int n = a->n+b->n;
	int32 v[n];
	COPY(v,a->v,a->n);
	COPY(v+a->n,b->v,b->n);
	out[0]->begin(new Dim(n,v),in->nt);
} GRID_FLOW {
	int b_prod = r.dim->prod();
	if (b_prod <= 4) {
		Pt<T> buf = ARRAY_NEW(T,b_prod*n);
		for (int i=0,k=0; i<n; i++) {
			for (int j=0; j<b_prod; j++, k++) buf[k] = data[i];
		}
		for (int j=0; j<n; j++) op->zip(b_prod,buf+b_prod*j,(Pt<T>)r);
		out[0]->give(b_prod*n,buf);
	} else {
		STACK_ARRAY(T,buf,b_prod);
		while (n) {
			for (int j=0; j<b_prod; j++) buf[j] = *data;
			op->zip(b_prod,buf,(Pt<T>)r);
			out[0]->send(b_prod,buf);
			data++; n--;
		}
	}
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridOuter,1,r) {} GRID_END

\def void initialize (Operator2 *op, Grid *r) {
	rb_call_super(argc,argv);
	this->op = op;
	if (r) this->r.swallow(r); else this->r.init_clear(new Dim(0,0));
}

GRCLASS(GridOuter,LIST(GRINLET4(GridOuter,0,4),GRINLET4(GridOuter,1,4)),
	\grdecl
) { IEVAL(rself,"install '@outer',2,1"); }

\end class GridOuter

/* **************************************************************** */
/* the incoming grid is stored as "c" with a margin on the four sides
   of it. the margin is the size of the "b" grid minus one then split in two.
*/   

/*{ Dim[A,B,*Cs]<T>,Dim[D,E]<T> -> Dim[A,B,*Cs]<T> }*/

static void expect_convolution_matrix (Dim *d) {
	if (d->n != 2) RAISE("only exactly two dimensions allowed for now (got %d)",
		d->n);
	/* because odd * odd = odd */
	if (d->prod()&1 == 0) RAISE("even number of elements");
}

\class GridConvolve < GridObject
struct GridConvolve : GridObject {
	Grid c,b;
	Operator2 *op_para, *op_fold;
	int32 rint;
	int margx,margy; /* margins */
	GridConvolve () { b.constrain(expect_convolution_matrix); }	
	\decl void initialize (Operator2 *op_para=op2_mul, Operator2 *op_fold=op2_add, int32 rint=0, Grid *r=0);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridConvolve,0) {
	Dim *da = in->dim, *db = b.dim;
	if (!db) RAISE("right inlet has no grid");
	if (db->n != 2) RAISE("right grid must have two dimensions");
	if (da->n < 2) RAISE("left grid has less than two dimensions");
	SAME_TYPE(*in,b);
	/* bug: da[0]>=db[0] and da[1]>=db[1] are also conditions */
	int32 v[da->n];
	COPY(v,da->v,da->n);
	margy = db->get(0)/2;
	margx = db->get(1)/2;
	v[0] += 2*margy;
	v[1] += 2*margx;
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
		COPY(base+factor,data,margx*l);
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
	COPY(cp+(margy+da->get(0))*ll,cp+margy*ll,     margy*ll);

	/* the real stuff */
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
			for (int i=l-1; i>=0; i--) buf3[i]=rint;
			op_para->zip(l*dbx*dby,buf,buf2);
			op_fold->fold(l,dbx*dby,buf3,buf);
			out[0]->send(l,buf3);
		}
	}
	c.del();
} GRID_END

GRID_INPUT(GridConvolve,1,b) {} GRID_END

\def void initialize (Operator2 *op_para, Operator2 *op_fold, int32 rint, Grid *r) {
	rb_call_super(argc,argv);
	this->op_para = op_para;
	this->op_fold = op_fold;
	this->rint = rint;
	if (r) this->b.swallow(r);
}

GRCLASS(GridConvolve,LIST(GRINLET4(GridConvolve,0,4),GRINLET4(GridConvolve,1,4)),
	\grdecl
) { IEVAL(rself,"install '@convolve',2,1"); }

\end class GridConvolve

/* **************************************************************** */

\class GridFor < GridObject
struct GridFor : GridObject {
	Grid from;
	Grid to;
	Grid step;
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
	STACK_ARRAY(T,x,n);
	Pt<T> fromb = ((Pt<T>)from);
	Pt<T>   tob = ((Pt<T>)to  );
	Pt<T> stepb = ((Pt<T>)step);

	for (int i=step.dim->prod()-1; i>=0; i--)
		if (!stepb[i]) RAISE("step must not contain zeroes");
	for (int i=0; i<n; i++) {
		nn[i] = (tob[i] - fromb[i] + stepb[i] - cmp(stepb[i],(T)0)) / stepb[i];
		if (nn[i]<0) nn[i]=0;
	}
	if (from.dim->n==0) {
		out[0]->begin(new Dim(1,(int32 *)nn), from.nt);
	} else {
		nn[n]=n;
		out[0]->begin(new Dim(n+1, (int32 *)nn), from.nt);
	}
	for(int d=0;;) {
		/* here d is the dim# to reset; d=n for none */
		for(;d<n;d++) x[d]=fromb[d];
		d--;
		out[0]->send(n,x);
		/* here d is the dim# to increment */
		for(;;) {
			if (d<0) return;
			x[d]+=stepb[d];
			if (x[d]<tob[d]) break;
			d--;
		}
		d++;
	}
}

\def void _0_bang () {
	if (!from.dim->equal(to.dim) || !to.dim->equal(step.dim))
		RAISE("dimension mismatch");
	if (from.nt != to.nt || to.nt != step.nt)
		RAISE("type mismatch");
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
	Grid dim_grid;
	Dim *dim;
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

/* ---------------------------------------------------------------- */
/* "@scale_by" does quick scaling of pictures by integer factors */

/*{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }*/
/*!@#$ should support N channels */

\class GridScaleBy < GridObject
struct GridScaleBy : GridObject {
	Grid scale; /* integer scale factor */
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

	/* there are restrictions on grid sizes for efficiency reasons */
	expect_rgb_picture(a);

	/* computing the output's size */
	int32 v[3]={ a->get(0)*scaley, a->get(1)*scalex, a->get(2) };
	out[0]->begin(new Dim(3,v),in->nt);

	/* configuring the input format */
	in->set_factor(a->get(1)*a->get(2));
} GRID_FLOW {
/* this section processes one packet of grid content */
	int rowsize = in->dim->prod(1);
	STACK_ARRAY(T,buf,rowsize*scalex);

	/* for every picture row in this packet... */
	for (; n>0; data+=rowsize, n-=rowsize) {

		/* scale the line x-wise */
		int p=0;
		for (int i=0; i<rowsize; i+=3) {
			for (int k=0; k<scalex; k++) {
				buf[p+0]=data[i+0];
				buf[p+1]=data[i+1];
				buf[p+2]=data[i+2];
				p+=3;
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
	Grid scale;
	int scaley;
	int scalex;
	bool smoothly;
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

\class GridJoin < GridObject
struct GridJoin : GridObject {
	Grid r;
	int which_dim;
	GRINLET3(0);
	GRINLET3(1);
	\decl void initialize (int which_dim=-1, Grid *r=0);
};

GRID_INLET(GridJoin,0) {
	if (r.is_empty()) RAISE("no grid in right inlet");
	SAME_TYPE(*in,r);
	Dim *d = in->dim;
	if (d->n != r.dim->n) RAISE("wrong number of dimensions");
	int w = which_dim;
	if (w<0) w+=d->n;
	if (w<0 || w>=d->n)
		RAISE("can't join on dim number %d on %d-dimensional grids",
			which_dim,d->n);
	int32 v[d->n];
	for (int i=0; i<d->n; i++) {
		v[i] = d->get(i);
		if (i==w) {
			v[i]+=r.dim->v[i];
		} else {
			if (v[i]!=r.dim->v[i]) RAISE("dimensions mismatch");
		}
	}
	out[0]->begin(new Dim(d->n,v));
	in->set_factor(d->prod(w));
} GRID_FLOW {
	Dim *d = in->dim;
	int w = which_dim;
	if (w<0) w+=d->n;
	int a = in->factor;
	int b = r.dim->prod(w);
	Pt<T> data2 = (Pt<T>)r + in->dex*b/a;
	if (a+b<=16) {
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

struct GradeFunction<float32> {
	static int comparator (const void *a, const void *b) {
		float32 x = **(float32**)a - **(float32**)b;
		return x<0 ? -1 : x>0;
	}
};

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
	int32 z;
	GRINLET3(0);
	\decl void initialize (int32 z=256);
};

GRID_INLET(GridPerspective,0) {
	int n = in->dim->n;
	int32 v[n];
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

\class GridLayer < GridObject
struct GridLayer : GridObject {
	Grid r;
	GridLayer() { r.constrain(expect_rgb_picture); }
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridLayer,0) {
	Dim *a = in->dim;
	expect_rgba_picture(a);
	SAME_TYPE(*in,r);
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
//template <class T>
struct Line {
	int32 y1,x1,y2,x2,x,m,pad1,pad2;
};

static void expect_polygon (Dim *d) {
	if (d->n!=2 || d->get(1)!=2) RAISE("expecting Dim[n,2] polygon");
}

\class DrawPolygon < GridObject
struct DrawPolygon : GridObject {
	Operator2 *op;
	Grid color;
	Grid polygon;
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
	if (color.is_empty()) RAISE("no color?");
	if (polygon.is_empty()) RAISE("no polygon?");
	if (lines.is_empty()) RAISE("no lines???");
	SAME_TYPE(*in,color);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	if (in->dim->get(2)!=color.dim->get(0))
		RAISE("image does not have same	number of channels as stored color");
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
				memswap(ld.p+i,ld.p+lines_start,1);
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

/*{ Dim[*As,3]<T> -> Dim[*As,3]<T> }*/

\class GridRGBtoHSV < GridObject
struct GridRGBtoHSV : GridObject {
	GRINLET3(0);
};

/*
  h=42*0: red
  h=42*1: yellow
  h=42*2: green
  h=42*3: cyan
  h=42*4: blue
  h=42*5: magenta
  h=42*6: crap
*/

GRID_INLET(GridRGBtoHSV,0) {
	if (in->dim->n<1) RAISE("at least 1 dim please");
	if (in->dim->get(in->dim->n-1)!=3) RAISE("3 chans please");
	out[0]->begin(in->dim->dup());
	in->set_factor(3);
} GRID_FLOW {
	STACK_ARRAY(T,buf,n);
	Pt<T> p=buf;
	for (int i=0; i<n; i+=3, p+=3, data+=3) {
		int r=data[0], g=data[1], b=data[2];
		int v=p[2]=max(max(r,g),b);
		int m=min(min(r,g),b);
		int d=(v-m)?(v-m):1;
		p[1]=255*(v-m)/(v?v:1);
		p[0] = 
			b==m ? 42*1+(g-r)*42/d :
			r==m ? 42*3+(b-g)*42/d :
			g==m ? 42*5+(r-b)*42/d : 0;
	}
	out[0]->give(n,buf);
} GRID_FINISH {
} GRID_END

GRCLASS(GridRGBtoHSV,LIST(GRINLET2(GridRGBtoHSV,0,4)),
	\grdecl
) { IEVAL(rself,"install '@rgb_to_hsv',1,1"); }

\end class GridRGBtoHSV

/* **************************************************************** */

/*{ Dim[*As,3]<T> -> Dim[*As,3]<T> }*/

\class GridHSVtoRGB < GridObject
struct GridHSVtoRGB : GridObject {
	GRINLET3(0);
};

GRID_INLET(GridHSVtoRGB,0) {
	if (in->dim->n<1) RAISE("at least 1 dim please");
	if (in->dim->get(in->dim->n-1)!=3) RAISE("3 chans please");
	out[0]->begin(in->dim->dup());
	in->set_factor(3);
} GRID_FLOW {
	STACK_ARRAY(T,buf,n);
	Pt<T> p = buf;
	for (int i=0; i<n; i+=3, p+=3, data+=3) {
		int h=mod(data[0],252), s=data[1], v=data[2];
		int j=h%42;
		int k=h/42;
		int m=(255-s)*v/255;
		int d=s*v/255;
		p[0]=(k==4?j:k==5||k==0?42:k==1?42-j:0)*d/42+m;
		p[1]=(k==0?j:k==1||k==2?42:k==3?42-j:0)*d/42+m;
		p[2]=(k==2?j:k==3||k==4?42:k==5?42-j:0)*d/42+m;
	}
	out[0]->give(n,buf);
} GRID_FINISH {
} GRID_END

GRCLASS(GridHSVtoRGB,LIST(GRINLET2(GridHSVtoRGB,0,4)),
	\grdecl
) { IEVAL(rself,"install '@hsv_to_rgb',1,1"); }

\end class GridHSVtoRGB

/* **************************************************************** */
/* [rtmetro] */

\class RtMetro < GridObject
struct RtMetro : GridObject {
	int ms; /* millisecond time interval */
	bool on;
	uint64 next_time; /* microseconds since epoch: next time an event occurs */
	uint64 last;      /* microseconds since epoch: last time we checked */
	int mode; /* 0=equal; 1=geiger */

	uint64 delay();

	\decl void initialize (int ms, Symbol mode);
	\decl void _0_int (int x);
	\decl void _1_int (int x);
	\decl void del ();

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

static double drand() {
	return 1.0*rand()/(RAND_MAX+1.0);
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

\def void del () {
	gfpost("RtMetro#del");
	rb_funcall(rself,SI(_0_int),1,INT2NUM(0));
}

GRCLASS(RtMetro,LIST(),
	\grdecl
) { IEVAL(rself,"install 'rtmetro',2,1"); }

\end class RtMetro

/* **************************************************************** */

static Operator2 *OP2(Ruby x) {
	return FIX2PTR(Operator2,rb_hash_aref(op2_dict,x));
}

void startup_flow_objects () {
	op2_add = OP2(SYM(+));
	op2_mul = OP2(SYM(*));
	op2_shl = OP2(SYM(<<));
	op2_mod = OP2(SYM(%));
	op2_and = OP2(SYM(&));
	op2_div = OP2(SYM(/));
	\startup
}