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

struct FormatAALib : Format {
	aa_context *context;
//	aa_hardwareparams *hparams;
	aa_renderparams *rparams;
	Dim *dim;

	DECL3(option);
	DECL3(close);
	DECL3(initialize);
	GRINLET3(0);
};

GRID_INLET(FormatAALib,0) {
	if (!context) RAISE("boo");
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 1)
		RAISE("expecting 1 greyscale channel (got %d)",in->dim->get(2));
	if (dim) delete dim;
	dim = in->dim->dup();
	in->set_factor(in->dim->get(1)*in->dim->get(2));
}

GRID_FLOW {
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

GRID_FINISH {
	aa_palette pal;
	for (int i=0; i<256; i++) aa_setpalette(pal,i,i,i,i);
	aa_renderpalette(context,pal,rparams,0,0,
		aa_scrwidth(context),aa_scrheight(context));
	aa_flush(context);
}
GRID_END


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
		VALUE foo = rb_str_new2("");
//		for (int i=3; i<argc; i++)
//			foo
		aa_puts(context,y,x,(enum aa_attribute)a,rb_sym_name(argv[4]));
		aa_flush(context);
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
	dim = 0;
	context = 0;
	if (argc<1) RAISE("wrong number of arguments");

	int argc2=argc;
	char *argv2[argc2];
	for (int i=0; i<argc2; i++)
		argv2[i] = strdup(rb_str_ptr(rb_funcall(argv[i],SI(to_s),0)));
	aa_parseoptions(0,0,&argc,argv2);
	for (int i=0; i<argc2; i++) delete argv2[i];

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
inlets:1,outlets:1,startup:startup,LIST(GRINLET(FormatAALib,0,4)),
DECL(FormatAALib,initialize),
DECL(FormatAALib,option),
DECL(FormatAALib,close))

