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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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

typedef struct GridIn {
	GridObject_FIELDS;
	Format *ff; /* a file reader object */
} GridIn;

typedef struct GridOut {
	GridObject_FIELDS;
	Format *ff; /* a file writer object */
} GridOut;

typedef struct VideoOut {
	GridObject_FIELDS;
	Format *ff; /* a file writer object */
} VideoOut;

/* ---------------------------------------------------------------- */

static void GridIn_p_reset(GridIn *$) {
	GridOutlet_abort($->out[0]);
	CHECK_FILE_OPEN
	/* $->ff->cl->frame($->ff,-1); */
}

METHOD(GridIn,init) {
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
	$->ff = 0;
}

METHOD(GridIn,close) {
	CHECK_FILE_OPEN
	$->ff->cl->close($->ff);
	$->ff = 0;
}

METHOD(GridIn,reset) {
	GridIn_p_reset($);
}

METHOD(GridIn,open) {
	fts_symbol_t filename = GET(0,symbol,fts_new_symbol("untitled.ppm"));
	const char *format = fts_symbol_name(GET(1,symbol,SYM(ppm)));
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
		$->ff = qlass->open(qlass,fts_symbol_name(filename),4);
	} else {
		whine("file format has no `open'");
	}
}

METHOD(GridIn,connect) {
	fts_symbol_t filename = GET(0,symbol,fts_new_symbol("untitled.ppm"));
	const char *format = fts_symbol_name(GET(1,symbol,SYM(ppm)));
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
		$->ff = qlass->connect(qlass,fts_symbol_name(filename),4);
	} else {
		whine("file format has no `connect'");
	}
}

METHOD(GridIn,delete) {
	if ($->ff) $->ff->cl->close($->ff);
	GridObject_delete((GridObject *)$);
}

METHOD(GridIn,bang) {
	CHECK_FILE_OPEN

	if (! $->ff->cl->frame($->ff,$->out[0],-1)) {
		whine("file format says: no, no, no");
		goto err;
	}
	return;
err:
	if (!GridOutlet_idle($->out[0])) GridOutlet_abort($->out[0]);
	return;
}

METHOD(GridIn,size) {
	int height = GET(0,int,320);
	int width = GET(1,int,240);

	CHECK_FILE_OPEN
	if ($->ff->cl->size) {
		$->ff->cl->size($->ff,height,width);
	} else {
		whine("can't set input size on this kind of format");
	}
}

METHOD(GridIn,option) {
	CHECK_FILE_OPEN
	if ($->ff->cl->option) {
		$->ff->cl->option($->ff,ac,at);
	} else {
		whine("this format has no options");
	}
}

CLASS(GridIn) {
	fts_type_t rien[]       = { fts_t_symbol };
	fts_type_t open_args[]  = { fts_t_symbol, fts_t_symbol };
	fts_type_t frame_args[] = { fts_t_symbol, fts_t_int };
	fts_type_t size_args[]  = { fts_t_symbol, fts_t_int, fts_t_int };
	fts_type_t option_args[]= { fts_t_symbol, fts_t_symbol, fts_t_int };

	MethodDecl methods[] = {
		{-1,fts_s_init,  METHOD_PTR(GridIn,init),  ARRAY(rien),-1},
		{-1,fts_s_delete,METHOD_PTR(GridIn,delete),0,0,0},
		{ 0,fts_s_bang,  METHOD_PTR(GridIn,bang),  ARRAY(rien),-1},
		{ 0,sym_reset,   METHOD_PTR(GridIn,reset), ARRAY(rien),-1},
		{ 0,sym_open,    METHOD_PTR(GridIn,open),  ARRAY(open_args),-1},
		{ 0,SYM(connect),METHOD_PTR(GridIn,connect),ARRAY(open_args),-1},
		{ 0,sym_close,   METHOD_PTR(GridIn,close), 0,0,0},
		{ 0,SYM(size),   METHOD_PTR(GridIn,size),  ARRAY(size_args),-1},
//		{ 0,SYM(frame),  METHOD_PTR(GridIn,frame), ARRAY(frame_args),-1},
		{ 0,SYM(option), METHOD_PTR(GridIn,option),ARRAY(option_args),-1},
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridIn), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));

	return fts_Success;
}

/* ---------------------------------------------------------------- */

GRID_BEGIN(GridOut,0) {
	Dim *dim = Dim_dup(in->dim);
	if (Dim_count(dim) != 3) {
		whine("supports only exactly three dimensions");
		return false;
	}
	if (Dim_equal_verbose_hwc(in->dim,dim)) {
		CHECK_FILE_OPEN2
		in->dex=0;
		$->ff->cl->begin($->ff, in);
		return true;
	} else {
		return false;
	}
}

GRID_FLOW(GridOut,0) {
	Format *f = $->ff;
	CHECK_FILE_OPEN
	$->ff->cl->flow($->ff, in, n, data);
}

GRID_END(GridOut,0) {
	$->ff->cl->end($->ff,in);
	LEAVE;
	fts_outlet_send(OBJ($),0,fts_s_bang,0,0);
	ENTER;
}

METHOD(GridOut,init) {
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,GridOut,0);
}

METHOD(GridOut,close) {
	CHECK_FILE_OPEN
	$->ff->cl->close($->ff);
}

METHOD(GridOut,open) {
	fts_symbol_t filename = GET(0,symbol,fts_new_symbol("untitled.ppm"));
	const char *format = fts_symbol_name(GET(1,symbol,SYM(ppm)));
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

METHOD(GridOut,connect) {
	fts_symbol_t filename = GET(0,symbol,fts_new_symbol("untitled.ppm"));
	const char *format = fts_symbol_name(GET(1,symbol,SYM(ppm)));
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

METHOD(GridOut,delete) {
	if ($->ff) $->ff->cl->close($->ff);
	GridObject_delete((GridObject *)$);
}

METHOD(GridOut,option) {
	CHECK_FILE_OPEN
	if ($->ff->cl->option) {
		$->ff->cl->option($->ff,ac,at);
	} else {
		whine("this format has no options");
	}
}

CLASS(GridOut) {
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t one_int[]    = { fts_t_symbol, fts_t_int };
	fts_type_t init_args[]  = { fts_t_symbol };
	fts_type_t open_args[]  = { fts_t_symbol, fts_t_symbol };
	fts_type_t option_args[]= { fts_t_symbol, fts_t_symbol, fts_t_int };

	MethodDecl methods[] = {
		{-1,fts_s_init,  METHOD_PTR(GridOut,init),  ARRAY(init_args),-1},
		{-1,fts_s_delete,METHOD_PTR(GridOut,delete),0,0,0 },
		{ 0,sym_open,    METHOD_PTR(GridOut,open),  ARRAY(open_args),-1},
		{ 0,SYM(connect),METHOD_PTR(GridOut,connect),ARRAY(open_args),-1},
		{ 0,sym_close,   METHOD_PTR(GridOut,close), 0,0,0 },
//		{ 0,SYM(frame),  METHOD_PTR(GridOut,frame),  ARRAY(frame_args),-1},
		{ 0,SYM(option), METHOD_PTR(GridOut,option), ARRAY(option_args),-1},
	};

	/* initialize the class */
	fts_class_init(class, sizeof(GridOut), 1, 1, 0);
	GridObject_conf_class(class,0);
	define_many_methods(class,ARRAY(methods));

	/* !@#$ type the outlet */
	return fts_Success;
}

/* ---------------------------------------------------------------- */

GRID_BEGIN(VideoOut,0) {
	Dim *dim = Dim_dup(in->dim);
	if (Dim_count(dim) != 3) {
		whine("supports only exactly three dimensions");
		return false;
	}
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
	LEAVE;
	fts_outlet_send(OBJ($),0,fts_s_bang,0,0);
	ENTER;
}

METHOD(VideoOut,close) {
	CHECK_FILE_OPEN
	$->ff->cl->close($->ff);
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
	if ($->ff) $->ff->cl->close($->ff);
	GridObject_delete((GridObject *)$);
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

/* ---------------------------------------------------------------- */

void startup_io (void) {
	fts_class_install(fts_new_symbol("@in"),  GridIn_class_init);
	fts_class_install(fts_new_symbol("@out"), GridOut_class_init);
	fts_class_alias(fts_new_symbol("@video_in_file"),fts_new_symbol("@in"));
	fts_class_alias(fts_new_symbol("@video_out_file"),fts_new_symbol("@out"));
	fts_class_install(fts_new_symbol("@video_out"),  VideoOut_class_init);
}
