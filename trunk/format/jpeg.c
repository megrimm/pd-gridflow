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
extern "C" {
#include <jpeglib.h>
};

struct FormatJPEG : Format {
	BitPacking *bit_packing;
	struct jpeg_decompress_struct djpeg;
	struct jpeg_error_mgr jerr;
	FILE *f;

	DECL3(close);
	DECL3(frame);
	DECL3(init);
	GRINLET3(0);
};

GRID_BEGIN(FormatJPEG,0) {
/*
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 greyscale channel (got %d)",in->dim->get(2));
	if (dim) delete dim;
	dim = in->dim->dup();
	in->set_factor(in->dim->get(1)*in->dim->get(2));
*/
}

GRID_FLOW(FormatJPEG,0) {
}

GRID_END(FormatJPEG,0) {
}

METHOD3(FormatJPEG,frame) {
	GridOutlet *o = out[0];
	rb_funcall(peer,SI(rewind_if_needed),0);
	djpeg.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&djpeg);
	jpeg_stdio_src(&djpeg,f);
	jpeg_read_header(&djpeg,TRUE);
	int sx = djpeg.image_width, sy = djpeg.image_height;
	int v[] = { sy, sx, 3 };
	o->begin(new Dim(3,v));
	jpeg_start_decompress(&djpeg);
	uint8 row[sx*3];
	uint8 *rows[1] = { row };
	for (int n=0; n<sy; n++) {
		jpeg_read_scanlines(&djpeg,rows,1);
		o->send(sx*3,Pt<uint8>(row,sx*4));
	}
	jpeg_finish_decompress(&djpeg);
	jpeg_destroy_decompress(&djpeg);
	o->end();
	return Qnil;
}

METHOD3(FormatJPEG,close) {
	return Qnil;
}

METHOD3(FormatJPEG,init) {
	rb_call_super(argc,argv);
	argv++, argc--;
	if (argc!=2 || argv[0] != SYM(file)) RAISE("usage: jpeg file <filename>");
	rb_funcall(peer,SI(raw_open),3,SYM(in),argv[0],argv[1]);
	uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
	bit_packing = new BitPacking(is_le(),3,3,mask);
	OpenFile *foo;
	GetOpenFile(rb_ivar_get(peer,SI(@stream)),foo);
	f = foo->f;
	return Qnil;
}

static void startup (GridClass *$) {
	fprintf(stderr,"HELLO\n");
	IEVAL($->rubyclass,
	"include GridFlow::EventIO; conf_format 4,'jpeg','JPEG'");
}

GRCLASS(FormatJPEG,"FormatJPEG",
inlets:1,outlets:1,startup:startup,LIST(GRINLET(FormatJPEG,0,4)),
DECL(FormatJPEG,init),
DECL(FormatJPEG,frame),
DECL(FormatJPEG,close))

