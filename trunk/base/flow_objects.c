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

/* **************************************************************** */

static NumberTypeIndex NumberType_find (Ruby sym) {
	if (TYPE(sym)!=T_SYMBOL) RAISE("expected symbol");
	if (sym==SYM(int32)) return int32_type_i;
	if (sym==SYM(uint8)) return uint8_type_i;
	RAISE("unknown element type \"%s\"", rb_sym_name(sym));
}

#define WATCH(n,ar) { \
	char foo[16*1024], *p=foo; \
	p += sprintf(p,"%s: ",#ar); \
	for (int i=0; i<n; i++) p += sprintf(p,"%ld ",ar[i]); \
	gfpost("%s",foo); \
}

#define rassert(_p_) if (!(_p_)) RAISE(#_p_);

#define IV(s) rb_ivar_get($->peer,SI(s))
#define IVS(s,v) rb_ivar_set($->peer,SI(s),v)

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

/* there's an inlet 0 hardcoded here, sorry */
#define DIM_INPUT(_class_,_inlet_,_member_) \
	GRID_BEGIN(_class_,1) { \
		expect_dim_dim_list(in->dim); \
		if (in->dim->n) in->set_factor(in->dim->n); \
	} \
	GRID_FLOW(_class_,1) { \
		if (dim) delete _member_, _member_=0; \
		_member_ = new Dim(n,(int *)(Number *)data); \
		this->in[0]->abort(); \
		out[0]->abort(); \
	} \
	GRID_END(_class_,1) {}

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
	DECL3(init);
	DECL3(_0_reset);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_BEGIN(GridImport,0) {}
GRID_FLOW(GridImport,0) {
	GridOutlet *o = out[0];
	while (n) {
		if (!o->is_busy()) o->begin(dim->dup());
		int n2 = o->dim->prod() - o->dex;
		if (n2 > n) n2 = n;
		o->send(n,data);
		if (o->dex >= o->dim->prod()) o->end();
		n -= n2;
		data += n2;
	}
}
GRID_END(GridImport,0) {}

GRID_INPUT(GridImport,1,dim_grid) { dim = dim_grid.to_dim(); }

METHOD3(GridImport,init) {
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
	DECL(GridImport,init),
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

GRID_BEGIN(GridExport,0) {}

GRID_FLOW(GridExport,0) {
	for (int i=0; i<n; i++) {
		Ruby a[] = { INT2NUM(0), sym_int, INT2NUM(data[i]) };
		FObject_send_out(COUNT(a),a,peer);
	}
}

GRID_END(GridExport,0) {}

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

GRID_BEGIN(GridExportList,0) {
	int n = in->dim->prod();
// 	fprintf(stderr,"flow_objects: this=%08x dim=%s\n",(long)in, in->dim->to_s());
//	if (n==1) abort();
	if (n>250000) RAISE("list too big (%d elements)", n);
	list = rb_ary_new2(n+2);
	this->n = n;
	rb_ivar_set(peer,SI(@list),list); /* keep */
	rb_ary_store(list,0,INT2NUM(0));
	rb_ary_store(list,1,sym_list);
}

GRID_FLOW(GridExportList,0) {
	for (int i=0; i<n; i++, data++)
		rb_ary_store(list,in->dex+i+2,INT2NUM(*data));
}
		
GRID_END(GridExportList,0) {
	FObject_send_out(rb_ary_len(list),rb_ary_ptr(list),peer);
	list = 0;
	rb_ivar_set(peer,SI(@list),Qnil); /* unkeep */
}

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
	DECL3(init);
	DECL3(_0_bang);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_BEGIN(GridStore,0) {
	if (r.is_empty()) RAISE("empty buffer, better luck next time.");

	int na = in->dim->n;
	int nb = r.dim->n;
	int nc = in->dim->get(na-1);
	int v[MAX_DIMENSIONS];

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

GRID_FLOW(GridStore,0) {
//	static Operator2 *op_mod = 0;
//	if (!op_mod) op_mod = OP2(rb_str_new2("%"));
	int na = in->dim->n;
	int nb = r.dim->n;
	int nc = in->dim->get(na-1);
	int size = r.dim->prod(nc);
	int v[nb];
	assert((n % nc) == 0);

	for (int i=nc; i<nb; i++) v[i] = 0;

	//!@#$ accelerate me.
	if (r.nt==int32_type_i) {
		while (n>0) {
			for (int i=0; i<nc; i++,data++) v[i] = mod(*data,r.dim->v[i]);
			out[0]->send(size,r.as_int32()+r.dim->calc_dex(v));
			n -= nc;
		}
	} else if (r.nt==uint8_type_i) {
		while (n>0) {
			for (int i=0; i<nc; i++,data++) v[i] = mod(*data,r.dim->v[i]);
			out[0]->send(size,r.as_uint8()+r.dim->calc_dex(v));
			n -= nc;
		}
	} else RAISE("unsupported type");
}

GRID_END(GridStore,0) {
	GridOutlet *o = out[0];
	if (in->dim->prod()==0) {
		int n = in->dim->prod(0,-2);
		int size = r.dim->prod();
		if (r.nt==int32_type_i) {
			while (n--) o->send(size,r.as_int32());
		} else if (r.nt==uint8_type_i) {
			while (n--) o->send(size,r.as_uint8());
		} else RAISE("unsupported type");
	}
	o->end();
}

GRID_BEGIN(GridStore,1) {
	this->in[0]->abort();
	r.del();
	r.init(in->dim->dup(),r.nt);
}

GRID_FLOW(GridStore,1) {
	if (r.nt==int32_type_i) {
		COPY(r.as_int32() + in->dex, data, n);
	} else if (r.nt==uint8_type_i) {
		uint8 *data2 = r.as_uint8() + in->dex;
		for(int i=0; i<n; i++) data2[i] = data[i];
	} else RAISE("unsupported type");
	in->dex += n;
}

GRID_END(GridStore,1) {}

METHOD3(GridStore,init) {
	rb_call_super(argc,argv);
	r.init(0, NumberType_find(argc==0 ? SYM(int32) : argv[0]));
	return Qnil;
}

METHOD3(GridStore,_0_bang) {
	rb_funcall(peer,SI(_0_list),3,INT2NUM(0),SYM(#),INT2NUM(0));
	return Qnil;
}

GRCLASS(GridStore,"@store",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridStore,0,4),GRINLET(GridStore,1,4)),
	DECL(GridStore,init),
	DECL(GridStore,_0_bang))

/* **************************************************************** */

/*{ Dim[*As] -> Dim[*As] }*/

struct GridOp1 : GridObject {
	const Operator1 *op;
	DECL3(init);
	GRINLET3(0);
};

GRID_BEGIN(GridOp1,0) { out[0]->begin(in->dim->dup()); }

GRID_FLOW(GridOp1,0) {
	op->on_int32.op_array(n,data);
	out[0]->give(n,data);
}

GRID_END(GridOp1,0) { out[0]->end(); }

METHOD3(GridOp1,init) {
	rb_call_super(argc,argv);
	op = OP1(argv[0]);
	return Qnil;
}

GRCLASS(GridOp1,"@!",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridOp1,0,6)),
	DECL(GridOp1,init))

/* **************************************************************** */
/*
  GridOp2 ("@") is the class of objects for parallel operation on the
  Rubys of the left grid with Rubys of a (stored) right grid or (stored)
  single Ruby.
*/

struct GridOp2 : GridObject {
	const Operator2 *op;
	Grid r;
	DECL3(init);
	GRINLET3(0);
	GRINLET3(1);
};

/*{ Dim[*As],Dim[*Bs] -> Dim[*As] }*/

GRID_BEGIN(GridOp2,0) { out[0]->begin(in->dim->dup()); }

GRID_FLOW(GridOp2,0) {
	if (r.is_empty()) RAISE("ARGH");
	Pt<int32> rdata = r.as_int32();
	int loop = r.dim->prod();
	if (loop>1) {
		if (in->dex+n <= loop) {
			op->on_int32.op_array2(n,data,rdata+in->dex);
		} else {
			STACK_ARRAY(Number,data2,n);
			//!@#$ accelerate me
			for (int i=0; i<n;) {
				int ii = (in->dex+i)%loop;
				int m = min(loop-ii,n-i);
				COPY(data2+i,rdata+ii,m);
				i+=m;
			}
			op->on_int32.op_array2(n,data,data2);
		}
	} else {
		op->on_int32.op_array(n,data,*rdata);
	}
	out[0]->give(n,data);
}

GRID_END(GridOp2,0) { out[0]->end(); }

GRID_INPUT(GridOp2,1,r) {}

METHOD3(GridOp2,init) {
	rb_call_super(argc,argv);
	op = OP2(argv[0]);
	if (argc>2) RAISE("too many args");
	if (argc<2) {
		r.init(new Dim(0,0),int32_type_i);
		r.as_int32()[0] = 0;
	} else {
		r.init_from_ruby(argv[1]);
	}
	return Qnil;
}

GRCLASS(GridOp2,"@",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridOp2,0,6),GRINLET(GridOp2,1,4)),
	DECL(GridOp2,init))

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
	DECL3(init);
	GRINLET3(0);
	GRINLET3(1);
};

/* fold: dim(*X,Y,*Z) x dim(*Z) -> dim(*X,*Z) */
GRID_BEGIN(GridFold,0) {
	int an = in->dim->n;
	int bn = r.dim->n;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand");
	int v[an-1];
	int yi = an-bn-1;
	COPY(v,in->dim->v,yi);
	COPY(v+yi,in->dim->v+yi+1,an-yi-1);
	for (int i=yi+1; i<an; i++)
		if (r.dim->v[i-yi-1]!=in->dim->v[i]) RAISE("dimension mismatch");
	out[0]->begin(new Dim(COUNT(v),v));
	in->set_factor(in->dim->get(yi)*r.dim->prod());
}

GRID_FLOW(GridFold,0) {
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
		COPY(buf+i,r.as_int32(),zn);
		op->on_int32.op_fold2(zn,buf+i,yn,data);
		i += zn;
		data += yn*zn;
		n -= yn*zn;
	}
	out[0]->send(nn/yn,buf);
}

GRID_END(GridFold,0) { out[0]->end(); }

GRID_INPUT(GridFold,1,r) {}

METHOD3(GridFold,init) {
	rb_call_super(argc,argv);
	op = OP2(argv[0]);
	if (argc>2) RAISE("too many args");
	if (argc<2) {
		r.init(new Dim(0,0),int32_type_i);
		r.as_int32()[0] = 0;
	} else {
		r.init_from_ruby(argv[1]);
	}
	return Qnil;
}

GRCLASS(GridFold,"@fold",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridFold,0,4)),
	DECL(GridFold,init))

/* **************************************************************** */
/*
  GridScan ("@scan") is similar to @fold except that it gives back all
  the partial results thereof; therefore the output is of the same
  size as the input (unlike @fold).
*/

/*{ Dim[*As,*Bs],Dim[*Bs] -> Dim[*As,*Bs] }*/

struct GridScan : GridFold {
	DECL3(init);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_BEGIN(GridScan,0) {
	int an = in->dim->n;
	int bn = r.dim->n;
	int yi = an-bn-1;
	if (an<=bn) RAISE("dimension mismatch");
	for (int i=yi+1; i<an; i++)
		if (r.dim->v[i-yi-1]!=in->dim->v[i]) RAISE("dimension mismatch");
	out[0]->begin(in->dim->dup());
	in->set_factor(in->dim->get(yi)*r.dim->prod());
}

GRID_FLOW(GridScan,0) {
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
		op->on_int32.op_scan2(zn,r.as_int32(),yn,buf);
		data += yn*zn;
		n -= yn*zn;
	}
	out[0]->send(nn,buf);
}

GRID_END(GridScan,0) { out[0]->end(); }

GRID_INPUT(GridScan,1,r) {}

METHOD3(GridScan,init) {
	rb_call_super(argc,argv);
	op = OP2(argv[0]);
	if (argc>2) RAISE("too many args");
	if (argc<2) {
		r.init(new Dim(0,0),int32_type_i);
		r.as_int32()[0] = 0;
	} else {
		r.init_from_ruby(argv[1]);
	}
	return Qnil;
}

GRCLASS(GridScan,"@scan",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridScan,0,4)),
	DECL(GridScan,init))

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
	DECL3(init);
	GRINLET3(0);
	GRINLET3(2);
};

GRID_BEGIN(GridInner,0) {
	Dim *a = in->dim;
	Dim *b = r.dim;
	if (!b) RAISE("right inlet has no grid");
	if (a->n<1) RAISE("minimum 1 dimension");
	int a_last = a->get(a->n-1);
	int b_first = b->get(0);
	int n = a->n+b->n-2;
	if (a_last != b_first)
		RAISE("last dim of left side should be same size as first dim of right side");
	int v[n];
	COPY(v,a->v,a->n-1);
	COPY(v+a->n-1,b->v+1,b->n-1);
	out[0]->begin(new Dim(n,v));
	in->set_factor(a_last);
}

GRID_FLOW(GridInner,0) {
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
			for (int k=0; k<factor; k++) bufr[k]=r.as_int32()[chunks*k+j];
			op_para->on_int32.op_array2(factor,buf,bufr);
			buf2[j] = op_fold->on_int32.op_fold(rint,factor,buf);
		}
		out[0]->send(b_prod/factor,buf2);
	}
}

GRID_END(GridInner,0) { out[0]->end(); }

GRID_BEGIN(GridInner,2) {
	this->in[0]->abort();
	if (in->dim->n<1) RAISE("minimum 1 dimension");
	r.del();
	r.init(in->dim->dup());
}

GRID_FLOW(GridInner,2) { COPY(&r.as_int32()[in->dex], data, n); }

GRID_END(GridInner,2) {}

METHOD3(GridInner,init) {
	rb_call_super(argc,argv);
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
	DECL(GridInner,init))

/* **************************************************************** */

/*{ Dim[*As,C],Dim[*Bs,C] -> Dim[*As,*Bs] }*/

struct GridInner2 : GridInner {
	DECL3(init);
	GRINLET3(0);
	GRINLET3(2);
};

GRID_BEGIN(GridInner2,0) {
	Dim *a = in->dim;
	Dim *b = r.dim;
	if (!b) RAISE("right inlet has no grid");
	if (a->n<1) RAISE("minimum 1 dimension");
	int a_last = a->get(a->n-1);
	int b_last = b->get(b->n-1);
	int n = a->n+b->n-2;
	if (a_last != b_last)
		RAISE("last dimension of each grid must have same size");
	int v[n];
	COPY(v,a->v,a->n-1);
	COPY(v+a->n-1,b->v,b->n-1);
	out[0]->begin(new Dim(n,v));
	in->set_factor(a_last);
}

GRID_FLOW(GridInner2,0) {
	int factor = in->dim->get(in->dim->n-1);
	int b_prod = r.dim->prod();
	STACK_ARRAY(Number,buf2,b_prod/factor);
	STACK_ARRAY(Number,buf,factor);
	assert (n % factor == 0);

	for (int i=0; i<n; i+=factor) {
		for (int j=0,k=0; j<b_prod; j+=factor,k++) {
			COPY(buf,&data[i],factor);
			op_para->on_int32.op_array2(factor,buf,r.as_int32()+j);
			buf2[k] = op_fold->on_int32.op_fold(rint,factor,buf);
		}
		out[0]->send(b_prod/factor,buf2);
	}
}

GRID_END(GridInner2,0) { out[0]->end(); }

GRID_BEGIN(GridInner2,2) {
	this->in[0]->abort();
	if (in->dim->n<1) RAISE("minimum 1 dimension");
	r.del();
	r.init(in->dim->dup());
}

GRID_FLOW(GridInner2,2) { COPY(&r.as_int32()[in->dex], data, n); }

GRID_END(GridInner2,2) {}

METHOD3(GridInner2,init) {
	rb_call_super(argc,argv);
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
	DECL(GridInner2,init))

/* **************************************************************** */

/*{ Dim[*As],Dim[*Bs] -> Dim[*As,*Bs] }*/

struct GridOuter : GridObject {
	Grid r;
	const Operator2 *op;
	DECL3(init);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_BEGIN(GridOuter,0) {
	Dim *a = in->dim;
	Dim *b = r.dim;
	if (!b) RAISE("right inlet has no grid");
	int n = a->n+b->n;
	int v[n];
	COPY(v,a->v,a->n);
	COPY(v+a->n,b->v,b->n);
	out[0]->begin(new Dim(n,v));
}

GRID_FLOW(GridOuter,0) {
	int b_prod = r.dim->prod();
	STACK_ARRAY(Number,buf,b_prod);
	while (n) {
		for (int j=0; j<b_prod; j++) buf[j] = *data;
		op->on_int32.op_array2(b_prod,buf,r.as_int32());
		out[0]->send(b_prod,buf);
		data++; n--;
	}
}

GRID_END(GridOuter,0) { out[0]->end(); }

GRID_BEGIN(GridOuter,1) {
	this->in[0]->abort();
	r.del();
	r.init(in->dim->dup());
}

GRID_FLOW(GridOuter,1) { COPY(&r.as_int32()[in->dex], data, n); }

GRID_END(GridOuter,1) {}

METHOD3(GridOuter,init) {
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
	DECL(GridOuter,init))

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
	DECL3(init);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_BEGIN(GridConvolve,0) {
	Dim *da = in->dim, *db = b.dim;
	if (!db) RAISE("right inlet has no grid");
	if (db->n != 2) RAISE("right grid must have two dimensions");
	if (da->n < 2) RAISE("left grid has less than two dimensions");
	/* bug: da[0]>=db[0] and da[1]>=db[1] are also conditions */
	int v[da->n];
	COPY(v,da->v,da->n);
	margy = db->get(0)/2;
	margx = db->get(1)/2;
	v[0] += 2*margy;
	v[1] += 2*margx;
	c.init(new Dim(da->n,v));
	out[0]->begin(da->dup());
	in->set_factor(da->prod(1));
}

GRID_FLOW(GridConvolve,0) {
	int factor = in->factor; /* line length of a */
	Dim *da = in->dim, *dc = c.dim, *db = b.dim;
	rassert(da);
	rassert(db);
	rassert(dc);
	int my = db->get(0), ny = da->get(0) - my/2*2;
	int mx = db->get(1);
	int ll = dc->prod(1); /* line length of c */
	int l  = dc->prod(2); /* "pixel" length of a,c */
	int oy = ll*(my/2), ox = l*(mx/2);
	int i = in->dex / factor; /* line number of a */
	Pt<Number> base = c.as_int32()+(i+margy)*ll+margx*l;
	
	/* building c from a */
	while (n) {
		COPY(base,data,factor);
		COPY(base+factor,data,margx*l);
		COPY(base-margx*l,data+factor-margx*l,margx*l);
		base += ll; data += factor; n -= factor; i++;
	}
}

GRID_END(GridConvolve,0) {
	Dim *da = in->dim, *dc = c.dim, *db = b.dim;
	int dby = db->get(0), day = da->get(0);
	int dbx = db->get(1), dax = da->get(1);
	int l  = dc->prod(2);
	int ll = dc->prod(1);
	Pt<Number> cp = c.as_int32();

	/* finishing building c from a */
	COPY(cp                         ,cp+da->get(0)*ll,margy*ll);
	COPY(cp+(margy+da->get(0))*ll,cp+margy*ll,  margy*ll);

	/* the real stuff */

	STACK_ARRAY(Number,buf3,l);
	STACK_ARRAY(Number,buf2,l*dbx*dby);
	STACK_ARRAY(Number,buf ,l*dbx*dby);
	Pt<Number> q=buf2;
	for (int i=0; i<dbx*dby; i++) for (int j=0; j<l; j++) *q++=b.as_int32()[i];

	for (int iy=0; iy<day; iy++) {
		for (int ix=0; ix<dax; ix++) {
			Pt<Number> p = c.as_int32() + iy*ll + ix*l;
			Pt<Number> r = buf;
			for (int jy=dby; jy; jy--,p+=ll,r+=dbx*l) COPY(r,p,dbx*l);
			for (int i=l-1; i>=0; i--) buf3[i]=rint;
			op_para->on_int32.op_array2(l*dbx*dby,buf,buf2);
			op_fold->on_int32.op_fold2(l,buf3,dbx*dby,buf);
			out[0]->send(l,buf3);
		}
	}
	out[0]->end();
	c.del();
}

GRID_BEGIN(GridConvolve,1) {
	int length = in->dim->prod();
	int count = in->dim->n;
	if (count != 2) RAISE("only exactly two dimensions allowed for now");
	/* because odd * odd = odd */
	if ((length & 1) == 0) RAISE("even number of elements");
	this->in[0]->abort();
	b.del();
	b.init(in->dim->dup());
}

GRID_FLOW(GridConvolve,1) { COPY(&b.as_int32()[in->dex], data, n); }

GRID_END(GridConvolve,1) {}

METHOD3(GridConvolve,init) {
	rb_call_super(argc,argv);
	if (argc>4) RAISE("too many args");
	op_para = OP2(argc<1 ? SYM(*) : argv[0]);
	op_fold = OP2(argc<2 ? SYM(+) : argv[1]);
	rint = argc<3 ? 0 : INT(argv[2]);
	c.init(0);
	b.init(0);
	if (argc==4) b.init_from_ruby(argv[3]);
	return Qnil;
}

GRCLASS(GridConvolve,"@convolve",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridConvolve,0,4),GRINLET(GridConvolve,1,4)),
	DECL(GridConvolve,init))

/* **************************************************************** */

struct GridFor : GridObject {
	Grid from;
	Grid to;
	Grid step;
	DECL3(init);
	DECL3(_0_set);
	DECL3(_0_bang);
	GRINLET3(0);
	GRINLET3(1);
	GRINLET3(2);
};

/*{ Dim[],Dim[],Dim[] -> Dim[A] }
  or
  { Dim[B],Dim[B],Dim[B] -> Dim[*As,B] }*/

METHOD3(GridFor,init) {
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
	int nn[n+1];
	STACK_ARRAY(Number,x,n);
	Pt<Number> fromb = from.as_int32();
	Pt<Number>   tob = to  .as_int32();
	Pt<Number> stepb = step.as_int32();
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
	out[0]->end();
	return Qnil;
}

METHOD3(GridFor,_0_set) {
	from.init_from_ruby(argv[0]);
	return Qnil;
}

GRID_INPUT(GridFor,2,step) {}
GRID_INPUT(GridFor,1,to) {}
GRID_INPUT(GridFor,0,from) {_0_bang(0,0);}

GRCLASS(GridFor,"@for",inlets:3,outlets:1,startup:0,
LIST(GRINLET(GridFor,0,4),GRINLET(GridFor,1,4),GRINLET(GridFor,2,4)),
	DECL(GridFor,init),
	DECL(GridFor,_0_bang),
	DECL(GridFor,_0_set))

/* **************************************************************** */

/*{ Dim[*As] -> Dim[B] }*/

struct GridDim : GridObject {
	GRINLET3(0);
};

GRID_BEGIN(GridDim,0) {
	int n = in->dim->n;
	out[0]->begin(new Dim(1,&n));
	out[0]->send(n,Pt<Number>((Number *)in->dim->v,in->dim->n));
	out[0]->end();
}

GRID_FLOW(GridDim,0) {}

GRID_END(GridDim,0) {}

GRCLASS(GridDim,"@dim",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridDim,0,4)))

/* **************************************************************** */

/*{ Dim[*As],Dim[B] -> Dim[*Cs] }*/

struct GridRedim : GridObject {
	Grid dim_grid;
	Dim *dim;
	Grid temp; /* temp.dim is not of the same shape as dim */

	~GridRedim() {
		if (dim) delete dim;
	}
	DECL3(init);
	GRINLET3(0);
	GRINLET3(1);
};

GRID_BEGIN(GridRedim,0) {
	temp.del();
	int a = in->dim->prod(), b = dim->prod();
	if (a<b) temp.init(new Dim(1,&a));
	out[0]->begin(dim->dup());
}

GRID_FLOW(GridRedim,0) {
	int i = in->dex;
	if (temp.is_empty()) {
		int b = dim->prod();
		int n2 = min(n,b-i);
		if (n2>0) out[0]->send(n2,data);
		/* discard other values if any */
	} else {
		int a = in->dim->prod();
		int n2 = min(n,a-i);
		COPY(&temp.as_int32()[i],data,n2);
		if (n2>0) out[0]->send(n2,data);
	}
}

GRID_END(GridRedim,0) {
	if (!temp.is_empty()) {
		int a = in->dim->prod(), b = dim->prod();
		int i = a;
		while (i<b) {
			out[0]->send(min(a,b-i),temp.as_int32());
			i += a;
		}
	}
	out[0]->end();
	temp.del();
}

GRID_INPUT(GridRedim,1,dim_grid) { dim = dim_grid.to_dim(); }

METHOD3(GridRedim,init) {
	rb_call_super(argc,argv);
	if (argc!=1) RAISE("wrong number of args");
	Grid t;
	t.init_from_ruby(argv[0]);
	expect_dim_dim_list(t.dim);
	dim = new Dim(t.dim->prod(),(int *)(Number *)t.as_int32());
	return Qnil;
}

GRCLASS(GridRedim,"@redim",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridRedim,0,4),GRINLET(GridRedim,1,4)),
	DECL(GridRedim,init))

/* ---------------------------------------------------------------- */
/* "@scale_by" does quick scaling of pictures by integer factors */

/*{ Dim[A,B,3] -> Dim[C,D,3] }*/

struct GridScaleBy : GridObject {
	Grid scale; /* integer scale factor */
	int scaley;
	int scalex;
	DECL3(init);
	GRINLET3(0);
};

/* processing a grid coming from inlet 0 */
/* one needs three special methods for that; they are declared using macros */
/* this one processes the header and accepts or rejects the grid */
GRID_BEGIN(GridScaleBy,0) {
	Dim *a = in->dim;

	/* there are restrictions on grid sizes for efficiency reasons */
	expect_rgb_picture(a);

	/* computing the output's size */
	int v[3]={ a->get(0)*scaley, a->get(1)*scalex, a->get(2) };
	out[0]->begin(new Dim(3,v));

	/* configuring the input format */
	in->set_factor(a->get(1)*a->get(2));
}

/* this method processes one packet of grid content */
GRID_FLOW(GridScaleBy,0) {
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
}

/* not much to do here: when the input is done, the output is done too */
GRID_END(GridScaleBy,0) { out[0]->end(); }

static void expect_scale_factor (Dim *dim) {
	if (dim->prod()!=1 && dim->prod()!=2)
		RAISE("expecting only one or two numbers");
}

/* the constructor accepts a scale factor as an argument */
/* that argument is not modifiable through an inlet yet (that would be the right inlet) */
METHOD3(GridScaleBy,init) {
	scale.constrain(expect_scale_factor);
	scale.init_from_ruby(argc<1 ? EVAL("[2]") : argv[0]);
	scaley = scale.as_int32()[0];
	scalex = scale.as_int32()[scale.dim->prod()==1 ? 0 : 1];
	if (scaley<1) scaley=1;
	if (scalex<1) scalex=1;

	rb_call_super(argc,argv);
	out[0] = new GridOutlet(this,0); // wtf?
	return Qnil;
}

/* there's one inlet, one outlet, and two system methods (inlet #-1) */
GRCLASS(GridScaleBy,"@scale_by",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridScaleBy,0,4)),
	DECL(GridScaleBy,init))

/* ---------------------------------------------------------------- */
/* "@downscale_by" does quick downscaling of pictures by integer factors */

/*{ Dim[A,B,3] -> Dim[C,D,3] }*/

struct GridDownscaleBy : GridObject {
	Grid scale;
	int scaley;
	int scalex;
	bool smoothly;
	Grid temp;
	DECL3(init);
	GRINLET3(0);
};

GRID_BEGIN(GridDownscaleBy,0) {
	Dim *a = in->dim;

	if (a->n!=3) RAISE("(height,width,chans) please");
	if (a->get(2)!=3) RAISE("3 chans please");

	int v[3]={ a->get(0)/scaley, a->get(1)/scalex, a->get(2) };
	out[0]->begin(new Dim(3,v));

	in->set_factor(a->get(1)*a->get(2));

	temp.del();
	int w[]={in->dim->get(1)/scalex,in->dim->get(2)};
	temp.init(new Dim(2,w));
}

GRID_FLOW(GridDownscaleBy,0) {
	int rowsize = in->dim->prod(1);
	int rowsize2 = temp.dim->prod();
	Pt<Number> buf = temp.as_int32();

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
				OP2(SYM(/))->on_int32.op_array(rowsize2,buf,scalex*scaley);
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
}

GRID_END(GridDownscaleBy,0) { out[0]->end(); }

METHOD3(GridDownscaleBy,init) {
	scale.constrain(expect_scale_factor);
	scale.init_from_ruby(argc<1 ? EVAL("[2]") : argv[0]);
	scaley = scale.as_int32()[0];
	scalex = scale.as_int32()[scale.dim->prod()==1 ? 0 : 1];
	if (scaley<1) scaley=1;
	if (scalex<1) scalex=1;

	smoothly = (argc>1 && argv[1]==SYM(smoothly));
	rb_call_super(argc,argv);
	out[0] = new GridOutlet(this,0); // wtf?
	return Qnil;
}

GRCLASS(GridDownscaleBy,"@downscale_by",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridDownscaleBy,0,4)),
	DECL(GridDownscaleBy,init))

/* **************************************************************** */

struct GridLayer : GridObject {
	Grid r;
	GridLayer() {
		r.constrain(expect_rgb_picture);
	}
	GRINLET3(0);
	GRINLET3(1);
	DECL3(init);
};

GRID_BEGIN(GridLayer,0) {
	Dim *a = in->dim;
	expect_rgba_picture(a);
	if (a->get(1)!=r.dim->get(1)) RAISE("same width please");
	if (a->get(0)!=r.dim->get(0)) RAISE("same height please");
	in->set_factor(a->prod(1));
	out[0]->begin(r.dim->dup());
}

GRID_FLOW(GridLayer,0) {
	Pt<Number> rr = r.as_int32() + in->dex*3/4;
	STACK_ARRAY(Number,foo,n*3/4);
	for (int i=0,j=0; i<n; i+=4,j+=3) {
		foo[j+0] = (data[i+0]*data[i+3] + rr[j+0]*(256-data[i+3])) >> 8;
		foo[j+1] = (data[i+1]*data[i+3] + rr[j+1]*(256-data[i+3])) >> 8;
		foo[j+2] = (data[i+2]*data[i+3] + rr[j+2]*(256-data[i+3])) >> 8;
	}
	out[0]->send(n*3/4,foo);
}

GRID_END(GridLayer,0) { out[0]->end(); }

GRID_INPUT(GridLayer,1,r) {}

METHOD3(GridLayer,init) {
	rb_call_super(argc,argv);
	return Qnil;
}

GRCLASS(GridLayer,"@layer",inlets:2,outlets:1,startup:0,
LIST(GRINLET(GridLayer,0,4),GRINLET(GridLayer,1,4)),
	DECL(GridLayer,init))

/* **************************************************************** */

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
/*
struct GridPolygon : GridObject {
	Grid polygon;
	Grid color;
	DECL3(init);
	GRINLET3(0);
	GRINLET3(1);
	GRINLET3(2);
};

GRID_BEGIN(GridPolygon,0) {
}

GRID_FLOW(GridPolygon,0) {
}

GRID_END(GridPolygon,0) {}

GRID_INPUT(GridPolygon,2,polygon) {}
GRID_INPUT(GridPolygon,1,color) {}

METHOD3(GridPolygon,init) {
}

GRCLASS(GridPolygon,"@polygon",inlets:3,outlets:1,startup:0,
LIST(GRINLET(GridPolygon,0,4),GRINLET(GridPolygon,1,4),GRINLET(GridPolygon,2,4)),
	DECL(GridPolygon,init))
*/

/* **************************************************************** */

/*{ Dim[*As,3] -> Dim[*As,3] }*/

struct GridRGBtoHSV : GridObject {
	DECL3(init);
	GRINLET3(0);
};

GRID_BEGIN(GridRGBtoHSV,0) {
	if (in->dim->n<1) RAISE("at least 1 dim please");
	if (in->dim->get(in->dim->n-1)!=3) RAISE("3 chans please");
	out[0]->begin(in->dim->dup());
	in->set_factor(3);
}

/*
  h=42*0: red
  h=42*1: yellow
  h=42*2: green
  h=42*3: cyan
  h=42*4: blue
  h=42*5: magenta
  h=42*6: crap
*/
GRID_FLOW(GridRGBtoHSV,0) {
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
}

GRID_END(GridRGBtoHSV,0) {
	out[0]->end();
}

METHOD3(GridRGBtoHSV,init) {
	rb_call_super(argc,argv);
	out[0] = new GridOutlet(this,0); // wtf?
	return Qnil;
}

GRCLASS(GridRGBtoHSV,"@rgb_to_hsv",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridRGBtoHSV,0,4)),
	DECL(GridRGBtoHSV,init))

/* **************************************************************** */

/*{ Dim[*As,3] -> Dim[*As,3] }*/

struct GridHSVtoRGB : GridObject {
	DECL3(init);
	GRINLET3(0);
};

GRID_BEGIN(GridHSVtoRGB,0) {
	if (in->dim->n<1) RAISE("at least 1 dim please");
	if (in->dim->get(in->dim->n-1)!=3) RAISE("3 chans please");
	out[0]->begin(in->dim->dup());
	in->set_factor(3);
}

GRID_FLOW(GridHSVtoRGB,0) {
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
}

GRID_END(GridHSVtoRGB,0) { out[0]->end(); }

METHOD3(GridHSVtoRGB,init) {
	rb_call_super(argc,argv);
	out[0] = new GridOutlet(this,0); // wtf?
	return Qnil;
}

GRCLASS(GridHSVtoRGB,"@hsv_to_rgb",inlets:1,outlets:1,startup:0,
LIST(GRINLET(GridHSVtoRGB,0,4)),
	DECL(GridHSVtoRGB,init))

/* **************************************************************** */
/* [rtmetro] */

struct RtMetro : GridObject {
	int ms; /* millisecond time interval */
	bool on;
	uint64 next_time; /* microseconds since epoch: next time an event occurs */
	uint64 last;      /* microseconds since epoch: last time we checked */
	int mode; /* 0=equal; 1=geiger */

	uint64 delay();

	DECL3(init);
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
	//gfpost("rtmetro alarm tick: %lld; next_time: %lld; now-last: %lld",now,$->next_time,now-$->last);
	if (now >= $->next_time) {
		//gfpost("rtmetro sending bang");
		Ruby a[] = { INT2NUM(0), sym_bang };
		FObject_send_out(COUNT(a),a,rself);
		/* $->next_time = now; */ /* jmax style, less realtime */
		$->next_time += $->delay();
	}
	$->last = now;
}

METHOD3(RtMetro,_0_int) {
	int oon = on;
	on = !! FIX2INT(argv[0]);
	gfpost("on = %d",on);
	if (oon && !on) {
		gfpost("deleting rtmetro alarm for $=%08x rself=%08x",(long)this,(long)peer);
		MainLoop_remove((void *)peer);
	} else if (!oon && on) {
		gfpost("creating rtmetro alarm for $=%08x rself=%08x",(long)this,(long)peer);
		MainLoop_add((void *)peer,(void(*)(void*))RtMetro_alarm);
		next_time = RtMetro_now();
	}
	return Qnil;
}

METHOD3(RtMetro,_1_int) {
	ms = FIX2INT(argv[0]);
	gfpost("ms = %d",ms);
	return Qnil;
}

METHOD3(RtMetro,init) {
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
	DECL(RtMetro,init))

/* **************************************************************** */
/* [@global] */

/* a dummy object that gives access to any stuff global to
   GridFlow.
*/
struct GridGlobal : GridObject {
};

GRCLASS(GridGlobal,"@global",inlets:1,outlets:1,startup:0,
LIST())

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
//	INSTALL(GridPolygon);
//	INSTALL(GridRGBtoHSV); /* buggy */
//	INSTALL(GridHSVtoRGB); /* buggy */
	INSTALL(RtMetro);
	INSTALL(GridGlobal);
}

