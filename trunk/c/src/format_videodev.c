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

#define FLAG(_name_,_num_,_desc_) #_name_,

#define OPTION(_name_,_num_,_desc_) #_name_,

const char *video_type_flags[] = {
	FLAG(CAPTURE,       1,      "Can capture")
	FLAG(TUNER,         2,      "Can tune")
	FLAG(TELETEXT,      4,      "Does teletext")
	FLAG(OVERLAY,       8,      "Overlay onto frame buffer")
	FLAG(CHROMAKEY,     16,     "Overlay by chromakey")
	FLAG(CLIPPING,      32,     "Can clip")
	FLAG(FRAMERAM,      64,     "Uses the frame buffer memory")
	FLAG(SCALES,        128,    "Scalable")
	FLAG(MONOCHROME,    256,    "Monochrome only")
	FLAG(SUBCAPTURE,    512,    "Can capture subareas of the image")
	FLAG(MPEG_DECODER,  1024,   "Can decode MPEG streams")
	FLAG(MPEG_ENCODER,  2048,   "Can encode MPEG streams")
	FLAG(MJPEG_DECODER, 4096,   "Can decode MJPEG streams")
	FLAG(MJPEG_ENCODER, 8192,   "Can encode MJPEG streams")
};

const char *tuner_flags[] = {
	FLAG(PAL,      1,  "")
	FLAG(NTSC,     2,  "")
	FLAG(SECAM,    4,  "")
	FLAG(LOW,      8,  "Uses KHz not MHz")
	FLAG(NORM,     16, "Tuner can set norm")
	FLAG(DUMMY5,   32, "")
	FLAG(DUMMY6,   64, "")
	FLAG(STEREO_ON,128,"Tuner is seeing stereo")
	FLAG(RDS_ON,   256,"Tuner is seeing an RDS datastream")
	FLAG(MBS_ON,   512,"Tuner is seeing an MBS datastream")
};

const char *channel_flags[] = {
	FLAG(TUNER,1,"")
	FLAG(AUDIO,2,"")
	FLAG(NORM ,4,"")
};

const char *video_palette_options[] = {
	OPTION(NIL,      0,      "(nil)")
	OPTION(GREY,     1,      "Linear greyscale")
	OPTION(HI240,    2,      "High 240 cube (BT848)")
	OPTION(RGB565,   3,      "565 16 bit RGB")
	OPTION(RGB24,    4,      "24bit RGB")
	OPTION(RGB32,    5,      "32bit RGB")
	OPTION(RGB555,   6,      "555 15bit RGB")
	OPTION(YUV422,   7,      "YUV422 capture")
	OPTION(YUYV,     8,		"")
	OPTION(UYVY,     9,      "The great thing about standards is ...")
	OPTION(YUV420,   10,     "")
	OPTION(YUV411,   11,     "YUV411 capture")
	OPTION(RAW,      12,     "RAW capture (BT848)")
	OPTION(YUV422P,  13,     "YUV 4:2:2 Planar")
	OPTION(YUV411P,  14,     "YUV 4:1:1 Planar")
	OPTION(YUV420P,  15,     "YUV 4:2:0 Planar")
	OPTION(YUV410P,  16,     "YUV 4:1:0 Planar")
};

/* **************************************************************** */

#define TAB "    "

#define WH(_field_,_spec_) \
	whine(TAB "%s: " _spec_, #_field_, $->_field_);

#define WHYX(_name_,_fieldy_,_fieldx_) \
	whine(TAB "%s: y=%d, x=%d", #_name_, $->_fieldy_, $->_fieldx_);

char *multichoice_to_s(int value, int n, const char **table) {
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

char *video_type_to_s(int type) {
	return multichoice_to_s(type,ARRAY(video_type_flags));
}

char *tuner_flags_to_s(int flags) {
	return multichoice_to_s(flags,ARRAY(tuner_flags));
}

char *channel_flags_to_s(int flags) {
	return multichoice_to_s(flags,ARRAY(channel_flags));
}

const char *video_palette_s(int palette) {
	if (palette < 0 || palette >= COUNT(video_palette_options)) {
		return "(Unknown)";
	} else {
		return video_palette_options[palette];
	}
}

void video_channel_whine(struct video_channel *$) {
	char *foo;
	whine("VideoChannel:");
	WH(channel,"%d");
	WH(name,"%-32s");
	WH(tuners,"%d");
	whine(TAB "flags: %s", foo=channel_flags_to_s($->flags)); free(foo);
//	WH(flags,"0x%08x");
	WH(type,"0x%04x");
	WH(norm,"%d");
}

void video_tuner_whine(struct video_tuner *$) {
	char *foo;
	whine("VideoTuner:");
	WH(tuner,"%d");
	WH(name,"%-32s");
	WH(rangelow,"%u");
	WH(rangehigh,"%u");
	whine(TAB "flags: %s", foo=tuner_flags_to_s($->flags)); free(foo);
//	WH(flags,"0x%08x");
	WH(mode,"%d");
	WH(signal,"%d");
}

void video_capability_whine(struct video_capability *$) {
	char *foo;
	whine("VideoCapability:");
	WH(name,"%-20s");
	whine(TAB "type: %s", foo=video_type_to_s($->type)); free(foo);
	WH(channels,"%d");
	WH(audios,"%d");
	WHYX(maxsize,maxheight,maxwidth);
	WHYX(minsize,minheight,minwidth);
}

void video_window_whine(struct video_window *$) {
	whine("VideoWindow:");
	WHYX(pos,y,x);
	WHYX(size,height,width);
	WH(chromakey,"0x%08x");
	WH(flags,"0x%08x");
	WH(clipcount,"%d");
}

void video_picture_whine(struct video_picture *$) {
	whine("VideoPicture:");
	WH(brightness,"%d");
	WH(hue,"%d");
	WH(contrast,"%d");
	WH(whiteness,"%d");
	WH(depth,"%d");
	whine(TAB "palette: %s", video_palette_s($->palette));
}

void video_mbuf_whine(struct video_mbuf *$) {
	whine("VideoMBuf:");
	WH(size,"%d");
	WH(frames,"%d");
	WH(offsets[0],"%d");
	WH(offsets[1],"%d");
	WH(offsets[2],"%d");
	WH(offsets[3],"%d");
}

void video_mmap_whine(struct video_mmap *$) {
	whine("VideoMBuf:");
	WH(frame,"%u");
	WHYX(size,height,width);
	whine(TAB "format: %s", video_palette_s($->format));
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
	struct video_window grab_win;
	$->dim = Dim_new(3,v);

	WIOCTL($->stream_raw, VIDIOCGWIN, &grab_win);
	video_window_whine(&grab_win);
	grab_win.clipcount = 0;
	grab_win.flags = 0;
	grab_win.height = height;
	grab_win.width  = width;
	video_window_whine(&grab_win);
	WIOCTL($->stream_raw, VIDIOCSWIN, &grab_win);
	WIOCTL($->stream_raw, VIDIOCGWIN, &grab_win);
	video_window_whine(&grab_win);
}

/* picture is read at once by frame() to facilitate debugging. */
Dim *FormatVideoDev_frame_by_read (FileFormat *$, int frame) {

	int n;
	if (frame != -1) return 0;
	$->left = Dim_prod($->dim);

	whine("will read %d bytes", $->left);

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
	struct video_mbuf vid_buf;
	struct video_mmap vid_mmap;
	void *buffer;

	int n;
	if (frame != -1) return 0;
	$->left = Dim_prod($->dim);

	whine("will read %d bytes", $->left);

	if ($->stuff) free($->stuff);
	$->stuff = NEW2(uint8,$->left);

	if (WIOCTL($->stream_raw, VIDIOCGMBUF, &vid_buf)) goto err1;
/*	video_mbuf_whine(&vid_buf); */

	buffer = mmap(0,vid_buf.size,
		PROT_READ|PROT_WRITE,MAP_SHARED,$->stream_raw,0);
	if (((int)buffer)==-1) {
		whine("mmap: %s", strerror(errno));
		goto err1;
	}
	vid_mmap.frame = 0;
	vid_mmap.format = VIDEO_PALETTE_RGB24;
	vid_mmap.width  = Dim_get($->dim,1);
	vid_mmap.height = Dim_get($->dim,0);
	if (WIOCTL($->stream_raw, VIDIOCMCAPTURE, &vid_mmap)) goto err2;
	if (WIOCTL($->stream_raw, VIDIOCSYNC, &vid_mmap)) goto err2;

	/* success goes here */
	memcpy($->stuff,buffer,$->left);

err2:
	munmap (buffer, vid_buf.size);
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

void FormatVideoDev_option (FileFormat *$, fts_symbol_t sym, int value) {
	if (sym == SYM(channel)) {
		struct video_channel vchan;
		vchan.channel = value;
		if (0> ioctl($->stream_raw, VIDIOCGCHAN, &vchan)) {
			whine("no channel #%d", value);
		} else {
			video_channel_whine(&vchan);
			WIOCTL($->stream_raw, VIDIOCSCHAN, &vchan);
			FormatVideoDev_option($,SYM(tuner),0);
		}
	} else if (sym == SYM(tuner)) {
		struct video_tuner vtuner;
		vtuner.tuner = value;
		if (0> ioctl($->stream_raw, VIDIOCGTUNER, &vtuner)) {
			whine("no tuner #%d", value);
		} else {
			video_tuner_whine(&vtuner);
			vtuner.mode = VIDEO_MODE_NTSC;
			WIOCTL($->stream_raw, VIDIOCSTUNER, &vtuner);
		}
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
		struct video_capability vcaps;
		WIOCTL($->stream_raw, VIDIOCGCAP, &vcaps);
		video_capability_whine(&vcaps);
	/*	$->size($,vcaps.minheight,vcaps.minwidth); */
		$->size($,vcaps.maxheight,vcaps.maxwidth);
	}

	{
		struct video_picture *gp = NEW(struct video_picture,1);
		WIOCTL($->stream_raw, VIDIOCGPICT, gp);
		video_picture_whine(gp);
		gp->depth = 24;
		gp->palette = VIDEO_PALETTE_RGB24;
		video_picture_whine(gp);
		WIOCTL($->stream_raw, VIDIOCSPICT, gp);
		WIOCTL($->stream_raw, VIDIOCGPICT, gp);
		video_picture_whine(gp);
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

	$->option($,SYM(channel),0);

	/* Sometimes a pause is needed here */
	sleep(1);

	return $;
err:
	$->close($);
	return 0;
}

FileFormatClass FormatVideoDev = {
	"videodev", "Video4linux 1.x", 0,
	FormatVideoDev_open, 0, 0,
};

