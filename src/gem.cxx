/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

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
#include "colorspace.hxx"
#include <GL/gl.h>


/* summarising GEM's headers: GemState.h and GemPixUtil.h */
#ifdef GEMSTATE93 // except Gem93 older than jan.2010 iirc
struct imageStruct {
  GLint xsize, ysize, csize; GLenum type, format; int notowned;
  unsigned char *data; unsigned char *pdata; size_t datasize; GLboolean upsidedown;
  virtual void clear(); imageStruct(); ~imageStruct();
  unsigned char *allocate(size_t size); unsigned char *allocate();
  virtual void convertFrom(imageStruct*from, GLenum dest_format=0);
  virtual void convertTo  (imageStruct*from, GLenum dest_format=0);
};
#else // older
struct imageStruct {
  GLint xsize, ysize, csize; GLenum type, format; int notowned;
  unsigned char *data; unsigned char *pdata; size_t datasize; GLboolean upsidedown;
  void clear(); imageStruct(); ~imageStruct();
  unsigned char *allocate(size_t size); unsigned char *allocate();
  void convertFrom(imageStruct*from, GLenum dest_format=0);
  void convertTo  (imageStruct*from, GLenum dest_format=0);
};
#endif
struct pixBlock {imageStruct image; int newimage, newfilm; pixBlock(){newimage=newfilm=0;}};
#ifdef IMAGESTRUCT93 // except Gem93 older than 25 mai 2010
struct GemState {bool dirty,inDisplayList,lighting,smooth; int texture; pixBlock *image;
                 GemState(); virtual ~GemState(); void reset(); char trabant[666];};
#else // older
struct GemState {int  dirty,inDisplayList,lighting,smooth; int texture; pixBlock *image;
                 GemState();         ~GemState(); void reset(); char trabant[666];};
#endif

#ifdef __WIN32__
#define GEM_VECTORALIGNMENT 128
imageStruct::imageStruct() : type(GL_UNSIGNED_BYTE), format(GL_RGBA), notowned(0),data(0),pdata(0),datasize(0), upsidedown(0) {}
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
#ifdef __WIN32__
pixBlock::pixBlock() : newimage(0), newfilm(0) {}
#endif
struct TexCoord {
  TexCoord() : s(0.f), t(0.f) {}
  TexCoord(float s_, float t_) : s(s_), t(t_) {}
  float s,t;
};
/* end of summary */

#ifdef MACOSX
#define GEM_RGBA GL_BGRA
#else
#define GEM_RGBA GL_RGBA
#endif

class gemhead;
#define GEMCACHE_MAGIC 0x1234567
struct GemCache {
    	GemCache(gemhead *parent);
        ~GemCache();
	void reset(gemhead *parent);
    	int dirty, resendImage, vertexDirty;
    	gemhead *m_parent;
	int m_magic;
};
#ifdef __WIN32__
    GemCache :: GemCache(gemhead *parent)
        : dirty(1), resendImage(0), vertexDirty(0),
        m_parent(parent), m_magic(GEMCACHE_MAGIC)
       {}
#endif

//  in 0: gem
//  in 1: grid
// out 0: gem
\class GridToPix {
	pixBlock *pb;
	\attr bool yflip;
	void render (GemCache *cache, GemState *state) {
		state->image = pb;
		pb->newimage = 1;
		//if (cache) post("gemcache: magic=%d, has to be %d",cache->m_magic,GEMCACHE_MAGIC);
		if (cache && cache->m_magic==GEMCACHE_MAGIC) {
			//imageStruct &im = pb->image;
			//post("im.ysize=%d im.xsize=%d im.format=%d im.type=%d",im.ysize,im.xsize,im.format,im.type);
			//post("cache->resendImage=%d, setting to 1", cache->resendImage);
			//post("cache->dirty=%d, setting to 1", cache->dirty);
			//cache->resendImage = 1;
			//cache->dirty = 1;
		}
	}
	void startRendering () {pb->newimage = 1;}
	\constructor () {
		yflip = false;
		pb = new pixBlock();
		imageStruct &im = pb->image;
		im.ysize = 1; im.xsize = 1; im.csize = 4; im.format = GEM_RGBA; im.type = GL_UNSIGNED_BYTE;
		im.allocate(); *(int*)im.data = 0x000000ff;
	}
	~GridToPix () {}
	\grin 1 int
	\decl 0 gem_state (...) {
		if (argc==2) {
			//post("#to_pix gem_state %p %p",(void *)argv[0],(void *)argv[1]);
			render((GemCache *)(void *)argv[0],
			       (GemState *)(void *)argv[1]);
		} else {
			if (argc==1) post("#to_pix gem_state %f",(float)argv[0]); else post("#to_pix gem_state ???");
			startRendering();
		}
		out[0](gensym("gem_state"),argc,argv);
	}
};

template <class T, class S>
static void convert_number_type(int n, T *out, S *in) {for (int i=0; i<n; i++) out[i]=(T)in[i];}
//#define NTIMES(FOO) for (; N>=4; N-=4) {FOO FOO FOO FOO} for (; N; N--) {FOO}

GRID_INLET(1) {
	if (in.dim.n != 3) RAISE("expecting 3 dimensions: rows,columns,channels, got %s",in.dim.to_s());
	int c = in.dim[2];
	if (c!=3 && c!=4)  RAISE("expecting 3 or 4 channels (got %d)",in.dim[2]);
	in.set_chunk(1);
	imageStruct &im = pb->image;
	im.clear(); im.ysize = in.dim[0]; im.xsize = in.dim[1]; im.type = GL_UNSIGNED_BYTE;
	switch (in.dim[2]) {
	case 1: im.csize = 1; im.format = GL_LUMINANCE; break;
	case 3: im.csize = 4; im.format = GEM_RGBA;     break; /* GEM_RGB hardly exists anymore */
	case 4: im.csize = 4; im.format = GEM_RGBA;     break;
	default: RAISE("you shouldn't see this error message.");
	}
	im.allocate();
} GRID_FLOW {
	uint8 *buf = (uint8 *)pb->image.data;
	int csize  = pb->image.csize;
	pb->image.upsidedown = !yflip;
	long sxc = in.dim.prod(1);
	long sx = in.dim[1];
	long chans = in.dim[2];
	for (long y=in.dex/sxc; n; data+=sxc, n-=sxc, y++) {
		if (chans==3) {
			uint8 *buf2 = buf+y*sx*csize;
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
			uint8 *buf2 = buf+y*sx*csize;
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
} GRID_END
\end class {install("#to_pix",2,1); add_creator("#export_pix");}

//------------------------------------------------------------------------

typedef t_symbol *Colorspace;

// LinuxIntel y:6409 yuv:34233
//   OsxIntel y:6409 yuv:34233
//   OsxPPC   y:6409 yuv:

\class GridFromPix {
	P<BitPacking> bp_rgba;
	P<BitPacking> bp_bgra;
	\attr bool yflip;
	\attr NumberTypeE cast;
	int channels;
	\attr Colorspace colorspace();
	t_symbol *m_colorspace;
	\constructor () {
		yflip = false;
		cast = int32_e;
		_0_colorspace(gensym("rgba"));
	}
	virtual ~GridFromPix () {}
	\decl 0 gem_state (...);
	void render_really(imageStruct &im) {
		BitPacking *bp;
		switch (im.format) {
		  case GL_RGBA: bp = bp_rgba; break;
		  #ifdef GL_VERSION_1_2
		  case GL_BGRA: bp = bp_bgra; break;
		  #endif
		  case GL_LUMINANCE: break;
		//  case 0x85b9 /*GL_YCBCR_422_APPLE*/: break;
		  default: ::post("can't produce grid from pix format %d (0x%x)",im.format,im.format); return;}
		switch (im.type) {
		  case GL_UNSIGNED_BYTE: break; /*ok*/
		  #ifdef GL_VERSION_1_2
		  case GL_UNSIGNED_INT_8_8_8_8: break; /*ok*/
		  case GL_UNSIGNED_INT_8_8_8_8_REV: break; /*ouate de phoque*/
		  #endif
		  default: ::post("can't produce grid from pix type %d (0x%x)",  im.type, im.type  ); return;}
		// on OSX, one was GL_UNSIGNED_INT_8_8_8_8 and the other was...?
		int32 v[] = {im.ysize, im.xsize, channels};
		GridOut out(this,0,Dim(3,v),cast);
		long sxc = im.xsize*channels;
		long sy = v[0];
		bool f = yflip^im.upsidedown;
		if (channels==4 && im.format==GL_RGBA) {
			for (int y=0; y<sy; y++) out.send(sxc,(uint8 *)im.data+sxc*(f?y:sy-1-y));
		} else if (channels==1 && im.format==GL_LUMINANCE) {
			for (int y=0; y<sy; y++) out.send(sxc,(uint8 *)im.data+sxc*(f?y:sy-1-y));
		} else if (im.format==GL_LUMINANCE) {
			#define FOO(T) {T buf[sxc]; \
			    for (int y=0; y<sy; y++) { \
				uint8 *data = (uint8 *)im.data+im.xsize*im.csize*(f?y:sy-1-y); \
				if      (m_colorspace==gensym("rgb"))  for (int i=v[1]-1,j=sxc-3; i>=0; i--, j-=3) {buf[j]=buf[j+1]=buf[j+2]=data[i];} \
				else if (m_colorspace==gensym("rgba")) for (int i=v[1]-1,j=sxc-4; i>=0; i--, j-=4) {buf[j]=buf[j+1]=buf[j+2]=data[i]; buf[3]=255;} \
				else if (m_colorspace==gensym("yuv"))  for (int i=v[1]-1,j=sxc-3; i>=0; i--, j-=3) {buf[j]=data[i]; buf[j+1]=buf[j+2]=128;} \
				else RAISE("euh"); \
				out.send(sxc,buf);}}
			TYPESWITCH(cast,FOO,)
			#undef FOO
		} else if (channels==1) {
			#define FOO(T) {uint8 buf[sxc*4]; \
			    for (int y=0; y<sy; y++) { \
				uint8 *data = (uint8 *)im.data+im.xsize*im.csize*(f?y:sy-1-y); \
				bp->unpack(im.xsize,data,buf); \
				for (int i=0,j=0; i<sxc; i++,j+=3) buf[i]=RGB2Y(buf[j],buf[j+1],buf[j+2]); \
				out.send(sxc,buf);}}
			TYPESWITCH(cast,FOO,)
			#undef FOO
		} else {
			imageStruct mi;
			uint8 *dada = (uint8 *)im.data;
			if (im.format==0x85b9) {im.convertTo(&mi,GLenum(GL_RGBA)); dada=(uint8 *)mi.data;}
			#define FOO(T) {T buf[sxc]; \
			    for (int y=0; y<sy; y++) { \
				uint8 *data = dada+im.xsize*im.csize*(f?y:sy-1-y); \
				bp->unpack(im.xsize,data,buf); out.send(sxc,buf);}}
			TYPESWITCH(cast,FOO,)
			#undef FOO
		}
	}
	void render (void *state) {
		pixBlock *pb = ((GemState *)state)->image;
		if (!pb) {::post("gemstate has no pix"); return;}
		render_really(pb->image);
	}
};
\def void colorspace () {return m_colorspace;}
\def 0 colorspace (t_symbol *s) {// 3 2 1 0 (numéro de byte)
	uint32 rgba[4] = {0x000000ff,
			  0x0000ff00,
			  0x00ff0000,
			  0xff000000};
	uint32 bgra[4] = {0x0000ff00,
			  0x00ff0000,
			  0xff000000,
			  0x000000ff}; // really argb 
	if (!is_le()) {swap32(4,rgba); swap32(4,bgra);}
	if (s==gensym("rgb")) {
		channels=3;
		bp_rgba = new BitPacking(is_le(),4,3,rgba);
		bp_bgra = new BitPacking(is_le(),4,3,bgra);
	} else
	if (s==gensym("rgba")) {
		channels=4;
		bp_rgba = new BitPacking(is_le(),4,4,rgba);
		bp_bgra = new BitPacking(is_le(),4,4,bgra);
	} else
	if (s==gensym("y")) {
		channels=1; // will reduce from 3 to 1 channels after bitpacking
		bp_rgba = new BitPacking(is_le(),4,3,rgba);
		bp_bgra = new BitPacking(is_le(),4,3,bgra);
	} else RAISE("unknown colorspace '%s'",s->s_name);
	m_colorspace = s;
}
\def 0 gem_state (...) {if (argc==2) render((GemState *)(void *)argv[1]);}
\end class {install("#from_pix",1,1); add_creator("#import_pix");}

//------------------------------------------------------------------------
// [gemdead]

#ifdef __WIN32__
GemState::GemState() {}
#endif

\class GemDead {
	GemState *state;
	GemCache *cache;
	\constructor () {
		//cache = new GemCache(this); ///???
		cache = new GemCache(0);
		state = new GemState();
	}
	~GemDead () {/*delete cache; delete state;*/}
	\decl 0 bang () {
		t_atom2 ap[2] = {(t_gpointer *)cache,(t_gpointer *)state};
		out[0](gensym("gem_state"),2,ap);
	}
	\decl 0 float (float state) {
		t_atom2 ap[1] = {!!state};
		out[0](gensym("gem_state"),1,ap);
	}
};
\end class {install("gemdead",1,1);}

//------------------------------------------------------------------------

void startup_gem () {
	\startall
}

extern "C" void gridflow_gem9292_setup () {post("GridFlow Gem 9292 module loaded"); startup_gem();}
extern "C" void gridflow_gem9293_setup () {post("GridFlow Gem 9293 module loaded"); startup_gem();}
extern "C" void gridflow_gem9393_setup () {post("GridFlow Gem 9393 module loaded"); startup_gem();}

/*
virtual void processRGBAImage(imageStruct &image) {}
virtual void processRGBImage (imageStruct &image) {}
virtual void processGrayImage(imageStruct &image) {}
virtual void processYUVImage (imageStruct &image) {}
*/

