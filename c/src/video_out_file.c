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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "video4jmax.h"
#include "grid.h"

/* ---------------------------------------------------------------- */

/* return and complain when file not open */
#define CHECK_FILE_OPEN \
	if (!$->file) { whine("can't do that: file not open"); return; }

/* some data/type decls */

typedef struct VideoOutFile VideoOutFile;

struct VideoOutFile {
	GridObject_FIELDS;
	
/* fields for: file writing */

	FILE *file;     /* a UNIX-style buffered file object */
	char *filename; /* a valid UNIX filename */
};

/* ---------------------------------------------------------------- */

/* same as vout's */
void VideoOutFile_acceptor0(GridInlet *$) {
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
	fprintf(parent->file,
		"P6\n"
		"# generated using Video4jmax " VIDEO4JMAX_VERSION "\n"
		"%d %d\n"
		"255\n",
		Dim_get($->dim,1),
		Dim_get($->dim,0));

	fflush(parent->file);
	$->dex=0;
}

void VideoOutFile_processor0(GridInlet *$, int n, const Number *data) {
	VideoOutFile *parent = (VideoOutFile *) GridInlet_parent($);
	FILE *f = parent->file;
	{
		VideoOutFile *$ = parent;
		CHECK_FILE_OPEN
	}
	while(n > 0) {
		int incr;
		int max = Dim_prod($->dim) - $->dex;
		int bs = n<max?n:max;
		uint8 data2[bs];
		int i;
		for (i=0; i<bs; i++) data2[i] = data[i];
		fwrite(data2,1,bs,f);
		data += bs;
		$->dex += bs;
		n -= bs;
		if ($->dex >= Dim_prod($->dim)) {
			fflush(f);
			fseek(f,0,SEEK_SET);
			fts_outlet_send(OBJ(parent),0,fts_s_bang,0,0);
		}
	}
}

/* ---------------------------------------------------------------- */

METHOD(VideoOutFile,init) {
	whine("VideoOutFile#init");

	$->filename = 0;
	$->file = 0;

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_new((GridObject *)$, 0,
		VideoOutFile_acceptor0, VideoOutFile_processor0);
}

static void VideoOutFile_p_close(VideoOutFile *$) {
	CHECK_FILE_OPEN
	fclose($->file);
	$->file = 0;
	$->filename = 0;
}

METHOD(VideoOutFile,close) {
	VideoOutFile_p_close($);
}

METHOD(VideoOutFile,open) {
	whine("VideoOutFile#open");

	if ($->file) { VideoOutFile_p_close($); }

	{
		fts_symbol_t foo = GET(0,symbol,fts_new_symbol("/tmp/untitled.ppm"));
		$->filename = strdup(fts_symbol_name(foo));
	}

	$->file = fopen($->filename,"w+");
	if (!$->file) {
		whine("can't open file %s for writing", $->filename);
		$->filename = 0;
		return;
	}
}

METHOD(VideoOutFile,delete) {
	whine("VideoOutFile#delete");
	if ($->file) { VideoOutFile_p_close($); }
}

METHOD(VideoOutFile,bang) {
	CHECK_FILE_OPEN
	fflush($->file);
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
		{ 0,fts_s_bang,  METHOD_PTR(VideoOutFile,bang), 0,0,0 },
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

