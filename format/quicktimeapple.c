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

#define T_DATA T_COCOA_DATA
#include <Quicktime/quicktime.h>
#undef T_DATA
#include "../base/grid.h.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

\class FormatQuickTimeApple < Format
struct FormatQuickTimeApple : Format {
	Movie movie;
	TimeValue time;
	short movie_file;
	GWorldPtr gw; /* just like an X11 Image or Pixmap, maybe. */
	Pt<uint8> buffer;
	Dim *dim;
	int nframe, nframes;

	FormatQuickTimeApple() : movie(0), movie_file(0), gw(0),
		buffer(), dim(0), nframe(0), nframes(0) {}
	\decl void initialize (Symbol mode, Symbol source, String filename);
	\decl void close ();
	\decl void codec_m (String c);
	\decl void colorspace_m (Symbol c);
	\decl Ruby frame ();
	\decl void seek (int frame);
	GRINLET3(0);
};

\def void seek (int frame) {
//!@#$
}

\def Ruby frame () {
	CGrafPtr savedPort;
	GDHandle savedDevice;
	GetGWorld(&savedPort, &savedDevice);
	SetGWorld(gw, NULL);
	Rect r;
	GetMovieBox(movie,&r);
	PixMapHandle pixmap = GetGWorldPixMap(gw);
	Ptr baseAddr = GetPixBaseAddr(pixmap);
	short flags = nextTimeStep;
//	if (nframe>=nframes) return Qfalse;
//	if (nframe==0) flags |= nextTimeEdgeOK;
	TimeValue duration;       
	OSType mediaType = VisualMediaCharacteristic;
	GetMovieNextInterestingTime(movie, 
		flags,1,&mediaType,time,0,&time,&duration);
	gfpost("quicktime frame # %d",index);
	SetMovieTimeValue(movie, time); 
	MoviesTask(movie,0);
	out[0]->begin(dim->dup());
	out[0]->send(dim->prod(),buffer);
	return INT2NUM(nframe);
}

GRID_INLET(FormatQuickTimeApple,0) {
	RAISE("Unimplemented. Sorry.");
//!@#$
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels (got %d)",in->dim->get(2));
	in->set_factor(in->dim->prod());
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

\def void codec_m (String c) {
	RAISE("Unimplemented. Sorry.");
//!@#$
}

\def void colorspace_m (Symbol c) {
	RAISE("Unimplemented. Sorry.");
//!@#$
/*	if (0) {
	} else if (c==SYM(rgb))  { colorspace=BC_RGB888; channels=3;
	} else if (c==SYM(rgba)) { colorspace=BC_RGBA8888; channels=4;
	} else if (c==SYM(bgr))  { colorspace=BC_BGR888; channels=3;
	} else if (c==SYM(bgrn)) { colorspace=BC_BGR8888; channels=4;
	} else if (c==SYM(yuv))  { colorspace=BC_YUV888; channels=3;
	} else if (c==SYM(yuva)) { colorspace=BC_YUVA8888; channels=4;
	} else RAISE("unknown colorspace '%s' (supported: rgb, rgba, bgr, bgrn, yuv, yuva)",rb_sym_name(c));
*/
}

\def void close () {
//!@#$
	if (movie) { 
		DisposeMovie(movie);
		DisposeGWorld(gw);
		CloseMovieFile(movie_file);
		movie_file=0;
	}
	rb_call_super(argc,argv);
}

/* note: will not go through jMax data paths */
/* libquicktime may be nice, but it won't take a filehandle, only filename */
\def void initialize (Symbol mode, Symbol source, String filename) {
	rb_call_super(argc,argv);
	if (source!=SYM(file)) RAISE("usage: quicktime file <filename>");
	filename = rb_funcall(mGridFlow,SI(find_file),1,filename);
	FSSpec fss;
	FSRef fsr;
	int err = FSPathMakeRef((const UInt8 *)rb_str_ptr(filename), &fsr, NULL);
	if (err) goto err;
	err = FSGetCatalogInfo(&fsr, kFSCatInfoNone, NULL, NULL, &fss, NULL);
	if (err) goto err;
	err = OpenMovieFile(&fss,&movie_file,fsRdPerm);
	if (err) goto err;
	//colorspace_m(0,0,SYM(rgb));
	gfpost("OpenMovieFile:");
	Rect r;
	NewMovieFromFile(&movie, movie_file, NULL, NULL, newMovieActive, NULL);
	GetMovieBox(movie, &r);
	gfpost("handle=%d movie=%d tracks=%d timescale=%d rect=((%d..%d),(%d..%d))",
		movie_file, movie,
		GetMovieTrackCount(movie),
		(long)GetMovieDuration(movie),
		(long)GetMovieTimeScale(movie),
		r.top, r.bottom, r.left, r.right);
	OffsetRect(&r, -r.left, -r.top);
	SetMovieBox(movie, &r);
	{int32 v[] = { r.bottom-r.top, r.right-r.left, 4 };
		dim = new Dim(3,v);}
	SetMoviePlayHints(movie, hintsHighQuality, hintsHighQuality);
	buffer = ARRAY_NEW(uint8,dim->prod());
	err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &r,
		NULL, NULL, 0, buffer, dim->prod(1));

	return;
err:
	RAISE("can't open file `%s': error #%d", rb_str_ptr(filename), err);
}

GRCLASS(FormatQuickTimeApple,LIST(GRINLET2(FormatQuickTimeApple,0,4)),
\grdecl
){
	IEVAL(rself,"install 'FormatQuickTimeApple',1,1;"
	"conf_format 4,'quicktime','Apple Quicktime (using Apple\\'s)','mov'");

	gfpost("EnterMovies() == %d",EnterMovies());
}

\end class FormatQuickTimeApple
