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

typedef struct GridImport GridImport;
struct GridImport {
	GridObject_FIELDS;
	Dim *dim; /* size of grids to send */
};

METHOD(GridImport,init) {
	int i;
	int v[ac-1];
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	for (i=0; i<ac-1; i++) {
		v[i] = GET(i+1,int,0);
		COERCE_INT_INTO_RANGE(v[i],1,MAX_INDICES);
	}
	$->dim = Dim_new(ac-1,v);
	whine("%s",Dim_to_s($->dim));
}

METHOD(GridImport,delete) { /* write me */ }

METHOD(GridImport,int) {
	Number data[] = { GET(0,int,0) };
	GridOutlet *out = $->out[0];
	if (GridOutlet_idle(out)) GridOutlet_begin(out,$->dim);
	GridOutlet_send(out,COUNT(data),data);
	if (out->dex >= Dim_prod(out->dim)) GridOutlet_abort(out);
}

METHOD(GridImport,reset) {
	GridOutlet *out = $->out[0];
	GridOutlet_abort(out);
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
	fts_class_init(class, sizeof(GridImport), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));

	/* define the methods */

	return fts_Success;
}

/* **************************************************************** */
/*
  GridExport ("@export") is the class for converting from streamed grids
  to old-style integer stream.
*/

typedef struct GridExport GridExport;
struct GridExport {
	GridObject_FIELDS;
};

GRID_BEGIN(GridExport,0) { /* nothing to do */ }

GRID_FLOW(GridExport,0) {
	int i;
	for (i=0; i<n; i++) {
		fts_atom_t a[1];
		fts_set_int(a+0,*data);
		fts_outlet_send(OBJ($->parent),0,fts_s_int,1,a);
		data++;
		/* grdex_incr($->out_dex); */
	}
}

METHOD(GridExport,init) {
	int i;
	int v[ac-1];
	GridInlet *in;
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = in = GridInlet_new((GridObject *)$, 0,
		(GridBegin)GridExport_0_begin,
		(GridFlow)GridExport_0_flow);
}

METHOD(GridExport,delete) { /* write me */ }

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

/* !@#$ */

typedef struct GridStore GridStore;
struct GridStore {
	GridObject_FIELDS;
	Number *data;
	Dim *dim;
	Number *buf;
	int bufn;
};

GRID_BEGIN(GridStore,0) {
	int na = Dim_count($->dim);
	int nb;
	int nc,nd,i;
	int v[MAX_DIMENSIONS];
	if (!parent->data || !parent->dim) {
		whine("empty buffer, better luck next time.");
		goto err;
	}

	nb = Dim_count(parent->dim);

	whine("[a] %s",Dim_to_s($->dim));
	whine("[b] %s",Dim_to_s(parent->dim));

	if (na<1 || na>1+nb) {
		whine("wrong number of dimensions: got %d, expecting %d..%d",
			na,1,1+nb);
		goto err;
	}
	nc = Dim_get($->dim,na-1);
	if (nc > nb) {
		whine("wrong number of elements in last dimension: "
			"got %d, expecting <= %d", nc, nb);
		goto err;
	}
	nd = nb - nc + na - 1;
	if (nd > MAX_DIMENSIONS) {
		whine("too many dimensions!");
		goto err;
	}
	for (i=0; i<na-1; i++) v[i] = Dim_get($->dim,i);
	for (i=nc; i<nb; i++) v[na-1+i-nc] = Dim_get(parent->dim,i);
	GridOutlet_begin(parent->out[0],Dim_new(nd,v));
	parent->buf = NEW2(Number,nc);
	parent->bufn = 0;
	whine("[r] %s",Dim_to_s(parent->out[0]->dim));
	return;
err:
	GridInlet_abort($);
}

GRID_FLOW(GridStore,0) {
	GridOutlet *out = parent->out[0];
	int na = Dim_count($->dim);
	int nb = Dim_count(parent->dim);
	int nc = Dim_get($->dim,na-1);

	while(n>0) {
		int bs = nc - parent->bufn;
		bs = n<bs?n:bs;
		n -= bs;
		
		while (bs--) parent->buf[parent->bufn++] = *data++;
		/*
		memcpy(&parent->buf[parent->bufn],data,sizeof(Number)*bs);
		parent->bufn += bs; data += bs; bs = 0;
		*/

		if (parent->bufn == nc) {
			int pos,size,i;
			int v[Dim_count(parent->dim)];
			for (i=0; i<nc; i++) {
				v[i] = mod(parent->buf[i],Dim_get(parent->dim,i));
			}
			for (; i<nb; i++) {
				v[i] = 0;
			}
			pos = Dim_calc_dex(parent->dim,v);
			size = Dim_prod_start(parent->dim,nc);
			GridOutlet_send_buffered(parent->out[0],size,&parent->data[pos]);

			parent->bufn=0;
		}
	}
	Dim_dex_add($->dim,n,&$->dex);
	GridOutlet_flush(parent->out[0]);
}

GRID_BEGIN(GridStore,1) {
	int length = Dim_prod($->dim);
	if (parent->data) {
		GridInlet_abort(parent->in[0]); /* bug? */
		free(parent->data);
	}
	parent->dim = $->dim;
	parent->data = NEW2(Number,length);
}

GRID_FLOW(GridStore,1) {
	int i;
	memcpy(&parent->data[$->dex], data, sizeof(Number)*n);
	Dim_dex_add($->dim,n,&$->dex);
}

METHOD(GridStore,init) {
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_new((GridObject *)$, 0,
		(GridBegin)GridStore_0_begin, (GridFlow)GridStore_0_flow);
	$->in[1] = GridInlet_new((GridObject *)$, 1,
		(GridBegin)GridStore_1_begin, (GridFlow)GridStore_1_flow);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->data = 0;
	$->dim  = 0;
}

METHOD(GridStore,delete) { /* write me */ }

METHOD(GridStore,bang) {
	if (! $->dim || ! $->data) {
		whine("empty buffer, better luck next time.");
		return;
	}
	GridOutlet_begin($->out[0],$->dim);
	GridOutlet_send($->out[0],Dim_prod($->dim),$->data);
}

CLASS(GridStore) {
	int i;
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t rien[] = { fts_t_symbol };
	MethodDecl methods[] = {
		{-1, fts_s_init,   METHOD_PTR(GridStore,init), ARRAY(rien),-1},
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

typedef struct GridOp1 GridOp1;
struct GridOp1 {
	GridObject_FIELDS;
	Operator1 *op;
};

GRID_BEGIN(GridOp1,0) {
	GridOutlet_begin(parent->out[0],$->dim);
	$->dex = 0;
}

GRID_FLOW(GridOp1,0) {
	int i;
	Number *data2 = NEW2(Number,n);
	GridOutlet *out = parent->out[0];

	memcpy(data2,data,sizeof(int)*n);
	parent->op->op_array(n,data2);
	$->dex += n;
	GridOutlet_send(out,n,data2);
	free(data2);
/*
	if ($->dex >= Dim_prod($->dim)) { GridInlet_end($); }
*/
}

METHOD(GridOp1,init) {
	fts_symbol_t sym = GET(1,symbol,op1_table[0].sym);

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_new((GridObject *)$, 0,
		(GridBegin)GridOp1_0_begin, (GridFlow)GridOp1_0_flow);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->op = op1_table_find(sym);
	if (!$->op) {
		fts_object_set_error(OBJ($),
			"unknown unary operator \"%s\"", fts_symbol_name(sym));
	}
}

METHOD(GridOp1,delete) { /* write me */ }

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

typedef struct GridOp2 GridOp2;
struct GridOp2 {
	GridObject_FIELDS;
	Operator2 *op;
	int rint;
	Number *data;
	Dim *dim;
};

GRID_BEGIN(GridOp2,0) {
	GridOutlet_begin(parent->out[0],$->dim);
	$->dex = 0;
}

GRID_FLOW(GridOp2,0) {
	Number *data2 = NEW2(Number,n);
	GridOutlet *out = parent->out[0];

	memcpy(data2,data,sizeof(int)*n);
	if (parent->dim) {
		int loop = Dim_prod(parent->dim);
		int i;
		for (i=0; i<n; i++) {
			int rint = parent->data[($->dex+i)%loop];
			data2[i] = parent->op->op(data2[i],rint);
		}
	} else {
		parent->op->op_array(n,data2,parent->rint);
	}
	$->dex += n;
	GridOutlet_send(out,n,data2);
	free(data2);
/*
	if ($->dex >= Dim_prod($->dim)) { GridInlet_end($); }
*/
}

GRID_BEGIN(GridOp2,1) {
	int length = Dim_prod($->dim);
	if (parent->data) {
		GridInlet_abort(parent->in[0]); /* bug? */
		free(parent->data);
	}
	parent->dim = $->dim;
	parent->data = NEW2(Number,length);
}

GRID_FLOW(GridOp2,1) {
	int i;
	memcpy(&parent->data[$->dex], data, sizeof(int)*n);
	Dim_dex_add($->dim,n,&$->dex);
}

METHOD(GridOp2,init) {
	fts_symbol_t sym = GET(1,symbol,op2_table[0].sym);
	$->rint = GET(2,int,0);

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_new((GridObject *)$, 0,
		(GridBegin)GridOp2_0_begin, (GridFlow)GridOp2_0_flow);
	$->in[1] = GridInlet_new((GridObject *)$, 1,
		(GridBegin)GridOp2_1_begin, (GridFlow)GridOp2_1_flow);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->op = op2_table_find(sym);
	if (!$->op) {
		fts_object_set_error(OBJ($),
			"unknown binary operator \"%s\"", fts_symbol_name(sym));
	}
}

METHOD(GridOp2,delete) { /* write me */ }

METHOD(GridOp2,rint) {
	free($->data);
	$->dim = 0;
	$->rint = GET(0,int,-42);
}

CLASS(GridOp2) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol, fts_t_symbol, fts_t_int };

	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridOp2,init),   ARRAY(init_args), 2 },
		{ -1, fts_s_delete, METHOD_PTR(GridOp2,delete), 0,0,0 },
		{  1, fts_s_int,    METHOD_PTR(GridOp2,rint),   0,0,0 },
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

  DOES NOT WORK YET
*/

typedef struct GridFold GridFold;
struct GridFold {
	GridObject_FIELDS;
	Operator2 *op;
	int rint;
	Number accum;
/*
	Number *data;
	Dim *dim;
*/
};

GRID_BEGIN(GridFold,0) {
	int n = Dim_count($->dim);
	Dim *foo;
	if (n<1) goto err;
	foo = Dim_new(n-1,$->dim->v);
	GridOutlet_begin(parent->out[0],foo);
err:
	GridInlet_abort($);
}

GRID_FLOW(GridFold,0) {
	int wrap = Dim_get($->dim,Dim_count($->dim)-1);
	int i;
	GridOutlet *out = parent->out[0];
	for (i=0; i<n; i++) {
		if ((($->dex+i) % wrap) == 0) parent->accum = parent->rint;
		parent->accum = parent->op->op_fold(parent->accum,1,&data[i]);
		if ((($->dex+i) % wrap) == wrap-1) {
			GridOutlet_send(out,1,&parent->accum);
		}
	}
	$->dex += n;
/*
	if ($->dex >= Dim_prod($->dim)) { GridInlet_end($); }
*/
}

/*
void GridFold_1_begin(GridInlet *$) {
	int length = Dim_prod($->dim);
	if (parent->data) {
		GridInlet_abort(parent->in[0]); (* bug? *)
		free(parent->data);
	}
	parent->dim = $->dim;
	parent->data = NEW2(Number,length);
}

void GridFold_1_flow(GridInlet *$, int n, const Number *data) {
	int i;
	memcpy(&parent->data[$->dex], data, sizeof(int)*n);
	Dim_dex_add($->dim,n,&$->dex);
}
*/

METHOD(GridFold,init) {
	int i;
	fts_symbol_t sym = GET(1,symbol,op2_table[0].sym);
	$->rint = GET(2,int,0);

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_new((GridObject *)$, 0,
		(GridBegin)GridOp2_0_begin, (GridFlow)GridOp2_0_flow);
	$->in[1] = GridInlet_new((GridObject *)$, 1,
		(GridBegin)GridOp2_1_begin, (GridFlow)GridOp2_1_flow);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->op = op2_table_find(sym);
	if (!$->op) {
		fts_object_set_error(OBJ($),
			"unknown binary operator \"%s\"", fts_symbol_name(sym));
	}
}

METHOD(GridFold,delete) { /* write me */ }

CLASS(GridFold) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol, fts_t_symbol, fts_t_int };

	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridFold,init),   ARRAY(init_args), 2 },
		{ -1, fts_s_delete, METHOD_PTR(GridFold,delete), 0,0,0 },
/*		{  1, fts_s_int,    METHOD_PTR(GridFold,rint),   0,0,0 }, */
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridFold), 2, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);
/*	GridObject_conf_class(class,1); */

	return fts_Success;
}

/* **************************************************************** */

void startup_grid_basic (void) {
	fts_class_install(fts_new_symbol("@import"), GridImport_class_init);
	fts_class_install(fts_new_symbol("@export"), GridExport_class_init);
	fts_class_install(fts_new_symbol("@store"),   GridStore_class_init);
	fts_class_install(fts_new_symbol("@!"),         GridOp1_class_init);
	fts_class_install(fts_new_symbol("@"),          GridOp2_class_init);
	fts_class_install(fts_new_symbol("@fold"),     GridFold_class_init);
}
