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
#include <mpeg.h>

/*
  i allow myself to make this global because
  libmpeg.so is broken by design.
*/
ImageDesc *mpeg_id = 0;

struct FormatMPEG : Format {
	DECL3(init),
	DECL3(frame),
	DECL3(close))
	GRINLET3(0);
};

METHOD3(FormatMPEG,frame) {
	uint8 *buf = NEW(uint8,mpeg_id->Size);
	GridOutlet *o = out[0];
	int npixels = mpeg_id->Height * mpeg_id->Width;
/*	SetMPEGOption(MPEG_QUIET,1); */
	if (!GetMPEGFrame((char *)buf)) RAISE("libmpeg: can't fetch frame");

	int v[] = { mpeg_id->Height, mpeg_id->Width, 3 };
	o->begin(new Dim(3,v));

	int sy = o->dim->get(0);
	int sx = o->dim->get(1);
	int bs = o->dim->prod(1);
	Number b2[bs];
	for(int y=0; y<sy; y++) {
		uint8 *b1 = buf + 4*sx*y;
		bit_packing->unpack(sx,b1,b2);
		o->send(bs,b2);
	}
	FREE(buf);
	o->end();
}

GRID_BEGIN(FormatMPEG,0) { RAISE("libmpeg.so can't write MPEG"); }
GRID_FLOW(FormatMPEG,0) {}
GRID_END(FormatMPEG,0) {}

METHOD3(FormatMPEG,close) {
	if (mpeg_id) {
		CloseMPEG();
		FREE(mpeg_id);
	}
	rb_call_super(argc,argv);
}

METHOD3(FormatMPEG,init) {
	rb_call_super(argc,argv);
	argv++, argc--;
	if (argc!=2 || argv[0] != SYM(file)) RAISE("usage: mpeg file <filename>");

	if (mpeg_id) RAISE("libmpeg.so is busy (you must close the other mpeg file)");

	const char *filename =
		rb_str_ptr(rb_funcall(GridFlow_module,SI(find_file),1,
			rb_funcall(argv[1],SI(to_s),0)));

	mpeg_id = NEW(ImageDesc,1);
	SetMPEGOption(MPEG_DITHER,FULL_COLOR_DITHER);
	VALUE f = rb_funcall(rb_cFile,SI(open),2,
		rb_str_new2(filename),rb_str_new2("r"));
	FILE *f2 = RFILE(f)->fptr->f;
	if (!OpenMPEG(f2,mpeg_id))
		RAISE("libmpeg: can't open mpeg file `%s': %s", filename, strerror(errno));

	uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
	bit_packing = new BitPacking(is_le(),4,3,mask);
}

FMTCLASS(FormatMPEG,"mpeg","Motion Picture Expert Group Format (using Ward's)",FF_R,
inlets:1,outlets:1,LIST(GRINLET(FormatMPEG,0)),
DECL(FormatMPEG,init),
DECL(FormatMPEG,frame),
DECL(FormatMPEG,close))

