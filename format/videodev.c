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

/* **************************************************************** */

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
	WH(name,"%-32s");
	WH(tuners,"%d");
	WHFLAGS(flags,channel_flags);
	WH(type,"0x%04x");
	WH(norm,"%d");
	gfpost("%s",buf);
}

static void gfpost(VideoTuner *self) {
	char buf[256] = "[VideoTuner] ";
	WH(tuner,"%d");
	WH(name,"%-32s");
	WH(rangelow,"%lu");
	WH(rangehigh,"%lu");
	WHFLAGS(flags,tuner_flags);
	WHCHOICE(mode,video_mode_choice);
	WH(signal,"%d");
	gfpost("%s",buf);
}

static void gfpost(VideoCapability *self) {
	char buf[256] = "[VideoCapability] ";
	WH(name,"%-20s");
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
	VideoMbuf vmbuf;
	VideoMmap vmmap;
	Pt<uint8> image;
	int pending_frames[2], next_frame;
	int current_channel, current_tuner;
	bool use_mmap;
	BitPacking *bit_packing;
	Dim *dim;

	FormatVideoDev () :	use_mmap(true), dim(0) {}
	void frame_finished (Pt<uint8> buf);

	\decl void initialize (Symbol mode, String filename, Symbol option=Qnil);
	\decl void close ();
	\decl void size (int sy, int sx);
	\decl void option (Symbol sym, int value);
	\decl void alloc_image ();
	\decl void dealloc_image ();
	\decl void frame ();
	\decl void frame_ask ();
	\decl void norm (int value);
	\decl void tuner (int value);
	\decl void channel (int value);
	\decl void frequency (int value);
	\decl void transfer (Symbol sym);
	\decl void initialize2 ();
	GRINLET3(0);
};

#define DEBUG(args...) 42
//#define DEBUG(args...) gfpost

#define IOCTL(_f_,_name_,_arg_) \
	(DEBUG("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",_f_,_name_,#_name_,_arg_), \
		ioctl(_f_,_name_,_arg_))

#define WIOCTL(_f_,_name_,_arg_) \
	(DEBUG("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",_f_,_name_,#_name_,_arg_), \
	ioctl(_f_,_name_,_arg_) < 0) && \
		(gfpost("ioctl %s: %s",#_name_,strerror(errno)),1)

#define WIOCTL2(_f_,_name_,_arg_) \
	((DEBUG("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",_f_,_name_,#_name_,_arg_), \
	ioctl(_f_,_name_,_arg_) < 0) && \
		(gfpost("ioctl %s: %s",#_name_,strerror(errno)), \
		 RAISE("ioctl error"), 0))

#define GETFD NUM2INT(rb_funcall(rb_ivar_get(rself,SI(@stream)),SI(fileno),0))

\def void size (int sy, int sx) {
	int fd = GETFD;
	int32 v[] = {sy,sx,3};
	VideoWindow grab_win;

	/* bug here: won't flush the frame queue */
	if (dim) delete dim;
	dim = new Dim(3,v);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	gfpost(&grab_win);
	grab_win.clipcount = 0;
	grab_win.flags = 0;
	if (v[0] && v[1]) {
		grab_win.height = v[0];
		grab_win.width  = v[1];
	}
	gfpost(&grab_win);
	WIOCTL(fd, VIDIOCSWIN, &grab_win);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	gfpost(&grab_win);
}

\def void dealloc_image () {
	if (!image) return;
	if (!use_mmap) {
		delete[] (uint8 *)image;
	} else {
		munmap(image, vmbuf.size);
		image = Pt<uint8>();
	}
}

\def void alloc_image () {
	if (!use_mmap) {
		image = ARRAY_NEW(uint8,dim->prod(0,1)*bit_packing->bytes);
		return;
	}
	int fd = GETFD;
	WIOCTL2(fd, VIDIOCGMBUF, &vmbuf);
	image = Pt<uint8>((uint8 *)
		mmap(0,vmbuf.size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0),
		vmbuf.size);
	if (((int)image)<=0) {
		image=Pt<uint8>();
		RAISE("mmap: %s", strerror(errno));
	}
}

\def void frame_ask () {
	int fd = GETFD;
	pending_frames[0] = pending_frames[1];
	vmmap.frame = pending_frames[1] = next_frame;
	vmmap.format = VIDEO_PALETTE_RGB24;
	vmmap.width  = dim->get(1);
	vmmap.height = dim->get(0);
	WIOCTL2(fd, VIDIOCMCAPTURE, &vmmap);
	next_frame = (pending_frames[1]+1) % vmbuf.frames;
}

void FormatVideoDev::frame_finished (Pt<uint8> buf) {
	GridOutlet *o = out[0];
	o->begin(dim->dup(),NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
	/* picture is converted here. */
	int sy = dim->get(0);
	int sx = dim->get(1);
	int bs = dim->prod(1);
	STACK_ARRAY(uint8,b2,bs);
	for(int y=0; y<sy; y++) {
		Pt<uint8> b1 = buf + bit_packing->bytes * sx * y;
		bit_packing->unpack(sx,b1,b2);
		o->send(bs,b2);
	}
}

static int read2(int fd, uint8 *image, int n) {
	int r=0;
	for (; n>0; ) {
		int rr=read(fd,image,n);
		if (rr<0) return rr;
		r+=rr, image+=rr, n-=rr;
	}
	return r;
}

static int read3(int fd, uint8 *image, int n) {
	int r=read(fd,image,n);
	if (r<0) return r;
	return n;
}

\def void frame () {
	if (!bit_packing) RAISE("no bit_packing");
	if (!image) rb_funcall(rself,SI(alloc_image),0);
	int fd = GETFD;
	if (!use_mmap) {
		/* picture is read at once by frame() to facilitate debugging. */
		int tot = dim->prod(0,1) * bit_packing->bytes;
		int n = (int) read3(fd,image,tot);
		if (n==tot) frame_finished(image);
		if (0> n) RAISE("error reading: %s", strerror(errno));
		if (n < tot) RAISE("unexpectedly short picture: %d of %d",n,tot);
		return;
	}
	int finished_frame;
	if (pending_frames[0] < 0) {
		next_frame = 0;
		for (int i=0; i<2; i++) rb_funcall(rself,SI(frame_ask),0);
	}
	vmmap.frame = finished_frame = pending_frames[0];
	WIOCTL2(fd, VIDIOCSYNC, &vmmap);
	frame_finished(image+vmbuf.offsets[finished_frame]);
	rb_funcall(rself,SI(frame_ask),0);
}

GRID_INLET(FormatVideoDev,0) {
	RAISE("can't write.");
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

\def void norm (int value) {
	int fd = GETFD;
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

\def void tuner (int value) {
	int fd = GETFD;
	VideoTuner vtuner;
	vtuner.tuner = current_tuner = value;
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) RAISE("no tuner #%d", value);
	vtuner.mode = VIDEO_MODE_NTSC;
	gfpost(&vtuner);
	WIOCTL(fd, VIDIOCSTUNER, &vtuner);
}

\def void channel (int value) {
	int fd = GETFD;
	VideoChannel vchan;
	vchan.channel = value;
	current_channel = value;
	if (0> IOCTL(fd, VIDIOCGCHAN, &vchan)) RAISE("no channel #%d", value);
	gfpost(&vchan);
	WIOCTL(fd, VIDIOCSCHAN, &vchan);
	rb_funcall(rself,SI(tuner),1,INT2NUM(0));
}

\def void frequency (int value) {
	int fd = GETFD;
	if (0> IOCTL(fd, VIDIOCSFREQ, &value)) RAISE("can't set frequency to %d",value);
}

\def void transfer (Symbol sym) {
	if (sym == SYM(read)) {
		rb_funcall(rself,SI(dealloc_image),0);
		use_mmap = false;
		gfpost("transfer read");
	} else if (sym == SYM(mmap)) {
		rb_funcall(rself,SI(dealloc_image),0);
		use_mmap = true;
		gfpost("transfer mmap");
	} else RAISE("don't know that transfer mode");
}

\def void option (Symbol sym, int value) {
	int fd = GETFD;
	if (0) {
#define PICTURE_ATTR(_name_) \
	} else if (sym == SYM(_name_)) { \
		VideoPicture vp; \
		WIOCTL(fd, VIDIOCGPICT, &vp); \
		vp._name_ = value; \
		WIOCTL(fd, VIDIOCSPICT, &vp);
	PICTURE_ATTR(brightness)
	PICTURE_ATTR(hue)
	PICTURE_ATTR(colour)
	PICTURE_ATTR(contrast)
	PICTURE_ATTR(whiteness)
	} else {
		rb_call_super(argc,argv);
	}
}

\def void close () {
	if (bit_packing) delete bit_packing;
	if (image) rb_funcall(rself,SI(dealloc_image),0);
	IEVAL(rself,"GridFlow.post \"VideoDev#close: #{self.inspect}\"; @stream.close if @stream");
	rb_call_super(argc,argv);
}

\def void initialize2 () {
	int fd = GETFD;
	VideoCapability vcaps;
	VideoPicture *gp = new VideoPicture;

	WIOCTL(fd, VIDIOCGCAP, &vcaps);
	gfpost(&vcaps);
	rb_funcall(rself,SI(size),2,INT2NUM(vcaps.maxheight),INT2NUM(vcaps.maxwidth));

	WIOCTL(fd, VIDIOCGPICT, gp);
	gfpost("original settings:");
	gfpost(gp);
	gp->depth = 24;
	gp->palette = VIDEO_PALETTE_RGB24;

	gfpost("trying settings:");
	gfpost(gp);
	WIOCTL(fd, VIDIOCSPICT, gp);
	WIOCTL(fd, VIDIOCGPICT, gp);
	gfpost("driver gave us settings:");
	gfpost(gp);

	switch(gp->palette) {
	case VIDEO_PALETTE_RGB24:{
//		uint32 masks[3] = { 0x0000ff,0x00ff00,0xff0000 };
		uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
		bit_packing = new BitPacking(is_le(),3,3,masks);
	}break;
	case VIDEO_PALETTE_RGB565:{
		/* I think the BTTV card is lying. */
		/* BIG_HACK_FIX_ME */
//			uint32 masks[3] = { 0xf800,0x07e0,0x001f };
//			uint32 masks[3] = { 0x0000ff,0x00ff00,0xff0000 };
			uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
//			uint32 masks[3] = { 0xff000000,0x00ff0000,0x0000ff00 };
		bit_packing = new BitPacking(is_le(),3,3,masks);
	}break;
	default:
		gfpost("can't handle palette %d", gp->palette);
	}
	rb_funcall(rself,SI(channel),1,INT2NUM(0));
	rb_funcall(rself,SI(size),2,INT2NUM(0),INT2NUM(0));
}

\def void initialize (Symbol mode, String filename, Symbol option=Qnil) {
	rb_call_super(argc,argv);
	pending_frames[0] = -1;
	pending_frames[1] = -1;
	image = Pt<uint8>();
	rb_ivar_set(rself,SI(@stream),
		rb_funcall(rb_cFile,SI(open),2,filename,rb_str_new2("r+")));
	if (option==SYM(noinit)) {
		uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
		bit_packing = new BitPacking(is_le(),3,3,masks);
		int32 v[]={288,352,3};
		dim = new Dim(3,v);
	} else {
		rb_funcall(rself,SI(initialize2),0);
	}
	/* Sometimes a pause is needed here (?) */
	usleep(250000);
}

/* **************************************************************** */

GRCLASS(FormatVideoDev,LIST(GRINLET2(FormatVideoDev,0,4)),
\grdecl
){
	IEVAL(rself,"install 'FormatVideoDev',1,1;"
	"conf_format 4,'videodev','Video4linux 1.x'");
}

\end class FormatVideoDev
