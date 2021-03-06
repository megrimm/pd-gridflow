/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

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
#include "gridflow.hxx.fcs"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
//#include <X11/StringDefs.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

/* X11 Error Handler type */
typedef int (*XEH)(Display *, XErrorEvent *);

struct FormatX11;
void FormatX11_call(FormatX11 *p);

typedef struct {
    long flags;
    long functions;
    long decorations;
    long input_mode;
    long state;
} MotifWmHints;

#define XA_INIT(x) XA##x = XInternAtom(display, #x, False) /* mplayer guys know how to write code. */

\class FormatX11 : Format {
/* at the Display/Screen level */
	Display *display; /* connection to xserver */
	Visual *visual;   /* screen properties */
	Window root_window;
	Colormap colormap;/* for 256-color mode */
	short depth;
	bool use_stripes; /* use alternate conversion in 256-color mode */
	bool shared_memory;
	Atom XA_NET_WM_STATE;
	Atom XA_NET_WM_STATE_FULLSCREEN;
	Atom XA_MotifHints;
/* at the Window level */
	Window window;    /* X11 window number */
	Window parent;    /* X11 window number of the parent */
	GC imagegc;       /* X11 graphics context (like java.awt.Graphics) */
	XImage *ximage;   /* X11 image descriptor */
	uint8 *image;     /* the real data (that XImage binds to) */
	bool fullscreen, border, is_owner, lock_size, override_redirect;
	MotifWmHints motifWmHints;
	int32 pos[2];
	P<BitPacking> bit_packing;
	Dim dim;
	int minsy,minsx;
	t_clock *clock;
	string title;
	XShmSegmentInfo *shm_info; /* to share memory with X11/Unix */
	~FormatX11 () {
		clock_unset(clock);
		if (is_owner) XDestroyWindow(display,window);
		XSync(display,0);
		dealloc_image();
		if (imagegc) XFreeGC(display,imagegc);
		XCloseDisplay(display);
	}
	template <class T> void frame_by_type (T bogus);
	void show_section(int x, int y, int sx, int sy);
	void set_wm_hints ();
	void dealloc_image ();
	bool alloc_image (int sx, int sy);
	void resize_window (int sx, int sy);
	void open_display(const char *disp_string);
	void report_pointer(int y, int x, int state) {t_atom2 a[3] = {y,x,state}; out[0](gensym("position"),COUNT(a),a);}
	void prepare_colormap();
	Window search_window_tree (Window xid, Atom key, const char *value, int level=0);
	\constructor (...) {
		shared_memory=false; use_stripes=false; window=0; ximage=0; image=0; is_owner=true;
		dim=0; lock_size=false; override_redirect=false; clock=0; imagegc=0;
		shm_info=0;
		minsy=16; minsx=16;
		int sy=240, sx=320; // defaults
		argv++, argc--;
		t_symbol *domain = argc<1 ? gensym("here") : (t_symbol *)argv[0];
		int i;
		char host[256];
		if (domain==gensym("here")) {
			open_display(0);
			i=1;
		} else if (domain==gensym("local")) {
			if (argc<2) RAISE("open x11 local: not enough args");
			sprintf(host,":%d",int32(argv[1]));
			open_display(host);
			i=2;
		} else if (domain==gensym("remote")) {
			if (argc<3) RAISE("open x11 remote: not enough args");
			sprintf(host,"%s:%d",string(argv[1]).data(),int32(argv[2]));
			open_display(host);
			i=3;
		} else if (domain==gensym("display")) {
			if (argc<2) RAISE("open x11 display: not enough args");
			strcpy(host,string(argv[1]).data());
			for (int k=0; host[k]; k++) if (host[k]=='%') host[k]==':';
			//post("mode `display', DISPLAY=`%s'",host);
			open_display(host);
			i=2;
		} else RAISE("x11 destination syntax error");
		for(;i<argc;i++) {
			if (argv[i]==gensym("override_redirect")) override_redirect = true;
			else if (argv[i]==gensym("use_stripes"))  use_stripes = true;
			else break; /*RAISE("argument '%s' not recognized",string(argv[i]).data());*/
		}
		pos[1]=pos[0]=0;
		parent = root_window;
		if (i>=argc) {
		} else {
			const t_atom2 &winspec = argv[i];
			if (winspec==gensym("root")) {
				window = root_window;
				is_owner = false;
			} else if (winspec==gensym("embed")) {
				string title = argv[i+1];
				sy = sx = pos[0] = pos[1] = 0;
				parent = search_window_tree(root_window,XInternAtom(display,"WM_NAME",0),title.data());
				if (parent == 0xDeadBeef) RAISE("Window not found.");
			} else if (winspec==gensym("embed_by_id")) {
				const char *winspec2 = string(argv[i+1]).data();
				if (strncmp(winspec2,"0x",2)==0) {
					parent = strtol(winspec2+2,0,16);
				} else {
					parent = atoi(winspec2);
				}
			} else {
				if (winspec.a_type==A_SYMBOL) {
					const char *winspec2 = string(winspec).data();
					if (strncmp(winspec2,"0x",2)==0) window = strtol(winspec2+2,0,16);
					else 				 window = atoi(winspec2); // huh?
				} else window = int32(winspec);
				is_owner = false;
				sy = sx = pos[0] = pos[1] = 0;
			}
		}
		resize_window(sx,sy); // "resize" also takes care of creation
		if (is_owner) {
			Atom wmDeleteAtom = XInternAtom(display, "WM_DELETE_WINDOW", False);
			XSetWMProtocols(display,window,&wmDeleteAtom,1);
		}
		Visual *v = visual;
		int disp_is_le = !ImageByteOrder(display);
		int bpp = ximage->bits_per_pixel;
		switch(v->c_class) {
		case TrueColor: case DirectColor: {
			uint32 masks[3] = {v->red_mask, v->green_mask, v->blue_mask};
			bit_packing = new BitPacking(disp_is_le, bpp/8, 3, masks);
		} break;
		case PseudoColor: {
			uint32 masks[3] = {0x07, 0x38, 0xC0}; // BBGGGRRR
			bit_packing = new BitPacking(disp_is_le, bpp/8, 3, masks);
		} break;
		default: RAISE("huh?");
		}
		clock = clock_new(this,(t_method)FormatX11_call);
		clock_delay(clock,0);
		show_section(0,0,sx,sy);
		if ((mode&4)!=0) {
			Window root; int x,y; unsigned sx,sy,sb,depth;
			XGetGeometry(display,window,&root,&x,&y,&sx,&sy,&sb,&depth);
			_0_out_size(sy,sx);
		}
		_0_border(1);
	}

	\decl 0 bang () {
		XGetSubImage(display, window, 0, 0, dim[1], dim[0], (unsigned)-1, ZPixmap, ximage, 0, 0);
		GridOut go(this,0,dim,cast);
		int sy=dim[0], sx=dim[1], bs=dim.prod(1);
		uint8 b2[bs];
		for(int y=0; y<sy; y++) {
			uint8 *b1 = image + ximage->bytes_per_line * y;
			bit_packing->unpack(sx,b1,b2);
			go.send(bs,b2);
		}
	}
	void call ();
	\decl 0 out_size         (int sy, int sx) {resize_window(sx,sy);}
	\decl 0 out_size_minimum (int sy, int sx) {minsy=max(1,sy); minsx=max(1,sx);}
	\decl 0 setcursor (int shape) {
		if (shape<0 || shape>=77) RAISE("unknown shape number (should be at least 0 but less than 77)");
		Cursor c = XCreateFontCursor(display,2*shape);
		XDefineCursor(display,window,c);
		XFlush(display);
	}
	\decl 0 hidecursor () {
		Font font = XLoadFont(display,"fixed");
		XColor color; /* bogus */
		Cursor c = XCreateGlyphCursor(display,font,font,' ',' ',&color,&color);
		XDefineCursor(display,window,c);
		XFlush(display);
	}
	\decl 0 set_geometry (int y, int x, int sy, int sx) {
		pos[0]=y; pos[1]=x;
		XMoveWindow(display,window,x,y);
		resize_window(sx,sy);
		XFlush(display);
	}
	\decl 0 move (int y, int x) {
		pos[0]=y; pos[1]=x;
		XMoveWindow(display,window,x,y);
		XFlush(display);
	}
	\decl 0 shared_memory (bool toggle=1) {shared_memory = toggle;}
	\decl 0 title (string title="") {this->title = title; set_wm_hints();}
	\decl 0 warp (int y, int x) {
		XWarpPointer(display,None,None,0,0,0,0,x,y);
		XFlush(display);
	}
	\decl 0 fullscreen (bool toggle=1) { // not working
		fullscreen=toggle;
		XEvent xev; CLEAR(&xev); typeof(xev.xclient) &xc = xev.xclient;
		xc.type = ClientMessage;
		xc.serial = 0;
		xc.send_event = True;
		xc.message_type = XA_NET_WM_STATE;
		xc.window = window;
		xc.format = 32;
		xc.data.l[0] = toggle;
		xc.data.l[1] = XA_NET_WM_STATE_FULLSCREEN;
		if (!XSendEvent(display,root_window,False,SubstructureRedirectMask|SubstructureNotifyMask,&xev))
		RAISE("can't set fullscreen");
		XFlush(display);
	}
	\decl 0 border     (bool toggle=1);
	\decl 0 loadbang () {out[0](gensym("nogrey"),0,0);}
//	\decl 0 raise ();
	\grin 0 int
	\decl 0 query_pointer () {
		Window rw,cw; int rx,ry,wx,wy; unsigned mask;
		if (XQueryPointer(display,window,&rw,&cw,&rx,&ry,&wx,&wy,&mask)) report_pointer(wy,wx,mask);
		else out[0](gensym("oops"),0,0);
	}
};

/* ---------------------------------------------------------------- */

//\def 0 raise () {
	//Window root,parent1; Window *children; unsigned int nchildren;
	//if (XQueryTree(display,window,&root,&parent1,&children,&nchildren)) XFree(children); else post("no parent1");
	//XRaiseWindow(display,parent1);
	//XWindowChanges changes; memset(&changes,0,sizeof(XWindowChanges)); /* avoid valgrind undef */
	//changes.stack_mode = Above;
	//XReconfigureWMWindow(display,window,DefaultScreen(display),CWStackMode,&changes);
	//XConfigureWindow(display,window,CWStackMode,&changes);
	//XFlush(display);
//}

// from Motif:
#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_HINTS_INPUT_MODE    (1L << 2)
#define MWM_HINTS_STATUS        (1L << 3)

#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)

#define MWM_DECOR_ALL           (1L << 0)
#define MWM_DECOR_BORDER        (1L << 1)
#define MWM_DECOR_RESIZEH       (1L << 2)
#define MWM_DECOR_TITLE         (1L << 3)
#define MWM_DECOR_MENU          (1L << 4)
#define MWM_DECOR_MINIMIZE      (1L << 5)
#define MWM_DECOR_MAXIMIZE      (1L << 6)

\def 0 border (bool toggle=1) {
    border=toggle;
    XA_MotifHints = XInternAtom(display, "_MOTIF_WM_HINTS", 0);
    memset(&motifWmHints, 0, sizeof(MotifWmHints));
    motifWmHints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    motifWmHints.functions = MWM_FUNC_MOVE|MWM_FUNC_MINIMIZE;
    if (toggle) {motifWmHints.decorations = MWM_DECOR_ALL;}
    XChangeProperty(display,window,XA_MotifHints,XA_MotifHints,32,PropModeReplace,(unsigned char *)&motifWmHints,5);
/*
    Atom XA_INIT(_WIN_LAYER);
    XClientMessageEvent xev;
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.display = display;
    xev.window = window;
    xev.message_type = XA_WIN_LAYER;
    xev.format = 32;
    xev.data.l[0] = 10;
    xev.data.l[1] = CurrentTime;
    XSendEvent(display,root_window,False,SubstructureNotifyMask,(XEvent *)&xev);
*/
    XFlush(display);
}

void FormatX11::show_section(int x, int y, int sx, int sy) {
	if ((mode&2)==0) return;
	int zy=dim[0], zx=dim[1];
	if (y>zy||x>zx) return;
	if (y+sy>zy) sy=zy-y;
	if (x+sx>zx) sx=zx-x;
	if (shared_memory) {
		XSync(display,False);
		XShmPutImage(display,window,imagegc,ximage,x,y,x,y,sx,sy,False);
		XFlush(display);
		// should completion events be waited for? looks like a bug
	} else {
		XPutImage(display,window,imagegc,ximage,x,y,x,y,sx,sy);
		XFlush(display);
	}
}

/* window manager hints, defines the window as non-resizable */
void FormatX11::set_wm_hints () {
	if (!is_owner) return;
	XWMHints wmh;
	char buf[256],*bufp=buf;
	if (title=="") sprintf(buf,"GridFlow (%d,%d,%d)",dim[0],dim[1],dim[2]);
	else           sprintf(buf,"%.255s",title.data());
	XTextProperty wtitle; XStringListToTextProperty((char **)&bufp, 1, &wtitle);
	XSizeHints sh;
	sh.flags=PSize|PMaxSize|PMinSize;
	sh.min_width  = sh.max_width  = sh.width  = dim[1];
	sh.min_height = sh.max_height = sh.height = dim[0];
	wmh.input = True;
	wmh.flags = InputHint;
	XSetWMProperties(display,window,&wtitle,&wtitle,0,0,&sh,&wmh,0);
	XFree(wtitle.value); // do i really have to do that?
}

void FormatX11::call() {
	XEvent e;
	for (;;) {
		int xpending = XEventsQueued(display, QueuedAfterFlush);
		if (!xpending) break;
		XNextEvent(display,&e);
		switch (e.type) {
		case Expose:{
			XExposeEvent &ex = e.xexpose;
			if (mode==2) show_section(ex.x,ex.y,ex.width,ex.height);
		}break;
		case ButtonPress:  {XButtonEvent &eb = e.xbutton; report_pointer(eb.y,eb.x,eb.state | (128<<eb.button));}break;
		case ButtonRelease:{XButtonEvent &eb = e.xbutton; report_pointer(eb.y,eb.x,eb.state &~(128<<eb.button));}break;
		case KeyPress:
		case KeyRelease:{
			XKeyEvent &ek = e.xkey; // what was XLookupString(ek,buf,63,0,0); supposed to be for ?
			char *kss = XKeysymToString(XLookupKeysym(&ek,0));
			char buf[64];
			if (!kss) return; /* unknown keys ignored */
			if (isdigit(*kss)) sprintf(buf,"D%s",kss); else strcpy(buf,kss);
			t_symbol *sel = gensym(const_cast<char *>(e.type==KeyPress ? "keypress" : "keyrelease"));
			t_atom2 at[4] = {ek.y,ek.x,ek.state,gensym(buf)};
			out[0](sel,4,at);
		}break;
		case MotionNotify:{
			XMotionEvent &em = *(XMotionEvent *)&e;
			report_pointer(em.y,em.x,em.state);
		}break;
		case DestroyNotify:{
			post("This window is being closed, so this handler will close too!");
			delete this; /* really! what else could i do here anyway? */
			return;
		}break;
		case ConfigureNotify:break; // as if we cared
		}
	}
	clock_delay(clock,20);
}
void FormatX11_call(FormatX11 *p) {p->call();}

/* loathe Xlib's error handlers */
static FormatX11 *current_x11;
static int FormatX11_error_handler (Display *d, XErrorEvent *xee) {
	post("XErrorEvent: type=0x%08x display=%p xid=0x%08lx\n... serial=0x%08lx error=0x%08x request=0x%08x minor=0x%08x",
		xee->type, xee->display, long(xee->resourceid), xee->serial, xee->error_code, xee->request_code, xee->minor_code);
	if (current_x11->shared_memory==1) {
		post("(note: turning shm off)");
		current_x11->shared_memory = 0;
	}
	return 42; /* it seems that the return value is ignored. */
}

bool FormatX11::alloc_image (int sx, int sy) {
	dim = Dim(sy,sx,3);
	dealloc_image();
	if (sx==0 || sy==0) return false;
	current_x11 = this;
	if (!shared_memory) {
		ximage = XCreateImage(display,visual,depth,ZPixmap,0,0,sx,sy,8,0);
		int size = ximage->bytes_per_line*ximage->height;
		if (!ximage) RAISE("can't create image");
		image = new uint8[size];
		ximage->data = (int8 *)image;
	} else {
		shm_info = new XShmSegmentInfo;
		ximage = XShmCreateImage(display,visual,depth,ZPixmap,0,shm_info,sx,sy);
                if (!ximage) {post("x11: will retry without shared memory"); shared_memory=false;}
		XSync(display,0);
		if (!shared_memory) return alloc_image(sx,sy);
		int size = ximage->bytes_per_line*ximage->height;
		shm_info->shmid = shmget(IPC_PRIVATE,size,IPC_CREAT|0777);
		if(shm_info->shmid < 0) RAISE("shmget() failed: %s",strerror(errno));
		ximage->data = shm_info->shmaddr = (char *)shmat(shm_info->shmid,0,0);
		if ((long)(shm_info->shmaddr) == -1) RAISE("shmat() failed: %s",strerror(errno));
		image = (uint8 *)ximage->data;
		shm_info->readOnly = False;
		if (!XShmAttach(display, shm_info)) RAISE("ERROR: XShmAttach: big problem");
		XSync(display,0); // make sure the server picks it up
		shmctl(shm_info->shmid,IPC_RMID,0); // yes, this can be done now. should cause auto-cleanup.
		if (!shared_memory) return alloc_image(sx,sy);
	}
	int status = XInitImage(ximage);
	if (status!=1) post("XInitImage returned: %d", status);
	//_L_ fprintf(stderr,"alloc_image: %p %p\n",ximage,image);
	return true;
retry:
	post("couldn't allocate image buffer for output... retrying...");
	return alloc_image(sx,sy);
}

void FormatX11::dealloc_image () {
	if (!ximage) return;
	if (!shared_memory) {
		XFree(ximage); ximage=0; image=0;
	} else {
		shmdt(ximage->data);
		XShmDetach(display,shm_info);
		if (shm_info) {delete shm_info; shm_info=0;}
		XFree(ximage);
		ximage = 0;
		image = 0;
	}
}

void FormatX11::resize_window (int sx, int sy) {
	if (sy<minsy) sy=minsy; if (sy>4096) RAISE("height too big");
	if (sx<minsx) sx=minsx; if (sx>4096) RAISE("width too big");
	alloc_image(sx,sy);
	if (window) {
		if (is_owner && !lock_size) {
			set_wm_hints();
			XResizeWindow(display,window,sx,sy);
		}
	} else {
		XSetWindowAttributes xswa;
		xswa.do_not_propagate_mask = 0; //?
		xswa.override_redirect = override_redirect; //#!@#$
		window = XCreateWindow(display, parent, pos[1], pos[0], sx, sy, 0,
			CopyFromParent, InputOutput, CopyFromParent, CWOverrideRedirect|CWDontPropagate, &xswa);
		if(!window) RAISE("can't create window");
		set_wm_hints();
		XSelectInput(display, window, ExposureMask|StructureNotifyMask|PointerMotionMask|
			ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|KeyPressMask|KeyReleaseMask);
		if (is_owner) XMapRaised(display, window);
		if (visual->c_class == PseudoColor) prepare_colormap();
	}
	if (!imagegc) imagegc = XCreateGC(display, window, 0, NULL);
	XSync(display,0);
}

GRID_INLET(0) {
	if (in.dim.n!=3)                  RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in.dim[2]!=3 && in.dim[2]!=4) RAISE("expecting 3 or 4 channels: red,green,blue,ignored (got %d)",in.dim[2]);
	int sx = in.dim[1], osx = dim[1];
	int sy = in.dim[0], osy = dim[0];
	in.set_chunk(1);
	if (sx!=osx || sy!=osy) resize_window(sx,sy);
	if (in.dim[2]!=bit_packing->size) {
		bit_packing->mask[3]=0;
		bit_packing = new BitPacking(bit_packing->endian, bit_packing->bytes, in.dim[2], bit_packing->mask);
	}
} GRID_FLOW {
	int bypl = ximage->bytes_per_line;
	int sxc = in.dim.prod(1);
	int sx = in.dim[1];
	int y = in.dex/sxc;
	for (; n>0; y++, data+=sxc, n-=sxc) {
		// convert line
		if (use_stripes) {
			int o=y*bypl;
			for (int x=0, i=0, k=y%3; x<sx; x++, i+=3, k=(k+1)%3) {image[o+x] = (k<<6) | data[i+k]>>2;}
		} else bit_packing->pack(sx, data, image+y*bypl);
	}
} GRID_FINISH {
	show_section(0,0,in.dim[1],in.dim[0]);
} GRID_END

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
	display = XOpenDisplay(disp_string);
	XA_INIT(_NET_WM_STATE);
	XA_INIT(_NET_WM_STATE_FULLSCREEN);
	if(!display) RAISE("ERROR: opening X11 display: %s",strerror(errno));
	// btw don't expect too much from Xlib error handling.
	// Xlib, you are so free of the ravages of intelligence...
	XSetErrorHandler(FormatX11_error_handler);
	Screen *screen = DefaultScreenOfDisplay(display);
	visual   = DefaultVisual(display,DefaultScreen(display));
	root_window = DefaultRootWindow(display);
	depth    = DefaultDepthOfScreen(screen);
	colormap = 0;
	switch(visual->c_class) {
	case TrueColor: case DirectColor: break; // no colormap
	case PseudoColor: if (depth!=8) RAISE("ERROR: with colormap, only supported depth is 8 (got %d)", depth); break;
	default: RAISE("ERROR: visual type not supported (got %d)", visual->c_class);
	}
	shared_memory = !! XShmQueryExtension(display);
}

Window FormatX11::search_window_tree (Window xid, Atom key, const char *value, int level) {
	if (level>2) return 0xDeadBeef;
	Window root, parent;
	Window *children;
	unsigned int nchildren;
	XQueryTree(display,xid,&root,&parent,&children,&nchildren);
	Window target = 0xDeadBeef;
	for (int i=0; i<(int)nchildren; i++) {
		Atom actual_type;
		int actual_format;
		unsigned long nitems, bytes_after;
		unsigned char *prop;
		XGetWindowProperty(display,children[i],key,0,666,0,AnyPropertyType,&actual_type,&actual_format,&nitems,&bytes_after,&prop);
		uint32 value_l = strlen(value);
		bool match = prop && nitems>=value_l && strncmp((char *)prop+nitems-value_l,value,value_l)==0;
		XFree(prop);
		if (match) {target=children[i]; break;}
		target = search_window_tree(children[i],key,value,level+1);
		if (target != 0xDeadBeef) break;
	}
	if (children) XFree(children);
	return target;
}

\end class FormatX11 {install_format("#io.x11",6,"");}
extern "C" void gridflow_x11_setup () {
	post("GridFlow x11 module loaded");
	\startall
}

