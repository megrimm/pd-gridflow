/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

\class FormatX11 < Format
struct FormatX11 : Format {
	template <class T> void frame_by_type (T bogus);
/* at the Display/Screen level */
	Display *display; /* connection to xserver */
	Visual *visual;   /* screen properties */
	Window root_window;
	Colormap colormap; /* for 256-color mode */
	long black,white; /* color numbers in default colormap */
	short depth;
	bool use_shm;	   /* should use shared memory? */
	bool use_stripes;  /* use alternate conversion in 256-color mode */

/* at the Window level */
	int autodraw;        /* how much to send to the display at once */
	Window window;       /* X11 window number */
	GC imagegc;          /* X11 graphics context (like java.awt.Graphics) */
	XImage *ximage;      /* X11 image descriptor */
	char *name;          /* window name (for use by window manager) */
	Pt<uint8> image;     /* the real data (that XImage binds to) */
	bool is_owner;
	bool verbose;

	BitPacking *bit_packing;
	Dim *dim;

#ifdef HAVE_X11_SHARED_MEMORY
	XShmSegmentInfo *shm_info; /* to share memory with X11/Unix */
#endif

	FormatX11 () : use_stripes(false), 
	autodraw(1), window(0), ximage(0), image(Pt<uint8>()), is_owner(true),
	verbose(false), dim(0)
#ifdef HAVE_X11_SHARED_MEMORY
		, shm_info(0)
#endif
	{}

	void show_section(int x, int y, int sx, int sy);
	void set_wm_hints (int sx, int sy);
	void dealloc_image ();
	bool alloc_image (int sx, int sy);
	void resize_window (int sx, int sy);
	void open_display(const char *disp_string);
	void report_pointer(int y, int x, int state);
	void prepare_colormap();
	void alarm();

	\decl void frame ();
	\decl void close ();
	\decl void out_size (int sy, int sx);
	\decl void draw ();
	\decl void autodraw_m (int autodraw);
	\decl void setcursor (int shape);
	\decl void hidecursor ();
	\decl void verbose_m (int verbose);
	\decl void initialize (...);
	GRINLET3(0);
};

/* ---------------------------------------------------------------- */

void FormatX11::show_section(int x, int y, int sx, int sy) {
	if (y>dim->get(0)||x>dim->get(1)) return;
	if (y+sy>dim->get(0)) sy=dim->get(0)-y;
	if (x+sx>dim->get(1)) sx=dim->get(1)-x;
#ifdef HAVE_X11_SHARED_MEMORY
	if (use_shm) {
		XSync(display,False);
//		gfpost("display,window,imagegc = %d,%d,%d",display,window,imagegc);
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

static Ruby button_sym(int i) {
	char foo[42];
	sprintf(foo,"button%d",i);
	return SYM(foo);
}

static void FormatX11_alarm(FormatX11 *self) { self->alarm(); }

void FormatX11::report_pointer(int y, int x, int state) {
	Ruby argv[5] = {
		INT2NUM(0), SYM(position),
		INT2NUM(y), INT2NUM(x), INT2NUM(state) };
	send_out(COUNT(argv),argv);
}

void FormatX11::alarm() {
	XEvent e;

	for (;;) {
		int xpending = XEventsQueued(display, QueuedAfterReading);
		if (!xpending) break;
		XNextEvent(display,&e);
		switch (e.type) {
		case Expose:{
			XExposeEvent *ex = (XExposeEvent *)&e;
			if (verbose)
				gfpost("ExposeEvent at (y=%d,x=%d) size (y=%d,x=%d)",
					ex->y,ex->x,ex->height,ex->width);
			if (mode() == SYM(out)) {
				show_section(ex->x,ex->y,ex->width,ex->height);
			}
		}break;
		case ButtonPress:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			if (verbose)
				gfpost("button %d press at (y=%d,x=%d)",eb->button,eb->y,eb->x);
			eb->state |= 128<<eb->button;
			report_pointer(eb->y,eb->x,eb->state);
		}break;
		case ButtonRelease:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			if (verbose)
				gfpost("button %d release at (y=%d,x=%d)",eb->button,eb->y,eb->x);
			eb->state &= ~(128<<eb->button);
			report_pointer(eb->y,eb->x,eb->state);
		}break;
		case MotionNotify:{
			XMotionEvent *em = (XMotionEvent *)&e;
			if (verbose)
				gfpost("drag at (y=%d,x=%d)",em->y,em->x);
			report_pointer(em->y,em->x,em->state);
		}break;
		case DestroyNotify:{
			/* should notify parent here */
		}break;
		case ConfigureNotify:{
			/* like we care */
		}break;
		default:
			if (verbose) gfpost("received event of type # %d", e.type);
		}
	}
}

/* ---------------------------------------------------------------- */

\def void frame () {
	XGetSubImage(display, window,
		0, 0, dim->get(1), dim->get(0),
		(unsigned)-1, ZPixmap, ximage, 0, 0);
	out[0]->begin(dim->dup(),
		NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
	int sy=dim->get(0), sx=dim->get(1);
	int bs=dim->prod(1);
	STACK_ARRAY(uint8,b2,bs);
	for(int y=0; y<sy; y++) {
		Pt<uint8> b1 = Pt<uint8>(image,ximage->bytes_per_line*dim->get(0))
			+ ximage->bytes_per_line * y;
		bit_packing->unpack(sx,b1,b2);
		out[0]->send(bs,b2);
	}
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
		/* !@#$ and what do i do to ximage? */
	#endif	
	} else {
		XDestroyImage(ximage);
		ximage = 0; 
		/* delete image; */
	}
}

bool FormatX11::alloc_image (int sx, int sy) {
	int32 v[3] = {sy, sx, 3};
	if (dim) delete dim;
	dim = new Dim(3,v);
top:
	dealloc_image();
	if (sx==0 || sy==0) return false;
#ifdef HAVE_X11_SHARED_MEMORY
	if (use_shm) {
		shm_info = new XShmSegmentInfo;
		ximage = XShmCreateImage(display,visual,depth,ZPixmap,0,shm_info,sx,sy);
		if (!ximage) {
			gfpost("shm got disabled, retrying...");
			goto top;
		}
		shm_info->shmid = shmget(
			IPC_PRIVATE,
			ximage->bytes_per_line * ximage->height,
			IPC_CREAT|0777);
		if(shm_info->shmid < 0) RAISE("ERROR: shmget failed: %s",strerror(errno));
		ximage->data = shm_info->shmaddr =
			(char *)shmat(shm_info->shmid,0,0);
		image = Pt<uint8>((uint8 *)ximage->data,
			ximage->bytes_per_line*sy);
		shm_info->readOnly = False;

		current_x11 = this;
		XSetErrorHandler(FormatX11_error_handler);
		if (!XShmAttach(display, shm_info)) RAISE("ERROR: XShmAttach: big problem");

		/* make sure the server picks it up */
		XSync(display,0);

		XSetErrorHandler(0);

		/* yes, this can be done now. should cause auto-cleanup. */
		shmctl(shm_info->shmid,IPC_RMID,0);

		if (!use_shm) {
			gfpost("shm got disabled, retrying...");
			goto top;
		}
	} else
#endif
	{
		/* let's overestimate the pixel size */
		/* int pixel_size = BitPacking_bytes(bit_packing); */
		int pixel_size = 4;
		ximage = XCreateImage(
			display,visual,depth,ZPixmap,0,0,sx,sy,8,0);
		if (!ximage) RAISE("can't create image"); 
		image = ARRAY_NEW(uint8,ximage->bytes_per_line*sy);
		ximage->data = (int8 *)image;
	}
	int status = XInitImage(ximage);
	if (status!=1) gfpost("XInitImage returned: %d", status);
	return true;
}

void FormatX11::resize_window (int sx, int sy) {
	if (sy<16) sy=16; if (sy>4000) RAISE("height too big");
	if (sx<16) sx=16; if (sx>4000) RAISE("width too big");
	alloc_image(sx,sy);
	name = strdup("GridFlow");
	if (window) {
		if (is_owner) XResizeWindow(display,window,sx,sy);
	} else {
		window = XCreateSimpleWindow(display,
			root_window, 0, 0, sx, sy, 0, white, black);
		if(!window) RAISE("can't create window");
	}
/*	set_wm_hints(sx,sy); */
	imagegc = XCreateGC(display, window, 0, NULL);
	if (is_owner) {
		XSelectInput(display, window,
			ExposureMask | StructureNotifyMask |
			PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
		XMapRaised(display, window);
	} else {
		XSelectInput(display, window,
			ExposureMask | StructureNotifyMask);
	}
	if (visual->c_class == PseudoColor) prepare_colormap(); 
	XSync(display,0);
}

GRID_INLET(FormatX11,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels: red,green,blue (got %d)",in->dim->get(2));
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1), osx = dim->get(1);
	int sy = in->dim->get(0), osy = dim->get(0);
	in->set_factor(sxc);
	if (sx!=osx || sy!=osy) resize_window(sx,sy);
} GRID_FLOW {
	int bypl = ximage->bytes_per_line;
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1);
	int y = in->dex/sxc;
	int oy = y;
	for (; n>0; y++, data+=sxc, n-=sxc) {
		/* convert line */
		if (use_stripes) {
			int o=y*bypl;
			for (int x=0, i=0, k=y%3; x<sx; x++, i+=3, k=(k+1)%3) {
				image[o+x] = (k<<6) | data[i+k]>>2;
			}
		} else {
			bit_packing->pack(sx, data, image+y*bypl);
		}
	}
	if (autodraw==2) show_section(0,oy,sx,y-oy);
} GRID_FINISH {
	if (autodraw==1) show_section(0,0,in->dim->get(1),in->dim->get(0));
	/* calling this here because the clock problem is annoying */
	alarm();
} GRID_END

\def void close () {
	if (!this) RAISE("stupid error: trying to close display NULL. =)");
	if (bit_packing) delete bit_packing;
	MainLoop_remove(this);
	if (is_owner) XDestroyWindow(display,window);
	XSync(display,0);
	dealloc_image();
	XCloseDisplay(display);
	rb_call_super(argc,argv);
}

\def void out_size (int sy, int sx) {
	resize_window(sx,sy);
}

\def void draw () {
	int sy = dim->get(0);
	int sx = dim->get(1);
	show_section(0,0,sx,sy);
}

\def void autodraw_m (int autodraw) {
	if (autodraw<0 || autodraw>2)
		RAISE("autodraw=%d is out of range",autodraw);
	this->autodraw = autodraw;
}

\def void setcursor (int shape) {
	if (argc<2) RAISE("not enough args");
	shape = 2*(shape&63);
	Cursor c = XCreateFontCursor(display,shape);
	XDefineCursor(display,window,c);
	XFlush(display);
}

\def void hidecursor () {
	Font font = XLoadFont(display,"fixed");
	XColor color; /* bogus */
	Cursor c = XCreateGlyphCursor(display,font,font,' ',' ',&color,&color);
	XDefineCursor(display,window,c);
	XFlush(display);
}

\def void verbose_m (int verbose) {
	this->verbose = !! verbose;
}

void FormatX11::prepare_colormap() {
	Colormap colormap = XCreateColormap(display,window,visual,AllocAll);
	XColor colors[256];
	if (use_stripes) {
		for (int i=0; i<192; i++) {
			int k=(i&63)*0xffff/63;
			colors[i].pixel = i;
			colors[i].red   = (i>>6)==0 ? k : 0;
			colors[i].green = (i>>6)==1 ? k : 0;
			colors[i].blue  = (i>>6)==2 ? k : 0;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		XStoreColors(display,colormap,colors,192);
	} else {	
		for (int i=0; i<256; i++) {
			colors[i].pixel = i;
			colors[i].red   = ((i>>0)&7)*0xffff/7;
			colors[i].green = ((i>>3)&7)*0xffff/7;
			colors[i].blue  = ((i>>6)&3)*0xffff/3;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		XStoreColors(display,colormap,colors,256);
	}
	XSetWindowColormap(display,window,colormap);
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
	colormap = 0;

	switch(visual->c_class) {
	case TrueColor: case DirectColor: /* without colormap */
	break;
	case PseudoColor: /* with colormap */
		if (depth!=8)
			RAISE("ERROR: with colormap, only supported depth is 8 (got %d)",
				depth);
	break;
	default: RAISE("ERROR: visual type not supported (got %d)",
		visual->c_class);
	}

#ifdef HAVE_X11_SHARED_MEMORY
	/* what do i do with remote windows? */
	use_shm = !! XShmQueryExtension(display);
	if (verbose) gfpost("x11 shared memory compiled in; use_shm = %d",use_shm);
#else
	use_shm = false;
	if (verbose) gfpost("x11 shared memory is not compiled in");
#endif
}

\def void initialize (...) {
	/* defaults */
	int sy = 240;
	int sx = 320;

	rb_call_super(argc,argv);
	argv++, argc--;
	VALUE domain = argc<1 ? SYM(here) : argv[0];

	if (domain==SYM(verbose)) {
		verbose = true;
		argv++, argc--;
		domain = argc<1 ? SYM(here) : argv[0];
	}

	int i;
	if (domain==SYM(here)) {
		if (verbose) gfpost("mode `here'");
		open_display(0);
		i=1;
	} else if (domain==SYM(local)) {
		if (argc<2) RAISE("open local: not enough args");
		char host[256];
		int dispnum = NUM2INT(argv[1]);
		if (verbose) gfpost("mode `local', display_number `%d'",dispnum);
		sprintf(host,":%d",dispnum);
		open_display(host);
		i=2;
	} else if (domain==SYM(remote)) {
		if (argc<3) RAISE("open remote: not enough args");
		char host[256];
		strcpy(host,rb_sym_name(argv[1]));
		int dispnum = NUM2INT(argv[2]);
		sprintf(host+strlen(host),":%d",dispnum);
		if (verbose) gfpost("mode `remote', host `%s', display_number `%d'",host,dispnum);
		open_display(host);
		i=3;
	} else {
		RAISE("x11 destination syntax error");
	}

	if (i<argc && argv[i]==SYM(use_stripes)) {
		use_stripes = true;
		i++;
	}

	if (i>=argc) {
		if (verbose) gfpost("will create new window");
	} else {
		VALUE winspec = argv[i];
		if (winspec==SYM(root)) {
			window = root_window;
			if (verbose) gfpost("will use root window (0x%x)", window);
			is_owner = false;
		} else {
			const char *winspec2 = rb_sym_name(winspec);
			if (strncmp(winspec2,"0x",2)==0) {
				window = strtol(winspec2+2,0,16);
			} else {
				window = atoi(winspec2);
			}
			if (window) {
				is_owner = false;
				if (verbose) gfpost("will use specified window (0x%x)", window);
			} else {
				if (verbose) gfpost("bad window specification");
			}
		}
	}

	/* "resize" also takes care of creation */
	resize_window(sx,sy);

	Visual *v = visual;
	int disp_is_le = !ImageByteOrder(display);
	if (verbose) gfpost("is_le = %d, disp_is_le = %d",is_le(),disp_is_le);

	switch(visual->c_class) {
	case TrueColor: case DirectColor: {
		uint32 masks[3] = { v->red_mask, v->green_mask, v->blue_mask };
		bit_packing = new BitPacking(
			disp_is_le, ximage->bits_per_pixel/8, 3, masks);
	} break;
	case PseudoColor: {
		uint32 masks[3] = { 0x07, 0x38, 0xC0 };
		bit_packing = new BitPacking(
			disp_is_le, ximage->bits_per_pixel/8, 3, masks);
	} break;
	}

	if (verbose) bit_packing->gfpost();
	MainLoop_add(this,(void(*)(void*))FormatX11_alarm);
}


GRCLASS(FormatX11,LIST(GRINLET2(FormatX11,0,4)),
\grdecl
){
	IEVAL(rself,"install 'FormatX11',1,1;"
	"conf_format 6,'x11','X Window System Version 11.5'");
}

\end class FormatX11
