/*
	$Id$

	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

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

#include <QuickTime/QuickTime.h>
#include <QuickTime/Movies.h>
#include <QuickTime/QuickTimeComponents.h>
#include "gridflow.hxx.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <CoreServices/CoreServices.h>
#include <map>
std::map<long,const char *> oserr_table;

const char *oserr_find(long err)
{
	std::map<long,const char *>::iterator it = oserr_table.find(err);
	return (it == oserr_table.end()) ? "undefined error..." : it->second;
}

#define ERR(str)  RAISE("(%s)\nerror #%d : %s", str, err, oserr_find(err))

\class FormatQuickTimeApple : Format {
	Movie movie;
	TimeValue time;
	short movie_file;
	GWorldPtr gw; /* just like an X11 Image or Pixmap, maybe. */
	uint8 *buffer;
	P<Dim> dim;
	int nframe, nframes;
	\constructor (t_symbol *mode, string filename) {
		/*vdc=0;*/ movie=0; time=0; movie_file=0; gw=0; buffer=0; dim=0; nframe=0; nframes=0;
		long err;
		filename = gf_find_file(filename);
		FSSpec fss;
		FSRef fsr;
		err = FSPathMakeRef((const UInt8 *)filename.data(), &fsr, NULL);      if (err) ERR("FSPathMakeRef");
		err = FSGetCatalogInfo(&fsr, kFSCatInfoNone, NULL, NULL, &fss, NULL); if (err) ERR("FSGetCatalogInfo");
		err = OpenMovieFile(&fss,&movie_file,fsRdPerm);                       if (err) ERR("OpenMovieFile");
		NewMovieFromFile(&movie, movie_file, NULL, NULL, newMovieActive, NULL);
		Rect r;
		GetMovieBox(movie, &r);
		post("handle=%d movie=%d tracks=%d", movie_file, movie, GetMovieTrackCount(movie));
		post("duration=%d; timescale=%d cHz", (long)GetMovieDuration(movie), (long)GetMovieTimeScale(movie));
		nframes = GetMovieDuration(movie); /* i don't think so */
		post("rect=((%d..%d),(%d..%d))", r.top, r.bottom, r.left, r.right);
		OffsetRect(&r, -r.left, -r.top);
		SetMovieBox(movie, &r);
		dim = new Dim(r.bottom-r.top, r.right-r.left, 4);
		SetMoviePlayHints(movie, hintsHighQuality, hintsHighQuality);
		buffer = new uint8[dim->prod()];
		err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &r, NULL, NULL, 0, buffer, dim->prod(1));
		if (err) ERR("QTNewGWorldFromPtr");
		return;
	}
	~FormatQuickTimeApple() {
		if (movie) {
			DisposeMovie(movie);
			DisposeGWorld(gw);
			CloseMovieFile(movie_file);
		}
	}
	\decl 0 codec (string c);
	\decl 0 colorspace (string c);
	\decl 0 bang ();
	\decl 0 seek (int frame);
	\decl 0 rewind ();
	\grin 0
};

\def 0 seek (int frame) {nframe=frame;}
\def 0 rewind () {_0_seek(0,0,0);}

\def 0 bang () {
	CGrafPtr savedPort;
	GDHandle savedDevice;
	SetMovieGWorld(movie,gw,GetGWorldDevice(gw));
	Rect r;
	GetMovieBox(movie,&r);
	PixMapHandle pixmap = GetGWorldPixMap(gw);
	short flags = nextTimeStep;
	if (nframe>=nframes) {outlet_bang(bself->te_outlet); return;}
	if (nframe==0) flags |= nextTimeEdgeOK;
	TimeValue duration;
	OSType mediaType = VisualMediaCharacteristic;
	GetMovieNextInterestingTime(movie,
		flags,1,&mediaType,time,0,&time,&duration);
	if (time<0) {
		time=0;
		outlet_bang(bself->te_outlet);
		return;
	}
//	post("quicktime frame #%d; time=%d duration=%d", nframe, (long)time, (long)duration);
	SetMovieTimeValue(movie,nframe*duration);
	MoviesTask(movie,0);
	GridOutlet out(this,0,dim,cast);
	uint32 *bufu32 = (uint32 *)buffer;
	int n = dim->prod()/4;
	int i;
	if (is_le()) {
		for (i=0; i<n&-4; i+=4) {
			bufu32[i+0]=bufu32[i+0]>>8;
			bufu32[i+1]=bufu32[i+1]>>8;
			bufu32[i+2]=bufu32[i+2]>>8;
			bufu32[i+3]=bufu32[i+3]>>8;
		}
		for (; i<n; i++) {
			bufu32[i+0]=bufu32[i+0]>>8;
		}
	} else {
		for (i=0; i<n&-4; i+=4) {
			bufu32[i+0]=(bufu32[i+0]<<8)+(bufu32[i+0]>>24);
			bufu32[i+1]=(bufu32[i+1]<<8)+(bufu32[i+1]>>24);
			bufu32[i+2]=(bufu32[i+2]<<8)+(bufu32[i+2]>>24);
			bufu32[i+3]=(bufu32[i+3]<<8)+(bufu32[i+3]>>24);
		}
		for (; i<n; i++) {
			bufu32[i+0]=(bufu32[i+0]<<8)+(bufu32[i+0]>>24);
		}
	}
	out.send(dim->prod(),buffer);
	int nf=nframe;
	nframe++;
	//return INT2NUM(nf);
}

GRID_INLET(0) {
	RAISE("Unimplemented. Sorry.");
//!@#$
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels (got %d)",in->dim->get(2));
	in->set_chunk(0);
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

\def 0 codec      (string c) { RAISE("Unimplemented. Sorry."); }
\def 0 colorspace (string c) { RAISE("Unimplemented. Sorry."); }

\classinfo {
	EnterMovies();
	install_format("#io.quicktimeapple",4,"mov");
}
\end class FormatQuickTimeApple
void startup_quicktimeapple () {
	#define OSERR(a,b,c) oserr_table[a] = b ": " c;
	#include "MacErrors2.i"
	\startall
}
