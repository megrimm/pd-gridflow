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

#include "video4jmax.h"
#include "grid.h"

/* ---------------------------------------------------------------- */

/* return and complain when file not open */
#define CHECK_FILE_OPEN \
	if (!$->ff) { whine("can't do that: file not open"); return; }

/* some data/type decls */

typedef struct VideoOutFile VideoOutFile;

struct VideoOutFile {
	GridObject_FIELDS;
	FileFormat *ff; /* a file writer object */
};

/* ---------------------------------------------------------------- */

void VideoOutFile_0_begin(GridInlet *$) {
	VideoOutFile *parent = (VideoOutFile *) GridInlet_parent($);
	int v[] = { Dim_get($->dim,0), Dim_get($->dim,1), 3 };
	Dim *dim = Dim_new(ARRAY(v));
	if (!Dim_equal_verbose_hwc($->dim,dim)) {
		GridInlet_abort($);
		return;
	}
	{
		VideoOutFile *$ = parent;
		CHECK_FILE_OPEN
	}
	$->dex=0;
	parent->ff->begin(parent->ff, $->dim);
}

void VideoOutFile_0_flow(GridInlet *$, int n, const Number *data) {
	VideoOutFile *parent = (VideoOutFile *) GridInlet_parent($);
	FileFormat *f = parent->ff;
	{
		VideoOutFile *$ = parent;
		CHECK_FILE_OPEN
	}
	while(n > 0) {
		int incr;
		int max = Dim_prod($->dim) - $->dex;
		int bs = n<max?n:max;
		parent->ff->flow(parent->ff, bs, data);
		
		data += bs;
		$->dex += bs;
		n -= bs;
		if ($->dex >= Dim_prod($->dim)) {
			parent->ff->end(parent->ff);
			fts_outlet_send(OBJ(parent),0,fts_s_bang,0,0);
		}
	}
}

/* ---------------------------------------------------------------- */

METHOD(VideoOutFile,init) {
	whine("VideoOutFile#init");

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_new((GridObject *)$, 0,
		VideoOutFile_0_begin, VideoOutFile_0_flow);
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

/* ---------------------------------------------------------------- */

CLASS(VideoOutFile) {
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t one_int[]    = { fts_t_symbol, fts_t_int };
	fts_type_t init_args[]  = { fts_t_symbol };
	fts_type_t open_args[]  = { fts_t_symbol, fts_t_symbol };
	MethodDecl methods[] = {
		{-1,fts_s_init,  METHOD_PTR(VideoOutFile,init),ARRAY(init_args),-1},
		{-1,fts_s_delete,METHOD_PTR(VideoOutFile,delete), 0,0,0 },
		{ 0,sym_open,    METHOD_PTR(VideoOutFile,open),ARRAY(open_args),-1},
		{ 0,sym_close,   METHOD_PTR(VideoOutFile,close), 0,0,0 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(VideoOutFile), 1, 1, 0);
	GridObject_conf_class(class,0);
	define_many_methods(class,ARRAY(methods));

	/* !@#$ type the outlet */
	return fts_Success;
}

void VideoOutFile_config (void) {
	fts_class_install(fts_new_symbol("@video_out_file"), VideoOutFile_instantiate);
}

