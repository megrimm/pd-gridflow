/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003,2004 by Mathieu Bouchard

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

#include "../base/grid.h.fcs"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <SDL/SDL.h>

static bool in_use = false;

\class FormatSDL < Format
struct FormatSDL : Format {
	SDL_Surface *screen;
	P<BitPacking> bit_packing;
	P<Dim> dim;

	void resize_window (int sx, int sy);
	void alarm ();

	Pt<uint8> pixels () {
		return Pt<uint8>((uint8 *)screen->pixels,
			dim->prod(0,1)*bit_packing->bytes);
	}

	\decl void initialize (Symbol mode);
	\decl void close ();
	GRINLET3(0);
};

static void FormatSDL_alarm(FormatSDL *self) { self->alarm(); }

void FormatSDL::alarm() {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {}
}

void FormatSDL::resize_window (int sx, int sy) {
//	gfpost("switching to size (%d,%d)",sy,sx);
	int32 v[] = {sy,sx,3};
	dim = new Dim(3,v);
	screen = SDL_SetVideoMode(v[1],v[0],0,SDL_SWSURFACE);
	if (!screen)
		RAISE("Can't switch to (%d,%d,%dbpp): %s", v[0],v[1],24, SDL_GetError());
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
	if (sx!=osx || sy!=osy) resize_window(sx,sy);
} GRID_FLOW {
	int bypl = screen->pitch;
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1);
	int y = in->dex / sxc;

	assert((in->dex % sxc) == 0);
	assert((n       % sxc) == 0);

	if (SDL_MUSTLOCK(screen)) if (SDL_LockSurface(screen) < 0) return; //???
	for (; n>0; y++, data+=sxc, n-=sxc) {
		/* convert line */
		bit_packing->pack(sx, data, pixels()+y*bypl);
	}
	if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
} GRID_FINISH {
	int sy = in->dim->get(0);
	int sx = in->dim->get(1);
	SDL_UpdateRect(screen, 0, 0, sx, sy);
} GRID_END

\def void close () {
	rb_funcall(EVAL("$tasks"),SI(delete), 1, PTR2FIX(this));
	in_use=false;
}

\def void initialize (Symbol mode) {
	dim=0;screen=0;
	rb_call_super(argc,argv);
	if (in_use) RAISE("only one FormatSDL object at a time; sorry");
	in_use=true;
	if (SDL_Init(SDL_INIT_VIDEO)<0)
		RAISE("SDL_Init() error: %s",SDL_GetError());
	atexit(SDL_Quit);
	resize_window(320,240);
	SDL_PixelFormat *f = screen->format;
	uint32 mask[3] = {f->Rmask,f->Gmask,f->Bmask};
	switch (f->BytesPerPixel) {
	case 1: RAISE("8 bpp not supported"); break;
	case 2: case 3: case 4:
		bit_packing = new BitPacking(is_le(),f->BytesPerPixel,3,mask);
		break;
	default: RAISE("%d bytes/pixel: how do I deal with that?",f->BytesPerPixel); break;
	}
	rb_funcall(EVAL("$tasks"),SI([]=), 2, PTR2FIX(this), PTR2FIX((void *)FormatSDL_alarm));
}

GRCLASS(FormatSDL,LIST(GRINLET2(FormatSDL,0)),
\grdecl
){
	IEVAL(rself,"install 'FormatSDL',1,1;"
	"conf_format 2,'sdl','Simple Directmedia Layer'");
}
\end class FormatSDL
