/*
	$Id$

	Video4jmax
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "grid.h"
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

const char *video_type_flags[] = {
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

const char *tuner_flags[] = {
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

const char *channel_flags[] = {
	FLAG(0,TUNER,"")
	FLAG(1,AUDIO,"")
	FLAG(2,NORM ,"")
};

const char *video_palette_choice[] = {
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

const char *video_mode_choice[] = {
	OPT( 0,PAL,  "")
	OPT( 1,NTSC, "")
	OPT( 2,SECAM,"")
	OPT( 3,AUTO, "")
};

/* **************************************************************** */

#define TAB "    "

#define WH(_field_,_spec_) \
	whine(TAB "%s: " _spec_, #_field_, $->_field_);

#define WHYX(_name_,_fieldy_,_fieldx_) \
	whine(TAB "%s: y=%d, x=%d", #_name_, $->_fieldy_, $->_fieldx_);

#define WHFLAGS(_field_,_table_) { \
	char *foo; \
	whine(TAB "%s: %s", #_field_, foo=flags_to_s($->_field_,ARRAY(_table_))); \
	free(foo);}

#define WHCHOICE(_field_,_table_) \
	whine(TAB "%s: %s", #_field_, choice_to_s($->_field_,ARRAY(_table_)));

char *flags_to_s(int value, int n, const char **table) {
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

const char *choice_to_s(int value, int n, const char **table) {
	return (value < 0 || value >= COUNT(table)) ? "(Unknown)" : table[value];
}

void VideoChannel_whine(VideoChannel *$) {
	whine("VideoChannel:");
	WH(channel,"%d");
	WH(name,"%-32s");
	WH(tuners,"%d");
	WHFLAGS(flags,channel_flags);
	WH(type,"0x%04x");
	WH(norm,"%d");
}

void VideoTuner_whine(VideoTuner *$) {
	whine("VideoTuner:");
	WH(tuner,"%d");
	WH(name,"%-32s");
	WH(rangelow,"%u");
	WH(rangehigh,"%u");
	WHFLAGS(flags,tuner_flags);
	WHCHOICE(mode,video_mode_choice);
	WH(signal,"%d");
}

void VideoCapability_whine(VideoCapability *$) {
	whine("VideoCapability:");
	WH(name,"%-20s");
	WHFLAGS(type,video_type_flags);
	WH(channels,"%d");
	WH(audios,"%d");
	WHYX(maxsize,maxheight,maxwidth);
	WHYX(minsize,minheight,minwidth);
}

void VideoWindow_whine(VideoWindow *$) {
	whine("VideoWindow:");
	WHYX(pos,y,x);
	WHYX(size,height,width);
	WH(chromakey,"0x%08x");
	WH(flags,"0x%08x");
	WH(clipcount,"%d");
}

void VideoPicture_whine(VideoPicture *$) {
	whine("VideoPicture:");
	WH(brightness,"%d");
	WH(hue,"%d");
	WH(contrast,"%d");
	WH(whiteness,"%d");
	WH(depth,"%d");
	WHCHOICE(palette,video_palette_choice)
}

void video_mbuf_whine(VideoMbuf *$) {
	whine("VideoMBuf:");
	WH(size,"%d");
	WH(frames,"%d");
	WH(offsets[0],"%d");
	WH(offsets[1],"%d");
	WH(offsets[2],"%d");
	WH(offsets[3],"%d");
}

void video_mmap_whine(VideoMmap *$) {
	whine("VideoMBuf:");
	WH(frame,"%u");
	WHYX(size,height,width);
	WHCHOICE(format,video_palette_choice)
};

/* **************************************************************** */

extern FileFormatClass FormatVideoDev;

/*
#define WIOCTL(_f_,_name_,_arg_) do { \
	if (ioctl(_f_,_name_,_arg_) < 0) { \
		whine("ioctl %s: %s",#_name_,strerror(errno)); \
	} \
} while(0)
*/

#define WIOCTL(_f_,_name_,_arg_) \
	((ioctl(_f_,_name_,_arg_) < 0) && \
		(whine("ioctl %s: %s",#_name_,strerror(errno)),1))

void FormatVideoDev_size (FileFormat *$, int height, int width) {
	int v[] = { height, width, 3 };
	VideoWindow grab_win;
	$->dim = Dim_new(3,v);

	WIOCTL($->stream_raw, VIDIOCGWIN, &grab_win);
	VideoWindow_whine(&grab_win);
	grab_win.clipcount = 0;
	grab_win.flags = 0;
	grab_win.height = height;
	grab_win.width  = width;
	VideoWindow_whine(&grab_win);
	WIOCTL($->stream_raw, VIDIOCSWIN, &grab_win);
	WIOCTL($->stream_raw, VIDIOCGWIN, &grab_win);
	VideoWindow_whine(&grab_win);
}

/* picture is read at once by frame() to facilitate debugging. */
Dim *FormatVideoDev_frame_by_read (FileFormat *$, int frame) {

	int n;
	if (frame != -1) return 0;
	$->left = Dim_prod($->dim);

/*	whine("will read %d bytes", $->left); */

	$->stuff = NEW2(uint8,$->left);

	n = (int) read($->stream_raw,$->stuff,$->left);
	if (0> n) {
		whine("error reading: %s", strerror(errno));
	} else if (n < $->left) {
		whine("unexpectedly short picture: %d of %d",n,$->left);
	}
	return $->dim;
}

Dim *FormatVideoDev_frame (FileFormat *$, int frame) {
	VideoMbuf vmbuf;
	VideoMmap vmmap;
	void *buffer;

	int n;
	if (frame != -1) return 0;
	$->left = Dim_prod($->dim);

	whine("will read %d bytes", $->left);

	if ($->stuff) free($->stuff);
	$->stuff = NEW2(uint8,$->left);

	if (WIOCTL($->stream_raw, VIDIOCGMBUF, &vmbuf)) goto err1;
/*	video_mbuf_whine(&vmbuf); */

	buffer = mmap(0,vmbuf.size,
		PROT_READ|PROT_WRITE,MAP_SHARED,$->stream_raw,0);
	if (((int)buffer)==-1) {
		whine("mmap: %s", strerror(errno));
		goto err1;
	}
	vmmap.frame = 0;
	vmmap.format = VIDEO_PALETTE_RGB24;
	vmmap.width  = Dim_get($->dim,1);
	vmmap.height = Dim_get($->dim,0);
	if (WIOCTL($->stream_raw, VIDIOCMCAPTURE, &vmmap)) goto err2;
	if (WIOCTL($->stream_raw, VIDIOCSYNC,     &vmmap)) goto err2;

	/* success goes here */
	memcpy($->stuff,buffer,$->left);

err2:
	munmap (buffer, vmbuf.size);
err1:
	return $->dim;
}

/* picture is converted here. assuming RGB 8:8:8 (RGB24) */
Number *FormatVideoDev_read (FileFormat *$, int n) {
	int i;
	int bs = $->left;
	uint8 *b1 = (uint8 *)$->stuff +
		BitPacking_bytes($->bit_packing) * (Dim_prod($->dim) - $->left)/3;
	Number *b2 = NEW2(Number,n);

	if (!$->dim) return 0;
	if ($->bit_packing) {
		BitPacking_unpack($->bit_packing,n/3,b1,b2);
	} else {
		whine("no bit_packing");
	}
	$->left -= n;
	return b2;
}

void FormatVideoDev_begin (FileFormat *$, Dim *dim) {

}

void FormatVideoDev_flow (FileFormat *$, int n, const Number *data) {

}

void FormatVideoDev_end (FileFormat *$) {

}

void FormatVideoDev_tuner (FileFormat *$, int value) {
	VideoTuner vtuner;
	vtuner.tuner = value;
	if (0> ioctl($->stream_raw, VIDIOCGTUNER, &vtuner)) {
		whine("no tuner #%d", value);
	} else {
		VideoTuner_whine(&vtuner);
		vtuner.mode = VIDEO_MODE_NTSC;
		WIOCTL($->stream_raw, VIDIOCSTUNER, &vtuner);
	}
}

void FormatVideoDev_channel (FileFormat *$, int value) {
	VideoChannel vchan;
	vchan.channel = value;
	if (0> ioctl($->stream_raw, VIDIOCGCHAN, &vchan)) {
		whine("no channel #%d", value);
	} else {
		VideoChannel_whine(&vchan);
		WIOCTL($->stream_raw, VIDIOCSCHAN, &vchan);
		FormatVideoDev_tuner($,0);
	}
}

void FormatVideoDev_option (FileFormat *$, int ac, const fts_atom_t *at) {
	fts_symbol_t sym = GET(0,symbol,SYM(foo));
	int value = GET(1,int,42424242);
	if (sym == SYM(channel)) {
		FormatVideoDev_channel($,value);
	} else if (sym == SYM(tuner)) {
		FormatVideoDev_tuner($,value);
	} else {
		whine("unknown option: %s", fts_symbol_name(sym));
	}
}

void FormatVideoDev_close (FileFormat *$) {
	if ($->stream_raw>=0) close($->stream_raw);
	free($);
}

FileFormat *FormatVideoDev_open (const char *filename, int mode) {
	FileFormat *$ = NEW(FileFormat,1);
	$->qlass  = &FormatVideoDev;
	$->frames = 0;
	$->frame  = FormatVideoDev_frame;
	$->size   = FormatVideoDev_size;
	$->read   = FormatVideoDev_read;
	$->begin  = FormatVideoDev_begin;
	$->flow   = FormatVideoDev_flow;
	$->end    = FormatVideoDev_end;
	$->color  = 0;
	$->option = FormatVideoDev_option;
	$->close  = FormatVideoDev_close;

	$->stuff = 0;

	switch(mode) {
	case 4: break;
	default: whine("unsupported mode (#%d)", mode); goto err;
	}

	whine("will try opening file");

/* actually you only can open devices using open() directly */
/*	$->stream_raw = v4j_file_open(filename, O_RDONLY); */
/*	$->stream_raw =          open(filename, O_RDONLY); */
	$->stream_raw =          open(filename, O_RDWR);
	if (0> $->stream_raw) {
		whine("can't open file: %s", filename);
		goto err;
	}

	{
		VideoCapability vcaps;
		WIOCTL($->stream_raw, VIDIOCGCAP, &vcaps);
		VideoCapability_whine(&vcaps);
	/*	$->size($,vcaps.minheight,vcaps.minwidth); */
		$->size($,vcaps.maxheight,vcaps.maxwidth);
	}

	{
		VideoPicture *gp = NEW(VideoPicture,1);
		WIOCTL($->stream_raw, VIDIOCGPICT, gp);
		VideoPicture_whine(gp);
		gp->depth = 24;
		gp->palette = VIDEO_PALETTE_RGB24;
		VideoPicture_whine(gp);
		WIOCTL($->stream_raw, VIDIOCSPICT, gp);
		WIOCTL($->stream_raw, VIDIOCGPICT, gp);
		VideoPicture_whine(gp);
		switch(gp->palette) {
		case VIDEO_PALETTE_RGB24:
			$->bit_packing = BitPacking_new(3,0x0000ff,0x00ff00,0xff0000);
		break;
		case VIDEO_PALETTE_RGB565:
//			$->bit_packing = BitPacking_new(2,0xf800,0x07e0,0x001f);
			$->bit_packing = BitPacking_new(3,0x0000ff,0x00ff00,0xff0000);
			$->bit_packing = BitPacking_new(3,0xff0000,0x00ff00,0x0000ff);
//			$->bit_packing = BitPacking_new(3,0xff000000,0x00ff0000,0x0000ff00);
		break;
		default:
			whine("can't handle palette %d", gp->palette);
			$->bit_packing = 0;
		}
	}

	FormatVideoDev_channel($,0);

	/* Sometimes a pause is needed here */
	usleep(500000);

	return $;
err:
	$->close($);
	return 0;
}

FileFormatClass FormatVideoDev = {
	"videodev", "Video4linux 1.x", 0,
	FormatVideoDev_open, 0, 0,
};

