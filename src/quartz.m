/*
	$Id: quartz.m 4517 2009-10-30 16:01:30Z matju $

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

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

/*
	This is written in Objective C++, which is the union of C++ and Objective C;
	Their intersection is C or almost. They add quite different sets of features.
	I need Objective C here because the Cocoa API is for Objective C and Java only,
	and the Objective C one was the easiest to integrate in GridFlow.
*/

#include <stdio.h>
#include <objc/Object.h>
#include <Cocoa/Cocoa.h>

#include "gridflow.hxx.fcs"

class FormatQuartz;

@interface GFView: NSView {
  @private
	uint8 *imdata;
	int imwidth;
	int imheight;
  @public
	FormatQuartz *boss;
}
- (id) drawRect: (NSRect)rect;
- (id) imageHeight: (int)w width: (int)h;
- (int) imageHeight;
- (int) imageWidth;
- (uint8 *) imageData;
- (int) imageDataSize;
- (BOOL) acceptsFirstResponder;
- (void)keyDown:(NSEvent *)e;
- (void)keyUp:(NSEvent *)e;
- (void)mouseMoved:(NSEvent *)e;
- (void)mouseDown:(NSEvent *)e;
- (void)mouseDragged:(NSEvent *)e;
- (void)mouseUp:(NSEvent *)e;
- (void)rightMouseDown:(NSEvent *)e;
- (void)rightMouseDragged:(NSEvent *)e;
- (void)rightMouseUp:(NSEvent *)e;
- (void)otherMouseDown:(NSEvent *)e;
- (void)otherMouseDragged:(NSEvent *)e;
- (void)otherMouseUp:(NSEvent *)e;
- (void)scrollWheel:(NSEvent *)e;
@end

///////////////////////////////////////////////////////////////////////////////

void FormatQuartz_call(FormatQuartz *self);

\class FormatQuartz : Format {
	NSWindow *window;
	NSWindowController *wc;
	GFView *widget; /* GridFlow's Cocoa widget */
	int mouse_state;
	t_clock *clock;
	\constructor (t_symbol *mode) {
		NSRect r = {{0,0}, {320,240}};
		window = [[NSWindow alloc]
			initWithContentRect: r
			styleMask: NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask
			backing: NSBackingStoreBuffered
			defer: YES];
		widget = [[GFView alloc] initWithFrame: r];
		widget->boss = this;
		wc = [[NSWindowController alloc] initWithWindow: window];
		[window setContentView: widget];
		[window setTitle: @"GridFlow"];
		[window makeKeyAndOrderFront: nil];
		[window orderFrontRegardless];
		[window makeFirstResponder: widget];
		[window setBackgroundColor: [NSColor clearColor]];
		[window setAcceptsMouseMovedEvents: YES];
		clock = clock_new(this,(t_method)FormatQuartz_call);
		clock_delay(clock,0);
		_0_move(0,0,0,0);
		mouse_state = 0;

		// hack for proper event handling
		ProcessSerialNumber proc;
		GetCurrentProcess(&proc);
		TransformProcessType(&proc, kProcessTransformToForegroundApplication);
	}
	~FormatQuartz () {
		clock_unset(clock);
		clock_free(clock);
		clock = 0;
		[window autorelease];
		[window setReleasedWhenClosed: YES];
		[window close];
	}
	void call ();
	void report_pointer(BOOL check_bounds);
	void report_key(NSEvent *e);
	\decl 0 title (string title="");
	\decl 0 move (int y, int x);
	\decl 0 set_geometry (int y, int x, int sy, int sx);
	\decl 0 loadbang () {outlet_anything(outlets[0],gensym("nogrey"),0,0);}
	\grin 0
};

///////////////////////////////////////////////////////////////////////////////

@implementation GFView

- (uint8 *) imageData {return imdata;}
- (int) imageDataSize {return imwidth*imheight*4;}
- (int) imageHeight {return imheight;}
- (int) imageWidth {return imwidth;}

- (id) imageHeight: (int)h width: (int)w {
	if (imheight==h && imwidth==w) return self;
	imheight=h;
	imwidth=w;
	if (imdata) delete imdata;
	int size = [self imageDataSize];
	imdata = new uint8[size];
	CLEAR(imdata,size);
	NSSize s = {w,h};
	[[self window] setContentSize: s];
	return self;
}

- (id) initWithFrame: (NSRect)r {
	[super initWithFrame: r];
	imdata=0; imwidth=-1; imheight=-1;
	[self imageHeight: 240 width: 320];
	return self;
}	

- (id) drawRect: (NSRect)rect {
	[super drawRect: rect];
	if (![self lockFocusIfCanDraw]) return self;
	CGContextRef g = (CGContextRef)
		[[NSGraphicsContext graphicsContextWithWindow: [self window]]
			graphicsPort];
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	CGDataProviderRef dp = CGDataProviderCreateWithData(
		NULL, imdata, imheight*imwidth*4, NULL);
	CGImageRef image = CGImageCreate(imwidth, imheight, 8, 32, imwidth*4, 
		cs, kCGImageAlphaFirst, dp, NULL, 0, kCGRenderingIntentDefault);
	CGDataProviderRelease(dp);
	CGColorSpaceRelease(cs);
	CGRect rectangle = CGRectMake(0,0,imwidth,imheight);
	CGContextDrawImage(g,rectangle,image);
	CGImageRelease(image);
	[self unlockFocus];
	return self;
}

- (BOOL) acceptsFirstResponder {return YES;}

#define SET_MOUSE_STATE(s)   boss->mouse_state |= (s); boss->report_pointer(NO);
#define UNSET_MOUSE_STATE(s) boss->mouse_state &=~(s); boss->report_pointer(NO);

- (void)mouseDown:         (NSEvent *)e {   SET_MOUSE_STATE(256)  }
- (void)mouseUp:           (NSEvent *)e { UNSET_MOUSE_STATE(256)  }
- (void)rightMouseDown:    (NSEvent *)e {   SET_MOUSE_STATE(1024) }
- (void)rightMouseUp:      (NSEvent *)e { UNSET_MOUSE_STATE(1024) }
- (void)otherMouseDown:    (NSEvent *)e {   SET_MOUSE_STATE(512)  }
- (void)otherMouseUp:      (NSEvent *)e { UNSET_MOUSE_STATE(512)  }

- (void)mouseMoved:        (NSEvent *)e { boss->report_pointer(YES); }
- (void)mouseDragged:      (NSEvent *)e { boss->report_pointer(NO);  }
- (void)rightMouseDragged: (NSEvent *)e { boss->report_pointer(NO);  }
- (void)otherMouseDragged: (NSEvent *)e { boss->report_pointer(NO);  }

- (void)scrollWheel: (NSEvent *)e {
	if ([e deltaY] > 0) { SET_MOUSE_STATE(2048) UNSET_MOUSE_STATE(2048) }
	else if ([e deltaY] < 0) { SET_MOUSE_STATE(4096) UNSET_MOUSE_STATE(4096) }
}

- (void)keyDown: (NSEvent *)e { boss->report_key(e); }
- (void)keyUp:   (NSEvent *)e { boss->report_key(e); }

@end

////////////////////////////////////////////////////////////////////////////////

#define INSIDE_WINDOW  ((p.y >= 0 && p.y <= [this->widget imageHeight]) && (p.x >= 0 && p.x <= [this->widget imageWidth]))

void FormatQuartz::report_pointer(BOOL check_bounds) {
	NSPoint p;
	p = [this->window mouseLocationOutsideOfEventStream];
	p.y = [this->widget imageHeight] - p.y;
	
	t_atom a[3];
	SETFLOAT(a+0,p.y);
	SETFLOAT(a+1,p.x);
	SETFLOAT(a+2,this->mouse_state);

	if (!check_bounds || INSIDE_WINDOW)
		outlet_anything(outlets[0],gensym("position"),COUNT(a),a);
}

// TODO: Use KEYS_ARE from SDL...
//       Or use only keycodes to prevent CAPS and things like ! @ # $ %

void FormatQuartz::report_key(NSEvent * e) {
	NSPoint p;
	p = [this->window mouseLocationOutsideOfEventStream];
	p.y = [this->widget imageHeight] - p.y;

	const char *keyname = [[e characters] cStringUsingEncoding: [NSString defaultCStringEncoding]];
	const int keycode = [e keyCode];
	char buf[64], *keysym = NULL;

	///fprintf(stderr, "%d\n", keycode);

	switch (keycode) {
	  case 36:  keysym = "Return"; break;
	  case 51:  keysym = "BackSpace"; break;
	  case 53:  keysym = "Escape"; break;
	  case 76:  keysym = "KP_Return"; break;
	  case 18:  keysym = "D1"; break;
	  case 19:  keysym = "D2"; break;
	  case 20:  keysym = "D3"; break;
	  case 21:  keysym = "D4"; break;
	  case 23:  keysym = "D5"; break;
	  case 22:  keysym = "D6"; break;
	  case 26:  keysym = "D7"; break;
	  case 28:  keysym = "D8"; break;
	  case 25:  keysym = "D9"; break;
	  case 29:  keysym = "D0"; break;
	  case 123: keysym = "Left"; break;
	  case 124: keysym = "Right"; break;
	  case 126: keysym = "Up"; break;
	  case 125: keysym = "Down"; break;
	  case 116: keysym = "Prior"; break;
	  case 121: keysym = "Next"; break;
	  case 115: keysym = "Home"; break;
	  case 119: keysym = "End"; break;
	  case 122: keysym = "F1"; break;
	  case 120: keysym = "F2"; break;
	  case 99:  keysym = "F3"; break;
	  case 118: keysym = "F4"; break;
	  case 96:  keysym = "F5"; break;
	  case 97:  keysym = "F6"; break;
	  case 98:  keysym = "F7"; break;
	  case 27:  keysym = "minus"; break;
	  case 24:  keysym = "equal"; break;
	  case 43:  keysym = "comma"; break;
	  case 47:  keysym = "period"; break;
	  case 44:  keysym = "slash"; break;
	  case 41:  keysym = "semicolon"; break;
	  case 39:  keysym = "apostrophe"; break;
	  case 33:  keysym = "bracketleft"; break;
	  case 30:  keysym = "bracketright"; break;
	  case 42:  keysym = "backslash"; break;
	  case 49:  keysym = "space"; break;
	  case 48:  keysym = "Tab"; break;
	}

	if (!keysym) {
		if (!keyname) strcpy(buf,"unknown");
		else strcpy(buf,keyname);
	} else {
		strcpy(buf,keysym);
	}

	t_symbol *sel = gensym(const_cast<char *>([e type] == NSKeyDown ? "keypress" : "keyrelease"));
	t_atom a[4];
	SETFLOAT(a+0,p.y);
	SETFLOAT(a+1,p.x);
	SETFLOAT(a+2,this->mouse_state);
	SETSYMBOL(a+3,gensym(buf));
	outlet_anything(outlets[0],sel,4,a);
}

static NSDate *distantFuture, *distantPast;

void FormatQuartz::call() {
	BOOL wasMouseEvent = NO;
	NSEvent *e = [NSApp nextEventMatchingMask: NSAnyEventMask
		// untilDate: distantFuture // blocking
		untilDate: distantPast      // nonblocking
		inMode: NSDefaultRunLoopMode
		dequeue: YES];
	if (e) {
		//NSLog(@"%@", e);
		[NSApp sendEvent: e];
	}
	[NSApp updateWindows];
	[this->window flushWindowIfNeeded];
	clock_delay(clock,20);
}

void FormatQuartz_call(FormatQuartz *self) {self->call();}

template <class T, class S>
static void convert_number_type(int n, T *out, S *in) {
	for (int i=0; i<n; i++) out[i]=(T)in[i];
}

GRID_INLET(0) {
	if (in->dim->n!=3) RAISE("expecting 3 dims, not %d", in->dim->n);
	int c=in->dim->get(2);
	if (c!=3&&c!=4) RAISE("expecting 3 or 4 channels, not %d", in->dim->get(2));
	[widget imageHeight: in->dim->get(0) width: in->dim->get(1)];
	in->set_chunk(1);
} GRID_FLOW {
	int off = dex/in->dim->prod(2);
	int c=in->dim->get(2);
	uint8 *data2 = ((uint8 *)[widget imageData])+off*4;
//	convert_number_type(n,data2,data);
	if (c==3) {
		while(n) {
			data2[0]=255;
			data2[1]=(uint8)data[0];
			data2[2]=(uint8)data[1];
			data2[3]=(uint8)data[2];
			data+=3; data2+=4; n-=3;
		}
	} else {
		while(n) {
			data2[0]=255;
			data2[1]=(uint8)data[0];
			data2[2]=(uint8)data[1];
			data2[3]=(uint8)data[2];
			data+=4; data2+=4; n-=4;
		}
	}
} GRID_FINISH {
	NSRect r = {{0,0},{[widget imageHeight],[widget imageWidth]}};
	[widget displayRect: r];
	[widget setNeedsDisplay: YES];
	[widget display];
} GRID_END

#define MAC_MENUBARHEIGHT  22    // there must be a better way to handle this...
#define MAC_BOTTOMLEFT_TO_TOPLEFT  ([[[NSScreen screens] objectAtIndex:0] frame].size.height - MAC_MENUBARHEIGHT - y)

\def 0 title (string title="") {
	NSString *str = [[NSString alloc] initWithCString:title.c_str()];
	[window setTitle: str];
}

\def 0 move (int y, int x) {
	int new_y = MAC_BOTTOMLEFT_TO_TOPLEFT;
	NSPoint pos = { x, new_y };
	[window setFrameTopLeftPoint: pos];
}

\def 0 set_geometry (int y, int x, int sy, int sx) {
	int new_y = MAC_BOTTOMLEFT_TO_TOPLEFT - sy;
	NSRect r = {{x, new_y}, {sx, sy}};
	[window setFrame: r display: YES];
}

\end class FormatQuartz {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	distantFuture = [NSDate distantFuture];
	distantPast = [NSDate distantPast];
	[NSApplication sharedApplication];
	install_format("#io.quartz",2,"");
}
void startup_quartz () {
        \startall
}

