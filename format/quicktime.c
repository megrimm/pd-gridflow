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
	int track;
	BitPacking *bit_packing;

	DECL3(initialize);
	DECL3(close);
	DECL3(frame);
	DECL3(seek);
	GRINLET3(0);
};

METHOD3(FormatQuickTime,seek) {
	int frame = INT(argv[0]);
	int result = quicktime_set_video_position(anim,frame,track);
	return Qnil;
}

METHOD3(FormatQuickTime,frame) {
	GridOutlet *o = out[0];
	int sx = quicktime_video_width(anim,track);
	int sy = quicktime_video_height(anim,track);
	int npixels = sx*sy;
	Pt<uint8> buf = ARRAY_NEW(uint8,sy*sx*4+16);
//	gfpost("pos = %d", quicktime_byte_position(anim));
//	quicktime_reads_cmodel(anim,BC_RGB888,0);
//	gfpost("size=%d",quicktime_frame_size(anim,0,track));

//	int channels = quicktime_track_channels(anim,track);
	int channels = quicktime_video_depth(anim,track)/8;

	uint8 *rows[sy];
	for (int i=0; i<sy; i++) rows[i]=buf+i*sx*channels;

	int result;
//	result = quicktime_read_frame(anim,buf,track);
//	result = quicktime_decode_video(anim,rows,track);
	result = quicktime_decode_scaled(anim,0,0,sx,sy,sx,sy,BC_RGB888,rows,track);
//	gfpost("result = %x", result);

	int32 v[] = { sy, sx, channels };
//	gfpost("dim(%d,%d,%d)",sy,sx,channels);
	o->begin(new Dim(3,v),
		NumberTypeIndex_find(rb_ivar_get(rself,SI(@cast))));

	int bs = o->dim->prod(1);
	STACK_ARRAY(uint8,b2,bs);
	for(int y=0; y<sy; y++) {
		bit_packing->unpack(sx,buf+channels*sx*y,b2);
		o->send(bs,b2);
	}

	delete[] (int32 *)buf;

	int nframe = min(
		quicktime_video_position(anim,track),
		quicktime_video_length(anim,track));
	return INT2NUM(nframe);
}

GRID_INLET(FormatQuickTime,0) { RAISE("write support not implemented"); }
GRID_FLOW {}
GRID_FINISH {}
GRID_END

METHOD3(FormatQuickTime,close) {
	if (bit_packing) delete bit_packing;
	if (anim) quicktime_close(anim);
	rb_call_super(argc,argv);
	return Qnil;
}

/* note: will not go through jMax data paths */
/* libquicktime may be nice, but it won't take a filehandle, only filename */
METHOD3(FormatQuickTime,initialize) {
	rb_call_super(argc,argv);
	argc--; argv++;

	if (argc!=2 || argv[0] != SYM(file))
		RAISE("usage: quicktime file <filename>");

	const char *filename = rb_str_ptr(
		rb_funcall(mGridFlow,SI(find_file),1,
			rb_funcall(argv[1],SI(to_s),0)));

	anim = quicktime_open(strdup(filename),1,0);
	if (!anim)
		RAISE("can't open file `%s': %s", filename, strerror(errno));

	track = 0;

	if (!quicktime_supported_video(anim,track))
		RAISE("quicktime: unsupported codec");

	uint32 masks[] = { 0x0000ff,0x00ff00,0xff0000 };
//	uint32 masks[] = { 0xff0000,0x00ff00,0x0000ff };
	bit_packing = new BitPacking(is_le(),3,3,masks);
	return Qnil;
}

static void startup (GridClass *self) {
	IEVAL(self->rubyclass,
	"conf_format 4,'quicktime','Apple Quicktime (using "
	"HeroineWarrior\\'s)'");
}

GRCLASS(FormatQuickTime,"FormatQuickTime",
inlets:1,outlets:1,startup:startup,LIST(GRINLET2(FormatQuickTime,0,4)),
DECL(FormatQuickTime,initialize),
DECL(FormatQuickTime,seek),
DECL(FormatQuickTime,frame),
DECL(FormatQuickTime,close))

