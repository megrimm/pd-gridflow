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

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "grid.h"

/* ---------------------------------------------------------------- */

/* return and complain when file not open */
#define CHECK_FILE_OPEN \
	if (!$->ff) { whine("can't do that: file not open"); return; }

/* same with false return */
#define CHECK_FILE_OPEN2 \
	if (!$->ff) { whine("can't do that: file not open"); return false; }

/* some data/type decls */

typedef struct VideoOut {
	GridObject_FIELDS;
	Format *ff; /* a file writer object */
} VideoOut;

/* ---------------------------------------------------------------- */

GRID_BEGIN(VideoOut,0) {
	int v[] = { Dim_get(in->dim,0), Dim_get(in->dim,1), 3 };
	Dim *dim = Dim_new(ARRAY(v));
	if (Dim_equal_verbose_hwc(in->dim,dim)) {
		CHECK_FILE_OPEN2
		in->dex=0;
		$->ff->cl->begin($->ff, in);
		return true;
	} else {
		return false;
	}
}

GRID_FLOW(VideoOut,0) {
	Format *f = $->ff;
	CHECK_FILE_OPEN
	$->ff->cl->flow($->ff, in, n, data);
}

GRID_END(VideoOut,0) {
	$->ff->cl->end($->ff,in);
	fts_outlet_send(OBJ($),0,fts_s_bang,0,0);
}


static void VideoOut_p_close(VideoOut *$) {
	CHECK_FILE_OPEN
	$->ff->cl->close($->ff);
}

METHOD(VideoOut,close) {
	VideoOut_p_close($);
}

METHOD(VideoOut,open) {
	fts_symbol_t filename = GET(0,symbol,fts_new_symbol("untitled.ppm"));
	const char *format = fts_symbol_name(GET(1,symbol,SYM("ppm")));
	FormatClass *qlass = FormatClass_find(format);

	if (qlass) {
		whine("file format: %s (%s)",qlass->symbol_name, qlass->long_name);
	} else {
		whine("unknown file format identifier: %s", format);
		return;
	}

	if ($->ff) $->ff->cl->close($->ff);
	/* if (!GridOutlet_idle($->out[0])) GridOutlet_abort($->out[0]); */
	if (qlass->open) {
		$->ff = qlass->open(qlass,fts_symbol_name(filename),2);
	} else {
		whine("file format has no `open'");
	}
}

METHOD(VideoOut,connect) {
	fts_symbol_t filename = GET(0,symbol,fts_new_symbol("untitled.ppm"));
	const char *format = fts_symbol_name(GET(1,symbol,SYM("ppm")));
	FormatClass *qlass = FormatClass_find(format);

	if (qlass) {
		whine("file format: %s (%s)",qlass->symbol_name, qlass->long_name);
	} else {
		whine("unknown file format identifier: %s", format);
		return;
	}

	if ($->ff) $->ff->cl->close($->ff);
	/* if (!GridOutlet_idle($->out[0])) GridOutlet_abort($->out[0]); */
	if (qlass->connect) {
		$->ff = qlass->connect(qlass,fts_symbol_name(filename),2);
	} else {
		whine("file format has no `connect'");
	}
}

METHOD(VideoOut,option) {
	CHECK_FILE_OPEN
	if ($->ff->cl->option) {
		$->ff->cl->option($->ff,ac,at);
	} else {
		whine("this format has no options");
	}
}

METHOD(VideoOut,init) {
	fts_atom_t at2[3];
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,VideoOut,0);
	fts_set_symbol(at2+0,SYM(here));
	fts_set_symbol(at2+1,SYM(x11));
	VideoOut_connect($,winlet,selector,2,at2);
	fts_set_symbol(at2+0,SYM(out_size));
	fts_set_int(at2+1,GET(1,int,0));
	fts_set_int(at2+2,GET(2,int,0));
	VideoOut_option($,winlet,selector,3,at2);
}

METHOD(VideoOut,delete) {
	if ($->ff) { VideoOut_p_close($); }
}

METHOD(VideoOut,bang) {
	fts_atom_t at2[1];
	fts_set_symbol(at2,SYM(draw));
	VideoOut_option($,winlet,selector,ARRAY(at2));
}

METHOD(VideoOut,autodraw) {
	fts_atom_t at2[2];
	fts_set_symbol(at2,SYM(autodraw));
	fts_set_int(at2+1,GET(0,int,0));
	VideoOut_option($,winlet,selector,ARRAY(at2));
}

/* ---------------------------------------------------------------- */

CLASS(VideoOut) {
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t one_int[]    = { fts_t_symbol, fts_t_int };
	fts_type_t init_args[]  = { fts_t_symbol, fts_t_int, fts_t_int };
	fts_type_t open_args[]  = { fts_t_symbol, fts_t_symbol };

	MethodDecl methods[] = {
		{-1,fts_s_init,  METHOD_PTR(VideoOut,init), ARRAY(init_args),-1},
		{-1,fts_s_delete,METHOD_PTR(VideoOut,delete), 0,0,0 },
		{ 0,fts_s_bang,  METHOD_PTR(VideoOut,bang), 0,0,0 },
		{ 0,SYM(connect),METHOD_PTR(VideoOut,connect),ARRAY(open_args),-1},
		{ 0,sym_autodraw,METHOD_PTR(VideoOut,autodraw),ARRAY(one_int),-1 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(VideoOut), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);

	return fts_Success;
}

void startup_video_out (void) {
	fts_class_install(fts_new_symbol("@video_out"), VideoOut_class_init);
}

