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

\class FormatMPEG < Format
struct FormatMPEG : Format {
	BitPacking *bit_packing;

	\decl void initialize (Symbol mode, Symbol source, String filename);
	\decl Ruby frame ();
	\decl void close ();
	GRINLET3(0);
};


\def Ruby frame () {
	uint8 *buf = new uint8[mpeg_id->Size];
	GridOutlet *o = out[0];
//	int npixels = mpeg_id->Height * mpeg_id->Width;
/*	SetMPEGOption(MPEG_QUIET,1); */
	if (!GetMPEGFrame((char *)buf)) RAISE("libmpeg: can't fetch frame");

	int32 v[] = { mpeg_id->Height, mpeg_id->Width, 3 };
	o->begin(new Dim(3,v),
		NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));

	int sy = o->dim->get(0);
	int sx = o->dim->get(1);
	int bs = o->dim->prod(1);
	STACK_ARRAY(uint8,b2,bs);
	for(int y=0; y<sy; y++) {
		uint8 *b1 = buf + 4*sx*y;
		bit_packing->unpack(sx,Pt<uint8>(b1,4*sx*y),Pt<uint8>(b2,sx*y));
		o->send(bs,b2);
	}
	delete[] buf;
	return Qnil;
}

GRID_INLET(FormatMPEG,0) { RAISE("libmpeg.so can't write MPEG"); }
GRID_FLOW {}
GRID_FINISH {}
GRID_END

\def void close () {
	if (bit_packing) delete bit_packing;
	if (mpeg_id) {
		CloseMPEG();
		delete mpeg_id;
	}
	rb_call_super(argc,argv);
	return Qnil;
}

\def void initialize (Symbol mode, Symbol source, String filename) {
	rb_call_super(argc,argv);
	argv++, argc--;
	if (source != SYM(file)) RAISE("usage: mpeg file <filename>");

	if (mpeg_id) RAISE("libmpeg.so is busy (you must close the other mpeg file)");

	filename = rb_funcall(mGridFlow,SI(find_file),1,filename)

	mpeg_id = new ImageDesc;
	SetMPEGOption(MPEG_DITHER,FULL_COLOR_DITHER);
	VALUE f = rb_funcall(rb_cFile,SI(open),2,filename,rb_str_new2("r"));
	FILE *f2 = RFILE(f)->fptr->f;
	if (!OpenMPEG(f2,mpeg_id))
		RAISE("libmpeg: can't open mpeg file `%s': %s", filename, strerror(errno));

	uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
	bit_packing = new BitPacking(is_le(),4,3,mask);
	return Qnil;
}

GRCLASS(FormatMPEG,LIST(GRINLET2(FormatMPEG,0,4)),
\grdecl
){
	IEVAL(rself,"install 'FormatMPEG',1,1;"
	"conf_format 4,'mpeg','Motion Picture Expert Group Format"
	" (using Ward\\'s)'");
}

\end class FormatMPEG
