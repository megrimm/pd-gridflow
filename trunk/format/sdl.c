/*
	$Id$

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

#include "../gridflow.h.fcs"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <SDL/SDL.h>

struct FormatSDL;
void FormatSDL_call(FormatSDL *self);
static bool in_use = false;
static bool full_screen = false;
static int mousex,mousey,mousem;
SDL_Surface *screen;
FObject *instance;

static void report_pointer () {
	t_atom a[3];
	SETFLOAT(a+0,mousey);
	SETFLOAT(a+1,mousex);
	SETFLOAT(a+2,mousem);
	outlet_anything(instance->bself->outlets[0],gensym("position"),COUNT(a),a);
}

static void HandleEvent () {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		    case SDL_KEYDOWN: case SDL_KEYUP: {
			int key = event.key.keysym.sym;
			int mod = event.key.keysym.mod;
			if (key==SDLK_F11) {full_screen = !full_screen; SDL_WM_ToggleFullScreen(screen); break;}
			t_symbol *sel = gensym(const_cast<char *>(event.type==SDL_KEYDOWN ? "keypress" : "keyrelease"));
			t_atom at[4];
			mousem &= ~0xFF;
			mousem |= mod;
			SETFLOAT(at+0,mousey);
			SETFLOAT(at+1,mousex);
			SETFLOAT(at+2,mousem);
			SETSYMBOL(at+3,gensym(const_cast<char *>("huh")));
			outlet_anything(instance->bself->outlets[0],sel,4,at);
		    } break;
		    case SDL_MOUSEBUTTONDOWN: SDL_MOUSEBUTTONUP: {
			if (SDL_MOUSEBUTTONDOWN) mousem |=  (128<<event.button.button);
			else                     mousem &= ~(128<<event.button.button);
			report_pointer();
		    } break;
		    case SDL_MOUSEMOTION: {
			mousey = event.motion.y;
			mousex = event.motion.x;
			report_pointer();
		    } break;
		    case SDL_VIDEORESIZE: {
		    } break;
		}
	}
}

\class FormatSDL : Format {
	P<BitPacking> bit_packing;
	P<Dim> dim;
	t_clock *clock;
	void resize_window (int sx, int sy);
	void call ();
	\decl 0 setcursor (int shape);
	\decl 0 hidecursor ();
	\constructor (t_symbol *mode) {
		dim=0;screen=0;
		if (in_use) RAISE("only one FormatSDL object at a time; sorry");
		in_use=true;
		if (SDL_Init(SDL_INIT_VIDEO)<0) RAISE("SDL_Init() error: %s",SDL_GetError());
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
		instance=this;
		clock = clock_new(this,(t_method)FormatSDL_call);
		clock_delay(clock,0);
	}
	\grin 0 int
	~FormatSDL () {
		clock_unset(clock);
		clock_free(clock);
		SDL_Quit();
		instance=0;
		in_use=false;
	}
};

void FormatSDL::call() {HandleEvent(); clock_delay(clock,20);}
void FormatSDL_call(FormatSDL *self) {self->call();}

void FormatSDL::resize_window (int sx, int sy) {
	dim = new Dim(sy,sx,3);
	screen = SDL_SetVideoMode(sx,sy,0,SDL_SWSURFACE);
	if (!screen)
		RAISE("Can't switch to (%d,%d,%dbpp): %s", sy,sx,24, SDL_GetError());
}

GRID_INLET(FormatSDL,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels: red,green,blue (got %d)",in->dim->get(2));
	int sx = in->dim->get(1), osx = dim->get(1);
	int sy = in->dim->get(0), osy = dim->get(0);
	in->set_chunk(1);
	if (sx!=osx || sy!=osy) resize_window(sx,sy);
} GRID_FLOW {
	int bypl = screen->pitch;
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1);
	int y = in->dex / sxc;
	if (SDL_MUSTLOCK(screen)) if (SDL_LockSurface(screen) < 0) return; //???
	for (; n>0; y++, data+=sxc, n-=sxc) {
		/* convert line */
		bit_packing->pack(sx, data, (uint8 *)screen->pixels+y*bypl);
	}
	if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
} GRID_FINISH {
	SDL_UpdateRect(screen,0,0,in->dim->get(1),in->dim->get(0));
} GRID_END

\def 0 setcursor  (int shape) {SDL_ShowCursor(SDL_ENABLE);}
\def 0 hidecursor ()          {SDL_ShowCursor(SDL_DISABLE);}

\end class FormatSDL {install_format("#io.sdl",2,"");}
void startup_sdl () {
	\startall
}
