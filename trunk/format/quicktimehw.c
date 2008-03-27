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

#define QUICKTIMEHW_INCLUDE_HERE
#include "../base/grid.h.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

\class FormatQuickTimeHW : Format {
	quicktime_t *anim;
	int track;
	P<Dim> dim;
	char *codec;
	int colorspace;
	int channels;
	bool started;
	P<Dim> force;
	int length; // in frames
	float64 framerate;
	P<BitPacking> bit_packing;
	int jpeg_quality; // in theory we shouldn't need this, but...
	FormatQuickTimeHW() : track(0), dim(0), codec(QUICKTIME_RAW), 
		started(false), force(0), framerate(29.97), bit_packing(0), jpeg_quality(75) {}
	\decl void initialize (Symbol mode, String filename);
	\decl 0 close ();
	\decl 0 bang ();
	\decl 0 seek (int frame);
	\decl 0 force_size (int32 height, int32 width);
	\decl 0 codec (String c);
	\decl 0 colorspace (Symbol c);
	\decl 0 parameter (Symbol name, int32 value);
	\decl 0 framerate (float64 f);
	\decl 0 size (int32 height, int32 width);
	\decl 0 get ();
	\grin 0 int
};

\def 0 force_size (int32 height, int32 width) { force = new Dim(height, width); }
\def 0 seek (int frame) {quicktime_set_video_position(anim,frame,track);}

\def 0 bang () {
	int nframe = quicktime_video_position(anim,track);
	if (nframe >= length) {outlet_bang(bself->te_outlet); return;}
	/* if it works, only do it once, to avoid silly stderr messages forgotten in LQT */
	if (!quicktime_reads_cmodel(anim,colorspace,0) && !started) {
		RAISE("LQT says this video cannot be decoded into the chosen colorspace");
	}
	int sx = quicktime_video_width(anim,track);
	int sy = quicktime_video_height(anim,track);
	int sz = quicktime_video_depth(anim,track);
	channels = sz/8; // hack. how do i get the video's native colormodel ?
	switch (sz) {
	case 24: colorspace=BC_RGB888; break;
	case 32: colorspace=BC_RGBA8888; break;
	default: post("strange quicktime. ask matju."); break;
	}
	if (force) {
		sy = force->get(0);
		sx = force->get(1);
	}
	uint8 *buf = new uint8[sy*sx*channels];
	uint8 *rows[sy]; for (int i=0; i<sy; i++) rows[i]=buf+i*sx*channels;
	quicktime_decode_scaled(anim,0,0,sx,sy,sx,sy,colorspace,rows,track);
	GridOutlet out(this,0,new Dim(sy,sx,channels),cast);
	out.give(sy*sx*channels,buf);
	started=true;
//	return INT2NUM(nframe);
}

//!@#$ should also support symbol values (how?)
\def 0 parameter (Symbol name, int32 value) {
	int val = value;
	//post("quicktime_set_parameter %s %d",(char*)rb_sym_name(name), val);
	quicktime_set_parameter(anim, (char*)rb_sym_name(name), &val);
	if (name==SYM(jpeg_quality)) jpeg_quality=value;
}

\def 0 framerate (float64 f) {
	framerate=f;
	quicktime_set_framerate(anim, f);
}

\def 0 size (int32 height, int32 width) {
	if (dim) RAISE("video size already set!");
	// first frame: have to do setup
	dim = new Dim(height, width, 3);
	quicktime_set_video(anim,1,dim->get(1),dim->get(0),framerate,codec);
	quicktime_set_cmodel(anim,colorspace);
}

GRID_INLET(FormatQuickTimeHW,0) {
	if (in->dim->n != 3)           RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2)!=channels) RAISE("expecting %d channels (got %d)",channels,in->dim->get(2));
	in->set_chunk(0);
	if (dim) {
		if (!dim->equal(in->dim)) RAISE("all frames should be same size");
	} else {
		// first frame: have to do setup
		dim = in->dim;
		quicktime_set_video(anim,1,dim->get(1),dim->get(0),framerate,codec);
		quicktime_set_cmodel(anim,colorspace);
		quicktime_set_depth(anim,8*channels,track);
	}
	//post("quicktime jpeg_quality %d", jpeg_quality);
	quicktime_set_parameter(anim, (char*)"jpeg_quality", &jpeg_quality);
} GRID_FLOW {
	int sx = quicktime_video_width(anim,track);
	int sy = quicktime_video_height(anim,track);
	uint8 *rows[sy];
	if (sizeof(T)>1) {
		uint8 data2[n];
		bit_packing->pack(sx*sy,data,(uint8 *)data2);
		for (int i=0; i<sy; i++) rows[i]=data2+i*sx*channels;
		quicktime_encode_video(anim,rows,track);
	} else {
		for (int i=0; i<sy; i++) rows[i]=(uint8 *)data+i*sx*channels;
		quicktime_encode_video(anim,rows,track);
	}
} GRID_FINISH {
} GRID_END

\def 0 codec (String c) {
	//fprintf(stderr,"codec = %s\n",rb_str_ptr(rb_inspect(c)));
#ifdef LQT_VERSION
	char buf[5];
	strncpy(buf,rb_str_ptr(c),4);
	for (int i=rb_str_len(c); i<4; i++) buf[i]=' ';
	buf[4]=0;
	Ruby fourccs = rb_ivar_get(rb_obj_class(rself),SI(@fourccs));
	if (Qnil==rb_hash_aref(fourccs,rb_str_new2(buf)))
		RAISE("warning: unknown fourcc '%s' (%s)",
			buf, rb_str_ptr(rb_inspect(rb_funcall(fourccs,SI(keys),0))));
#endif	
	codec = strdup(buf);
}

\def 0 colorspace (Symbol c) {
	if (0) {
	} else if (c==SYM(rgb))     { channels=3; colorspace=BC_RGB888; 
	} else if (c==SYM(rgba))    { channels=4; colorspace=BC_RGBA8888;
	} else if (c==SYM(bgr))     { channels=3; colorspace=BC_BGR888;
	} else if (c==SYM(bgrn))    { channels=4; colorspace=BC_BGR8888;
//	} else if (c==SYM(yuv))     { channels=3; colorspace=BC_YUV888;
	} else if (c==SYM(yuva))    { channels=4; colorspace=BC_YUVA8888;
	} else if (c==SYM(YUV420P)) { channels=3; colorspace=BC_YUV420P;
	} else RAISE("unknown colorspace '%s' (supported: rgb, rgba, bgr, bgrn, yuv, yuva)",rb_sym_name(c));
}

\def 0 close () {
	if (anim) { quicktime_close(anim); anim=0; }
	SUPER;
}

\def 0 get () {
/*	t_atom a[1];
	SETFLOAT(a,(float)length);
	outlet_anything(bself->te_outlet,gensym("frames"),1,a);
*/
	Ruby a[] = {INT2NUM(1),SYM(frames),INT2NUM(length)};
	send_out(3,a);
	//SUPER;
}

// libquicktime may be nice, but it won't take a filehandle, only filename
\def void initialize (Symbol mode, String filename) {
	SUPER;
	filename = rb_funcall(mGridFlow,SI(find_file),1,filename);
	anim = quicktime_open(rb_str_ptr(filename),mode==SYM(in),mode==SYM(out));
	if (!anim) RAISE("can't open file `%s': %s (or some other reason that libquicktime won't tell us)",
		rb_str_ptr(filename), strerror(errno));
	if (mode==SYM(in)) {
		length = quicktime_video_length(anim,track);
		post("quicktime: codec=%s height=%d width=%d depth=%d framerate=%f",
			quicktime_video_compressor(anim,track),
			quicktime_video_height(anim,track),
			quicktime_video_width(anim,track),
			quicktime_video_depth(anim,track),
			quicktime_frame_rate(anim,track));
/* This doesn't really work: (is it just for encoding?)
		if (!quicktime_supported_video(anim,track))
			RAISE("quicktime: unsupported codec: %s",
			      quicktime_video_compressor(anim,track));
*/
	}
	_0_colorspace(0,0,SYM(rgb));
	quicktime_set_cpus(anim,1);
	uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
	bit_packing = new BitPacking(is_le(),3,3,mask);
}

\classinfo {install_format("#io.quicktime",6,"mov");
//  def self.info; %[codecs: #{@codecs.keys.join' '}] end
//#define L fprintf(stderr,"%s:%d in %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#ifdef LQT_VERSION
	lqt_registry_init();
	int n = lqt_get_num_video_codecs();
	Ruby codecs = rb_hash_new();
	Ruby fourccs = rb_hash_new();
	for (int i=0; i<n; i++) {
		const lqt_codec_info_t *s = lqt_get_video_codec_info(i);
		if (!s->name) {
			fprintf(stderr,"[#in quicktime]: skipping codec with null name!\n");
			continue;
		}
		Ruby name = rb_str_new2(s->name);
		Ruby f = rb_ary_new2(s->num_fourccs);
		if (!s->fourccs) {
			post("WARNING: no fourccs (quicktime library is broken?)");
			goto hell;
		}
		//fprintf(stderr,"num_fourccs=%d fourccs=%p\n",s->num_fourccs,s->fourccs);
		for (int j=0; j<s->num_fourccs; j++) {
			Ruby fn = rb_str_new2(s->fourccs[j]);
			rb_ary_push(f,fn);
			rb_hash_aset(fourccs,fn,name);
		}
		rb_hash_aset(codecs,name,f);
		hell:;
	}
	rb_ivar_set(rself,SI(@codecs),codecs);
	rb_ivar_set(rself,SI(@fourccs),fourccs);
#endif
}
\end class FormatQuickTimeHW
void startup_quicktimehw () {
	\startall
}
