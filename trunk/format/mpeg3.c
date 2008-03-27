/*
	$Id$

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

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
#include "../base/grid.h.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

\class FormatMPEG3 : Format {
	mpeg3_t *mpeg;
	int track;
	FormatMPEG3 () : track(0) {}
	\decl void initialize (Symbol mode, String filename);
	\decl 0 seek (int frame);
	\decl 0 bang ();
	\decl 0 close ();
};

\def 0 seek (int frame) { mpeg3_set_frame(mpeg,frame,track); }

\def 0 bang () {
	int nframe = mpeg3_get_frame(mpeg,track);
	int nframes = mpeg3_video_frames(mpeg,track);
	post("track=%d; nframe=%d; nframes=%d",track,nframe,nframes);
	if (nframe >= nframes) {outlet_bang(bself->te_outlet); return;}
	int sx = mpeg3_video_width(mpeg,track);
	int sy = mpeg3_video_height(mpeg,track);
	int channels = 3;
	/* !@#$ the doc says "You must allocate 4 extra bytes in the
	last output_row. This is scratch area for the MMX routines." */
	uint8 * buf = new uint8[sy*sx*channels+16];
	uint8 *rows[sy];
	for (int i=0; i<sy; i++) rows[i]=buf+i*sx*channels;
	mpeg3_read_frame(mpeg,rows,0,0,sx,sy,sx,sy,MPEG3_RGB888,track);
	GridOutlet out(this,0,new Dim(sy, sx, channels),
		NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
	int bs = out.dim->prod(1);
	for(int y=0; y<sy; y++) {
		uint8 *row = buf+channels*sx*y;
		out.send(bs,row);
	}
	delete[] (uint8 *)buf;
//	return INT2NUM(nframe);
}

\def 0 close () {
	if (mpeg) { mpeg3_close(mpeg); mpeg=0; }
	SUPER;
}

// libmpeg3 may be nice, but it won't take a filehandle, only filename
\def void initialize (Symbol mode, String filename) {
	SUPER;
	if (mode!=SYM(in)) RAISE("read-only, sorry");
	if (TYPE(filename)!=T_STRING) RAISE("PATATE POILUE");
	filename = rb_funcall(mGridFlow,SI(find_file),1,filename);
#ifdef MPEG3_UNDEFINED_ERROR
	{
		int err;
		mpeg = mpeg3_open(rb_str_ptr(filename),&err);
		post("mpeg error code = %d",err);
	}
#else
	mpeg = mpeg3_open(rb_str_ptr(filename));
#endif
	if (!mpeg) RAISE("IO Error: can't open file `%s': %s", filename, strerror(errno));
}

\classinfo {install_format("#io:mpeg",4,"mpg mpeg");}
\end class FormatMPEG3
void startup_mpeg3 () {
	\startall
}
