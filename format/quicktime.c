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

#include "../base/grid.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <quicktime/quicktime.h>
#include <quicktime/colormodels.h>

struct FormatQuickTime : Format {
	quicktime_t *anim;
};

METHOD(FormatQuickTime,frame) {
	GridOutlet *o = $->out[0];
	int sx = quicktime_video_width($->anim,0);
	int sy = quicktime_video_height($->anim,0);
	int npixels = sx*sy;
	uint8 *buf = NEWA(uint8,sy*sx*4+16);
	uint8 *rows[sy];
	for (int i=0; i<sy; i++) rows[i]=buf+i*sx*4;
	gfpost("pos = %d", quicktime_byte_position($->anim));
//	quicktime_reads_cmodel($->anim,BC_RGB888,0);
//	gfpost("size=%d",quicktime_frame_size($->anim,0,0));

	int result;
//	result = quicktime_read_frame($->anim,buf,0);
//	result = quicktime_decode_video($->anim,rows,0);
	result = quicktime_decode_scaled($->anim,0,0,sx,sy,sx,sy,BC_RGB888,rows,0);
//	gfpost("result = %x", result);

	int v[] = { sy, sx, 3 };
	gfpost("dim(%d,%d,%d)",sy,sx,3);
	o->begin(new Dim(3,v));

	int bs = o->dim->prod(1);
	Number b2[bs];
	for(int y=0; y<sy; y++) {
		uint8 *b1 = buf + 4*sx*y;
		$->bit_packing->unpack(sx,b1,b2);
		o->send(bs,b2);
	}

	FREE(buf);
	o->end();
}

GRID_BEGIN(FormatQuickTime,0) { RAISE("write support not implemented"); }
GRID_FLOW(FormatQuickTime,0) {}
GRID_END(FormatQuickTime,0) {}

METHOD(FormatQuickTime,close) {
	if ($->anim) quicktime_close($->anim);
	rb_call_super(argc,argv);
}

/* note: will not go through jMax data paths */
/* libquicktime may be nice, but it won't take a filehandle, only filename */
METHOD(FormatQuickTime,init) {
	rb_call_super(argc,argv);
	argc--; argv++;

	if (argc!=2 || argv[0] != SYM(file))
		RAISE("usage: quicktime file <filename>");

	const char *filename = rb_str_ptr(
		rb_funcall(GridFlow_module,SI(find_file),1,
			rb_funcall(argv[1],SI(to_s),0)));

	$->anim = quicktime_open(strdup(filename),1,0);
	if (!$->anim)
		RAISE("can't open file `%s': %s", filename, strerror(errno));

	if (!quicktime_supported_video($->anim,0))
		RAISE("quicktime: unsupported codec");

//	uint32 masks[] = { 0x0000ff,0x00ff00,0xff0000 };
	uint32 masks[] = { 0xff0000,0x00ff00,0x0000ff };
	$->bit_packing = new BitPacking(is_le(),4,3,masks);
}

FMTCLASS(FormatQuickTime,"quicktime","Apple Quicktime (using HeroineWarrior's)",FF_R,
inlets:1,outlets:1,LIST(GRINLET(FormatQuickTime,0)),
DECL(FormatQuickTime,init),
//DECL(FormatQuickTime,seek),
DECL(FormatQuickTime,frame),
DECL(FormatQuickTime,close))

