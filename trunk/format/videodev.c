/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002 by Mathieu Bouchard

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

#define TAB "    "

#define WH(_field_,_spec_) \
	gfpost(TAB "%s: " _spec_, #_field_, $->_field_);

#define WHYX(_name_,_fieldy_,_fieldx_) \
	gfpost(TAB "%s: y=%d, x=%d", #_name_, $->_fieldy_, $->_fieldx_);

#define WHFLAGS(_field_,_table_) { \
	char *foo; \
	gfpost(TAB "%s: %s", #_field_, foo=flags_to_s($->_field_,COUNT(_table_),_table_)); \
	delete[] foo;}

#define WHCHOICE(_field_,_table_) { \
	char *foo; \
	gfpost(TAB "%s: %s", #_field_, foo=choice_to_s($->_field_,COUNT(_table_),_table_));\
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

static void VideoChannel_gfpost(VideoChannel *$) {
	gfpost("VideoChannel:");
	WH(channel,"%d");
	WH(name,"%-32s");
	WH(tuners,"%d");
	WHFLAGS(flags,channel_flags);
	WH(type,"0x%04x");
	WH(norm,"%d");
}

static void VideoTuner_gfpost(VideoTuner *$) {
	gfpost("VideoTuner:");
	WH(tuner,"%d");
	WH(name,"%-32s");
	WH(rangelow,"%u");
	WH(rangehigh,"%u");
	WHFLAGS(flags,tuner_flags);
	WHCHOICE(mode,video_mode_choice);
	WH(signal,"%d");
}

static void VideoCapability_gfpost(VideoCapability *$) {
	gfpost("VideoCapability:");
	WH(name,"%-20s");
	WHFLAGS(type,video_type_flags);
	WH(channels,"%d");
	WH(audios,"%d");
	WHYX(maxsize,maxheight,maxwidth);
	WHYX(minsize,minheight,minwidth);
}

static void VideoWindow_gfpost(VideoWindow *$) {
	gfpost("VideoWindow:");
	WHYX(pos,y,x);
	WHYX(size,height,width);
	WH(chromakey,"0x%08x");
	WH(flags,"0x%08x");
	WH(clipcount,"%d");
}

static void VideoPicture_gfpost(VideoPicture *$) {
	gfpost("VideoPicture:");
	WH(brightness,"%d");
	WH(hue,"%d");
	WH(contrast,"%d");
	WH(whiteness,"%d");
	WH(depth,"%d");
	WHCHOICE(palette,video_palette_choice);
}

static void video_mbuf_gfpost(VideoMbuf *$) {
	gfpost("VideoMBuf:");
	WH(size,"%d");
	WH(frames,"%d");
	WH(offsets[0],"%d");
	WH(offsets[1],"%d");
	WH(offsets[2],"%d");
	WH(offsets[3],"%d");
}

static void video_mmap_gfpost(VideoMmap *$) {
	gfpost("VideoMMap:");
	WH(frame,"%u");
	WHYX(size,height,width);
	WHCHOICE(format,video_palette_choice);
};

/* **************************************************************** */

struct FormatVideoDev : Format {
	VideoMbuf vmbuf;
	VideoMmap vmmap;
	Pt<uint8> image;
	int pending_frames[2], next_frame;
	int current_channel;
	int current_tuner;
	bool use_mmap;
};

#define IOCTL(_f_,_name_,_arg_) \
	(gfpost("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",_f_,_name_,#_name_,_arg_), \
		ioctl(_f_,_name_,_arg_))

#define WIOCTL(_f_,_name_,_arg_) \
	(gfpost("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",_f_,_name_,#_name_,_arg_), \
	ioctl(_f_,_name_,_arg_) < 0) && \
		(gfpost("ioctl %s: %s",#_name_,strerror(errno)),1)

#define WIOCTL2(_f_,_name_,_arg_) \
	((gfpost("fd%d.ioctl(0x%08x(:%s),0x%08x)\n",_f_,_name_,#_name_,_arg_), \
	ioctl(_f_,_name_,_arg_) < 0) && \
		(gfpost("ioctl %s: %s",#_name_,strerror(errno)), \
		 RAISE("ioctl error"), 0))

#define GETFD NUM2INT(rb_funcall(rb_ivar_get(rself,SI(@stream)),SI(fileno),0))

METHOD(FormatVideoDev,size) {
	int fd = GETFD;
	int v[] = {NUM2INT(argv[0]), NUM2INT(argv[1]), 3};
	VideoWindow grab_win;

	/* bug here: won't flush the frame queue */

	if ($->dim) delete $->dim;
	$->dim = new Dim(3,v);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	VideoWindow_gfpost(&grab_win);
	grab_win.clipcount = 0;
	grab_win.flags = 0;
	if (v[0] && v[1]) {
		grab_win.height = v[0];
		grab_win.width  = v[1];
	}
	VideoWindow_gfpost(&grab_win);
	WIOCTL(fd, VIDIOCSWIN, &grab_win);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	VideoWindow_gfpost(&grab_win);
}

METHOD(FormatVideoDev,dealloc_image) {
	if (!$->image) return;
	if (!$->use_mmap) {
		delete[] (uint8 *)$->image;
	} else {
		munmap($->image, $->vmbuf.size);
		$->image = Pt<uint8>();
	}
}

METHOD(FormatVideoDev,alloc_image) {
	if (!$->use_mmap) {
		$->image = ARRAY_NEW(uint8,$->dim->prod(0,1)*$->bit_packing->bytes);
		return;
	}
	RAISE("hello");
	int fd = GETFD;
	WIOCTL2(fd, VIDIOCGMBUF, &$->vmbuf);
	video_mbuf_gfpost(&$->vmbuf);
	$->image = Pt<uint8>((uint8 *) mmap(
		0,$->vmbuf.size,
		PROT_READ|PROT_WRITE,MAP_SHARED,
		fd,0), $->vmbuf.size);
	if (((int)$->image)<=0) {
		$->image=Pt<uint8>();
		RAISE("mmap: %s", strerror(errno));
	}
}

METHOD(FormatVideoDev,frame_ask) {
	int fd = GETFD;
	$->pending_frames[0] = $->pending_frames[1];
	$->vmmap.frame = $->pending_frames[1] = $->next_frame;
	$->vmmap.format = VIDEO_PALETTE_RGB24;
	$->vmmap.width  = $->dim->get(1);
	$->vmmap.height = $->dim->get(0);
//	gfpost("will try:");
//	video_mmap_gfpost(&$->vmmap);
	WIOCTL2(fd, VIDIOCMCAPTURE, &$->vmmap);
//	gfpost("driver gave us:");
//	video_mmap_gfpost(&$->vmmap);
	$->next_frame = ($->pending_frames[1]+1) % $->vmbuf.frames;
}

static void FormatVideoDev_frame_finished (FormatVideoDev *$, GridOutlet *out,
Pt<uint8> buf) {
	out->begin($->dim->dup());
	/* picture is converted here. */
	int sy = $->dim->get(0);
	int sx = $->dim->get(1);
	int bs = $->dim->prod(1);
	STACK_ARRAY(Number,b2,bs);
	for(int y=0; y<sy; y++) {
		Pt<uint8> b1 = buf + $->bit_packing->bytes * sx * y;
		$->bit_packing->unpack(sx,b1,b2);
		out->send(bs,b2);
	}
	out->end();
}

int read2(int fd,uint8 *image,int n) {
	int r=0;
	while (n>0) {
		int rr=read(fd,image,n);
		if (rr<0) return rr;
		r+=rr, image+=rr, n-=rr;
	}
	return r;
}

int read3(int fd,uint8 *image,int n) {
	int r=0;
	int rr=read(fd,image,n);
	if (rr<0) return rr;
	return n;
}

METHOD(FormatVideoDev,frame) {
	if (!$->bit_packing) RAISE("no bit_packing");
	if (!$->image) rb_funcall(rself,SI(alloc_image),0);
	int fd = GETFD;
	if (!$->use_mmap) {
//		gfpost("$=%p; $->image=%p",$,$->image);
		/* picture is read at once by frame() to facilitate debugging. */
		int tot = $->dim->prod(0,1) * $->bit_packing->bytes;

//		memset($->image,0,tot);
//		int n = tot;

		int n = (int) read3(fd,$->image,tot);
		if (n==tot) FormatVideoDev_frame_finished($,$->out[0],$->image);
		if (0> n) RAISE("error reading: %s", strerror(errno));
		if (n < tot) RAISE("unexpectedly short picture: %d of %d",n,tot);
		return;
	}
	int finished_frame;
	if ($->pending_frames[0] < 0) {
		$->next_frame = 0;
		rb_funcall(rself,SI(frame_ask),0);
		rb_funcall(rself,SI(frame_ask),0);
	}
	$->vmmap.frame = finished_frame = $->pending_frames[0];
	WIOCTL2(fd, VIDIOCSYNC, &$->vmmap);
	FormatVideoDev_frame_finished($,$->out[0],$->image+$->vmbuf.offsets[finished_frame]);
	rb_funcall(rself,SI(frame_ask),0);
}

GRID_BEGIN(FormatVideoDev,0) { RAISE("can't write."); }
GRID_FLOW(FormatVideoDev,0) {}
GRID_END(FormatVideoDev,0) {}

METHOD(FormatVideoDev,norm) {
	int value = INT(argv[0]);
	int fd = GETFD;
	VideoTuner vtuner;
	vtuner.tuner = $->current_tuner;
	if (value<0 || value>3) RAISE("norm must be in range 0..3");
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) {
		gfpost("no tuner #%d", value);
	} else {
		vtuner.mode = value;
		VideoTuner_gfpost(&vtuner);
		WIOCTL(fd, VIDIOCSTUNER, &vtuner);
	}
}

METHOD(FormatVideoDev,tuner) {
	int value = INT(argv[0]);
	int fd = GETFD;
	VideoTuner vtuner;
	vtuner.tuner = value;
	$->current_tuner = value;
	if (0> IOCTL(fd, VIDIOCGTUNER, &vtuner)) RAISE("no tuner #%d", value);
	vtuner.mode = VIDEO_MODE_NTSC;
	VideoTuner_gfpost(&vtuner);
	WIOCTL(fd, VIDIOCSTUNER, &vtuner);
}

METHOD(FormatVideoDev,channel) {
	int value = INT(argv[0]);
	int fd = GETFD;
	VideoChannel vchan;
	vchan.channel = value;
	$->current_channel = value;
	if (0> IOCTL(fd, VIDIOCGCHAN, &vchan)) RAISE("no channel #%d", value);
	VideoChannel_gfpost(&vchan);
	WIOCTL(fd, VIDIOCSCHAN, &vchan);
	rb_funcall(rself,SI(tuner),1,INT2NUM(0));
}

METHOD(FormatVideoDev,frequency) {
	int value = INT(argv[0]);
	int fd = GETFD;
	if (0> IOCTL(fd, VIDIOCSFREQ, &value)) RAISE("can't set frequency to %d",value);
}

METHOD(FormatVideoDev,transfer) {
	VALUE sym = argv[0];
	gfpost("transfer %s",rb_sym_name(argv[0]));
	if (sym == SYM(read)) {
		rb_funcall(rself,SI(dealloc_image),0);
		$->use_mmap = false;
		gfpost("transfer read");
	} else if (sym == SYM(mmap)) {
		rb_funcall(rself,SI(dealloc_image),0);
		$->use_mmap = true;
		gfpost("transfer mmap");
	} else RAISE("don't know that transfer mode");
}

METHOD(FormatVideoDev,option) {
	int fd = GETFD;
	VALUE sym = argv[0];
	int value = argv[1];
	gfpost("option %s %08x",rb_sym_name(sym),value);
	if (sym == SYM(channel)) {
		rb_funcall(rself,SI(channel),1,value);
	} else if (sym == SYM(tuner)) {
		rb_funcall(rself,SI(tuner),1,value);
	} else if (sym == SYM(norm)) {
		rb_funcall(rself,SI(norm),1,value);
	} else if (sym == SYM(frequency)) {
		rb_funcall(rself,SI(frequency),1,value);
	} else if (sym == SYM(transfer)) {
		rb_funcall(rself,SI(transfer),1,value);
	} else if (sym == SYM(size)) {
		rb_funcall(rself,SI(size),2,value,argv[2]);

#define PICTURE_ATTR(_name_) \
	} else if (sym == SYM(_name_)) { \
		VideoPicture vp; \
		WIOCTL(fd, VIDIOCGPICT, &vp); \
		vp._name_ = INT(value); \
		WIOCTL(fd, VIDIOCSPICT, &vp); \

	PICTURE_ATTR(brightness)
	PICTURE_ATTR(hue)
	PICTURE_ATTR(colour)
	PICTURE_ATTR(contrast)
	PICTURE_ATTR(whiteness)

	} else {
		RAISE("crap");
		rb_call_super(argc,argv);
	}
}

METHOD(FormatVideoDev,close) {
	if ($->image) rb_funcall(rself,SI(dealloc_image),0);
	EVAL("@stream.close if @stream");
	rb_call_super(argc,argv);
}

METHOD(FormatVideoDev,init2) {
	int fd = GETFD;
	VideoCapability vcaps;
	VideoPicture *gp = new VideoPicture;

	WIOCTL(fd, VIDIOCGCAP, &vcaps);
	VideoCapability_gfpost(&vcaps);
	rb_funcall(rself,SI(size),2,INT2NUM(vcaps.maxheight),INT2NUM(vcaps.maxwidth));

	WIOCTL(fd, VIDIOCGPICT, gp);
	gfpost("original settings:");
	VideoPicture_gfpost(gp);
	gp->depth = 24;
	gp->palette = VIDEO_PALETTE_RGB24;

	gfpost("trying settings:");
	VideoPicture_gfpost(gp);
	WIOCTL(fd, VIDIOCSPICT, gp);
	WIOCTL(fd, VIDIOCGPICT, gp);
	gfpost("driver gave us settings:");
	VideoPicture_gfpost(gp);

	switch(gp->palette) {
	case VIDEO_PALETTE_RGB24:{
//		uint32 masks[3] = { 0x0000ff,0x00ff00,0xff0000 };
		uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
		$->bit_packing = new BitPacking(is_le(),3,3,masks);
	}break;
	case VIDEO_PALETTE_RGB565:{
		/* I think the BTTV card is lying. */
		/* BIG_HACK_FIX_ME */
//			uint32 masks[3] = { 0xf800,0x07e0,0x001f };
//			uint32 masks[3] = { 0x0000ff,0x00ff00,0xff0000 };
			uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
//			uint32 masks[3] = { 0xff000000,0x00ff0000,0x0000ff00 };
		$->bit_packing = new BitPacking(is_le(),3,3,masks);
	}break;
	default:
		gfpost("can't handle palette %d", gp->palette);
	}
	rb_funcall(rself,SI(channel),1,INT2NUM(0));
	rb_funcall(rself,SI(size),2,INT2NUM(0),INT2NUM(0));
}

METHOD(FormatVideoDev,init) {
	rb_call_super(argc,argv);
	argv++, argc--;
	$->pending_frames[0] = -1;
	$->pending_frames[1] = -1;
	$->image = Pt<uint8>();
	$->use_mmap = true;
	if (argc<1) RAISE("usage: videodev <devicename>");
	const char *filename = rb_sym_name(argv[0]);
	VALUE file = rb_funcall(EVAL("File"),SI(open),2,
		rb_str_new2(filename), rb_str_new2("r+"));
	rb_ivar_set(rself,SI(@stream),file);
	rb_p(file);
	rb_p(rb_ivar_get(rself,SI(@stream)));
	if (argc>1 && argv[1]==SYM(noinit)) {
		uint32 masks[3] = { 0xff0000,0x00ff00,0x0000ff };
		$->bit_packing = new BitPacking(is_le(),3,3,masks);
		int v[]={288,352,3};
		$->dim = new Dim(3,v);
	} else {
		rb_funcall(rself,SI(init2),0);
	}
	/* Sometimes a pause is needed here (?) */
	usleep(250000);
}

/* **************************************************************** */

static void startup (GridClass *$) {
	IEVAL($->rubyclass,
	"conf_format 4,'videodev','Video4linux 1.x'");
}

GRCLASS(FormatVideoDev,"FormatVideoDev",
inlets:1,outlets:1,startup:startup,LIST(GRINLET(FormatVideoDev,0,4)),
DECL(FormatVideoDev,init),
DECL(FormatVideoDev,init2),
DECL(FormatVideoDev,alloc_image),
DECL(FormatVideoDev,dealloc_image),
DECL(FormatVideoDev,frame_ask),
DECL(FormatVideoDev,size),
DECL(FormatVideoDev,norm),
DECL(FormatVideoDev,tuner),
DECL(FormatVideoDev,channel),
DECL(FormatVideoDev,frequency),
DECL(FormatVideoDev,transfer), // for some reason there's a method blackhole...
DECL(FormatVideoDev,frame),
DECL(FormatVideoDev,option),
DECL(FormatVideoDev,close))

