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

/* **************************************************************** */
/*
  GridImport ("@import") is the class for converting a old-style stream
  of integers to a streamed grid as used now.
*/

typedef struct GridImport {
	GridObject_FIELDS;
	Dim *dim; /* size of grids to send */
} GridImport;

GRID_BEGIN(GridImport,0) { return true; }
GRID_FLOW(GridImport,0) {
	GridOutlet *out = $->out[0];
	while (n) {
		int n2;
		if (!GridOutlet_busy(out)) GridOutlet_begin(out,Dim_dup($->dim));
		n2 = Dim_prod(out->dim) - out->dex;
		if (n2 > n) n2 = n;
		GridOutlet_send(out,n,data);
		if (out->dex >= Dim_prod(out->dim)) GridOutlet_end(out);
		n -= n2;
		data += n2;
	}
}
GRID_END(GridImport,0) {}

/* same inlet 1 as @redim */

GRID_BEGIN(GridImport,1) {
	int n;
	if (Dim_count(in->dim)!=1) RAISE("expected 1 dim");
	n = Dim_get(in->dim,0);
	if (n>MAX_DIMENSIONS) RAISE("too big");
	GridInlet_set_factor(in,n);
	return true;
}

GRID_FLOW(GridImport,1) {
	int i;
	int dim[n];
	FREE($->dim);
	for(i=0;i<n;i++) {
		dim[i]=data[i];
		if (dim[i]<1 || dim[i]>MAX_INDICES) RAISE("dim[%d]=%d is out of range",i,dim[i]);
	}
	$->dim = Dim_new(n,dim);
	GridInlet_abort($->in[0]);
	GridOutlet_abort($->out[0]);
}

GRID_END(GridImport,1) {}

METHOD(GridImport,init) {
	int i;
	int dim[argc];
	rb_call_super(argc,argv);

	for (i=0; i<argc; i++) {
		dim[i] = INT(argv[i]);
		if (dim[i]<1 || dim[i]>MAX_INDICES) RAISE("dim[%d]=%d is out of range",i,dim[i]);
	}
	$->dim = Dim_new(argc,dim);
}

METHOD(GridImport,delete) {
	FREE($->dim);
	rb_call_super(argc,argv);
}

METHOD(GridImport,_0_int) {
	GridOutlet *out = $->out[0];
	Number data[] = { INT(argv[0]) };
	if (!GridOutlet_busy(out)) GridOutlet_begin(out,Dim_dup($->dim));
	GridOutlet_send(out,COUNT(data),data);
	if (out->dex >= Dim_prod(out->dim)) GridOutlet_end(out);
}

METHOD(GridImport,_0_reset) {
	GridOutlet *out = $->out[0];
	if (GridOutlet_busy(out)) GridOutlet_abort(out);
}

GRCLASS(GridImport,inlets:2,outlets:1,
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

typedef struct GridExport {
	GridObject_FIELDS;
} GridExport;

GRID_BEGIN(GridExport,0) { return true; }

GRID_FLOW(GridExport,0) {
	int i;
	for (i=0; i<n; i++) {
		VALUE a[] = { INT2NUM(0), sym_int, INT2NUM(*data) };
		FObject_send_out(COUNT(a),a,rself);
		data++;
	}
}

GRID_END(GridExport,0) {}

METHOD(GridExport,init)   { rb_call_super(argc,argv); }
METHOD(GridExport,delete) { rb_call_super(argc,argv); }

GRCLASS(GridExport,inlets:1,outlets:1,
LIST(GRINLET(GridExport,0)),
/* outlet 0 not used for grids */
	DECL(GridExport,init),
	DECL(GridExport,delete))

/* **************************************************************** */

typedef struct GridExportList {
	GridObject_FIELDS;
	VALUE /*Array*/ list; //!@#$mark
	int n;
} GridExportList;

GRID_BEGIN(GridExportList,0) {
	int n = Dim_prod(in->dim);
	if (n>1000) RAISE("list too big (%d elements)", n);
	$->list = rb_ary_new();
	$->n = n;
	return true;
}

GRID_FLOW(GridExportList,0) {
	int i;
	for (i=0; i<n; i++, data++)
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

METHOD(GridExportList,init)   { rb_call_super(argc,argv); }
METHOD(GridExportList,delete) { rb_call_super(argc,argv); }

GRCLASS(GridExportList,inlets:1,outlets:1,
LIST(GRINLET(GridExportList,0)),
/* outlet 0 not used for grids */
	DECL(GridExportList,init),
	DECL(GridExportList,delete))

/* **************************************************************** */
/*
  grid_store ("@store") is the class for storing a grid and restituting
  it on demand. The right inlet receives the grid. The left inlet receives
  either a bang (which forwards the whole image) or a grid describing what to
  send.
*/

typedef struct GridStore {
	GridObject_FIELDS;
	int nt; /* number type */
	void *data;
	Dim *dim;
} GridStore;

GRID_BEGIN(GridStore,0) {
	int na = Dim_count(in->dim);
	int nb,nc,nd,i;
	int v[MAX_DIMENSIONS];
	if (!$->dim) RAISE("empty buffer, better luck next time.");

	nb = Dim_count($->dim);

	if (na<1) RAISE("must have at least 1 dimension.",na,1,1+nb);

	nc = Dim_get(in->dim,na-1);
	if (nc > nb)
		RAISE("wrong number of elements in last dimension: "
			"got %d, expecting <= %d", nc, nb);
	nd = nb - nc + na - 1;
	if (nd > MAX_DIMENSIONS) RAISE("too many dimensions!");
	for (i=0; i<na-1; i++) v[i] = Dim_get(in->dim,i);
	for (i=nc; i<nb; i++) v[na-1+i-nc] = Dim_get($->dim,i);
	GridOutlet_begin($->out[0],Dim_new(nd,v));
	GridInlet_set_factor(in,nc);
	return true;
}

GRID_FLOW(GridStore,0) {
	int na = Dim_count(in->dim);
	int nb = Dim_count($->dim);
	int nc = Dim_get(in->dim,na-1);
	int size = Dim_prod_start($->dim,nc);
	int i;
	int v[nb];
	assert((n % nc) == 0);
	assert($->data);

	while (n>0) {
		int pos;
		for (i=0; i<nc; i++,data++) v[i] = mod(*data,$->dim->v[i]);
		while (i<nb) v[i++] = 0;
		pos = Dim_calc_dex($->dim,v);
		switch ($->nt) {
		case int32_type_i: {
			Number *data2 = ((Number *)$->data) + pos;
			GridOutlet_send($->out[0],size,data2);
		 	break;}
		case uint8_type_i: {
			int bs = gf_max_packet_length;
			Number data3[bs];
			uint8 *data2 = ((uint8 *)$->data) + pos;
			int left = size;
			while (left>=bs) {
				for (i=0; i<bs; i++) { data3[i] = data2[i]; }
				GridOutlet_send($->out[0],bs,data3);
				data2+=bs;
				left-=bs;
			}
			while (left) {
				data3[0] = *data2++;
				GridOutlet_send($->out[0],1,data3);
				left--;
			}
			break;}
		default: assert(0);
		}
		n -= nc;
	}
}

GRID_END(GridStore,0) { GridOutlet_end($->out[0]); }

GRID_BEGIN(GridStore,1) {
	int length = Dim_prod(in->dim);
	GridInlet_abort($->in[0]);
	FREE($->dim);
	FREE($->data);
	$->dim = Dim_dup(in->dim);
	switch ($->nt) {
	case int32_type_i: $->data = NEW2(Number,length); break;
	case uint8_type_i: $->data = NEW2(uint8,length); break;
	default: assert(0);
	}
	return true;
}

GRID_FLOW(GridStore,1) {
	int i;
	switch ($->nt) {
	case int32_type_i:{
		Number *data2 = (Number *)$->data + in->dex;
		memcpy(data2, data, n*sizeof(Number)); break;}
	case uint8_type_i:{
		uint8 *data2 = (uint8 *)$->data + in->dex;
		for(i=0; i<n; i++) data2[i] = data[i];
		break;}
	default: assert(0);
	}
	in->dex += n;
}

GRID_END(GridStore,1) {}

METHOD(GridStore,init) {
	VALUE t = argc==0 ? SYM(int32) : argv[0];
	rb_call_super(argc,argv);
	$->data = 0;
	$->dim  = 0;
	if (t==SYM(int32)) {
		$->nt = int32_type_i;
	} else if (t==SYM(uint8)) {
		$->nt = uint8_type_i;
	} else {
		RAISE("unknown element type \"%s\"", t);
	}
}

METHOD(GridStore,delete) {
	FREE($->data);
	FREE($->dim);
	rb_call_super(argc,argv);
}

METHOD(GridStore,_0_bang) {
	if (! $->dim || ! $->data) {
		whine("empty buffer, better luck next time.");
		return;
	}
	GridOutlet_begin($->out[0],Dim_dup($->dim));
	GridOutlet_send( $->out[0],Dim_prod($->dim),$->data);
	GridOutlet_end(  $->out[0]);
}

GRCLASS(GridStore,inlets:2,outlets:1,
LIST(GRINLET(GridStore,0),GRINLET(GridStore,1)),
	DECL(GridStore,init),
	DECL(GridStore,delete),
	DECL(GridStore,_0_bang))

/* **************************************************************** */

typedef struct GridOp1 {
	GridObject_FIELDS;
	const Operator1 *op;
} GridOp1;

GRID_BEGIN(GridOp1,0) {
	GridOutlet_begin($->out[0],Dim_dup(in->dim));
	in->dex = 0;
	return true;
}

GRID_FLOW(GridOp1,0) {
	Number *data2 = NEW2(Number,n);
	GridOutlet *out = $->out[0];
	memcpy(data2,data,n*sizeof(Number));
	$->op->op_array(n,data2);
	in->dex += n;
	GridOutlet_send(out,n,data2);
	FREE(data2);
}

GRID_END(GridOp1,0) { GridOutlet_end($->out[0]); }

METHOD(GridOp1,init) {
	rb_call_super(argc,argv);
	$->op = OP1(argv[0]);
}

METHOD(GridOp1,delete) { rb_call_super(argc,argv); }

GRCLASS(GridOp1,inlets:1,outlets:1,
LIST(GRINLET(GridOp1,0)),
	DECL(GridOp1,init),
	DECL(GridOp1,delete))

/* **************************************************************** */
/*
  GridOp2 ("@") is the class of objects for parallel operation on the
  values of the left grid with values of a (stored) right grid or (stored)
  single value.
*/

typedef struct GridOp2 {
	GridObject_FIELDS;
	const Operator2 *op;
	int rint;
	Number *data;
	Dim *dim;
} GridOp2;

GRID_BEGIN(GridOp2,0) {
	GridOutlet_begin($->out[0],Dim_dup(in->dim));
	in->dex = 0;
	return true;
}

GRID_FLOW2(GridOp2,0) {
	GridOutlet *out = $->out[0];
	if ($->dim) {
		int loop = Dim_prod($->dim);
		if (in->dex+n <= loop) {
			$->op->op_array2(n,data,$->data+in->dex);
		} else {
			int i;
			Number data2[n];
			for (i=0; i<n; i++) {
				data2[i] = $->data[(in->dex+i)%loop];
			}
			$->op->op_array2(n,data,data2);
		}
	} else {
		$->op->op_array(n,data,$->rint);
	}
	in->dex += n;
	GridOutlet_give(out,n,data);
}

GRID_END(GridOp2,0) { GridOutlet_end($->out[0]); }

GRID_BEGIN(GridOp2,1) {
	int length = Dim_prod(in->dim);
	if ($->data) GridInlet_abort($->in[0]); /* bug? */
	FREE($->data);
	FREE($->dim);
	$->dim = Dim_dup(in->dim);
	$->data = NEW2(Number,length);
	return true;
}

GRID_FLOW(GridOp2,1) {
	memcpy(&$->data[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridOp2,1) {}

METHOD(GridOp2,init) {
	rb_call_super(argc,argv);
	$->op = OP2(argv[0]);
	$->rint = argc<2 ? 0 : INT(argv[1]);
	$->data = 0;
	$->dim = 0;
}

METHOD(GridOp2,delete) {
	FREE($->data);
	FREE($->dim);
	rb_call_super(argc,argv);
}

METHOD(GridOp2,_1_int) {
	FREE($->data);
	if ($->dim) FREE($->dim);
	$->rint = INT(argv[0]);
}

GRCLASS(GridOp2,inlets:2,outlets:1,
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

typedef struct GridFold {
	GridObject_FIELDS;
	const Operator2 *op;
	int rint;
} GridFold;

GRID_BEGIN(GridFold,0) {
	int n = Dim_count(in->dim);
	if (n<1) RAISE("minimum 1 dimension");
	{
		Dim *foo = Dim_new(n-1,in->dim->v);
		GridOutlet_begin($->out[0],foo);
	}
	GridInlet_set_factor(in,Dim_get(in->dim,Dim_count(in->dim)-1));
	return true;
}

GRID_FLOW(GridFold,0) {
	int factor = Dim_get(in->dim,Dim_count(in->dim)-1);
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
	GridOutlet_send(out,bs,buf);
}

GRID_END(GridFold,0) { GridOutlet_end($->out[0]); }

METHOD(GridFold,init) {
	rb_call_super(argc,argv);
	$->op = OP2(argv[0]);
	$->rint = argc<2 ? 0 : INT(argv[1]);
}

METHOD(GridFold,delete) { rb_call_super(argc,argv); }

METHOD(GridFold,_1_int) {
	$->rint = INT(argv[0]);
}

GRCLASS(GridFold,inlets:2,outlets:1,
LIST(GRINLET(GridFold,0)),
	DECL(GridFold,init),
	DECL(GridFold,delete),
	DECL(GridFold,_1_int))

/* **************************************************************** */
typedef struct GridInner {
	GridObject_FIELDS;
	Dim *dim;
	Number *data;
	Number rint;
	const Operator2 *op_para;
	const Operator2 *op_fold;
} GridInner;

/*
GRID_BEGIN(GridInner,0) {
	Dim *a = in->dim;
	Dim *b = $->dim;
	if (!b) RAISE("right inlet has no grid");
	if (Dim_count(a)<1) RAISE("minimum 1 dimension");
	{
		int a_last = Dim_get(a,Dim_count(a)-1);
		int b_last = Dim_get(b,Dim_count(b)-1);
		int n = Dim_count(a)+Dim_count(b)-2;
		int v[n];
		int i,j;
		if (a_last != b_last)
			RAISE("last dimension of each grid must have same size");
		for (i=j=0; j<Dim_count(a)-1; i++,j++) { v[i] = Dim_get(a,j); }
		for (  j=0; j<Dim_count(b)-1; i++,j++) { v[i] = Dim_get(b,j); }
		GridOutlet_begin($->out[0],Dim_new(n,v));
		GridInlet_set_factor(in,a_last);
	}	
	return true;
}

GRID_FLOW(GridInner,0) {
	GridOutlet *out = $->out[0];
	int factor = Dim_get(in->dim,Dim_count(in->dim)-1);
	int i,j;
	int b_prod = Dim_prod($->dim);
	Number *buf2 = NEW(Number,b_prod/factor);
	Number *buf = NEW(Number,n/factor);
	assert (n % factor == 0);

	for (i=0; i<n; i+=factor) {
		for (j=0; j<b_prod; j+=factor) {
			memcpy(buf,&data[i],factor*sizeof(Number));
			$->op_para->op_array2(factor,buf,&$->data[j]);
			buf2[j/factor] = $->op_fold->op_fold($->rint,factor,buf);
		}
		GridOutlet_send(out,b_prod/factor,buf2);
	}
	FREE(buf);
	FREE(buf2);
}

GRID_END(GridInner,0) { GridOutlet_end($->out[0]); }

GRID_BEGIN(GridInner,2) {
	int length = Dim_prod(in->dim);
	GridInlet_abort($->in[0]);
	if (Dim_count(in->dim)<1) RAISE("minimum 1 dimension");
	FREE($->dim);
	FREE($->data);
	$->dim = Dim_dup(in->dim);
	$->data = NEW2(Number,length);
	return true;
}

GRID_FLOW(GridInner,2) {
	memcpy(&$->data[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridInner,2) {}

METHOD(GridInner,init) {
	VALUE sym_para = argv[0];
	VALUE sym_fold = argv[1];
	$->rint = INT(argv[2]);
	$->dim = 0;
	$->data = 0;

	rb_call_super(argc,argv);

	$->op_para = Dict_get(op2_dict,(void *)sym_para);
	$->op_fold = Dict_get(op2_dict,(void *)sym_fold);
	if (!$->op_para) RAISE2("unknown binary operator \"%s\"", sym_para);
	if (!$->op_fold) RAISE2("unknown binary operator \"%s\"", sym_fold);
}

METHOD(GridInner,delete) {
	FREE($->dim);
	FREE($->data);
	rb_call_super(argc,argv);
}

GRCLASS(GridInner,inlets:3,outlets:1,
LIST(GRINLET(GridInner,0),GRINLET(GridInner,2)),
	DECL(GridInner,init),
	DECL(GridInner,delete))
*/
/* **************************************************************** */

typedef struct GridInner GridInner2;

GRID_BEGIN(GridInner2,0) {
	Dim *a = in->dim;
	Dim *b = $->dim;
	if (!b) RAISE("right inlet has no grid");
	if (Dim_count(a)<1) RAISE("minimum 1 dimension");
	{
		int a_last = Dim_get(a,Dim_count(a)-1);
		int b_last = Dim_get(b,Dim_count(b)-1);
		int n = Dim_count(a)+Dim_count(b)-2;
		int v[n];
		int i,j;
		if (a_last != b_last)
			RAISE("last dimension of each grid must have same size");
		for (i=j=0; j<Dim_count(a)-1; i++,j++) { v[i] = Dim_get(a,j); }
		for (  j=0; j<Dim_count(b)-1; i++,j++) { v[i] = Dim_get(b,j); }
		GridOutlet_begin($->out[0],Dim_new(n,v));
		GridInlet_set_factor(in,a_last);
	}	
	return true;
}

GRID_FLOW(GridInner2,0) {
	GridOutlet *out = $->out[0];
	int factor = Dim_get(in->dim,Dim_count(in->dim)-1);
	int i,j;
	int b_prod = Dim_prod($->dim);
	Number *buf2 = NEW(Number,b_prod/factor);
	Number *buf = NEW(Number,n/factor);
	assert (n % factor == 0);

	for (i=0; i<n; i+=factor) {
		for (j=0; j<b_prod; j+=factor) {
			memcpy(buf,&data[i],factor*sizeof(Number));
			$->op_para->op_array2(factor,buf,&$->data[j]);
			buf2[j/factor] = $->op_fold->op_fold($->rint,factor,buf);
		}
		GridOutlet_send(out,b_prod/factor,buf2);
	}
	FREE(buf);
	FREE(buf2);
}

GRID_END(GridInner2,0) { GridOutlet_end($->out[0]); }

GRID_BEGIN(GridInner2,2) {
	int length = Dim_prod(in->dim);
	GridInlet_abort($->in[0]);
	if (Dim_count(in->dim)<1) RAISE("minimum 1 dimension");
	FREE($->dim);
	FREE($->data);
	$->dim = Dim_dup(in->dim);
	$->data = NEW2(Number,length);
	return true;
}

GRID_FLOW(GridInner2,2) {
	memcpy(&$->data[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridInner2,2) {}

METHOD(GridInner2,init) {
	rb_call_super(argc,argv);
	$->op_para = argc<1 ? OP2(SYM(*)) : OP2(argv[0]);
	$->op_fold = argc<2 ? OP2(SYM(+)) : OP2(argv[1]);
	$->rint = argc<3 ? 0 : INT(argv[2]);
	$->dim = 0;
	$->data = 0;
}

METHOD(GridInner2,delete) {
	FREE($->dim);
	FREE($->data);
	rb_call_super(argc,argv);
}

GRCLASS(GridInner2,inlets:3,outlets:1,
LIST(GRINLET(GridInner2,0),GRINLET(GridInner2,2)),
	DECL(GridInner2,init),
	DECL(GridInner2,delete))

/* **************************************************************** */

typedef struct GridOuter {
	GridObject_FIELDS;
	Dim *dim;
	Number *data;
	const Operator2 *op;
} GridOuter;

GRID_BEGIN(GridOuter,0) {
	Dim *a = in->dim;
	Dim *b = $->dim;
	if (!b) RAISE("right inlet has no grid");
	{
		int n = Dim_count(a)+Dim_count(b);
		int v[n];
		int i,j;
		for (i=j=0; j<Dim_count(a); i++,j++) { v[i] = Dim_get(a,j); }
		for (  j=0; j<Dim_count(b); i++,j++) { v[i] = Dim_get(b,j); }
		GridOutlet_begin($->out[0],Dim_new(n,v));
	}
	return true;
}

GRID_FLOW(GridOuter,0) {
	int b_prod = Dim_prod($->dim);
	Number *buf = NEW(Number,b_prod);
	while (n) {
		int j;
		for (j=0; j<b_prod; j++) buf[j] = *data;
		$->op->op_array2(b_prod,buf,$->data);
		GridOutlet_send($->out[0],b_prod,buf);
		data++; n--;
	}
	FREE(buf);
}

GRID_END(GridOuter,0) { GridOutlet_end($->out[0]); }

GRID_BEGIN(GridOuter,1) {
	int length = Dim_prod(in->dim);
	GridInlet_abort($->in[0]);
	FREE($->dim);
	FREE($->data);
	$->dim = Dim_dup(in->dim);
	$->data = NEW2(Number,length);
	return true;
}

GRID_FLOW(GridOuter,1) {
	memcpy(&$->data[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridOuter,1) {}

METHOD(GridOuter,init) {
	rb_call_super(argc,argv);
	$->op = OP2(argv[0]);
	$->dim = 0;
	$->data = 0;
}

METHOD(GridOuter,delete) {
	FREE($->dim);
	FREE($->data);
	rb_call_super(argc,argv);
}

GRCLASS(GridOuter,inlets:2,outlets:1,
LIST(GRINLET(GridOuter,0),GRINLET(GridOuter,1)),
	DECL(GridOuter,init),
	DECL(GridOuter,delete))

/* **************************************************************** */

typedef struct GridConvolve {
	GridObject_FIELDS;
	Dim *dim;  Number *data;
	Dim *diml; Number *datal;
	const Operator2 *op_para;
	const Operator2 *op_fold;
	Number rint;
} GridConvolve;

GRID_BEGIN(GridConvolve,0) {
	Dim *a = in->dim;
	Dim *b = $->dim;
	if (!b) RAISE("right inlet has no grid");
	if (Dim_count(a) < Dim_count(b))
		RAISE("left grid has less dimensions than right grid");
	if (Dim_count(a) < 2)
		RAISE("left grid has less than two dimensions");
	
	{
		int v[MAX_DIMENSIONS];
		int i;
		for (i=0; i<Dim_count(a); i++) v[i]=a->v[i];
		v[0] += Dim_get($->dim,0)/2*2;
		v[1] += Dim_get($->dim,1)/2*2;
		$->diml = Dim_new(Dim_count(a),v);
	}
	$->datal = NEW2(Number,Dim_prod($->diml));
	GridOutlet_begin($->out[0],Dim_dup(in->dim));
	GridInlet_set_factor(in,Dim_prod_start(a,1));
	return true;
}

GRID_FLOW(GridConvolve,0) {
	int my = Dim_get($->dim,0), ny = Dim_get($->diml,0) - my/2*2;
	int mx = Dim_get($->dim,1);
	int oy = Dim_prod_start($->diml,1) * (my/2);
	int ox = Dim_prod_start($->diml,2) * (mx/2);
	int factor = in->factor;
	int ll = Dim_prod_start($->diml,1);
	int l = Dim_prod_start($->diml,2);
	
	while (n) {
		int i = in->dex / factor;
		Number *p = $->datal+oy+i*ll;
		memcpy(p+ox,data,factor*sizeof(Number));
		memcpy(p+ox+factor,data,(mx/2)*l*sizeof(Number));
		memcpy(p,data+factor-ox,(mx/2)*l*sizeof(Number));
		if (i<    my/2) memcpy(p+ny*ll,p,ll*sizeof(Number));
		if (i>=ny-my/2) memcpy(p-ny*ll,p,ll*sizeof(Number));
		in->dex += factor;
		data += factor;
		n -= factor;
	}
}

GRID_END(GridConvolve,0) {
	int iy,ix,jy,jx,k;
	int my = Dim_get($->dim,0), ny = Dim_get($->diml,0) - my/2*2;
	int mx = Dim_get($->dim,1), nx = Dim_get($->diml,1) - mx/2*2;
	int l  = Dim_prod_start($->diml,2);
	int ll = Dim_prod_start($->diml,1);
	Number buf[l],buf2[l];
	Number as[l];

	for (k=0; k<l; k++) buf2[k]=$->rint;

	for (iy=0; iy<ny; iy++) {
		for (ix=0; ix<nx; ix++) {
			Number *d = $->data;
			Number *p = $->datal + iy*ll + ix*l;
			memcpy(buf,buf2,l*sizeof(Number));
			for (jy=my; jy; jy--,p+=ll-mx*l) {
				for (jx=mx; jx; jx--,p+=l) {
					Number b = *d++;
					memcpy(as,p,l*sizeof(Number));
					$->op_para->op_array(l,as,b);
					$->op_fold->op_array2(l,buf,as);
				}
			}
			GridOutlet_send($->out[0],l,buf);
		}
	}
	GridOutlet_end($->out[0]);
	FREE($->diml);
	FREE($->datal);
}

GRID_BEGIN(GridConvolve,1) {
	int length = Dim_prod(in->dim);
	int count = Dim_count(in->dim);
	if (count != 2) RAISE("only exactly two dimensions allowed for now");
	/* because odd * odd = odd */
	if ((length & 1) == 0) RAISE("even number of elements");
	GridInlet_abort($->in[0]);
	FREE($->dim);
	FREE($->data);
	$->dim = Dim_dup(in->dim);
	$->data = NEW2(Number,length);
	return true;
}

GRID_FLOW(GridConvolve,1) {
	memcpy(&$->data[in->dex], data, n*sizeof(Number));
}

GRID_END(GridConvolve,1) {}

METHOD(GridConvolve,init) {
	rb_call_super(argc,argv);
	$->op_para = OP2(argc<1 ? SYM(*) : argv[0]);
	$->op_fold = OP2(argc<2 ? SYM(*) : argv[1]);
	$->rint = argc<3 ? 0 : INT(argv[2]);
	$->dim = 0;
	$->data = 0;
	$->diml = 0;
	$->datal = 0;
}

METHOD(GridConvolve,delete) {
	FREE($->dim);  FREE($->diml);
	FREE($->data); FREE($->datal);
	rb_call_super(argc,argv);
}

GRCLASS(GridConvolve,inlets:2,outlets:1,
LIST(GRINLET(GridConvolve,0),GRINLET(GridConvolve,1)),
	DECL(GridConvolve,init),
	DECL(GridConvolve,delete))

/* **************************************************************** */

typedef struct GridFor {
	GridObject_FIELDS;
	Number from;
	Number to;
	Number step;
} GridFor;

METHOD(GridFor,init) {
	rb_call_super(argc,argv);
	$->from = INT(argv[0]);
	$->to   = INT(argv[1]);
	$->step = INT(argv[2]);
	if (!$->step) $->step=1;
}

METHOD(GridFor,delete) { rb_call_super(argc,argv); }

METHOD(GridFor,_0_set) { $->from = INT(argv[0]); }
METHOD(GridFor,_1_int) { $->to   = INT(argv[0]); }
METHOD(GridFor,_2_int) { $->step = INT(argv[0]); if (!$->step) $->step=1; }

METHOD(GridFor,_0_bang) {
	int v = ($->to - $->from + $->step - cmp($->step,0)) / $->step;
	Number x;
	if (v<0) v=0;
	GridOutlet_begin($->out[0],Dim_new(1,&v));
	if ($->step > 0) {
		for (x=$->from; x<$->to; x+=$->step) {
			GridOutlet_send($->out[0],1,&x);
		}
	} else {
		for (x=$->from; x>$->to; x+=$->step) {
			GridOutlet_send($->out[0],1,&x);
		}
	}
	GridOutlet_end($->out[0]);
}

METHOD(GridFor,_0_int) {
	$->from = INT(argv[0]);
	GridFor__0_bang($,rself,argc,argv);
}

GRCLASS(GridFor,inlets:3,outlets:1,
LIST(),
	DECL(GridFor,init),
	DECL(GridFor,delete),
	DECL(GridFor,_0_bang),
	DECL(GridFor,_0_int),
	DECL(GridFor,_0_set),
	DECL(GridFor,_1_int),
	DECL(GridFor,_2_int))

/* **************************************************************** */

typedef struct GridDim {
	GridObject_FIELDS;
} GridDim;

GRID_BEGIN(GridDim,0) {
	int n = Dim_count(in->dim);
	int i;
	Dim *foo = Dim_new(1,&n);
	Number v[n];
	GridOutlet_begin($->out[0],foo);
	for (i=0; i<n; i++) {
		v[i] = Dim_get(in->dim,i);
	}
	GridOutlet_send($->out[0],n,v);
	GridOutlet_end($->out[0]);
	return true;
}

GRID_FLOW(GridDim,0) {}

GRID_END(GridDim,0) {}

METHOD(GridDim,init) { rb_call_super(argc,argv); }

METHOD(GridDim,delete) { rb_call_super(argc,argv); }

GRCLASS(GridDim,inlets:1,outlets:1,
LIST(GRINLET(GridDim,0)),
	DECL(GridDim,init),
	DECL(GridDim,delete))

/* **************************************************************** */

typedef struct GridRedim {
	GridObject_FIELDS;
	Dim *dim;
	Number *data;
} GridRedim;

GRID_BEGIN(GridRedim,0) {
	int a = Dim_prod(in->dim);
	int b = Dim_prod($->dim);
	if (a==0) {
		/* int v=1; $->dim = Dim_new(1,&v); */
		$->data = NEW2(Number,1);
		$->data[0] = 0;
	} else if (a<b) {
		/* $->dim = Dim_dup(in->dim); */
		$->data = NEW2(Number,Dim_prod(in->dim));
	} else {
		/* nothing */
	}
	GridOutlet_begin($->out[0],Dim_dup($->dim));
	return true;
}

GRID_FLOW(GridRedim,0) {
	int a = Dim_prod(in->dim);
	int b = Dim_prod($->dim);
	int i = in->dex;
	if ($->data) {
		int n2 = min(n,a-i);
		memcpy(&$->data[i],data,n2*sizeof(Number));
		if (n2>0) GridOutlet_send($->out[0],n2,data);
	} else {
		int n2 = min(n,b-i);
		if (n2>0) GridOutlet_send($->out[0],n2,data);
		/* discard other values if any */
	}
}

GRID_END(GridRedim,0) {
	int a = Dim_prod(in->dim);
	int b = Dim_prod($->dim);
	int i = a;
	if ($->data) {
		while (i<b) {
			int n = min(a,b-i);
			GridOutlet_send($->out[0],n,$->data);
			i += n;
		}
	}
	GridOutlet_end($->out[0]);
	FREE($->data);
}

/* same inlet 1 as @import */

GRID_BEGIN(GridRedim,1) {
	int n;
	if (Dim_count(in->dim)!=1)
		RAISE("dimension list must have 1 dimension");
	n = Dim_get(in->dim,0);
	if (n>MAX_DIMENSIONS) RAISE("too many dimensions");
	if (n) GridInlet_set_factor(in,n);
	return true;
}

GRID_FLOW(GridRedim,1) {
	int i;
	int dim[n];
	FREE($->dim);
	for(i=0;i<n;i++) {
		dim[i]=data[i];
		if (dim[i]<1 || dim[i]>MAX_INDICES)
			RAISE("dim[%d]=%d is out of range",i,dim[i]);
	}
	$->dim = Dim_new(n,dim);
	GridInlet_abort($->in[0]);
	GridOutlet_abort($->out[0]);
}

GRID_END(GridRedim,1) {}

METHOD(GridRedim,init) {
	int i;
	int dim[argc];
	rb_call_super(argc,argv);

	for (i=0; i<argc; i++) {
		dim[i] = INT(argv[i]);
		if (dim[i]<1 || dim[i]>MAX_INDICES) RAISE("dim[%d]=%d is out of range",i,dim[i]);
	}
	$->dim = Dim_new(argc,dim);
	$->data = 0;
}

METHOD(GridRedim,delete) {
	FREE($->dim);
	FREE($->data);
	rb_call_super(argc,argv);
}

GRCLASS(GridRedim,inlets:2,outlets:1,
LIST(GRINLET(GridRedim,0),GRINLET(GridRedim,1)),
	DECL(GridRedim,init),
	DECL(GridRedim,delete))

/* **************************************************************** */

typedef struct GridPrint GridPrint;
struct GridPrint {
	GridObject_FIELDS;
};

GRID_BEGIN(GridPrint,0) {
	if (Dim_count(in->dim)>1) {
		char *s = Dim_to_s(in->dim);
		whine("Grid %s: %s",s);
		FREE(s);
	}
	return true;
}

GRID_FLOW(GridPrint,0) {
	if (Dim_count(in->dim)<=1) { /* write me */ }
	/* write me */
}

GRID_END(GridPrint,0) {}

METHOD(GridPrint,init)   { rb_call_super(argc,argv); }
METHOD(GridPrint,delete) { rb_call_super(argc,argv); }

GRCLASS(GridPrint,inlets:1,outlets:0,
LIST(GRINLET(GridPrint,0)),
	DECL(GridPrint,init),
	DECL(GridPrint,delete))

/* ---------------------------------------------------------------- */
/* some data/type decls */

typedef struct GridIn {
	GridObject_FIELDS;
	VALUE /*GridFlow::Format*/ ff; /* a file reader object */
	bool timelog; /* future use */
	struct timeval tv; /* future use */
	int framecount;
} GridIn;

typedef struct GridOut {
	GridObject_FIELDS;
	VALUE /*GridFlow::Format*/ ff; /* a file writer object */
	bool timelog;
	struct timeval tv;   /* time of the last grid_end */
	int framecount;
} GridOut;

/* ---------------------------------------------------------------- */

/* return and complain when file not open */
#define CHECK_FILE_OPEN \
	if (!$->ff) { whine("can't do that: file not open"); return; }

/* same with false return */
#define CHECK_FILE_OPEN2 \
	if (!$->ff) RAISE("can't do that: file not open");

/* ---------------------------------------------------------------- */

#define FC(s) ((FormatClass *)s->ff->grid_class)

METHOD(GridIn,_0_close) {
	CHECK_FILE_OPEN
	rb_funcall($->ff,SI(close),0);
	$->ff = 0;
}

METHOD(GridIn,_0_reset) {
	GridOutlet_abort($->out[0]);
	CHECK_FILE_OPEN
	/* $->ff->cl->frame($->ff,-1); */
}

void GridIn_frame(GridIn *$, int frame) {
	CHECK_FILE_OPEN
	/* this is not a sufficient check. see format_grid.c */
	if (GridOutlet_busy($->out[0]))
		RAISE("ignoring frame request: already waiting for a frame");
	if (frame!=-1) {
		rb_funcall($->ff,SI(seek),1,INT2NUM(frame));
	}
	rb_funcall($->ff,SI(frame),0);
}

METHOD(GridIn,_0_open) {
	VALUE qlass;
	qlass = rb_hash_aref(format_classes_dex,argv[0]);
	if (qlass==Qnil) RAISE("unknown file format identifier: %s",
		rb_sym_name(argv[0]));

//	whine("file format: %s (%s)",qlass->symbol_name, qlass->description);

	if ($->ff) rb_funcall($->ff,SI(close),0);
	argv[0] = SYM(in);
	$->ff = rb_funcall2(qlass,SI(new),argc,argv);
	rb_funcall($->ff,SI(connect),3,INT2NUM(0),rself,INT2NUM(1));
}

METHOD(GridIn,_0_bang) {
	CHECK_FILE_OPEN
	rb_funcall($->ff,SI(frame),0);
}

METHOD(GridIn,_0_int) {
	int frame = INT(argv[0]);
	CHECK_FILE_OPEN
	rb_funcall($->ff,SI(seek),1,frame);
	rb_funcall($->ff,SI(frame),0);
}

METHOD(GridIn,_0_option) {
	CHECK_FILE_OPEN
	rb_funcall2($->ff,SI(option),argc,argv);
}

METHOD(GridIn,init) {
	rb_call_super(argc,argv);
	$->ff = 0;
	$->timelog = 0; /* not used in @in yet */
	$->framecount = 0;
	gettimeofday(&$->tv,0);
}

METHOD(GridIn,delete) {
	if ($->ff!=Qnil) rb_funcall2($->ff,SI(close),argc,argv);
	$->ff = Qnil;
	rb_call_super(argc,argv);
}

METHOD(GridIn,_1_grid_begin) {
	int i;
	VALUE a[argc+2];
	a[0]=INT2NUM(0);
	a[1]=sym_grid_begin;
	for (i=0; i<argc; i++) a[2+i]=argv[i];
	FObject_send_out(argc+2,a,rself);
}

METHOD(GridIn,_1_grid_flow ) {
	int i;
	VALUE a[argc+2];
	a[0]=INT2NUM(0);
	a[1]=sym_grid_flow;
	for (i=0; i<argc; i++) a[2+i]=argv[i];
	FObject_send_out(argc+2,a,rself);
}

METHOD(GridIn,_1_grid_end  ) {
	int i;
	VALUE a[argc+2];
	a[0]=INT2NUM(0);
	a[1]=sym_grid_end;
	for (i=0; i<argc; i++) a[2+i]=argv[i];
	FObject_send_out(argc+2,a,rself);
}

GRCLASS(GridIn,inlets:1,outlets:1,
LIST(),
	DECL(GridIn,init),
	DECL(GridIn,delete),
	DECL(GridIn,_1_grid_begin),
	DECL(GridIn,_1_grid_flow),
	DECL(GridIn,_1_grid_end),
	DECL(GridIn,_0_bang),
	DECL(GridIn,_0_reset),
	DECL(GridIn,_0_open),
	DECL(GridIn,_0_close),
	DECL(GridIn,_0_int),
	DECL(GridIn,_0_option))

/* ---------------------------------------------------------------- */

/* virtual gridinlet */
METHOD(GridOut,_0_grid_begin) {
	rb_funcall2($->ff,SI(_0_grid_begin),argc,argv);
}

METHOD(GridOut,_0_grid_flow) {
	rb_funcall2($->ff,SI(_0_grid_flow),argc,argv);
}

METHOD(GridOut,_0_grid_end) {
	rb_funcall2($->ff,SI(_0_grid_end),argc,argv);
	LEAVE;
	{
		VALUE a[] = { INT2NUM(0), sym_bang };
		FObject_send_out(COUNT(a),a,rself);
	}
	if (!$->timelog) return;
	{
		struct timeval t;
		gettimeofday(&t,0);
		whine("@out:0:end: frame#%03d time: %6d.%03d s; minus last: %5d ms)\n",
			$->framecount,
			t.tv_sec%1000000, t.tv_usec/1000,
			(t.tv_sec-$->tv.tv_sec)*1000 + (t.tv_usec-$->tv.tv_usec)/1000);
		memcpy(&$->tv,&t,sizeof(struct timeval));
	}
	$->framecount+=1;
	ENTER;
}

METHOD(GridOut,_0_list) {
	rb_funcall2($->ff,SI(_0_list),argc,argv);
}

METHOD(GridOut,_0_option) {
	VALUE sym = argv[0];
	CHECK_FILE_OPEN
	if (sym == SYM(timelog)) {
		$->timelog = !! INT(argv[1]);
		whine("timelog = %d",$->timelog);
	} else {
		rb_funcall2($->ff,SI(option),argc,argv);
	}
}

METHOD(GridOut,_0_close) {
	CHECK_FILE_OPEN
/*	if (GridOutlet_busy($->out[0])) GridOutlet_abort($->out[0]); */
	rb_funcall($->ff,SI(close),0);
}

METHOD(GridOut,_0_open) {
	VALUE qlass;
	qlass = rb_hash_aref(format_classes_dex,argv[0]);
	if (qlass==Qnil) RAISE("unknown file format identifier: %s",
		rb_sym_name(argv[0]));

//	whine("file format: %s (%s)",qlass->symbol_name, qlass->description);

	if ($->ff) rb_funcall($->ff,SI(close),0);
	argv[0] = SYM(out);
	$->ff = rb_funcall2(qlass,SI(new),argc,argv);
}

METHOD(GridOut,init) {
	$->framecount = 0;
	$->timelog = 0;
	$->ff = 0;
	gettimeofday(&$->tv,0);
	rb_call_super(argc,argv);
	if (argc>0) {
		rb_funcall(rself,SI(_0_open),2,SYM(x11),SYM(here));
		rb_funcall(rself,SI(_0_option),3,SYM(out_size),argv[0],argv[1]);
	}
}

METHOD(GridOut,delete) {
	if ($->ff) rb_funcall($->ff,SI(close),0);
	rb_call_super(argc,argv);
}

GRCLASS(GridOut,inlets:1,outlets:1,
LIST(),
	DECL(GridOut,init),
	DECL(GridOut,delete),
	DECL(GridOut,_0_grid_begin),
	DECL(GridOut,_0_grid_flow),
	DECL(GridOut,_0_grid_end),
	DECL(GridOut,_0_list),
	DECL(GridOut,_0_open),
	DECL(GridOut,_0_close),
	DECL(GridOut,_0_option))

/* ---------------------------------------------------------------- */

/* "@scale_by" does quick scaling of pictures by integer factors */
typedef struct GridScaleBy {
	GridObject_FIELDS; /* inherit from GridObject */
	int rint; /* integer scale factor (r as in right inlet, which does not exist yet) */
} GridScaleBy;

/* processing a grid coming from inlet 0 */
/* one needs three special methods for that; they are declared using macros */
/* this one processes the header and accepts or rejects the grid */
GRID_BEGIN(GridScaleBy,0) {
	int scale = $->rint;
	Dim *a = in->dim;

	/* there are restrictions on grid sizes for efficiency reasons */
	if (Dim_count(a)!=3) RAISE("(height,width,chans) please");
	if (Dim_get(a,2)!=3) RAISE("3 chans please");

	{
		/* computing the output's size */
		int v[3]={ Dim_get(a,0)*scale, Dim_get(a,1)*scale, Dim_get(a,2) };
		GridOutlet_begin($->out[0],Dim_new(3,v));

		/* configuring the input format */
		GridInlet_set_factor(in,Dim_get(a,1)*Dim_get(a,2));
	}
	return true;
}

/* this method processes one packet of grid content */
GRID_FLOW(GridScaleBy,0) {
	int scale = $->rint;
	int rowsize = Dim_get(in->dim,1)*Dim_get(in->dim,2);
	int i,j,k;
	/* for every picture row in this packet... */
	for (; n>0; data+=rowsize, n-=rowsize) {

		/* scale the line x-wise */
		Number buf[rowsize*scale];
		int p=0;
		for (i=0; i<rowsize; i+=3) {
			for (k=0; k<scale; k++) {
				buf[p++]=data[i];
				buf[p++]=data[i+1];
				buf[p++]=data[i+2];
			}
		}

		/* send the x-scaled line several times (thus y-scaling) */
		for (j=0; j<scale; j++) {
			GridOutlet_send($->out[0],rowsize*scale,buf);
		}
	}
}

/* not much to do here: when the input is done, the output is done too */
GRID_END(GridScaleBy,0) {
	GridOutlet_end($->out[0]);
}

/* the constructor accepts a scale factor as an argument */
/* that argument is not modifiable through an inlet yet (that would be the right inlet) */
METHOD(GridScaleBy,init) {
	$->rint = argc<1 ? 2 : INT(argv[0]);
	rb_call_super(argc,argv);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

/* destructor */
/* this object has nothing more to deallocate than a plain GridObject */
/* therefore it only calls super() */
METHOD(GridScaleBy,delete) { rb_call_super(argc,argv); }

/* there's one inlet, one outlet, and two system methods (inlet #-1) */
GRCLASS(GridScaleBy,inlets:1,outlets:1,
LIST(GRINLET(GridScaleBy,0)),
	DECL(GridScaleBy,init),
	DECL(GridScaleBy,delete))

/* **************************************************************** */

typedef struct GridRGBtoHSV {
	GridObject_FIELDS;
} GridRGBtoHSV;

GRID_BEGIN(GridRGBtoHSV,0) {
	if (Dim_count(in->dim)<1) RAISE("at least 1 dim please");
	if (Dim_get(in->dim,Dim_count(in->dim)-1)!=3) RAISE("3 chans please");
	GridOutlet_begin($->out[0],Dim_dup(in->dim));
	GridInlet_set_factor(in,3);
	return true;
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
	int i;
	for (i=0; i<n; i+=3, buf+=3, data+=3) {
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
	GridOutlet_give(out,n,buf2);
}

GRID_END(GridRGBtoHSV,0) {
	GridOutlet_end($->out[0]);
}

METHOD(GridRGBtoHSV,init) {
	rb_call_super(argc,argv);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

METHOD(GridRGBtoHSV,delete) { rb_call_super(argc,argv); }

GRCLASS(GridRGBtoHSV,inlets:1,outlets:1,
LIST(GRINLET(GridRGBtoHSV,0)),
	DECL(GridRGBtoHSV,init),
	DECL(GridRGBtoHSV,delete))

/* **************************************************************** */

typedef struct GridHSVtoRGB {
	GridObject_FIELDS;
} GridHSVtoRGB;

GRID_BEGIN(GridHSVtoRGB,0) {
	if (Dim_count(in->dim)<1) RAISE("at least 1 dim please");
	if (Dim_get(in->dim,Dim_count(in->dim)-1)!=3) RAISE("3 chans please");
	GridOutlet_begin($->out[0],Dim_dup(in->dim));
	GridInlet_set_factor(in,3);
	return true;
}

GRID_FLOW(GridHSVtoRGB,0) {
	GridOutlet *out = $->out[0];
	Number *buf = NEW(Number,n), *buf2 = buf;
	int i;
	for (i=0; i<n; i+=3, buf+=3, data+=3) {
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
	GridOutlet_give(out,n,buf2);
}

GRID_END(GridHSVtoRGB,0) {
	GridOutlet_end($->out[0]);
}

METHOD(GridHSVtoRGB,init) {
	rb_call_super(argc,argv);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

METHOD(GridHSVtoRGB,delete) { rb_call_super(argc,argv); }

GRCLASS(GridHSVtoRGB,inlets:1,outlets:1,
LIST(GRINLET(GridHSVtoRGB,0)),
	DECL(GridHSVtoRGB,init),
	DECL(GridHSVtoRGB,delete))

/* **************************************************************** */
/* [rtmetro] */

typedef struct RtMetro {
	GridObject_FIELDS; /* yes, i know, it doesn't do grids */
	int ms; /* millisecond time interval */
	int on;
	uint64 next_time; /* next time an event occurred */
	uint64 last;
} RtMetro;

uint64 RtMetro_now(void) {
	struct timeval nowtv;
	gettimeofday(&nowtv,0);
	return nowtv.tv_sec * 1000000LL + nowtv.tv_usec;
}

static void RtMetro_alarm(VALUE rself) {
	uint64 now = RtMetro_now();
	DGS(RtMetro);
	//whine("rtmetro alarm tick: %lld; next_time: %lld; now-last: %lld",now,$->next_time,now-$->last);
	if (now >= $->next_time) {
		//whine("rtmetro sending bang");
		VALUE a[] = { INT2NUM(0), sym_bang };
		FObject_send_out(COUNT(a),a,rself);
		/* $->next_time = now; */ /* jmax style, less realtime */
		$->next_time += 1000*$->ms;
	}
	$->last = now;
}

METHOD(RtMetro,_0_int) {
	int oon = $->on;
	$->on = !! FIX2INT(argv[0]);
	whine("on = %d",$->on);
	if (oon && !$->on) {
		whine("deleting rtmetro alarm...");
		MainLoop_remove($);
	} else if (!oon && $->on) {
		whine("creating rtmetro alarm...");
		MainLoop_add($,RtMetro_alarm);
		$->next_time = RtMetro_now();
	}
}

METHOD(RtMetro,_1_int) {
	$->ms = FIX2INT(argv[0]);
	whine("ms = %d",$->ms);
}

METHOD(RtMetro,init) {
	rb_call_super(argc,argv);
	$->ms = FIX2INT(argv[0]);
	$->on = 0;
	whine("ms = %d",$->ms);
	whine("on = %d",$->on);
}

METHOD(RtMetro,delete) {
	rb_call_super(argc,argv);
}

GRCLASS(RtMetro,inlets:2,outlets:1,
LIST(),
	DECL(RtMetro,_0_int),
	DECL(RtMetro,_1_int),
	DECL(RtMetro,init),
	DECL(RtMetro,delete))

/* **************************************************************** */
/* [@global] */

/* a dummy object that gives access to any stuff global to
   GridFlow.
*/
typedef struct GridGlobal {
	GridObject_FIELDS; /* yes, i know, it doesn't do grids */
} GridGlobal;

static void profiler_reset$1(void*d,void*k,void*v) {
	((GridObject *)k)->profiler_cumul = 0;
}

METHOD(GridGlobal,_0_profiler_reset) {
/*
	VALUE os = gf_object_set;
	Dict_each(os,profiler_reset$1,0);
*/
}

static int by_profiler_cumul(void **a, void **b) {
	uint64 apc = (*(const GridObject **)a)->profiler_cumul;
	uint64 bpc = (*(const GridObject **)b)->profiler_cumul;
	return apc>bpc ? -1 : apc<bpc ? +1 : 0;
}

static void profiler_dump$1(void *d,void *k,void *v) {
/*
	List_push((List *)d,k);
*/
}

METHOD(GridGlobal,_0_profiler_dump) {
	/* if you blow 256 chars it's your own fault */
/*
	List *ol = List_new(0);

	uint64 total=0;
	int i;
	whine("--------------------------------");
	whine("         clock-ticks percent pointer  constructor");
	Dict_each(gf_object_set,profiler_dump$1,ol);
	List_sort(ol,by_profiler_cumul);
	for(i=0;i<List_size(ol);i++) {
		GridObject *o = List_get(ol,i);
		total += o->profiler_cumul;
	}
	if (total<1) total=1;
	for(i=0;i<List_size(ol);i++) {
		GridObject *o = List_get(ol,i);
		int ppm = o->profiler_cumul * 1000000 / total;
		char *buf = "(fix-me)"; // !@#$
		whine("%20lld %2d.%04d %08x [%s]\n",
			o->profiler_cumul,
			ppm/10000,
			ppm%10000,
			o,
			buf);
		FREE(buf);
	}
	whine("--------------------------------");
*/
}

METHOD(GridGlobal,init)   { rb_call_super(argc,argv); }
METHOD(GridGlobal,delete) { rb_call_super(argc,argv); }

GRCLASS(GridGlobal,inlets:1,outlets:1,
LIST(),
	DECL(GridGlobal,init),
	DECL(GridGlobal,delete),
	DECL(GridGlobal,_0_profiler_reset),
	DECL(GridGlobal,_0_profiler_dump))

/* **************************************************************** */

void startup_flow_objects (void) {
	INSTALL("@import",     GridImport);
	INSTALL("@export",     GridExport);
	INSTALL("@export_list",GridExportList);
	INSTALL("@store",      GridStore);
	INSTALL("@!",          GridOp1);
	INSTALL("@",           GridOp2);
	INSTALL("@fold",       GridFold);
/*	INSTALL("@inner",      GridInner); */
	INSTALL("@inner2",     GridInner2);
	INSTALL("@outer",      GridOuter);
	INSTALL("@convolve",   GridConvolve);
	INSTALL("@for",        GridFor);
	INSTALL("@dim",        GridDim);
	INSTALL("@redim",      GridRedim);
	INSTALL("@print",      GridPrint);
	INSTALL("@in",         GridIn);
	INSTALL("@out",        GridOut);
	INSTALL("@scale_by",   GridScaleBy);
	INSTALL("@rgb_to_hsv", GridRGBtoHSV);
	INSTALL("@hsv_to_rgb", GridHSVtoRGB);
	INSTALL("@rtmetro",    RtMetro);
	INSTALL("@global",     GridGlobal);
}

