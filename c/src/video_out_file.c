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

#include "grid.h"

/* ---------------------------------------------------------------- */

/* return and complain when file not open */
#define CHECK_FILE_OPEN \
	if (!$->ff) { whine("can't do that: file not open"); return; }

/* same with false return */
#define CHECK_FILE_OPEN2 \
	if (!$->ff) { whine("can't do that: file not open"); return false; }

/* some data/type decls */

typedef struct VideoOutFile VideoOutFile;

struct VideoOutFile {
	GridObject_FIELDS;
	FileFormat *ff; /* a file writer object */
};

/* ---------------------------------------------------------------- */

GRID_BEGIN(VideoOutFile,0) {
	int v[] = { Dim_get(in->dim,0), Dim_get(in->dim,1), 3 };
	Dim *dim = Dim_new(ARRAY(v));
	if (Dim_equal_verbose_hwc(in->dim,dim)) {
		CHECK_FILE_OPEN2
		in->dex=0;
		$->ff->begin($->ff, in->dim);
		return true;
	} else {
		return false;
	}
}

GRID_FLOW(VideoOutFile,0) {
	FileFormat *f = $->ff;
	CHECK_FILE_OPEN
	while(n > 0) {
		int incr;
		int max = Dim_prod(in->dim) - in->dex;
		int bs = n<max?n:max;
		$->ff->flow($->ff, bs, data);
		
		data += bs;
		in->dex += bs;
		n -= bs;
		if (in->dex >= Dim_prod(in->dim)) {
			$->ff->end($->ff);
			fts_outlet_send(OBJ($),0,fts_s_bang,0,0);
		}
	}
}

GRID_END(VideoOutFile,0) {}

/* ---------------------------------------------------------------- */

METHOD(VideoOutFile,init) {
	whine("VideoOutFile#init");

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_NEW3($,VideoOutFile,0);
}

static void VideoOutFile_p_close(VideoOutFile *$) {
	CHECK_FILE_OPEN
	$->ff->close($->ff);
}

METHOD(VideoOutFile,close) {
	VideoOutFile_p_close($);
}

METHOD(VideoOutFile,open) {
	fts_symbol_t filename = GET(0,symbol,fts_new_symbol("/tmp/untitled.ppm"));
	const char *format = fts_symbol_name(GET(1,symbol,fts_new_symbol("ppm")));
	FileFormatClass *qlass = FileFormatClass_find(format);

	if (qlass) {
		whine("file format: %s (%s)",qlass->symbol_name, qlass->long_name);
	} else {
		whine("unknown file format identifier: %s", format);
		return;
	}

	if ($->ff) $->ff->close($->ff);
	/* if (!GridOutlet_idle($->out[0])) GridOutlet_abort($->out[0]); */
	$->ff = qlass->open(fts_symbol_name(filename),2);
}

METHOD(VideoOutFile,delete) {
	whine("VideoOutFile#delete");
	if ($->ff) { VideoOutFile_p_close($); }
}

METHOD(VideoOutFile,option) {
	CHECK_FILE_OPEN
	if ($->ff->option) {
		$->ff->option($->ff,ac,at);
	} else {
		whine("this format has no options");
	}
}

/* ---------------------------------------------------------------- */

CLASS(VideoOutFile) {
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t one_int[]    = { fts_t_symbol, fts_t_int };
	fts_type_t init_args[]  = { fts_t_symbol };
	fts_type_t open_args[]  = { fts_t_symbol, fts_t_symbol };
	fts_type_t option_args[]= { fts_t_symbol, fts_t_symbol, fts_t_int };

	MethodDecl methods[] = {
		{-1,fts_s_init,  METHOD_PTR(VideoOutFile,init),  ARRAY(init_args),-1},
		{-1,fts_s_delete,METHOD_PTR(VideoOutFile,delete),0,0,0 },
		{ 0,sym_open,    METHOD_PTR(VideoOutFile,open),  ARRAY(open_args),-1},
		{ 0,sym_close,   METHOD_PTR(VideoOutFile,close), 0,0,0 },
//		{ 0,SYM(frame),  METHOD_PTR(VideoOutFile,frame),  ARRAY(frame_args),-1},
		{ 0,SYM(option), METHOD_PTR(VideoOutFile,option), ARRAY(option_args),-1},
	};

	/* initialize the class */
	fts_class_init(class, sizeof(VideoOutFile), 1, 1, 0);
	GridObject_conf_class(class,0);
	define_many_methods(class,ARRAY(methods));

	/* !@#$ type the outlet */
	return fts_Success;
}

void startup_video_out_file (void) {
	fts_class_install(fts_new_symbol("@video_out_file"), VideoOutFile_class_init);
}

