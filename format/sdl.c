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
#include <signal.h>

//#include <SDL/SDL_video.h>
#include <SDL/SDL.h>

static bool in_use = false;

struct FormatSDL : Format {
	SDL_Surface *screen;
	BitPacking *bit_packing;
	Dim *dim;

	void resize_window (int sx, int sy);

	Pt<uint8> pixels () {
		return Pt<uint8>((uint8 *)screen->pixels,
			dim->prod(0,1)*bit_packing->bytes);
	}

	DECL3(initialize);
	DECL3(close);
	GRINLET3(0);
};

void FormatSDL::resize_window (int sx, int sy) {
	//???
}

GRID_INLET(FormatSDL,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels: red,green,blue (got %d)",in->dim->get(2));
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1), osx = dim->get(1);
	int sy = in->dim->get(0), osy = dim->get(0);
	in->set_factor(sxc);
//	if (sx!=osx || sy!=osy) resize_window(sx,sy);
}

GRID_FLOW {
	int bypl = screen->pitch;
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1);
	int y = in->dex / sxc;

	assert((in->dex % sxc) == 0);
	assert((n       % sxc) == 0);

	if (SDL_MUSTLOCK(screen)) {
		if (SDL_LockSurface(screen) < 0) return; //???
	}

	while (n>0) {
		/* gfpost("bypl=%d sxc=%d sx=%d y=%d n=%d",bypl,sxc,sx,y,n); */
		/* convert line */
		bit_packing->pack(sx, data, pixels()+y*bypl);
		y++;
		data += sxc;
		n -= sxc;
	}

    if (SDL_MUSTLOCK(screen)) {
        SDL_UnlockSurface(screen);
    }
}

GRID_FINISH {
	int sy = in->dim->get(0);
	int sx = in->dim->get(1);
    SDL_UpdateRect(screen, 0, 0, sx, sy);
}
GRID_END

METHOD3(FormatSDL,close) {
	return Qnil;
}

METHOD3(FormatSDL,initialize) {
	rb_call_super(argc,argv);
	argv++, argc--;
	if (argc>0) RAISE("too many arguments");
	if (SDL_Init(SDL_INIT_VIDEO)<0)
		RAISE("SDL_Init() error: %s",SDL_GetError());
	signal(11,SIG_DFL); // leave me alone
	atexit(SDL_Quit);
	screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
	if (!screen)
		RAISE("Can't switch to (%d,%d,%dbpp): %s", 480, 640, 16, SDL_GetError());
	int32 v[] = {480,640,3};
	dim = new Dim(3,v);

	SDL_PixelFormat *f = screen->format;
	uint32 mask[3] = {f->Rmask,f->Gmask,f->Bmask};
	switch (screen->format->BytesPerPixel) {
	case 1: RAISE("8 bpp not supported"); break;
	case 2: /* Certainement 15 ou 16 bpp */
		bit_packing = new BitPacking(2,f->BytesPerPixel,3,mask);
		break;
	case 3: /* 24 bpp lent et généralement pas utilisé */
		bit_packing = new BitPacking(3,f->BytesPerPixel,3,mask);
		break;
	case 4: /* Probablement 32 bpp alors */
		bit_packing = new BitPacking(4,f->BytesPerPixel,3,mask);
		break;
	default: RAISE("unknown bpp"); break;
	}
	return Qnil;
}

static void startup (GridClass *self) {
	IEVAL(self->rubyclass,
	"conf_format 2,'sdl','Simple Directmedia Layer'");
}

GRCLASS(FormatSDL,"FormatSDL",
inlets:1,outlets:1,startup:startup,LIST(GRINLET(FormatSDL,0,4)),
DECL(FormatSDL,initialize),
DECL(FormatSDL,close))

