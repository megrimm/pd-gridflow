/*
	$Id$

	GridFlow
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>
#include <math.h>
#include "grid.h"

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

/* !@#$ this could blow up */
GRID_FLOW(GridImport,1) {
	int i;
	int *dim = NEW(int,n);
	FREE($->dim);
	for(i=0;i<n;i++) { dim[i]=data[i]; COERCE_INT_INTO_RANGE(dim[i],1,MAX_INDICES); }
	$->dim = Dim_new(n,dim);
	GridInlet_abort($->in[0]);
	GridOutlet_abort($->out[0]);
	FREE(dim);
}

GRID_END(GridImport,1) {}

METHOD(GridImport,init) {
	int i;
	int v[ac-1];
	GridObject_init((GridObject *)$);

	for (i=0; i<ac-1; i++) {
		v[i] = GET(i+1,int,0);
		if (v[i]<1 || v[i]>MAX_INDICES) RAISE2("dim out of bounds");
	}
	$->dim = Dim_new(ac-1,v);
}

METHOD(GridImport,delete) {
	FREE($->dim);
	GridObject_delete((GridObject *)$);
}

METHOD(GridImport,int) {
	GridOutlet *out = $->out[0];
	Number data[] = { GET(0,int,0) };
	if (!GridOutlet_busy(out)) GridOutlet_begin(out,Dim_dup($->dim));
	GridOutlet_send(out,ARRAY(data));
	if (out->dex >= Dim_prod(out->dim)) GridOutlet_end(out);
}

METHOD(GridImport,reset) {
	GridOutlet *out = $->out[0];
	if (GridOutlet_busy(out)) GridOutlet_abort(out);
}

GRCLASS(GridImport,inlets:2,outlets:1,
LIST(GRINLET(GridImport,0),GRINLET(GridImport,1)),
	DECL(GridImport,-1,init,  "si+"),
	DECL(GridImport,-1,delete,""),
	DECL(GridImport, 0,int,   "i"),
	DECL(GridImport, 0,reset, ""))

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
		Var a[1];
		Var_put_int(a+0,*data);
		Object_send_thru(OBJ($),0,sym_int,1,a);
		data++;
	}
}

GRID_END(GridExport,0) {}

METHOD(GridExport,init)   { GridObject_init(  (GridObject *)$); }
METHOD(GridExport,delete) { GridObject_delete((GridObject *)$); }

GRCLASS(GridExport,inlets:1,outlets:1,
LIST(GRINLET(GridExport,0)),
/* outlet 0 not used for grids */
	DECL(GridExport,-1,init,  "s"),
	DECL(GridExport,-1,delete,""))

/* **************************************************************** */

typedef struct GridExportList {
	GridObject_FIELDS;
	Var *list;
	int n;
} GridExportList;

GRID_BEGIN(GridExportList,0) {
	int n = Dim_prod(in->dim);
	if (n>1000) RAISE("list too big (%d elements)", n);
	$->list = NEW(Var,n);
	$->n = n;
	return true;
}

GRID_FLOW(GridExportList,0) {
	int i;
	for (i=0; i<n; i++, data++) Var_put_int($->list+in->dex+i,*data);
}

GRID_END(GridExportList,0) {
	Object_send_thru(OBJ($),0,sym_list,$->n,$->list);
	FREE($->list);
}

METHOD(GridExportList,init)   { GridObject_init(  (GridObject *)$); }
METHOD(GridExportList,delete) { GridObject_delete((GridObject *)$); }

GRCLASS(GridExportList,inlets:1,outlets:1,
LIST(GRINLET(GridExportList,0)),
/* outlet 0 not used for grids */
	DECL(GridExportList,-1,init,  "s"),
	DECL(GridExportList,-1,delete,""))

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
	Symbol t = GET(1,symbol,SYM(int32));
	GridObject_init((GridObject *)$);
	$->data = 0;
	$->dim  = 0;
	if (t==SYM(int32)) {
		$->nt = int32_type_i;
	} else if (t==SYM(uint8)) {
		$->nt = uint8_type_i;
	} else {
		RAISE2("unknown element type \"%s\"", Symbol_name(t));
	}
}

METHOD(GridStore,delete) {
	FREE($->data);
	FREE($->dim);
	GridObject_delete((GridObject *)$);
}

METHOD(GridStore,bang) {
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
	DECL(GridStore,-1,init,  "s;s"),
	DECL(GridStore,-1,delete,""),
	DECL(GridStore, 0,bang,  ""))

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
	Symbol sym = GET(1,symbol,op1_table[0].sym);

	GridObject_init((GridObject *)$);

	$->op = op1_table_find(sym);
	if (!$->op) RAISE2("unknown unary operator \"%s\"", Symbol_name(sym));
}

METHOD(GridOp1,delete) { GridObject_delete((GridObject *)$); }

GRCLASS(GridOp1,inlets:1,outlets:1,
LIST(GRINLET(GridOp1,0)),
	DECL(GridOp1,-1,init,  "ss"),
	DECL(GridOp1,-1,delete,""))

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
	Symbol sym = GET(1,symbol,op2_table[0].sym);
	$->rint = GET(2,int,0);
	$->dim = 0;

	GridObject_init((GridObject *)$);

	$->op = op2_table_find(sym);
	if (!$->op) RAISE2("unknown binary operator \"%s\"", Symbol_name(sym));
}

METHOD(GridOp2,delete) {
	FREE($->data);
	FREE($->dim);
	GridObject_delete((GridObject *)$);
}

METHOD(GridOp2,int) {
	FREE($->data);
	if ($->dim) FREE($->dim);
	$->rint = GET(0,int,-42);
}

GRCLASS(GridOp2,inlets:2,outlets:1,
LIST(GRINLET2(GridOp2,0),GRINLET(GridOp2,1)),
	DECL(GridOp2,-1,init,  "ss;i"),
	DECL(GridOp2,-1,delete,""),
	DECL(GridOp2, 1,int,   ""),/*why zero?*/)

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
	Symbol sym = GET(1,symbol,op2_table[0].sym);
	$->rint = GET(2,int,0);

	GridObject_init((GridObject *)$);

	$->op = op2_table_find(sym);
	if (!$->op) RAISE2("unknown binary operator \"%s\"", Symbol_name(sym));
}

METHOD(GridFold,delete) { GridObject_delete((GridObject *)$); }

METHOD(GridFold,int) {
	$->rint = GET(0,int,-42);
}

GRCLASS(GridFold,inlets:2,outlets:1,
LIST(GRINLET(GridFold,0)),
	DECL(GridFold,-1,init,  "ss;i"),
	DECL(GridFold,-1,delete,""),
	DECL(GridFold, 1,int,   ""),/*why zero?*/)

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
	Symbol sym_para = GET(1,symbol,op2_table[0].sym);
	Symbol sym_fold = GET(2,symbol,op2_table[0].sym);
	$->rint = GET(3,int,0);
	$->dim = 0;
	$->data = 0;

	GridObject_init((GridObject *)$);

	$->op_para = op2_table_find(sym_para);
	$->op_fold = op2_table_find(sym_fold);
	if (!$->op_para) RAISE2("unknown binary operator \"%s\"", Symbol_name(sym_para));
	if (!$->op_fold) RAISE2("unknown binary operator \"%s\"", Symbol_name(sym_fold));
}

METHOD(GridInner,delete) {
	FREE($->dim);
	FREE($->data);
	GridObject_delete((GridObject *)$);
}

GRCLASS(GridInner,inlets:3,outlets:1,
LIST(GRINLET(GridInner,0),GRINLET(GridInner,2)),
	DECL(GridInner,-1,init,  "sss;i"),
	DECL(GridInner,-1,delete,""))
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
	Symbol sym_para = GET(1,symbol,op2_table[0].sym);
	Symbol sym_fold = GET(2,symbol,op2_table[0].sym);
	$->rint = GET(3,int,0);
	$->dim = 0;
	$->data = 0;

	GridObject_init((GridObject *)$);

	$->op_para = op2_table_find(sym_para);
	$->op_fold = op2_table_find(sym_fold);
	if (!$->op_para) RAISE2("unknown binary operator \"%s\"", Symbol_name(sym_para));
	if (!$->op_fold) RAISE2("unknown binary operator \"%s\"", Symbol_name(sym_fold));
}

METHOD(GridInner2,delete) {
	FREE($->dim);
	FREE($->data);
	GridObject_delete((GridObject *)$);
}

GRCLASS(GridInner2,inlets:3,outlets:1,
LIST(GRINLET(GridInner2,0),GRINLET(GridInner2,2)),
	DECL(GridInner2,-1,init,  "sss;i"),
	DECL(GridInner2,-1,delete,""))

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
	Symbol sym = GET(1,symbol,op2_table[0].sym);

	GridObject_init((GridObject *)$);

	$->dim = 0;
	$->data = 0;

	$->op = op2_table_find(sym);
	if (!$->op) RAISE2("unknown binary operator \"%s\"", Symbol_name(sym));
}

METHOD(GridOuter,delete) {
	FREE($->dim);
	FREE($->data);
	GridObject_delete((GridObject *)$);
}

GRCLASS(GridOuter,inlets:2,outlets:1,
LIST(GRINLET(GridOuter,0),GRINLET(GridOuter,1)),
	DECL(GridOuter,-1,init,  "ss"),
	DECL(GridOuter,-1,delete,""))

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
	Symbol sym_para = GET(1,symbol,SYM(*));
	Symbol sym_fold = GET(2,symbol,SYM(+));

	GridObject_init((GridObject *)$);
	$->dim = 0;
	$->data = 0;
	$->diml = 0;
	$->datal = 0;

	$->op_para = op2_table_find(sym_para);
	$->op_fold = op2_table_find(sym_fold);
	$->rint = GET(3,int,0);

	if (!$->op_para) RAISE2("unknown binary operator \"%s\"", Symbol_name(sym_para));
	if (!$->op_fold) RAISE2("unknown binary operator \"%s\"", Symbol_name(sym_fold));
}

METHOD(GridConvolve,delete) {
	FREE($->dim);  FREE($->diml);
	FREE($->data); FREE($->datal);
	GridObject_delete((GridObject *)$);
}

GRCLASS(GridConvolve,inlets:2,outlets:1,
LIST(GRINLET(GridConvolve,0),GRINLET(GridConvolve,1)),
	DECL(GridConvolve,-1,init,  "s;ssi"),
	DECL(GridConvolve,-1,delete,""))

/* **************************************************************** */

typedef struct GridFor {
	GridObject_FIELDS;
	Number from;
	Number to;
	Number step;
} GridFor;

METHOD(GridFor,init) {
	GridObject_init((GridObject *)$);
	$->from = GET(1,int,0);
	$->to   = GET(2,int,0);
	$->step = GET(3,int,0);
	if (!$->step) $->step=1;
}

METHOD(GridFor,delete) { GridObject_delete((GridObject *)$); }

METHOD(GridFor,from) { $->from = GET(0,int,0); }
METHOD(GridFor,to  ) { $->to   = GET(0,int,0); }
METHOD(GridFor,step) { $->step = GET(0,int,0); if (!$->step) $->step=1; }

METHOD(GridFor,bang) {
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

METHOD(GridFor,from2) {
	$->from = GET(0,int,0);
	GridFor_bang($,winlet,selector,ac,at);
}

GRCLASS(GridFor,inlets:3,outlets:1,
LIST(),
	DECL(GridFor,-1,init,  "siii"),
	DECL(GridFor,-1,delete,""),
	DECL2(GridFor, 0,bang,bang, ""),
	DECL2(GridFor, 0,int, from2,"i"),
	DECL2(GridFor, 0,set, from, "i"),
	DECL2(GridFor, 1,int, to,   "i"),
	DECL2(GridFor, 2,int, step, "i"))

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

METHOD(GridDim,init) { GridObject_init((GridObject *)$); }

METHOD(GridDim,delete) { GridObject_delete((GridObject *)$); }

GRCLASS(GridDim,inlets:1,outlets:1,
LIST(GRINLET(GridDim,0)),
	DECL(GridDim,-1,init,  "s"),
	DECL(GridDim,-1,delete,""))

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

/* !@#$ this could blow up */
GRID_FLOW(GridRedim,1) {
	int i;
	int *dim = NEW(int,n);
	FREE($->dim);
	for(i=0;i<n;i++) { dim[i]=data[i]; COERCE_INT_INTO_RANGE(dim[i],1,MAX_INDICES); }
	$->dim = Dim_new(n,dim);
	GridInlet_abort($->in[0]);
	GridOutlet_abort($->out[0]);
	FREE(dim);
}

GRID_END(GridRedim,1) {}

METHOD(GridRedim,init) {
	int i;
	int v[ac-1];
	GridObject_init((GridObject *)$);

	for (i=0; i<ac-1; i++) {
		v[i] = GET(i+1,int,0);
		COERCE_INT_INTO_RANGE(v[i],1,MAX_INDICES);
	}
	$->dim = Dim_new(ac-1,v);
	$->data = 0;
}

METHOD(GridRedim,delete) {
	FREE($->dim);
	FREE($->data);
	GridObject_delete((GridObject *)$);
}

GRCLASS(GridRedim,inlets:2,outlets:1,
LIST(GRINLET(GridRedim,0),GRINLET(GridRedim,0)),
	DECL(GridRedim,-1,init,  "si+"),
	DECL(GridRedim,-1,delete,""))

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

METHOD(GridPrint,init) { GridObject_init((GridObject *)$); }

METHOD(GridPrint,delete) { GridObject_delete((GridObject *)$); }

GRCLASS(GridPrint,inlets:1,outlets:0,
LIST(GRINLET(GridPrint,0)),
	DECL(GridPrint,-1,init,  "s"),
	DECL(GridPrint,-1,delete,""))

/* **************************************************************** */

#define INSTALL(_sym_,_name_) \
	fts_class_install(Symbol_new(_sym_),_name_##_class_init)

void startup_grid_basic (void) {
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
}
