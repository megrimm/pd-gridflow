/*
	Video4jmax
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file LICENSE for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>

#include "video4jmax.h"
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

METHOD(GridImport,delete) {
	/* write me */
}

METHOD(GridImport,int) {
	Number data[] = { GET(0,int,0) };
	GridOutlet *out = $->out[0];
	if (GridOutlet_idle(out)) GridOutlet_propose(out,$->dim);
	GridOutlet_send(out,COUNT(data),data);
//	if (out->dex >= Dim_prod(out->dim)) GridOutlet_abort(out);
}

METHOD(GridImport,reset) {
	GridOutlet *out = $->out[0];
	GridOutlet_abort(out);
}

CLASS(GridImport) {
	int i;
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t int_dims[MAX_DIMENSIONS+1] = { fts_t_symbol };
	MethodDecl methods[] = {
		{-1, fts_s_init,  METHOD_PTR(GridImport,init),   ARRAY(int_dims),1},
		{-1, fts_s_delete,METHOD_PTR(GridImport,delete), 0,0,0},
		{ 0, fts_s_int,   METHOD_PTR(GridImport,int),    ARRAY(int_alone),-1},
		{ 0, sym_reset,   METHOD_PTR(GridImport,reset), 0,0,-1},
	};

	for (i=0; i<MAX_DIMENSIONS; i++) int_dims[i+1] = fts_t_int;

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
	Dim *dim;
};

void GridExport_acceptor(GridInlet *$) {
	/* norhing to do */
}

void GridExport_processor(GridInlet *$, int n, const Number *data) {
	int i;
/*	whine("grfunction(%p,%d,%p): %d",$,n,data,*data);*/
	for (i=0; i<n; i++) {
		fts_atom_t a[1];
		fts_set_int(a+0,*data);
		fts_outlet_send(OBJ($),0,fts_s_int,1,a);
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
		GridExport_acceptor,
		GridExport_processor);

	for (i=0; i<ac-1; i++) {
		v[i] = GET(i+1,int,0);
		COERCE_INT_INTO_RANGE(v[i],1,MAX_INDICES);
	}
	$->dim = Dim_new(ac-1,v);
	whine("%s",Dim_to_s($->dim));
}

METHOD(GridExport,delete) {
	/* write me */
}

CLASS(GridExport) {
	int i;
	fts_type_t int_dims[MAX_DIMENSIONS+1] = { fts_t_symbol };
	MethodDecl methods[] = {
		{-1, fts_s_init,   METHOD_PTR(GridExport,init), ARRAY(int_dims),2 },
		{-1, fts_s_delete, METHOD_PTR(GridExport,delete), 0,0,0 },
	};

	for (i=0; i<MAX_DIMENSIONS; i++) int_dims[i+1] = fts_t_int;

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

void GridStore_acceptor0(GridInlet *$) {
	GridStore *parent = (GridStore *) GridInlet_parent($);
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
	GridOutlet_propose(parent->out[0],Dim_new(nd,v));
	parent->buf = NEW2(Number,nc);
	parent->bufn = 0;
	whine("[r] %s",Dim_to_s(parent->out[0]->dim));
	return;
err:
	GridInlet_abort($);
}

static inline int mod(int a, int b) {
	if (a<0) a += b * (1-(a/b));
	return a%b;
/*
	int r = a%b;
	r += (-( (r<0) ^ (b<0) )) & y;
*/
}

void GridStore_processor0(GridInlet *$, int n, const Number *data) {
	GridStore *parent = (GridStore *) GridInlet_parent($);
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

void GridStore_acceptor1(GridInlet *$) {
	GridStore *parent = (GridStore *) GridInlet_parent($);
	int length = Dim_prod($->dim);
	if (parent->data) {
		GridInlet_abort(parent->in[0]);
		free(parent->data);
	}
	parent->dim = $->dim;
	parent->data = NEW2(Number,length);
}

void GridStore_processor1(GridInlet *$, int n, const Number *data) {
	GridStore *parent = (GridStore *) GridInlet_parent($);
	int i;
	memcpy(&parent->data[$->dex], data, sizeof(Number)*n);
	Dim_dex_add($->dim,n,&$->dex);
}

METHOD(GridStore,init) {
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_new((GridObject *)$, 0,
		GridStore_acceptor0, GridStore_processor0);
	$->in[1] = GridInlet_new((GridObject *)$, 1,
		GridStore_acceptor1, GridStore_processor1);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	$->data = 0;
	$->dim  = 0;
}

METHOD(GridStore,delete) {
}

METHOD(GridStore,bang) {
	if (! $->dim || ! $->data) {
		whine("empty buffer, better luck next time.");
		return;
	}
	GridOutlet_propose($->out[0],$->dim);
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

#define DEF_OP(__name,__expr) \
	static Number op_##__name (Number a, Number b) { return __expr; } \
	static void op_array_##__name (int n, Number *as, Number b) { \
		while (n--) { Number a = *as; *as++ = __expr; } }

DEF_OP(add, a+b)
DEF_OP(sub, a-b)
DEF_OP(bus, b-a)

DEF_OP(mul, a*b)
DEF_OP(div, b==0 ? 0 : a/b)
DEF_OP(vid, a==0 ? 0 : b/a)
DEF_OP(mod, b==0 ? 0 : a%b)
DEF_OP(dom, a==0 ? 0 : b%a)

DEF_OP(or , a|b)
DEF_OP(xor, a^b)
DEF_OP(and, a&b)
DEF_OP(shl, a<<b)
DEF_OP(shr, a>>b)

DEF_OP(min, a<b?a:b)
DEF_OP(max, a>b?a:b)

DEF_OP(eq,  a == b);
DEF_OP(ne,  a != b);
DEF_OP(gt,  a >  b);
DEF_OP(le,  a <= b);
DEF_OP(lt,  a <  b);
DEF_OP(ge,  a >= b);

#define DECLOP(__name,__sym) \
	{ 0, __sym, &op_##__name, &op_array_##__name }

typedef struct Operation Operation;
struct Operation {
	fts_symbol_t sym;
	const char *name;
	Number (*op)(Number,Number);
	void (*op_array)(int,Number *,Number);
};

Operation optable[] = {
	DECLOP(add, "+"),
	DECLOP(sub, "-"),
	DECLOP(bus, "inv+"),

	DECLOP(mul, "*"),
	DECLOP(div, "/"),
	DECLOP(vid, "inv*"),
	DECLOP(mod, "%"),
	DECLOP(dom, "rev%"),

	DECLOP(or , "|"),
	DECLOP(xor, "^"),
	DECLOP(and, "&"),
	DECLOP(shl, "<<"),
	DECLOP(shr, ">>"),

	DECLOP(min, "min"),
	DECLOP(max, "max"),

	DECLOP(eq,  "=="),
	DECLOP(ne,  "!="),
	DECLOP(gt,  ">"),
	DECLOP(le,  "<="),
	DECLOP(lt,  "<"),
	DECLOP(ge,  ">="),
};

/* **************************************************************** */
/*
  GridOp1 ("@") is the class of objects for operating on an array
  in a left inlet, and a scalar (integer) alone in the right inlet.
  They support operators listed in the section above.
*/

typedef struct GridOp1 GridOp1;
struct GridOp1 {
	GridObject_FIELDS;
	Operation *op;
	int rint;
	Number *data;
	Dim *dim;
};

void GridOp1_acceptor0(GridInlet *$) {
	GridOp1 *parent = (GridOp1 *) GridInlet_parent($);
	GridOutlet_propose(parent->out[0],$->dim);
	$->dex = 0;
}

void GridOp1_processor0(GridInlet *$, int n, const Number *data) {
	int i;
	Number *data2 = NEW2(Number,n);
	GridOp1 *parent = (GridOp1 *) GridInlet_parent($);
	GridOutlet *out = parent->out[0];

	memcpy(data2,data,sizeof(int)*n);
	if (parent->dim) {
		int loop = Dim_prod(parent->dim);
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
	if ($->dex >= Dim_prod($->dim)) {
		GridInlet_finish($);
	}
*/
}

void GridOp1_acceptor1(GridInlet *$) {
	GridOp1 *parent = (GridOp1 *) GridInlet_parent($);
	int length = Dim_prod($->dim);
	if (parent->data) {
		GridInlet_abort(parent->in[0]);
		free(parent->data);
	}
	parent->dim = $->dim;
	parent->data = NEW2(Number,length);
}

void GridOp1_processor1(GridInlet *$, int n, const Number *data) {
	GridOp1 *parent = (GridOp1 *) GridInlet_parent($);
	int i;
	memcpy(&parent->data[$->dex], data, sizeof(int)*n);
	Dim_dex_add($->dim,n,&$->dex);
}

METHOD(GridOp1,init) {
	int i;
	fts_symbol_t sym = GET(1,symbol,optable[0].sym);
	$->rint = GET(2,int,0);

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_new((GridObject *)$, 0,
		GridOp1_acceptor0, GridOp1_processor0);
	$->in[1] = GridInlet_new((GridObject *)$, 1,
		GridOp1_acceptor1, GridOp1_processor1);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);

	for(i=0; i<COUNT(optable); i++) {
		if (optable[i].sym == sym) {
			whine("created @ object with operator '%s'", optable[i].name);
			$->op = &optable[i];
			return;
		}
	}

	//!@#$
	whine("unknown operator, trying '+' instead");
	$->op = &optable[0];
}

METHOD(GridOp1,delete) {
	/* write me */
}

METHOD(GridOp1,rint) {
	free($->data);
	$->dim = 0;
	$->rint = GET(0,int,-42);
}

METHOD(GridOp1,bang) {
	if (! $->dim || ! $->data) {
		whine("empty buffer, better luck next time.");
		return;
	}
	GridOutlet_propose($->out[0],$->dim);
	GridOutlet_send($->out[0],Dim_prod($->dim),$->data);
}


CLASS(GridOp1) {
	int i;
	fts_type_t init_args[]  = { fts_t_symbol, fts_t_symbol, fts_t_int };

	MethodDecl methods[] = {
		{ -1, fts_s_init,   METHOD_PTR(GridOp1,init),   ARRAY(init_args), 2 },
		{ -1, fts_s_delete, METHOD_PTR(GridOp1,delete), 0,0,0 },
		{  1, fts_s_int,    METHOD_PTR(GridOp1,rint),   0,0,0 },
		{  1, fts_s_bang,   METHOD_PTR(GridOp1,bang),   0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridOp1), 2, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);
	GridObject_conf_class(class,1);

	return fts_Success;
}

/* **************************************************************** */

void grid_basic_config (void) {
	int i;
	for(i=0; i<COUNT(optable); i++) {
		optable[i].sym = fts_new_symbol(optable[i].name);
	} 

	fts_class_install(fts_new_symbol("@import"), GridImport_instantiate);
	fts_class_install(fts_new_symbol("@export"), GridExport_instantiate);
	fts_class_install(fts_new_symbol("@store"),   GridStore_instantiate);
	fts_class_install(fts_new_symbol("@"),          GridOp1_instantiate);
}
