/*
	$Id$

	GridFlow
	Copyright (c) 2001 by Mathieu Bouchard

	file: TGA format support
	(based on Christian Klippel's vobj/tga.c)

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
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "grid.h"

extern FormatClass class_FormatTarga;
typedef struct Format FormatTarga;

/* targa header is like:
	[:comment, Uint8, :length], 1,
	[:colors,  Uint8], 9,
	[:w, Uint16],
	[:h, Uint16],
	[:depth, Uint8], 1,
	[:comment, String8Unpadded, :data],
*/

Dim *FormatTarga_read_header (Format *$) {
	int i;
	short w,h;
	char *comment;
	int comment_length, depth;

	comment_length = (uint8)getc($->bstream);
	getc($->bstream); /* skip */
	{
		int colors = (uint8)getc($->bstream);
		if (colors != 2) {
			whine("unsupported color format: %d", colors);
			goto err;
		}
	}
	for (i=0;i<9;i++) getc($->bstream); /* skip */
	fread(&w,1,2,$->bstream); /* !@#$ byte order problem */
	fread(&h,1,2,$->bstream); /* !@#$ byte order problem */
	depth = getc($->bstream);
	whine("tga: size y=%d x=%d depth=%d",h,w,depth);
	getc($->bstream); /* skip */
	comment = NEW2(char,comment_length+1);
	fread(comment, 1, comment_length, $->bstream);
	whine("tga: comment: %s", comment);
	FREE(comment);

/*	if (depth != 24 && depth != 32) { */

	/* temporary limitation of Format: only 3 channels */
	if (depth != 24) {
		whine("tga: wrong colour depth: %i\n", depth);
		goto err;
	}
	{
		int v[] = { h, w, depth/8 };
		return Dim_new(3,v);
	}
err:
	return 0;
}

bool FormatTarga_frame (Format *$, GridOutlet *out, int frame) {
	Dim *dim;
	if (frame!=-1) return false;
	dim = FormatTarga_read_header($);
	if (!dim) return false;
	GridOutlet_begin(out,dim);

	{
		int bs = Dim_prod_start(out->dim,1);
		int y;
		uint8 b1[bs];
		Number b2[bs];
		for (y=0; y<dim->v[0]; y++) {
			int i;
			int bs2 = (int) fread(b1,1,bs,$->bstream);
			if (bs2 < bs) {
				whine("unexpected end of file: bs=%d; bs2=%d",bs,bs2);
			}
			/* should use bitpacking instead... */
			for (i=0; i<bs; i+=3) {
				b2[i+0] = b1[i+2];
				b2[i+1] = b1[i+1];
				b2[i+2] = b1[i+0];
			}
			GridOutlet_send(out,bs,b2);
		}
	}
	GridOutlet_end(out);
	return true;
}

GRID_BEGIN(FormatTarga,0) { RAISE("not implemented"); }
GRID_FLOW(FormatTarga,0) {}
GRID_END(FormatTarga,0) {}

void FormatTarga_close (Format *$) {
	if ($->bstream) fclose($->bstream);
	FREE($);
}

Format *FormatTarga_open (FormatClass *class, GridObject *parent, int mode, ATOMLIST) {
	FormatTarga *$ = (FormatTarga *)Format_open(&class_FormatTarga,parent,mode);
	const char *filename;

	if (!$) return 0;

	if (ac!=2 || fts_get_symbol(at+0) != SYM(file)) {
		whine("usage: targa file <filename>"); goto err;
	}
	filename = Symbol_name(fts_get_symbol(at+1));

	$->bstream = gf_file_fopen(filename,mode);
	if (!$->bstream) {
		whine("can't open file `%s': %s", filename, strerror(errno));
		goto err;
	}
	return $;
err:
	$->cl->close($);
	return 0;
}

static GridHandler FormatTarga_handler = GRINLET(FormatTarga,0);
FormatClass class_FormatTarga = {
	object_size: sizeof(FormatTarga),
	symbol_name: "targa",
	long_name: "TrueVision Targa",
	flags: FF_R,

	open: FormatTarga_open,
	frames: 0,
	frame:  FormatTarga_frame,
	handler: &FormatTarga_handler,
	option: 0,
	close:  FormatTarga_close,
};

