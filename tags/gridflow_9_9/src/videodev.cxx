/*
	$Id: videodev.c 4620 2009-11-01 21:16:58Z matju $

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

/* bt878 on matju's comp supports only palette 4 */
/* bt878 on heri's comp supports palettes 3, 6, 7, 8, 9, 13 */
/* pwc supports palettes 12 and 15 on most computers, but...
 * apparently not all of them. but I can't tell which. it remains a mystery. */

#include "gridflow.hxx.fcs"
#include "colorspace.hxx"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "pwc-ioctl.h"
#include <sstream>

#ifdef HAVE_LIBV4L1
#include <libv4l1.h>
//#define open   v4l1_open
#define close  v4l1_close
#define ioctl  v4l1_ioctl
#define mmap   v4l1_mmap
#define munmap v4l1_munmap
#define read   v4l1_read
#warning Using libv4l1 !!!
#else
#warning NOT Using libv4l1 !!!
#define v4l1_open(a,b) (RAISE("this [#io.videodev] wasn't compiled with libv4l1 support"),-1)
#endif

//#define error post
static bool debug=0;

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
#define WH(_field_,_spec_)             oprintf(buf, "%s=" _spec_ " ", #_field_, self->_field_);
#define WHYX(_name_,_fieldy_,_fieldx_) oprintf(buf, "%s=(%d %d) ", #_name_, self->_fieldy_, self->_fieldx_);
#define WHFLAGS(_field_,_table_)       oprintf(buf, "%s:%s " ,#_field_,flags_to_s( self->_field_,COUNT(_table_),_table_).data());
#define WHCHOICE(_field_,_table_)      oprintf(buf, "%s=%s; ",#_field_,choice_to_s(self->_field_,COUNT(_table_),_table_).data());
#if 0
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
#endif
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
static string flags_to_s(int value, int n, const char **table) {
	char foo[256];
	*foo = 0;
	for(int i=0; i<n; i++) {
		if ((value & (1<<i)) == 0) continue;
		if (*foo) strcat(foo," | ");
		strcat(foo,table[i]);
	}
	if (!*foo) strcat(foo,"0");
	return string(foo);
}
static string choice_to_s(int value, int n, const char **table) {
	if (value < 0 || value >= n) {
		char foo[64];
		sprintf(foo,"(Unknown #%d)",value);
		return string(foo);
	} else {
		return string(table[value]);
	}
}
void post(std::ostringstream &o) {string os = o.str(); post("%s",os.data());}
static void gfpost(VideoChannel *self) {std::ostringstream buf; buf << "[VideoChannel] ";
	WH(channel,"%d");
	WH(name,"\"%.32s\"");
	WH(tuners,"%d");
	WHFLAGS(flags,channel_flags);
	WH(type,"0x%04x");
	WH(norm,"%d");
	post(buf);}
static void gfpost(VideoTuner *self) {std::ostringstream buf; buf << "[VideoTuner] ";
	WH(tuner,"%d");
	WH(name,"\"%.32s\"");
	WH(rangelow,"%lu");
	WH(rangehigh,"%lu");
	WHFLAGS(flags,tuner_flags);
	WHCHOICE(mode,video_mode_choice);
	WH(signal,"%d");
	post(buf);}
static void gfpost(VideoWindow *self) {std::ostringstream buf; buf << "[VideoWindow] ";
	WHYX(pos,y,x);
	WHYX(size,height,width);
	WH(chromakey,"0x%08x");
	WH(flags,"0x%08x");
	WH(clipcount,"%d");
	post(buf);}
static void gfpost(VideoMbuf *self) {std::ostringstream buf; buf << "[VideoMBuf] ";
	WH(size,"%d");
	WH(frames,"%d");
	oprintf(buf,"offsets=[");
	for (int i=0; i<self->frames; i++) oprintf(buf,"%d%s",self->offsets[i],i+1==self->frames?"]":", ");
	post(buf);}
static void gfpost(VideoMmap *self) {std::ostringstream buf; buf << "[VideoMMap] ";
	WH(frame,"%u");
	WHYX(size,height,width);
	WHCHOICE(format,video_palette_choice);
	post(buf);}

t_symbol *safe_gensym(const char *name) {
	char buf[33];
	memcpy(buf,name,32);
	int i;
	for (i=31; buf[i] && !isspace(buf[i]); i--) buf[i]=0;
	for (i=0; buf[i]; i++) {
		if (isspace(buf[i])) buf[i]='_';
		if (buf[i]=='(') buf[i]='[';
		if (buf[i]==')') buf[i]=']';
	}
	return gensym(buf);
}

#define DEBUG(args...) 42
//#define DEBUG(args...) post(args)
#define  IOCTL( F,NAME,ARG) (DEBUG("fd%d.ioctl(0x%08x,0x%08x)",F,NAME,ARG), ioctl(F,NAME,ARG))
#define WIOCTL( F,NAME,ARG) (IOCTL(F,NAME,ARG)<0 && (error("%s: ioctl %s: %s",__FUNCTION__,#NAME,strerror(errno)),1))
#define WIOCTL2(F,NAME,ARG) (IOCTL(F,NAME,ARG)<0 && (RAISE("%s: ioctl %s: %s",__FUNCTION__,#NAME,strerror(errno)), RAISE("ioctl error"), 0))

/* **************************************************************** */

\class FormatVideoDev : Format {
	VideoCapability vcaps;
	VideoPicture vp;
	VideoMbuf vmbuf;
	VideoMmap vmmap;
	uint8 *image;
	int queue[8], queuesize, queuemax, next_frame;
	int current_channel, current_tuner;
	bool use_pwc;
	P<BitPacking> bit_packing3, bit_packing4;
	P<Dim> dim;
	bool has_frequency, has_tuner, has_norm;
	bool use_libv4l;
	int fd;
	int palettes; /* bitfield */

	\constructor (string mode, string filename, bool libv4l=false) {
		queuesize=0; queuemax=2; next_frame=0; use_pwc=false;
		colorspace=gensym("none"); /* non-existent colorspace just to prevent crash in case of other problems */
		has_frequency=false;
		has_tuner=false;
		has_norm=false;
		image=0;
		if (libv4l) fd = v4l1_open(filename.data(),O_RDWR);
		else 	    fd =      open(filename.data(),O_RDWR);
		if (fd<0) RAISE("can't open device '%s': %s",filename.data(),strerror(errno));
		WIOCTL(fd, VIDIOCGCAP, &vcaps);
		dim = new Dim(0,0,0);
		name = safe_gensym(vcaps.name);
		WIOCTL(fd, VIDIOCGPICT,&vp);
		detect_palettes();
		_0_size(0,0,vcaps.maxheight,vcaps.maxwidth);
		_0_colorspace(0,0,gensym("rgb"));
		_0_channel(0,0,0);
	}
	void frame_finished (uint8 *buf);

	void alloc_image ();
	void dealloc_image ();
	void frame_ask ();
	~FormatVideoDev () {
		if (image) dealloc_image();
		close(fd); // wtf
	}

	\decl 0 bang ();
	\grin 0 int

	\attr int channel();
	\attr int tuner();
	\attr int norm();
	\decl 0 size (int sy, int sx);

	\attr t_symbol *colorspace;
	\attr int32  frequency();
	\attr uint16 brightness();
	\attr uint16 hue();
	\attr uint16 colour();
	\attr uint16 contrast();
	\attr uint16 whiteness();

	\attr bool   pwc(); /* 0..1 */
	\attr uint16 framerate();
	\attr uint16 white_mode(); /* 0..1 */
	\attr uint16 white_red();
	\attr uint16 white_blue();
	\attr uint16 white_speed();
	\attr uint16 white_delay();
	\attr int    auto_gain();
	\attr int    noise_reduction(); /* 0..3 */
	\attr int    compression();     /* 0..3 */
	\attr t_symbol *name;

	\decl 0 get (t_symbol *s=0);
	
	void detect_palettes () {
		palettes=0;
		std::ostringstream supp;
		supp << "camera supports palettes :";
#if 1 /* keep this at "1" most of the time, because at "0" it crashes certain camera drivers ! */
		int checklist[] = {VIDEO_PALETTE_RGB565,VIDEO_PALETTE_RGB24,VIDEO_PALETTE_RGB32,VIDEO_PALETTE_YUYV,VIDEO_PALETTE_YUV420P};
		for (size_t i=0; i<sizeof(checklist)/sizeof(*checklist); i++) {
			int p = checklist[i];
#else
		for (size_t p=1; p<17; p++) {
#endif
			vp.palette = p;
			if (ioctl(fd, VIDIOCSPICT,&vp)>=0) {palettes |= 1<<p; supp << " " << p;}
		}
		post(supp);
	}
};

\def 0 get (t_symbol *s=0) {
	// this is abnormal for a get-function
	if (s==gensym("frequency") && !has_frequency  ) return;
	if (s==gensym("tuner")     && !has_tuner      ) return;
	if (s==gensym("norm")      && !has_norm       ) return;
	if (s==gensym("channel")   && vcaps.channels<2) return;
	if (!use_pwc && (s==gensym("white_mode")      || s==gensym("white_red")   || s==gensym("white_blue") ||
			 s==gensym("white_speed")     || s==gensym("white_delay") || s==gensym("auto_gain")  ||
			 s==gensym("noise_reduction") || s==gensym("compression") || s==gensym("framerate"))) return;
	FObject::_0_get(argc,argv,s);
	// size are abnormal attributes (does not use nested list)
	if (!s) {
		t_atom a[2];
		SETFLOAT(a+0,vcaps.minheight); SETFLOAT(a+1,vcaps.minwidth); outlet_anything(outlets[0],gensym("minsize"),2,a);
		SETFLOAT(a+0,vcaps.maxheight); SETFLOAT(a+1,vcaps.maxwidth); outlet_anything(outlets[0],gensym("maxsize"),2,a);
		string foo = choice_to_s(vp.palette,COUNT(video_palette_choice),video_palette_choice);
		SETSYMBOL(a,gensym(foo.data()));
		outlet_anything(outlets[0],gensym("palette"),1,a);
		SETFLOAT(a+0,dim->v[0]); SETFLOAT(a+1,dim->v[1]); outlet_anything(outlets[0],gensym("size"),    2,a);
	}
}

\def 0 size (int sy, int sx) {
	VideoWindow grab_win;
	// !@#$ bug here: won't flush the frame queue
	dim = new Dim(sy,sx,dim->v[2]);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	if (debug) gfpost(&grab_win);
	grab_win.clipcount = 0;
	grab_win.flags = 0;
	if (sy && sx) {
		grab_win.height = sy;
		grab_win.width  = sx;
	}
	if (debug) gfpost(&grab_win);
	WIOCTL(fd, VIDIOCSWIN, &grab_win);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	if (debug) gfpost(&grab_win);
}

void FormatVideoDev::dealloc_image () {if (image) {munmap(image,vmbuf.size); image=0;}}
void FormatVideoDev::alloc_image () {
	WIOCTL2(fd, VIDIOCGMBUF, &vmbuf);
	image = (uint8 *)mmap(0,vmbuf.size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if (((long)image)==-1) {image=0; RAISE("mmap: %s", strerror(errno));}
}

void FormatVideoDev::frame_ask () {
	if (queuesize>=queuemax) RAISE("queue is full (queuemax=%d)",queuemax);
	if (queuesize>=vmbuf.frames) RAISE("queue is full (vmbuf.frames=%d)",vmbuf.frames);
	vmmap.frame = queue[queuesize++] = next_frame;
	vmmap.format = vp.palette;
	vmmap.width  = dim->get(1);
	vmmap.height = dim->get(0);
	WIOCTL2(fd, VIDIOCMCAPTURE, &vmmap);
	//gfpost(&vmmap);
	next_frame = (next_frame+1) % vmbuf.frames;
}

void FormatVideoDev::frame_finished (uint8 *buf) {
	string cs = colorspace->s_name;
	int downscale = cs=="magic";
	/* picture is converted here. */
	int sy = dim->get(0);
	int sx = dim->get(1);
	int bs = dim->prod(1); if (downscale) bs/=2;
	uint8 b2[bs];
	//post("frame_finished sy=%d sx=%d bs=%d, vp.palette = %d; colorspace = %s",sy,sx,bs,vp.palette,cs.data());
	GridOutlet out(this,0,cs=="magic"?new Dim(sy>>downscale,sx>>downscale,3):(Dim *)dim,cast);
	if (vp.palette==VIDEO_PALETTE_YUYV) {
		uint8 *bufy = buf;
		if (cs=="y") {
		    for(int y=0; y<sy; bufy+=sx, y++) {
			for (int x=0,xx=0; x<sx; x+=2,xx+=4) {
				b2[xx+0]=YUV2Y(bufy[x+0],0,0);
				b2[xx+1]=YUV2Y(bufy[x+2],0,0);
			}
			out.send(bs,b2);
		    }
		} else if (cs=="rgb") {
		    for(int y=0; y<sy; bufy+=2*sx, y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx*2; x+=4,xx+=6) {GETYUYV(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+3,Y2,U,V);}
			out.send(bs,b2);}
		} else if (cs=="rgba") {
		    for(int y=0; y<sy; bufy+=2*sx, y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx*2; x+=4,xx+=8) {GETYUYV(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+4,Y2,U,V);
				b2[xx+3]=255; b2[xx+7]=255;}
			out.send(bs,b2);}
		} else if (cs=="yuv") {
		    for(int y=0; y<sy; bufy+=2*sx, y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx*2; x+=4,xx+=6) {GETYUYV(x); YUV2YUV(b2+xx,Y1,U,V); YUV2YUV(b2+xx+3,Y2,U,V);}
			out.send(bs,b2);}
		} else if (cs=="magic") {
		    int sx2 = sx/2;
		    for(int y=0; y<sy; bufy+=2*sx, y+=2) {
 			for (int x=0,xx=0; x<sx2; x+=3,xx+=3) {b2[xx+0]=bufy[x+0]; b2[xx+1]=bufy[x+1]; b2[xx+2]=bufy[x+3];}
			out.send(bs,b2);
		    }
		}
	} else if (vp.palette==VIDEO_PALETTE_YUV420P) {
		uint8 *bufy = buf, *bufu = buf+sx*sy, *bufv = buf+sx*sy*5/4;
		if (cs=="y") {
		    out.send(sy*sx,buf);
		} else if (cs=="rgb") {
		    for(int y=0; y<sy; bufy+=sx, bufu+=(sx/2)*(y&1), bufv+=(sx/2)*(y&1), y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx; x+=2,xx+=6) {GET420P(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+3,Y2,U,V);}
			out.send(bs,b2);}
		} else if (cs=="rgba") {
		    for(int y=0; y<sy; bufy+=sx, bufu+=(sx/2)*(y&1), bufv+=(sx/2)*(y&1), y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx; x+=2,xx+=8) {GET420P(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+4,Y2,U,V);
			b2[xx+3]=255; b2[xx+7]=255;}
			out.send(bs,b2);}
		} else if (cs=="yuv") {
		    for(int y=0; y<sy; bufy+=sx, bufu+=(sx/2)*(y&1), bufv+=(sx/2)*(y&1), y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx; x+=2,xx+=6) {GET420P(x); YUV2YUV(b2+xx,Y1,U,V); YUV2YUV(b2+xx+3,Y2,U,V);}
			out.send(bs,b2);}
		} else if (cs=="magic") {
		    int sx2 = sx/2;
		    for(int y=0; y<sy/2; bufy+=2*sx, bufu+=sx2, bufv+=sx2, y++) {
 			for (int x=0,xx=0; x<sx2; x++,xx+=3) {b2[xx+0]=bufy[x+x]; b2[xx+1]=bufu[x]; b2[xx+2]=bufv[x];}
			out.send(bs,b2);
		    }
		}
	} else if (vp.palette==VIDEO_PALETTE_RGB32 || vp.palette==VIDEO_PALETTE_RGB24 || vp.palette==VIDEO_PALETTE_RGB565) {
		uint8 rgb[sx*4];
		uint8 b2[sx*3];
		if (cs=="y") {
			for(int y=0; y<sy; y++) {
			        bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					// this doesn't clip!
					b2[x+0] = (76*rgb[xx+0]+150*rgb[xx+1]+29*rgb[xx+2])>>8;
					b2[x+1] = (76*rgb[xx+3]+150*rgb[xx+4]+29*rgb[xx+5])>>8;
				}
				out.send(bs,b2);
			}
		} else if (cs=="rgb") {
			for(int y=0; y<sy; y++) {
			        bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
			 	out.send(bs,rgb);
			}
		} else if (cs=="rgba") {
			for(int y=0; y<sy; y++) {
				uint8 *buf2 = buf+y*sx*bit_packing4->bytes;
			        bit_packing4->unpack(sx,buf2,rgb);
			        for (int x=3; x<bs; x+=4) rgb[x]=255;
			 	out.send(bs,rgb);
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
		} else if (cs=="magic") {
		    for(int y=0; y<sy; y+=2) {
			bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
 			for (int x=0,xx=0; x<bs; x+=3,xx+=6) {
				b2[x+0] = RGB2Y_(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
				b2[x+1] = RGB2U_(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
				b2[x+2] = RGB2V_(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
			}
			out.send(bs,b2);
		    }
		}
	} else {
		RAISE("unsupported palette %d",vp.palette);
	}
}

\def 0 bang () {
	if (!image) alloc_image();
	while(queuesize<queuemax) frame_ask();
	vmmap.frame = queue[0];
	//uint64 t0 = gf_timeofday();
	WIOCTL2(fd, VIDIOCSYNC, &vmmap);
	//uint64 t1 = gf_timeofday();
	//if (t1-t0 > 100) gfpost("VIDIOCSYNC delay: %d us",t1-t0);
	frame_finished(image+vmbuf.offsets[queue[0]]);
	queuesize--;
	for (int i=0; i<queuesize; i++) queue[i]=queue[i+1];
	frame_ask();
}

GRID_INLET(0) {RAISE("can't write.");} GRID_END

\def 0 norm (int value) {
	VideoTuner vtuner;
	vtuner.tuner = current_tuner;
	if (value<0 || value>3) RAISE("norm must be in range 0..3");
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) {
		post("no tuner #%d", value);
	} else {
		vtuner.mode = value; //gfpost(&vtuner);
		WIOCTL(fd, VIDIOCSTUNER, &vtuner);
	}
}

\def int norm () {
	VideoTuner vtuner;
	vtuner.tuner = current_tuner;
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) {post("no tuner #%d", current_tuner); return -1;}
	return vtuner.mode;
}

\def 0 tuner (int value) {
	VideoTuner vtuner;
	vtuner.tuner = current_tuner = value;
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) RAISE("no tuner #%d", value);
	vtuner.mode = VIDEO_MODE_NTSC; //??? //gfpost(&vtuner);
	WIOCTL(fd, VIDIOCSTUNER, &vtuner);
	has_norm = (vtuner.mode<=3);
	int meuh;
	has_frequency = (ioctl(fd, VIDIOCGFREQ, &meuh)>=0);
}
\def int tuner () {return current_tuner;}

#define warn(fmt,stuff...) post("warning: " fmt,stuff)

\def 0 channel (int value) {
	if (vcaps.channels<2) return; /* can't set this in this case */
	VideoChannel vchan;
	vchan.channel = value;
	current_channel = value;
	if (0> IOCTL(fd, VIDIOCGCHAN, &vchan)) warn("no channel #%d", value);
	//gfpost(&vchan);
	WIOCTL(fd, VIDIOCSCHAN, &vchan);
	if (vcaps.type & VID_TYPE_TUNER) _0_tuner(0,0,0);
	has_tuner = (vcaps.type & VID_TYPE_TUNER && vchan.tuners > 1);
}
\def int channel () {return current_channel;}

#define PICTURE_ATTR(_name_) {\
	WIOCTL(fd, VIDIOCGPICT, &vp); \
	vp._name_ = _name_; \
	WIOCTL(fd, VIDIOCSPICT, &vp);}

#define PICTURE_ATTRGET(_name_) { \
	WIOCTL(fd, VIDIOCGPICT, &vp); \
	/*gfpost("getting %s=%d",#_name_,vp._name_);*/ \
	return vp._name_;}

\def uint16 brightness ()                 {PICTURE_ATTRGET(brightness)}
\def 0      brightness (uint16 brightness){PICTURE_ATTR(   brightness)}
\def uint16 hue        ()                 {PICTURE_ATTRGET(hue)}
\def 0      hue        (uint16 hue)       {PICTURE_ATTR(   hue)}
\def uint16 colour     ()                 {PICTURE_ATTRGET(colour)}
\def 0      colour     (uint16 colour)    {PICTURE_ATTR(   colour)}
\def uint16 contrast   ()                 {PICTURE_ATTRGET(contrast)}
\def 0      contrast   (uint16 contrast)  {PICTURE_ATTR(   contrast)}
\def uint16 whiteness  ()                 {PICTURE_ATTRGET(whiteness)}
\def 0      whiteness  (uint16 whiteness) {PICTURE_ATTR(   whiteness)}
\def int32 frequency  () {
	int32 value;
	//if (ioctl(fd, VIDIOCGFREQ, &value)<0) {has_frequency=false; return 0;}
	WIOCTL(fd, VIDIOCGFREQ, &value);
	return value;
}
\def 0 frequency (int32 frequency) {
    long frequency_ = frequency;
	WIOCTL(fd, VIDIOCSFREQ, &frequency_);
}

\def 0 colorspace (t_symbol *colorspace) { /* y yuv rgb rgba magic */
	string c = colorspace->s_name;
	if (c=="y"    ) {} else
	if (c=="yuv"  ) {} else
	if (c=="rgb"  ) {} else
	if (c=="rgba" ) {} else
	if (c=="magic") {} else
	   RAISE("got '%s' but supported colorspaces are: y yuv rgb rgba magic",c.data());
	WIOCTL(fd, VIDIOCGPICT, &vp);
	int palette = (palettes&(1<<VIDEO_PALETTE_RGB24 )) ? VIDEO_PALETTE_RGB24  :
	              (palettes&(1<<VIDEO_PALETTE_RGB32 )) ? VIDEO_PALETTE_RGB32  :
	              (palettes&(1<<VIDEO_PALETTE_RGB565)) ? VIDEO_PALETTE_RGB565 :
	              (palettes&(1<<VIDEO_PALETTE_YUYV  )) ? VIDEO_PALETTE_YUYV :
                      VIDEO_PALETTE_YUV420P;
	vp.palette = palette;
	post("request use of palette %d",palette);
	WIOCTL(fd, VIDIOCSPICT, &vp);
	WIOCTL(fd, VIDIOCGPICT, &vp);
	if (vp.palette != palette) {
		post("this driver is unsupported: it wants palette %d instead of %d",vp.palette,palette);
		return;
	}
	#define RGB(R,G,B,bytes) do { \
		uint32 masks[4]={R,G,B,0}; \
		bit_packing3 = new BitPacking(is_le(),bytes,3,masks);\
		bit_packing4 = new BitPacking(is_le(),bytes,4,masks);} while(0)
        if (palette==VIDEO_PALETTE_RGB565) RGB(0x00f800,0x0007e0,0x00001f,2); else
	if (palette==VIDEO_PALETTE_RGB24 ) RGB(0xff0000,0x00ff00,0x0000ff,3); else
	if (palette==VIDEO_PALETTE_RGB32 ) RGB(0xff0000,0x00ff00,0x0000ff,4);
	this->colorspace=gensym(c.data());
	dim = new Dim(dim->v[0],dim->v[1],c=="y"?1:c=="rgba"?4:3);
}

\def bool pwc ()         {return use_pwc;}
\def 0    pwc (bool pwc) {use_pwc=pwc;}
#define PWC(R) if (!use_pwc) return R;

\def uint16 framerate() {PWC(0)
	struct video_window vwin; WIOCTL(fd, VIDIOCGWIN, &vwin);
	return (vwin.flags & PWC_FPS_MASK) >> PWC_FPS_SHIFT;
}
\def 0 framerate(uint16 framerate) {PWC()
	struct video_window vwin; WIOCTL(fd, VIDIOCGWIN, &vwin);
	vwin.flags &= ~PWC_FPS_FRMASK;
	vwin.flags |= (framerate << PWC_FPS_SHIFT) & PWC_FPS_FRMASK;
	WIOCTL(fd, VIDIOCSWIN, &vwin);
}

\def int auto_gain() {int auto_gain=0; if (use_pwc) WIOCTL(fd, VIDIOCPWCGAGC, &auto_gain); return auto_gain;}
\def 0   auto_gain   (int auto_gain)  {if (use_pwc) WIOCTL(fd, VIDIOCPWCSAGC, &auto_gain);}

\def uint16 white_mode () {PWC(0)
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	if (pwcwb.mode==PWC_WB_AUTO)   return 0;
	if (pwcwb.mode==PWC_WB_MANUAL) return 1;
	return 2;
}
\def 0 white_mode (uint16 white_mode) {PWC()
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	if      (white_mode==0) pwcwb.mode = PWC_WB_AUTO;
	else if (white_mode==1) pwcwb.mode = PWC_WB_MANUAL;
	else {error("unknown mode number %d", white_mode); return;}
	WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}

\def uint16 white_red()  {PWC(0) struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb); return pwcwb.manual_red ;}
\def uint16 white_blue() {PWC(0) struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb); return pwcwb.manual_blue;}
\def 0 white_red(uint16 white_red)   {PWC() struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
					    pwcwb.manual_red = white_red;  WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}
\def 0 white_blue(uint16 white_blue) {PWC() struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
					    pwcwb.manual_blue = white_blue;WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}

\def uint16 white_speed() {PWC(0) struct pwc_wb_speed pwcwbs; WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs); return pwcwbs.control_speed;}
\def uint16 white_delay() {PWC(0) struct pwc_wb_speed pwcwbs; WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs); return pwcwbs.control_delay;}
\def 0 white_speed(uint16 white_speed) {PWC() struct pwc_wb_speed pwcwbs;         WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs);
					      pwcwbs.control_speed = white_speed; WIOCTL(fd, VIDIOCPWCSAWBSPEED, &pwcwbs);}
\def 0 white_delay(uint16 white_delay) {PWC() struct pwc_wb_speed pwcwbs;         WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs);
					      pwcwbs.control_delay = white_delay; WIOCTL(fd, VIDIOCPWCSAWBSPEED, &pwcwbs);}

\def int noise_reduction() {PWC(0) int noise_reduction; WIOCTL(fd, VIDIOCPWCGDYNNOISE, &noise_reduction); return noise_reduction;}
\def 0 noise_reduction(int noise_reduction) {PWC()      WIOCTL(fd, VIDIOCPWCSDYNNOISE, &noise_reduction);}
\def int compression()     {PWC(0) int compression;     WIOCTL(fd, VIDIOCPWCSCQUAL,    &compression    ); return compression;    }
\def 0 compression(int compression) {PWC()              WIOCTL(fd, VIDIOCPWCGCQUAL,    &compression    );}

\end class FormatVideoDev {install_format("#io.videodev",4,"");}
void startup_videodev () {
	\startall
}
