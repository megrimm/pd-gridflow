/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

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

typedef struct FormatMPEG3 {
	Format_FIELDS;
	mpeg3_t *mpeg;
} FormatMPEG3;

METHOD(FormatMPEG3,seek) {
	int frame = INT(argv[0]);
	int result;
	whine("attempting to seek frame # %d",frame);
	result = mpeg3_set_frame($->mpeg, frame, 0);
	whine("seek result: %d", result);
}

METHOD(FormatMPEG3,frame) {
	int sx = mpeg3_video_width($->mpeg,0);
	int sy = mpeg3_video_height($->mpeg,0);
	int npixels = sx*sy;
	int result;
	int i;
	uint8 *buf = NEW(uint8,sy*sx*3+16);
	uint8 *rows[sy];
	for (i=0; i<sy; i++) rows[i]=buf+i*sx*3;
	result = mpeg3_read_frame($->mpeg,rows,0,0,sx,sy,sx,sy,MPEG3_RGB888,0);
	{
		int v[] = { sy, sx, 3 };
		GridOutlet_begin($->out[0], Dim_new(3,v));
	}
	{
		int y;
		int sy = Dim_get($->out[0]->dim,0);
		int sx = Dim_get($->out[0]->dim,1);
		int bs = Dim_prod_start($->out[0]->dim,1);
		Number b2[bs];
		for(y=0; y<sy; y++) {
			uint8 *b1 = buf + 3*sx*y;
			BitPacking_unpack($->bit_packing,sx,b1,b2);
			GridOutlet_send($->out[0],bs,b2);
		}
	}
	FREE(buf);
	GridOutlet_end($->out[0]);
}

GRID_BEGIN(FormatMPEG3,0) { RAISE("write support not implemented"); }
GRID_FLOW(FormatMPEG3,0) {}
GRID_END(FormatMPEG3,0) {}

METHOD(FormatMPEG3,close) {
	if ($->mpeg) {
		mpeg3_close($->mpeg);
		$->mpeg=0;
	}
	rb_call_super(argc,argv);
}

/* note: will not go through jMax data paths */
/* libmpeg3 may be nice, but it won't take a filehandle, only filename */
METHOD(FormatMPEG3,init) {
	const char *filename;

	rb_call_super(argc,argv);
	argv++, argc--;

	if (argc!=2 || argv[0] != SYM(file)) RAISE("usage: mpeg file <filename>");
	filename = rb_str_ptr(rb_funcall(GridFlow_module,SI(find_file),1,
		rb_funcall(argv[1],SI(to_s),0)));

	$->mpeg = mpeg3_open(strdup(filename));
	if (!$->mpeg) RAISE("IO Error: can't open file `%s': %s", filename, strerror(errno));

	$->bit_packing = BitPacking_new(is_le(),3,0x0000ff,0x00ff00,0xff0000);
}

FMTCLASS(FormatMPEG3,"mpeg","Motion Picture Expert Group Format (using HeroineWarrior's)",FF_R,
inlets:1,outlets:1,LIST(GRINLET(FormatMPEG3,0)),
DECL(FormatMPEG3,init),
DECL(FormatMPEG3,seek),
DECL(FormatMPEG3,frame),
DECL(FormatMPEG3,close))
