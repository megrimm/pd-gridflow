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

#define INFO(__self) \
	fts_symbol_name(fts_get_class_name((__self)->parent->o.head.cl)), \
	(__self)->winlet

fts_type_t *make_dims_list (void) {
	int i;
	fts_type_t *dims = NEW(fts_type_t,MAX_DIMENSIONS+1);
	dims[0] = fts_t_symbol;
	for (i=0; i<MAX_DIMENSIONS; i++) {
		dims[i+1] = fts_t_int;
	}
	return dims;
}

/* **************** GridInlet ************************************* */

GridInlet *GridInlet_new(
	GridObject *parent, int winlet, GridAcceptor a, GridProcessor p)
{
	GridInlet *$ = NEW(GridInlet,1);
	$->parent = parent;
	$->winlet = winlet;
	$->dim = 0;
	$->dex = 0;
	$->acceptor = a;
	$->processor = p;
	return $;
}

GridObject *GridInlet_parent(GridInlet *$) {
	return $->parent;
}

void GridInlet_abort(GridInlet *$) {
	if ($->dim) {
		whine("%s:i%d: aborting grid: %d of %d", INFO($),
			$->dex, Dim_prod($->dim));
	}
	$->dim = 0;
	$->dex = 0;
}

void GridInlet_finish(GridInlet *$) {
	if ($->dim && Dim_prod($->dim) != $->dex) {
		whine("%s:i%d: finish: short end: %d of %d", INFO($),
			$->dex, Dim_prod($->dim));
	}
	$->dim = 0;
	$->dex = 0;
}

int GridInlet_idle_verbose(GridInlet *$, const char *where) {
	assert($);
	if (!$->dim) {
		whine("%s:i%d: no dim",INFO($));
	} else if (!$->processor) {
		whine("%s:i%d: no processor",INFO($));
	} else {
		return 0;
	}
	return 1;
}

void GridInlet_accept(GridInlet *$, int ac, const fts_atom_t *at) {
	int i;
	int *v = NEW(int,ac);

	for (i=0; i<ac; i++) v[i] = GET(i,int,0);

	$->dim = Dim_new(ac,v);
	free(v);

/*	whine("%s:i%d: grid_begin: %s",INFO($),Dim_to_s($->dim)); */
	$->dex = 0;
	if (!$->acceptor) {
		whine("%s:i%d: no acceptor",INFO($));
	} else {
		$->acceptor($);
	}
}

void GridInlet_packet(GridInlet *$, int ac, const fts_atom_t *at) {
	int n = GET(0,int,-1);
	const Number *data = (Number *) GET(1,ptr,(void *)0xDeadBeef);
	if (GridInlet_idle_verbose($,"packet")) return;
	assert(n>0);
	$->processor($,n,data);
}

void GridInlet_end(GridInlet *$, int ac, const fts_atom_t *at) {
	if (GridInlet_idle_verbose($,"end")) return;
	if (Dim_prod($->dim) != $->dex) {
		whine("%s:i%d: incomplete grid: %d of %d", INFO($),
			Dim_prod($->dim), $->dex);
	}
}

/* **************** GridOutlet ************************************ */

GridOutlet *GridOutlet_new(GridObject *parent, int woutlet) {
	GridOutlet *$ = NEW(GridOutlet,1);
	$->parent = parent;
	$->woutlet = woutlet;
	$->dim = 0;
	$->dex = 0;
	$->buf = NEW2(Number,PACKET_LENGTH);
	$->bufn = 0;
	return $;
}

GridObject *GridOutlet_parent(GridOutlet *$) {
	assert($);
	return $->parent;
}

int GridOutlet_idle(GridOutlet *$) {
	assert($);
	return !$->dim;
}

void GridOutlet_abort(GridOutlet *$) {
	assert($);
	$->dim = 0;
	$->dex = 0;
}

void GridOutlet_propose(GridOutlet *$, Dim *dim) {
	int i;
	int n = Dim_count(dim);
	fts_atom_t a[MAX_DIMENSIONS];

	assert($);

	$->dim = dim;
	$->dex = 0;
	for(i=0; i<n; i++) fts_set_int(&a[i],Dim_get($->dim,i));
	fts_outlet_send(
		OBJ($->parent),$->woutlet,sym_grid_begin,n,a);
}

void GridOutlet_send_direct(GridOutlet *$, int n, const Number *data) {
	int incr;

	assert(!GridOutlet_idle($));
	while(n>0) {
		int pn = n > PACKET_LENGTH ? PACKET_LENGTH : n;
		fts_atom_t a[2];
		int i;
		fts_set_int(a+0,pn);
		fts_set_ptr(a+1,data);
		fts_outlet_send(OBJ($->parent),0,sym_grid_packet,COUNT(a),a);
		data += pn;
		n -= pn;
	}
}

int GridOutlet_send(GridOutlet *$, int n, const Number *data) {
	int incr;

	assert(!GridOutlet_idle($));
	incr = Dim_dex_add($->dim,n,&$->dex);
	GridOutlet_send_direct($,n,data);
	return incr;
}

int GridOutlet_send_buffered(GridOutlet *$, int n, const Number *data) {
	int incr;

	assert(!GridOutlet_idle($));
	incr = Dim_dex_add($->dim,n,&$->dex);
	if ($->bufn + n > PACKET_LENGTH) {
		GridOutlet_flush($);
	}
	if (n > PACKET_LENGTH) {
		GridOutlet_send_direct($,n,data);
	} else {
		memcpy(&$->buf[$->bufn],data,sizeof(int)*n);
		$->bufn += n;
	}
	if (incr==0) {
		GridOutlet_flush($);
	}
	return incr;
}

void GridOutlet_flush(GridOutlet *$) {
	GridOutlet_send_direct($,$->bufn,$->buf);
	$->bufn = 0;
}

/* **************** GridPacket ************************************ */
/* for future use
	represents a connection between a GridOutlet and GridInlets
*/


/* **************** GridObject ************************************ */
/*
  abstract class for an FTS Object that has Grid-aware inlets/outlets
*/

METHOD(GridObject,init) {
	int i;
	for (i=0; i<MAX_INLETS;  i++) self->in[i]  = 0;
	for (i=0; i<MAX_OUTLETS; i++) self->out[i] = 0;
}

/* category: input */

METHOD(GridObject,grid_begin) {
	GridInlet_accept(self->in[winlet],ac,at);
}

METHOD(GridObject,grid_packet) {
	GridInlet_packet(self->in[winlet],ac,at);
}

METHOD(GridObject,grid_end) {
	GridInlet_end(self->in[winlet],ac,at);
}

void GridObject_conf_class(fts_class_t *class, int winlet) {
	fts_type_t int_alone[] = { fts_t_int };
	fts_type_t int_dims[MAX_DIMENSIONS+1] = { fts_t_symbol, };
	fts_type_t packet[] = { fts_t_symbol, fts_t_int, fts_t_ptr };
	MethodDecl methods[] = {
		{winlet,sym_grid_begin, METHOD_PTR(GridObject,grid_begin), ARRAY(int_dims),-1},
		{winlet,sym_grid_packet,METHOD_PTR(GridObject,grid_packet),ARRAY(packet),-1},
		{winlet,sym_grid_end,   METHOD_PTR(GridObject,grid_end),   0,0,0},
	};
	int i;
	for (i=0; i<MAX_DIMENSIONS; i++) int_dims[i+1] = fts_t_int;

	define_many_methods(class,ARRAY(methods));
}
