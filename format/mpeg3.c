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

#define LIBMPEG_INCLUDE_HERE
#include "../base/grid.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern FormatClass class_FormatMPEG3;
typedef struct FormatMPEG3 {
	Format_FIELDS;
	mpeg3_t *mpeg;
} FormatMPEG3;

static bool FormatMPEG3_frame (FormatMPEG3 *$, GridOutlet *out, int frame) {
	int sx = mpeg3_video_width($->mpeg,0);
	int sy = mpeg3_video_height($->mpeg,0);
	int npixels = sx*sy;
	int result;
	int i;
	uint8 *buf = NEW(uint8,sy*sx*3+16);
	uint8 *rows[sy];
	for (i=0; i<sy; i++) rows[i]=buf+i*sx*3;

	if (frame != -1) {
		int result;
		whine("attempting to seek frame # %d",frame);
		result = mpeg3_set_frame($->mpeg, frame, 0);
		whine("seek result: %d", result);
	}

	result = mpeg3_read_frame($->mpeg,rows,0,0,sx,sy,sx,sy,MPEG3_RGB888,0);
			 
	{
		int v[] = { sy, sx, 3 };
		GridOutlet_begin(out, Dim_new(3,v));
	}

	{
		int y;
		int sy = Dim_get(out->dim,0);
		int sx = Dim_get(out->dim,1);
		int bs = Dim_prod_start(out->dim,1);
		Number b2[bs];
		for(y=0; y<sy; y++) {
			uint8 *b1 = buf + 3*sx*y;
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

GRID_BEGIN(FormatMPEG3,0) { RAISE("write support not implemented"); }
GRID_FLOW(FormatMPEG3,0) {}
GRID_END(FormatMPEG3,0) {}

static void FormatMPEG3_close (FormatMPEG3 *$) {
	if ($->mpeg) mpeg3_close($->mpeg);
	FREE($);
}

/* note: will not go through jMax data paths */
/* libmpeg3 may be nice, but it won't take a filehandle, only filename */
static Format *FormatMPEG3_open (FormatClass *qlass, GridObject *parent, int
mode, int argc, VALUE *argv) {
	FormatMPEG3 *$ = (FormatMPEG3 *)Format_open(&class_FormatMPEG3,parent,mode);
	const char *filename;

	if (!$) return 0;

	if (argc!=2 || argv[0] != SYM(file)) {
		whine("usage: mpeg file <filename>"); goto err;
	}
	filename = rb_id2name(SYM2ID(argv[1]));

	$->mpeg = mpeg3_open(strdup(filename));
	if (!$->mpeg) {
		whine("can't open file `%s': %s", filename, strerror(errno));
		goto err;
	}

	$->bit_packing = BitPacking_new(is_le(),3,0x0000ff,0x00ff00,0xff0000);

	return (Format *)$;
err:
	$->cl->close((Format *)$);
	return 0;
}

static GridHandler FormatMPEG3_handler = GRINLET(FormatMPEG3,0);
FormatClass class_FormatMPEG3 = {
	object_size: sizeof(FormatMPEG3),
	symbol_name: "mpeg",
	long_name: "Motion Picture Expert Group Format (using HeroineWarrior's)",
	flags: FF_R,

	open: FormatMPEG3_open,
	frames: 0,
	frame:  (Format_frame_m)FormatMPEG3_frame,
	handler: &FormatMPEG3_handler,
	option: 0,
	close:  (Format_close_m)FormatMPEG3_close,
};

