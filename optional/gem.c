/*
	$Id$

	GridFlow
	Copyright (c) 2001-2006 by Mathieu Bouchard

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
#undef T_OBJECT
#undef EXTERN
#include "Base/GemPixDualObj.h"

//  in 0: gem
//  in 1: grid
// out 0: gem
\class GridExportPix < GridObject
struct GridExportPix : GridObject, GemPixObj {
	CPPEXTERN_HEADER(GridExportPix,GemPixObj)
public:
	P<BitPacking> bit_packing;
	pixBlock m_pixBlock;
	\attr bool yflip;

	GridExportPix () {
		yflip = false;
		imageStruct &im = m_pixBlock.image = imageStruct();
		im.ysize = 1;
		im.xsize = 1;
		im.csize = 4;
		im.format = GL_RGBA;
		im.type = GL_UNSIGNED_BYTE;
		im.allocate();
		*(int*)im.data = 0x0000ff;
		uint32 mask[4] = {0x0000ff,0x00ff00,0xff0000,0x000000};
		bit_packing = new BitPacking(is_le(),4,4,mask);
	}
	virtual ~GridExportPix () {}
	\grin 1 int

	virtual void startRendering() {m_pixBlock.newimage = 1;}
	virtual void render(GemState *state) {state->image = &m_pixBlock;}
};
CPPEXTERN_NEW(GridExportPix)

//	fprintf(stderr,"bang GridExportPix this=%p, this->x_obj=%p, bself=%p, rself=%p gemself=%p\n", this,this->x_obj,bself,(void *)rself,((Obj_header *)this->x_obj)->data);
//	fprintf(stderr,"bang GridExportPix this=%p, (GemPixObj *)this=%p\n", this,(GemPixObj *)this);

GRID_INLET(GridExportPix,1) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 4)
		RAISE("expecting 4 channels (got %d)",in->dim->get(2));
	in->set_factor(in->dim->prod(1));
	imageStruct &im = m_pixBlock.image;
	im.clear();
	im.ysize = in->dim->get(0);
	im.xsize = in->dim->get(1);
	im.csize = in->dim->get(2);
	im.type = GL_UNSIGNED_BYTE;
	im.allocate();
	im.format = GL_RGBA;
	/*
	switch (in->dim->get(2)) {
	    case 4: im.format = GL_RGBA; break;
	    default: RAISE("you shouldn't see this error message.");
	}
	*/
} GRID_FLOW {
	uint8 *buf = (uint8 *)m_pixBlock.image.data;
	/*!@#$ it would be nice to skip the bitpacking when we can */
	long sxc = in->dim->prod(1);
	long sx = in->dim->v[1];
	long sy = in->dim->v[0];
	if (yflip) {
		for (long y=     in->dex/sxc; n; data+=sxc, n-=sxc, y++)
			bit_packing->pack(sx,data,buf+y*sxc);
	} else {
		for (long y=sy-1-in->dex/sxc; n; data+=sxc, n-=sxc, y--)
			bit_packing->pack(sx,data,buf+y*sxc);
	}
} GRID_END

void GridExportPix::obj_setupCallback(t_class *) {}

\classinfo {
	IEVAL(rself,"install '#export_pix',2,1;");
	t_class *qlass = FIX2PTR(t_class,rb_ivar_get(EVAL("GridFlow::GridExportPix"),SI(@bfclass)));
	GridExportPix::real_obj_setupCallback(qlass);
}
\end class GridExportPix

//------------------------------------------------------------------------
//  in 0: gem (todo: auto 0 = manual mode; bang = send next frame; type = number type attr)
// out 0: grid
\class GridImportPix < GridObject
struct GridImportPix : GridObject, GemPixObj {
	CPPEXTERN_HEADER(GridImportPix,GemPixObj)
public:
	P<BitPacking> bit_packing;
	\attr bool yflip;
	GridImportPix () {
		uint32 mask[4] = {0x0000ff,0x00ff00,0xff0000,0x000000};
		bit_packing = new BitPacking(is_le(),4,4,mask);
		yflip = false;
	}
	\decl void initialize ();
	virtual ~GridImportPix () {}
	virtual void render(GemState *state) {
		if (!state->image) {gfpost("gemstate has no pix"); return;}
		imageStruct &im = state->image->image;
		if (im.format != GL_RGBA         ) {gfpost("can't produce grid from pix format %d",im.format); return;}
		if (im.type   != GL_UNSIGNED_BYTE) {gfpost("can't produce grid from pix type %d",  im.type  ); return;}
		int32 v[] = { im.ysize, im.xsize, im.csize };
		GridOutlet out(this,0,new Dim(3,v));
		long sxc = im.xsize*im.csize;
		long sy = v[0];
		uint8 buf[sxc];
		for (int y=0; y<v[0]; y++) {
			uint8 *data = (uint8 *)im.data+sxc*(yflip?y:sy-1-y);
			bit_packing->pack(im.xsize,data,buf);
			out.send(sxc,buf);
		}
	}
};
CPPEXTERN_NEW(GridImportPix)

\def void initialize () {
	rb_call_super(argc,argv);
}

void GridImportPix::obj_setupCallback(t_class *) {}

\classinfo {
	IEVAL(rself,"install '#import_pix',2,1;");
	t_class *qlass = FIX2PTR(t_class,rb_ivar_get(EVAL("GridFlow::GridImportPix"),SI(@bfclass)));
	GridExportPix::real_obj_setupCallback(qlass);
}
\end class GridImportPix

//------------------------------------------------------------------------

void startup_gem () {
	\startall
}

/*
virtual void processRGBAImage(imageStruct &image) {}
virtual void processRGBImage (imageStruct &image) {}
virtual void processGrayImage(imageStruct &image) {}
virtual void processYUVImage (imageStruct &image) {}
*/

