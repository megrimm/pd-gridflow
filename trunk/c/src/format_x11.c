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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>

/*
	Note: sending images through shared memory doesn't work yet.
*/

#ifdef HAVE_X11_SHARED_MEMORY
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <X11/extensions/XShm.h>
#endif

extern FileFormatClass class_FormatX11;
typedef struct FormatX11 FormatX11;
typedef struct X11Display {
	fts_alarm_t *alarm;
	Display *display; /* connection to xserver */
	Visual *visual;   /* screen properties */
	Window root_window;
	int white_pixel;
	int black_pixel;
	int depth;
	int use_shm;	   /* should use shared memory? */
	int vouts_n;
	FormatX11 **vouts;
} X11Display;

struct FormatX11 {
	FileFormat_FIELDS;
	int autodraw;        /* how much to send to the display at once */
	X11Display *display; /* our own display struct (see above) */
	Window window;       /* X11 window number */
	GC imagegc;          /* X11 graphics context (like java.awt.Graphics) */
	XImage *ximage;      /* X11 image descriptor */
	char *name;          /* window name (for use by window manager) */
	uint8 *image;        /* the real data (that XImage binds to) */
	struct timeval tv;   /* time of the last grid_end */
	bool not_ours;
	int mode;
};

/* default connection (not used for now) */
static X11Display *x11 = 0;

/* ---------------------------------------------------------------- */

static void FormatX11_show_section(FormatX11 *$, int x, int y, int sx, int sy) {
	X11Display *d = $->display;
	#ifdef HAVE_X11_SHARED_MEMORY
	if ($->display->use_shm)
		XShmPutImage(d->display, $->window,
			$->imagegc, $->ximage, x, y, x, y, sx, sy, False);
	else
	#endif
		XPutImage(d->display, $->window,
			$->imagegc, $->ximage, x, y, x, y, sx, sy);

	XFlush(d->display);
}

/* window manager hints, defines the window as non-resizable */
static void FormatX11_set_wm_hints (FormatX11 *$, int sx, int sy) {
	XWMHints wmhints;
	XTextProperty window_name, icon_name;
	XSizeHints hints;

	/* enable recommended, maximum and minimum size */
	hints.flags=PSize|PMaxSize|PMinSize;

	/* set those */
	hints.min_width  = hints.max_width  = hints.width  = sx;
	hints.min_height = hints.max_height = hints.height = sy;
  
	wmhints.input = True;
	wmhints.flags = InputHint;

	XStringListToTextProperty((char **)&$->name, 1, &window_name);
	XStringListToTextProperty((char **)&$->name, 1, &icon_name);

	XSetWMProperties($->display->display, $->window,
		&window_name, &icon_name,
		NULL, 0, &hints, &wmhints, NULL);
}

/* ---------------------------------------------------------------- */

void X11Display_vout_add(X11Display *$, FormatX11 *vout) {
	FormatX11 **vouts = $->vouts;
	whine("adding [%p] to vout list", vout);
	$->vouts_n += 1;
	$->vouts = NEW2(FormatX11 *, $->vouts_n);
	memcpy($->vouts,vouts,$->vouts_n*sizeof(FormatX11 *));
	$->vouts[$->vouts_n-1] = vout;
	whine("vout count: %d", $->vouts_n);
}

void X11Display_vout_remove(X11Display *$, FormatX11 *vout) {
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

FormatX11 *X11Display_vout_find(X11Display *$, Window wid) {
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

void X11Display_set_alarm(X11Display *$);

void X11Display_alarm(fts_alarm_t *foo, void *obj) {
	X11Display *$ = (X11Display *)obj;
	XEvent e;
	int xpending;
	
	while (1) {
		int xpending = XEventsQueued($->display, QueuedAfterReading);
		if (!xpending) break;
		XNextEvent($->display,&e);
		switch (e.type) {
		case Expose:{
			XExposeEvent *ex = (XExposeEvent *)&e;
			FormatX11 *vout;
			/*whine("ExposeEvent at (y=%d,x=%d) size (y=%d,x=%d)",
				ex->y,ex->x,ex->height,ex->width);*/
			vout = X11Display_vout_find($,ex->window);
			if (vout && vout->mode == 2) {
				FormatX11_show_section(vout, ex->x, ex->y, ex->width,
				ex->height);
			}
		}break;
		case ButtonPress:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			whine("button %d press at (y=%d,x=%d)",eb->button,eb->y,eb->x);
		}break;
		case ButtonRelease:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			whine("button %d release at (y=%d,x=%d)",eb->button,eb->y,eb->x);
		}break;
		case MotionNotify:{
			XMotionEvent *em = (XMotionEvent *)&e;
			whine("drag at (y=%d,x=%d)",em->y,em->x);
		}break;
		default:
			whine("received event of type # %d", e.type);
		}
	}
	X11Display_set_alarm($);
}

void X11Display_set_alarm(X11Display *$) {
	if (!$->alarm) {
		fts_clock_t *clock = fts_sched_get_clock();
		$->alarm = fts_alarm_new(clock, X11Display_alarm, $);
	}
	fts_alarm_set_delay($->alarm, 125.0);
	fts_alarm_arm($->alarm);
}

/*
void X11Display_abort(Display *d, XErrorEvent *e) {
	kill(getpid(),4);
}
*/

X11Display *X11Display_new(const char *disp_string) {
	X11Display *$ = NEW(X11Display,1);
	
	int i;
	int screen_num;
	Screen *screen;

	$->alarm = 0;
	$->vouts = NEW2(FormatX11 *, 1);
	$->vouts_n = 0;

	/* Open an X11 connection */
	$->display = XOpenDisplay(disp_string);
	if(!$->display) {
		whine("ERROR: opening X11 display");
		goto err;
	}

	/* XSetErrorHandler(X11Display_abort); */

	screen      = DefaultScreenOfDisplay($->display);
	screen_num  = DefaultScreen($->display);
	$->visual      = DefaultVisual($->display, screen_num);
	$->white_pixel = XWhitePixel($->display,screen_num);
	$->black_pixel = XBlackPixel($->display,screen_num);
	$->root_window = DefaultRootWindow($->display);
	$->depth       = DefaultDepthOfScreen(screen);

	whine("depth = %d",$->depth);

	/* !@#$ check for visual type instead */
	if ($->depth != 16 && $->depth != 24 && $->depth != 32) {
		whine("ERROR: depth %d not supported.", $->depth);
		goto err;
	}

	$->use_shm = 0;
	#ifdef HAVE_X11_SHARED_MEMORY
		$->use_shm = 1;
	#endif

	X11Display_set_alarm($);
	return $;
err:;
	return 0;
}

/* ---------------------------------------------------------------- */

bool FormatX11_frame (FormatX11 *$, GridOutlet *out, int frame) {
	if (frame != -1) return 0;

	whine("$->dim = %s",Dim_to_s($->dim));

	XGetSubImage($->display->display, $->window,
		0, 0, Dim_get($->dim,1), Dim_get($->dim,0),
		-1, ZPixmap, $->ximage, 0, 0);

	GridOutlet_begin(out,Dim_dup($->dim));

	{
		int sy = Dim_get($->dim,0);
		int sx = Dim_get($->dim,1);
		int y;
		int bs = Dim_prod_start($->dim,1);
		Number b2[bs];
		for(y=0; y<sy; y++) {
			uint8 *b1 = $->image + $->ximage->bytes_per_line * y;
			BitPacking_unpack($->bit_packing,sx,b1,b2);
			GridOutlet_send(out,bs,b2);
		}
	}

	GridOutlet_end(out);

	return true;
}

/* ---------------------------------------------------------------- */

void FormatX11_dealloc_image (FormatX11 *$) {
	if (!$->ximage) return;
	if ($->display->use_shm) {
	#ifdef HAVE_X11_SHARED_MEMORY
		/* ??? */
	#endif	
	} else {
		XDestroyImage($->ximage);
		$->ximage = 0; 
		/* FREE($->image); */
	}
}

void FormatX11_alloc_image (FormatX11 *$, int sx, int sy) {
	X11Display *d = $->display;
	FormatX11_dealloc_image($);
	if (d->use_shm) {
	#ifdef HAVE_X11_SHARED_MEMORY
		XShmSegmentInfo shm_info;
		$->ximage = XShmCreateImage(d->display, d->visual,
			d->depth, ZPixmap, 0, &shm_info, sx, sy);
		assert($->ximage);
		shm_info.shmid = shmget(
			IPC_PRIVATE,
			$->ximage->bytes_per_line * $->ximage->height,
			IPC_CREAT|0777);
		if(shm_info.shmid < 0) {
			perror("shmget failed:");
			assert(0);
		}
		$->ximage->data = shm_info.shmaddr =
			(char *) shmat(shm_info.shmid, 0, 0);
		$->image = (uint8 *) $->ximage->data;
		XShmAttach(d->display, &shm_info);
		XSync(d->display,0);
		shmctl(shm_info.shmid,IPC_RMID,0);
	#endif
	} else {
		/* this buffer may be too big, but at least it won't be too small */
		$->image = (uint8 *)calloc(sx * sy, sizeof(int));

		$->ximage = XCreateImage(d->display, d->visual,
			d->depth, ZPixmap, 0, (int8 *) $->image,
			sx, sy, 8, 0);
	}
	{
		int status = XInitImage($->ximage);
		whine("XInitImage returned: %d", status);
	}
}

void FormatX11_resize_window (FormatX11 *$, int sx, int sy) {
	X11Display *d = $->display;
	int v[3] = {sy, sx, 3};
	Window oldw;

	FREE($->dim);
	$->dim = Dim_new(3,v);

/* ximage */

	FormatX11_alloc_image($,sx,sy);

/* window */
	
	$->name = strdup("Video4jmax");

	oldw = $->window;
	if (oldw) {
		if (!$->not_ours) {
			whine("About to resize window: %s",Dim_to_s($->dim));
			XResizeWindow(d->display,$->window,sx,sy);
		}
	} else {
		whine("About to create window: %s",Dim_to_s($->dim));
		$->window = XCreateSimpleWindow(d->display,
			d->root_window, 0, 0, sx, sy, 0,
			d->white_pixel, d->black_pixel);
		if(!$->window) {
			whine("can't create window");
			goto err;
		}
	}

	/* FormatX11_set_wm_hints($,sx,sy); */

	$->imagegc = XCreateGC(d->display, $->window, 0, NULL);
	if ($->not_ours) {
		XSelectInput(d->display, $->window,
			ExposureMask | StructureNotifyMask);
	} else {
		XSelectInput(d->display, $->window,
			ExposureMask | StructureNotifyMask |
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
	}

	if (!$->not_ours) {
		XMapRaised(d->display, $->window);
	}
	XSync(d->display,0);
	return;
err:
	return;
}

GRID_BEGIN(FormatX11,0) {
	int sxc = Dim_prod_start(in->dim,1);
	int sx = Dim_get(in->dim,1), osx = Dim_get($->dim,1);
	int sy = Dim_get(in->dim,0), osy = Dim_get($->dim,0);
	GridInlet_set_factor(in,sxc);
	if (Dim_count(in->dim) != 3) {
		whine("expecting 3 dimensions: rows,columns,channels");
		return false;
	} else if (Dim_get(in->dim,2) != 3) {
		whine("expecting 3 channels: red,green,blue");
		return false;
	}
	if (sx != osx || sy != osy) FormatX11_resize_window($,sx,sy);
	return true;
}

GRID_FLOW(FormatX11,0) {
	int bypl = $->ximage->bytes_per_line;
	int sxc = Dim_prod_start(in->dim,1);
	int sx = Dim_get(in->dim,1);
	int y = in->dex / sxc;

	assert((in->dex % sxc) == 0);
	assert((n       % sxc) == 0);

	while (n>0) {
		/* whine("bypl=%d sxc=%d sx=%d y=%d n=%d",bypl,sxc,sx,y,n); */
		/* convert line */
		BitPacking_pack($->bit_packing, sx, data, &$->image[y*bypl]);
		if ($->autodraw==2) FormatX11_show_section($,0,y,sx,1);
		y++;
		data += sxc;
		n -= sxc;
	}
}

GRID_END(FormatX11,0) {
	if ($->autodraw==1) {
		int sx = Dim_get(in->dim,1);
		int sy = Dim_get(in->dim,0);
		FormatX11_show_section($,0,0,sx,sy);
	}
/*	whine_time("FormatX11:0:end"); */
	{
		struct timeval t;
		gettimeofday(&t,0);
		whine("x11:0:end: %d.%06d (diff %8d usec)\n",t.tv_sec,t.tv_usec,
			(t.tv_sec-$->tv.tv_sec)*1000000 + (t.tv_usec-$->tv.tv_usec));
		memcpy(&$->tv,&t,sizeof(struct timeval));
	}
}

void FormatX11_close (FormatX11 *$) {
	X11Display *d = $->display;
	X11Display_vout_remove(d,$);
	if (!$->not_ours) XDestroyWindow(d->display,$->window);
	XSync(d->display,0);
	FormatX11_dealloc_image($);
	FREE($->dim);
	FREE($);
	/* X11-socket leakage going on here */
}

/*
void FormatX11_size (FormatX11 *$, int sy, int sx) {
	int v[] = {sy,sx,3};
	$->dim = Dim_new(ARRAY(v));
}
*/

void FormatX11_option (FormatX11 *$, int ac, const fts_atom_t *at) {
	fts_symbol_t sym = GET(0,symbol,SYM(foo));
	if (sym == SYM(out_size)) {
		int sy = GET(1,int,0);
		int sx = GET(2,int,0);
		COERCE_INT_INTO_RANGE(sy,16,MAX_INDICES);
		COERCE_INT_INTO_RANGE(sx,16,MAX_INDICES);
		FormatX11_resize_window($,sx,sy);
	} else if (sym == SYM(draw)) {
		int sy = Dim_get($->dim,0);
		int sx = Dim_get($->dim,1);
		FormatX11_show_section($,0,0,sx,sy);
	} else if (sym == SYM(autodraw)) {
		$->autodraw = GET(1,int,0);
		COERCE_INT_INTO_RANGE($->autodraw,0,2);
		whine("autodraw = %d",$->autodraw);
	} else {
		whine("unknown option: %s", fts_symbol_name(sym));
	}
}

int strsplit(char *victim, int max, char **witnesses) {
	int n=0;
	for(;;) {
		*witnesses++ = victim;
		n++;
		if (n==max) return n;
		while (*victim != '/') {
			if (!*victim) return n;
			victim++;
		}
		*victim++ = 0;
	}
}

FileFormat *FormatX11_connect (FileFormatClass *qlass, const char *target, int mode) {
	FormatX11 *$ = NEW(FormatX11,1);

	/* defaults */
	int sy = 240;
	int sx = 320;

	$->cl     = &class_FormatX11;
	$->stream  = 0;
	$->bstream = 0;
	$->window  = 0;
	$->not_ours= false;
	$->image   = 0;
	$->ximage  = 0;
	$->autodraw= 1;
	$->mode = mode;
	$->dim = 0;

	gettimeofday(&$->tv,0);

	switch(mode) {
	case 4: case 2: break;
	default: whine("unsupported mode (#%d)", mode); goto err;
	}

	/*
		"here"
		"local/0"
		"remote/relayer/0"
	*/

	/* plenty of ways to crash this */
	{
		char *copy = strdup(target);
		char *field[10];
		int i=0,n=strsplit(copy,10,field);
		char *t = copy+strlen(copy);
		while (n<10) field[n++] = t;
/*
		{int j; for(j=0; j<10; j++) {
			whine("field[%d] is `%s'",j,field[j]);
		}}
*/
		if (strcmp(field[0],"here")==0) {
			whine("mode `here'");
			$->display = X11Display_new(0);
			i=1;
		} else if (strcmp(field[0],"local")==0) {
			char host[64];
			int dispnum = atoi(field[1]);
			whine("mode `local'");
			whine("display_number `%d'",dispnum);
			sprintf(host,":%d",dispnum);
			$->display = X11Display_new(host);
			i=2;
		} else if (strcmp(field[0],"remote")==0) {
			char host[64];
			int dispnum = 0;
			strcpy(host,field[1]);
			dispnum = atoi(field[2]);
			sprintf(host+strlen(host),":%d",dispnum);
			whine("mode `remote'");
			whine("host `%s'",host);
			whine("display_number `%d'",dispnum);
			$->display = X11Display_new(host);
			i=3;
		} else {
			whine("error in target `%s'",target);
			return 0;
		}

		if (!$->display) {
			// ???
			whine("can't open target `%s': %s", target, strerror(errno));
			goto err;
		}

		if (strcmp(field[i],"root")==0) {
			$->window = $->display->root_window;
			whine("will use root window (0x%x)", $->window);
			$->not_ours = true;
		} else {
			if (strncmp(field[i],"0x",2)==0) {
				$->window = strtol(field[i]+2,0,16);
			} else {
				$->window = atoi(field[i]);
			}
			if ($->window) {
				$->not_ours = true;
				whine("will use specified window (0x%x)", $->window);
			} else {
				whine("will create new window");
			}
		}
		i++;
	}

	FormatX11_resize_window($,sx,sy);
	X11Display_vout_add($->display,$);

	{
		Visual *v = $->display->visual;
		$->bit_packing = BitPacking_new(
			$->ximage->bits_per_pixel/8,
			v->red_mask,
			v->green_mask,
			v->blue_mask);
	}

	BitPacking_whine($->bit_packing);

	return (FileFormat *)$;
err:;
	$->cl->close((FileFormat *)$);
	return 0;
}

FileFormatClass class_FormatX11 = {
	symbol_name: "x11",
	long_name: "X Window System Version 11.5",
	flags: (FileFormatFlags)0,

	open: 0,
	connect: FormatX11_connect,
	chain_to: 0,

	frames: 0,
	frame:  FormatX11_frame,

	begin:  GRID_BEGIN_PTR(FormatX11,0),
	flow:    GRID_FLOW_PTR(FormatX11,0),
	end:      GRID_END_PTR(FormatX11,0),

//	size:   FormatX11_size,
	size:   0,
	color:  0,
	option: FormatX11_option,
	close:  FormatX11_close,
};
