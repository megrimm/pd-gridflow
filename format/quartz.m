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

#include <stdio.h>
#include <objc/Object.h>

/* wrapping name conflict */
#define T_DATA T_COCOA_DATA
#include <Cocoa/Cocoa.h>
#undef T_DATA

#include "../base/grid.h.fcs"

@interface GFView: NSView {
	char *gfdata;
	int gfwidth;
	int gfheight;
}
- (id) drawRect: (NSRect)rect;
@end

@implementation GFView
- (id) initWithFrame: (NSRect)r {
	[super initWithFrame: r];
	gfwidth=256;
	gfheight=256;
	char *p = gfdata = (char *) malloc(gfheight*gfwidth*4);
	int i,j;
	for (i=0; i<gfheight; i++) {
		for (j=0; j<gfwidth; j++) {
			*p++ = 255;
			*p++ = i;
			*p++ = i^j;
			*p++ =   j;
		}
	}
	return self;
}	

- (id) drawRect: (NSRect)rect {
	fprintf(stderr,"drawRect: {%d,%d,%d,%d}\n",
//	fprintf(stderr,"drawRect: {%g,%g,%g,%g}\n",
		rect.origin.x, rect.origin.y,
		rect.size.width, rect.size.height);
	CGContextRef g = (CGContextRef)
		[[NSGraphicsContext graphicsContextWithWindow: [self window]]
			graphicsPort];
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	CGDataProviderRef dp = CGDataProviderCreateWithData(
		NULL, gfdata, gfheight*gfwidth*4, NULL);
	fprintf(stderr,"gfdata=%08lx dp=%08lx\n",(long)gfdata,(long)dp);
	CGImageRef image = CGImageCreate(gfwidth, gfheight, 8, 32, gfwidth*4, 
		cs, kCGImageAlphaFirst, dp, NULL, 0, kCGRenderingIntentDefault);
	CGDataProviderRelease(dp);
	CGColorSpaceRelease(cs);
	CGRect rectangle = CGRectMake(0,0,gfwidth,gfheight);
	CGContextDrawImage(g,rectangle,image);
	CGImageRelease(image);
	[super drawRect: rect];
	return self;
}
@end

\class FormatQuartz < Format
struct FormatQuartz : Format {
	NSWindow *window;
	NSDate *distantFuture;
	\decl void initialize (Symbol mode);
	\decl void tick ();
	GRINLET3(0);
};

GRID_INLET(FormatQuartz,0) {

} GRID_FLOW {
} GRID_FINISH {
} GRID_END

\def void tick () {
	NSEvent *e = [NSApp nextEventMatchingMask: NSAnyEventMask
		untilDate: distantFuture inMode: NSDefaultRunLoopMode
		dequeue: YES];
	//NSLog(@"%@", e);
	[NSApp sendEvent: e];
}

\def void initialize (Symbol mode) {
	rb_call_super(argc,argv);
	NSRect r = {{0,0}, {256,256}};
	window = [[NSWindow alloc]
		initWithContentRect: r
		styleMask: (NSTitledWindowMask |
		NSMiniaturizableWindowMask | NSResizableWindowMask)
		backing: NSBackingStoreNonretained
		defer: NO];
	[window setContentView: [[GFView alloc] initWithFrame: r]];
	[window setAutodisplay: YES];
	[window setTitle: @"GridFlow"];
	[window makeKeyAndOrderFront: NSApp];
	[NSApp finishLaunching];
	[NSApp activateIgnoringOtherApps: YES];
	[window orderFrontRegardless];
	distantFuture = [NSDate distantFuture];
}

GRCLASS(FormatQuartz,LIST(GRINLET2(FormatQuartz,0,4)),
\grdecl
){
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
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
