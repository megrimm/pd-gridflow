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

extern FormatClass class_FormatMPEG;
typedef struct FormatMPEG {
	Format_FIELDS;
} FormatMPEG;

static bool FormatMPEG_frame (FormatMPEG *$, GridOutlet *out, int frame) {
	char *buf = NEW(char,mpeg_id->Size);
	int npixels = mpeg_id->Height * mpeg_id->Width;
	if (frame != -1) RAISE("can't seek frames with this driver");
/*	SetMPEGOption(MPEG_QUIET,1); */
	if (!GetMPEGFrame(buf)) RAISE("libmpeg: can't fetch frame");

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

METHOD(FormatMPEG,close) {
	if (mpeg_id) FREE(mpeg_id);
	if ($->st) Stream_close($->st);
	rb_call_super(argc,argv);
}

METHOD(FormatMPEG,init) {
	rb_call_super(argc,argv);
	const char *filename;
	argv++, argc--;

	if (argc!=2 || argv[0] != SYM(file)) RAISE("usage: mpeg file <filename>");

	filename = Symbol_name(Var_get_symbol(at+1));
	filename = RSTRING(
	rb_funcall(GridFlow_module,rb_intern("find_file"),1,rb_str_new2(filename))
	)->ptr;
	
	$->st = Stream_open_file(filename,mode);
	if (!$->st) RAISE("can't open file `%s': %s", filename, strerror(errno));

	if (mpeg_id) RAISE("libmpeg.so is busy (you must close the other mpeg file)");

	mpeg_id = NEW(ImageDesc,1);

	SetMPEGOption(MPEG_DITHER,FULL_COLOR_DITHER);
	if (!OpenMPEG(Stream_get_file($->st),mpeg_id))
		RAISE("libmpeg: can't open mpeg file");

	$->bit_packing = BitPacking_new(is_le(),4,0x0000ff,0x00ff00,0xff0000);

	return (Format *)$;
err:
	FormatMPEG_close($);
	return 0;
}

FMTCLASS(FormatMPEG,"mpeg","Motion Picture Expert Group Format (using Ward's)",FF_R,
inlets:1,outlets:1,LIST(GRINLET(FormatMPEG,0)),
DECL(FormatMPEG,init),
DECL(FormatMPEG,frame),
DECL(FormatMPEG,close))
