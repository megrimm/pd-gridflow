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
#include "grid.h"

FormatClass *format_classes[] = { FORMAT_LIST(&,class_) };

#define INFO(_self_) \
	fts_symbol_name(fts_get_class_name((_self_)->parent->o.head.cl)), \
	(_self_)->winlet

/* **************** GridInlet ************************************* */

GridInlet *GridInlet_new(GridObject *parent, int winlet,
	GridBegin b, GridFlow f, GridEnd e, int mode
) {
	GridInlet *$ = NEW(GridInlet,1);
	$->parent = parent;
	$->winlet = winlet;
	$->dim   = 0;
	$->dex   = 0;
	$->begin = b;
	$->flow  = mode==4 ? f : 0;
	$->flow2 = (GridFlow2)(mode==6 ? f : 0);
	$->end   = e;
	$->factor= 1;
	$->buf   = 0;
	$->bufn  = 0;
	$->mode  = mode;
	return $;
}

void GridInlet_delete(GridInlet *$) {
	FREE($->dim);
	FREE($->buf);
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

bool GridInlet_idle(GridInlet *$) {
	assert($);
	return !$->dim;
}

bool GridInlet_idle_verbose(GridInlet *$, const char *where) {
	assert($);
	if (!$->dim) {
		whine("%s:i%d(%s): no dim", INFO($), where);
	} else if (!$->flow && !$->flow2) {
		whine("%s:i%d(%s): no flow()", INFO($), where);
	} else {
		return 0;
	}
	return 1;
}

void GridInlet_set_factor(GridInlet *$, int factor) {
	assert(factor > 0);
	assert(Dim_prod($->dim) % factor == 0);
	$->factor = factor;
	FREE($->buf);
	if (factor > 1) {
		$->buf = NEW(Number,factor);
		$->bufn = 0;
	}
}

void GridInlet_begin(GridInlet *$, ATOMLIST) {
	int i;
	int *v = NEW(int,ac-1);
	GridOutlet *back_out = (GridOutlet *) GET(0,ptr,(void *)0xDeadBeef);

/* if (!GridInlet_idle($)) { then what do i do? } */

	for (i=0; i<ac-1; i++) v[i] = GET(i+1,int,0);
	$->dim = Dim_new(ac-1,v);
	FREE(v);

/*	whine("%s:i%d: grid_begin: %s",INFO($),Dim_to_s($->dim)); */
	$->dex = 0;
	assert($->begin);
	if (!$->begin((GridObject *)$->parent,$)) { GridInlet_abort($); }

	GridOutlet_callback((GridOutlet *)back_out,$,$->mode);
}

void GridInlet_flow(GridInlet *$, ATOMLIST) {
	int n = GET(0,int,-1);
	const Number *data = (Number *) GET(1,ptr,(void *)0xDeadBeef);
	if (GridInlet_idle_verbose($,"flow")) return;
	assert(n>0);
	{
		int d = $->dex + $->bufn;
		if (d+n > Dim_prod($->dim)) {
			whine("%s:i%d: grid input overflow: %d of %d",
				INFO($), d+n, Dim_prod($->dim));
			n = Dim_prod($->dim) - d;
			if (n<=0) return;
		}
		if ($->buf && $->bufn) {
			while (n && $->bufn<$->factor) { $->buf[$->bufn++] = *data++; n--; }
			if ($->bufn == $->factor) {
				int newdex = $->dex + $->factor;
				if ($->flow2) {
					Number *data2 = NEW2(Number,$->factor);
					memcpy(data2,$->buf,$->factor*sizeof(Number));
					$->flow2((GridObject *)$->parent,$,$->factor,data2);
				} else {
					$->flow((GridObject *)$->parent,$,$->factor,$->buf);
				}
				$->dex = newdex;
				$->bufn = 0;
			}
		}
		{
			int m = (n / $->factor) * $->factor;
			int newdex = $->dex + m;
			if (m) {
				if ($->flow2) {
					Number *data2 = NEW2(Number,m);
					memcpy(data2,data,m*sizeof(Number));
					$->flow2((GridObject *)$->parent,$,m,data2);
				} else {
					$->flow((GridObject *)$->parent,$,m,data);
				}
			}
			$->dex = newdex;
			data += m;
			n -= m;
		}
		if ($->buf) { while (n) { $->buf[$->bufn++] = *data++; n--; }}
	}
}

void GridInlet_flow2(GridInlet *$, ATOMLIST) {
	int n = GET(0,int,-1);
	Number *data = (Number *) GET(1,ptr,(void *)0xDeadBeef);
	if (GridInlet_idle_verbose($,"flow2")) return;
	assert(n>0);
	assert($->factor==1);
	assert($->flow || $->flow2);
	if ($->flow2) {
		$->flow2((GridObject *)$->parent,$,n,data);
	} else if ($->flow) {
		$->flow((GridObject *)$->parent,$,n,data);
		FREE(data);
	}
}

void GridInlet_end(GridInlet *$, ATOMLIST) {
	if (GridInlet_idle_verbose($,"end")) return;
/*	whine("%s:i%d: GridInlet_end()", INFO($)); */
	if (Dim_prod($->dim) != $->dex) {
		whine("%s:i%d: incomplete grid: %d of %d", INFO($),
			$->dex, Dim_prod($->dim));
	}
	if ($->end) { $->end((GridObject *)$->parent,$); }
	FREE($->dim);
	$->dex = 0;
}

void GridInlet_list(GridInlet *$, ATOMLIST) {
	int i;
	Number *v = NEW(Number,ac);
	int n = ac;
	for (i=0; i<ac; i++) v[i] = GET(i,int,0);
	$->dim = Dim_new(1,&n);

	assert($->begin);
	if ($->begin((GridObject *)$->parent,$)) {
		$->flow((GridObject *)$->parent,$,n,v);
		if ($->end) { $->end((GridObject *)$->parent,$); }
	} else {
		GridInlet_abort($);
	}

	FREE(v);
	FREE($->dim);
	$->dex = 0;
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
	$->frozen = 0;
	$->ron = 0;
	$->ro = 0;
	$->rwn = 0;
	$->rw = 0;
	return $;
}

void GridOutlet_delete(GridOutlet *$) {
	FREE($->dim);
	FREE($->buf);
}

GridObject *GridOutlet_parent(GridOutlet *$) {
	assert($);
	return $->parent;
}

bool GridOutlet_idle(GridOutlet *$) {
	assert($);
	return !$->dim;
}

void GridOutlet_abort(GridOutlet *$) {
	assert($);
	if (GridOutlet_idle($)) return;
	FREE($->dim);
	$->dim = 0;
	$->dex = 0;
	LEAVE_P;
	fts_outlet_send(OBJ($->parent),$->woutlet,sym_grid_end,0,0);
	ENTER_P;
}

void GridOutlet_end(GridOutlet *$) {
	assert($);
	assert(!GridOutlet_idle($));
	GridOutlet_flush($);
	LEAVE_P;
	fts_outlet_send(OBJ($->parent),$->woutlet,sym_grid_end,0,0);
	ENTER_P;
	FREE($->dim);
	$->dim = 0;
	$->dex = 0;
}

void GridOutlet_begin(GridOutlet *$, Dim *dim) {
	int i;
	int winlet = $->woutlet; /* hack */
	int n = Dim_count(dim);
	fts_atom_t a[MAX_DIMENSIONS+1];

	assert($);

/*	whine("%s:o%d: beginning a %s",INFO((GridInlet *)$), Dim_to_s(dim)); */

	/* if (!GridOutlet_idle($)) GridOutlet_abort($); */

	$->dim = dim;
	$->dex = 0;
	$->frozen = 0;
	$->ron = 0;
	$->ro  = 0;
	$->rwn = 0;
	$->rw  = 0;
	fts_set_ptr(a,$);
	for(i=0; i<n; i++) fts_set_int(&a[i+1],Dim_get($->dim,i));
	LEAVE_P;
	fts_outlet_send(OBJ($->parent),$->woutlet,sym_grid_begin,n+1,a);
	ENTER_P;
	$->frozen = 1;
/*	whine("$ = %p; $->ron = %d; $->rwn = %d", $, $->ron, $->rwn); */
}

void GridOutlet_send_direct(GridOutlet *$, int n, const Number *data) {
	int incr;

	assert(!GridOutlet_idle($));
	while(n>0) {
		int pn = n > PACKET_LENGTH ? PACKET_LENGTH : n;
		fts_atom_t a[2];
		int i;
		fts_set_int(a+0,pn);
		fts_set_ptr(a+1,(void*)(long)data); /* explicitly removing const */
		LEAVE_P;
		fts_outlet_send(OBJ($->parent),0,sym_grid_flow,COUNT(a),a);
		ENTER_P;
		data += pn;
		n -= pn;
	}
}

void GridOutlet_send(GridOutlet *$, int n, const Number *data) {
	assert(!GridOutlet_idle($));
	$->dex += n;
	assert($->dex <= Dim_prod($->dim));
	if (n > PACKET_LENGTH/2 || $->bufn + n > PACKET_LENGTH) {
		GridOutlet_flush($);
	}
	if (n > PACKET_LENGTH/2) {
		GridOutlet_send_direct($,n,data);
	} else {
		memcpy(&$->buf[$->bufn],data,n*sizeof(Number));
		$->bufn += n;
	}
}

void GridOutlet_give(GridOutlet *$, int n, Number *data) {
	assert(!GridOutlet_idle($));
	$->dex += n;
	assert($->dex <= Dim_prod($->dim));
	GridOutlet_flush($);
	if ($->ron == 0 && $->rwn == 1) {
		/* this is the copyless buffer passing */
		fts_atom_t a[2];
		fts_set_int(a+0,n);
		fts_set_ptr(a+1,(void*)data);
		LEAVE_P;
		fts_outlet_send(OBJ($->parent),0,sym_grid_flow2,COUNT(a),a);
		ENTER_P;
	} else {
		/* normal stuff */
		GridOutlet_send_direct($,n,data);
		FREE(data);
	}
}

void GridOutlet_flush(GridOutlet *$) {
	assert(!GridOutlet_idle($));
	GridOutlet_send_direct($,$->bufn,$->buf);
	$->bufn = 0;
}

void GridOutlet_callback(GridOutlet *$, GridInlet *in, int mode) {
	assert(!GridOutlet_idle($));
	assert(!$->frozen);
	assert(mode==6 || mode==4);
	/* whine("callback: outlet=%p, inlet=%p, mode=%d",$,in,mode); */
	/* not using ->ro, ->rw yet */
	if (mode==4) $->ron += 1;
	if (mode==6) $->rwn += 1;
}

/* **************** GridObject ************************************ */
/*
  abstract class for an FTS Object that has Grid-aware inlets/outlets
*/

METHOD(GridObject,init) {
	int i;
	for (i=0; i<MAX_INLETS;  i++) $->in[i]  = 0;
	for (i=0; i<MAX_OUTLETS; i++) $->out[i] = 0;
	ObjectSet_add(video4jmax_object_set,$);
	$->profiler_cumul = 0;
}

/* category: input */

METHOD(GridObject,grid_begin){ENTER;GridInlet_begin($->in[winlet],ac,at);LEAVE;}
METHOD(GridObject,grid_flow ){ENTER;GridInlet_flow( $->in[winlet],ac,at);LEAVE;}
METHOD(GridObject,grid_flow2){ENTER;GridInlet_flow2($->in[winlet],ac,at);LEAVE;}
METHOD(GridObject,grid_end  ){ENTER;GridInlet_end(  $->in[winlet],ac,at);LEAVE;}
METHOD(GridObject,list      ){ENTER;GridInlet_list( $->in[winlet],ac,at);LEAVE;}

void GridObject_delete(GridObject *$) {
	int i;
	for (i=0; i<MAX_INLETS;  i++) {
/*		GridInlet_delete($->in[i]); */
		FREE($->in[i]);
	}
	for (i=0; i<MAX_OUTLETS; i++) {
/*		GridOutlet_delete($->out[i]); */
		FREE($->out[i]);
	}
	ObjectSet_del(video4jmax_object_set,$);
}

void GridObject_conf_class(fts_class_t *class, int winlet) {
	fts_type_t int_alone[] = { fts_t_int };
	fts_type_t int_dims[MAX_DIMENSIONS+2] = { fts_t_symbol, fts_t_ptr, };
	fts_type_t packet[] = { fts_t_symbol, fts_t_int, fts_t_ptr };
	fts_type_t rien[] = { fts_t_symbol };
	fts_type_t list[] = { fts_t_list };
	MethodDecl methods[] = {
		{winlet,sym_grid_begin,METHOD_PTR(GridObject,grid_begin),ARRAY(int_dims),2},
		{winlet,sym_grid_flow, METHOD_PTR(GridObject,grid_flow), ARRAY(packet),-1},
		{winlet,sym_grid_flow2,METHOD_PTR(GridObject,grid_flow2),ARRAY(packet),-1},
		{winlet,sym_grid_end,  METHOD_PTR(GridObject,grid_end), ARRAY(rien),-1},
		{winlet,fts_s_list,    METHOD_PTR(GridObject,list),ARRAY(list),-1},
	};
	int i;
	for (i=0; i<MAX_DIMENSIONS; i++) int_dims[i+2] = fts_t_int;
	define_many_methods(class,ARRAY(methods));
}

/* **************** Format **************************************** */
/* this is an abstract base class for file formats, network protocols, etc */

/* **************** FormatClass *********************************** */

FormatClass *FormatClass_find(const char *name) {
	int i;
	for (i=0; i<COUNT(format_classes); i++) {
		/* whine("comparing `%s' with `%s'", name,
			format_classes[i]->symbol_name); */
		if (strcmp(format_classes[i]->symbol_name,name)==0) {
			return format_classes[i];
		}
	}
	return 0; /* fail */
}

