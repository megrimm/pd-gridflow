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

/*
	This is written in Objective C++, which is the union of C++ and Objective C;
	Their intersection is C or almost. They add quite different sets of features.
	I need Objective C here because the Cocoa API is for Objective C and Java only,
	and the Objective C one was the easiest to integrate in GridFlow.

	The next best possibility may be using RubyCocoa, a port of the Cocoa API to Ruby;
	However I haven't checked whether Quartz is wrapped, and how easy it is to
	process images.
*/

#include <stdio.h>
#include <objc/Object.h>

/* wrapping name conflict */
#define T_DATA T_COCOA_DATA
#include <Cocoa/Cocoa.h>
#undef T_DATA

#include "../base/grid.h.fcs"

@interface GFView: NSView {
	uint8 *imdata;
	int imwidth;
	int imheight;
}
- (id) drawRect: (NSRect)rect;
- (id) imageHeight: (int)w width: (int)h;
- (uint8 *) imageData;
- (int) imageDataSize;
@end

@implementation GFView

- (uint8 *) imageData { return imdata; }
- (int) imageDataSize { return imwidth*imheight*4; }

- (id) imageHeight: (int)h width: (int)w {
	if (imheight==h && imwidth==w) return self;
	gfpost("new size: y=%d x=%d",h,w);
	imheight=h;
	imwidth=w;
	if (imdata) delete imdata;
	imdata = ARRAY_NEW(uint8,[self imageDataSize]);
	uint8 *p = imdata;
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			*p++ = 255;
			*p++ = 255;
			*p++ = 255;
			*p++ = 0;
		}
	}
//	NSRect r = [self frame];
//	r.size.height=h;
//	r.size.width=w;
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
L	fprintf(stderr,"drawRect: {%g,%g,%g,%g}\n",
		rect.origin.x, rect.origin.y,
		rect.size.width, rect.size.height);
	if (![self lockFocusIfCanDraw]) return self;
	CGContextRef g = (CGContextRef)
		[[NSGraphicsContext graphicsContextWithWindow: [self window]]
			graphicsPort];
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	CGDataProviderRef dp = CGDataProviderCreateWithData(
		NULL, imdata, imheight*imwidth*4, NULL);
	fprintf(stderr,"imheight=%d imwidth=%d imdata=%08lx dp=%08lx\n",
		imheight,imwidth,(long)imdata,(long)dp);
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
@end

/* workaround: bus error in gcc */
Pt<uint8> GFView_imageData(GFView *self) {
	return Pt<uint8>([self imageData], [self imageDataSize]);
}
void GFView_imageHeight_width(GFView *self, int height, int width) {
	[self imageHeight: height width: width];
}

void GFView_display(GFView *self) {
	NSRect r = {{0,0},{self->imheight,self->imwidth}};
	[self displayRect: r];
	[self setNeedsDisplay: YES];
	[self display];
}

\class FormatQuartz < Format
struct FormatQuartz : Format {
	NSWindow *window;
	NSWindowController *wc;
	GFView *widget; /* GridFlow's Cocoa widget */
	NSDate *distantFuture;
	\decl void initialize (Symbol mode);
	\decl void delete_m ();
	GRINLET3(0);
};

static NSDate *distantFuture, *distantPast;

void FormatQuartz_tick(FormatQuartz *self) {
	NSEvent *e = [NSApp nextEventMatchingMask: NSAnyEventMask
//		untilDate: distantFuture // blocking
		untilDate: distantPast // nonblocking
		inMode: NSDefaultRunLoopMode
		dequeue: YES];
	if (e) {
		NSLog(@"%@", e);
		[NSApp sendEvent: e];
	}
	[NSApp updateWindows];
	[self->window update];
}

template <class T, class S>
static void convert_number_type(int n, Pt<T> out, Pt<S> in) {
	for (int i=0; i<n; i++) out[i]=(T)in[i];
}

GRID_INLET(FormatQuartz,0) {
	if (in->dim->n!=3) RAISE("expecting 3 dims, not %d", in->dim->n);
	if (in->dim->get(2)!=3) RAISE("expecting 3 channels, not %d", in->dim->get(2));
//	[widget imageHeight: in->dim->get(0) width: in->dim->get(1) ];
	GFView_imageHeight_width(widget,in->dim->get(0),in->dim->get(1));
	in->set_factor(in->dim->prod(1));
} GRID_FLOW {
	int off = in->dex/in->dim->prod(2);
//	gfpost("off=%d n=%d",off,n);
	NSView *w = widget;
	Pt<uint8> data2 = GFView_imageData(w)+off*4;
//	convert_number_type(n,data2,data);
	while(n) {
		data2[0]=255;
		data2[1]=data[0];
		data2[2]=data[1];
		data2[3]=data[2];
		data+=3; data2+=4; n-=3;
	}
} GRID_FINISH {
	GFView_display(widget);
//	[widget display];
	FormatQuartz_tick(this);
} GRID_END

\def void initialize (Symbol mode) {
	rb_call_super(argc,argv);
	NSRect r = {{0,0}, {320,240}};
	window = [[NSWindow alloc]
		initWithContentRect: r
		styleMask: (NSTitledWindowMask |
		NSMiniaturizableWindowMask
//		| NSResizableWindowMask // can't make this one to work
		)
//		backing: NSBackingStoreNonretained
		backing: NSBackingStoreRetained
		defer: NO];
	widget = [[GFView alloc] initWithFrame: r];
	[window setContentView: widget];
//	[window setAutodisplay: YES];
	[window setTitle: @"GridFlow"];
	[window makeKeyAndOrderFront: NSApp];
//	[NSApp finishLaunching];
//	[NSApp activateIgnoringOtherApps: YES];
	[window orderFrontRegardless];
	wc = [[NSWindowController alloc]
		initWithWindow: window];
	MainLoop_add(this, (void(*)(void*)) FormatQuartz_tick);
//	[window makeMainWindow]; //doesn't work
//	[window makeKeyWindow];  //doesn't work
	[window makeFirstResponder: widget];
	gfpost("mainWindow = %08lx",(long)[NSApp mainWindow]);
	gfpost(" keyWindow = %08lx",(long)[NSApp keyWindow]);
	NSColor *color = [NSColor clearColor];
	[window setBackgroundColor: color];
}

\def void delete_m () {
L
	MainLoop_remove(this);
}

GRCLASS(FormatQuartz,LIST(GRINLET2(FormatQuartz,0,4)),
\grdecl
){
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	distantFuture = [NSDate distantFuture];
	distantPast = [NSDate distantPast];
	[NSApplication sharedApplication];
	IEVAL(rself,"install 'FormatQuartz',1,1;"
	"conf_format 6,'quartz','Apple Quartz/Cocoa'");

//	id mainMenu = [[NSMenu alloc] initWithTitle: @"GridFlow"];
//	[w makeFirstResponder: [w contentView]];//???
//	[NSApp setMainMenu: mainMenu];
//	[w orderFront: NSApp];
//	[NSApp run];

}

\end class FormatQuartz