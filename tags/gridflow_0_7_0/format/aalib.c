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
#define aa_hardwareparams aa_hardware_params
#include <aalib.h>

/* MINNOR is a typo in aalib.h, sorry */
typedef 
#if AA_LIB_MINNOR == 2
      int
#else
      enum aa_attribute
#endif
AAAttr;

struct FormatAALib : Format {
	aa_context *context;
//	aa_hardwareparams *hparams;
	aa_renderparams *rparams;
	int autodraw; /* as for X11 */
	bool raw_mode;

	DECL3(option);
	DECL3(close);
	DECL3(initialize);
	GRINLET3(0);
};

GRID_INLET(FormatAALib,0) {
	if (!context) RAISE("boo");
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	switch (in->dim->get(2)) {
	case 1: raw_mode = false; break;
	case 2: raw_mode = true; break;
	default:
		RAISE("expecting 1 greyscale channel (got %d)",in->dim->get(2));
	}
	in->set_factor(in->dim->get(1)*in->dim->get(2));
} GRID_FLOW {
	if (raw_mode) {
		int sx = min(in->factor,aa_scrwidth(context));
		int y = in->dex/in->factor;
		while (n) {
			if (y>=aa_scrheight(context)) return;
			for (int x=0; x<sx; x++) {
				context->textbuffer[y*aa_scrwidth(context)+x]=data[x*2+0];
				context->attrbuffer[y*aa_scrwidth(context)+x]=data[x*2+1];
			}
			y++;
			n -= in->factor;
			data += in->factor;
		}
	} else {
		int sx = min(in->factor,context->imgwidth);
		int y = in->dex/in->factor;
		while (n) {
			if (y>=context->imgheight) return;
			for (int x=0; x<sx; x++) aa_putpixel(context,x,y,data[x]);
			y++;
			n -= in->factor;
			data += in->factor;
		}
	}
} GRID_FINISH {
	if (!raw_mode) {
		aa_palette pal;
		for (int i=0; i<256; i++) aa_setpalette(pal,i,i,i,i);
		aa_renderpalette(context,pal,rparams,0,0,
			aa_scrwidth(context),aa_scrheight(context));
	}
	if (autodraw==1) aa_flush(context);
} GRID_END

METHOD3(FormatAALib,close) {
	if (context) {
		aa_close(context);
		context=0;
	}
	return Qnil;
}

METHOD3(FormatAALib,option) {
	VALUE sym = argv[0];
	if (sym==SYM(hidecursor)) {
		aa_hidemouse(context);
	} else if (sym==SYM(print)) {
		if (argc!=5 || TYPE(argv[4]) != T_SYMBOL) RAISE("boo");
		int y = INT(argv[1]);
		int x = INT(argv[2]);
		int a = INT(argv[3]);
		aa_puts(context,x,y,(AAAttr)a,(char *)rb_sym_name(argv[4]));
		if (autodraw==1) aa_flush(context);
	} else if (sym == SYM(draw)) {
		aa_flush(context);
	} else if (sym == SYM(autodraw)) {
		int autodraw = INT(argv[1]);
		if (autodraw<0 || autodraw>1)
			RAISE("autodraw=%d is out of range",autodraw);
		this->autodraw = autodraw;
	} else if (sym == SYM(dump)) {
		int32 v[] = {aa_scrheight(context), aa_scrwidth(context), 2};
		out[0]->begin(new Dim(3,v));
		for (int y=0; y<aa_scrheight(context); y++) {
			for (int x=0; x<aa_scrwidth(context); x++) {
				STACK_ARRAY(int32,data,2);
				data[0] = context->textbuffer[y*aa_scrwidth(context)+x];
				data[1] = context->attrbuffer[y*aa_scrwidth(context)+x];
				out[0]->send(2,data);
			}
		}		
	} else
		return rb_call_super(argc,argv);
	return Qnil;
}

METHOD3(FormatAALib,initialize) {
	rb_call_super(argc,argv);
	for (int i=0; i<argc; i++) {
		gfpost("aalib argv[%d]=%s",i,
			rb_str_ptr(rb_funcall(argv[i],SI(inspect),0)));
	}
	argv++, argc--;
	context = 0;
	autodraw = 1;
	if (argc<1) RAISE("wrong number of arguments");

	int argc2=argc;
	char *argv2[argc2];
	for (int i=0; i<argc2; i++)
		argv2[i] = strdup(rb_str_ptr(rb_funcall(argv[i],SI(to_s),0)));
	aa_parseoptions(0,0,&argc,argv2);
	for (int i=0; i<argc2; i++) free(argv2[i]);

	Ruby drivers = rb_ivar_get(grid_class->rubyclass,SI(@drivers));
	Ruby driver_address = rb_hash_aref(drivers,argv[0]);
	if (driver_address==Qnil)
		RAISE("unknown aalib driver '%s'",rb_sym_name(argv[0]));
	aa_driver *driver = FIX2PTR(aa_driver,driver_address);
	context = aa_init(driver,&aa_defparams,0);

	rparams = aa_getrenderparams();
	if (!context) RAISE("opening aalib didn't work");
	int32 v[]={context->imgheight,context->imgwidth,1};
	gfpost("aalib image size: %s",(new Dim(3,v))->to_s());
	return Qnil;
}

static void startup (GridClass *self) {
	Ruby drivers = rb_ivar_set(self->rubyclass,SI(@drivers),rb_hash_new());
	const aa_driver * const *p = aa_drivers;
	while (*p) {
		rb_hash_aset(drivers,ID2SYM(rb_intern((*p)->shortname)),
			PTR2FIX(*p));
		p++;
	}
	
	IEVAL(self->rubyclass,
		"GridFlow.post('aalib supports: %s', "
		"@drivers.keys.join(', '))");

	IEVAL(self->rubyclass,
	"conf_format 2,'aalib','Ascii Art Library'");
}

GRCLASS(FormatAALib,"FormatAALib",
inlets:1,outlets:1,startup:startup,LIST(GRINLET2(FormatAALib,0,4)),
DECL(FormatAALib,initialize),
DECL(FormatAALib,option),
DECL(FormatAALib,close))
