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

#include "../base/grid.h.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <quicktime/quicktime.h>
#include <quicktime/colormodels.h>

#include <quicktime/lqt_version.h>
#ifdef LQT_VERSION
#include <quicktime/lqt.h>
#include <quicktime/lqt_codecinfo.h>
#endif

\class FormatQuickTimeApple < Format
struct FormatQuickTimeApple : Format {
	short anim;
	GWorldPtr gw; /* just like an X11 Image or Pixmap, maybe. */
	Pt<uint8> buffer;
	Dim *dim;

	FormatQuickTimeApple() : track(0), dim(0), codec(QUICKTIME_RAW), started(false) {}
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
	GetMovieBox(m_movie, &r);
	PixMapHandle pixmap = GetGWorldPixMap(gw);
	Ptr baseAddr = GetPixBaseAddr(pixmap);
	short flags = nextTimeStep;
	if (nframe>=nframes) return Qfalse;
	if (nframe==0) flags |= nextTimeEdgeOK;
	TimeValue duration;       
	OSType mediaType = VisualMediaCharacteristic;
	GetMovieNextInterestingTime(anim, 
		flags,1,&mediaType,time,0,&time,&duration);
	post("quicktime frame # %d",nframe);
	SetMovieTimeValue(m_movie, time); 
	MoviesTask(anim,0);
	out[0]->begin(dim->dup());
	out[0]->send(buffer);
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
//!@#$
	if (dim) {
		if (!dim->equal(in->dim)) RAISE("all frames should be same size");
	} else {
		/* first frame: have to do setup */
		dim = in->dim->dup();
		quicktime_set_video(anim,1,dim->get(1),dim->get(0),15,codec);
		quicktime_set_cmodel(anim,colorspace);
	}
	int sx = quicktime_video_width(anim,track);
	int sy = quicktime_video_height(anim,track);
	uint8 *rows[sy]; for (int i=0; i<sy; i++) rows[i]=data+i*sx*channels;
	quicktime_encode_video(anim,rows,track);
} GRID_FINISH {
} GRID_END

\def void codec_m (String c) {
	RAISE("Unimplemented. Sorry.");
//!@#$
	//fprintf(stderr,"codec = %s\n",rb_str_ptr(rb_inspect(c)));
#ifdef LQT_VERSION
	char buf[5];
	strncpy(buf,rb_str_ptr(c),4);
	for (int i=rb_str_len(c); i<4; i++) buf[i]=' ';
	buf[4]=0;
	Ruby fourccs = rb_ivar_get(rb_obj_class(rself),SI(@fourccs));
	if (Qnil==rb_hash_aref(fourccs,rb_str_new2(buf)))
		RAISE("warning: unknown fourcc '%s' (%s)",
			buf, rb_str_ptr(rb_inspect(rb_funcall(fourccs,SI(keys),0))));
#endif	
	codec = strdup(buf);
}

\def void colorspace_m (Symbol c) {
	RAISE("Unimplemented. Sorry.");
//!@#$
	if (0) {
	} else if (c==SYM(rgb))  { colorspace=BC_RGB888; channels=3;
	} else if (c==SYM(rgba)) { colorspace=BC_RGBA8888; channels=4;
	} else if (c==SYM(bgr))  { colorspace=BC_BGR888; channels=3;
	} else if (c==SYM(bgrn)) { colorspace=BC_BGR8888; channels=4;
	} else if (c==SYM(yuv))  { colorspace=BC_YUV888; channels=3;
	} else if (c==SYM(yuva)) { colorspace=BC_YUVA8888; channels=4;
	} else RAISE("unknown colorspace '%s' (supported: rgb, rgba, bgr, bgrn, yuv, yuva)",rb_sym_name(c));
}

\def void close () {
//!@#$
	if (anim) { CloseMovieFile(anim); anim=0; }
	rb_call_super(argc,argv);
}

/* note: will not go through jMax data paths */
/* libquicktime may be nice, but it won't take a filehandle, only filename */
\def void initialize (Symbol mode, Symbol source, String filename) {
	rb_call_super(argc,argv);
	if (source!=SYM(file)) RAISE("usage: quicktime file <filename>");
	filename = rb_funcall(mGridFlow,SI(find_file),1,filename);
	int err = OpenMovieFile(rb_str_ptr(filename),&anim,fsRdPerm);
	if (err) RAISE("can't open file `%s': error #%d", rb_str_ptr(filename), err);
	//colorspace_m(0,0,SYM(rgb));
	gfpost("OpenMovieFile:");
	int r;
	GetMovieBox(m_movie, &r);
	gfpost("handle=%d tracks=%d timescale=%d rect=((%d..%d),(%d..%d))",anim,
		GetMovieTrackCount(m_movie)
		(long)GetMovieDuration(m_movie),
		(long)GetMovieTimeScale(m_movie),
		r.top, r.bottom, r.left, r.right);
	OffsetRect(&r, -r.left, -r.top);
	SetMovieBox(anim, &r);
	int v[] = { r.bottom-r.top, r.right-r.left, 4 };
	dim = new Dim(3,v);
	SetMoviePlayHints(anim, hintsHighQuality, hintsHighQuality);
	buffer = ARRAY_NEW(uint8,dim->prod());
	err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &r,
		NULL, NULL, 0, buffer, dim->prod(1));

}

GRCLASS(FormatQuickTimeApple,LIST(GRINLET2(FormatQuickTimeApple,0,4)),
\grdecl
){
	IEVAL(rself,"install 'FormatQuickTimeApple',1,1;"
	"conf_format 4,'quicktime','Apple Quicktime (using Apple\\'s)','mov'");

	gfpost("EnterMovies() == %d",EnterMovies());
}

\end class FormatQuickTimeApple
