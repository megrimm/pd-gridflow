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

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>

#include "grid.h"

/*
	Note: sending images through shared memory doesn't work yet.
*/

#ifdef VIDEO_OUT_SHM
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <X11/extensions/XShm.h>
#endif


/* ---------------------------------------------------------------- */

typedef struct VideoDisplay VideoDisplay;
typedef struct VideoOut VideoOut;

struct VideoDisplay {
	fts_alarm_t *alarm;
	Display *display; /* connection to xserver */
	Visual *visual;   /* screen properties */
	int root_window;
	int white_pixel; 
	int black_pixel;
	int depth;
	int bytes;
	int big_error; //!@#$
	int low_bit[3];
	int high_bit[3];
	int mask[3];
	int use_shm;	   /* should use shared memory? */
	VideoOut **vouts;
	int vouts_n;
};

/* support for only one x11 connection at the moment */
static VideoDisplay x11;

struct VideoOut {
	GridObject_FIELDS;

/* fields for: general purpose */

	Dim *dim;     /* what the constructor said we would get */
	Number *buf;     /* buffer for values of the next line */
	int bufn;     /* buffer portion used */
	int autodraw; /* how much to actually send to the display at once */

/* fields for: x window system */

	VideoDisplay *display; /* our own display struct (see above) */
	uint window;       /* window number */
	GC imagegc;       /* graphics context (like java.awt.Graphics) */
	XImage *ximage;   /* image descriptor */
	char *name;       /* window name (for use by window manager) */
	uint8 *image;     /* the real data (that XImage binds to) */
};

static void VideoOut_show_section(VideoOut *$, int x, int y, int sx, int sy);

/* ---------------------------------------------------------------- */

void display_vout_add(VideoDisplay *$, VideoOut *vout) {
	VideoOut **vouts = $->vouts;
	whine("adding [%p] to vout list", vout);
	$->vouts_n += 1;
	$->vouts = NEW2(VideoOut *, $->vouts_n);
	memcpy($->vouts,vouts,$->vouts_n*sizeof(VideoOut *));
	$->vouts[$->vouts_n-1] = vout;
	whine("vout count: %d", $->vouts_n);
}

void display_vout_remove(VideoDisplay *$, VideoOut *vout) {
	int i;
	whine("looking for vout [%p]", vout);
	for (i=0; i<$->vouts_n; i++) {
		if ($->vouts[i] != vout) continue;
		whine("removing [%p] from vout list (index %d)", vout, i);
		$->vouts[i] = $->vouts[$->vouts_n-1];
		$->vouts_n--;
		whine("vout count: %d", $->vouts_n);
		return;
	}
}

VideoOut *display_vout_find(VideoDisplay *$, Window wid) {
	int i;
	/*whine("looking for vout that has ->window == %ld", wid);*/
	for (i=0; i<$->vouts_n; i++) {
		if ($->vouts[i]->window != wid) continue;
		/*whine("found vout [%p] at index %d", $->vouts[i], i);*/
		return $->vouts[i];
	}
	whine("vout not found!");
	return 0;
}

void display_set_alarm(VideoDisplay *$);

void display_alarm(fts_alarm_t *foo, void *obj) {
	VideoDisplay *$ = (VideoDisplay *)obj;
	XEvent e;
	int xpending;
	
	while (1) {
		int xpending = XEventsQueued($->display, QueuedAfterReading);
		if (!xpending) break;
		XNextEvent($->display,&e);
		if (e.type == Expose) {
			XExposeEvent *ex = (XExposeEvent *)&e;
			VideoOut *vout;
			/*whine("ExposeEvent at (y=%d,x=%d) size (y=%d,x=%d)",
				ex->y,ex->x,ex->height,ex->width);*/
			vout = display_vout_find($,ex->window);
			if (vout) {
				VideoOut_show_section(vout, ex->x, ex->y, ex->width,
				ex->height);
			}
			
		} else {
			whine("received event of type # %d", e.type);
		}
	}
	display_set_alarm($);
}

void display_set_alarm(VideoDisplay *$) {
	if (!$->alarm) {
		fts_clock_t *clock = fts_sched_get_clock();
		$->alarm = fts_alarm_new(clock, display_alarm, $);
	}
	fts_alarm_set_delay($->alarm, 500.0);
	fts_alarm_arm($->alarm);
}

void display_init(VideoDisplay *$) {
	int i;
	int screen_num;
	Screen *screen;

	$->alarm = 0;
	$->vouts = NEW2(VideoOut *, 1);
	$->vouts_n = 0;

	/* Open an X11 connection */
	$->display = XOpenDisplay(0);
	if(!$->display) {
		whine("ERROR: opening X11 display");
		$->big_error=1; return;
	}
	screen      = DefaultScreenOfDisplay($->display);
	screen_num  = DefaultScreen($->display);
	$->visual      = DefaultVisual($->display, screen_num);
	$->white_pixel = XWhitePixel($->display,screen_num);
	$->black_pixel = XBlackPixel($->display,screen_num);
	$->root_window = DefaultRootWindow($->display);
	$->depth       = DefaultDepthOfScreen(screen);

	whine("depth = %d",$->depth);

	if ($->depth != 16 && $->depth != 24 && $->depth != 32) {
		whine("ERROR: depth %d not supported.", $->depth);
		$->big_error=1; return;
	}

	$->mask[0] = $->visual->red_mask;
	$->mask[1] = $->visual->green_mask;
	$->mask[2] = $->visual->blue_mask;

	for(i=0;i<3;i++) {
		$->low_bit[i] = low_bit ($->mask[i]);
		$->high_bit[i] = high_bit($->mask[i]);
	}

	for (i=0;i<3;i++) {
		static const char *colour_name[] = {"red","green","blue"};
		whine("mask[%5s] = %08x (bits from %2d up to %2d)",
			colour_name[i],
			$->mask[i],
			$->low_bit[i],
			$->high_bit[i]);
	}
	#ifdef VIDEO_OUT_SHM
		$->use_shm = 1;
	#else
		$->use_shm = 0;
	#endif

	display_set_alarm($);
}

/* ---------------------------------------------------------------- */

static void VideoOut_show_section(
	VideoOut *$, int x, int y, int sx, int sy
) {
	#ifdef VIDEO_OUT_SHM
	if ($->display->use_shm)
		XShmPutImage($->display->display, $->window,
			$->imagegc, $->ximage, x, y, x, y, sx, sy, False);
	else
	#endif
		XPutImage($->display->display, $->window,
			$->imagegc, $->ximage, x, y, x, y, sx, sy);

	XFlush($->display->display);
}

GRID_BEGIN(VideoOut,0) {
	VideoOut *parent = (VideoOut *) GridInlet_parent($);

	parent->bufn = 0;

	if (!Dim_equal_verbose_hwc($->dim,parent->dim)) {
		GridInlet_abort($);
	}
}

/*
  This piece of code is non-portable.
  Will fix it when I get hardware to test it.
*/
static uint8 *VideoOut_convert(
	GridInlet *$, int n, Number *data, uint8 *target
) {
	VideoOut *parent = (VideoOut *) GridInlet_parent($);
	VideoDisplay *d = parent->display;
	while (n--) {
		unsigned int temp =
			(((data[0] << d->high_bit[0]) >> 7) & d->mask[0]) |
			(((data[1] << d->high_bit[1]) >> 7) & d->mask[1]) |
			(((data[2] << d->high_bit[2]) >> 7) & d->mask[2]);
		//int bytes = parent->display->depth/8;
		int bytes = parent->ximage->bits_per_pixel/8;

		while(bytes--) {
			*target++ = temp;
			temp >>= 8;
		}
		data += 3;
	}
	return target;
}

GRID_FLOW(VideoOut,0) {
	VideoOut *parent = (VideoOut *) GridInlet_parent($);

	int bytes_per_pixel = parent->ximage->bits_per_pixel/8;
	int linesize = Dim_get($->dim,1) * 3;

	while (n>0) {
		int pixel_num = $->dex / 3;
		int sx = Dim_get($->dim,1);
		int line_num = pixel_num / sx;
		int incr, on=n;

		int size = linesize - parent->bufn;
		if (size > n) size = n;
		memcpy(&parent->buf[parent->bufn],data,sizeof(int)*size);
		parent->bufn += size;
		data += size;
		n -= size;
		if (parent->bufn<linesize) break;

		/* convert line */
		VideoOut_convert($,sx,parent->buf,
			&parent->image[line_num * sx * bytes_per_pixel]);
		parent->bufn = 0;
		if (parent->autodraw==2) {
				VideoOut_show_section(parent,0,line_num,sx,1);
		}
		$->dex += linesize;
		if ($->dex >= Dim_prod($->dim)) {
			if (parent->autodraw==1) {
				int sy = Dim_get($->dim,0);
				VideoOut_show_section(parent,0,0,sx,sy);
			}
			/*
			struct timeval foo;
			gettimeofday(&foo,0);
			whine("shown image after: %d.%.06d",foo.tv_sec,foo.tv_usec);
			*/
			fts_outlet_send(OBJ(parent),0,fts_s_bang,0,0);
		}
	}
}

/* ---------------------------------------------------------------- */

/* window manager hints, defines the window as non-resizable */
static void VideoOut_set_wm_hints (VideoOut *$) { 
	XWMHints wmhints;
	XTextProperty window_name, icon_name;
	XSizeHints hints;

	/* enable recommended, maximum and minimum size */
	hints.flags=PSize|PMaxSize|PMinSize;

	/* set those */
	hints.min_width  = hints.max_width  = hints.width  = Dim_get($->dim,1);
	hints.min_height = hints.max_height = hints.height = Dim_get($->dim,0);
  
	wmhints.input = True;
	wmhints.flags = InputHint;

	XStringListToTextProperty((char **)&$->name, 1, &window_name);
	XStringListToTextProperty((char **)&$->name, 1, &icon_name);

	XSetWMProperties($->display->display, $->window,
		&window_name, &icon_name,
		NULL, 0, &hints, &wmhints, NULL);
}

METHOD(VideoOut,init) {
	XEvent event;
	VideoDisplay *d = $->display = &x11;
	int height = GET(1,int,0);
	int width  = GET(2,int,0);

	GridObject_init((GridObject *)$,winlet,selector,ac,at);
	$->in[0] = GridInlet_new((GridObject *)$, 0,
		VideoOut_0_begin, VideoOut_0_flow);

	COERCE_INT_INTO_RANGE(height,1,MAX_INDICES);
	COERCE_INT_INTO_RANGE(width, 1,MAX_INDICES);

	$->autodraw = 2;
	$->buf = NEW(Number,width*3);
	$->bufn = 0;

	{
		int v[3] = { height, width, 3 };
		$->dim = Dim_new(3,v);
	}

	#ifdef VIDEO_OUT_SHM
	if ($->display->use_shm) {
		XShmSegmentInfo shm_info;
		$->ximage = XShmCreateImage(d->display, d->visual,
			d->depth, ZPixmap, 0, &shm_info,
			Dim_get($->dim,1), Dim_get($->dim,0));
		assert($->ximage);
		shm_info.shmid = shmget(
			IPC_PRIVATE,
			$->ximage->bytes_per_line*$->ximage->height,
			IPC_CREAT|0777);
		if(shm_info.shmid < 0) {
			perror("shmget failed:");
			assert(0);
		}
		$->ximage->data = shm_info.shmaddr =
			(char *) shmat(shm_info.shmid, 0, 0);
		$->image = (uint8 *) $->ximage->data;
		XShmAttach(d->display, &shm_info);
		XSync(d->display, 0);
		shmctl(shm_info.shmid,IPC_RMID,0);
	} else {
	#endif
		/* this buffer may be too big, but at least it won't be too small */
		$->image = calloc(
			Dim_get($->dim,1) * Dim_get($->dim,0), sizeof(int));

		$->ximage = XCreateImage(d->display, d->visual,
			d->depth, ZPixmap, 0, $->image,
			Dim_get($->dim,1), Dim_get($->dim,0),
			8, 0);

	#ifdef VIDEO_OUT_SHM
	}
	#endif

	{
		int status = XInitImage($->ximage);
		whine("XInitImage returned: %d", status);
	}

	$->name = strdup("Video4jmax");

	whine("About to create window: %s",Dim_to_s($->dim));

	$->window = XCreateSimpleWindow($->display->display,
		d->root_window, 0, 0,
		Dim_get($->dim,1), Dim_get($->dim,0), 0,
		d->white_pixel,
		d->black_pixel);

	if(!$->window) {
		whine("ERROR: can't create window");
		d->big_error = 1;
		return;
	}

	$->imagegc = XCreateGC(d->display, $->window, 0, NULL);

	VideoOut_set_wm_hints($);

	XSelectInput(d->display, $->window, ExposureMask);
	XMapRaised(d->display, $->window);
	XNextEvent(d->display, &event);

	XSync(d->display,0);

	display_vout_add($->display, $);
}

METHOD(VideoOut,delete) {
	display_vout_remove($->display, $);
	XDestroyWindow($->display->display,$->window);
}

METHOD(VideoOut,bang) {
	VideoOut_show_section($,0,0,
		Dim_get($->dim,1),
		Dim_get($->dim,0));
}

METHOD(VideoOut,autodraw) {
	$->autodraw = GET(0,int,0); /* why 0 and not 1 ? */
	COERCE_INT_INTO_RANGE($->autodraw,0,2);
	whine("autodraw = %d",$->autodraw);
}

/* ---------------------------------------------------------------- */

CLASS(VideoOut) {
	fts_type_t int_alone[]  = { fts_t_int };
	fts_type_t one_int[]    = { fts_t_symbol, fts_t_int };
	fts_type_t init_args[]  = { fts_t_symbol, fts_t_int, fts_t_int };

	MethodDecl methods[] = {
		{-1,fts_s_init,  METHOD_PTR(VideoOut,init), ARRAY(init_args),-1},
		{-1,fts_s_delete,METHOD_PTR(VideoOut,delete), 0,0,0 },
		{ 0,fts_s_bang,  METHOD_PTR(VideoOut,bang), 0,0,0 },
		{ 0,sym_autodraw,METHOD_PTR(VideoOut,autodraw),ARRAY(one_int),-1 },
	};

	/* initialize the class */
	fts_class_init(class, sizeof(VideoOut), 1, 1, 0);
	define_many_methods(class,ARRAY(methods));
	GridObject_conf_class(class,0);

	return fts_Success;
}

void VideoOut_config (void) {
	display_init(&x11);
	fts_class_install(fts_new_symbol("@video_out"), VideoOut_class_init);
}

