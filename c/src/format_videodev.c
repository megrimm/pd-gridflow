/*
	Video4jmax
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file LICENSE for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "grid.h"
#include <stdlib.h>
#include <unistd.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

void video_picture_debug(struct video_picture *$) {
	whine("********************");
	whine("brightness: %d",$->brightness);
	whine("hue:        %d",$->hue);
	whine("contrast:   %d",$->contrast);
	whine("whiteness:  %d",$->whiteness);
	whine("depth:      %d",$->depth);
	whine("palette:    %d",$->palette);
}

/* **************************************************************** */

extern FileFormatClass FormatVideoDev;

void FormatVideoDev_size (FileFormat *$, int height, int width) {
	int v[] = { height, width, 3 };
	struct video_window grab_win;
	struct video_picture gp;
	$->dim = Dim_new(3,v);

	if (-1 == ioctl($->stream_raw, VIDIOCGWIN, &grab_win)) {
		perror("ioctl VIDIOCGWIN");
	}
	grab_win.clipcount = 0;
	grab_win.height = height;
	grab_win.width  = width;
	if (-1 == ioctl($->stream_raw, VIDIOCSWIN, &grab_win)) {
		perror("ioctl VIDIOCSWIN");
	}

	whine("actual rows=%d, columns=%d",
		grab_win.height,
		grab_win.width);

	if (0> ioctl($->stream_raw, VIDIOCGPICT, &gp)) {
		perror("error getting video_picture info");
	}

	video_picture_debug(&gp);
	gp.depth = 24;
	gp.palette = VIDEO_PALETTE_RGB24;
	video_picture_debug(&gp);

	if (0> ioctl($->stream_raw, VIDIOCSPICT, &gp)) {
		perror("error setting video_picture info");
	}
	if (0> ioctl($->stream_raw, VIDIOCGPICT, &gp)) {
		perror("error getting video_picture info");
	}
	video_picture_debug(&gp);
}

Dim *FormatVideoDev_frame (FileFormat *$, int frame) {
	char buf[256];
	int metrics[6],n;
	if (frame != -1) return 0;
	$->left = Dim_prod($->dim);
	$->stuff = NEW2(uint8,$->left);
	n = (int) read($->stream_raw,$->stuff,$->left);
	if (0> n) {
		perror("error reading");
		whine("error reading");
	}

	if (n < $->left) {
		whine("unexpectedly short picture: %d of %d",n,$->left);
	}

	return $->dim;
}

Number *FormatVideoDev_read (FileFormat *$, int n) {
	int i;
	int bs = $->left;
	uint8 *b1 = (uint8 *)$->stuff + Dim_prod($->dim) - $->left;
	Number *b2 = NEW2(Number,n);

	if (!$->dim) return 0;
	for (i=0; i<n; i++) b2[i] = b1[i];
	$->left -= n;
	return b2;
}

void FormatVideoDev_acceptor (FileFormat *$, Dim *dim) {

}

void FormatVideoDev_processor (FileFormat *$, int n, const Number *data) {

}

void FormatVideoDev_close (FileFormat *$) {
	if ($->stream_raw>=0) close($->stream_raw);
	free($);
}

FileFormat *FormatVideoDev_open (const char *filename, int mode) {
	struct video_capability vcaps;
	FileFormat *$ = NEW(FileFormat,1);
	$->qlass     = &FormatVideoDev;
	$->frames    = 0;
	$->frame     = FormatVideoDev_frame;
	$->size      = FormatVideoDev_size;
	$->read      = FormatVideoDev_read;
	$->acceptor  = FormatVideoDev_acceptor;
	$->processor = FormatVideoDev_processor;
	$->close     = FormatVideoDev_close;

	whine("will try opening file");
	$->stream_raw = open(filename, O_RDONLY);
	if (0> $->stream_raw) {
		whine("can't open file: %s", filename);
		goto err;
	}

    if (0> ioctl($->stream_raw, VIDIOCGCAP, &vcaps)) {
        perror("wrong device");
        exit(1);
    }

	ioctl($->stream_raw, VIDIOCGCAP, &vcaps);
	whine("*** Card info *** ");
	whine("name: %-20s",vcaps.name);
	whine("type: %d",vcaps.type);
	whine("channels: %d",vcaps.channels);
	whine("audios: %d",vcaps.audios);
	whine("max size: %d rows, %d columns",vcaps.maxheight,vcaps.maxwidth);
	whine("min size: %d rows, %d columns",vcaps.minheight,vcaps.minwidth);
	whine("***************** ");

//	$->size($,vcaps.minheight,vcaps.minwidth);
	$->size($,vcaps.maxheight,vcaps.maxwidth);

	/* Sometimes a pause is needed here */
	sleep(1);

	return $;
err:
	$->close($);
	return 0;
}

FileFormatClass FormatVideoDev = {
	"videodev", "Video4linux", 0,
	FormatVideoDev_open, 0, 0,
};

