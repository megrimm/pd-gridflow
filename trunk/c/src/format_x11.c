/*
	$Id$

	GridFlow
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
#include <stdio.h>
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

extern FormatClass class_FormatX11;
typedef struct FormatX11 {
	Format_FIELDS;

/* at the Display/Screen level */
	Display *display; /* connection to xserver */
	Visual *visual;   /* screen properties */
	Window root_window;
	long white_pixel;
	long black_pixel;
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
	XShmSegmentInfo *shm_info; /* to share memory with X11/Unix */
} FormatX11;

/* ---------------------------------------------------------------- */

static void FormatX11_show_section(FormatX11 *$, int x, int y, int sx, int sy) {
	#ifdef HAVE_X11_SHARED_MEMORY
	if ($->use_shm) {
		XSync($->display,False);
		if (sy>Dim_get($->dim,0)) sy=Dim_get($->dim,0);
		if (sx>Dim_get($->dim,1)) sx=Dim_get($->dim,1);
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

static Symbol button_sym(int i) {
	char foo[42];
	sprintf(foo,"button%d",i);
	return fts_new_symbol(foo);
}

void FormatX11_alarm(FormatX11 *$) {
	fts_atom_t at[4];
	XEvent e;
	int xpending;

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
			fts_set_symbol(at+0,SYM(press));
			fts_set_symbol(at+1,button_sym(eb->button));
			fts_set_int(at+2,eb->y);
			fts_set_int(at+3,eb->x);
			//fts_outlet_send(OBJ($->out->parent),0,4,at);
		}break;
		case ButtonRelease:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			//whine("button %d release at (y=%d,x=%d)",eb->button,eb->y,eb->x);
			fts_set_symbol(at+0,SYM(release));
			fts_set_symbol(at+1,button_sym(eb->button));
			fts_set_int(at+2,eb->y);
			fts_set_int(at+3,eb->x);
		}break;
		case MotionNotify:{
			XMotionEvent *em = (XMotionEvent *)&e;
			//whine("drag at (y=%d,x=%d)",em->y,em->x);
			fts_set_symbol(at+0,SYM(motion));
			fts_set_int(at+1,em->y);
			fts_set_int(at+2,em->x);
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

bool FormatX11_frame (FormatX11 *$, GridOutlet *out, int frame) {
	if (frame != -1) return 0;

	{
		char *s = Dim_to_s($->dim);
		whine("$->dim = %s",s);
		FREE(s);
	}

	XGetSubImage($->display, $->window,
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
err:
}

/* ---------------------------------------------------------------- */

void FormatX11_dealloc_image (FormatX11 *$) {
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

/* X11 Error Handler type */
typedef int (*XEH)(Display *, XErrorEvent *);

/* loathe Xlib's error handlers */
static FormatX11 *current_x11;
int FormatX11_error_handler (Display *d, XErrorEvent *xee) {
	current_x11->use_shm = false;
	whine("caught X11 error (should be \"BadAccess: can't find shm\")");
	return 42; /* it seems that the return value is ignored. */
}

bool FormatX11_alloc_image (FormatX11 *$, int sx, int sy) {
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
		if($->shm_info->shmid < 0) {
			whine("ERROR: shmget failed: %s",strerror(errno));
			goto err;
		}
		$->ximage->data = $->shm_info->shmaddr =
			(char *)shmat($->shm_info->shmid,0,0);
		$->image = (uint8 *)$->ximage->data;
		$->shm_info->readOnly = False;

		current_x11 = $;
		XSetErrorHandler(FormatX11_error_handler);
		if (!XShmAttach($->display, $->shm_info)) {
			whine("ERROR: XShmAttach: big problem");
			goto err;
		}
		/* make sure the server picks it up */
		XSync($->display,0);

		XSetErrorHandler(0);

		/* yes, this can be done now. should cause auto-cleanup. */
		shmctl($->shm_info->shmid,IPC_RMID,0);

		if (!$->use_shm) {
			whine("shm got disabled, retrying...");
			goto top;
		}
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
err:
	return false;
}

void FormatX11_resize_window (FormatX11 *$, int sx, int sy) {
	int v[3] = {sy, sx, 3};
	Window oldw;

	if ($->parent->in[0]->dex > 0) {
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
			$->root_window, 0, 0, sx, sy, 0,
			$->white_pixel, $->black_pixel);
		if(!$->window) {
			whine("can't create window");
			goto err;
		}
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
}

void FormatX11_close (FormatX11 *$) {
	Dict_del(gf_alarm_set,$);
	if (!$) {whine("stupid error: trying to close display NULL. =)"); return;}
	if ($->is_owner) XDestroyWindow($->display,$->window);
	XSync($->display,0);
	FormatX11_dealloc_image($);
	XCloseDisplay($->display);
	Format_close((Format *)$);
}

void FormatX11_option (FormatX11 *$, ATOMLIST) {
	Symbol sym = GET(0,symbol,SYM(foo));
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

FormatX11 *FormatX11_open_display(FormatX11 *$, const char *disp_string) {
	int screen_num;
	Screen *screen;

	/* Open an X11 connection */
	$->display = XOpenDisplay(disp_string);
	if(!$->display) {
		whine("ERROR: opening X11 display: %s",strerror(errno));
		goto err;
	}

	/*
	  btw don't try to set the X11 error handler.
	  it sucks big time and it won't work.
	*/

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

#ifdef HAVE_X11_SHARED_MEMORY
	/* what do i do with remote windows? */
	$->use_shm = !! XShmQueryExtension($->display);
	whine("x11 shared memory compiled in; use_shm = %d",$->use_shm);
#else
	$->use_shm = false;
	whine("x11 shared memory is not compiled in");
#endif

	return $;
err:;
	return 0;
}

Format *FormatX11_open (FormatClass *qlass, GridObject *parent, int mode, ATOMLIST) {
	FormatX11 *$ = (FormatX11 *)Format_open(&class_FormatX11,parent,mode);

	/* defaults */
	int sy = 240;
	int sx = 320;

	if (!$) return 0;

	$->window  = 0;
	$->is_owner= true;
	$->image   = 0;
	$->ximage  = 0;
	$->autodraw= 1;
	$->mode    = mode;
	$->shm_info= 0;

	/*
		{here}
		{local 0}
		{remote relayer 0}
	*/

	{
		Symbol domain = fts_get_symbol(at+0);
		int i;
		// assert (ac>0);
		if (domain==SYM(here)) {
			whine("mode `here'");
			if (!FormatX11_open_display($,0)) return 0;
			i=1;
		} else if (domain==SYM(local)) {
			char host[64];
			int dispnum = fts_get_int(at+1);
			whine("mode `local'");
			whine("display_number `%d'",dispnum);
			sprintf(host,":%d",dispnum);
			if (!FormatX11_open_display($,host)) return 0;
			i=2;
		} else if (domain==SYM(remote)) {
			char host[64];
			int dispnum = 0;
			strcpy(host,fts_symbol_name(fts_get_symbol(at+1)));
				dispnum = fts_get_int(at+2);
			sprintf(host+strlen(host),":%d",dispnum);
			whine("mode `remote'");
			whine("host `%s'",host);
			whine("display_number `%d'",dispnum);
			if (!FormatX11_open_display($,host)) return 0;
			i=3;
		} else {
			whine("x11 destination syntax error");
			return 0;
		}

		if (!$->display) {
			// ???
			whine("can't open x11 connection: %s", strerror(errno));
			goto err;
		}

		if (i>=ac) {
			whine("will create new window");
		} else {
			Symbol winspec = fts_get_symbol(at+i);
			if (winspec==SYM(root)) {
				$->window = $->root_window;
				whine("will use root window (0x%x)", $->window);
				$->is_owner = false;
			} else {
				const char *winspec2 = fts_symbol_name(winspec);
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
	Dict_put(gf_alarm_set,$,FormatX11_alarm);
	return (Format *)$;
err:;
	$->cl->close((Format *)$);
	return 0;
}

FormatClass class_FormatX11 = {
	object_size: sizeof(FormatX11),
	symbol_name: "x11",
	long_name: "X Window System Version 11.5",
	flags: FF_R | FF_W,

	open: FormatX11_open,
	frames: 0,
	frame:  (Format_frame_m)FormatX11_frame,
	begin:  (GRID_BEGIN_(Format,(*)))GRID_BEGIN_PTR(FormatX11,0),
	flow:    (GRID_FLOW_(Format,(*))) GRID_FLOW_PTR(FormatX11,0),
	end:      (GRID_END_(Format,(*)))  GRID_END_PTR(FormatX11,0),
	option: (Format_option_m)FormatX11_option,
	close:  (Format_close_m)FormatX11_close,
};

