/*
	$Id$

	Video4jmax
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
	int n;
	int *v;
} GridImport;

GRID_BEGIN(GridImport,0) { /* nothing to do */ return true; }
GRID_FLOW(GridImport,0) {
	GridOutlet *out = $->out[0];
	while(n) {
		int n2;
		if (GridOutlet_idle(out)) GridOutlet_begin(out,Dim_dup($->dim));
		n2 = Dim_prod(out->dim) - out->dex;
		if (n2 > n) n2 = n;
		GridOutlet_send(out,n,data);
		if (out->dex >= Dim_prod(out->dim)) GridOutlet_end(out);
		n -= n2;
		data += n2;
	}
}
GRID_END(GridImport,0) { /* nothing to do */ }

GRID_BEGIN(GridImport,1) {
	if (Dim_count(in->dim) != 1) return false;
	$->n = Dim_get(in->dim,0);
	if ($->n < 0 || $->n > MAX_DIMENSIONS) return false;
	$->v = NEW(int,Dim_get(in->dim,0));
	return true;
}

GRID_FLOW(GridImport,1) {
	int i;
	for(i=0;i<n;i++) { $->v[in->dex+i] = data[in->dex+i]; }
}

GRID_END(GridImport,1) {
	int i;
/*	GridInlet_abort($->in[0]); */
	if (!GridOutlet_idle($->out[0])) GridOutlet_abort($->out[0]);
	FREE($->dim);
	for(i=0;i<$->n;i++) {
		COERCE_INT_INTO_RANGE($->v[i],1,MAX_INDICES);
	}
	$->dim = Dim_new($->n, $->v);
	FREE($->v);
}

METHOD(GridImport,init) {
	int i;
	int v[ac-1];
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridImport,0);
	$->in[1] = GridInlet_NEW3($,GridImport,1);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	for (i=0; i<ac-1; i++) {
		v[i] = GET(i+1,int,0);
		COERCE_INT_INTO_RANGE(v[i],1,MAX_INDICES);
	}
	$->dim = Dim_new(ac-1,v);
	$->v = 0;
}

METHOD(GridImport,delete) {
	FREE($->dim);
	FREE($->v);
	GridObject_delete((GridObject *)$);
}

METHOD(GridImport,int) {
	GridOutlet *out = $->out[0];
	Number data[] = { GET(0,int,0) };
	if (GridOutlet_idle(out)) GridOutlet_begin(out,Dim_dup($->dim));
	GridOutlet_send(out,ARRAY(data));
	if (out->dex >= Dim_prod(out->dim)) GridOutlet_end(out);
}

METHOD(GridImport,reset) {
	GridOutlet *out = $->out[0];
	if (!GridOutlet_idle(out)) GridOutlet_abort(out);
}

CLASS(GridImport) {
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t int_dims[MAX_DIMENSIONS+1] = { fts_t_symbol };
	MethodDecl methods[] = {
		{-1, fts_s_init,  METHOD_PTR(GridImport,init),   ARRAY(int_dims),2},
		{-1, fts_s_delete,METHOD_PTR(GridImport,delete), 0,0,0},
		{ 0, fts_s_int,   METHOD_PTR(GridImport,int),    ARRAY(int_alone),-1},
		{ 0, sym_reset,   METHOD_PTR(GridImport,reset), 0,0,-1},
	};

	{int i; for (i=0; i<MAX_DIMENSIONS; i++) int_dims[i+1] = fts_t_int;}

	/* initialize the class */
	fts_class_init(class, sizeof(GridImport), 2, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);
	GridObject_conf_class(class,1);

	/* define the methods */

	return fts_Success;
}

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
		fts_atom_t a[1];
		fts_set_int(a+0,*data);
		fts_outlet_send(OBJ($),0,fts_s_int,1,a);
		data++;
	}
}

GRID_END(GridExport,0) {}

METHOD(GridExport,init) {
	int i;
	int v[ac-1];
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridExport,0);
}

METHOD(GridExport,delete) { GridObject_delete((GridObject *)$); }

CLASS(GridExport) {
	fts_type_t rien[] = { fts_t_symbol };
	MethodDecl methods[] = {
		{-1, fts_s_init,   METHOD_PTR(GridExport,init), ARRAY(rien), -1 },
		{-1, fts_s_delete, METHOD_PTR(GridExport,delete), 0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridExport), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);

	/* define the methods */

	return fts_Success;
}

/* **************************************************************** */
/*
  grid_store ("@store") is the class for storing a grid and restituting
  it on demand. The right inlet receives the grid. The left inlet receives
  either a bang (which forwards the whole image) or a grid describing what to
  send.
*/

typedef struct GridStore {
	GridObject_FIELDS;
/*	NumberType *nt; */
	int nt;
	Number *data;
	Dim *dim;
} GridStore;

GRID_BEGIN(GridStore,0) {
	int na = Dim_count(in->dim);
	int nb;
	int nc,nd,i;
	int v[MAX_DIMENSIONS];
	if (!$->data || !$->dim) {
		whine("empty buffer, better luck next time.");
		return false;
	}

	nb = Dim_count($->dim);

/*
	whine("[a] %s",Dim_to_s(in->dim));
	whine("[b] %s",Dim_to_s($->dim));
*/

	if (na<1 || na>1+nb) {
		whine("wrong number of dimensions: got %d, expecting %d..%d",
			na,1,1+nb);
		return false;
	}
	nc = Dim_get(in->dim,na-1);
	if (nc > nb) {
		whine("wrong number of elements in last dimension: "
			"got %d, expecting <= %d", nc, nb);
		return false;
	}
	nd = nb - nc + na - 1;
	if (nd > MAX_DIMENSIONS) {
		whine("too many dimensions!");
		return false;
	}
	for (i=0; i<na-1; i++) v[i] = Dim_get(in->dim,i);
	for (i=nc; i<nb; i++) v[na-1+i-nc] = Dim_get($->dim,i);
	GridOutlet_begin($->out[0],Dim_new(nd,v));
	GridInlet_set_factor(in,nc);
/*	whine("[r] %s",Dim_to_s($->out[0]->dim)); */
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

	while(n>0) {
		for (i=0; i<nc; i++) v[i] = mod(*data++,$->dim->v[i]);
		while (i<nb) v[i++] = 0;
		switch ($->nt) {
		case int32_type_i:
			GridOutlet_send($->out[0],size,&$->data[ Dim_calc_dex($->dim,v) ]);
		 	break;
		case uint8_type_i: {
			Number data3[16];
			uint8 *data2 = (uint8 *)$->data + Dim_calc_dex($->dim,v);
			int left = size;
			while (left>=16) {
				for (i=0; i<16; i++) { data3[i] = data2[i]; }
				GridOutlet_send($->out[0],16,data3);
				data2+=16;
				left-=16;
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

GRID_END(GridStore,0) {
	GridOutlet_end($->out[0]);
}

GRID_BEGIN(GridStore,1) {
	int length = Dim_prod(in->dim);
	GridInlet_abort($->in[0]);
	FREE($->dim);
	FREE($->data);
	$->dim = Dim_dup(in->dim);
	switch ($->nt) {
	case int32_type_i: $->data = NEW2(Number,length); break;
	case uint8_type_i: $->data = (Number *) NEW2(uint8,length); break;
	default: assert(0);
	}
	return true;
}

GRID_FLOW(GridStore,1) {
	int i;
	switch ($->nt) {
	case int32_type_i: memcpy(&$->data[in->dex], data, n*sizeof(Number)); break;
	case uint8_type_i: {
		uint8 *data2 = (uint8 *)$->data + in->dex;
		for(i=0; i<n; i++) data2[i] = data[i];
		break;}
	default: assert(0);
	}
	in->dex += n;
}

GRID_END(GridStore,1) {}

METHOD(GridStore,init) {
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridStore,0);
	$->in[1] = GridInlet_NEW3($,GridStore,1);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
	{
		fts_symbol_t t = GET(1,symbol,SYM(int32));
		if (t==SYM(int32)) {
			$->nt = int32_type_i;
		} else if (t==SYM(uint8)) {
			$->nt = uint8_type_i;
		} else {
			fts_object_set_error(OBJ($),
				"unknown element type \"%s\"", fts_symbol_name(t));
		}
	}
	$->data = 0;
	$->dim  = 0;
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

CLASS(GridStore) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol, fts_t_symbol };
	MethodDecl methods[] = {
		{-1, fts_s_init,   METHOD_PTR(GridStore,init), ARRAY(init_args),1},
		{-1, fts_s_delete, METHOD_PTR(GridStore,delete),0,0,0 },
		{ 0, fts_s_bang,   METHOD_PTR(GridStore,bang),  0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridStore), 2, 1, 0);
	define_many_methods(class,ARRAY(methods));

	GridObject_conf_class(class,0);
	GridObject_conf_class(class,1);

	return fts_Success;
}

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
	int i;
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
	fts_symbol_t sym = GET(1,symbol,op1_table[0].sym);

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridOp1,0);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->op = op1_table_find(sym);
	if (!$->op) {
		fts_object_set_error(OBJ($),
			"unknown unary operator \"%s\"", fts_symbol_name(sym));
	}
}

METHOD(GridOp1,delete) { GridObject_delete((GridObject *)$); }

CLASS(GridOp1) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol, fts_t_symbol };

	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridOp1,init),   ARRAY(init_args), 1 },
		{ -1, fts_s_delete, METHOD_PTR(GridOp1,delete), 0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridOp1), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);

	return fts_Success;
}

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

GRID_FLOW(GridOp2,0) {
	Number *data2 = NEW2(Number,n);
	GridOutlet *out = $->out[0];

	memcpy(data2,data,n*sizeof(Number));
	if ($->dim) {
		int loop = Dim_prod($->dim);
		int i;
		for (i=0; i<n; i++) {
			int rint = $->data[(in->dex+i)%loop];
			data2[i] = $->op->op(data2[i],rint);
		}
	} else {
		$->op->op_array(n,data2,$->rint);
	}
	in->dex += n;
	GridOutlet_send(out,n,data2);
	FREE(data2);
}

GRID_END(GridOp2,0) { GridOutlet_end($->out[0]); }

GRID_BEGIN(GridOp2,1) {
	int length = Dim_prod(in->dim);
	if ($->data) {
		GridInlet_abort($->in[0]); /* bug? */
		FREE($->data);
	}
	$->dim = Dim_dup(in->dim);
	$->data = NEW2(Number,length);
	return true;
}

GRID_FLOW(GridOp2,1) {
	int i;
	memcpy(&$->data[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridOp2,1) { /* nothing goes here */ }

METHOD(GridOp2,init) {
	fts_symbol_t sym = GET(1,symbol,op2_table[0].sym);
	$->rint = GET(2,int,0);

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridOp2,0);
	$->in[1] = GridInlet_NEW3($,GridOp2,1);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->op = op2_table_find(sym);
	if (!$->op) {
		fts_object_set_error(OBJ($),
			"unknown binary operator \"%s\"", fts_symbol_name(sym));
	}
}

METHOD(GridOp2,delete) {
	FREE($->data);
	FREE($->dim);
	GridObject_delete((GridObject *)$);
}

METHOD(GridOp2,rint) {
	FREE($->data);
	if ($->dim) FREE($->dim);
	$->rint = GET(0,int,-42);
}

CLASS(GridOp2) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol, fts_t_symbol, fts_t_int };

	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridOp2,init),   ARRAY(init_args), 2 },
		{ -1, fts_s_delete, METHOD_PTR(GridOp2,delete), 0,0,0 },
		{  1, fts_s_int,    METHOD_PTR(GridOp2,rint),   0,0,0 },/*why zero?*/
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridOp2), 2, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);
	GridObject_conf_class(class,1);

	return fts_Success;
}

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
	Dim *foo;
	if (n<1) { whine("minimum 1 dimension"); return false; }

	foo = Dim_new(n-1,in->dim->v);
/*	whine("fold dimension = %s",Dim_to_s(foo)); */
	GridOutlet_begin($->out[0],foo);
	GridInlet_set_factor(in,Dim_get(in->dim,Dim_count(in->dim)-1));
	return true;
}

GRID_FLOW(GridFold,0) {
	int factor = Dim_get(in->dim,Dim_count(in->dim)-1);
	int i=0;
	GridOutlet *out = $->out[0];
	int bs = n/factor;
	Number buf[bs];

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
	int i;
	fts_symbol_t sym = GET(1,symbol,op2_table[0].sym);
	$->rint = GET(2,int,0);

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridFold,0);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->op = op2_table_find(sym);
	if (!$->op) {
		fts_object_set_error(OBJ($),
			"unknown binary operator \"%s\"", fts_symbol_name(sym));
	}
}

METHOD(GridFold,delete) { GridObject_delete((GridObject *)$); }

METHOD(GridFold,rint) {
	$->rint = GET(0,int,-42);
}

CLASS(GridFold) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol, fts_t_symbol, fts_t_int };

	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridFold,init),   ARRAY(init_args), 2 },
		{ -1, fts_s_delete, METHOD_PTR(GridFold,delete), 0,0,0 },
		{  1, fts_s_int,    METHOD_PTR(GridFold,rint),   0,0,0 },/*why zero?*/
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridFold), 2, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);
	return fts_Success;
}

/* **************************************************************** */

typedef struct GridInner {
	GridObject_FIELDS;
	Dim *dim;
	Number *data;
	Number rint;
	Operator2 *op_para;
	Operator2 *op_fold;
} GridInner;

GRID_BEGIN(GridInner,0) {
	Dim *a = in->dim;
	Dim *b = $->dim;
	if (!b) { whine("right inlet has no grid"); return false; }
	if (Dim_count(a)<1) { whine("minimum 1 dimension"); return false; }
	{
		int a_last = Dim_get(a,Dim_count(a)-1);
		int b_last = Dim_get(b,Dim_count(b)-1);
		int n = Dim_count(a)+Dim_count(b)-1;
		int v[n];
		int i,j;
		if (a_last != b_last) {
			whine("last dimension of each grid must have same size");
			return false;
		}
		for (i=j=0; j<Dim_count(a)-1; i++,j++) { v[i] = Dim_get(a,j); }
		for (  j=0; j<Dim_count(b)  ; i++,j++) { v[i] = Dim_get(b,j); }
		GridOutlet_begin($->out[0],Dim_new(n,v));
	}	
	return true;
}

GRID_FLOW(GridInner,0) {
	/* write me */
}

GRID_END(GridInner,0) {
	GridOutlet_end($->out[0]);
}

GRID_BEGIN(GridInner,2) {
	int length = Dim_prod(in->dim);
	GridInlet_abort($->in[0]);
	if (Dim_count(in->dim)<1) { whine("minimum 1 dimension"); return false; }
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
	fts_symbol_t sym_para = GET(1,symbol,op2_table[0].sym);
	fts_symbol_t sym_fold = GET(2,symbol,op2_table[0].sym);
	$->rint = GET(3,int,0);
	$->dim = 0;
	$->data = 0;

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridInner,0);
	$->in[2] = GridInlet_NEW3($,GridInner,2);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->op_para = op2_table_find(sym_para);
	$->op_fold = op2_table_find(sym_fold);
	if (!$->op_para) {
		fts_object_set_error(OBJ($),
			"unknown binary operator \"%s\"", fts_symbol_name(sym_para));
	}
	if (!$->op_fold) {
		fts_object_set_error(OBJ($),
			"unknown binary operator \"%s\"", fts_symbol_name(sym_fold));
	}
}

METHOD(GridInner,delete) { GridObject_delete((GridObject *)$); }

CLASS(GridInner) {
	int i;
	fts_type_t int_alone[]  = {fts_t_int};
	fts_type_t init_args[] = {fts_t_symbol, fts_t_symbol, fts_t_symbol, fts_t_int};
	MethodDecl methods[] = { 
		{-1, fts_s_init,   METHOD_PTR(GridInner,init), ARRAY(init_args),-1},
		{-1, fts_s_delete, METHOD_PTR(GridInner,delete),0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridInner), 3, 1, 0);
	define_many_methods(class,ARRAY(methods));

	GridObject_conf_class(class,0);
	GridObject_conf_class(class,1);

	return fts_Success;
}

/* **************************************************************** */

typedef struct GridOuter {
	GridObject_FIELDS;
	Dim *dim;
	Number *data;
	Operator2 *op;
} GridOuter;

GRID_BEGIN(GridOuter,0) {
	Dim *a = in->dim;
	Dim *b = $->dim;
	if (!b) { whine("right inlet has no grid"); return false; }
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
	int i,j;
	int b_prod = Dim_prod($->dim);
	Number *buf = NEW(Number,b_prod);
	for (i=0; i<n; i++) {
		for (j=0; j<b_prod; j++) {
			buf[j] = $->op->op(data[i],$->data[j]);
		}
		GridOutlet_send($->out[0],b_prod,buf);
	}
	FREE(buf);
}

GRID_END(GridOuter,0) {
	GridOutlet_end($->out[0]);
}

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
	fts_symbol_t sym = GET(1,symbol,op2_table[0].sym);

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridOuter,0);
	$->in[1] = GridInlet_NEW3($,GridOuter,1);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->dim = 0;
	$->data = 0;

	$->op = op2_table_find(sym);
	if (!$->op) {
		fts_object_set_error(OBJ($),
			"unknown binary operator \"%s\"", fts_symbol_name(sym));
	}
}

METHOD(GridOuter,delete) { GridObject_delete((GridObject *)$); }

CLASS(GridOuter) {
	int i;
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t init_args[] = { fts_t_symbol, fts_t_symbol };
	MethodDecl methods[] = {
		{-1, fts_s_init,   METHOD_PTR(GridOuter,init), ARRAY(init_args),-1},
		{-1, fts_s_delete, METHOD_PTR(GridOuter,delete),0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridOuter), 2, 1, 0);
	define_many_methods(class,ARRAY(methods));

	GridObject_conf_class(class,0);
	GridObject_conf_class(class,1);

	return fts_Success;
}

/* **************************************************************** */

typedef struct GridConvolve {
	GridObject_FIELDS;
	Dim *dim;
	Number *data;
	Dim *diml;
	Number *datal;
	const Operator2 *op_para;
	const Operator2 *op_fold;
} GridConvolve;

GRID_BEGIN(GridConvolve,0) {
	Dim *a = in->dim;
	Dim *b = $->dim;
	if (!b) { whine("right inlet has no grid"); return false; }
	if (Dim_count(a) < Dim_count(b)) {
		whine("left grid has less dimensions than right grid");
		return false;
	}
	$->diml = Dim_dup(a);
	$->datal = NEW(Number,Dim_prod(a));
	GridOutlet_begin($->out[0],Dim_dup(in->dim));
	return true;
}

GRID_FLOW(GridConvolve,0) {
	memcpy(&$->datal[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridConvolve,0) {
	int k;
	int iy, ix, ny = Dim_get($->diml,0), nx = Dim_get($->diml,1);
	int jy, jx, my = Dim_get($->dim ,0), mx = Dim_get($->dim ,1);
	int l = Dim_prod_start($->diml,2);
	Number buf[l];
	Number as[l];
	
	for (iy=0; iy<ny; iy++) {
		for (ix=0; ix<nx; ix++) {
			Number *d = $->data;
			for (k=0; k<l; k++) { buf[k] = 0; }
			for (jy=0; jy<my; jy++) {
				int pos1 = ((iy+jy-my/2+ny)%ny) * nx;
				for (jx=0; jx<mx; jx++) {
					Number b = *d++;
					int pos = pos1 + (ix+jx-mx/2+nx)%nx;
					memcpy(as,&$->datal[pos*l],l*sizeof(Number));
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
	int i;
	if (count != 2) {
		whine("only exactly two dimensions allowed for now");
		return false;
	}
	for (i=0; i<count; i++) {
		if ((Dim_get(in->dim,i) & 1) == 0) {
			whine("even number of elements");
			return false;
		}
	}
	GridInlet_abort($->in[0]);
	FREE($->dim);
	FREE($->data);
	$->dim = Dim_dup(in->dim);
	$->data = NEW2(Number,length);
	return true;
}

GRID_FLOW(GridConvolve,1) {
	memcpy(&$->data[in->dex], data, n*sizeof(Number));
	in->dex += n;
}

GRID_END(GridConvolve,1) {}

METHOD(GridConvolve,init) {
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridConvolve,0);
	$->in[1] = GridInlet_NEW3($,GridConvolve,1);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
	$->dim = 0;
	$->data = 0;
	$->diml = 0;
	$->datal = 0;
	$->op_para = op2_table_find(SYM(*));
	$->op_fold = op2_table_find(SYM(+));
}

METHOD(GridConvolve,delete) { GridObject_delete((GridObject *)$); }

CLASS(GridConvolve) {
	int i;
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t rien[] = { fts_t_symbol };
	MethodDecl methods[] = {
		{-1, fts_s_init,   METHOD_PTR(GridConvolve,init), ARRAY(rien),-1},
		{-1, fts_s_delete, METHOD_PTR(GridConvolve,delete),0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridConvolve), 2, 1, 0);
	define_many_methods(class,ARRAY(methods));

	GridObject_conf_class(class,0);
	GridObject_conf_class(class,1);

	return fts_Success;
}

/* **************************************************************** */

typedef struct GridFor {
	GridObject_FIELDS;
	Number from;
	Number to;
	Number step;
} GridFor;

METHOD(GridFor,init) {
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->from = GET(1,int,0);
	$->to   = GET(2,int,0);
	$->step = GET(3,int,0);
	if (!$->step) $->step=1;
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

METHOD(GridFor,delete) { GridObject_delete((GridObject *)$); }

METHOD(GridFor,from) { $->from = GET(0,int,0); }
METHOD(GridFor,to  ) { $->to   = GET(0,int,0); }
METHOD(GridFor,step) { $->step = GET(0,int,0); if (!$->step) $->step=1; }

METHOD(GridFor,bang) {
	int v = ($->to - $->from + $->step - 1) / $->step;
	Number x;
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

CLASS(GridFor) {
	int i;
	fts_type_t init_args[] = {fts_t_symbol, fts_t_int, fts_t_int, fts_t_int};
	fts_type_t set_int[]   = {fts_t_symbol, fts_t_int};
	fts_type_t one_int[]   = {fts_t_int};

	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridFor,init),   ARRAY(init_args),1 },
		{ -1, fts_s_delete, METHOD_PTR(GridFor,delete), 0,0,0 },
		{  0, fts_s_bang,   METHOD_PTR(GridFor,bang),   0,0,0 },
		{  0, fts_s_int,    METHOD_PTR(GridFor,from2),  ARRAY(one_int),-1 },
		{  0, fts_s_set,    METHOD_PTR(GridFor,from),   ARRAY(one_int),-1 },
		{  1, fts_s_int,    METHOD_PTR(GridFor,to),     ARRAY(one_int),-1 },
		{  2, fts_s_int,    METHOD_PTR(GridFor,step),   ARRAY(one_int),-1 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridFor), 3, 1, 0);
	define_many_methods(class,ARRAY(methods));
	return fts_Success;
}

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

GRID_FLOW(GridDim,0) { /* nothing to do */ }

GRID_END(GridDim,0) {}

METHOD(GridDim,init) {
	int i;
	int v[ac-1];
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridDim,0);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

METHOD(GridDim,delete) { GridObject_delete((GridObject *)$); }

CLASS(GridDim) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol };
	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridDim,init),   ARRAY(init_args),0 },
		{ -1, fts_s_delete, METHOD_PTR(GridDim,delete), 0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridDim), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);
	return fts_Success;
}

/* **************************************************************** */

typedef struct GridRedim {
	GridObject_FIELDS;
	Dim *dim;
	Number *data;
	int n;
	int *v;
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
}

/* same inlet 1 as @import */

GRID_BEGIN(GridRedim,1) {
	if (Dim_count(in->dim) != 1) return false;
	$->n = Dim_get(in->dim,0);
	if ($->n < 0 || $->n > MAX_DIMENSIONS) return false;
	$->v = NEW(int,Dim_get(in->dim,0));
	return true;
}

GRID_FLOW(GridRedim,1) {
	int i;
	for(i=0;i<n;i++) {
		int v = data[in->dex+i];
		COERCE_INT_INTO_RANGE(v,1,MAX_INDICES);
		$->v[in->dex+i] = v;
	}
}

GRID_END(GridRedim,1) {
	GridInlet_abort($->in[0]);
	GridOutlet_abort($->out[0]);
	FREE($->dim);
	$->dim = Dim_new($->n, $->v);
	FREE($->v);
}

METHOD(GridRedim,init) {
	int i;
	int v[ac-1];
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridRedim,0);
	$->in[1] = GridInlet_NEW3($,GridRedim,1);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	for (i=0; i<ac-1; i++) {
		v[i] = GET(i+1,int,0);
		COERCE_INT_INTO_RANGE(v[i],1,MAX_INDICES);
	}
	$->dim = Dim_new(ac-1,v);
	$->v = 0;
}

METHOD(GridRedim,delete) {
	FREE($->dim);
	FREE($->data);
	FREE($->v);
}

CLASS(GridRedim) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol };
	fts_type_t int_dims[MAX_DIMENSIONS+1] = { fts_t_symbol };
	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridRedim,init),   ARRAY(int_dims),2 },
		{ -1, fts_s_delete, METHOD_PTR(GridRedim,delete), 0,0,0 },
	};

	{int i; for (i=0; i<MAX_DIMENSIONS; i++) int_dims[i+1] = fts_t_int;}

	/* initialize the class */
	fts_class_init(class, sizeof(GridRedim), 2, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);
	GridObject_conf_class(class,1);
	return fts_Success;
}

/* **************************************************************** */

typedef struct GridPrint GridPrint;
struct GridPrint {
	GridObject_FIELDS;
};

GRID_BEGIN(GridPrint,0) {
	if (Dim_count(in->dim)>1) {
		char *foo = Dim_to_s(in->dim);
		whine("Grid %s: %s",foo);
		FREE(foo);
	}
	return true;
}

GRID_FLOW(GridPrint,0) {
	if (Dim_count(in->dim)<=1) { /* write me */ }
	/* write me */
}

GRID_END(GridPrint,0) {}

METHOD(GridPrint,init) {
	int i;
	int v[ac-1];
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridPrint,0);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
}

METHOD(GridPrint,delete) { GridObject_delete((GridObject *)$); }

CLASS(GridPrint) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol };
	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridPrint,init),   ARRAY(init_args),0 },
		{ -1, fts_s_delete, METHOD_PTR(GridPrint,delete), 0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridPrint), 1, 0, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);
	return fts_Success;
}

/* **************************************************************** */

#define INSTALL(_sym_,_name_) \
	fts_class_install(fts_new_symbol(_sym_),_name_##_class_init)

void startup_grid_basic (void) {
	INSTALL("@import",     GridImport);
	INSTALL("@export",     GridExport);
	INSTALL("@store",       GridStore);
	INSTALL("@!",             GridOp1);
	INSTALL("@",              GridOp2);
	INSTALL("@fold",         GridFold);
	INSTALL("@inner",       GridInner);
	INSTALL("@outer",       GridOuter);
	INSTALL("@convolve", GridConvolve);
	INSTALL("@for",           GridFor);
	INSTALL("@dim",           GridDim);
	INSTALL("@redim",       GridRedim);
	INSTALL("@print",       GridPrint);
}
