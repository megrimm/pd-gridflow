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

/* !@#$ not handling abort on compress */
/* !@#$ not handling abort on decompress */

#include "../base/grid.h"
extern "C" {
#include <jpeglib.h>
};

struct FormatJPEG : Format {
	BitPacking *bit_packing;
	struct jpeg_compress_struct cjpeg;
	struct jpeg_decompress_struct djpeg;
	struct jpeg_error_mgr jerr;
	FILE *f;

	DECL3(close);
	DECL3(frame);
	DECL3(initialize);
	GRINLET3(0);
};

GRID_INLET(FormatJPEG,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels (got %d)",in->dim->get(2));
	in->set_factor(in->dim->get(1)*in->dim->get(2));
	cjpeg.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cjpeg);
	jpeg_stdio_dest(&cjpeg,f);
	cjpeg.image_width = in->dim->get(1);
	cjpeg.image_height = in->dim->get(0);
	cjpeg.input_components = 3;
	cjpeg.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cjpeg);
	jpeg_start_compress(&cjpeg,TRUE);
}

GRID_FLOW {
	int rowsize = in->dim->get(1)*in->dim->get(2);
	int rowsize2 = in->dim->get(1)*3;
	uint8 row[rowsize2];
	uint8 *rows[1] = { row };
	while (n) {
		bit_packing->pack(in->dim->get(1),data,Pt<uint8>(row,rowsize));
		jpeg_write_scanlines(&cjpeg,rows,1);		
		n-=rowsize; data+=rowsize;
	}
}

GRID_FINISH {
	jpeg_finish_compress(&cjpeg);
	jpeg_destroy_compress(&cjpeg);
}
GRID_END

METHOD3(FormatJPEG,frame) {
	GridOutlet *o = out[0];
	rb_funcall(rself,SI(rewind_if_needed),0);
	djpeg.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&djpeg);
	jpeg_stdio_src(&djpeg,f);
	jpeg_read_header(&djpeg,TRUE);
	int sx = djpeg.image_width, sy = djpeg.image_height;
	int32 v[] = { sy, sx, 3 };
	o->begin(new Dim(3,v));
	jpeg_start_decompress(&djpeg);
	uint8 row[sx*3];
	uint8 *rows[1] = { row };
	for (int n=0; n<sy; n++) {
		jpeg_read_scanlines(&djpeg,rows,1);
		o->send(sx*3,Pt<uint8>(row,sx*3));
	}
	jpeg_finish_decompress(&djpeg);
	jpeg_destroy_decompress(&djpeg);
	return Qnil;
}

METHOD3(FormatJPEG,close) {
	return Qnil;
}

METHOD3(FormatJPEG,initialize) {
	rb_call_super(argc,argv);
	argv++, argc--;
	if (argc!=2 || argv[0] != SYM(file)) RAISE("usage: jpeg file <filename>");
	if (mode()!=SYM(in) && mode()!=SYM(out)) RAISE("AAARGH!");
	rb_funcall(rself,SI(raw_open),3,mode(),argv[0],argv[1]);
	OpenFile *foo;
	GetOpenFile(rb_ivar_get(rself,SI(@stream)),foo);
	f = foo->f;
	uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
	bit_packing = new BitPacking(is_le(),3,3,mask);
	return Qnil;
}

static void startup (GridClass *self) {
	IEVAL(self->rubyclass,
	"include GridFlow::EventIO; conf_format 6,'jpeg','JPEG'");
}

GRCLASS(FormatJPEG,"FormatJPEG",
inlets:1,outlets:1,startup:startup,LIST(GRINLET(FormatJPEG,0,4)),
DECL(FormatJPEG,initialize),
DECL(FormatJPEG,frame),
DECL(FormatJPEG,close))
