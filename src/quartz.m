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

@interface GFView: NSView {
	uint8 *imdata;
	int imwidth;
	int imheight;
}
- (id) drawRect: (NSRect)rect;
- (id) imageHeight: (int)w width: (int)h;
- (int) imageHeight;
- (int) imageWidth;
- (uint8 *) imageData;
- (int) imageDataSize;
- (BOOL) acceptsFirstResponder;
@end

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

@end

/* workaround: bus error in gcc */
uint8 *GFView_imageData(GFView *self) {return (uint8 *)[self imageData];}

void GFView_imageHeight_width(GFView *self, int height, int width) {
	[self imageHeight: height width: width];
}

void GFView_display(GFView *self) {
	NSRect r = {{0,0},{[self imageHeight],[self imageWidth]}};
	[self displayRect: r];
	[self setNeedsDisplay: YES];
	[self display];
}

struct FormatQuartz;
void FormatQuartz_call(FormatQuartz *self);

\class FormatQuartz : Format {
	NSWindow *window;
	NSWindowController *wc;
	GFView *widget; /* GridFlow's Cocoa widget */
	NSPoint mouse_lastpos;
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
		[window setContentView: widget];
		[window setTitle: @"GridFlow"];
		[window makeKeyAndOrderFront: nil];
		[window orderFrontRegardless];
		wc = [[NSWindowController alloc] initWithWindow: window];
		clock = clock_new(this,(t_method)FormatQuartz_call);
		clock_delay(clock,0);
		[window makeFirstResponder: widget];
		//post("mainWindow = %08lx",(long)[NSApp mainWindow]);
		//post(" keyWindow = %08lx",(long)[NSApp keyWindow]);
		NSColor *color = [NSColor clearColor];
		[window setBackgroundColor: color];
		_0_move(0,0,0,0);
		mouse_state = 0;
		mouse_lastpos.x = 0;
		mouse_lastpos.y = 0;
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
	void report_pointer(int y, int x, int state);
	\decl 0 title (string title="");
	\decl 0 move (int y, int x);
	\decl 0 set_geometry (int y, int x, int sy, int sx);
	\grin 0
};

void FormatQuartz::report_pointer(int y, int x, int state) {
	t_atom a[3];
	SETFLOAT(a+0,y);
	SETFLOAT(a+1,x);
	SETFLOAT(a+2,state);
	outlet_anything(outlets[0],gensym("position"),COUNT(a),a);
	this->mouse_lastpos.y = y;
	this->mouse_lastpos.x = x;
}

static NSDate *distantFuture, *distantPast;

#define VALIDATE_MOUSEEVENT  if ((p.y >= 0 && p.y <= [this->widget imageHeight]) && \
                                 (p.x >= 0 && p.x <= [this->widget imageWidth])) wasMouseEvent = YES

void FormatQuartz::call() {
	BOOL wasMouseEvent = NO;
	NSPoint p, *l=&this->mouse_lastpos;
	NSEvent *e = [NSApp nextEventMatchingMask: NSAnyEventMask
		// untilDate: distantFuture // blocking
		untilDate: distantPast      // nonblocking
		inMode: NSDefaultRunLoopMode
		dequeue: YES];
		
	p = [this->window mouseLocationOutsideOfEventStream];
	p.y = [this->widget imageHeight] - p.y;
	if (p.y != l->y || p.x != l->x) // mouse moved
		VALIDATE_MOUSEEVENT;    // inside our window ?
	if (e) {
		//NSLog(@"%@", e);
		[NSApp sendEvent: e];
		switch ([e type]) {
		case NSLeftMouseDown:
			this->mouse_state += 256;  VALIDATE_MOUSEEVENT; break;
		case NSLeftMouseUp:
			this->mouse_state -= 256;  VALIDATE_MOUSEEVENT; break;
		case NSOtherMouseDown:
			this->mouse_state += 512;  VALIDATE_MOUSEEVENT; break;
		case NSOtherMouseUp:
			this->mouse_state -= 512;  VALIDATE_MOUSEEVENT; break;
		case NSRightMouseDown:
			this->mouse_state += 1024; VALIDATE_MOUSEEVENT; break;
		case NSRightMouseUp:
			this->mouse_state -= 1024; VALIDATE_MOUSEEVENT; break;
		case NSLeftMouseDragged:
		case NSRightMouseDragged:
		case NSOtherMouseDragged:
			wasMouseEvent = YES; break; // don't VALIDATE_MOUSEEVENT for when dragging outside the window
		}
	}
	if (wasMouseEvent) report_pointer((int)p.y, (int)p.x, this->mouse_state);
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
//	[widget imageHeight: in->dim->get(0) width: in->dim->get(1) ];
	GFView_imageHeight_width(widget,in->dim->get(0),in->dim->get(1));
	in->set_chunk(1);
} GRID_FLOW {
	int off = dex/in->dim->prod(2);
	int c=in->dim->get(2);
	NSView *w = widget;
	uint8 *data2 = GFView_imageData(w)+off*4;
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
	GFView_display(widget);
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

