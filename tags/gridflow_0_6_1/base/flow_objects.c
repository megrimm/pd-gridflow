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

#define IV(s) rb_ivar_get(rself,SI(s))
#define IVS(s,v) rb_ivar_set(rself,SI(s),v)

Operator1 *OP1(VALUE x) {
	VALUE s = rb_hash_aref(op1_dict,x);
	if (s==Qnil) RAISE("expected one-input-operator");
	return (Operator1 *)FIX2PTR(s);
}

Operator2 *OP2(VALUE x) {
	VALUE s = rb_hash_aref(op2_dict,x);
	if (s==Qnil) RAISE("expected two-input-operator");
	return (Operator2 *)FIX2PTR(s);
}

static void expect_dim_dim_list (Dim *d) {
	if (d->count()!=1) RAISE("dimension list must have 1 dimension");
	int n = d->get(0);
	if (n>MAX_DIMENSIONS) RAISE("too many dimensions");
}

/* **************************************************************** */
/*
  GridImport ("@import") is the class for converting a old-style stream
  of integers to a streamed grid as used now.
*/

struct GridImport : GridObject {
	Dim *dim; /* size of grids to send */
};

GRID_BEGIN(GridImport,0) {}
GRID_FLOW(GridImport,0) {
	GridOutlet *out = $->out[0];
	while (n) {
		if (!out->is_busy()) out->begin($->dim->dup());
		int n2 = out->dim->prod() - out->dex;
		if (n2 > n) n2 = n;
		out->send(n,data);
		if (out->dex >= out->dim->prod()) out->end();
		n -= n2;
		data += n2;
	}
}
GRID_END(GridImport,0) {}

/* same inlet 1 as @redim */

GRID_BEGIN(GridImport,1) {
	expect_dim_dim_list(in->dim);
	if (in->dim->n) in->set_factor(in->dim->n);
}

GRID_FLOW(GridImport,1) {
	FREE($->dim);
	$->dim = new Dim(n,(int *)data);
	$->in[0]->abort();
	$->out[0]->abort();
}

GRID_END(GridImport,1) {}

METHOD(GridImport,init) {
	rb_call_super(argc,argv);
	if (argc!=1) RAISE("wrong number of args");
	Grid t;
	t.init_from_ruby(argv[0]);
	expect_dim_dim_list(t.dim);
	$->dim = new Dim(t.dim->prod(),(int *)t.as_int32());
}

METHOD(GridImport,delete) {
	FREE($->dim);
	rb_call_super(argc,argv);
}

METHOD(GridImport,_0_int) {
	GridOutlet *out = $->out[0];
	Number data[] = { INT(argv[0]) };
	if (!out->is_busy()) out->begin($->dim->dup());
	out->send(COUNT(data),data);
	if (out->dex >= out->dim->prod()) out->end();
}

METHOD(GridImport,_0_reset) {
	GridOutlet *out = $->out[0];
	if (out->is_busy()) out->abort();
}

GRCLASS(GridImport,"@import",inlets:2,outlets:1,
LIST(GRINLET(GridImport,0),GRINLET(GridImport,1)),
	DECL(GridImport,init),
	DECL(GridImport,delete),
	DECL(GridImport,_0_int),
	DECL(GridImport,_0_reset))

/* **************************************************************** */
/*
  GridExport ("@export") is the class for converting from streamed grids
  to old-style integer stream.
*/

struct GridExport : GridObject {
};

GRID_BEGIN(GridExport,0) {}

GRID_FLOW(GridExport,0) {
	for (int i=0; i<n; i++) {
		VALUE a[] = { INT2NUM(0), sym_int, INT2NUM(*data) };
		FObject_send_out(COUNT(a),a,rself);
		data++;
	}
}

GRID_END(GridExport,0) {}

GRCLASS(GridExport,"@export",inlets:1,outlets:1,
LIST(GRINLET(GridExport,0)))
/* outlet 0 not used for grids */

/* **************************************************************** */

struct GridExportList : GridObject {
	VALUE /*Array*/ list; //!@#$mark
	int n;
};

GRID_BEGIN(GridExportList,0) {
	int n = in->dim->prod();
	if (n>1000) RAISE("list too big (%d elements)", n);
	$->list = rb_ary_new();
	$->n = n;
}

GRID_FLOW(GridExportList,0) {
	for (int i=0; i<n; i++, data++)
		rb_ary_store($->list,in->dex+i,INT2NUM(*data));
}
		
GRID_END(GridExportList,0) {
	VALUE a[$->n+2];
	a[0] = INT2NUM(0);
	a[1] = sym_list;
	memcpy(a+2,RARRAY($->list)->ptr,$->n*sizeof(void*));
	FObject_send_out(COUNT(a),a,rself);
	$->list = 0;
}

GRCLASS(GridExportList,"@export_list",inlets:1,outlets:1,
LIST(GRINLET(GridExportList,0)))
/* outlet 0 not used for grids */

/* **************************************************************** */
/*
  grid_store ("@store") is the class for storing a grid and restituting
  it on demand. The right inlet receives the grid. The left inlet receives
  either a bang (which forwards the whole image) or a grid describing what to
  send.
*/

struct GridStore : GridObject {
	Grid r;
};

GRID_BEGIN(GridStore,0) {
	assert(in->dim);
	int na = in->dim->count();
	int nb,nc,nd,i;
	int v[MAX_DIMENSIONS];
	if ($->r.is_empty()) RAISE("empty buffer, better luck next time.");

	nb = $->r.dim->count();

	if (na<1) RAISE("must have at least 1 dimension.",na,1,1+nb);

	nc = in->dim->get(na-1);
	if (nc > nb)
		RAISE("wrong number of elements in last dimension: "
			"got %d, expecting <= %d", nc, nb);
	nd = nb - nc + na - 1;
	if (nd > MAX_DIMENSIONS) RAISE("too many dimensions!");
	for (i=0; i<na-1; i++) v[i] = in->dim->get(i);
	for (i=nc; i<nb; i++) v[na-1+i-nc] = $->r.dim->get(i);
	$->out[0]->begin(new Dim(nd,v));
	in->set_factor(nc);
}

GRID_FLOW(GridStore,0) {
	int na = in->dim->count();
	int nb = $->r.dim->count();
	int nc = in->dim->get(na-1);
	int size = $->r.dim->prod(nc);
	int v[nb];
	assert((n % nc) == 0);
	assert($->data);

	while (n>0) {
		int pos;
		int i;
		for (i=0; i<nc; i++,data++) v[i] = mod(*data,$->r.dim->v[i]);
		while (i<nb) v[i++] = 0;
		pos = $->r.dim->calc_dex(v);
		switch ($->r.nt) {
		case int32_type_i: {
			Number *data2 = $->r.as_int32() + pos;
			$->out[0]->send(size,data2);
		 	break;}
		case uint8_type_i: {
			int bs = gf_max_packet_length;
			Number data3[bs];
			uint8 *data2 = $->r.as_uint8() + pos;
			int left = size;
			while (left>=bs) {
				for (int i=0; i<bs; i++) { data3[i] = data2[i]; }
				$->out[0]->send(bs,data3);
				data2+=bs;
				left-=bs;
			}
			while (left) {
				data3[0] = *data2++;
				$->out[0]->send(1,data3);
				left--;
			}
			break;}
		default: assert(0);
		}
		n -= nc;
	}
}

GRID_END(GridStore,0) { $->out[0]->end(); }

GRID_BEGIN(GridStore,1) {
	$->in[0]->abort();
	NumberTypeIndex nt = $->r.nt;
	$->r.del();
	$->r.init(in->dim->dup(),nt);
}

GRID_FLOW(GridStore,1) {
	switch ($->r.nt) {
	case int32_type_i:{
		Number *data2 = $->r.as_int32() + in->dex;
		memcpy(data2, data, n*sizeof(Number)); break;}
	case uint8_type_i:{
		uint8 *data2 = $->r.as_uint8() + in->dex;
		for(int i=0; i<n; i++) data2[i] = data[i];
		break;}
	default: assert(0);
	}
	in->dex += n;
}

GRID_END(GridStore,1) {}

METHOD(GridStore,init) {
	VALUE t = argc==0 ? SYM(int32) : argv[0];
	rb_call_super(argc,argv);
	$->r.init(0,
		t==SYM(int32) ? int32_type_i :
		t==SYM(uint8) ? uint8_type_i :
		(RAISE("unknown element type \"%s\"", t),int32_type_i));
}

METHOD(GridStore,delete) {
	$->r.del();
	rb_call_super(argc,argv);
}

METHOD(GridStore,_0_bang) {
	if ($->r.is_empty()) RAISE("empty buffer, better luck next time.");
	$->out[0]->begin($->r.dim->dup());
	$->out[0]->send($->r.dim->prod(),$->r.as_int32());
	$->out[0]->end();
}

GRCLASS(GridStore,"@store",inlets:2,outlets:1,
LIST(GRINLET(GridStore,0),GRINLET(GridStore,1)),
	DECL(GridStore,init),
	DECL(GridStore,delete),
	DECL(GridStore,_0_bang))

/* **************************************************************** */

struct GridOp1 : GridObject {
	const Operator1 *op;
};

GRID_BEGIN(GridOp1,0) {
	$->out[0]->begin(in->dim->dup());
	in->dex = 0;
}

GRID_FLOW(GridOp1,0) {
	Number *data2 = NEW2(Number,n);
	GridOutlet *out = $->out[0];
	memcpy(data2,data,n*sizeof(Number));
	$->op->op_array(n,data2);
	in->dex += n;
	out->send(n,data2);
	FREE(data2);
}

GRID_END(GridOp1,0) { $->out[0]->end(); }

METHOD(GridOp1,init) {
	rb_call_super(argc,argv);
	$->op = OP1(argv[0]);
}

GRCLASS(GridOp1,"@!",inlets:1,outlets:1,
LIST(GRINLET(GridOp1,0)),
	DECL(GridOp1,init))

/* **************************************************************** */
/*
  GridOp2 ("@") is the class of objects for parallel operation on the
  values of the left grid with values of a (stored) right grid or (stored)
  single value.
*/

struct GridOp2 : GridObject {
	const Operator2 *op;
	Grid r;
};

GRID_BEGIN(GridOp2,0) {
	$->out[0]->begin(in->dim->dup());
	in->dex = 0;
}

GRID_FLOW2(GridOp2,0) {
	GridOutlet *out = $->out[0];
	if ($->r.is_empty()) RAISE("ARGH");
	int32 *rdata = $->r.as_int32();
	int loop = $->r.dim->prod();
	if (loop>1) {
		if (in->dex+n <= loop) {
			$->op->op_array2(n,data,rdata+in->dex);
		} else {
			Number data2[n];
			//!@#$ make this faster
			for (int i=0; i<n; i++) {
				data2[i] = rdata[(in->dex+i)%loop];
			}
			$->op->op_array2(n,data,data2);
		}
	} else {
		$->op->op_array(n,data,*rdata);
	}
	in->dex += n;
	out->give(n,data);
}

GRID_END(GridOp2,0) { $->out[0]->end(); }

GRID_BEGIN(GridOp2,1) {
	$->r.del();
	$->r.init(in->dim->dup(),int32_type_i);
}

GRID_FLOW(GridOp2,1) {
	memcpy(&$->r.as_int32()[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridOp2,1) {}

METHOD(GridOp2,init) {
	rb_call_super(argc,argv);
	$->op = OP2(argv[0]);
	if (argc>2) RAISE("too many args");
	if (argc<2) {
		$->r.init(new Dim(0,0),int32_type_i);
		$->r.as_int32()[0] = 0;
	} else {
		$->r.init_from_ruby(argv[1]);
	}
}

METHOD(GridOp2,delete) {
	$->r.del();
	rb_call_super(argc,argv);
}

METHOD(GridOp2,_1_int) {
	$->r.del();
	$->r.init(new Dim(0,0),int32_type_i);
	*$->r.as_int32() = INT(argv[0]);
}

GRCLASS(GridOp2,"@",inlets:2,outlets:1,
LIST(GRINLET2(GridOp2,0),GRINLET(GridOp2,1)),
	DECL(GridOp2,init),
	DECL(GridOp2,delete),
	DECL(GridOp2,_1_int))

/* **************************************************************** */
/*
  GridFold ("@fold") is the class of objects for removing the last dimension
  by cascading an operation on all those values. There is a start value. When
  doing [@fold + 42] each new value is computed like 42+a+b+c+...
*/

struct GridFold : GridObject {
	const Operator2 *op;
	int rint;
};

GRID_BEGIN(GridFold,0) {
	int n = in->dim->count();
	if (n<1) RAISE("minimum 1 dimension");
	Dim *foo = new Dim(n-1,in->dim->v);
	$->out[0]->begin(foo);
	in->set_factor(in->dim->get(in->dim->count()-1));
}

GRID_FLOW(GridFold,0) {
	int factor = in->dim->get(in->dim->count()-1);
	GridOutlet *out = $->out[0];
	int bs = n/factor;
	Number buf[bs];
	int i=0;

	assert (n % factor == 0);

	while (n) {
		buf[i++] = $->op->op_fold($->rint,factor,data);
		data += factor;
		n -= factor;
	}
	out->send(bs,buf);
}

GRID_END(GridFold,0) { $->out[0]->end(); }

METHOD(GridFold,init) {
	rb_call_super(argc,argv);
	$->op = OP2(argv[0]);
	$->rint = argc<2 ? 0 : INT(argv[1]);
}

METHOD(GridFold,_1_int) {
	$->rint = INT(argv[0]);
}

GRCLASS(GridFold,"@fold",inlets:2,outlets:1,
LIST(GRINLET(GridFold,0)),
	DECL(GridFold,init),
	DECL(GridFold,_1_int))

/* **************************************************************** */
/*
  GridScan ("@scan") is similar to @fold except that it gives back all
  the partial results thereof; therefore the output is of the same
  size as the input (unlike @fold).
*/

typedef struct GridFold GridScan;

GRID_BEGIN(GridScan,0) {
	int n = in->dim->count();
	if (n<1) RAISE("minimum 1 dimension");
	$->out[0]->begin(in->dim->dup());
	in->set_factor(in->dim->get(in->dim->count()-1));
}

GRID_FLOW(GridScan,0) {
	int factor = in->dim->get(in->dim->count()-1);
	GridOutlet *out = $->out[0];
	Number buf[n];
	int nn=n;

	assert (n % factor == 0);

	while (n) {
		memcpy(buf,data,n*sizeof(Number));
		$->op->op_scan($->rint,factor,buf);
		data += factor;
		n -= factor;
	}
	out->send(nn,buf);
}

GRID_END(GridScan,0) { $->out[0]->end(); }

METHOD(GridScan,init) {
	rb_call_super(argc,argv);
	$->op = OP2(argv[0]);
	$->rint = argc<2 ? 0 : INT(argv[1]);
}

METHOD(GridScan,_1_int) {
	$->rint = INT(argv[0]);
}

GRCLASS(GridScan,"@scan",inlets:2,outlets:1,
LIST(GRINLET(GridScan,0)),
	DECL(GridScan,init),
	DECL(GridScan,_1_int))

/* **************************************************************** */
/* inner: (op_para,op_fold,rint,A in dim(*As,A0), B in dim(B0,*Bs))
          -> c in dim(*As,*Bs)
   c = map((*As,*Bs),fold(op_fold,rint,map2(op_para,...... whatever
*/

struct GridInner : GridObject {
	const Operator2 *op_para;
	const Operator2 *op_fold;
	Number rint;
	Grid r;
};

GRID_BEGIN(GridInner,0) {
	Dim *a = in->dim;
	Dim *b = $->r.dim;
	if (!b) RAISE("right inlet has no grid");
	if (a->count()<1) RAISE("minimum 1 dimension");
	int a_last = a->get(a->count()-1);
	int b_first = b->get(0);
	int n = a->count()+b->count()-2;
	int v[n];
	int i,j;
	if (a_last != b_first)
		RAISE("last dim of left side should be same size as first dim of right side");
	for (i=j=0; j<a->count()-1; i++,j++) v[i] = a->get(j);
	for (  j=1; j<b->count()  ; i++,j++) v[i] = b->get(j);
	$->out[0]->begin(new Dim(n,v));
	in->set_factor(a_last);
}

GRID_FLOW(GridInner,0) {
	GridOutlet *out = $->out[0];
	int factor = in->dim->get(in->dim->count()-1);
	int b_prod = $->r.dim->prod();
	Number *buf2 = NEW(Number,b_prod/factor);
	Number buf[factor], bufr[factor];
	assert (n % factor == 0);

	for (int i=0; i<n; i+=factor) {
		for (int j=0; j<b_prod/factor; j++) {
			memcpy(buf,&data[i],factor*sizeof(Number));
			for (int k=0; k<factor; k++) bufr[k]=$->r.as_int32()[b_prod/factor*k+j];
			$->op_para->op_array2(factor,buf,bufr);
			buf2[j] = $->op_fold->op_fold($->rint,factor,buf);
		}
		out->send(b_prod/factor,buf2);
	}
	FREE(buf2);
}

GRID_END(GridInner,0) { $->out[0]->end(); }

GRID_BEGIN(GridInner,2) {
	$->in[0]->abort();
	if (in->dim->count()<1) RAISE("minimum 1 dimension");
	$->r.del();
	$->r.init(in->dim->dup(),int32_type_i);
}

GRID_FLOW(GridInner,2) {
	memcpy(&$->r.as_int32()[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridInner,2) {}

METHOD(GridInner,init) {
	rb_call_super(argc,argv);
	$->op_para = argc<1 ? OP2(SYM(*)) : OP2(argv[0]);
	$->op_fold = argc<2 ? OP2(SYM(+)) : OP2(argv[1]);
	$->rint = argc<3 ? 0 : INT(argv[2]);
	$->r.init(0);
}

METHOD(GridInner,delete) {
	$->r.del();
	rb_call_super(argc,argv);
}

GRCLASS(GridInner,"@inner",inlets:3,outlets:1,
LIST(GRINLET(GridInner,0),GRINLET(GridInner,2)),
	DECL(GridInner,init),
	DECL(GridInner,delete))

/* **************************************************************** */

typedef struct GridInner GridInner2;

GRID_BEGIN(GridInner2,0) {
	Dim *a = in->dim;
	Dim *b = $->r.dim;
	if (!b) RAISE("right inlet has no grid");
	if (a->count()<1) RAISE("minimum 1 dimension");
	int a_last = a->get(a->count()-1);
	int b_last = b->get(b->count()-1);
	int n = a->count()+b->count()-2;
	int v[n];
	int i,j;
	if (a_last != b_last)
		RAISE("last dimension of each grid must have same size");
	for (i=j=0; j<a->count()-1; i++,j++) { v[i] = a->get(j); }
	for (  j=0; j<b->count()-1; i++,j++) { v[i] = b->get(j); }
	$->out[0]->begin(new Dim(n,v));
	in->set_factor(a_last);
}

GRID_FLOW(GridInner2,0) {
	GridOutlet *out = $->out[0];
	int factor = in->dim->get(in->dim->count()-1);
	int b_prod = $->r.dim->prod();
	Number *buf2 = NEW(Number,b_prod/factor);
	Number buf[factor];
	assert (n % factor == 0);

	for (int i=0; i<n; i+=factor) {
		for (int j=0; j<b_prod; j+=factor) {
			memcpy(buf,&data[i],factor*sizeof(Number));
			$->op_para->op_array2(factor,buf,$->r.as_int32()+j);
			buf2[j/factor] = $->op_fold->op_fold($->rint,factor,buf);
		}
		out->send(b_prod/factor,buf2);
	}
	FREE(buf2);
}

GRID_END(GridInner2,0) { $->out[0]->end(); }

GRID_BEGIN(GridInner2,2) {
	$->in[0]->abort();
	if (in->dim->count()<1) RAISE("minimum 1 dimension");
	$->r.del();
	$->r.init(in->dim->dup(),int32_type_i);
}

GRID_FLOW(GridInner2,2) {
	memcpy(&$->r.as_int32()[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridInner2,2) {}

METHOD(GridInner2,init) {
	rb_call_super(argc,argv);
	$->op_para = argc<1 ? OP2(SYM(*)) : OP2(argv[0]);
	$->op_fold = argc<2 ? OP2(SYM(+)) : OP2(argv[1]);
	$->rint = argc<3 ? 0 : INT(argv[2]);
	$->r.init(0);
}

METHOD(GridInner2,delete) {
	$->r.del();
	rb_call_super(argc,argv);
}

GRCLASS(GridInner2,"@inner2",inlets:3,outlets:1,
LIST(GRINLET(GridInner2,0),GRINLET(GridInner2,2)),
	DECL(GridInner2,init),
	DECL(GridInner2,delete))

/* **************************************************************** */

struct GridOuter : GridObject {
	Grid r;
	const Operator2 *op;
};

GRID_BEGIN(GridOuter,0) {
	Dim *a = in->dim;
	Dim *b = $->r.dim;
	if (!b) RAISE("right inlet has no grid");
	int n = a->count()+b->count();
	int v[n];
	int i,j;
	for (i=j=0; j<a->count(); i++,j++) { v[i] = a->get(j); }
	for (  j=0; j<b->count(); i++,j++) { v[i] = b->get(j); }
	$->out[0]->begin(new Dim(n,v));
}

GRID_FLOW(GridOuter,0) {
	int b_prod = $->r.dim->prod();
	Number *buf = NEW(Number,b_prod);
	while (n) {
		for (int j=0; j<b_prod; j++) buf[j] = *data;
		$->op->op_array2(b_prod,buf,$->r.as_int32());
		$->out[0]->send(b_prod,buf);
		data++; n--;
	}
	FREE(buf);
}

GRID_END(GridOuter,0) { $->out[0]->end(); }

GRID_BEGIN(GridOuter,1) {
	$->in[0]->abort();
	$->r.del();
	$->r.init(in->dim->dup());
}

GRID_FLOW(GridOuter,1) {
	memcpy(&$->r.as_int32()[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridOuter,1) {}

METHOD(GridOuter,init) {
	rb_call_super(argc,argv);
	$->op = OP2(argv[0]);
	$->r.init(0);
	if (argc<1) RAISE("not enough args");
	if (argc>2) RAISE("too many args");
	if (argc==2) $->r.init_from_ruby(argv[1]);
}

METHOD(GridOuter,delete) {
	$->r.del();
	rb_call_super(argc,argv);
}

GRCLASS(GridOuter,"@outer",inlets:2,outlets:1,
LIST(GRINLET(GridOuter,0),GRINLET(GridOuter,1)),
	DECL(GridOuter,init),
	DECL(GridOuter,delete))

/* **************************************************************** */
/* the incoming grid is stored as "c" with a margin on the four sides
   of it. the margin is the size of the "b" grid minus one then split in two.
*/   

struct GridConvolve : GridObject {
	Grid c,b;
	const Operator2 *op_para, *op_fold;
	Number rint;
	int margx,margy; /* margins */
};

GRID_BEGIN(GridConvolve,0) {
	Dim *da = in->dim, *db = $->b.dim;
	if (!db) RAISE("right inlet has no grid");
	if (db->count() != 2) RAISE("right grid must have two dimensions");
	if (da->count() < 2) RAISE("left grid has less than two dimensions");
	/* bug: da[0]>=db[0] and da[1]>=db[1] are also conditions */
	int v[da->count()];
	memcpy(v,da->v,da->count()*sizeof(int));
	$->margy = db->get(0)/2;
	$->margx = db->get(1)/2;
	v[0] += 2*$->margy;
	v[1] += 2*$->margx;
	$->c.init(new Dim(da->count(),v));
	$->out[0]->begin(da->dup());
	in->set_factor(da->prod(1));
}

#define rassert(_p_) if (!(_p_)) RAISE(#_p_);

GRID_FLOW(GridConvolve,0) {
	int factor = in->factor; /* line length of a */
	Dim *da = in->dim, *dc = $->c.dim, *db = $->b.dim;
	rassert(da);
	rassert(db);
	rassert(dc);
	int my = db->get(0), ny = da->get(0) - my/2*2;
	int mx = db->get(1);
	int ll = dc->prod(1); /* line length of c */
	int l  = dc->prod(2); /* "pixel" length of a,c */
	int oy = ll*(my/2), ox = l*(mx/2);
	int i = in->dex / factor; /* line number of a */
	Number *base = $->c.as_int32()+(i+$->margy)*ll+$->margx*l;
	
	/* building c from a */
	while (n) {
		memcpy(base,data,factor*sizeof(Number));
		memcpy(base+factor,data,$->margx*l*sizeof(Number));
		memcpy(base-$->margx*l,data+factor-$->margx*l,$->margx*l*sizeof(Number));
		in->dex += factor; base += ll; data += factor; n -= factor; i++;
	}
}

GRID_END(GridConvolve,0) {
	Dim *da = in->dim, *dc = $->c.dim, *db = $->b.dim;
	int dby = db->get(0), day = da->get(0);
	int dbx = db->get(1), dax = da->get(1);
	int l  = dc->prod(2);
	int ll = dc->prod(1);
	Number buf[l],buf2[l],as[l];
	Number *cp = $->c.as_int32();

	memcpy(cp                         ,cp+da->get(0)*ll,$->margy*ll*sizeof(Number));
	memcpy(cp+($->margy+da->get(0))*ll,cp+$->margy*ll,  $->margy*ll*sizeof(Number));

	for (int k=0; k<l; k++) buf2[k]=$->rint;

	for (int iy=0; iy<day; iy++) {
		for (int ix=0; ix<dax; ix++) {
			Number *d = $->b.as_int32();
			Number *p = $->c.as_int32() + iy*ll + ix*l;
			memcpy(buf,buf2,l*sizeof(Number));
			for (int jy=dby; jy; jy--,p+=ll-dbx*l) {
				for (int jx=dbx; jx; jx--,p+=l) {
					Number b = *d++;
					memcpy(as,p,l*sizeof(Number));
					$->op_para->op_array(l,as,b);
					$->op_fold->op_array2(l,buf,as);
				}
			}
			$->out[0]->send(l,buf);
		}
	}
	$->out[0]->end();
	$->c.del();
}

GRID_BEGIN(GridConvolve,1) {
	int length = in->dim->prod();
	int count = in->dim->count();
	if (count != 2) RAISE("only exactly two dimensions allowed for now");
	/* because odd * odd = odd */
	if ((length & 1) == 0) RAISE("even number of elements");
	$->in[0]->abort();
	$->b.del();
	$->b.init(in->dim->dup());
}

GRID_FLOW(GridConvolve,1) {
	memcpy(&$->b.as_int32()[in->dex], data, n*sizeof(Number));
}

GRID_END(GridConvolve,1) {}

METHOD(GridConvolve,init) {
	rb_call_super(argc,argv);
	$->op_para = OP2(argc<1 ? SYM(*) : argv[0]);
	$->op_fold = OP2(argc<2 ? SYM(+) : argv[1]);
	$->rint = argc<3 ? 0 : INT(argv[2]);
	$->c.init(0);
	$->b.init(0);
}

METHOD(GridConvolve,delete) {
	$->c.del();
	$->b.del();
	rb_call_super(argc,argv);
}

GRCLASS(GridConvolve,"@convolve",inlets:2,outlets:1,
LIST(GRINLET(GridConvolve,0),GRINLET(GridConvolve,1)),
	DECL(GridConvolve,init),
	DECL(GridConvolve,delete))

/* **************************************************************** */

struct GridFor : GridObject {
	Number from;
	Number to;
	Number step;
};

METHOD(GridFor,init) {
	rb_call_super(argc,argv);
	$->from = INT(argv[0]);
	$->to   = INT(argv[1]);
	$->step = INT(argv[2]);
	if (!$->step) $->step=1;
}

METHOD(GridFor,_0_set) { $->from = INT(argv[0]); }
METHOD(GridFor,_1_int) { $->to   = INT(argv[0]); }
METHOD(GridFor,_2_int) { $->step = INT(argv[0]); if (!$->step) $->step=1; }

METHOD(GridFor,_0_bang) {
	int v = ($->to - $->from + $->step - cmp($->step,0)) / $->step;
	Number x;
	if (v<0) v=0;
	$->out[0]->begin(new Dim(1,&v));
	if ($->step > 0) {
		for (x=$->from; x<$->to; x+=$->step) $->out[0]->send(1,&x);
	} else {
		for (x=$->from; x>$->to; x+=$->step) $->out[0]->send(1,&x);
	}
	$->out[0]->end();
}

METHOD(GridFor,_0_int) {
	$->from = INT(argv[0]);
	GridFor__0_bang($,rself,argc,argv);
}

GRCLASS(GridFor,"@for",inlets:3,outlets:1,
LIST(),
	DECL(GridFor,init),
	DECL(GridFor,_0_bang),
	DECL(GridFor,_0_int),
	DECL(GridFor,_0_set),
	DECL(GridFor,_1_int),
	DECL(GridFor,_2_int))

/* **************************************************************** */

struct GridDim : GridObject {
};

GRID_BEGIN(GridDim,0) {
	int n = in->dim->count();
	$->out[0]->begin(new Dim(1,&n));
	$->out[0]->send(n,(Number *)in->dim->v);
	$->out[0]->end();
}

GRID_FLOW(GridDim,0) {}

GRID_END(GridDim,0) {}

GRCLASS(GridDim,"@dim",inlets:1,outlets:1,
LIST(GRINLET(GridDim,0)))

/* **************************************************************** */

struct GridRedim : GridObject {
	Dim *dim;
	Number *data; /* data not always used. so not using Grid here */
};

GRID_BEGIN(GridRedim,0) {
	FREE($->data);
	int a = in->dim->prod(), b = $->dim->prod();
	if (a==0) {
		//!@#$wrong
		$->data = NEW2(Number,1);
		$->data[0] = 0;
	} else if (a<b) {
		$->data = NEW2(Number,a);
	}
	$->out[0]->begin($->dim->dup());
}

GRID_FLOW(GridRedim,0) {
	int i = in->dex;
	if ($->data) {
		int a = in->dim->prod();
		int n2 = min(n,a-i);
		memcpy(&$->data[i],data,n2*sizeof(Number));
		if (n2>0) $->out[0]->send(n2,data);
	} else {
		int b = $->dim->prod();
		int n2 = min(n,b-i);
		if (n2>0) $->out[0]->send(n2,data);
		/* discard other values if any */
	}
}

GRID_END(GridRedim,0) {
	if ($->data) {
		int a = in->dim->prod(), b = $->dim->prod();
		int i = a;
		while (i<b) {
			$->out[0]->send(min(a,b-i),$->data);
			i += a;
		}
	}
	$->out[0]->end();
	FREE($->data);
}

/* same inlet 1 as @import */

GRID_BEGIN(GridRedim,1) {
	expect_dim_dim_list(in->dim);
	if (in->dim->n) in->set_factor(in->dim->n);
}

GRID_FLOW(GridRedim,1) {
	FREE($->dim);
	$->dim = new Dim(n,(int *)data);
	$->in[0]->abort();
	$->out[0]->abort();
}

GRID_END(GridRedim,1) {}

METHOD(GridRedim,init) {
	rb_call_super(argc,argv);
	if (argc!=1) RAISE("wrong number of args");
	Grid t;
	t.init_from_ruby(argv[0]);
	expect_dim_dim_list(t.dim);
	$->dim = new Dim(t.dim->prod(),(int *)t.as_int32());
	$->data = 0;
}

METHOD(GridRedim,delete) {
	FREE($->dim);
	FREE($->data);
	rb_call_super(argc,argv);
}

GRCLASS(GridRedim,"@redim",inlets:2,outlets:1,
LIST(GRINLET(GridRedim,0),GRINLET(GridRedim,1)),
	DECL(GridRedim,init),
	DECL(GridRedim,delete))

/* ---------------------------------------------------------------- */

/* "@scale_by" does quick scaling of pictures by integer factors */
struct GridScaleBy : GridObject {
	int rint; /* integer scale factor (r as in right inlet, which does not exist yet) */
};

/* processing a grid coming from inlet 0 */
/* one needs three special methods for that; they are declared using macros */
/* this one processes the header and accepts or rejects the grid */
GRID_BEGIN(GridScaleBy,0) {
	int scale = $->rint;
	Dim *a = in->dim;

	/* there are restrictions on grid sizes for efficiency reasons */
	if (a->count()!=3) RAISE("(height,width,chans) please");
	if (a->get(2)!=3) RAISE("3 chans please");

	/* computing the output's size */
	int v[3]={ a->get(0)*scale, a->get(1)*scale, a->get(2) };
	$->out[0]->begin(new Dim(3,v));

	/* configuring the input format */
	in->set_factor(a->get(1)*a->get(2));
}

/* this method processes one packet of grid content */
GRID_FLOW(GridScaleBy,0) {
	int scale = $->rint;
	int rowsize = in->dim->prod(1);

	/* for every picture row in this packet... */
	for (; n>0; data+=rowsize, n-=rowsize) {

		/* scale the line x-wise */
		Number buf[rowsize*scale];
		int p=0;
		for (int i=0; i<rowsize; i+=3) {
			for (int k=0; k<scale; k++) {
				buf[p+0]=data[i+0];
				buf[p+1]=data[i+1];
				buf[p+2]=data[i+2];
				p+=3;
			}
		}

		/* send the x-scaled line several times (thus y-scaling) */
		for (int j=0; j<scale; j++) {
			$->out[0]->send(rowsize*scale,buf);
		}
	}
}

/* not much to do here: when the input is done, the output is done too */
GRID_END(GridScaleBy,0) { $->out[0]->end(); }

/* the constructor accepts a scale factor as an argument */
/* that argument is not modifiable through an inlet yet (that would be the right inlet) */
METHOD(GridScaleBy,init) {
	$->rint = argc<1 ? 2 : INT(argv[0]);
	rb_call_super(argc,argv);
	$->out[0] = new GridOutlet((GridObject *)$, 0); // wtf?
}

/* there's one inlet, one outlet, and two system methods (inlet #-1) */
GRCLASS(GridScaleBy,"@scale_by",inlets:1,outlets:1,
LIST(GRINLET(GridScaleBy,0)),
	DECL(GridScaleBy,init))

/* **************************************************************** */

struct GridRGBtoHSV : GridObject {
};

GRID_BEGIN(GridRGBtoHSV,0) {
	if (in->dim->count()<1) RAISE("at least 1 dim please");
	if (in->dim->get(in->dim->count()-1)!=3) RAISE("3 chans please");
	$->out[0]->begin(in->dim->dup());
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
	GridOutlet *out = $->out[0];
	Number *buf = NEW(Number,n), *buf2=buf;
	for (int i=0; i<n; i+=3, buf+=3, data+=3) {
		int r=data[0], g=data[1], b=data[2];
		int v=buf[2]=max(max(r,g),b);
		int m=min(min(r,g),b);
		int d=(v-m)?(v-m):1;
		buf[1]=255*(v-m)/(v?v:1);
		buf[0] = 
			b==m ? 42*1+(g-r)*42/d :
			r==m ? 42*3+(b-g)*42/d :
			g==m ? 42*5+(r-b)*42/d : 0;
	}
	in->dex += n;
	out->give(n,buf2);
}

GRID_END(GridRGBtoHSV,0) {
	$->out[0]->end();
}

METHOD(GridRGBtoHSV,init) {
	rb_call_super(argc,argv);
	$->out[0] = new GridOutlet((GridObject *)$, 0);
}

GRCLASS(GridRGBtoHSV,"@rgb_to_hsv",inlets:1,outlets:1,
LIST(GRINLET(GridRGBtoHSV,0)),
	DECL(GridRGBtoHSV,init))

/* **************************************************************** */

typedef struct GridHSVtoRGB : GridObject {
};

GRID_BEGIN(GridHSVtoRGB,0) {
	if (in->dim->count()<1) RAISE("at least 1 dim please");
	if (in->dim->get(in->dim->count()-1)!=3) RAISE("3 chans please");
	$->out[0]->begin(in->dim->dup());
	in->set_factor(3);
}

GRID_FLOW(GridHSVtoRGB,0) {
	GridOutlet *out = $->out[0];
	Number *buf = NEW(Number,n), *buf2 = buf;
	for (int i=0; i<n; i+=3, buf+=3, data+=3) {
		int h=mod(data[0],252), s=data[1], v=data[2];
		int j=h%42;
		int k=h/42;
		int m=(255-s)*v/255;
		int d=s*v/255;
		buf[0]=(k==4?j:k==5||k==0?42:k==1?42-j:0)*d/42+m;
		buf[1]=(k==0?j:k==1||k==2?42:k==3?42-j:0)*d/42+m;
		buf[2]=(k==2?j:k==3||k==4?42:k==5?42-j:0)*d/42+m;
	}
	in->dex += n;
	out->give(n,buf2);
}

GRID_END(GridHSVtoRGB,0) { $->out[0]->end(); }

METHOD(GridHSVtoRGB,init) {
	rb_call_super(argc,argv);
	$->out[0] = new GridOutlet((GridObject *)$, 0);
}

GRCLASS(GridHSVtoRGB,"@hsv_to_rgb",inlets:1,outlets:1,
LIST(GRINLET(GridHSVtoRGB,0)),
	DECL(GridHSVtoRGB,init))

/* **************************************************************** */
/* [rtmetro] */

struct RtMetro : GridObject {
	int ms; /* millisecond time interval */
	bool on;
	uint64 next_time; /* microseconds since epoch: next time an event occurs */
	uint64 last;      /* microseconds since epoch: last time we checked */
	int mode; /* 0=equal; 1=geiger */
};

uint64 RtMetro_now(void) {
	struct timeval nowtv;
	gettimeofday(&nowtv,0);
	return nowtv.tv_sec * 1000000LL + nowtv.tv_usec;
}

static double drand() {
	return 1.0*rand()/(RAND_MAX+1.0);
}

static uint64 RtMetro_delay(VALUE rself) {
	DGS(RtMetro);
	switch ($->mode) {
		case 0:
			return 1000LL*$->ms;
		case 1:
			/* Bangs in a geiger counter have a l*dt probability of occuring
			in a dt time interval. Therefore the time till the next event
			follows a Exp(l) law, for which:
			f(t) = l*exp(-l*t)
			F(t) = integral of l*exp(-l*t)*dt = 1-exp(-l*t)
			F^-1(p) = -log(1-p)/l
			*/
			return (uint64)(1000LL*$->ms*-log(1-drand()));
		default:
			abort();
	}
}

static void RtMetro_alarm(VALUE rself) {
	uint64 now = RtMetro_now();
	unsigned int *r = (unsigned int*)rself;
//	fprintf(stderr,"%08x\n",(unsigned int)r);
//	fprintf(stderr,"%08x %08x %08x %08x %08x\n",r[0],r[1],r[2],r[3],r[4]);
	DGS(RtMetro);
	//gfpost("rtmetro alarm tick: %lld; next_time: %lld; now-last: %lld",now,$->next_time,now-$->last);
	if (now >= $->next_time) {
		//gfpost("rtmetro sending bang");
		VALUE a[] = { INT2NUM(0), sym_bang };
		FObject_send_out(COUNT(a),a,rself);
		/* $->next_time = now; */ /* jmax style, less realtime */
		$->next_time += RtMetro_delay(rself);
	}
	$->last = now;
}

METHOD(RtMetro,_0_int) {
	int oon = $->on;
	$->on = !! FIX2INT(argv[0]);
	gfpost("on = %d",$->on);
	if (oon && !$->on) {
		gfpost("deleting rtmetro alarm for $=%08x rself=%08x",(long)$,(long)rself);
		MainLoop_remove($);
	} else if (!oon && $->on) {
		gfpost("creating rtmetro alarm for $=%08x rself=%08x",(long)$,(long)rself);
//		MainLoop_add($,(void(*)(void*))RtMetro_alarm);
		MainLoop_add((void *)rself,(void(*)(void*))RtMetro_alarm);
		$->next_time = RtMetro_now();
	}
}

METHOD(RtMetro,_1_int) {
	$->ms = FIX2INT(argv[0]);
	gfpost("ms = %d",$->ms);
}

METHOD(RtMetro,init) {
	rb_call_super(argc,argv);
	$->ms = FIX2INT(argv[0]);
	$->on = 0;
	$->mode = 0;
	if (argc>=2) {
		if (argv[1]==SYM(equal)) $->mode=0;
		else if (argv[1]==SYM(geiger)) $->mode=1;
		else RAISE("this is not a known mode");
	}		
	gfpost("ms = %d",$->ms);
	gfpost("on = %d",$->on);
	gfpost("mode = %d",$->mode);
}

GRCLASS(RtMetro,"rtmetro",inlets:2,outlets:1,
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

GRCLASS(GridGlobal,"@global",inlets:1,outlets:1,
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
//	INSTALL(GridRGBtoHSV); /* buggy */
//	INSTALL(GridHSVtoRGB); /* buggy */
	INSTALL(RtMetro);
	INSTALL(GridGlobal);
}
