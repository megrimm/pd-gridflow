/*
	$Id$

	GridFlow
	Copyright (c) 2001-2006 by Mathieu Bouchard

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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>

/* **************************************************************** */

typedef video_capability VideoCapability;
typedef video_channel    VideoChannel   ;
typedef video_tuner      VideoTuner     ;
typedef video_window     VideoWindow    ;
typedef video_picture    VideoPicture   ;
typedef video_mbuf       VideoMbuf      ;
typedef video_mmap       VideoMmap      ;

static const char *ruby_code =
\ruby
module ::GridFlow
add :VideoTypeFlags,MultiChoiceType[*%w(
	CAPTURE TUNER TELETEXT OVERLAY CHROMAKEY CLIPPING FRAMERAM
	SCALES MONOCHROME SUBCAPTURE
	MPEG_DECODER MPEG_ENCODER MJPEG_DECODER MJPEG_ENCODER
).map{|x|x.intern}]
add :VideoTunerFlags,MultiChoiceType[*%w(
	PAL NTSC SECAM LOW NORM DUMMY5 DUMMY6 STEREO_ON RDS_ON MBS_ON
).map{|x|x.intern}]
add :VideoPalette,ChoiceType[%w(
	NIL GREY HI240
	RGB565 RGB24 RGB32 RGB555
	YUV422 YUYV UYVY YUV420 YUV411 RAW
	YUV422P YUV411P YUV420P YUV410P
).map{|x|x.intern}]
add :VideoTunerMode,        ChoiceType[:PAL,:NTSC,:SECAM,:AUTO]
add :VideoWindowFlags, MultiChoiceType[:INTERLACE,:FOO1,:FOO2,:FOO3,:CHROMAKEY]
add :VideoAudioFlags,  MultiChoiceType[*%w[MUTE MUTABLE VOLUME BASS TREBLE BALANCE].map{|x|x.intern}]
add :VideoSoundFlags,  MultiChoiceType[*%w[MONO STEREO LANG1 LANG2].map{|x|x.intern}]
add :VideoChannelFlags,MultiChoiceType[:TUNER,:AUDIO]
add :VideoChannelType, MultiChoiceType[:TV,:CAMERA]
add :VideoPlayMaster,       ChoiceType[*%w[FOO1 NONE VIDEO AUDIO].map{|x|x.intern}]
add :VideoWrite,            ChoiceType[*%w[MPEG_AUD MPEG_VID OSD TTX CC MJPEG].map{|x|x.intern}]
add :VBIFormatFlags, MultiChoiceType[:UNSYNC,:VBI_INTERLACED]
add :VideoPlay, ChoiceType[*%w[
	VID_OUT_MODE GENLOCK NORMAL PAUSE SINGLE_FRAME
	FAST_FORWARD SLOW_MOTION IMMEDIATE_NORMAL SWITCH_CHANNELS FREEZE_FRAME
	STILL_MODE MASTER_MODE ACTIVE_SCANLINES RESET END_MARK
].map{|x|x.intern}]
add :VideoHardware, ChoiceType[*%w[
	NIL BT848 QCAM_BW PMS QCAM_C PSEUDO SAA5249 AZTECH SF16MI RTRACK
	ZOLTRIX	SAA7146 VIDEUM RTRACK2 PERMEDIA2 RIVA128 PLANB BROADWAY GEMTEK TYPHOON
	VINO CADET TRUST TERRATEC CPIA ZR36120 ZR36067 OV511 ZR356700 W9966
	SE401 PWC MEYE CPIA2 VICAM SF16FMR2 W9968CF SAA7114H SN9C102 ARV
].map{|x|x.intern}]

 Int =  Long =  Int32          = Integer
Uint = Ulong = Uint32 = Uint16 = Integer

class VideoCapability<Form; fields_are \
	[:name,String], # 32 chars
	[:type,Int],
	[:channels,Int],
	[:audios,Int],
	[:maxwidth,Int], [:maxheight,Int],
	[:minwidth,Int], [:minheight,Int] end
class VideoChannel<Form; fields_are \
	[:channel, Int],
	[:name, String], # 32 bytes
	[:tuners,Int],
	[:flags,VideoChannelFlags],
	[:tipe,VideoChannelType], # type is a reserved name in Ruby
	[:norm, Uint16] end
class VideoTuner<Form; fields_are \
	[:tuner,Int],
	[:name, String], # 32 bytes
	[:rangelow, Ulong],
	[:rangehigh, Ulong],
	[:flags, VideoTunerFlags],
	[:mode, VideoTunerMode],
	[:signal, Uint16] end
class VideoPicture<Form; fields_are \
	[:brightness,Uint16],
	[:hue,Uint16],
	[:colour,Uint16],
	[:contrast,Uint16],
	[:whiteness,Uint16],
	[:depth,Uint16],
	[:palette,Uint16] end
class VideoAudio<Form; fields_are \
	[:audio,Int],
	[:volume,Uint16],
	[:bass,Uint16],
	[:treble,Uint16],
	[:flags,VideoAudioFlags],
	[:name, String], # 16 bytes
	[:mode,Uint16],
	[:balance,Uint16],
	[:step,Uint16] end
class VideoClip<Form; fields_are \
	[:x,    Int32], [:y,     Int32],
	[:width,Int32], [:height,Int32] end
#	struct video_clip *next end
class VideoWindow<Form; fields_are \
	[:x,    Uint32], [:y,     Uint32],
	[:width,Uint32], [:height,Uint32],
	[:chromakey,Uint32],
	[:flags,VideoWindowFlags],
#	struct	video_clip *clips;	/* Set only */
	[:clipcount,Int] end
#define VIDEO_CLIP_BITMAP	-1
#define VIDEO_CLIPMAP_SIZE	(128 * 625)
class VideoCapture<Form; fields_are \
	[:x,    Uint32], [:y,     Uint32],
	[:width,Uint32], [:height,Uint32],
	[:decimation,Uint16] end
#	[:flags,VideoCaptureFlags],
#define VIDEO_CAPTURE_ODD
#define VIDEO_CAPTURE_EVEN
class VideoBuffer<Form; fields_are \
	[:base,Long], # void*
	[:height,Int], [:width,Int],
	[:depth,Int],
	[:bytesperline,Int] end
class VideoMmap<Form; fields_are \
	[:frame,Uint],
	[:height,Int], [:width,Int],
	[:format,Uint] end
class VideoKey<Form; fields_are \
	[:key,String] end # 8 bytes
#	[:flaga,VideoKeyFlags] end
#define VIDEO_MAX_FRAME		32
class VideoMbuf<Form; fields_are \
	[:size,Int],
	[:frames,Int] end#,
#	[:frames,Array.of(Int,VIDEO_MAX_FRAME) end
#define 	VIDEO_NO_UNIT	(-1)
class VideoUnit<Form; fields_are \
	[:video,Int],
	[:vbi,Int],
	[:radio,Int],
	[:audio,Int],
	[:teletext,Int] end
class VBIFormat<Form; fields_are \
	[:sampling_rate,Uint32],
	[:samples_per_line,Uint32],
	[:sample_format,Uint32],
#	[:start,Array.of( Int32,2)],
#	[:count,Array.of(Uint32,2)],
	[:flags,VBIFormatFlags] end
class VideoInfo<Form; fields_are \
	[:frame_count,Uint32],
	[:h_size,Uint32], [:v_size,Uint32],
	[:smpte_timecode,Uint32],
	[:picture_type,Uint32],
	[:temporal_reference,Uint32] end
#	[:user_data, Array[Byte,256] end
#	/* user_data[0] contains user data flags, user_data[1] has count */

class VideoPlayMode<Form; fields_are \
	[:mode,Int],
	[:p1,Int],
	[:p2,Int] end

class VideoCode<Form; fields_are \
	[:loadwhat,String], # 16 bytes
	[:datasize,Int] end
#	__u8	*data;

#-------------------------------------------------------

#[VideoTypeFlags,VideoTunerFlags,VideoChannelFlags,VideoPalette,VideoTunerMode].each {|x|
#	GridFlow.post "%s: %s", x.inspect, x.index_to_val.inspect
#}
def self.postit pickle
#	head = pickle.shift
#	stuff = const_get(head)[*pickle]
#	GridFlow.post "%s",stuff.inspect
	GridFlow.post "%s",pickle.inspect
end
end # module GridFlow
\end ruby
;

class RStream {
public:
	Ruby a;
	RStream(Ruby a) {this->a = a;}
	RStream()       {this->a = rb_ary_new();}
	RStream &operator <<(/*Ruby*/ void *v) { rb_ary_push(a,(Ruby)v); return *this; }
	RStream &operator <<(int v) { return *this<<(void *)INT2NUM(v); }
};
static void gfpost(RStream &rs) {rb_funcall(mGridFlow,SI(postit),1,rs.a);}
static void gfpost(VideoChannel *self) {RStream rs;rs
	<< (void *)SYM(VideoChannel) << self->channel
	<< (void *)rb_str_new(self->name,strnlen(self->name,32))
	<< self->tuners << self->flags << self->type << self->norm
;gfpost(rs);}
static void gfpost(VideoTuner *self) {RStream rs;rs
	<< (void *)SYM(VideoTuner) << self->tuner
	<< (void *)rb_str_new(self->name,strnlen(self->name,32))
	<< self->rangelow << self->rangehigh
	<< self->flags << self->mode << self->signal
;gfpost(rs);}
static void gfpost(VideoCapability *self) {RStream rs;rs
	<< (void *)SYM(VideoCapability)
	<< (void *)rb_str_new(self->name,strnlen(self->name,32))
	<< self->type
	<< self->channels << self->audios
	<< self->maxheight << self->maxwidth
	<< self->minheight << self->minwidth
;gfpost(rs);}
static void gfpost(VideoWindow *self) {RStream rs;rs
	<< (void *)SYM(VideoWindow)
	<< self->y << self->x 
	<< self->height << self->width
	<< self->chromakey << self->flags << self->clipcount
;gfpost(rs);}
static void gfpost(VideoPicture *self) {RStream rs;rs
	<< (void *)SYM(VideoPicture)
	<< self->brightness << self->contrast << self->colour
	<< self->hue << self->whiteness << self->depth << self->palette
;gfpost(rs);}
static void gfpost(VideoMbuf *self) {RStream rs;rs
	<< (void *)SYM(VideoMBuf) << self->size << self->frames;
	for (int i=0; i<4; i++) rs << self->offsets[i];
;gfpost(rs);}
static void gfpost(VideoMmap *self) {RStream rs;rs
	<< (void *)SYM(VideoMMap) << self->frame
	<< self->height << self->width << self->format
;gfpost(rs);}

/* **************************************************************** */

\class FormatVideoDev < Format
struct FormatVideoDev : Format {
	VideoCapability vcaps;
	VideoPicture vp;
	VideoMbuf vmbuf;
	VideoMmap vmmap;
	uint8 *image;
	int palette;
	int queue[8], queuesize, queuemax, next_frame;
	int current_channel, current_tuner;
	bool use_mmap;
	P<BitPacking> bit_packing;
	P<Dim> dim;
	int fd;

	FormatVideoDev () : queuesize(0), queuemax(2), next_frame(0), use_mmap(true), bit_packing(0), dim(0) {}
	void frame_finished (uint8 * buf);

	\decl void initialize (Symbol mode, String filename, Symbol option=Qnil);
	\decl void initialize2 ();
	\decl void close ();
	\decl void alloc_image ();
	\decl void dealloc_image ();
	\decl void frame ();
	\decl void frame_ask ();
	\grin 0 int

	\decl void _0_size (int sy, int sx);
	\decl void _0_norm (int value);
	\decl void _0_tuner (int value);
	\decl void _0_channel (int value);
	\decl void _0_transfer (Symbol sym, int queuemax=2);
	\decl void _0_colorspace (Symbol c);
	\attr int    frequency();
	\attr uint16 brightness();
	\attr uint16 hue();
	\attr uint16 colour();
	\attr uint16 contrast();
	\attr uint16 whiteness();
};

#define DEBUG(args...) 42
//#define DEBUG(args...) gfpost(args)

#define  IOCTL( F,NAME,ARG) \
  (DEBUG("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",F,NAME,#NAME,_arg_), ioctl(F,NAME,ARG))
#define WIOCTL( F,NAME,ARG) \
  (IOCTL(F,NAME,ARG)<0 && (gfpost("ioctl %s: %s",#NAME,strerror(errno)),1))
#define WIOCTL2(F,NAME,ARG) \
  (IOCTL(F,NAME,ARG)<0 && (gfpost("ioctl %s: %s",#NAME,strerror(errno)), RAISE("ioctl error"), 0))

#define GETFD NUM2INT(rb_funcall(rb_ivar_get(rself,SI(@stream)),SI(fileno),0))

\def void _0_size (int sy, int sx) {
	VideoWindow grab_win;
	// !@#$ bug here: won't flush the frame queue
	dim = new Dim(sy,sx,3);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	gfpost(&grab_win);
	grab_win.clipcount = 0;
	grab_win.flags = 0;
	if (sy && sx) {
		grab_win.height = sy;
		grab_win.width  = sx;
	}
	gfpost(&grab_win);
	WIOCTL(fd, VIDIOCSWIN, &grab_win);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	gfpost(&grab_win);
}

\def void dealloc_image () {
	if (!image) return;
	if (use_mmap) {
		munmap(image, vmbuf.size);
		image=0;
	} else {
		delete[] (uint8 *)image;
	}
}

\def void alloc_image () {
	if (use_mmap) {
		WIOCTL2(fd, VIDIOCGMBUF, &vmbuf);
		image = (uint8 *)mmap(0,vmbuf.size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
		if (((long)image)==-1) {image=0; RAISE("mmap: %s", strerror(errno));}
	} else {
		image = new uint8[dim->prod(0,1)*bit_packing->bytes];
	}
}

\def void frame_ask () {
	if (queuesize>=queuemax) RAISE("queue is full (queuemax=%d)",queuemax);
	if (queuesize>=vmbuf.frames) RAISE("queue is full (vmbuf.frames=%d)",vmbuf.frames);
	vmmap.frame = queue[queuesize++] = next_frame;
	vmmap.format = palette;
	vmmap.width  = dim->get(1);
	vmmap.height = dim->get(0);
	WIOCTL2(fd, VIDIOCMCAPTURE, &vmmap);
	next_frame = (next_frame+1) % vmbuf.frames;
}

void FormatVideoDev::frame_finished (uint8 * buf) {
	GridOutlet out(this,0,dim,NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
	/* picture is converted here. */
	int sy = dim->get(0);
	int sx = dim->get(1);
	int bs = dim->prod(1);
	if (palette==VIDEO_PALETTE_YUV420P) {
		uint8 b2[bs];
		for(int y=0; y<sy; y++) {
			uint8 *bufy = buf+sx* y;
			uint8 *bufu = buf+sx*sy    +(sx/2)*(y/2);
			uint8 *bufv = buf+sx*sy*5/4+(sx/2)*(y/2);
			for (int x=0; x<sx; x++) {
				b2[x*3+0]=bufy[x];
				b2[x*3+1]=bufu[x/2];
				b2[x*3+2]=bufv[x/2];
			}
			out.send(bs,b2);
		}
	} else if (bit_packing) {
		uint8 b2[bs];
		for(int y=0; y<sy; y++) {
			uint8 * buf2 = buf+bit_packing->bytes*sx*y;
			bit_packing->unpack(sx,buf2,b2);
			out.send(bs,b2);
		}
	} else {
		out.send(sy*bs,buf);
	}
}

// strange that read2 is not used and read3 is used instead
static int read2(int fd, uint8 *image, int n) {
	int r=0;
	while (n>0) {
		int rr=read(fd,image,n);
		if (rr<0) return rr; else {r+=rr; image+=rr; n-=rr;}
	}
	return r;
}

static int read3(int fd, uint8 *image, int n) {
	int r=read(fd,image,n);
	if (r<0) return r;
	return n;
}

\def void frame () {
	if (!image) rb_funcall(rself,SI(alloc_image),0);
	if (!use_mmap) {
		/* picture is read at once by frame() to facilitate debugging. */
		int tot = dim->prod(0,1) * bit_packing->bytes;
		int n = (int) read3(fd,image,tot);
		if (n==tot) frame_finished(image);
		if (0> n) RAISE("error reading: %s", strerror(errno));
		if (n < tot) RAISE("unexpectedly short picture: %d of %d",n,tot);
		return;
	}
	while(queuesize<queuemax) rb_funcall(rself,SI(frame_ask),0);
	vmmap.frame = queue[0];
	//uint64 t0 = gf_timeofday();
	WIOCTL2(fd, VIDIOCSYNC, &vmmap);
	//uint64 t1 = gf_timeofday();
	//if (t1-t0 > 100) gfpost("VIDIOCSYNC delay: %d us",t1-t0);
	frame_finished(image+vmbuf.offsets[queue[0]]);
	queuesize--;
	for (int i=0; i<queuesize; i++) queue[i]=queue[i+1];
	rb_funcall(rself,SI(frame_ask),0);
}

GRID_INLET(FormatVideoDev,0) {
	RAISE("can't write.");
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

\def void _0_norm (int value) {
	VideoTuner vtuner;
	vtuner.tuner = current_tuner;
	if (value<0 || value>3) RAISE("norm must be in range 0..3");
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) {
		gfpost("no tuner #%d", value);
	} else {
		vtuner.mode = value;
		gfpost(&vtuner);
		WIOCTL(fd, VIDIOCSTUNER, &vtuner);
	}
}

\def void _0_tuner (int value) {
	VideoTuner vtuner;
	vtuner.tuner = current_tuner = value;
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) RAISE("no tuner #%d", value);
	vtuner.mode = VIDEO_MODE_NTSC; //???
	gfpost(&vtuner);
	WIOCTL(fd, VIDIOCSTUNER, &vtuner);
}

\def void _0_channel (int value) {
	VideoChannel vchan;
	vchan.channel = value;
	current_channel = value;
	if (0> IOCTL(fd, VIDIOCGCHAN, &vchan)) RAISE("no channel #%d", value);
	gfpost(&vchan);
	WIOCTL(fd, VIDIOCSCHAN, &vchan);
	if (vcaps.type & VID_TYPE_TUNER) rb_funcall(rself,SI(_0_tuner),1,INT2NUM(0));
}

\def void _0_transfer (Symbol sym, int queuemax=2) {
	if (sym == SYM(read)) {
		rb_funcall(rself,SI(dealloc_image),0);
		use_mmap = false;
		gfpost("transfer read");
	} else if (sym == SYM(mmap)) {
		rb_funcall(rself,SI(dealloc_image),0);
		use_mmap = true;
		rb_funcall(rself,SI(alloc_image),0);
		queuemax=min(queuemax,vmbuf.frames);
		gfpost("transfer mmap with queuemax=%d (max max is vmbuf.frames=%d)"
			,queuemax,vmbuf.frames);
		this->queuemax=queuemax;
	} else RAISE("don't know that transfer mode");
}

#define PICTURE_ATTR(_name_) {\
	WIOCTL(fd, VIDIOCGPICT, &vp); \
	vp._name_ = _name_; \
	WIOCTL(fd, VIDIOCSPICT, &vp);}

#define PICTURE_ATTRGET(_name_) { \
	WIOCTL(fd, VIDIOCGPICT, &vp); \
	gfpost("getting %s=%d",#_name_,vp._name_); \
	return vp._name_;}

\def uint16    brightness ()                 {PICTURE_ATTRGET(brightness)}
\def void   _0_brightness (uint16 brightness){PICTURE_ATTR(   brightness)}
\def uint16    hue        ()                 {PICTURE_ATTRGET(hue)}
\def void   _0_hue        (uint16 hue)       {PICTURE_ATTR(   hue)}
\def uint16    colour     ()                 {PICTURE_ATTRGET(colour)}
\def void   _0_colour     (uint16 colour)    {PICTURE_ATTR(   colour)}
\def uint16    contrast   ()                 {PICTURE_ATTRGET(contrast)}
\def void   _0_contrast   (uint16 contrast)  {PICTURE_ATTR(   contrast)}
\def uint16    whiteness  ()                 {PICTURE_ATTRGET(whiteness)}
\def void   _0_whiteness  (uint16 whiteness) {PICTURE_ATTR(   whiteness)}
\def int frequency  () {
	int value;
	WIOCTL(fd, VIDIOCGFREQ, &value);
	return value;
}
\def void _0_frequency (int frequency) {
	if (0> IOCTL(fd, VIDIOCSFREQ, &frequency)) RAISE("can't set frequency to %d",frequency);
}

\def void close () {
	if (image) rb_funcall(rself,SI(dealloc_image),0);
	rb_call_super(argc,argv);
}

\def void _0_colorspace (Symbol c) {
	if (c==SYM(RGB24)) palette=VIDEO_PALETTE_RGB24;
	else if (c==SYM(YUV420P)) palette=VIDEO_PALETTE_YUV420P;
	else RAISE("supported: RGB24, YUV420P");
	WIOCTL(fd, VIDIOCGPICT, &vp);
	vp.palette = palette;
	WIOCTL(fd, VIDIOCSPICT, &vp);
	WIOCTL(fd, VIDIOCGPICT, &vp);
	switch(palette) {
	case VIDEO_PALETTE_RGB24:{
		uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
		bit_packing = new BitPacking(is_le(),3,3,masks);
	}break;
	case VIDEO_PALETTE_YUV420P:{
		// woops, special case already, can't do that with bit_packing
	}
	default: RAISE("can't handle palette %d", vp.palette);
	}
}

\def void initialize2 () {
//	long flags;          fcntl(fd,F_GETFL,&flags);
//	flags |= O_NONBLOCK; fcntl(fd,F_SETFL,&flags);
	WIOCTL(fd, VIDIOCGCAP, &vcaps);
	gfpost(&vcaps);
	rb_funcall(rself,SI(_0_size),2,INT2NUM(vcaps.maxheight),INT2NUM(vcaps.maxwidth));
	WIOCTL(fd, VIDIOCGPICT,&vp);
	gfpost(&vp);
	char buf[1024] = "";
	int n = 17 /*COUNT(video_palette_choice)*/;
	for (int i=0; i<n; i++) {
		vp.palette = i;
		ioctl(fd, VIDIOCSPICT,&vp);
		ioctl(fd, VIDIOCGPICT,&vp);
		if (vp.palette == i) {
			if (*buf) strcpy(buf+strlen(buf),", ");
			//strcpy(buf+strlen(buf),video_palette_choice[i].name);
			sprintf(buf+strlen(buf),"%d",i);
		}
	}
	gfpost("This card supports palettes: %s", buf);
	_0_colorspace(0,0,SYM(RGB24));
	rb_funcall(rself,SI(_0_channel),1,INT2NUM(0));
}

\def void initialize (Symbol mode, String filename, Symbol option=Qnil) {
	rb_call_super(argc,argv);
	image=0;
	rb_ivar_set(rself,SI(@stream),
		rb_funcall(rb_cFile,SI(open),2,filename,rb_str_new2("r+")));
	fd = GETFD;
	rb_funcall(rself,SI(initialize2),0);
}

\classinfo {
	IEVAL(rself,ruby_code);
	IEVAL(rself,"install '#io:videodev',1,2;@flags=4;@comment='Video4linux 1.x'");
}
\end class FormatVideoDev
void startup_videodev () {
	\startall
}
