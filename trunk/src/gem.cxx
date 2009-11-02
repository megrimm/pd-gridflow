/*
	$Id: gem.c 4621 2009-11-01 21:18:17Z matju $

	GridFlow
	Copyright (c) 2001-2009 by Mathieu Bouchard

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
#include "Base/GemPixDualObj.h"

struct GridToPix;
struct GridToPixHelper : GemPixObj {
	GridToPix *boss;
	CPPEXTERN_HEADER(GridToPixHelper,GemPixObj)
public:
	GridToPixHelper () {}
	virtual void startRendering();
	virtual void render(GemState *state);
	virtual bool isRunnable () {return true;} // required to keep GEM 0.9.1 (the real 0.9.1) happy
};
CPPEXTERN_NEW(GridToPixHelper)
void GridToPixHelper::render(GemState *state) {}

//  in 0: gem
//  in 1: grid
// out 0: gem
\class GridToPix : FObject {
	P<BitPacking> bit_packing;
	pixBlock m_pixBlock;
	\attr bool yflip;
	\decl 0 gem_state (...);
	void render(GemState *state) {state->image = &m_pixBlock;}
	void startRendering () {m_pixBlock.newimage = 1;}
	GridToPix (BFObject *bself, MESSAGE) : FObject(bself,MESSAGE2) {
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
	~GridToPix () {}
	\grin 1 int
};
\def 0 gem_state (...) {
	if (argc==2) render((GemState *)(void *)argv[1]); else startRendering();
	outlet_anything(bself->te_outlet,gensym("gem_state"),argc,argv);
}
GRID_INLET(1) {
	if (in->dim->n != 3)      RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 4) RAISE("expecting 4 channels (got %d)",in->dim->get(2));
	in->set_chunk(1);
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
	if (yflip) {for (long y=     dex/sxc; n; data+=sxc, n-=sxc, y++) bit_packing->pack(sx,data,buf+y*sxc);}
        else       {for (long y=sy-1-dex/sxc; n; data+=sxc, n-=sxc, y--) bit_packing->pack(sx,data,buf+y*sxc);}
} GRID_END
\end class {install("#to_pix",2,1); add_creator("#export_pix");}
void GridToPixHelper::obj_setupCallback(t_class *) {}
void GridToPixHelper::startRendering() {boss->m_pixBlock.newimage = 1;}

//------------------------------------------------------------------------

\class GridFromPix : FObject {
	P<BitPacking> bit_packing;
	\attr bool yflip;
	\attr NumberTypeE cast;
	GridFromPix () : FObject(0,0,0,0) {RAISE("don't call this. this exists only to make GEM happy.");}
	GridFromPix (BFObject *bself, MESSAGE) : FObject(bself,MESSAGE2) {
		uint32 mask[4] = {0x0000ff,0x00ff00,0xff0000,0x000000};
		bit_packing = new BitPacking(is_le(),4,4,mask);
		yflip = false;
		cast = int32_e;
	}
	virtual ~GridFromPix () {}
	\decl 0 gem_state (...);
	void render(GemState *state) {
		if (!state->image) {::post("gemstate has no pix"); return;}
		imageStruct &im = state->image->image;
		if (im.format != GL_RGBA         ) {::post("can't produce grid from pix format %d",im.format); return;}
		if (im.type   != GL_UNSIGNED_BYTE) {::post("can't produce grid from pix type %d",  im.type  ); return;}
		int32 v[] = { im.ysize, im.xsize, im.csize };
		GridOutlet out(this,0,new Dim(3,v),cast);
		long sxc = im.xsize*im.csize;
		long sy = v[0];
		uint8 buf[sxc];
		for (int y=0; y<v[0]; y++) {
			uint8 *data = (uint8 *)im.data+sxc*(yflip?y:sy-1-y);
			bit_packing->pack(im.xsize,data,buf);
			// not supposed to be using pack here, but it's faster than unpack, and they're self-inverses here.
			out.send(sxc,buf);
		}
/*		#define FOO(T) { \
			uint8 buf[sxc]; \
			for (int y=0; y<v[0]; y++) { \
				uint8 *data = (uint8 *)im.data+sxc*(yflip?y:sy-1-y); \
				bit_packing->unpack(im.xsize,data,buf); \
				out.send(sxc,buf);}}
		TYPESWITCH(cast,FOO,)
		#undef FOO
*/
	}
};
\def 0 gem_state (...) {if (argc==2) render((GemState *)(void *)argv[1]);}
\end class {install("#from_pix",2,1); add_creator("#import_pix");}

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

