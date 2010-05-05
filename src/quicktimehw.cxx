/*
	$Id: quicktimehw.c 4620 2009-11-01 21:16:58Z matju $

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

#define QUICKTIMEHW_INCLUDE_HERE
#include "gridflow.hxx.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <map>
#include <vector>

static std::map<string,std::vector<string> *> codecs;
static std::map<string,string> fourccs;

\class FormatQuickTimeHW : Format {
	quicktime_t *anim;
	int track;
	Dim dim;
	bool gotdim;
	char *m_codec;
	int colorspace;
	int channels;
	bool started;
	Dim force;
	bool doforce;
	float64 m_framerate;
	P<BitPacking> bit_packing;
	int jpeg_quality; // in theory we shouldn't need this, but...
	float advance;
	~FormatQuickTimeHW() {if (anim) quicktime_close(anim);}
	\constructor (t_symbol *mode, string filename) {
		track=0; gotdim=false; m_codec=const_cast<char *>(QUICKTIME_RAW);
		started=false; doforce=false; m_framerate=29.97; bit_packing=0; jpeg_quality=75;
		advance = 0;
// libquicktime may be nice, but it won't take a filehandle, only filename
		filename = gf_find_file(filename);
		anim = quicktime_open((char *)filename.data(),mode==gensym("in"),mode==gensym("out"));
		if (!anim) RAISE("can't open file `%s': see error message above",filename.data());
		if (mode==gensym("in")) {
	/* This doesn't really work: (is it just for encoding?)
			if (!quicktime_supported_video(anim,track))
				RAISE("quicktime: unsupported codec: %s", quicktime_video_compressor(anim,track));
	*/
		}
		_0_colorspace(0,0,string("rgb"));
		quicktime_set_cpus(anim,1);
		uint32 mask[3] = {0x0000ff,0x00ff00,0xff0000};
		bit_packing = new BitPacking(is_le(),3,3,mask);
		with_audio = false;
	}
	\decl 0 bang ();
	\decl 0 seek (int32 frame);
	\decl 0 rewind ();
	\decl 0 force_size (int32 height, int32 width);
	\decl 0 colorspace (string c);
	\decl 0 parameter (string name, int32 value);
	\decl 0 size (int32 height, int32 width);
	\grin 0 int

	\attr int32   frames();
	\attr float64 framerate();
	\attr int32   height();
	\attr int32   width();
	\attr int32   depth();
	\attr string  codec();
	\attr bool with_audio;
};

\def 0 force_size (int32 height, int32 width) {force = Dim(height, width); doforce=true;}
\def 0 seek (int32 frame) {
	quicktime_set_video_position(anim,clip(frame,int32(0),int32(quicktime_video_length(anim,track)-1)),track);
}
\def 0 rewind () {_0_seek(0,0,0);}

\def 0 bang () {
	if (with_audio) {
	    int track = 0;
	    float a_rate = quicktime_sample_rate(anim,track);
	    float v_rate = quicktime_frame_rate(anim,track);
	    advance -= a_rate/v_rate;
	    if (advance < 0) {
		int samples = 2*1024;
		//post("audio rate = %f, video rate = %f, ratio = %f",a_rate,v_rate,a_rate/v_rate);
		int achannels = quicktime_track_channels(anim,track);
		float sound[achannels*samples];
		float *output_f[achannels];
		for (int j=0; j<achannels; j++) output_f[j] = sound+samples*j;
		lqt_decode_audio_track(anim,0,output_f,samples,track);
		float sound2[samples*achannels];
		for (int i=0; i<samples; i++) for (int j=0; j<achannels; j++) sound2[i*achannels+j] = sound[j*samples+i];
		GridOutlet out(this,0,Dim(samples,achannels),float32_e);
		out.send(samples*achannels,sound2);
		advance += samples;
	    }
	}
	long length = quicktime_video_length(anim,track);
	long nframe = quicktime_video_position(anim,track);
	if (nframe >= length) {outlet_bang(outlets[0]); return;}
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
	if (doforce) {sy = force->get(0); sx = force->get(1);}
	uint8 buf[sy*sx*channels];
	uint8 *rows[sy]; for (int i=0; i<sy; i++) rows[i]=buf+i*sx*channels;
	quicktime_decode_scaled(anim,0,0,sx,sy,sx,sy,colorspace,rows,track);
	GridOutlet out(this,0,Dim(sy,sx,channels),cast);
	out.send(sy*sx*channels,buf);
	started=true;
//	return INT2NUM(nframe);
}

//!@#$ should also support symbol values (how?)
\def 0 parameter (string name, int32 value) {
	int val = value;
	//post("quicktime_set_parameter %s %d",name.data(), val);
	quicktime_set_parameter(anim, const_cast<char *>(name.data()), &val);
	if (name=="jpeg_quality") jpeg_quality=value;
}

\def 0 framerate (float64 f) {m_framerate=f; quicktime_set_framerate(anim, f);}

\def 0 size (int32 height, int32 width) {
	if (gotdim) RAISE("video size already set!");
	// first frame: have to do setup
	dim = Dim(height,width,3);
	gotdim = true;
	quicktime_set_video(anim,1,dim->get(1),dim->get(0),m_framerate,m_codec);
	quicktime_set_cmodel(anim,colorspace);
}

GRID_INLET(0) {
	if (in->dim->n != 3)           RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2)!=channels) RAISE("expecting %d channels (got %d)",channels,in->dim->get(2));
	in->set_chunk(0);
	if (gotdim) {
		if (!dim->equal(in->dim)) RAISE("all frames should be same size");
	} else {
		// first frame: have to do setup
		dim = in->dim;
		// this is a duplicate: see _0_size. what should I do with that?
		quicktime_set_video(anim,1,dim->get(1),dim->get(0),m_framerate,m_codec);
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

\def 0 codec (string c) {
#ifdef LQT_VERSION
	char buf[5];
	strncpy(buf,c.data(),4);
	for (int i=c.length(); i<4; i++) buf[i]=' ';
	buf[4]=0;
	if (fourccs.find(string(buf))==fourccs.end())
		RAISE("warning: unknown fourcc '%s'" /*" (%s)"*/, buf /*, rb_str_ptr(rb_inspect(rb_funcall(fourccs,SI(keys),0)))*/);
#endif	
	m_codec = strdup(buf);
}

\def 0 colorspace (string c) {
	if (0) {
	} else if (c=="rgb")     { channels=3; colorspace=BC_RGB888; 
	} else if (c=="rgba")    { channels=4; colorspace=BC_RGBA8888;
	} else if (c=="bgr")     { channels=3; colorspace=BC_BGR888;
	} else if (c=="bgrn")    { channels=4; colorspace=BC_BGR8888;
//	} else if (c=="yuv")     { channels=3; colorspace=BC_YUV888;
	} else if (c=="yuva")    { channels=4; colorspace=BC_YUVA8888;
	} else if (c=="YUV420P") { channels=3; colorspace=BC_YUV420P;
	} else RAISE("unknown colorspace '%s' (supported: rgb, rgba, bgr, bgrn, yuv, yuva)",c.data());
}
\def int32 depth  () {return quicktime_video_length(anim,track);}
\def int32 height () {return quicktime_video_height(anim,track);}
\def int32 width  () {return quicktime_video_width(anim,track);}
\def int32 frames () {return quicktime_video_depth(anim,track);}
\def float64 framerate () {return quicktime_frame_rate(anim,track);}
\def string codec () {return string(quicktime_video_compressor(anim,track));}
\def 0 depth  (int32 v) {RAISE("read-only");}
\def 0 height (int32 v) {RAISE("use size instead");}
\def 0 width  (int32 v) {RAISE("use size instead");}
\def 0 frames (int32 v) {RAISE("read-only");}

\classinfo {install_format("#io.quicktimehw",6,"mov avi");
//  def self.info; %[codecs: #{@codecs.keys.join' '}] end
//#define L fprintf(stderr,"%s:%d in %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#ifdef LQT_VERSION
	lqt_registry_init();
	int n = lqt_get_num_video_codecs();
	for (int i=0; i<n; i++) {
		const lqt_codec_info_t *s = lqt_get_video_codec_info(i);
		if (!s->name) {fprintf(stderr,"[#in quicktime]: skipping codec with null name!\n"); continue;}
		string name = string(s->name);
		std::vector<string> *f = new std::vector<string>(s->num_fourccs);
		if (!s->fourccs) {post("WARNING: no fourccs (quicktime library is broken?)"); goto hell;}
		//fprintf(stderr,"num_fourccs=%d fourccs=%p\n",s->num_fourccs,s->fourccs);
		for (int j=0; j<s->num_fourccs; j++) {
			string fn = string(s->fourccs[j]);
			f->push_back(fn);
			fourccs[fn]=name;
		}
		codecs[name]=f;
		hell:;
	}
#endif
}
\end class FormatQuickTimeHW

void gf_lqt_log_callback(lqt_log_level_t level, const char *domain, const char *message, void *data) {
	if (level&1) post("libquicktime: %s",message);
}

void startup_quicktimehw () {
	\startall
	lqt_set_log_callback(gf_lqt_log_callback,0);
}
