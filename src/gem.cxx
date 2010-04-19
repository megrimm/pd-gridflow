/*
	$Id: gem.c 4621 2009-11-01 21:18:17Z matju $

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
#include <GL/gl.h>

/* summarising GEM's headers: GemState.h and GemPixUtil.h */
struct imageStruct {
  imageStruct(); ~imageStruct();
  unsigned char*   allocate(size_t size);  unsigned char*   allocate();
  unsigned char* reallocate(size_t size);  unsigned char* reallocate();
  void clear();
  GLint xsize, ysize, csize;
  GLenum type, format;
  int notowned;
  void copy2Image(imageStruct *to) const;
  void copy2ImageStruct(imageStruct *to) const; // copy the imageStruct (but not the actual data)
  void refreshImage(imageStruct *to);
  void swapRedBlue ();
  void convertTo  (imageStruct*to,   GLenum dest_format=0);
  void convertFrom(imageStruct*from, GLenum dest_format=0);
  unsigned char *data;
  private:
  unsigned char *pdata;
  size_t    datasize;
  public:
  GLboolean upsidedown;
};
#ifdef __WIN32__
#define GEM_VECTORALIGNMENT 128
imageStruct::imageStruct() : type(GL_UNSIGNED_BYTE), format(GL_RGBA), notowned(0),data(NULL),pdata(NULL),datasize(0), upsidedown(0) {}
imageStruct::~imageStruct() {}
void imageStruct::clear() {if (pdata) delete[] pdata; data=pdata=0; datasize=0;}
unsigned char *imageStruct::allocate() {return allocate(xsize*ysize*csize);}
unsigned char *imageStruct::allocate(size_t size) {
  if (pdata) {delete [] pdata; pdata=0;}
  size_t array_size= size+(GEM_VECTORALIGNMENT/8-1);
  try {pdata = new unsigned char[array_size];}
  catch (const std::bad_alloc &e) {error("out of memory!"); data=pdata=0; datasize=0; return 0;}
  size_t alignment = (reinterpret_cast<size_t>(pdata))&(GEM_VECTORALIGNMENT/8-1);
  size_t offset    = (alignment == 0?0:(GEM_VECTORALIGNMENT/8-alignment));
  data = pdata+offset;
  datasize=array_size-offset;
  notowned=0;
  return data; 
}
#endif
struct pixBlock {
  pixBlock();
  imageStruct image;
  int newimage, newfilm;
};
#ifdef __WIN32__
pixBlock::pixBlock() : newimage(0), newfilm(0) {}
#endif
class TexCoord {
 public:
  TexCoord() : s(0.f), t(0.f) {}
  TexCoord(float s_, float t_) : s(s_), t(t_) {}
  float s,t;
};

#if 0 /* unused GemState fields */
  TexCoord *texCoords;
  int numTexCoords, multiTexUnits;
  float tickTime;
  GLenum drawType;
  int stackDepth[4];
  int VertexDirty;
  GLfloat *VertexArray;   int VertexArraySize; int VertexArrayStride;
  GLfloat *ColorArray;    int HaveColorArray;
  GLfloat *NormalArray;   int HaveNormalArray;
  GLfloat *TexCoordArray; int HaveTexCoordArray;
  float texCoordX(int num) const {if (texture && numTexCoords > num) return texCoords[num].s; else return 0.;}
  float texCoordY(int num) const {if (texture && numTexCoords > num) return texCoords[num].t; else return 0.;}
#endif

static int gem=0;
struct GemState92 {
  int dirty, inDisplayList, lighting, smooth, texture; pixBlock *image;
  GemState92(); ~GemState92(); void reset();
};
/* you need at least one virtual dummy function in order to enable the implicit inclusion of the vtable pointer,
 * that is, C++'s class pointer. */
struct GemState93 {
  bool dirty, inDisplayList, lighting, smooth; int texture; pixBlock *image;
  GemState93(); ~GemState93(); void reset(); virtual void your_mom() = 0;
};
struct GemVersion {static const char *versionString();};
/* end of summary */

#ifdef MACOSX
#define GEM_RGBA GL_BGRA
#else
#define GEM_RGBA GL_RGBA
#endif

//  in 0: gem
//  in 1: grid
// out 0: gem
\class GridToPix : FObject {
	pixBlock m_pixBlock;
	\attr bool yflip;
	\decl 0 gem_state (...);
	void render(void *state) {
		if (gem>=93) ((GemState93 *)state)->image = &m_pixBlock;
		else         ((GemState92 *)state)->image = &m_pixBlock;
	}
	void startRendering () {m_pixBlock.newimage = 1;}
	\constructor () {
		yflip = false;
		imageStruct &im = m_pixBlock.image = imageStruct();
		im.ysize = 1;
		im.xsize = 1;
		im.csize = 4;
		im.format = GEM_RGBA;
		im.type = GL_UNSIGNED_BYTE;
		im.allocate();
		/* this is red on Linux-386, blue on OSX-386, what color on OSX-PPC ? (blue ?) */
		*(int*)im.data = 0x000000ff;
	}
	~GridToPix () {}
	\grin 1 int
};
\def 0 gem_state (...) {
	if (argc==2) {
		if (gem>=93) render((GemState93 *)(void *)argv[1]);
		else         render((GemState92 *)(void *)argv[1]);
	} else startRendering();
	outlet_anything(bself->te_outlet,gensym("gem_state"),argc,argv);
}

template <class T, class S>
static void convert_number_type(int n, T *out, S *in) {for (int i=0; i<n; i++) out[i]=(T)in[i];}
//#define NTIMES(FOO) for (; N>=4; N-=4) {FOO FOO FOO FOO} for (; N; N--) {FOO}

GRID_INLET(1) {
	if (in->dim->n != 3) RAISE("expecting 3 dimensions: rows,columns,channels");
	int c = in->dim->get(2);
	if (c!=3 && c!=4)    RAISE("expecting 3 or 4 channels (got %d)",in->dim->get(2));
	in->set_chunk(1);
	imageStruct &im = m_pixBlock.image;
	im.clear();
	im.ysize = in->dim->get(0);
	im.xsize = in->dim->get(1);
	im.type = GL_UNSIGNED_BYTE;
	switch (in->dim->get(2)) {
	    case 1: im.csize = 1; im.format = GL_LUMINANCE; break;
	    case 3: im.csize = 4; im.format = GEM_RGBA;     break;
	    case 4: im.csize = 4; im.format = GEM_RGBA;     break;
	    default: RAISE("you shouldn't see this error message.");
	}
	im.allocate();
} GRID_FLOW {
	uint8 *buf = (uint8 *)m_pixBlock.image.data;
	long sxc = in->dim->prod(1);
	long sx = in->dim->v[1];
	long chans = in->dim->get(2);
	imageStruct &im = m_pixBlock.image;
	im.upsidedown = !yflip;
	for (long y=dex/sxc; n; data+=sxc, n-=sxc, y++) {
		if (chans==3) {
			uint8 *buf2 = buf+y*sx*im.csize;
			T    *data2 = data;
			#ifdef MACOSX
			#define FOO buf2[0]=data2[2]; buf2[1]=data2[1]; buf2[2]=data2[0]; buf2[3]=255; data2+=3; buf2+=4;
			#else
			#define FOO buf2[0]=data2[0]; buf2[1]=data2[1]; buf2[2]=data2[2]; buf2[3]=255; data2+=3; buf2+=4;
			#endif
			int x;
			for (x=0; x<(sx&-4); x+=4) {FOO FOO FOO FOO}
			for (   ; x< sx    ; x++ ) {FOO}
			#undef FOO
		} else if (chans==4) {
			uint8 *buf2 = buf+y*sx*im.csize;
			T    *data2 = data;
			#ifdef MACOSX
			#define FOO buf2[0]=data2[2]; buf2[1]=data2[1]; buf2[2]=data2[0]; buf2[3]=data2[3]; data2+=4; buf2+=4;
			#else
			#define FOO buf2[0]=data2[0]; buf2[1]=data2[1]; buf2[2]=data2[2]; buf2[3]=data2[3]; data2+=4; buf2+=4;
			#endif
			int x;
			for (x=0; x<(sx&-4); x+=4) {FOO FOO FOO FOO}
			for (   ; x< sx    ; x++ ) {FOO}
			#undef FOO
			//convert_number_type(sx*4,buf+y*sx*im.csize,data);
		} else {
			//huh?
		}
	}
} GRID_FINISH {
	m_pixBlock.newimage = 1;
} GRID_END
\end class {install("#to_pix",2,1); add_creator("#export_pix");}

//------------------------------------------------------------------------

\class GridFromPix : FObject {
	P<BitPacking> bp_rgba;
	P<BitPacking> bp_bgra;
	\attr bool yflip;
	\attr NumberTypeE cast;
	int channels;
	\constructor () {
		yflip = false;
		cast = int32_e;
		_0_colorspace(0,0,gensym("rgba"));
	}
	virtual ~GridFromPix () {}
	\decl 0 gem_state (...);
	\decl 0 colorspace (t_symbol *s);
	void render(void *state) {
		pixBlock *pb = gem>=93 ?
			((GemState93 *)state)->image:
			((GemState92 *)state)->image;
		if (!pb) {::post("gemstate has no pix"); return;}
		imageStruct &im = pb->image;
		BitPacking *bp;
		switch (im.format) {
		  case GL_RGBA: bp = bp_rgba; break;
		  #ifdef GL_VERSION_1_2
		  case GL_BGRA: bp = bp_bgra; break;
		  #endif
		  default: ::post("can't produce grid from pix format %d (0x%x)",im.format,im.format); return;}
		switch (im.type) {
		  case GL_UNSIGNED_BYTE: break; /*ok*/
		  #ifdef GL_VERSION_1_2
		  case GL_UNSIGNED_INT_8_8_8_8: break; /*ok*/
		  #endif
		  default: ::post("can't produce grid from pix type %d (0x%x)",  im.type, im.type  ); return;}
		// on OSX, one was GL_UNSIGNED_INT_8_8_8_8 and the other was...?
		int32 v[] = { im.ysize, im.xsize, channels };
		GridOutlet out(this,0,new Dim(3,v),cast);
		long sxc = im.xsize*channels;
		long sy = v[0];
		bool f = yflip^im.upsidedown;
		//if (channels==4 && im.format==GL_RGBA) {
		//	for (int y=0; y<v[0]; y++) out.send(sxc,(uint8 *)im.data+sxc*(f?y:sy-1-y));
		//} else {
			#define FOO(T) {T buf[sxc]; \
			    for (int y=0; y<v[0]; y++) { \
				uint8 *data = (uint8 *)im.data+im.xsize*im.csize*(f?y:sy-1-y); \
				bp->unpack(im.xsize,data,buf); out.send(sxc,buf);}}
			TYPESWITCH(cast,FOO,)
			#undef FOO
		//}
	}
};
\def 0 colorspace (t_symbol *s) {// 3 2 1 0 (numéro de byte)
	static uint32 rgba[4] = {0x000000ff,
				 0x0000ff00,
				 0x00ff0000,
				 0xff000000};
	static uint32 bgra[4] = {0x0000ff00,
				 0x00ff0000,
				 0xff000000,
				 0x000000ff}; // really argb 
	if (s==gensym("rgb" )) {
		channels=3;
		bp_rgba = new BitPacking(is_le(),4,3,rgba);
		bp_bgra = new BitPacking(is_le(),4,3,bgra);
	} else
	if (s==gensym("rgba")) {
		channels=4;
		bp_rgba = new BitPacking(is_le(),4,4,rgba);
		bp_bgra = new BitPacking(is_le(),4,4,bgra);
	} else
	RAISE("unknown colorspace '%s'",s->s_name);
}
\def 0 gem_state (...) {
	if (argc==2) {
		if (gem>=93) render((GemState93 *)(void *)argv[1]);
		else         render((GemState92 *)(void *)argv[1]);
	}
}
\end class {install("#from_pix",1,1); add_creator("#import_pix");}

//------------------------------------------------------------------------

void startup_gem () {
	\startall
	//post("GF sizeof(imageStruct)=%d sizeof(pixBlock)=%d sizeof(GemState)=%d",sizeof(imageStruct),sizeof(pixBlock),sizeof(GemState));
	int major,minor;
	sscanf(GemVersion::versionString(),"%d.%d",&major,&minor);
	//post("GridFlow/GEM bridge : GEM version is detected to be '%s', major=%d minor=%d",GemVersion::versionString(),major,minor);
	gem = major*1000+minor;
}

/*
virtual void processRGBAImage(imageStruct &image) {}
virtual void processRGBImage (imageStruct &image) {}
virtual void processGrayImage(imageStruct &image) {}
virtual void processYUVImage (imageStruct &image) {}
*/

