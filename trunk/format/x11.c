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

#ifdef HAVE_X11_SHARED_MEMORY
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

struct FormatX11 : Format {

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
	Pt<uint8> image;     /* the real data (that XImage binds to) */
	bool is_owner;
#ifdef HAVE_X11_SHARED_MEMORY
	XShmSegmentInfo *shm_info; /* to share memory with X11/Unix */
#endif

	void show_section(int x, int y, int sx, int sy);
	void set_wm_hints (int sx, int sy);
	void dealloc_image ();
	bool alloc_image (int sx, int sy);
	void resize_window (int sx, int sy);
	void open_display(const char *disp_string);
};

/* ---------------------------------------------------------------- */

void FormatX11::show_section(int x, int y, int sx, int sy) {
#ifdef HAVE_X11_SHARED_MEMORY
	if (use_shm) {
		XSync(display,False);
		if (sy>dim->get(0)) sy=dim->get(0);
		if (sx>dim->get(1)) sx=dim->get(1);
//		gfpost("x,y,sx,sy = %d,%d,%d,%d",x,y,sx,sy);
		XShmPutImage(display,window,imagegc,ximage,x,y,x,y,sx,sy,False);
		/* should completion events be waited for? looks like a bug */
	} else
#endif
		XPutImage(display,window,imagegc,ximage,x,y,x,y,sx,sy);
	XFlush(display);
}

/* window manager hints, defines the window as non-resizable */
void FormatX11::set_wm_hints (int sx, int sy) {
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

	XStringListToTextProperty((char **)&name, 1, &window_name);
	XStringListToTextProperty((char **)&name, 1, &icon_name);

	XSetWMProperties(display, window,
		&window_name, &icon_name, NULL, 0, &hints, &wmhints, NULL);
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

//	gfpost("X11 HELLO? (%lld)",RtMetro_now());

	for (;;) {
		int xpending = XEventsQueued($->display, QueuedAfterReading);
		if (!xpending) break;
		XNextEvent($->display,&e);
		switch (e.type) {
		case Expose:{
			XExposeEvent *ex = (XExposeEvent *)&e;
			/*gfpost("ExposeEvent at (y=%d,x=%d) size (y=%d,x=%d)",
				ex->y,ex->x,ex->height,ex->width);*/
			if ($->mode == SYM(out)) {
				$->show_section(ex->x,ex->y,ex->width,ex->height);
			}
		}break;
		case ButtonPress:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			//gfpost("button %d press at (y=%d,x=%d)",eb->button,eb->y,eb->x);
			argv[0] = SYM(press);
			argv[1] = button_sym(eb->button);
			argv[2] = INT2NUM(eb->y);
			argv[3] = INT2NUM(eb->x);
			//FObject_send_thru($->out->parent,0,4,at);
		}break;
		case ButtonRelease:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			//gfpost("button %d release at (y=%d,x=%d)",eb->button,eb->y,eb->x);
			argv[0] = SYM(release);
			argv[1] = button_sym(eb->button);
			argv[2] = INT2NUM(eb->y);
			argv[3] = INT2NUM(eb->x);
		}break;
		case MotionNotify:{
			XMotionEvent *em = (XMotionEvent *)&e;
			//gfpost("drag at (y=%d,x=%d)",em->y,em->x);
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
			gfpost("received event of type # %d", e.type);
		}
	}
}

/* ---------------------------------------------------------------- */

METHOD(FormatX11,frame) {
//	gfpost("$->dim = %s",$->dim->to_s());

	XGetSubImage($->display, $->window,
		0, 0, $->dim->get(1), $->dim->get(0),
		(unsigned)-1, ZPixmap, $->ximage, 0, 0);

	$->out[0]->begin($->dim->dup());

	int sy=$->dim->get(0), sx=$->dim->get(1);
	int bs=$->dim->prod(1);
	STACK_ARRAY(Number,b2,bs);
	for(int y=0; y<sy; y++) {
		Pt<uint8> b1 = Pt<uint8>($->image,$->ximage->bytes_per_line*$->dim->get(0))
			+ $->ximage->bytes_per_line * y;
		$->bit_packing->unpack(sx,b1,b2);
		$->out[0]->send(bs,b2);
	}

	$->out[0]->end();
}

/* ---------------------------------------------------------------- */

/* loathe Xlib's error handlers */
static FormatX11 *current_x11;
static int FormatX11_error_handler (Display *d, XErrorEvent *xee) {
	current_x11->use_shm = false;
	gfpost("caught X11 error (should be \"BadAccess: can't find shm\")");
	return 42; /* it seems that the return value is ignored. */
}

void FormatX11::dealloc_image () {
	if (!ximage) return;
	if (use_shm) {
	#ifdef HAVE_X11_SHARED_MEMORY
		if (shm_info) delete shm_info, shm_info=0;
		/* and what do i do to ximage? */
	#endif	
	} else {
		XDestroyImage(ximage);
		ximage = 0; 
		/* delete $->image; */
	}
}

bool FormatX11::alloc_image (int sx, int sy) {
top:
	dealloc_image();
#ifdef HAVE_X11_SHARED_MEMORY
	if (use_shm) {
		shm_info = new XShmSegmentInfo;
		ximage = XShmCreateImage(display,visual,depth,ZPixmap,0,shm_info,sx,sy);
		assert(ximage);
		shm_info->shmid = shmget(
			IPC_PRIVATE,
			ximage->bytes_per_line * ximage->height,
			IPC_CREAT|0777);
		if(shm_info->shmid < 0) RAISE("ERROR: shmget failed: %s",strerror(errno));
		ximage->data = shm_info->shmaddr =
			(char *)shmat(shm_info->shmid,0,0);
		image = Pt<uint8>((uint8 *)ximage->data,
			ximage->bytes_per_line*dim->get(0));
		shm_info->readOnly = False;

		current_x11 = this;
		XSetErrorHandler(FormatX11_error_handler);
		if (!XShmAttach(display, shm_info)) RAISE("ERROR: XShmAttach: big problem");

		/* make sure the server picks it up */
		XSync(display,0);

		XSetErrorHandler(0);

		/* yes, this can be done now. should cause auto-cleanup. */
		shmctl(shm_info->shmid,IPC_RMID,0);

		if (!use_shm) RAISE("shm got disabled, retrying...");
	} else
#endif
	{
		/* let's overestimate the pixel size */
		/* int pixel_size = BitPacking_bytes($->bit_packing); */
		int pixel_size = 4;
		image = Pt<uint8>((uint8 *)calloc(sx*sy, pixel_size),
			ximage->bytes_per_line*dim->get(0));
		ximage = XCreateImage(
			display,visual,depth,ZPixmap,0,(int8 *)image,sx,sy,8,0);
	}
	int status = XInitImage(ximage);
	gfpost("XInitImage returned: %d", status);
	return true;
}

void FormatX11::resize_window (int sx, int sy) {
	int v[3] = {sy, sx, 3};
	Window oldw;
	if (parent && parent->in[0]->dex > 0) {
		gfpost("resizing while receiving picture (ignoring)");
		return;
	}
	if (dim) delete dim;
	dim = new Dim(3,v);

/* ximage */

	alloc_image(sx,sy);

/* window */
	
	name = strdup("GridFlow");

	oldw = window;
	if (oldw) {
		if (is_owner) {
			//gfpost("About to resize window: %s",dim->to_s());
			XResizeWindow(display,window,sx,sy);
		}
	} else {
		//gfpost("About to create window: %s",dim->to_s());
		window = XCreateSimpleWindow(display,
			root_window, 0, 0, sx, sy, 0, white, black);
		if(!window) RAISE("can't create window");
	}

/*	set_wm_hints(sx,sy); */

	imagegc = XCreateGC(display, window, 0, NULL);
	printf("is_owner: %d\n", is_owner);
	if (is_owner) {
		XSelectInput(display, window,
			ExposureMask | StructureNotifyMask |
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
		XMapRaised(display, window);
	} else {
		XSelectInput(display, window,
			ExposureMask | StructureNotifyMask);
	}
	XSync(display,0);
}

GRID_BEGIN(FormatX11,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels: red,green,blue (got %d)",in->dim->get(2));
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1), osx = $->dim->get(1);
	int sy = in->dim->get(0), osy = $->dim->get(0);
	in->set_factor(sxc);
	if (sx!=osx || sy!=osy) $->resize_window(sx,sy);
}

GRID_FLOW(FormatX11,0) {
	int bypl = $->ximage->bytes_per_line;
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1);
	int y = in->dex / sxc;

	assert((in->dex % sxc) == 0);
	assert((n       % sxc) == 0);

	while (n>0) {
		/* gfpost("bypl=%d sxc=%d sx=%d y=%d n=%d",bypl,sxc,sx,y,n); */
		/* convert line */
		$->bit_packing->pack(sx, data, $->image+y*bypl);
		if ($->autodraw==2) $->show_section(0,y,sx,1);
		y++;
		data += sxc;
		n -= sxc;
	}
}

GRID_END(FormatX11,0) {
	if ($->autodraw==1) {
		int sx = in->dim->get(1);
		int sy = in->dim->get(0);
		$->show_section(0,0,sx,sy);
	}
}

METHOD(FormatX11,close) {
	MainLoop_remove($);
	if (!$) {gfpost("stupid error: trying to close display NULL. =)"); return;}
	if ($->is_owner) XDestroyWindow($->display,$->window);
	XSync($->display,0);
	$->dealloc_image();
	XCloseDisplay($->display);
	rb_call_super(argc,argv);
}

METHOD(FormatX11,option) {
	VALUE sym = argv[0];
	if (sym == SYM(out_size)) {
		int sy = INT(argv[1]);
		int sx = INT(argv[2]);
		if (sy<16) sy=16;
		if (sx<16) sx=16;
		if (sy>MAX_INDICES) RAISE("height too big");
		if (sx>MAX_INDICES) RAISE("width too big");
		$->resize_window(sx,sy);
	} else if (sym == SYM(draw)) {
		int sy = $->dim->get(0);
		int sx = $->dim->get(1);
		$->show_section(0,0,sx,sy);
	} else if (sym == SYM(autodraw)) {
		int autodraw = INT(argv[1]);
		if (autodraw<0 || autodraw>2)
			RAISE("autodraw=%d is out of range",autodraw);
		$->autodraw = autodraw;
	} else
		rb_call_super(argc,argv);
}

void FormatX11::open_display(const char *disp_string) {
	int screen_num;
	Screen *screen;

	/* Open an X11 connection */
	display = XOpenDisplay(disp_string);
	if(!display) RAISE("ERROR: opening X11 display: %s",strerror(errno));

	/*
	  btw don't expect too much from X11 error handling system.
	  it sucks big time and it won't work.
	*/

	screen   = DefaultScreenOfDisplay(display);
	screen_num = DefaultScreen(display);
	visual   = DefaultVisual(display, screen_num);
	white    = XWhitePixel(display,screen_num);
	black    = XBlackPixel(display,screen_num);
	root_window = DefaultRootWindow(display);
	depth    = DefaultDepthOfScreen(screen);

	gfpost("depth = %d",depth);

	/* !@#$ check for visual type instead */
	if (depth != 16 && depth != 24 && depth != 32)
		RAISE("ERROR: depth %d not supported.", depth);

#ifdef HAVE_X11_SHARED_MEMORY
	/* what do i do with remote windows? */
	use_shm = !! XShmQueryExtension(display);
	gfpost("x11 shared memory compiled in; use_shm = %d",use_shm);
#else
	use_shm = false;
	gfpost("x11 shared memory is not compiled in");
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
	$->image   = Pt<uint8>();
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

	VALUE domain = argv[0];
	int i;
	// assert (ac>0);
	if (domain==SYM(here)) {
		gfpost("mode `here'");
		$->open_display(0);
		i=1;
	} else if (domain==SYM(local)) {
		if (argc<2) RAISE("open local: not enough args");
		char host[256];
		int dispnum = NUM2INT(argv[1]);
		gfpost("mode `local', display_number `%d'",dispnum);
		sprintf(host,":%d",dispnum);
		$->open_display(host);
		i=2;
	} else if (domain==SYM(remote)) {
		if (argc<3) RAISE("open remote: not enough args");
		char host[256];
		strcpy(host,rb_sym_name(argv[1]));
		int dispnum = NUM2INT(argv[2]);
		sprintf(host+strlen(host),":%d",dispnum);
		gfpost("mode `remote', host `%s', display_number `%d'",host,dispnum);
		$->open_display(host);
		i=3;
	} else {
		RAISE("x11 destination syntax error");
	}

	if (i>=argc) {
		gfpost("will create new window");
	} else {
		VALUE winspec = argv[i];
		if (winspec==SYM(root)) {
			$->window = $->root_window;
			gfpost("will use root window (0x%x)", $->window);
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
				gfpost("will use specified window (0x%x)", $->window);
			} else {
				gfpost("bad window specification");
			}
		}
	}

	/* "resize" also takes care of creation */
	$->resize_window(sx,sy);

	Visual *v = $->visual;
	int disp_is_le = !ImageByteOrder($->display);
	uint32 masks[3] = { v->red_mask, v->green_mask, v->blue_mask };
	gfpost("is_le = %d",is_le());
	gfpost("disp_is_le = %d", disp_is_le);
	$->bit_packing = new BitPacking(
		disp_is_le, $->ximage->bits_per_pixel/8, 3, masks);

	$->bit_packing->gfpost();
	MainLoop_add($,(void(*)(void*))FormatX11_alarm);
}

static void startup (GridClass *$) {
	IEVAL($->rubyclass,
	"conf_format 6,'x11','X Window System Version 11.5'");
}

GRCLASS(FormatX11,"FormatX11",
inlets:1,outlets:1,startup:startup,LIST(GRINLET(FormatX11,0)),
DECL(FormatX11,init),
DECL(FormatX11,frame),
DECL(FormatX11,option),
DECL(FormatX11,close))

