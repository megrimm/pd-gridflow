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

#include "../base/grid.h"

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

static Dim *FormatTarga_read_header (Format *$) {
	int i;
	short w,h;
	char *comment;
	int comment_length, depth;
	FILE *f = Stream_get_file($->st);

	comment_length = (uint8)getc(f);
	getc(f); /* skip */
	{
		int colors = (uint8)getc(f);
		if (colors != 2) RAISE("unsupported color format: %d", colors);
	}
	for (i=0;i<9;i++) getc(f); /* skip */
	fread(&w,1,2,f); /* !@#$ byte order problem */
	fread(&h,1,2,f); /* !@#$ byte order problem */
	depth = getc(f);
	whine("tga: size y=%d x=%d depth=%d",h,w,depth);
	getc(f); /* skip */
	comment = NEW2(char,comment_length+1);
	fread(comment, 1, comment_length, f);
	whine("tga: comment: %s", comment);
	FREE(comment);

/*	if (depth != 24 && depth != 32) { */

	/* temporary limitation of Format: only 3 channels */
	if (depth != 24) RAISE("tga: wrong colour depth: %i\n", depth);
	{
		int v[] = { h, w, depth/8 };
		return Dim_new(3,v);
	}
}

METHOD(FormatTarga,frame) {
	FILE *f = Stream_get_file($->st);
	Dim *dim = FormatTarga_read_header($);
	GridOutlet_begin($->out[0],dim);

	{
		int bs = Dim_prod_start($->out[0]->dim,1);
		int y;
		uint8 b1[bs];
		Number b2[bs];
		for (y=0; y<dim->v[0]; y++) {
			int i;
			int bs2 = (int) fread(b1,1,bs,f);
			if (bs2 < bs) {
				whine("unexpected end of file: bs=%d; bs2=%d",bs,bs2);
			}
			/* should use bitpacking instead... */
			for (i=0; i<bs; i+=3) {
				b2[i+0] = b1[i+2];
				b2[i+1] = b1[i+1];
				b2[i+2] = b1[i+0];
			}
			GridOutlet_send($->out[0],bs,b2);
		}
	}
	GridOutlet_end($->out[0]);
}

GRID_BEGIN(FormatTarga,0) { RAISE("not implemented"); }
GRID_FLOW(FormatTarga,0) {}
GRID_END(FormatTarga,0) {}

METHOD(FormatTarga,close) {
	if ($->st) Stream_close($->st);
	rb_call_super(argc,argv);
}

METHOD(FormatTarga,init) {
	const char *filename;
	rb_call_super(argc,argv);
	argv++, argc--;

	if (argc!=2 || argv[0] != SYM(file)) RAISE("usage: targa file <filename>");
	filename = rb_sym_name(argv[1]);

	$->st = Stream_open_file(filename,$->mode);
	if (!$->st) RAISE("can't open file `%s': %s", filename, strerror(errno));
}

/* **************************************************************** */

FMTCLASS(FormatTarga,"targa","TrueVision Targa",FF_R,
inlets:1,outlets:1,LIST(GRINLET(FormatTarga,0)),
DECL(FormatTarga,init),
DECL(FormatTarga,frame),
DECL(FormatTarga,close))

