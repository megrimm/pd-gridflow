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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>

/* X11 Error Handler type */
typedef int (*XEH)(Display *, XErrorEvent *);

/*
	Note: sending images through shared memory doesn't work yet.
*/

#ifdef HAVE_X11_SHARED_MEMORY
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

typedef struct FormatX11 {
	Format_FIELDS;

/* at the Display/Screen level */
	Display *display; /* connection to xserver */
	Visual *visual;   /* screen properties */
	Window root_window;
	long black,white; /* color numbers in default palette */
	short depth;
	bool use_shm;	   /* should use shared memory? */

/* at the Window level */
	int autodraw;        /* how much to send to the display at once */
	Window window;       /* X11 window number */
	GC imagegc;          /* X11 graphics context (like java.awt.Graphics) */
	XImage *ximage;      /* X11 image descriptor */
	char *name;          /* window name (for use by window manager) */
	uint8 *image;        /* the real data (that XImage binds to) */
	bool is_owner;
#ifdef HAVE_X11_SHARED_MEMORY
	XShmSegmentInfo *shm_info; /* to share memory with X11/Unix */
#endif
} FormatX11;

/* ---------------------------------------------------------------- */

static void FormatX11_show_section(FormatX11 *$, int x, int y, int sx, int sy) {
	#ifdef HAVE_X11_SHARED_MEMORY
	if ($->use_shm) {
		XSync($->display,False);
		if (sy>Dim_get($->dim,0)) sy=Dim_get($->dim,0);
		if (sx>Dim_get($->dim,1)) sx=Dim_get($->dim,1);
//		whine("x,y,sx,sy = %d,%d,%d,%d",x,y,sx,sy);
		XShmPutImage($->display, $->window,
			$->imagegc, $->ximage, x, y, x, y, sx, sy, False);
		/* should completion events be waited for? looks like a bug */
	} else
	#endif
		XPutImage($->display, $->window,
			$->imagegc, $->ximage, x, y, x, y, sx, sy);

	XFlush($->display);
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

	XSetWMProperties($->display, $->window,
		&window_name, &icon_name,
		NULL, 0, &hints, &wmhints, NULL);
}

/* ---------------------------------------------------------------- */

static VALUE button_sym(int i) {
	char foo[42];
	sprintf(foo,"button%d",i);
	return SYM(foo);
}

static void FormatX11_alarm(FormatX11 *$) {
	VALUE argv[4];
	XEvent e;

//	whine("X11 HELLO? (%lld)",RtMetro_now());

	for (;;) {
		int xpending = XEventsQueued($->display, QueuedAfterReading);
		if (!xpending) break;
		XNextEvent($->display,&e);
		switch (e.type) {
		case Expose:{
			XExposeEvent *ex = (XExposeEvent *)&e;
			/*whine("ExposeEvent at (y=%d,x=%d) size (y=%d,x=%d)",
				ex->y,ex->x,ex->height,ex->width);*/
			if ($->mode == 2) {
				FormatX11_show_section($, ex->x, ex->y, ex->width,
				ex->height);
			}
		}break;
		case ButtonPress:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			//whine("button %d press at (y=%d,x=%d)",eb->button,eb->y,eb->x);
			argv[0] = SYM(press);
			argv[1] = button_sym(eb->button);
			argv[2] = INT2NUM(eb->y);
			argv[3] = INT2NUM(eb->x);
			//FObject_send_thru($->out->parent,0,4,at);
		}break;
		case ButtonRelease:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			//whine("button %d release at (y=%d,x=%d)",eb->button,eb->y,eb->x);
			argv[0] = SYM(release);
			argv[1] = button_sym(eb->button);
			argv[2] = INT2NUM(eb->y);
			argv[3] = INT2NUM(eb->x);
		}break;
		case MotionNotify:{
			XMotionEvent *em = (XMotionEvent *)&e;
			//whine("drag at (y=%d,x=%d)",em->y,em->x);
			argv[0] = SYM(motion);
			argv[1] = INT2NUM(em->y);
			argv[2] = INT2NUM(em->x);
		}break;
		case DestroyNotify:{
			/* should notify $->parent here */
		}break;
		case ConfigureNotify:{
			/* like we care */
		}break;
		default:
			whine("received event of type # %d", e.type);
		}
	}
}

/* ---------------------------------------------------------------- */

METHOD(FormatX11,frame) {
	{
		char *s = Dim_to_s($->dim);
		whine("$->dim = %s",s);
		FREE(s);
	}

	XGetSubImage($->display, $->window,
		0, 0, Dim_get($->dim,1), Dim_get($->dim,0),
		-1, ZPixmap, $->ximage, 0, 0);

	GridOutlet_begin($->out[0],Dim_dup($->dim));

	{
		int sy = Dim_get($->dim,0);
		int sx = Dim_get($->dim,1);
		int y;
		int bs = Dim_prod_start($->dim,1);
		Number b2[bs];
		for(y=0; y<sy; y++) {
			uint8 *b1 = $->image + $->ximage->bytes_per_line * y;
			BitPacking_unpack($->bit_packing,sx,b1,b2);
			GridOutlet_send($->out[0],bs,b2);
		}
	}

	GridOutlet_end($->out[0]);
}

/* ---------------------------------------------------------------- */

static void FormatX11_dealloc_image (FormatX11 *$) {
	if (!$->ximage) return;
	if ($->use_shm) {
	#ifdef HAVE_X11_SHARED_MEMORY
		FREE($->shm_info);
	#endif	
	} else {
		XDestroyImage($->ximage);
		$->ximage = 0; 
		/* FREE($->image); */
	}
}

/* loathe Xlib's error handlers */
static FormatX11 *current_x11;
static int FormatX11_error_handler (Display *d, XErrorEvent *xee) {
	current_x11->use_shm = false;
	whine("caught X11 error (should be \"BadAccess: can't find shm\")");
	return 42; /* it seems that the return value is ignored. */
}

static bool FormatX11_alloc_image (FormatX11 *$, int sx, int sy) {
top:
	FormatX11_dealloc_image($);
	#ifdef HAVE_X11_SHARED_MEMORY
	if ($->use_shm) {
		$->shm_info = NEW(XShmSegmentInfo,1);
		$->ximage = XShmCreateImage($->display, $->visual,
			$->depth, ZPixmap, 0, $->shm_info, sx, sy);
		assert($->ximage);
		$->shm_info->shmid = shmget(
			IPC_PRIVATE,
			$->ximage->bytes_per_line * $->ximage->height,
			IPC_CREAT|0777);
		if($->shm_info->shmid < 0) RAISE("ERROR: shmget failed: %s",strerror(errno));
		$->ximage->data = $->shm_info->shmaddr =
			(char *)shmat($->shm_info->shmid,0,0);
		$->image = (uint8 *)$->ximage->data;
		$->shm_info->readOnly = False;

		current_x11 = $;
		XSetErrorHandler(FormatX11_error_handler);
		if (!XShmAttach($->display, $->shm_info)) RAISE("ERROR: XShmAttach: big problem");

		/* make sure the server picks it up */
		XSync($->display,0);

		XSetErrorHandler(0);

		/* yes, this can be done now. should cause auto-cleanup. */
		shmctl($->shm_info->shmid,IPC_RMID,0);

		if (!$->use_shm) RAISE("shm got disabled, retrying...");
	} else
	#endif
	{
		/* let's overestimate the pixel size */
		/* int pixel_size = BitPacking_bytes($->bit_packing); */
		int pixel_size = 4;

		$->image = (uint8 *)calloc(sx * sy, pixel_size);

		$->ximage = XCreateImage($->display, $->visual,
			$->depth, ZPixmap, 0, (int8 *) $->image,
			sx, sy, 8, 0);
	}
	{
		int status = XInitImage($->ximage);
		whine("XInitImage returned: %d", status);
	}
	return true;
}

static void FormatX11_resize_window (FormatX11 *$, int sx, int sy) {
	int v[3] = {sy, sx, 3};
	Window oldw;

	if ($->parent && $->parent->in[0]->dex > 0) {
		whine("resizing while receiving picture (ignoring)");
		return;
	}

	FREE($->dim);
	$->dim = Dim_new(3,v);

/* ximage */

	FormatX11_alloc_image($,sx,sy);

/* window */
	
	$->name = strdup("GridFlow");

	oldw = $->window;
	if (oldw) {
		if ($->is_owner) {
			char *s = Dim_to_s($->dim);
			whine("About to resize window: %s",s);
			FREE(s);
			XResizeWindow($->display,$->window,sx,sy);
		}
	} else {
		char *s = Dim_to_s($->dim);
		whine("About to create window: %s",s);
		FREE(s);
		$->window = XCreateSimpleWindow($->display,
			$->root_window, 0, 0, sx, sy, 0, $->white, $->black);
		if(!$->window) RAISE("can't create window");
	}

	/* FormatX11_set_wm_hints($,sx,sy); */

	$->imagegc = XCreateGC($->display, $->window, 0, NULL);
	if ($->is_owner) {
		XSelectInput($->display, $->window,
			ExposureMask | StructureNotifyMask |
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
		XMapRaised($->display, $->window);
	} else {
		XSelectInput($->display, $->window,
			ExposureMask | StructureNotifyMask);
	}
	XSync($->display,0);
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
}

METHOD(FormatX11,close) {
	MainLoop_remove($);
	if (!$) {whine("stupid error: trying to close display NULL. =)"); return;}
	if ($->is_owner) XDestroyWindow($->display,$->window);
	XSync($->display,0);
	FormatX11_dealloc_image($);
	XCloseDisplay($->display);
	rb_call_super(argc,argv);
}

METHOD(FormatX11,option) {
	VALUE sym = argv[0];
	if (sym == SYM(out_size)) {
		int sy = INT(argv[1]);
		int sx = INT(argv[2]);
		COERCE_INT_INTO_RANGE(sy,16,MAX_INDICES);
		COERCE_INT_INTO_RANGE(sx,16,MAX_INDICES);
		FormatX11_resize_window($,sx,sy);
	} else if (sym == SYM(draw)) {
		int sy = Dim_get($->dim,0);
		int sx = Dim_get($->dim,1);
		FormatX11_show_section($,0,0,sx,sy);
	} else if (sym == SYM(autodraw)) {
		$->autodraw = INT(argv[1]);
		COERCE_INT_INTO_RANGE($->autodraw,0,2);
	} else
		rb_call_super(argc,argv);
}

static void FormatX11_open_display(FormatX11 *$, const char *disp_string) {
	int screen_num;
	Screen *screen;

	/* Open an X11 connection */
	$->display = XOpenDisplay(disp_string);
	if(!$->display) RAISE("ERROR: opening X11 display: %s",strerror(errno));

	/*
	  btw don't expect too much from X11 error handling system.
	  it sucks big time and it won't work.
	*/

	screen      = DefaultScreenOfDisplay($->display);
	screen_num  = DefaultScreen($->display);
	$->visual   = DefaultVisual($->display, screen_num);
	$->white    = XWhitePixel($->display,screen_num);
	$->black    = XBlackPixel($->display,screen_num);
	$->root_window = DefaultRootWindow($->display);
	$->depth    = DefaultDepthOfScreen(screen);

	whine("depth = %d",$->depth);

	/* !@#$ check for visual type instead */
	if ($->depth != 16 && $->depth != 24 && $->depth != 32)
		RAISE("ERROR: depth %d not supported.", $->depth);

#ifdef HAVE_X11_SHARED_MEMORY
	/* what do i do with remote windows? */
	$->use_shm = !! XShmQueryExtension($->display);
	whine("x11 shared memory compiled in; use_shm = %d",$->use_shm);
#else
	$->use_shm = false;
	whine("x11 shared memory is not compiled in");
#endif
}

METHOD(FormatX11,init) {
	/* defaults */
	int sy = 240;
	int sx = 320;

	rb_call_super(argc,argv);

	argv++, argc--;

	$->window  = 0;
	$->is_owner= true;
	$->image   = 0;
	$->ximage  = 0;
	$->autodraw= 1;
#ifdef HAVE_X11_SHARED_MEMORY
	$->shm_info= 0;
#endif

	if (argc<1) RAISE("not enough arguments");

	/*
		{here}
		{local 0}
		{remote relayer 0}
	*/

	{
		VALUE domain = argv[0];
		int i;
		// assert (ac>0);
		if (domain==SYM(here)) {
			whine("mode `here'");
			FormatX11_open_display($,0);
			i=1;
		} else if (domain==SYM(local)) {
			char host[256];
			int dispnum = NUM2INT(argv[1]);
			whine("mode `local', display_number `%d'",dispnum);
			sprintf(host,":%d",dispnum);
			FormatX11_open_display($,host);
			i=2;
		} else if (domain==SYM(remote)) {
			char host[256];
			int dispnum = 0;
			strcpy(host,rb_sym_name(argv[1]));
			dispnum = NUM2INT(argv[2]);
			sprintf(host+strlen(host),":%d",dispnum);
			whine("mode `remote', host `%s', display_number `%d'",host,dispnum);
			FormatX11_open_display($,host);
			i=3;
		} else {
			RAISE("x11 destination syntax error");
		}

		if (i>=argc) {
			whine("will create new window");
		} else {
			VALUE winspec = argv[i];
			if (winspec==SYM(root)) {
				$->window = $->root_window;
				whine("will use root window (0x%x)", $->window);
				$->is_owner = false;
			} else {
				const char *winspec2 = rb_sym_name(winspec);
				if (strncmp(winspec2,"0x",2)==0) {
					$->window = strtol(winspec2+2,0,16);
				} else {
					$->window = atoi(winspec2);
				}
				if ($->window) {
					$->is_owner = false;
					whine("will use specified window (0x%x)", $->window);
				} else {
					whine("bad window specification");
				}
			}
		}
	}

	/* "resize" also takes care of creation */
	FormatX11_resize_window($,sx,sy);

	{
		Visual *v = $->visual;
		int disp_is_le = !ImageByteOrder($->display);
		whine("is_le = %d",is_le());
		whine("disp_is_le = %d", disp_is_le);
		$->bit_packing = BitPacking_new(
			disp_is_le,
			$->ximage->bits_per_pixel/8,
			v->red_mask,
			v->green_mask,
			v->blue_mask);
	}

	BitPacking_whine($->bit_packing);
	MainLoop_add($,FormatX11_alarm);
}

FMTCLASS(FormatX11,"x11","X Window System Version 11.5",FF_R | FF_W,
inlets:1,outlets:1,LIST(GRINLET(FormatX11,0)),
DECL(FormatX11,init),
DECL(FormatX11,frame),
DECL(FormatX11,option),
DECL(FormatX11,close))

