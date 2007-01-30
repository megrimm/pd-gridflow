/*
	$Id$

	GridFlow
	Copyright (c) 2001-2007 by Mathieu Bouchard

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

/* bt878 on matju's comp supports only palette 4 */
/* bt878 on heri's comp supports palettes 3, 6, 7, 8, 9, 13 */
/* pwc supports palettes 12 and 15 */

#include "../base/grid.h.fcs"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "pwc-ioctl.h"

#define error gfpost

/* **************************************************************** */

typedef video_capability VideoCapability;
typedef video_channel    VideoChannel   ;
typedef video_tuner      VideoTuner     ;
typedef video_window     VideoWindow    ;
typedef video_picture    VideoPicture   ;
typedef video_mbuf       VideoMbuf      ;
typedef video_mmap       VideoMmap      ;

#define FLAG(_num_,_name_,_desc_) #_name_,
#define  OPT(_num_,_name_,_desc_) #_name_,

static const char *video_type_flags[] = {
	FLAG( 0,CAPTURE,       "Can capture")
	FLAG( 1,TUNER,         "Can tune")
	FLAG( 2,TELETEXT,      "Does teletext")
	FLAG( 3,OVERLAY,       "Overlay onto frame buffer")
	FLAG( 4,CHROMAKEY,     "Overlay by chromakey")
	FLAG( 5,CLIPPING,      "Can clip")
	FLAG( 6,FRAMERAM,      "Uses the frame buffer memory")
	FLAG( 7,SCALES,        "Scalable")
	FLAG( 8,MONOCHROME,    "Monochrome only")
	FLAG( 9,SUBCAPTURE,    "Can capture subareas of the image")
	FLAG(10,MPEG_DECODER,  "Can decode MPEG streams")
	FLAG(11,MPEG_ENCODER,  "Can encode MPEG streams")
	FLAG(12,MJPEG_DECODER, "Can decode MJPEG streams")
	FLAG(13,MJPEG_ENCODER, "Can encode MJPEG streams")
};

static const char *tuner_flags[] = {
	FLAG(0,PAL,      "")
	FLAG(1,NTSC,     "")
	FLAG(2,SECAM,    "")
	FLAG(3,LOW,      "Uses KHz not MHz")
	FLAG(4,NORM,     "Tuner can set norm")
	FLAG(5,DUMMY5,   "")
	FLAG(6,DUMMY6,   "")
	FLAG(7,STEREO_ON,"Tuner is seeing stereo")
	FLAG(8,RDS_ON,   "Tuner is seeing an RDS datastream")
	FLAG(9,MBS_ON,   "Tuner is seeing an MBS datastream")
};

static const char *channel_flags[] = {
	FLAG(0,TUNER,"")
	FLAG(1,AUDIO,"")
	FLAG(2,NORM ,"")
};

static const char *video_palette_choice[] = {
	OPT( 0,NIL,     "(nil)")
	OPT( 1,GREY,    "Linear greyscale")
	OPT( 2,HI240,   "High 240 cube (BT848)")
	OPT( 3,RGB565,  "565 16 bit RGB")
	OPT( 4,RGB24,   "24bit RGB")
	OPT( 5,RGB32,   "32bit RGB")
	OPT( 6,RGB555,  "555 15bit RGB")
	OPT( 7,YUV422,  "YUV422 capture")
	OPT( 8,YUYV,    "")
	OPT( 9,UYVY,    "The great thing about standards is ...")
	OPT(10,YUV420,  "")
	OPT(11,YUV411,  "YUV411 capture")
	OPT(12,RAW,     "RAW capture (BT848)")
	OPT(13,YUV422P, "YUV 4:2:2 Planar")
	OPT(14,YUV411P, "YUV 4:1:1 Planar")
	OPT(15,YUV420P, "YUV 4:2:0 Planar")
	OPT(16,YUV410P, "YUV 4:1:0 Planar")
};

static const char *video_mode_choice[] = {
	OPT( 0,PAL,  "pal")
	OPT( 1,NTSC, "ntsc")
	OPT( 2,SECAM,"secam")
	OPT( 3,AUTO, "auto")
};

#define WH(_field_,_spec_) \
	sprintf(buf+strlen(buf), "%s: " _spec_ "; ", #_field_, self->_field_);
#define WHYX(_name_,_fieldy_,_fieldx_) \
	sprintf(buf+strlen(buf), "%s: y=%d, x=%d; ", #_name_, self->_fieldy_, self->_fieldx_);
#define WHFLAGS(_field_,_table_) { \
	char *foo; \
	sprintf(buf+strlen(buf), "%s: %s; ", #_field_, \
		foo=flags_to_s(self->_field_,COUNT(_table_),_table_)); \
	delete[] foo;}
#define WHCHOICE(_field_,_table_) { \
	char *foo; \
	sprintf(buf+strlen(buf), "%s: %s; ", #_field_, \
		foo=choice_to_s(self->_field_,COUNT(_table_),_table_));\
	delete[] foo;}

static char *flags_to_s(int value, int n, const char **table) {
	char foo[256];
	*foo = 0;
	for(int i=0; i<n; i++) {
		if ((value & (1<<i)) == 0) continue;
		if (*foo) strcat(foo," | ");
		strcat(foo,table[i]);
	}
	if (!*foo) strcat(foo,"0");
	return strdup(foo);
}

static char *choice_to_s(int value, int n, const char **table) {
	if (value < 0 || value >= n) {
		char foo[64];
		sprintf(foo,"(Unknown #%d)",value);
		return strdup(foo);
	} else {
		return strdup(table[value]);
	}
}

static void gfpost(VideoChannel *self) {
	char buf[256] = "[VideoChannel] ";
	WH(channel,"%d");
	WH(name,"%.32s");
	WH(tuners,"%d");
	WHFLAGS(flags,channel_flags);
	WH(type,"0x%04x");
	WH(norm,"%d");
	gfpost("%s",buf);
}

static void gfpost(VideoTuner *self) {
	char buf[256] = "[VideoTuner] ";
	WH(tuner,"%d");
	WH(name,"%.32s");
	WH(rangelow,"%lu");
	WH(rangehigh,"%lu");
	WHFLAGS(flags,tuner_flags);
	WHCHOICE(mode,video_mode_choice);
	WH(signal,"%d");
	gfpost("%s",buf);
}

static void gfpost(VideoCapability *self) {
	char buf[256] = "[VideoCapability] ";
	WH(name,"%.20s");
	WHFLAGS(type,video_type_flags);
	WH(channels,"%d");
	WH(audios,"%d");
	WHYX(maxsize,maxheight,maxwidth);
	WHYX(minsize,minheight,minwidth);
	gfpost("%s",buf);
}

static void gfpost(VideoWindow *self) {
	char buf[256] = "[VideoWindow] ";
	WHYX(pos,y,x);
	WHYX(size,height,width);
	WH(chromakey,"0x%08x");
	WH(flags,"0x%08x");
	WH(clipcount,"%d");
	gfpost("%s",buf);
}

static void gfpost(VideoPicture *self) {
	char buf[256] = "[VideoPicture] ";
	WH(brightness,"%d");
	WH(hue,"%d");
	WH(contrast,"%d");
	WH(whiteness,"%d");
	WH(depth,"%d");
	WHCHOICE(palette,video_palette_choice);
	gfpost("%s",buf);
}

static void gfpost(VideoMbuf *self) {
	char buf[256] = "[VideoMBuf] ";
	WH(size,"%d");
	WH(frames,"%d");
	for (int i=0; i<4; i++) WH(offsets[i],"%d");
	gfpost("%s",buf);
}

static void gfpost(VideoMmap *self) {
	char buf[256] = "[VideoMMap] ";
	WH(frame,"%u");
	WHYX(size,height,width);
	WHCHOICE(format,video_palette_choice);
	gfpost("%s",buf);
};

/* **************************************************************** */

\class FormatVideoDev < Format
struct FormatVideoDev : Format {
	VideoCapability vcaps;
	VideoPicture vp;
	VideoMbuf vmbuf;
	VideoMmap vmmap;
	uint8 *image;
	int queue[8], queuesize, queuemax, next_frame;
	int current_channel, current_tuner;
	bool use_mmap;
	P<BitPacking> bit_packing;
	P<Dim> dim;
	int fd;
	Symbol colorspace;
	int palettes; /* bitfield */

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
	\attr uint16 framerate();
	\attr uint16 white_mode(); /* 0..1 */
	\attr uint16 white_red();
	\attr uint16 white_blue();
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
	vmmap.format = vp.palette;
	vmmap.width  = dim->get(1);
	vmmap.height = dim->get(0);
	WIOCTL2(fd, VIDIOCMCAPTURE, &vmmap);
	next_frame = (next_frame+1) % vmbuf.frames;
}

static uint8 clip(int x) {return x<0?0 : x>255?255 : x;}

void FormatVideoDev::frame_finished (uint8 *buf) {
	/* picture is converted here. */
	int sy = dim->get(0);
	int sx = dim->get(1);
	int bs = dim->prod(1);
	uint8 b2[bs];
	if (vp.palette==VIDEO_PALETTE_YUV420P) {
		GridOutlet out(this,0,dim,NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
		if (colorspace==SYM(y)) {
			out.send(sy*sx,buf);
		} else if (colorspace==SYM(rgb)) {
			for(int y=0; y<sy; y++) {
				uint8 *bufy = buf+sx* y;
				uint8 *bufu = buf+sx*sy    +(sx/2)*(y/2);
				uint8 *bufv = buf+sx*sy*5/4+(sx/2)*(y/2);
				int Y1,Y2,U,V;
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					Y1=bufy[x]   - 16;
					Y2=bufy[x+1] - 16;
					U=bufu[x/2] - 128;
					V=bufv[x/2] - 128;
					b2[xx+0]=clip((298*Y1         + 409*V)>>8);
					b2[xx+1]=clip((298*Y1 - 100*U - 208*V)>>8);
					b2[xx+2]=clip((298*Y1 + 516*U        )>>8);
					b2[xx+3]=clip((298*Y2         + 409*V)>>8);
					b2[xx+4]=clip((298*Y2 - 100*U - 208*V)>>8);
					b2[xx+5]=clip((298*Y2 + 516*U        )>>8);
				}
				out.send(bs,b2);
			}
		} else if (colorspace==SYM(yuv)) {
			for(int y=0; y<sy; y++) {
				uint8 *bufy = buf+sx* y;
				uint8 *bufu = buf+sx*sy    +(sx/2)*(y/2);
				uint8 *bufv = buf+sx*sy*5/4+(sx/2)*(y/2);
				int U,V;
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					U=bufu[x/2];
					V=bufv[x/2];
					//b2[xx+0]=bufy[x+0]; b2[xx+1]=U; b2[xx+2]=V;
					//b2[xx+3]=bufy[x+1]; b2[xx+4]=U; b2[xx+5]=V;
					b2[xx+0]=clip(((bufy[x+0]-16)*298)>>8);
					b2[xx+1]=clip(((U-128)*293)>>8);
					b2[xx+2]=clip(((V-128)*293)>>8);
				}
				out.send(bs,b2);
			}
		}
	} else if (vp.palette==VIDEO_PALETTE_RGB24) {
		GridOutlet out(this,0,dim,NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
		if (colorspace==SYM(y)) {
			for(int y=0; y<sy; y++) {
				uint8 *rgb = buf+sx*y*3;
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					b2[x+0] = (76*rgb[xx+0]+150*rgb[xx+1]+29*rgb[xx+2])>>8;
					b2[x+1] = (76*rgb[xx+3]+150*rgb[xx+4]+29*rgb[xx+5])>>8;
				}
				out.send(bs,b2);
			}
		} else if (colorspace==SYM(rgb)) {
			uint8 b2[bs];
//			uint64 t = gf_timeofday();
			for(int y=0; y<sy; y++) {
				uint8 *buf2 = buf+bit_packing->bytes*sx*y;
				bit_packing->unpack(sx,buf2,b2);
				out.send(bs,b2);
			}
//			t=gf_timeofday()-t;
//			fprintf(stderr,"decoding frame took %lld us\n",t);
		} else if (colorspace==SYM(yuv)) {
			/* unimplemented??? */
			// Y =   76*R + 150*G +  29*B
			// U = - 44*R -  85*G + 108*B
			// V =  128*R - 108*G -  21*B
		}
	} else {
		RAISE("unsupported palette %d",vp.palette);
	}
}

/* these are factors for RGB to analog YUV */
// Y =   66*R + 129*G +  25*B
// U = - 38*R -  74*G + 112*B
// V =  112*R -  94*G -  18*B

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

#define warn(fmt,stuff...) gfpost("warning: " fmt,stuff)

\def void _0_channel (int value) {
	VideoChannel vchan;
	vchan.channel = value;
	current_channel = value;
	if (0> IOCTL(fd, VIDIOCGCHAN, &vchan)) warn("no channel #%d", value);
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
	/*gfpost("getting %s=%d",#_name_,vp._name_);*/ \
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

\def void _0_colorspace (Symbol c) { /* y yuv rgb */
	if      (c==SYM(y)) {}
	else if (c==SYM(yuv)) {}
	else if (c==SYM(rgb)) {}
	else RAISE("supported: y yuv rgb");
	WIOCTL(fd, VIDIOCGPICT, &vp);
	int palette = (palettes&(1<<VIDEO_PALETTE_RGB24)) ? VIDEO_PALETTE_RGB24 : VIDEO_PALETTE_YUV420P;
	vp.palette = palette;
	WIOCTL(fd, VIDIOCSPICT, &vp);
	WIOCTL(fd, VIDIOCGPICT, &vp);
	if (vp.palette != palette) {
		gfpost("this driver is unsupported: it wants palette %d instead of %d",vp.palette,palette);
		return;
	}
	if (c==SYM(yuv) && palette==VIDEO_PALETTE_RGB24) {
		gfpost("sorry, this conversion is currently unsupported");
		return;
	}
	uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff }; /* for use by RGB mode */
	bit_packing = new BitPacking(is_le(),3,3,masks);
	colorspace=c;
	dim = new Dim(dim->v[0],dim->v[1],c==SYM(y)?1:3);
}

void set_pan_and_tilt(int fd, char what, int pan, int tilt) {
	struct pwc_mpt_angles pma;
	pma.absolute=1;
	WIOCTL(fd, VIDIOCPWCMPTGANGLE, &pma);
	pma.pan = pan;
	pma.tilt = tilt;
	WIOCTL(fd, VIDIOCPWCMPTSANGLE, &pma);
}
\def uint16 framerate() {
	struct video_window vwin;
	WIOCTL(fd, VIDIOCGWIN, &vwin);
	return (vwin.flags & PWC_FPS_MASK) >> PWC_FPS_SHIFT;
}

\def void _0_framerate(uint16 framerate) {
	struct video_window vwin;
	WIOCTL(fd, VIDIOCGWIN, &vwin);
	vwin.flags &= ~PWC_FPS_FRMASK;
	vwin.flags |= (framerate << PWC_FPS_SHIFT) & PWC_FPS_FRMASK;
	WIOCTL(fd, VIDIOCSWIN, &vwin);
}

/* those functions are still mostly unused */
void set_compression_preference(int fd, int pref) {WIOCTL(fd, VIDIOCPWCSCQUAL, &pref);}
void set_automatic_gain_control(int fd, int pref) {WIOCTL(fd, VIDIOCPWCSAGC, &pref);}
void set_shutter_speed(int fd, int pref) {WIOCTL(fd, VIDIOCPWCSSHUTTER, &pref);}
\def uint16 white_mode () {
	struct pwc_whitebalance pwcwb;
	WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	if (pwcwb.mode==PWC_WB_AUTO)   return 0;
	if (pwcwb.mode==PWC_WB_MANUAL) return 1;
	return 2;
}

\def void _0_white_mode (uint16 white_mode) {
	struct pwc_whitebalance pwcwb;
	WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	if      (white_mode==0) pwcwb.mode = PWC_WB_AUTO;
	else if (white_mode==1) pwcwb.mode = PWC_WB_MANUAL;
	/*else if (strcasecmp(mode, "indoor") == 0)  pwcwb.mode = PWC_WB_INDOOR;*/
	/*else if (strcasecmp(mode, "outdoor") == 0) pwcwb.mode = PWC_WB_OUTDOOR;*/
	/*else if (strcasecmp(mode, "fl") == 0)      pwcwb.mode = PWC_WB_FL;*/
	else {error("unknown mode '%s'", white_mode); return;}
	WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}

\def uint16 white_red() {
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb); return pwcwb.manual_red;}
\def uint16 white_blue() {
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb); return pwcwb.manual_blue;}
\def void _0_white_red(uint16 white_red) {
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	pwcwb.manual_red = white_red;  WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}
\def void _0_white_blue(uint16 white_blue) {
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	pwcwb.manual_blue = white_blue;WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}

void set_automatic_white_balance_speed(int fd, int val) {
	struct pwc_wb_speed pwcwbs; WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs);
	pwcwbs.control_speed = val; WIOCTL(fd, VIDIOCPWCSAWBSPEED, &pwcwbs);}
void set_automatic_white_balance_delay(int fd, int val) {
	struct pwc_wb_speed pwcwbs; WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs);
	pwcwbs.control_delay = val; WIOCTL(fd, VIDIOCPWCSAWBSPEED, &pwcwbs);}
void set_led_on_time(int fd, int val) {
	struct pwc_leds pwcl; WIOCTL(fd, VIDIOCPWCGLED, &pwcl);
	pwcl.led_on = val;    WIOCTL(fd, VIDIOCPWCSLED, &pwcl);}
void set_led_off_time(int fd, int val) {
	struct pwc_leds pwcl; WIOCTL(fd, VIDIOCPWCGLED, &pwcl);
	pwcl.led_off = val;   WIOCTL(fd, VIDIOCPWCSLED, &pwcl);}
void set_sharpness(int fd, int val) {WIOCTL(fd, VIDIOCPWCSCONTOUR, &val);}
void set_backlight_compensation(int fd, int val) {WIOCTL(fd, VIDIOCPWCSBACKLIGHT, &val);}
void set_antiflicker_mode(int fd, int val) {WIOCTL(fd, VIDIOCPWCSFLICKER, &val);}
void set_noise_reduction(int fd, int val) {WIOCTL(fd, VIDIOCPWCSDYNNOISE, &val);}

\def void initialize2 () {
	WIOCTL(fd, VIDIOCGCAP, &vcaps);
	gfpost(&vcaps);
	rb_funcall(rself,SI(_0_size),2,INT2NUM(vcaps.maxheight),INT2NUM(vcaps.maxwidth));
	WIOCTL(fd, VIDIOCGPICT,&vp);
	gfpost(&vp);
	palettes=0;
	int checklist[] = {VIDEO_PALETTE_RGB24,VIDEO_PALETTE_YUV420P};
#if 0
	for (size_t i=0; i<sizeof(checklist)/sizeof(*checklist); i++) {
		int p = checklist[i];
#else
	for (size_t p=0; p<17; p++) {
#endif
		vp.palette = p;
		ioctl(fd, VIDIOCSPICT,&vp);
		ioctl(fd, VIDIOCGPICT,&vp);
		if (vp.palette == p) {
			palettes |= 1<<p;
			gfpost("palette %d supported",p);
		}
	}
	_0_colorspace(0,0,SYM(rgb));
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
	IEVAL(rself,"install '#io:videodev',1,2;@flags=4;@comment='Video4linux 1.x'");
}
\end class FormatVideoDev
void startup_videodev () {
	\startall
}
