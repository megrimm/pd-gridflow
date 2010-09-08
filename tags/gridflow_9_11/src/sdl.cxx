/*
	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <SDL/SDL.h>

#define pd_anything pd_typedmess

struct FormatSDL;
void FormatSDL_call(FormatSDL *self);
static int in_use = 0;
static bool full_screen = false;
static int mousex,mousey,mousem;
static Dim dim;
static t_class *sdl_global_class; /* this pointer is also a dummy pd object */

SDL_Surface *screen;

static t_symbol *keyboard[SDLK_LAST+1];

static void KEYS_ARE (int i, const char *s__) {
	char *s_ = strdup(s__);
	char *s = s_;
	while (*s) {
		char *t = strchr(s,' ');
		if (t) *t=0;
		keyboard[i] = gensym(s);
		if (!t) break;
		s=t+1; i++;
	}
	free(s_);
}

static void build_keyboard () {
	KEYS_ARE(8,"BackSpace Tab");
	KEYS_ARE(13,"Return");
	KEYS_ARE(19,"Pause");
	KEYS_ARE(27,"Escape");
	KEYS_ARE(32,"space exclam quotedbl numbersign dollar percent ampersand apostrophe");
	KEYS_ARE(40,"parenleft parenright asterisk plus comma minus period slash");
	KEYS_ARE(48,"D0 D1 D2 D3 D4 D5 D6 D7 D8 D9");
	KEYS_ARE(58,"colon semicolon less equal greater question at");
	//KEYS_ARE(65,"A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
	KEYS_ARE(91,"bracketleft backslash bracketright asciicircum underscore grave quoteleft");
	KEYS_ARE(97,"a b c d e f g h i j k l m n o p q r s t u v w x y z");
	KEYS_ARE(127,"Delete");
	KEYS_ARE(256,"KP_0 KP_1 KP_2 KP_3 KP_4 KP_5 KP_6 KP_7 KP_8 KP_9");
	KEYS_ARE(266,"KP_Delete KP_Divide KP_Multiply KP_Subtract KP_Add KP_Enter KP_Equal");
	KEYS_ARE(273,"KP_Up KP_Down KP_Right KP_Left KP_Insert KP_Home KP_End KP_Prior KP_Next");
	KEYS_ARE(282,"F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11 F12 F13 F14 F15");
	KEYS_ARE(300,"Num_Lock Caps_Lock Scroll_Lock");
	KEYS_ARE(303,"Shift_R Shift_L Control_R Control_L Alt_R Alt_L Meta_L Meta_R");
	KEYS_ARE(311,"Super_L Super_R Mode_switch Multi_key");
	KEYS_ARE(316,"Print"); // win32
	KEYS_ARE(319,"Multi_key"); // win32
}

static void report_pointer () {
	t_atom2 a[3] = {mousey,mousex,mousem};
	pd_anything(gensym("#sdl")->s_thing,gensym("position"),COUNT(a),a);
}

void resize_window (int sx, int sy) {
	dim = Dim(sy,sx,3);
	screen = SDL_SetVideoMode(sx,sy,0,SDL_SWSURFACE);
	if (!screen) RAISE("Can't switch to (%d,%d,%dbpp): %s", sy,sx,24, SDL_GetError());
}

static void HandleEvent () {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		    case SDL_KEYDOWN: case SDL_KEYUP: {
			int key = event.key.keysym.sym;
			int mod = event.key.keysym.mod;
			if (event.type==SDL_KEYDOWN && (key==SDLK_F11 || key==SDLK_ESCAPE || key=='f')) {
				full_screen = !full_screen;
				SDL_WM_ToggleFullScreen(screen);
				break;
			}
			t_symbol *sel = gensym(const_cast<char *>(event.type==SDL_KEYDOWN ? "keypress" : "keyrelease"));
			mousem &= ~0xFF;
			mousem |= mod;
			int k = event.key.keysym.sym;
			if (k<0 || k>=SDLK_LAST) RAISE("impossible key number %d, SDLK_LAST = %d",k,SDLK_LAST);
			t_atom2 at[4] = {mousey,mousex,mousem, keyboard[k] ? keyboard[k] : symprintf("unknown_%d",k)};
			pd_anything(gensym("#sdl")->s_thing,sel,4,at);
		    } break;
		    case SDL_MOUSEBUTTONDOWN: {mousem |=  (128<<event.button.button); report_pointer();} break;
		    case SDL_MOUSEBUTTONUP:   {mousem &= ~(128<<event.button.button); report_pointer();} break;
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

static t_clock *cloque;
static void start ();
static void stop ();

\class FormatSDL : Format {
	P<BitPacking> bit_packing;
	void call () {HandleEvent(); clock_delay(cloque,20);}
	static void call_(FormatSDL *self) {self->call();}
	\decl 0 setcursor (int shape) {SDL_ShowCursor(SDL_ENABLE);}
	\decl 0 hidecursor ()         {SDL_ShowCursor(SDL_DISABLE);}
	\decl 0 title (const char *title) {SDL_WM_SetCaption(title,title);}
	\decl 0 position   (...) {out[0](gensym("position"),  argc,argv);}
	\decl 0 keypress   (...) {out[0](gensym("keypress"),  argc,argv);}
	\decl 0 keyrelease (...) {out[0](gensym("keyrelease"),argc,argv);}
	\decl 0 loadbang ()      {out[0](gensym("nogrey"),0,0);}
	\constructor (t_symbol *mode) {
		pd_bind((t_pd *)bself,gensym("#sdl"));
		if (!in_use) start();
		in_use++;
		SDL_PixelFormat *f = screen->format;
		uint32 mask[3] = {f->Rmask,f->Gmask,f->Bmask};
		switch (f->BytesPerPixel) {
		case 1: RAISE("8 bpp not supported"); break;
		case 2: case 3: case 4:
			bit_packing = new BitPacking(is_le(),f->BytesPerPixel,3,mask);
			break;
		default: RAISE("%d bytes/pixel: how do I deal with that?",f->BytesPerPixel); break;
		}
		_0_title("GridFlow SDL");
	}
	\grin 0 int
	~FormatSDL () {
		pd_unbind((t_pd *)bself,gensym("#sdl"));
		if (!--in_use) stop();
	}
};
static void start () {
	screen=0;
	if (SDL_Init(SDL_INIT_VIDEO)<0) RAISE("SDL_Init() error: %s",SDL_GetError());
	atexit(SDL_Quit);
	resize_window(320,240);
	cloque = clock_new(&sdl_global_class,(t_method)FormatSDL::call_);
	clock_delay(cloque,0);
}
static void stop () {
	clock_unset(cloque);
	clock_free(cloque);
	SDL_Quit();
}

GRID_INLET(0) {
	if (in.dim.n != 3) RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in.dim[2] != 3) RAISE("expecting 3 channels: red,green,blue (got %d)",in.dim[2]);
	int sx = in.dim[1], osx = dim[1];
	int sy = in.dim[0], osy = dim[0];
	in.set_chunk(1);
	if (sx!=osx || sy!=osy) resize_window(sx,sy);
} GRID_FLOW {
	int bypl = screen->pitch;
	int sxc = in.dim.prod(1);
	int sx = in.dim[1];
	int y = in.dex/sxc;
	if (SDL_MUSTLOCK(screen)) if (SDL_LockSurface(screen) < 0) return; //???
	for (; n>0; y++, data+=sxc, n-=sxc) {
		/* convert line */
		bit_packing->pack(sx, data, (uint8 *)screen->pixels+y*bypl);
	}
	if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
} GRID_FINISH {
	SDL_UpdateRect(screen,0,0,in.dim[1],in.dim[0]);
} GRID_END

\end class FormatSDL {install_format("#io.sdl",2,"");}
void startup_sdl () {
	\startall
	build_keyboard();
	sdl_global_class = class_new(gensym("#io.sdl.global"),0,0,sizeof(t_pd),CLASS_DEFAULT,A_NULL);
}
