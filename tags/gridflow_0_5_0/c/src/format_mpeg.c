/*
	$Id$

	GridFlow
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

#include "grid.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mpeg.h>

/*
  i allow myself to make this global because
  libmpeg.so is broken by design.
*/
ImageDesc *mpeg_id = 0;

extern FormatClass class_FormatMPEG;
typedef struct FormatMPEG {
	Format_FIELDS;
} FormatMPEG;

static bool FormatMPEG_frame (FormatMPEG *$, GridOutlet *out, int frame) {
	char *buf = NEW(char,mpeg_id->Size);
	int npixels = mpeg_id->Height * mpeg_id->Width;
	if (frame != -1) {
		whine("can't seek frames with this driver");
		goto err;
	}
/*	SetMPEGOption(MPEG_QUIET,1); */
	if (!GetMPEGFrame(buf)) {
		whine("libmpeg: can't fetch frame");
		goto err;
	}

	{
		int v[] = { mpeg_id->Height, mpeg_id->Width, 3 };
		GridOutlet_begin(out, Dim_new(3,v));
	}

	{
		int y;
		int sy = Dim_get(out->dim,0);
		int sx = Dim_get(out->dim,1);
		int bs = Dim_prod_start(out->dim,1);
		Number b2[bs];
		for(y=0; y<sy; y++) {
			uint8 *b1 = buf + 4*sx*y;
			BitPacking_unpack($->bit_packing,sx,b1,b2);
			GridOutlet_send(out,bs,b2);
		}
	}

	FREE(buf);

	GridOutlet_end(out);
	return true;
err:
	return false;
}

GRID_BEGIN(FormatMPEG,0) { RAISE("libmpeg.so can't write MPEG"); }
GRID_FLOW(FormatMPEG,0) {}
GRID_END(FormatMPEG,0) {}

static void FormatMPEG_close (FormatMPEG *$) {
	if (mpeg_id) FREE(mpeg_id);
	if ($->st) Stream_close($->st);
	FREE($);
}

static Format *FormatMPEG_open (FormatClass *qlass, GridObject *parent, int mode, ATOMLIST) {
	FormatMPEG *$ = (FormatMPEG *)Format_open(&class_FormatMPEG,parent,mode);
	const char *filename;

	if (!$) return 0;

	if (ac!=2 || Var_get_symbol(at+0) != SYM(file)) {
		whine("usage: mpeg file <filename>"); goto err;
	}
	filename = Symbol_name(Var_get_symbol(at+1));

	$->st = Stream_open_file(filename,mode);
	if (!$->st) {
		whine("can't open file `%s': %s", filename, strerror(errno));
		goto err;
	}

	if (mpeg_id) {
		whine("libmpeg.so is busy (you must close the other mpeg file)");
		goto err;
	}

	mpeg_id = NEW(ImageDesc,1);

	SetMPEGOption(MPEG_DITHER,FULL_COLOR_DITHER);
	if (!OpenMPEG(Stream_get_file($->st),mpeg_id)) {
		whine("libmpeg: can't open mpeg file");
		goto err;
	}

	$->bit_packing = BitPacking_new(is_le(),4,0x0000ff,0x00ff00,0xff0000);

	return (Format *)$;
err:
	$->cl->close((Format *)$);
	return 0;
}

static GridHandler FormatMPEG_handler = GRINLET(FormatMPEG,0);
FormatClass class_FormatMPEG = {
	object_size: sizeof(FormatMPEG),
	symbol_name: "mpeg",
	long_name: "Motion Picture Expert Group Format (using Ward's)",
	flags: FF_R,

	open: FormatMPEG_open,
	frames: 0,
	frame:  (Format_frame_m)FormatMPEG_frame,
	handler: &FormatMPEG_handler,
	option: 0,
	close:  (Format_close_m)FormatMPEG_close,
};

