/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

#include "../base/grid.h.fcs"
extern "C" {
#include <libpng12/png.h>
};

\class FormatPNG < Format
struct FormatPNG : Format {
	BitPacking *bit_packing;
	png_structp png;
	png_infop info;
	FILE *f;

	FormatPNG () : bit_packing(0), png(0), f(0) {}
	
	\decl void close ();
	\decl Ruby frame ();
	\decl void initialize (Symbol mode, Symbol source, String filename);
	GRINLET3(0);
};

GRID_INLET(FormatPNG,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels (got %d)",in->dim->get(2));
	in->set_factor(in->dim->get(1)*in->dim->get(2));
	RAISE("bother, said pooh, as the PNG encoding was found unimplemented");
} GRID_FLOW {
	int rowsize = in->dim->get(1)*in->dim->get(2);
	int rowsize2 = in->dim->get(1)*3;
	uint8 row[rowsize2];
	uint8 *rows[1] = { row };
	while (n) {
		bit_packing->pack(in->dim->get(1),data,Pt<uint8>(row,rowsize));
		n-=rowsize; data+=rowsize;
	}
} GRID_FINISH {
} GRID_END

\def Ruby frame () {
	if (feof(f)) return Qfalse;
	
	uint8 sig[8];
	if (!fread(sig, 1, 8, f)) return Qfalse;
	if (!png_check_sig(sig, 8)) RAISE("bad signature");
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) RAISE("!png");
	info = png_create_info_struct(png);
	if (!info) {
		png_destroy_read_struct(&png, NULL, NULL);
		RAISE("!info");
	}

	if (setjmp(png_jmpbuf(png))) {
	barf_out:
		png_destroy_read_struct(&png, &info, NULL);
		RAISE("png read error");
	}

	png_init_io(png, f);
	png_set_sig_bytes(png, 8);  /* we already read the 8 signature bytes */
	png_read_info(png, info);  /* read all PNG info up to image data */

	/* alternatively, could make separate calls to png_get_image_width(),
	* etc., but want bit_depth and color_type for later [don't care about
	* compression_type and filter_type => NULLs] */

	uint32 width, height;
	int bit_depth, color_type;
	png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type,
		NULL, NULL, NULL);

/////////////////////////////////////////////////////// part 2

	png_bytepp row_pointers = 0;
	if (color_type == PNG_COLOR_TYPE_PALETTE
		|| (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		|| png_get_valid(png, info, PNG_INFO_tRNS))
			png_set_expand(png);
	// 16bpp y, 32bpp ya, 48bpp rgb, 64bpp rgba...
	if (bit_depth == 16) png_set_strip_16(png);

	double display_gamma = 2.2;
	double gamma;
	if (png_get_gAMA(png, info, &gamma))
		png_set_gamma(png, display_gamma, gamma);
	png_read_update_info(png, info);

	int rowbytes = png_get_rowbytes(png, info);
	int channels = (int)png_get_channels(png, info);
	uint8 *image_data = (uint8 *)malloc(rowbytes*height);
	row_pointers = new png_bytep[height];
	gfpost("png: color_type=%d channels=%d, width=%d, rowbytes=%ld, height=%ld, gamma=%f",
		color_type, channels, width, rowbytes, height, gamma);
	for (int i=0; i<(int)height; i++) row_pointers[i] = image_data + i*rowbytes;
	if ((uint32)rowbytes != width*channels)
		RAISE("rowbytes mismatch: %d is not %d*%d=%d",
			rowbytes, width, channels, width*channels);
	png_read_image(png, row_pointers);
	delete row_pointers;
	row_pointers = 0;
	png_read_end(png, 0);

	int32 v[] = { height, width, channels };
	out[0]->begin(new Dim(3,v),
		NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
	out[0]->send(rowbytes*height, Pt<uint8>(image_data,rowbytes*height));
	free(image_data);
	return Qnil;
}

\def void close () {
	rb_call_super(argc,argv);
}

\def void initialize (Symbol mode, Symbol source, String filename) {
	rb_call_super(argc,argv);
	if (source!=SYM(file)) RAISE("usage: png file <filename>");
	rb_funcall(rself,SI(raw_open),3,mode,source,filename);
	OpenFile *foo;
	GetOpenFile(rb_ivar_get(rself,SI(@stream)),foo);
	f = foo->f;
	uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
	bit_packing = new BitPacking(is_le(),3,3,mask);
}

GRCLASS(FormatPNG,LIST(GRINLET2(FormatPNG,0,4)),
\grdecl
) {
	IEVAL(rself,
	"install 'FormatPNG',1,1;"
	"include GridFlow::EventIO; conf_format 4,'png','PNG'");
}

\end class FormatPNG