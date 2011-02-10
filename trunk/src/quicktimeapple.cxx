/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

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
#include <CoreServices/CoreServices.h>
#include "gridflow.hxx.fcs"
#include "colorspace.hxx"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <map>
map<long,const char *> oserr_table;

const char *oserr_find(long err)
{
	map<long,const char *>::iterator it = oserr_table.find(err);
	return (it == oserr_table.end()) ? "undefined error..." : it->second;
}

#define ERR(str)  RAISE("(%s)\nerror #%d : %s", str, err, oserr_find(err))

\class FormatQuickTimeApple : Format {
	Movie movie;
	Track track;
	Media media;
	TimeValue time;
	ComponentResult compErr;
	ComponentInstance component;
	SCTemporalSettings temporal;
	SCSpatialSettings spatial;
	SCDataRateSettings datarate;
	ImageDescriptionHandle imagedesc;
	CodecType codec;
	GWorldPtr gw; /* just like an X11 Image or Pixmap, maybe. */
	Rect r;
	uint8 *buffer, *buf2;
	Dim dim;
	int nframe, nframes, quality;
	short movie_file;
	P<BitPacking> bit_packing3;
	bool gotdim;
	OSErr err;
	\constructor (t_symbol *mode, string filename) {
		/*vdc=0;*/
		movie=0; time=0; movie_file=0; gw=0; buffer=0; dim=0; nframe=0; nframes=0; quality=0;
		gotdim=false; err=noErr; compErr=noErr; component=NULL;
		filename = gf_find_file(filename);
		FSSpec fss;
		FSRef fsr;

		if (mode==gensym("in")) {
			err = FSPathMakeRef((const UInt8 *)filename.data(), &fsr, NULL);
			if (err) ERR("FSPathMakeRef");
			err = FSGetCatalogInfo(&fsr, kFSCatInfoNone, NULL, NULL, &fss, NULL);
			if (err) ERR("FSGetCatalogInfo");
			err = OpenMovieFile(&fss,&movie_file,fsRdPerm);
			if (err) ERR("OpenMovieFile");
			else post("\n#io.quicktimeapple: opened movie file %s", filename.data());
			NewMovieFromFile(&movie, movie_file, NULL, NULL, newMovieActive, NULL);

			track = GetMovieIndTrackType(movie,1,VideoMediaType,movieTrackMediaType);
			media = GetTrackMedia(track);
			nframes = GetMediaSampleCount(media);			
			//post("total frames: %d", nframes);

			GetMovieBox(movie, &r);
			//post("height: %d  width: %d", r.bottom, r.right);

			//post("handle=%d movie=%d tracks=%d", movie_file, movie, GetMovieTrackCount(movie));
			//post("duration=%d; timescale=%d cHz", (long)GetMovieDuration(movie), (long)GetMovieTimeScale(movie));
			//post("rect=((%d..%d),(%d..%d))", r.top, r.bottom, r.left, r.right);

			OffsetRect(&r, -r.left, -r.top);
			SetMovieBox(movie, &r);
			dim = Dim(r.bottom-r.top, r.right-r.left, 4);
			SetMoviePlayHints(movie, hintsHighQuality, hintsHighQuality);
			buffer = new uint8[dim.prod()];
			buf2 = new uint8[dim.prod()];
			err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &r, NULL, NULL, 0, buffer, dim.prod(1));
			if (err) ERR("QTNewGWorldFromPtr");
			_0_colorspace(gensym("rgba"));

		} else if (mode=gensym("out")) {
			err = FSPathMakeRef((UInt8*)filename.data(), &fsr, NULL);
			if (err == fnfErr) { /* if file doesn't exist, create it */ 
				int fd = open(filename.data(), O_CREAT | O_RDWR, 0644);
				if (fd < 0) RAISE("can't create file");
				//else post("#io.quicktimeapple: created file %s", filename.data());
				close(fd);
				err = FSPathMakeRef((UInt8*)filename.data(), &fsr, NULL);
			}
			if (err) ERR("FSPathMakeRef");
			err = FSGetCatalogInfo(&fsr, kFSCatInfoNodeFlags, NULL, NULL, &fss, NULL);
			if (err) ERR("FSGetCatalogInfo");
			err = FSMakeFSSpec(fss.vRefNum, fss.parID, (UInt8*)filename.data(), &fss);			
			if (err && err != bdNamErr) ERR("FSMakeFSSpec");
			err = CreateMovieFile(&fss, FOUR_CHAR_CODE('TVOD'), smSystemScript,
				createMovieFileDeleteCurFile | createMovieFileDontCreateResFile,
				&movie_file, &movie);
			if (err != noErr) ERR("CreateMovieFile");
			component = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);
			if (!component) RAISE("OpenDefaultComponent failed");

			// unfinished... these values are user-defined
			spatial.codecType = kJPEGCodecType;
			spatial.codec = (CodecComponent)65719; // quoi ?
			spatial.depth = 0;
			spatial.spatialQuality = codecNormalQuality;
			temporal.temporalQuality = codecNormalQuality;
			temporal.frameRate = 0;
			temporal.keyFrameRate = 0;
			datarate.frameDuration = 33;

		} else {
			RAISE("invalid mode (%s)", mode->s_name);
		}
		return;
	}
	~FormatQuickTimeApple() {
		if (movie) {
			DisposeMovie(movie);
			DisposeGWorld(gw);
			CloseMovieFile(movie_file);
		}
		if (component){
			compErr = CloseComponent(component);
			if (compErr != noErr) RAISE("(CloseComponent)\nerror #%d : %s", compErr, oserr_find(compErr));
		}
	}
	\decl 0 codec (string c);
	\decl 0 bang ();
	\decl 0 seek (int frame);
	\decl 0 rewind ();
	\decl 0 size (int32 height, int32 width);
	\grin 0

	\attr t_symbol *colorspace;
	\attr int32 frames();
	//\attr float64 framerate();
	\attr int32 height();
	\attr int32 width();
	//\attr int32 depth();
	//\attr bool with_audio;

};

\def 0 seek (int frame) {nframe=frame;}
\def 0 rewind () {_0_seek(0);}

\def 0 bang () {
	CGrafPtr savedPort;
	GDHandle savedDevice;
	SetMovieGWorld(movie,gw,GetGWorldDevice(gw));
	Rect r;
	GetMovieBox(movie,&r);
	PixMapHandle pixmap = GetGWorldPixMap(gw);
	short flags = nextTimeStep;
	if (nframe>=nframes) {out[0](); return;}
	if (nframe==0) flags |= nextTimeEdgeOK;
	TimeValue duration;
	OSType mediaType = VisualMediaCharacteristic;
	GetMovieNextInterestingTime(movie,flags,1,&mediaType,time,0,&time,&duration);
	if (time<0) {
		time=0;
		out[0]();
		return;
	}
//	post("quicktime frame #%d; time=%d duration=%d", nframe, (long)time, (long)duration);
	SetMovieTimeValue(movie,nframe*duration);
	MoviesTask(movie,0);

//////// début de copier-coller de #io.quicktimeapple 0 bang

	GridOut out(this,0,dim,cast);
	string cs = colorspace->s_name;
	int sy = dim[0];
	int sx = dim[1];
	int sc = dim[2];
	uint8 rgb[sx*4+4]; // with extra padding in case of odd size...
	uint8 b2[ sx*3+3];
	uint8 *buf = buffer; // sorry
	int bs = sx*sc;
	if (cs=="y") {
		for(int y=0; y<sy; y++) {
		        bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
			for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
				b2[x+0] = (76*rgb[xx+0]+150*rgb[xx+1]+29*rgb[xx+2])>>8;
				b2[x+1] = (76*rgb[xx+3]+150*rgb[xx+4]+29*rgb[xx+5])>>8;
			}
			out.send(bs,b2);
		}
	} else if (cs=="yuv") {
		for(int y=0; y<sy; y++) {
			bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
			for (int x=0,xx=0; x<sx; x++,xx+=3) {
				b2[xx+0] = RGB2Y(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
				b2[xx+1] = RGB2U(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
				b2[xx+2] = RGB2V(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
			}
			out.send(bs,b2);
		}
	} else if (cs=="rgb") {
		int n = dim.prod()/3;
		/*for (int i=0,j=0; i<n; i+=4,j+=3) {
			buf2[j+0] = buf[i+0];
			buf2[j+1] = buf[i+1];
			buf2[j+2] = buf[i+2];
		}*/
		//bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
		bit_packing3->unpack(n,buf,buf2);
		out.send(dim.prod(),buf2);
	} else if (cs=="rgba") { // does this really work on PPC ?
		int n = dim.prod()/4;
		if (is_le()) {
			for (int i=0; i<n; i++) ((uint32 *)buf2)[i] = (((uint32 *)buf)[i] >> 8) | 0xff000000;

		} else {
			for (int i=0; i<n; i++) ((uint32 *)buf2)[i] = (((uint32 *)buf)[i] << 8) | 0x000000ff;
		}
		
		out.send(dim.prod(),buf2);
	} else
		RAISE("colorspace problem");

//////// fin de copier-coller

	int nf=nframe;
	nframe++;
	//return INT2NUM(nf);
}

GRID_INLET(0) {
	RAISE("Unimplemented. Sorry.");
//!@#$
	if (in.dim.n  != 3) RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in.dim[2] != 3) RAISE("expecting 3 channels (got %d)",in.dim[2]);
	in.set_chunk(0);
	if (gotdim) {
		if (dim != in.dim) RAISE("all frames should be same size");
	} else {
		dim = in.dim;
		gotdim = true;
		r.top = 0;
		r.left = 0;
		r.bottom = dim[0];
		r.right = dim[1];
		compErr = SCSetInfo(component, scTemporalSettingsType, &temporal);
		compErr = SCSetInfo(component, scSpatialSettingsType, &spatial);
		compErr = SCSetInfo(component, scDataRateSettingsType, &datarate);
		if (compErr != noErr) RAISE("(SCSetInfo)\nerror #%d : %s", int(compErr), oserr_find(compErr));
		compErr = SCCompressSequenceBegin(component, GetPortPixMap(gw), &r, &imagedesc);
		if (compErr != noErr) RAISE("(SCCompressSequenceBegin)\nerror #%d : %s", int(compErr), oserr_find(compErr));
		err = BeginMediaEdits(media);
		if (err != noErr) ERR("BeginMediaEdits");
	}
////	err = QTNewGWorldFromPtr(&m_srcGWorld, k32ARGBPixelFormat, &m_srcRect, NULL, NULL, 0, m_compressImage.data, m_rowBytes);
	err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &r, NULL, NULL, 0, buffer, dim.prod(1));
	if (err) ERR("QTNewGWorldFromPtr");
	_0_colorspace(gensym("rgba"));
	SetMovieGWorld(movie, gw, GetGWorldDevice(gw));

	track = NewMovieTrack(movie, FixRatio(r.right, 1), FixRatio(r.bottom, 1), kNoVolume);	
	media = NewTrackMedia(track, VideoMediaType, 600, NULL, 0);

} GRID_FLOW {
} GRID_FINISH {
} GRID_END

\def int32 frames () { return nframes; }
\def int32 height () { return r.bottom; }
\def int32 width  () { return r.right; }
\def 0 frames (int32 v) { RAISE("read-only"); }
\def 0 height (int32 v) { RAISE("use size instead"); }
\def 0 width  (int32 v) { RAISE("use size instead"); }

\def 0 codec (string c) { RAISE("Unimplemented. Sorry."); }

void post_BitPacking(BitPacking *);

// copy of the one in #io.quicktimecamera
\def 0 colorspace (t_symbol *colorspace) { /* y yuv rgb rgba magic */
	string c = colorspace->s_name;
	if (c=="y"    ) {} else
	if (c=="yuv"  ) {} else
	if (c=="rgb"  ) {} else
	if (c=="rgba" ) {} else
	//if (c=="magic") {} else
	RAISE("got '%s' but supported colorspaces are: y yuv rgb rgba",c.data());
	if (is_le()) {
		uint32 masks[4]={0x0000ff00,0x00ff0000,0xff000000,0x00000000};
		bit_packing3 = new BitPacking(is_le(),4,3,masks);
	} else {
		uint32 masks[4]={0x00ff0000,0x0000ff00,0x000000ff,0x00000000};
		bit_packing3 = new BitPacking(is_le(),4,3,masks);
	}
	//post_BitPacking(bit_packing3);
	//bit_packing4 = new BitPacking(is_le(),bytes,4,masks);
	this->colorspace=gensym(c.data());
	dim = Dim(dim[0],dim[1],c=="y"?1:c=="rgba"?4:3);
}

\def 0 size (int32 height, int32 width) {
	if (gotdim) RAISE("video size already set!");
	// first frame: have to do setup
	dim = Dim(height,width,3);
	gotdim = true;
	r.top = 0;
	r.left = 0;
	r.bottom = height;
	r.right = width;
}

\classinfo {
	EnterMovies();
	install_format("#io.quicktimeapple",6,"mov");
}
\end class FormatQuickTimeApple
void startup_quicktimeapple () {
	#define OSERR(a,b,c) oserr_table[a] = b ": " c;
	#include "MacErrors2.i"
	\startall
}
