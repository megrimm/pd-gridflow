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

struct FormatMPEG3 : Format {
	mpeg3_t *mpeg;
	BitPacking *bit_packing;
	int track;

	DECL3(initialize);
	DECL3(seek);
	DECL3(frame);
	DECL3(close);
};

METHOD3(FormatMPEG3,seek) {
	int frame = INT(argv[0]);
	int result = mpeg3_set_frame(mpeg,frame,track);
	return Qnil;
}

METHOD3(FormatMPEG3,frame) {
	int nframe = mpeg3_get_frame(mpeg,track);
	if (nframe >= mpeg3_video_frames(mpeg,track)) return Qfalse;

	GridOutlet *o = out[0];
	int sx = mpeg3_video_width(mpeg,track);
	int sy = mpeg3_video_height(mpeg,track);
	int npixels = sx*sy;
	int channels = 3;
	Pt<uint8> buf = ARRAY_NEW(uint8,sy*sx*channels+16);
	uint8 *rows[sy];
	for (int i=0; i<sy; i++) rows[i]=buf+i*sx*channels;
	int result = mpeg3_read_frame(mpeg,rows,0,0,sx,sy,sx,sy,MPEG3_RGB888,track);

	int32 v[] = { sy, sx, channels };
	o->begin(new Dim(3,v),
		NumberTypeIndex_find(rb_ivar_get(rself,SI(@cast))));

	int bs = o->dim->prod(1);
	STACK_ARRAY(int32,b2,bs);
	
	for(int y=0; y<sy; y++) {
		bit_packing->unpack(sx,buf+channels*sx*y,b2);
		o->send(bs,b2);
	}

	delete[] (uint8 *)buf;

	return INT2NUM(nframe);
}

METHOD3(FormatMPEG3,close) {
	if (bit_packing) delete bit_packing;
	if (mpeg) {
		mpeg3_close(mpeg);
		mpeg=0;
	}
	rb_call_super(argc,argv);
	return Qnil;
}

/* note: will not go through jMax data paths */
/* libmpeg3 may be nice, but it won't take a filehandle, only filename */
METHOD3(FormatMPEG3,initialize) {
	rb_call_super(argc,argv);
	argv++, argc--;
	if (argc!=2 || argv[0] != SYM(file)) RAISE("usage: mpeg file <filename>");
	const char *filename = rb_str_ptr(
		rb_funcall(mGridFlow,SI(find_file),1,
			rb_funcall(argv[1],SI(to_s),0)));

	mpeg = mpeg3_open(strdup(filename));
	if (!mpeg) RAISE("IO Error: can't open file `%s': %s", filename, strerror(errno));
	track = 0;

	uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
	bit_packing = new BitPacking(is_le(),3,3,mask);
	return Qnil;
}

static void startup (GridClass *self) {
	IEVAL(self->rclass,
	"conf_format 4,'mpeg','Motion Picture Expert Group Format"
	" (using HeroineWarrior\\'s)'");
}

GRCLASS(FormatMPEG3,"FormatMPEG3",
inlets:1,outlets:1,startup:startup,LIST(),
DECL(FormatMPEG3,initialize),
DECL(FormatMPEG3,seek),
DECL(FormatMPEG3,frame),
DECL(FormatMPEG3,close))

