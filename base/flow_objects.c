/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002 by Mathieu Bouchard

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

/*!@#$ HACK */
typedef int32 Number;

/* **************************************************************** */

#define WATCH(n,ar) { \
	char foo[16*1024], *p=foo; \
	p += sprintf(p,"%s: ",#ar); \
	for (int q=0; q<n; q++) p += sprintf(p,"%ld ",ar[q]); \
	gfpost("%s",foo); \
}

#define rassert(_p_) if (!(_p_)) RAISE(#_p_);

#define IV(s) rb_ivar_get(rself,SI(s))
#define IVS(s,v) rb_ivar_set(rself,SI(s),v)

Operator1 *OP1(Ruby x) {
	Ruby s = rb_hash_aref(op1_dict,x);
	if (s==Qnil) RAISE("expected one-input-operator");
	return FIX2PTR(Operator1,s);
}

Operator2 *OP2(Ruby x) {
	Ruby s = rb_hash_aref(op2_dict,x);
	if (s==Qnil) RAISE("expected two-input-operator");
	return FIX2PTR(Operator2,s);
}

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
#define DIM_INPUT(_class_,_inlet_,_member_) \
	GRID_INLET(_class_,1) { \
		expect_dim_dim_list(in->dim); \
		if (in->dim->n) in->set_factor(in->dim->n); \
	} \
	GRID_FLOW { \
		if (dim) delete _member_, _member_=0; \
		_member_ = new Dim(n,(int *)(Number *)data); \
		this->in[0]->abort(); \
		out[0]->abort(); \
	} \
	GRID_FINISH {}

/* **************************************************************** */
/*
  GridImport ("@import") is the class for converting a old-style stream
  of integers to a streamed grid as used now.
*/

/*{ ?,Dim[B] -> Dim[*Cs] }*/

struct GridImport : GridObject {
	Grid dim_grid;
	Dim *dim; /* size of grids to send */
	GridImport() { dim_grid.constrain(expect_dim_dim_list); }
	~GridImport() { if (dim) delete dim; }
	DECL3(initialize);
	DECL3(_0_reset);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridImport,0) {}
GRID_FLOW {
	GridOutlet *o = out[0];
	while (n) {
		if (!o->is_busy()) o->begin(dim->dup());
		int n2 = min(n,o->dim->prod()-o->dex);
		o->send(n2,data);
		n -= n2;
		data += n2;
	}
}
GRID_FINISH {}
GRID_END

GRID_INPUT(GridImport,1,dim_grid) {	dim = dim_grid.to_dim(); } GRID_END

METHOD3(GridImport,initialize) {
	rb_call_super(argc,argv);
	if (argc!=1) RAISE("wrong number of args");
	dim_grid.init_from_ruby(argv[0]);
	dim = dim_grid.to_dim();
	return Qnil;
}

METHOD3(GridImport,_0_reset) {
	if (out[0]->is_busy()) out[0]->abort();
	return Qnil;
}

GRCLASS(GridImport,"@import",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridImport,0,4),GRINLET(GridImport,1,4)),
	DECL(GridImport,initialize),
	DECL(GridImport,_0_reset))

/* **************************************************************** */
/*
  GridExport ("@export") is the class for converting from streamed grids
  to old-style integer stream.
*/

/*{ Dim[*As] -> ? }*/

struct GridExport : GridObject {
	GRINLET3(0);
};

GRID_INLET(GridExport,0) {}
GRID_FLOW {
	for (int i=0; i<n; i++) {
		Ruby a[] = { INT2NUM(0), sym_int, INT2NUM(data[i]) };
		FObject_send_out(COUNT(a),a,rself);
	}
}
GRID_FINISH {}
GRID_END

GRCLASS(GridExport,"@export",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridExport,0,4)))
/* outlet 0 not used for grids */

/* **************************************************************** */

/*{ Dim[*As] -> ? }*/

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
	rb_ary_store(list,1,sym_list);
} GRID_FLOW {
	for (int i=0; i<n; i++, data++)
		rb_ary_store(list,in->dex+i+2,INT2NUM(*data));
} GRID_FINISH {
	FObject_send_out(rb_ary_len(list),rb_ary_ptr(list),rself);
	list = 0;
	rb_ivar_set(rself,SI(@list),Qnil); /* unkeep */
} GRID_END

GRCLASS(GridExportList,"@export_list",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridExportList,0,4)))
/* outlet 0 not used for grids */

/* **************************************************************** */
/*
  grid_store ("@store") is the class for storing a grid and restituting
  it on demand. The right inlet receives the grid. The left inlet receives
  either a bang (which forwards the whole image) or a grid describing what to
  send.
*/

/*{ Dim[*As,B],Dim[*Cs,*Ds] -> Dim[*As,*Ds] }*/

struct GridStore : GridObject {
	Grid r;
	DECL3(initialize);
	DECL3(_0_bang);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridStore,0) {
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
	out[0]->begin(new Dim(nd,v));
	if (nc>0) in->set_factor(nc);
}

/*!@#$ i should ensure that n is not exceedingly large */
/*!@#$ worse: the size of the foo buffer may still be too large */
GRID_FLOW {
	static Operator2 *op_mod = 0; if (!op_mod) op_mod = OP2(SYM(%));
	static Operator2 *op_mul = 0; if (!op_mul) op_mul = OP2(SYM(*));
	static Operator2 *op_add = 0; if (!op_add) op_add = OP2(SYM(+));
	int na = in->dim->n;
	int nc = in->dim->get(na-1);
	int size = r.dim->prod(nc);
	assert((n % nc) == 0);
	int nd = n/nc;
	STACK_ARRAY(int32,v,n);
	for (int k=0,i=0; i<nc; i++) for (int j=0; j<n; j+=nc) v[k++] = data[i+j];
	for (int i=0; i<nc; i++) {
		if (i) op_mul->on_int32.op_map(nd,(Number*)v,r.dim->v[i]);
		op_mod->on_int32.op_map(nd,(Number*)v+nd*i,r.dim->v[i]);
		if (i) op_add->on_int32.op_map2(nd,(Number*)v,(Number*)v+nd*i);
	}
	if (r.nt==int32_type_i) {
		Pt<int32> p = (Pt<int32>)r;
		if (size<8) {
			STACK_ARRAY(int32,foo,nd*size);
			for (int i=0; i<nd; i++) COPY(foo+size*i,p+size*v[i],size);
			out[0]->send(size*nd,foo);
		} else {
			for (int i=0; i<nd; i++) out[0]->send(size,p+size*v[i]);
		}			
	} else if (r.nt==uint8_type_i) {
		Pt<uint8> p = (Pt<uint8>)r;
		for (int i=0; i<nd; i++) out[0]->send(size,p+size*v[i]);
	} else RAISE("unsupported type");
}

GRID_FINISH {
	GridOutlet *o = out[0];
	if (in->dim->prod()==0) {
		int n = in->dim->prod(0,-2);
		int size = r.dim->prod();
		switch(r.nt) {
		case int32_type_i: while (n--) o->send(size,(Pt<int32>)r); break;
		case uint8_type_i: while (n--) o->send(size,(Pt<uint8>)r); break;
		default: RAISE("unsupported type");
		}
	}
} GRID_END

GRID_INLET(GridStore,1) {
	this->in[0]->abort();
	r.init(in->dim->dup(),r.nt);
} GRID_FLOW {
	if (r.nt==int32_type_i) {
		COPY((Pt<int32>)r + in->dex, data, n);
	} else if (r.nt==uint8_type_i) {
		uint8 *data2 = (Pt<uint8>)r + in->dex;
		for(int i=0; i<n; i++) data2[i] = data[i];
	} else RAISE("unsupported type");
	in->dex += n;
} GRID_FINISH {
} GRID_END

METHOD3(GridStore,initialize) {
	rb_call_super(argc,argv);
	r.init(0, NumberType_find(argc==0 ? SYM(int32) : argv[0]));
	return Qnil;
}

METHOD3(GridStore,_0_bang) {
	rb_funcall(rself,SI(_0_list),3,INT2NUM(0),SYM(#),INT2NUM(0));
	return Qnil;
}

GRCLASS(GridStore,"@store",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridStore,0,4),GRINLET(GridStore,1,4)),
	DECL(GridStore,initialize),
	DECL(GridStore,_0_bang))

/* **************************************************************** */

/*{ Dim[*As] -> Dim[*As] }*/

struct GridOp1 : GridObject {
	const Operator1 *op;
	DECL3(initialize);
	GRINLET3(0);
};

GRID_INLET(GridOp1,0) { out[0]->begin(in->dim->dup()); }
GRID_FLOW {	op->on_int32.op_map(n,(Number*)data); out[0]->give(n,data); }
GRID_FINISH {}
GRID_END

METHOD3(GridOp1,initialize) {
	rb_call_super(argc,argv);
	op = OP1(argv[0]);
	return Qnil;
}

GRCLASS(GridOp1,"@!",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridOp1,0,6)),
	DECL(GridOp1,initialize))

/* **************************************************************** */
/*
  GridOp2 ("@") is the class of objects for parallel operation on the
  Rubys of the left grid with Rubys of a (stored) right grid or (stored)
  single Ruby.
*/

struct GridOp2 : GridObject {
	const Operator2 *op;
	Grid r;
	DECL3(initialize);
	GRINLET3(0);
	GRINLET3(1);
};

/*{ Dim[*As],Dim[*Bs] -> Dim[*As] }*/

GRID_INLET(GridOp2,0) {
	out[0]->begin(in->dim->dup());
} GRID_FLOW {
	if (r.is_empty()) RAISE("ARGH");
	Pt<int32> rdata = (Pt<int32>)r;
	int loop = r.dim->prod();
	if (loop>1) {
		if (in->dex+n <= loop) {
			op->on_int32.op_map2(n,(Number*)data,(Number*)rdata+in->dex);
		} else {
			STACK_ARRAY(Number,data2,n);
			//!@#$ accelerate me
			for (int i=0; i<n;) {
				int ii = (in->dex+i)%loop;
				int m = min(loop-ii,n-i);
				COPY(data2+i,rdata+ii,m);
				i+=m;
			}
			op->on_int32.op_map2(n,(Number*)data,(Number*)data2);
		}
	} else {
		op->on_int32.op_map(n,(Number*)data,*rdata);
	}
	out[0]->give(n,data);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridOp2,1,r) {} GRID_END

METHOD3(GridOp2,initialize) {
	rb_call_super(argc,argv);
	op = OP2(argv[0]);
	if (argc>2) RAISE("too many args");
	if (argc<2) {
		r.init(new Dim(0,0),int32_type_i);
		((Pt<int32>)r)[0] = 0;
	} else {
		r.init_from_ruby(argv[1]);
	}
	return Qnil;
}

GRCLASS(GridOp2,"@",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridOp2,0,6),GRINLET(GridOp2,1,4)),
	DECL(GridOp2,initialize))

/* **************************************************************** */
/*
  GridFold ("@fold") is the class of objects for removing the last dimension
  by cascading an operation on all those Rubys. There is a start Ruby. When
  doing [@fold + 42] each new Ruby is computed like 42+a+b+c+...
*/

/*{ Dim[*As,*Bs],Dim[*Bs] -> Dim[*As] }*/

struct GridFold : GridObject {
	const Operator2 *op;
	Grid r;
	DECL3(initialize);
	GRINLET3(0);
	GRINLET3(1);
};

/* fold: dim(*X,Y,*Z) x dim(*Z) -> dim(*X,*Z) */
GRID_INLET(GridFold,0) {
	int an = in->dim->n;
	int bn = r.dim->n;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand");
	int32 v[an-1];
	int yi = an-bn-1;
	COPY(v,in->dim->v,yi);
	COPY(v+yi,in->dim->v+yi+1,an-yi-1);
	for (int i=yi+1; i<an; i++)
		if (r.dim->v[i-yi-1]!=in->dim->v[i]) RAISE("dimension mismatch");
	out[0]->begin(new Dim(COUNT(v),v));
	in->set_factor(in->dim->get(yi)*r.dim->prod());
} GRID_FLOW {
	int an = in->dim->n;
	int bn = r.dim->n;
	int yn = in->dim->v[an-bn-1];
	int zn = in->dim->prod(an-bn);
	int factor = in->factor;
	STACK_ARRAY(Number,buf,n/yn);
	assert (n % factor == 0);
	int i=0;
	int nn=n;
	while (n) {
		COPY(buf+i,((Pt<int32>)r),zn);
		op->on_int32.op_fold2(zn,(Number*)buf+i,yn,(Number*)data);
		i += zn;
		data += yn*zn;
		n -= yn*zn;
	}
	out[0]->send(nn/yn,buf);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridFold,1,r) {} GRID_END

METHOD3(GridFold,initialize) {
	rb_call_super(argc,argv);
	op = OP2(argv[0]);
	if (argc>2) RAISE("too many args");
	if (argc<2) {
		r.init(new Dim(0,0),int32_type_i);
		((Pt<int32>)r)[0] = 0;
	} else {
		r.init_from_ruby(argv[1]);
	}
	return Qnil;
}

GRCLASS(GridFold,"@fold",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridFold,0,4)),
	DECL(GridFold,initialize))

/* **************************************************************** */
/*
  GridScan ("@scan") is similar to @fold except that it gives back all
  the partial results thereof; therefore the output is of the same
  size as the input (unlike @fold).
*/

/*{ Dim[*As,*Bs],Dim[*Bs] -> Dim[*As,*Bs] }*/

struct GridScan : GridFold {
	DECL3(initialize);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridScan,0) {
	int an = in->dim->n;
	int bn = r.dim->n;
	int yi = an-bn-1;
	if (an<=bn) RAISE("dimension mismatch");
	for (int i=yi+1; i<an; i++)
		if (r.dim->v[i-yi-1]!=in->dim->v[i]) RAISE("dimension mismatch");
	out[0]->begin(in->dim->dup());
	in->set_factor(in->dim->get(yi)*r.dim->prod());
} GRID_FLOW {
	int an = in->dim->n;
	int bn = r.dim->n;
	int yn = in->dim->v[an-bn-1];
	int zn = in->dim->prod(an-bn);
	int factor = in->factor;
	STACK_ARRAY(Number,buf,n);
	assert (n % factor == 0);
	int nn=n;
	while (n) {
		COPY(buf,data,n);
		op->on_int32.op_scan2(zn,(Number*)r,yn,(Number*)buf);
		data += yn*zn;
		n -= yn*zn;
	}
	out[0]->send(nn,buf);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridScan,1,r) {} GRID_END

METHOD3(GridScan,initialize) {
	rb_call_super(argc,argv);
	op = OP2(argv[0]);
	if (argc>2) RAISE("too many args");
	if (argc<2) {
		r.init(new Dim(0,0),int32_type_i);
		((Pt<int32>)r)[0] = 0;
	} else {
		r.init_from_ruby(argv[1]);
	}
	return Qnil;
}

GRCLASS(GridScan,"@scan",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridScan,0,4)),
	DECL(GridScan,initialize))

/* **************************************************************** */
/* inner: (op_para,op_fold,rint,A in dim(*As,A0), B in dim(B0,*Bs))
          -> c in dim(*As,*Bs)
   c = map((*As,*Bs),fold(op_fold,rint,map2(op_para,...... whatever
*/

/*{ Dim[*As,C],Dim[C,*Bs] -> Dim[*As,*Bs] }*/

struct GridInner : GridObject {
	const Operator2 *op_para;
	const Operator2 *op_fold;
	Number rint;
	Grid r;
	DECL3(initialize);
	GRINLET3(0);
	GRINLET3(2);
};

GRID_INLET(GridInner,0) {
	Dim *a = in->dim;
	Dim *b = r.dim;
	if (!b) RAISE("right inlet has no grid");
	if (a->n<1) RAISE("minimum 1 dimension");
	int a_last = a->get(a->n-1);
	int b_first = b->get(0);
	int n = a->n+b->n-2;
	if (a_last != b_first)
		RAISE("last dim of left side (%d) should be same size as "
			"first dim of right side (%d)",a_last,b_first);
	int32 v[n];
	COPY(v,a->v,a->n-1);
	COPY(v+a->n-1,b->v+1,b->n-1);
	out[0]->begin(new Dim(n,v));
	in->set_factor(a_last);
} GRID_FLOW {
	int factor = in->dim->get(in->dim->n-1);
	int b_prod = r.dim->prod();
	int chunks = b_prod/factor;
	STACK_ARRAY(Number,buf2,b_prod/factor);
	STACK_ARRAY(Number,buf,factor);
	STACK_ARRAY(Number,bufr,factor);
	assert (n % factor == 0);

	for (int i=0; i<n; i+=factor) {
		for (int j=0; j<chunks; j++) {
			COPY(buf,&data[i],factor);
			for (int k=0; k<factor; k++) bufr[k]=((Pt<int32>)r)[chunks*k+j];
			op_para->on_int32.op_map2(factor,(Number*)buf,(Number*)bufr);
			buf2[j] = op_fold->on_int32.op_fold(rint,factor,(Number*)buf);
		}
		out[0]->send(b_prod/factor,buf2);
	}
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridInner,2,r) {} GRID_END

METHOD3(GridInner,initialize) {
	rb_call_super(argc,argv);
	r.constrain(expect_min_one_dim);
	if (argc>4) RAISE("too many args");
	op_para = argc<1 ? OP2(SYM(*)) : OP2(argv[0]);
	op_fold = argc<2 ? OP2(SYM(+)) : OP2(argv[1]);
	rint = argc<3 ? 0 : INT(argv[2]);
	r.init(0);
	if (argc==4) r.init_from_ruby(argv[3]);
	return Qnil;
}

GRCLASS(GridInner,"@inner",inlets:3,outlets:1,startup:0,
LIST(GRINLET(GridInner,0,4),GRINLET(GridInner,2,4)),
	DECL(GridInner,initialize))

/* **************************************************************** */

/*{ Dim[*As,C],Dim[*Bs,C] -> Dim[*As,*Bs] }*/

struct GridInner2 : GridInner {
	DECL3(initialize);
	GRINLET3(0);
	GRINLET3(2);
};

GRID_INLET(GridInner2,0) {
	Dim *a = in->dim;
	Dim *b = r.dim;
	if (!b) RAISE("right inlet has no grid");
	if (a->n<1) RAISE("minimum 1 dimension");
	int a_last = a->get(a->n-1);
	int b_last = b->get(b->n-1);
	int n = a->n+b->n-2;
	if (a_last != b_last)
		RAISE("last dimension of each grid must have same size");
	int32 v[n];
	COPY(v,a->v,a->n-1);
	COPY(v+a->n-1,b->v,b->n-1);
	out[0]->begin(new Dim(n,v));
	in->set_factor(a_last);
} GRID_FLOW {
	int factor = in->dim->get(in->dim->n-1);
	int b_prod = r.dim->prod();
	STACK_ARRAY(Number,buf2,b_prod/factor);
	STACK_ARRAY(Number,buf,factor);
	assert (n % factor == 0);

	for (int i=0; i<n; i+=factor) {
		for (int j=0,k=0; j<b_prod; j+=factor,k++) {
			COPY(buf,&data[i],factor);
			op_para->on_int32.op_map2(factor,buf,(Number*)r+j);
			buf2[k] = op_fold->on_int32.op_fold(rint,factor,(Number*)buf);
		}
		out[0]->send(b_prod/factor,buf2);
	}
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridInner2,2,r) {} GRID_END

METHOD3(GridInner2,initialize) {
	rb_call_super(argc,argv);
	r.constrain(expect_min_one_dim);
	if (argc>4) RAISE("too many args");
	op_para = argc<1 ? OP2(SYM(*)) : OP2(argv[0]);
	op_fold = argc<2 ? OP2(SYM(+)) : OP2(argv[1]);
	rint = argc<3 ? 0 : INT(argv[2]);
	r.init(0);
	if (argc==4) r.init_from_ruby(argv[3]);
	return Qnil;
}

GRCLASS(GridInner2,"@inner2",inlets:3,outlets:1,startup:0,
LIST(GRINLET(GridInner2,0,4),GRINLET(GridInner2,2,4)),
	DECL(GridInner2,initialize))

/* **************************************************************** */

/*{ Dim[*As],Dim[*Bs] -> Dim[*As,*Bs] }*/

struct GridOuter : GridObject {
	Grid r;
	const Operator2 *op;
	DECL3(initialize);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridOuter,0) {
	Dim *a = in->dim;
	Dim *b = r.dim;
	if (!b) RAISE("right inlet has no grid");
	int n = a->n+b->n;
	int32 v[n];
	COPY(v,a->v,a->n);
	COPY(v+a->n,b->v,b->n);
	out[0]->begin(new Dim(n,v));
} GRID_FLOW {
	int b_prod = r.dim->prod();
	STACK_ARRAY(Number,buf,b_prod);
	while (n) {
		for (int j=0; j<b_prod; j++) buf[j] = *data;
		op->on_int32.op_map2(b_prod,buf,(Number*)r);
		out[0]->send(b_prod,buf);
		data++; n--;
	}
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridOuter,1,r) {} GRID_END

METHOD3(GridOuter,initialize) {
	rb_call_super(argc,argv);
	op = OP2(argv[0]);
	r.init(0);
	if (argc<1) RAISE("not enough args");
	if (argc>2) RAISE("too many args");
	if (argc==2) r.init_from_ruby(argv[1]);
	return Qnil;
}

GRCLASS(GridOuter,"@outer",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridOuter,0,4),GRINLET(GridOuter,1,4)),
	DECL(GridOuter,initialize))

/* **************************************************************** */
/* the incoming grid is stored as "c" with a margin on the four sides
   of it. the margin is the size of the "b" grid minus one then split in two.
*/   

/*{ Dim[A,B,*Cs],Dim[D,E] -> Dim[A,B,*Cs] }*/

struct GridConvolve : GridObject {
	Grid c,b;
	const Operator2 *op_para, *op_fold;
	Number rint;
	int margx,margy; /* margins */
	DECL3(initialize);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridConvolve,0) {
	Dim *da = in->dim, *db = b.dim;
	if (!db) RAISE("right inlet has no grid");
	if (db->n != 2) RAISE("right grid must have two dimensions");
	if (da->n < 2) RAISE("left grid has less than two dimensions");
	/* bug: da[0]>=db[0] and da[1]>=db[1] are also conditions */
	int32 v[da->n];
	COPY(v,da->v,da->n);
	margy = db->get(0)/2;
	margx = db->get(1)/2;
	v[0] += 2*margy;
	v[1] += 2*margx;
	c.init(new Dim(da->n,v));
	out[0]->begin(da->dup());
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
	Pt<Number> base = ((Pt<int32>)c)+(i+margy)*ll+margx*l;
	
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
	Pt<Number> cp = ((Pt<int32>)c);

	/* finishing building c from a */
	COPY(cp                      ,cp+da->get(0)*ll,margy*ll);
	COPY(cp+(margy+da->get(0))*ll,cp+margy*ll,     margy*ll);

	/* the real stuff */

	STACK_ARRAY(Number,buf3,l);
	STACK_ARRAY(Number,buf2,l*dbx*dby);
	STACK_ARRAY(Number,buf ,l*dbx*dby);
	Pt<Number> q=buf2;
	for (int i=0; i<dbx*dby; i++) for (int j=0; j<l; j++) *q++=((Pt<int32>)b)[i];

	for (int iy=0; iy<day; iy++) {
		for (int ix=0; ix<dax; ix++) {
			Pt<Number> p = ((Pt<int32>)c) + iy*ll + ix*l;
			Pt<Number> r = buf;
			for (int jy=dby; jy; jy--,p+=ll,r+=dbx*l) COPY(r,p,dbx*l);
			for (int i=l-1; i>=0; i--) buf3[i]=rint;
			op_para->on_int32.op_map2(l*dbx*dby,(Number*)buf,(Number*)buf2);
			op_fold->on_int32.op_fold2(l,(Number*)buf3,dbx*dby,(Number*)buf);
			out[0]->send(l,buf3);
		}
	}
	c.del();
} GRID_END

static void expect_convolution_matrix (Dim *d) {
	if (d->n != 2) RAISE("only exactly two dimensions allowed for now");
	/* because odd * odd = odd */
	if (d->prod()&1 == 0) RAISE("even number of elements");
}

GRID_INPUT(GridConvolve,1,b) {} GRID_END

METHOD3(GridConvolve,initialize) {
	rb_call_super(argc,argv);
	b.constrain(expect_convolution_matrix);
	if (argc>4) RAISE("too many args");
	op_para = OP2(argc<1 ? SYM(*) : argv[0]);
	op_fold = OP2(argc<2 ? SYM(+) : argv[1]);
	rint = argc<3 ? 0 : INT(argv[2]);
	if (argc==4) b.init_from_ruby(argv[3]);
	return Qnil;
}

GRCLASS(GridConvolve,"@convolve",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridConvolve,0,4),GRINLET(GridConvolve,1,4)),
	DECL(GridConvolve,initialize))

/* **************************************************************** */

struct GridFor : GridObject {
	Grid from;
	Grid to;
	Grid step;
	DECL3(initialize);
	DECL3(_0_set);
	DECL3(_0_bang);
	GRINLET3(0);
	GRINLET3(1);
	GRINLET3(2);
};

/*{ Dim[],Dim[],Dim[] -> Dim[A] }
  or
  { Dim[B],Dim[B],Dim[B] -> Dim[*As,B] }*/

METHOD3(GridFor,initialize) {
	from.constrain(expect_max_one_dim);
	to  .constrain(expect_max_one_dim);
	step.constrain(expect_max_one_dim);
	rb_call_super(argc,argv);
	if (argc<3) RAISE("not enough arguments");
	from.init_from_ruby(argv[0]);
	to  .init_from_ruby(argv[1]);
	step.init_from_ruby(argv[2]);
	return Qnil;
}

METHOD3(GridFor,_0_bang) {
	int n = from.dim->prod();
	int32 nn[n+1];
	STACK_ARRAY(Number,x,n);
	Pt<Number> fromb = ((Pt<int32>)from);
	Pt<Number>   tob = ((Pt<int32>)to  );
	Pt<Number> stepb = ((Pt<int32>)step);
	if (!from.dim->equal(to.dim) || !to.dim->equal(step.dim))
		RAISE("dimension mismatch");
	for (int i=step.dim->prod()-1; i>=0; i--)
		if (!stepb[i]) RAISE("step must not contain zeroes");
	for (int i=0; i<n; i++) {
		nn[i] = (tob[i] - fromb[i] + stepb[i] - cmp(stepb[i],0)) / stepb[i];
		if (nn[i]<0) nn[i]=0;
	}
	if (from.dim->n==0) {
		out[0]->begin(new Dim(1,nn));
	} else {
		nn[n]=n;
		out[0]->begin(new Dim(n+1,nn));
	}
	for(int d=0;;) {
		/* here d is the dim# to reset; d=n for none */
		for(;d<n;d++) x[d]=fromb[d];
		d--;
		out[0]->send(n,x);
		/* here d is the dim# to increment */
		for(;;) {
			if (d<0) goto end;
			x[d]+=stepb[d];
			if (x[d]<tob[d]) break;
			d--;
		}
		d++;
	}
	end:
	return Qnil;
}

METHOD3(GridFor,_0_set) {
	from.init_from_ruby(argv[0]);
	return Qnil;
}

GRID_INPUT(GridFor,2,step) {} GRID_END
GRID_INPUT(GridFor,1,to) {} GRID_END
GRID_INPUT(GridFor,0,from) {_0_bang(0,0);} GRID_END

GRCLASS(GridFor,"@for",inlets:3,outlets:1,startup:0,
LIST(GRINLET(GridFor,0,4),GRINLET(GridFor,1,4),GRINLET(GridFor,2,4)),
	DECL(GridFor,initialize),
	DECL(GridFor,_0_bang),
	DECL(GridFor,_0_set))

/* **************************************************************** */

/*{ Dim[*As] -> Dim[B] }*/

struct GridDim : GridObject {
	GRINLET3(0);
};

GRID_INLET(GridDim,0) {
	int32 v[1] = {in->dim->n};
	out[0]->begin(new Dim(1,v));
	out[0]->send(v[0],Pt<Number>((Number *)in->dim->v,in->dim->n));
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

GRCLASS(GridDim,"@dim",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridDim,0,0)))

/* **************************************************************** */

/*{ Dim[*As],Dim[B] -> Dim[*Cs] }*/

struct GridRedim : GridObject {
	Grid dim_grid;
	Dim *dim;
	Grid temp; /* temp.dim is not of the same shape as dim */

	~GridRedim() {
		if (dim) delete dim;
	}
	DECL3(initialize);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_INLET(GridRedim,0) {
	int a = in->dim->prod(), b = dim->prod();
	if (a<b) {int32 v[1]={a}; temp.init(new Dim(1,v));}
	out[0]->begin(dim->dup());
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
		COPY(&((Pt<int32>)temp)[i],data,n2);
		if (n2>0) out[0]->send(n2,data);
	}
} GRID_FINISH {
	if (!temp.is_empty()) {
		int a = in->dim->prod(), b = dim->prod();
		int i = a;
		while (i<b) {
			out[0]->send(min(a,b-i),(Pt<int32>)temp);
			i += a;
		}
	}
	temp.del();
} GRID_END

GRID_INPUT(GridRedim,1,dim_grid) { dim = dim_grid.to_dim(); } GRID_END

METHOD3(GridRedim,initialize) {
	rb_call_super(argc,argv);
	if (argc!=1) RAISE("wrong number of args");
	Grid t;
	t.init_from_ruby(argv[0]);
	expect_dim_dim_list(t.dim);
	dim = new Dim(t.dim->prod(),(Number *)(Pt<int32>)t);
	return Qnil;
}

GRCLASS(GridRedim,"@redim",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridRedim,0,4),GRINLET(GridRedim,1,4)),
	DECL(GridRedim,initialize))

/* ---------------------------------------------------------------- */
/* "@scale_by" does quick scaling of pictures by integer factors */

/*{ Dim[A,B,3] -> Dim[C,D,3] }*/

struct GridScaleBy : GridObject {
	Grid scale; /* integer scale factor */
	int scaley;
	int scalex;
	DECL3(initialize);
	GRINLET3(0);
};

/* processing a grid coming from inlet 0 */
GRID_INLET(GridScaleBy,0) {
/* this section processes the header and accepts or rejects the grid */
	Dim *a = in->dim;

	/* there are restrictions on grid sizes for efficiency reasons */
	expect_rgb_picture(a);

	/* computing the output's size */
	int32 v[3]={ a->get(0)*scaley, a->get(1)*scalex, a->get(2) };
	out[0]->begin(new Dim(3,v));

	/* configuring the input format */
	in->set_factor(a->get(1)*a->get(2));
} GRID_FLOW {
/* this section processes one packet of grid content */
	int rowsize = in->dim->prod(1);
	STACK_ARRAY(Number,buf,rowsize*scalex);

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

/* the constructor accepts a scale factor as an argument */
/* that argument is not modifiable through an inlet yet (that would be the right inlet) */
METHOD3(GridScaleBy,initialize) {
	scale.constrain(expect_scale_factor);
	scale.init_from_ruby(argc<1 ? EVAL("[2]") : argv[0]);
	scaley = ((Pt<int32>)scale)[0];
	scalex = ((Pt<int32>)scale)[scale.dim->prod()==1 ? 0 : 1];
	if (scaley<1) scaley=1;
	if (scalex<1) scalex=1;

	rb_call_super(argc,argv);
	return Qnil;
}

/* there's one inlet, one outlet, and two system methods (inlet #-1) */
GRCLASS(GridScaleBy,"@scale_by",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridScaleBy,0,4)),
	DECL(GridScaleBy,initialize))

/* ---------------------------------------------------------------- */
/* "@downscale_by" does quick downscaling of pictures by integer factors */

/*{ Dim[A,B,3] -> Dim[C,D,3] }*/

struct GridDownscaleBy : GridObject {
	Grid scale;
	int scaley;
	int scalex;
	bool smoothly;
	Grid temp;
	DECL3(initialize);
	GRINLET3(0);
};

GRID_INLET(GridDownscaleBy,0) {
	Dim *a = in->dim;
	if (a->n!=3) RAISE("(height,width,chans) please");
	if (a->get(2)!=3) RAISE("3 chans please");
	int32 v[3]={ a->get(0)/scaley, a->get(1)/scalex, a->get(2) };
	out[0]->begin(new Dim(3,v));
	in->set_factor(a->get(1)*a->get(2));
	int32 w[]={in->dim->get(1)/scalex,in->dim->get(2)};
	temp.init(new Dim(2,w));
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	int rowsize2 = temp.dim->prod();
	Pt<Number> buf = ((Pt<int32>)temp);

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
				OP2(SYM(/))->on_int32.op_map(rowsize2,(Number*)buf,scalex*scaley);
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

METHOD3(GridDownscaleBy,initialize) {
	scale.constrain(expect_scale_factor);
	scale.init_from_ruby(argc<1 ? EVAL("[2]") : argv[0]);
	scaley = ((Pt<int32>)scale)[0];
	scalex = ((Pt<int32>)scale)[scale.dim->prod()==1 ? 0 : 1];
	if (scaley<1) scaley=1;
	if (scalex<1) scalex=1;

	smoothly = (argc>1 && argv[1]==SYM(smoothly));
	rb_call_super(argc,argv);
	return Qnil;
}

GRCLASS(GridDownscaleBy,"@downscale_by",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridDownscaleBy,0,4)),
	DECL(GridDownscaleBy,initialize))

/* **************************************************************** */

struct GridJoin : GridObject {
	Grid r;
	int which_dim;
	GRINLET3(0);
	GRINLET3(1);
	DECL3(initialize);
};

GRID_INLET(GridJoin,0) {
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridJoin,1,r) {}
GRID_END

METHOD3(GridJoin,initialize) {
	which_dim = argc<1 ? -1 : INT(argv[0]);
	r.init_from_ruby(argv[1]);
	return Qnil;
}

GRCLASS(GridJoin,"@join",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridJoin,0,4),GRINLET(GridJoin,1,4)),
	DECL(GridJoin,initialize))

/* **************************************************************** */

struct GridLayer : GridObject {
	Grid r;
	GridLayer() {
		r.constrain(expect_rgb_picture);
	}
	GRINLET3(0);
	GRINLET3(1);
	DECL3(initialize);
};

GRID_INLET(GridLayer,0) {
	Dim *a = in->dim;
	expect_rgba_picture(a);
	if (a->get(1)!=r.dim->get(1)) RAISE("same width please");
	if (a->get(0)!=r.dim->get(0)) RAISE("same height please");
	in->set_factor(a->prod(1));
	out[0]->begin(r.dim->dup());
} GRID_FLOW {
	Pt<Number> rr = ((Pt<int32>)r) + in->dex*3/4;
	STACK_ARRAY(Number,foo,n*3/4);
	for (int i=0,j=0; i<n; i+=4,j+=3) {
		foo[j+0] = (data[i+0]*data[i+3] + rr[j+0]*(256-data[i+3])) >> 8;
		foo[j+1] = (data[i+1]*data[i+3] + rr[j+1]*(256-data[i+3])) >> 8;
		foo[j+2] = (data[i+2]*data[i+3] + rr[j+2]*(256-data[i+3])) >> 8;
	}
	out[0]->send(n*3/4,foo);
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridLayer,1,r) {} GRID_END

METHOD3(GridLayer,initialize) {
	rb_call_super(argc,argv);
	return Qnil;
}

GRCLASS(GridLayer,"@layer",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridLayer,0,4),GRINLET(GridLayer,1,4)),
	DECL(GridLayer,initialize))

/* **************************************************************** */

struct GridFinished : GridObject {
	GRINLET3(0);
	DECL3(initialize);
};

GRID_INLET(GridFinished,0) {
} GRID_FLOW {
} GRID_FINISH {
	Ruby a[] = { INT2NUM(0), sym_bang };
	FObject_send_out(COUNT(a),a,rself);
} GRID_END

METHOD3(GridFinished,initialize) {
	rb_call_super(argc,argv);
	return Qnil;
}

GRCLASS(GridFinished,"@finished",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridFinished,0,0)),
	DECL(GridFinished,initialize))

/* **************************************************************** */
/*
<matju> @polygon
<matju> inlet 0: image sur laquelle superposer un polygone
<matju> inlet 1: couleur
<matju> inlet 2: coords de polygone, dim(n,2)
<matju> outlet 0: polygone uni-sur-transparent par dessus l'image, sans antialias, recoupements en odd-even rule

<matju> mais je pourrais faire @perspective
<matju> inlet 0: dim(n,3)
<matju> inlet 1: facteur k (l'oeil est au point 0,0,0; l'écran est le plan z=k)
<matju> outlet 0: dim(n,2)
*/

/* { Dim[A,B,C],Dim[C],Dim[N,2] -> Dim[A,B,C] } */

struct GridPolygon : GridObject {
	Grid color;
	Grid polygon;
	Grid lines;
	int lines_start;
	int lines_stop;
	Pt<int32> lines_current;
	int lines_current_n;
	DECL3(initialize);
	GRINLET3(0);
	GRINLET3(1);
	GRINLET3(2);

	void init_lines ();
};

template <class T> static void memswap (T *a, T *b, int n) {
	T c[n];
	COPY(c,a,n);
	COPY(a,b,n);
	COPY(b,c,n);
}

void GridPolygon::init_lines () {
	int nl = polygon.dim->get(0);
	int32 v[] = {nl,4};
	lines.init(new Dim(2,v));
	Pt<Number> ld = (Pt<int32>)lines;
	Pt<Number> pd = (Pt<int32>)polygon;
	for (int i=0,j=0; i<4*nl; i+=4) {
		ld[i+0] = pd[j+0];
		ld[i+1] = pd[j+1];
		j=(j+2)%(2*nl);
		ld[i+2] = pd[j+0];
		ld[i+3] = pd[j+1];
		if (ld[i+0]>ld[i+2]) memswap((Number *)ld+i+0,(Number *)ld+i+2,2);
	}
}

static int order_by_starting_scanline (const void *a, const void *b) {
	return ((Number *)a)[0] - ((Number *)b)[0];
}

static int order_by_starting_column (const void *a, const void *b) {
	return ((Number *)a)[1] - ((Number *)b)[1];
}

GRID_INLET(GridPolygon,0) {
	if (polygon.is_empty()) RAISE("no polygon?");
	if (lines.is_empty()) RAISE("no lines???");
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	if (in->dim->get(2)!=color.dim->get(0))
		RAISE("image does not have same	number of channels as stored color");
	out[0]->begin(in->dim->dup());
	lines_start = lines_stop = 0;
	in->set_factor(in->dim->get(1)*in->dim->get(2));
	int nl = polygon.dim->get(0);
	qsort((Number *)lines,nl,4*sizeof(Number),order_by_starting_scanline);
} GRID_FLOW {
	int nl = polygon.dim->get(0);
	Pt<Number> ld = (Pt<Number>)lines;

	for (int i=0; i<4*nl; i+=4) WATCH(4,(ld+i));

	gfpost("n=%d",n);
	int y = in->dex / in->factor;
	while (n) {
		if (lines_stop != nl && ld[4*lines_stop]<=y) lines_stop++;
		for (int i=lines_start; i<lines_stop; i++) {
			if (ld[4*i+2]<=y) {
				memswap((Number *)ld+4*i,(Number *)ld+4*lines_start,4);
				lines_start++;
			}
		}
		gfpost("dex=%6d y=%3d: start=%d stop=%d",in->dex,y,lines_start,lines_stop);
		Pt<Number> cd = (Pt<Number>)color;
		int cn = color.dim->prod();
		if (lines_start == lines_stop) {
			out[0]->send(in->factor,data);
		} else {
			Pt<Number> data2 = ARRAY_NEW(Number,in->factor);
			COPY(data2,data,in->factor);
			qsort(ld+4*lines_start,lines_stop-lines_start,
				4*sizeof(Number),order_by_starting_column);
			int xb=0;
			for (int i=lines_start; i<lines_stop; i++) {
				int y1=ld[4*i+0], y2=ld[4*i+2];
				int x1=ld[4*i+1], x2=ld[4*i+3];
				int x = x1 + (y-y1)*(x2-x1)/(y2-y1);
				if ((i-lines_start)&1) {
					gfpost("odd: xb=%d x=%d",xb,x);
					while (xb<=x) COPY(data2+cn*xb++,cd,cn);
				} else {
					gfpost("even");
					xb=x;
				}
			}
			out[0]->give(in->factor,data2);
		}
		n-=in->factor;
		data+=in->factor;
		y++;
	}
} GRID_FINISH {
} GRID_END

GRID_INPUT(GridPolygon,1,color) {} GRID_END
GRID_INPUT(GridPolygon,2,polygon) {init_lines();} GRID_END

static void expect_polygon (Dim *d) {
	if (d->n!=2 || d->get(1)!=2) RAISE("expecting Dim[n,2] polygon");
}

METHOD3(GridPolygon,initialize) {
	color.constrain(expect_max_one_dim);
	polygon.constrain(expect_polygon);
	rb_call_super(argc,argv);
	if (argc>=1) color.init_from_ruby(argv[0]);
	if (argc>=2) polygon.init_from_ruby(argv[1]), init_lines();
	return Qnil;
}

GRCLASS(GridPolygon,"@polygon",inlets:3,outlets:1,startup:0,
LIST(GRINLET(GridPolygon,0,4),GRINLET(GridPolygon,1,4),GRINLET(GridPolygon,2,4)),
	DECL(GridPolygon,initialize))

/* **************************************************************** */

/*{ Dim[*As,3] -> Dim[*As,3] }*/

struct GridRGBtoHSV : GridObject {
	DECL3(initialize);
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
	STACK_ARRAY(Number,buf,n);
	Pt<Number> p=buf;
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

METHOD3(GridRGBtoHSV,initialize) {
	rb_call_super(argc,argv);
	return Qnil;
}

GRCLASS(GridRGBtoHSV,"@rgb_to_hsv",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridRGBtoHSV,0,4)),
	DECL(GridRGBtoHSV,initialize))

/* **************************************************************** */

/*{ Dim[*As,3] -> Dim[*As,3] }*/

struct GridHSVtoRGB : GridObject {
	DECL3(initialize);
	GRINLET3(0);
};

GRID_INLET(GridHSVtoRGB,0) {
	if (in->dim->n<1) RAISE("at least 1 dim please");
	if (in->dim->get(in->dim->n-1)!=3) RAISE("3 chans please");
	out[0]->begin(in->dim->dup());
	in->set_factor(3);
} GRID_FLOW {
	STACK_ARRAY(Number,buf,n);
	Pt<Number> p = buf;
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

METHOD3(GridHSVtoRGB,initialize) {
	rb_call_super(argc,argv);
	return Qnil;
}

GRCLASS(GridHSVtoRGB,"@hsv_to_rgb",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridHSVtoRGB,0,4)),
	DECL(GridHSVtoRGB,initialize))

/* **************************************************************** */
/* [rtmetro] */

struct RtMetro : GridObject {
	int ms; /* millisecond time interval */
	bool on;
	uint64 next_time; /* microseconds since epoch: next time an event occurs */
	uint64 last;      /* microseconds since epoch: last time we checked */
	int mode; /* 0=equal; 1=geiger */

	uint64 delay();

	DECL3(initialize);
	DECL3(_0_int);
	DECL3(_1_int);
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
		Ruby a[] = { INT2NUM(0), sym_bang };
		FObject_send_out(COUNT(a),a,rself);
		/* self->next_time = now; */ /* jmax style, less realtime */
		self->next_time += self->delay();
	}
	self->last = now;
}

METHOD3(RtMetro,_0_int) {
	int oon = on;
	on = !! FIX2INT(argv[0]);
	gfpost("on = %d",on);
	if (oon && !on) {
		gfpost("deleting rtmetro alarm for self=%08x rself=%08x",
			(long)this,(long)rself);
		MainLoop_remove((void *)rself);
	} else if (!oon && on) {
		gfpost("creating rtmetro alarm for self=%08x rself=%08x",
			(long)this,(long)rself);
		MainLoop_add((void *)rself,(void(*)(void*))RtMetro_alarm);
		next_time = RtMetro_now();
	}
	return Qnil;
}

METHOD3(RtMetro,_1_int) {
	ms = FIX2INT(argv[0]);
	gfpost("ms = %d",ms);
	return Qnil;
}

METHOD3(RtMetro,initialize) {
	rb_call_super(argc,argv);
	ms = FIX2INT(argv[0]);
	on = 0;
	mode = 0;
	if (argc>=2) {
		if (argv[1]==SYM(equal)) mode=0;
		else if (argv[1]==SYM(geiger)) mode=1;
		else RAISE("this is not a known mode");
	}		
	gfpost("ms = %d",ms);
	gfpost("on = %d",on);
	gfpost("mode = %d",mode);
	return Qnil;
}

GRCLASS(RtMetro,"rtmetro",inlets:2,outlets:1,startup:0,
LIST(),
	DECL(RtMetro,_0_int),
	DECL(RtMetro,_1_int),
	DECL(RtMetro,initialize))

/* **************************************************************** */

void startup_flow_objects () {
	INSTALL(GridImport);
	INSTALL(GridExport);
	INSTALL(GridExportList);
	INSTALL(GridStore);
	INSTALL(GridOp1);
	INSTALL(GridOp2);
	INSTALL(GridFold);
	INSTALL(GridScan);
	INSTALL(GridInner);
	INSTALL(GridInner2);
	INSTALL(GridOuter);
	INSTALL(GridConvolve);
	INSTALL(GridFor);
	INSTALL(GridDim);
	INSTALL(GridRedim);
	INSTALL(GridScaleBy);
	INSTALL(GridDownscaleBy);
	INSTALL(GridLayer);
	INSTALL(GridFinished);
//	INSTALL(GridJoin);
	INSTALL(GridPolygon);
//	INSTALL(GridRGBtoHSV); /* buggy */
//	INSTALL(GridHSVtoRGB); /* buggy */
	INSTALL(RtMetro);
}
