/*
	Video4jmax
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

extern FileFormatClass FormatTarga;

/* targa header is like:
	[:comment, Uint8, :length], 1,
	[:colors,  Uint8], 9,
	[:w, Uint16],
	[:h, Uint16],
	[:depth, Uint8], 1,
	[:comment, String8Unpadded, :data],
*/

Dim *FormatTarga_frame (FileFormat *$, int frame) {
	char in;
	int dims[3],n=0,i;
	short w,h;
	char *comment;
	int comment_length, depth;

	comment_length = (uint8)getc($->stream);
	getc($->stream); /* skip */
	{
		int colors = (uint8)getc($->stream);
		if (colors != 2) {
			whine("unsupported color format: %d", colors);
			goto err;
		}
	}
	for (i=0;i<9;i++) getc($->stream); /* skip */
	fread(&w,1,2,$->stream); /* !@#$ byte order problem */
	fread(&h,1,2,$->stream); /* !@#$ byte order problem */
	depth = getc($->stream);
	whine("tga: size y=%d x=%d depth=%d",h,w,depth);
	getc($->stream); /* skip */
	comment = NEW2(char,comment_length+1);
	fread(comment, 1, comment_length, $->stream);
	whine("tga: comment: %s", comment);
	free(comment);

	if (depth != 24 && depth != 32) {
		whine("tga: wrong colour depth: %i\n", depth);
		goto err;
	}
	{
		int v[] = { h, w, depth/8 };
		$->dim = Dim_new(3,v);
		$->left = Dim_prod($->dim);
		return $->dim;
	}
err:
	fclose($->stream);
	return 0;
}

Number *FormatTarga_read (FileFormat *$, int n) {
	int i;
	int bs = $->left;
	int bs2;
	uint8 b1[n];
	Number *b2 = NEW2(Number,n);

	if (!$->dim) return 0;
	if (bs > n) bs = n;
	bs2 = (int) fread(b1,1,bs,$->stream);
	if (bs2 < bs) {
		whine("unexpected end of file: bs=%d; bs2=%d",bs,bs2);
	}
	for (i=0; i<bs; i++) b2[i] = b1[i];
	n -= bs;
	$->left -= bs;
	return b2;
}

void FormatTarga_begin (FileFormat *$, Dim *dim) {
}

void FormatTarga_flow (FileFormat *$, int n, const Number *data) {
}

void FormatTarga_end (FileFormat *$) {
}

void FormatTarga_close (FileFormat *$) {
	if ($->stream) fclose($->stream);
	free($);
}

FileFormat *FormatTarga_open (const char *filename, int mode) {
	const char *modestr;
	FileFormat *$ = NEW(FileFormat,1);
	$->qlass  = &FormatTarga;
	$->frames = 0;
	$->frame  = FormatTarga_frame;
	$->size   = 0;
	$->read   = FormatTarga_read;
	$->begin  = FormatTarga_begin;
	$->flow   = FormatTarga_flow;
	$->end    = FormatTarga_end;
	$->close  = FormatTarga_close;
	$->stuff  = NEW(int,4);

	switch(mode) {
//	case 4: case 2: break;
	case 4: break; /* read-only for now */
	default: whine("unsupported mode (#%d)", mode); goto err;
	}

	$->stream = v4j_file_fopen(filename,mode);
	if (!$->stream) {
		whine("can't open file `%s': %s", filename, strerror(errno));
		goto err;
	}
	return $;
err:
	$->close($);
	return 0;
}

FileFormatClass FormatTarga = {
	"targa", "Targa", 0,
	FormatTarga_open, 0, 0,
};

