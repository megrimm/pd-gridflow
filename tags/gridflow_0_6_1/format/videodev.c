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

typedef struct video_capability VideoCapability;
typedef struct video_channel    VideoChannel   ;
typedef struct video_tuner      VideoTuner     ;
typedef struct video_window     VideoWindow    ;
typedef struct video_picture    VideoPicture   ;
typedef struct video_mbuf       VideoMbuf      ;
typedef struct video_mmap       VideoMmap      ;

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
	FREE(foo);}

#define WHCHOICE(_field_,_table_) { \
	char *foo; \
	gfpost(TAB "%s: %s", #_field_, foo=choice_to_s($->_field_,COUNT(_table_),_table_));\
	FREE(foo);}

static char *flags_to_s(int value, int n, const char **table) {
	int i;
	char *foo = NEW(char,256);
	*foo = 0;
	for(i=0; i<n; i++) {
		if ((value & (1<<i)) == 0) continue;
		if (*foo) strcat(foo," | ");
		strcat(foo,table[i]);
	}
	if (!*foo) strcat(foo,"0");
	return foo;
}

static char *choice_to_s(int value, int n, const char **table) {
	if (value < 0 || value >= n) {
		char *foo = NEW(char,64);
		sprintf(foo,"(Unknown #%d)",value);
		return foo;
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
	uint8 *image;
	int pending_frames[2], next_frame;
	int current_channel;
	int current_tuner;
};

#define WIOCTL(_f_,_name_,_arg_) \
	((ioctl(_f_,_name_,_arg_) < 0) && \
		(gfpost("ioctl %s: %s",#_name_,strerror(errno)),1))

#define GETFD NUM2INT(rb_funcall(rb_ivar_get(rself,SI(@stream)),SI(fileno),0))

METHOD(FormatVideoDev,size) {
	int fd = GETFD;
	int v[] = {NUM2INT(argv[0]), NUM2INT(argv[1]), 3};
	VideoWindow grab_win;

	/* bug here: won't flush the frame queue */

	FREE($->dim);
	$->dim = new Dim(3,v);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	VideoWindow_gfpost(&grab_win);
	grab_win.clipcount = 0;
	grab_win.flags = 0;
	grab_win.height = v[0];
	grab_win.width  = v[1];
	VideoWindow_gfpost(&grab_win);
	WIOCTL(fd, VIDIOCSWIN, &grab_win);
	WIOCTL(fd, VIDIOCGWIN, &grab_win);
	VideoWindow_gfpost(&grab_win);
}

/* picture is read at once by frame() to facilitate debugging. */
/*
static Dim *FormatVideoDev_frame_by_read (Format *$, int frame) {

	int n;
	if (frame != -1) return 0;
	$->left = $->dim->prod();

	$->stuff = NEW2(uint8,$->left);

	n = (int) read($->stream,$->stuff,$->left);
	if (0> n) {
		gfpost("error reading: %s", strerror(errno));
	} else if (n < $->left) {
		gfpost("unexpectedly short picture: %d of %d",n,$->left);
	}
	return $->dim;
}
*/

METHOD(FormatVideoDev,dealloc_image) {
	munmap($->image, $->vmbuf.size);
}

METHOD(FormatVideoDev,alloc_image) {
	int fd = GETFD;
	if (WIOCTL(fd, VIDIOCGMBUF, &$->vmbuf)) RAISE("ioctl error");
	video_mbuf_gfpost(&$->vmbuf);
	$->image = (uint8 *) mmap(
		0,$->vmbuf.size,
		PROT_READ|PROT_WRITE,MAP_SHARED,
		fd,0);
	if (((int)$->image)<=0) {
		$->image=0;
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
	if (WIOCTL(fd, VIDIOCMCAPTURE, &$->vmmap)) RAISE("ioctl error");
//	gfpost("driver gave us:");
//	video_mmap_gfpost(&$->vmmap);
	$->next_frame = ($->pending_frames[1]+1) % $->vmbuf.frames;
}

static void FormatVideoDev_frame_finished (FormatVideoDev *$, GridOutlet *out, uint8 *buf) {
	out->begin($->dim->dup());

	/* picture is converted here. */
	{
		int sy = $->dim->get(0);
		int sx = $->dim->get(1);
		int y;
		int bs = $->dim->prod(1);
		Number b2[bs];
		for(y=0; y<sy; y++) {
			uint8 *b1 = buf + $->bit_packing->bytes * sx * y;
			$->bit_packing->unpack(sx,b1,b2);
			out->send(bs,b2);
		}
	}
	out->end();
}

METHOD(FormatVideoDev,frame) {
	int fd = GETFD;
	int finished_frame;

	if (!$->bit_packing) RAISE("no bit_packing");

	if (!$->image) {
		rb_funcall(rself,SI(alloc_image),0);
	}

	if ($->pending_frames[0] < 0) {
		$->next_frame = 0;
		rb_funcall(rself,SI(frame_ask),0);
		rb_funcall(rself,SI(frame_ask),0);
	}

	$->vmmap.frame = finished_frame = $->pending_frames[0];
	if (WIOCTL(fd, VIDIOCSYNC, &$->vmmap)) RAISE("wioctl");

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
	if (value<0 || value>3) {
		gfpost("norm must be in range 0..3");
		return;
	}
	if (0> ioctl(fd, VIDIOCGTUNER, &vtuner)) {
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
	if (0> ioctl(fd, VIDIOCGTUNER, &vtuner)) {
		gfpost("no tuner #%d", value);
	} else {
		vtuner.mode = VIDEO_MODE_NTSC;
		VideoTuner_gfpost(&vtuner);
		WIOCTL(fd, VIDIOCSTUNER, &vtuner);
	}
}

METHOD(FormatVideoDev,channel) {
	int value = INT(argv[0]);
	int fd = GETFD;
	VideoChannel vchan;
	vchan.channel = value;
	$->current_channel = value;
	if (0> ioctl(fd, VIDIOCGCHAN, &vchan)) {
		gfpost("no channel #%d", value);
	} else {
		VideoChannel_gfpost(&vchan);
		WIOCTL(fd, VIDIOCSCHAN, &vchan);
		rb_funcall(rself,SI(tuner),1,INT2NUM(0));
	}
}

METHOD(FormatVideoDev,option) {
	int fd = GETFD;
	VALUE sym = argv[0];
	int value = NUM2INT(argv[1]);
	if (sym == SYM(channel)) {
		rb_funcall(rself,SI(channel),1,INT2NUM(value));
	} else if (sym == SYM(tuner)) {
		rb_funcall(rself,SI(tuner),1,INT2NUM(value));
	} else if (sym == SYM(norm)) {
		rb_funcall(rself,SI(norm),1,INT2NUM(value));
	} else if (sym == SYM(size)) {
		int value2 = NUM2INT(argv[2]);
		rb_funcall(rself,SI(size),2,INT2NUM(value),INT2NUM(value2));

#define PICTURE_ATTR(_name_) \
	} else if (sym == SYM(_name_)) { \
		VideoPicture vp; \
		WIOCTL(fd, VIDIOCGPICT, &vp); \
		vp._name_ = value; \
		WIOCTL(fd, VIDIOCSPICT, &vp); \

	PICTURE_ATTR(brightness)
	PICTURE_ATTR(hue)
	PICTURE_ATTR(colour)
	PICTURE_ATTR(contrast)
	PICTURE_ATTR(whiteness)

	} else {
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
//	char *s;
	VideoPicture *gp = NEW(VideoPicture,1);

	WIOCTL(fd, VIDIOCGCAP, &vcaps);
	VideoCapability_gfpost(&vcaps);

/*
	PUT(0,symbol,SYM(size));
	PUT(1,int,vcaps.maxheight);
	PUT(2,int,vcaps.maxwidth);
	$->cl->option((Format *)$,3,at);
*/
	rb_funcall(rself,SI(size),2,INT2NUM(vcaps.maxheight),INT2NUM(vcaps.maxwidth));

	WIOCTL(fd, VIDIOCGPICT, gp);
	gfpost("original settings:");
	VideoPicture_gfpost(gp);
	gp->depth = 24;
	gp->palette = VIDEO_PALETTE_RGB24;

//	FREE(s);
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
}

METHOD(FormatVideoDev,init) {
	const char *filename;
	int stream;
	rb_call_super(argc,argv);
	argv++, argc--;
	$->pending_frames[0] = -1;
	$->pending_frames[1] = -1;
	$->image = 0;
	if (argc!=1) RAISE("usage: videodev <devicename>");
	filename = rb_sym_name(argv[0]);
	{
		VALUE file = rb_funcall(EVAL("File"),SI(open),2,
			rb_str_new2(filename), rb_str_new2("r+"));
		rb_ivar_set(rself,SI(@stream),file);
		rb_p(file);
		rb_p(rb_ivar_get(rself,SI(@stream)));
	}
	rb_funcall(rself,SI(init2),0);
	/* Sometimes a pause is needed here (?) */
	usleep(250000);
}

/* **************************************************************** */

FMTCLASS(FormatVideoDev,"videodev","Video4linux 1.x",FF_R,
inlets:1,outlets:1,LIST(GRINLET(FormatVideoDev,0)),
DECL(FormatVideoDev,init),
DECL(FormatVideoDev,init2),
DECL(FormatVideoDev,alloc_image),
DECL(FormatVideoDev,dealloc_image),
DECL(FormatVideoDev,frame_ask),
DECL(FormatVideoDev,size),
DECL(FormatVideoDev,norm),
DECL(FormatVideoDev,tuner),
DECL(FormatVideoDev,channel),
DECL(FormatVideoDev,frame),
DECL(FormatVideoDev,option),
DECL(FormatVideoDev,close))

