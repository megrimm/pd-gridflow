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

/* return and complain when file not open */
#define CHECK_FILE_OPEN \
	if (!$->ff) { whine("can't do that: file not open"); return; }

/* some data/type decls */

typedef struct VideoInFile VideoInFile;

struct VideoInFile {
	GridObject_FIELDS;
	FileFormat *ff; /* a file reader object */
};

/* ---------------------------------------------------------------- */

static void VideoInFile_p_reset(VideoInFile *$) {
	GridOutlet_abort($->out[0]);
	CHECK_FILE_OPEN
	$->ff->frame($->ff,-1);
}

/* ---------------------------------------------------------------- */

METHOD(VideoInFile,init) {
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
	$->ff = 0;
}

METHOD(VideoInFile,close) {
	CHECK_FILE_OPEN
	$->ff->close($->ff);
	$->ff = 0;
}

METHOD(VideoInFile,reset) {
	VideoInFile_p_reset($);
}

METHOD(VideoInFile,open) {
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
	if (!GridOutlet_idle($->out[0])) GridOutlet_abort($->out[0]);
	$->ff = qlass->open(fts_symbol_name(filename),4);
}

METHOD(VideoInFile,delete) {
	CHECK_FILE_OPEN
	$->ff->close($->ff);
}

METHOD(VideoInFile,bang) {
	Number *data;
	int i,n;
	Dim *dim;

	CHECK_FILE_OPEN

	dim = $->ff->frame($->ff,-1);
	if (!dim) {
		whine("file format says: no, no, no");
		goto err;
	}
	whine("file format handler says: %s", Dim_to_s(dim));
	GridOutlet_begin($->out[0],dim);

	n = Dim_prod(dim);
	while (n>0) {
		int maxbs = 16*1024;
		int bs = Dim_prod($->out[0]->dim) - $->out[0]->dex;
		if (bs > maxbs) bs = maxbs;
		if (bs > n) bs = n;
		data = $->ff->read($->ff,bs);
		if (!data) {
			whine("Frame data error?");
			goto err;
		}
		n -= bs;
		GridOutlet_send($->out[0],bs,data);
		free(data);
	}
	return;
err:
	return;
}

METHOD(VideoInFile,size) {
	int height = GET(0,int,320);
	int width = GET(1,int,240);

	CHECK_FILE_OPEN
	if ($->ff->size) {
		$->ff->size($->ff,height,width);
	} else {
		whine("can't set input size on this kind of format");
	}
}

/* ---------------------------------------------------------------- */

CLASS(VideoInFile) {
	fts_type_t rien[]       = { fts_t_symbol };
	fts_type_t open_args[]  = { fts_t_symbol, fts_t_symbol };
	fts_type_t size_args[]  = { fts_t_symbol, fts_t_int, fts_t_int };

	MethodDecl methods[] = {
		{-1,fts_s_init,  METHOD_PTR(VideoInFile,init),  ARRAY(rien),-1},
		{-1,fts_s_delete,METHOD_PTR(VideoInFile,delete),0,0,0},
		{ 0,fts_s_bang,  METHOD_PTR(VideoInFile,bang),  ARRAY(rien),-1},
		{ 0,sym_reset,   METHOD_PTR(VideoInFile,reset), ARRAY(rien),-1},
		{ 0,sym_open,    METHOD_PTR(VideoInFile,open),  ARRAY(open_args),-1},
		{ 0,sym_close,   METHOD_PTR(VideoInFile,close), 0,0,0},
		{ 0,fts_new_symbol("size"),METHOD_PTR(VideoInFile,size),ARRAY(size_args),-1},
	};

	/* initialize the class */
	fts_class_init(class, sizeof(VideoInFile), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));

	return fts_Success;
}

void VideoInFile_config(void) {
	fts_class_install(fts_new_symbol("@video_in_file"), VideoInFile_class_init);
}

