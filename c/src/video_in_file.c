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

typedef struct VideoInFile {
	GridObject_FIELDS;
	FileFormat *ff; /* a file reader object */
} VideoInFile;

/* ---------------------------------------------------------------- */

static void VideoInFile_p_reset(VideoInFile *$) {
	GridOutlet_abort($->out[0]);
	CHECK_FILE_OPEN
	/* $->ff->cl->frame($->ff,-1); */
}

/* ---------------------------------------------------------------- */

METHOD(VideoInFile,init) {
	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
	$->ff = 0;
}

METHOD(VideoInFile,close) {
	CHECK_FILE_OPEN
	$->ff->cl->close($->ff);
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

	if ($->ff) $->ff->cl->close($->ff);
	if (!GridOutlet_idle($->out[0])) GridOutlet_abort($->out[0]);
	$->ff = qlass->open(qlass,fts_symbol_name(filename),4);
}

METHOD(VideoInFile,delete) {
	CHECK_FILE_OPEN
	$->ff->cl->close($->ff);
}

METHOD(VideoInFile,bang) {
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

METHOD(VideoInFile,size) {
	int height = GET(0,int,320);
	int width = GET(1,int,240);

	CHECK_FILE_OPEN
	if ($->ff->cl->size) {
		$->ff->cl->size($->ff,height,width);
	} else {
		whine("can't set input size on this kind of format");
	}
}

METHOD(VideoInFile,option) {
	CHECK_FILE_OPEN
	if ($->ff->cl->option) {
		$->ff->cl->option($->ff,ac,at);
	} else {
		whine("this format has no options");
	}
}

/* ---------------------------------------------------------------- */

CLASS(VideoInFile) {
	fts_type_t rien[]       = { fts_t_symbol };
	fts_type_t open_args[]  = { fts_t_symbol, fts_t_symbol };
	fts_type_t frame_args[] = { fts_t_symbol, fts_t_int };
	fts_type_t size_args[]  = { fts_t_symbol, fts_t_int, fts_t_int };
	fts_type_t option_args[]= { fts_t_symbol, fts_t_symbol, fts_t_int };

	MethodDecl methods[] = {
		{-1,fts_s_init,  METHOD_PTR(VideoInFile,init),  ARRAY(rien),-1},
		{-1,fts_s_delete,METHOD_PTR(VideoInFile,delete),0,0,0},
		{ 0,fts_s_bang,  METHOD_PTR(VideoInFile,bang),  ARRAY(rien),-1},
		{ 0,sym_reset,   METHOD_PTR(VideoInFile,reset), ARRAY(rien),-1},
		{ 0,sym_open,    METHOD_PTR(VideoInFile,open),  ARRAY(open_args),-1},
		{ 0,sym_close,   METHOD_PTR(VideoInFile,close), 0,0,0},
		{ 0,SYM(size),   METHOD_PTR(VideoInFile,size),  ARRAY(size_args),-1},
//		{ 0,SYM(frame),  METHOD_PTR(VideoInFile,frame), ARRAY(frame_args),-1},
		{ 0,SYM(option), METHOD_PTR(VideoInFile,option),ARRAY(option_args),-1},
	};

	/* initialize the class */
	fts_class_init(class, sizeof(VideoInFile), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));

	return fts_Success;
}

void startup_video_in_file (void) {
	fts_class_install(fts_new_symbol("@video_in_file"), VideoInFile_class_init);
}

