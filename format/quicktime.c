/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <quicktime/quicktime.h>
#include <quicktime/colormodels.h>

#include <quicktime/lqt_version.h>
#ifdef LQT_VERSION
#include <quicktime/lqt.h>
#include <quicktime/lqt_codecinfo.h>
#endif

\class FormatQuickTime < Format
struct FormatQuickTime : Format {
	quicktime_t *anim;
	int track;
	BitPacking *bit_packing;
	Dim *dim;
	char *codec;

	FormatQuickTime() : track(0), dim(0), codec(QUICKTIME_RAW) {}
	\decl void initialize (Symbol mode, Symbol source, String filename);
	\decl void close ();
	\decl void codec_m (String c);
	\decl Ruby frame ();
	\decl void seek (int frame);
	GRINLET3(0);
};

\def void seek (int frame) {
	int result = quicktime_set_video_position(anim,frame,track);
}

\def Ruby frame () {
	int nframe = quicktime_video_position(anim,track);
	if (nframe >= quicktime_video_length(anim,track)) return Qfalse;

	GridOutlet *o = out[0];
	int channels = bit_packing->size;
	int bytes = bit_packing->bytes;
	int sx = quicktime_video_width(anim,track);
	int sy = quicktime_video_height(anim,track);
	int npixels = sx*sy;
	Pt<uint8> buf = ARRAY_NEW(uint8,sy*sx*bytes+16);
	//if (!quicktime_reads_cmodel(anim,BC_RGB888,0)) RAISE("shoo");
	uint8 *rows[sy];
	for (int i=0; i<sy; i++) rows[i]=buf+i*sx*bytes;
	int result;
	result = quicktime_decode_scaled(anim,0,0,sx,sy,sx,sy,BC_RGB888,rows,track);
	int32 v[] = { sy, sx, channels };
	o->begin(new Dim(3,v),
		NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));

	int bs = o->dim->prod(1);
	if (1 /* && BG_RGB888 color model */) {
		//o->send(bytes*sx*sy,buf);
		o->give(bytes*sx*sy,buf);
	} else {
		STACK_ARRAY(uint8,b2,bs);
		for(int y=0; y<sy; y++) {
			bit_packing->unpack(sx,buf+bytes*sx*y,b2);
			o->send(bs,b2);
		}
		delete[] (uint8 *)buf;
	}
	return INT2NUM(nframe);
}

GRID_INLET(FormatQuickTime,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels (got %d)",in->dim->get(2));
	in->set_factor(in->dim->prod());
} GRID_FLOW {
	if (dim) {
		if (!dim->equal(in->dim)) RAISE("all frames should be same size");
	} else {
		/* first frame: have to do setup */
		dim = in->dim->dup();
		quicktime_set_video(anim,1,dim->get(1),dim->get(0),
			15,codec);
	}
	int sx = quicktime_video_width(anim,track);
	int sy = quicktime_video_height(anim,track);
	int npixels = in->factor/in->dim->get(2);
	Pt<uint8> buf = ARRAY_NEW(uint8,npixels*bit_packing->bytes);
	bit_packing->pack(npixels,data,buf);

	uint8 *rows[sy];
	for (int i=0; i<sy; i++) rows[i]=buf+i*sx*bit_packing->bytes;

	quicktime_encode_video(anim,rows,track);
	delete[] (uint8 *)buf;
} GRID_FINISH {
} GRID_END

\def void codec_m (String c) {
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

\def void close () {
	if (bit_packing) delete bit_packing;
	if (anim) quicktime_close(anim);
	rb_call_super(argc,argv);
}

/* note: will not go through jMax data paths */
/* libquicktime may be nice, but it won't take a filehandle, only filename */
\def void initialize (Symbol mode, Symbol source, String filename) {
	rb_call_super(argc,argv);
	if (source!=SYM(file)) RAISE("usage: quicktime file <filename>");
	filename = rb_funcall(mGridFlow,SI(find_file),1,filename);
	anim = quicktime_open(rb_str_ptr(filename),mode==SYM(in),mode==SYM(out));
	if (!anim) RAISE("can't open file `%s': %s", filename, strerror(errno));
	if (mode==SYM(in)) {
		gfpost("quicktime: codec=%s height=%d width=%d depth=%d framerate=%f",
			quicktime_video_compressor(anim,track),
			quicktime_video_height(anim,track),
			quicktime_video_width(anim,track),
			quicktime_video_depth(anim,track),
			quicktime_frame_rate(anim,track));
		if (!quicktime_supported_video(anim,track))
			RAISE("quicktime: unsupported codec: %s",
			      quicktime_video_compressor(anim,track));
	}
	uint32 masks[] = { 0x0000ff,0x00ff00,0xff0000 };
	bit_packing = new BitPacking(is_le(),3,3,masks);
}

GRCLASS(FormatQuickTime,LIST(GRINLET2(FormatQuickTime,0,4)),
\grdecl
){
	IEVAL(rself,"install 'FormatQuickTime',1,1;"
	"conf_format 6,'quicktime','Apple Quicktime (using "
	"HeroineWarrior\\'s)'");

#ifdef LQT_VERSION
	lqt_registry_init();
	int n = lqt_get_num_video_codecs();
	Ruby codecs = rb_hash_new();
	Ruby fourccs = rb_hash_new();
	for (int i=0; i<n; i++) {
		const lqt_codec_info_t *s = lqt_get_video_codec_info(i);
		Ruby name = rb_str_new2(s->name);
		Ruby f = rb_ary_new2(s->num_fourccs);
		for (int j=0; j<s->num_fourccs; j++) {
			Ruby fn = rb_str_new2(s->fourccs[j]);
			rb_ary_push(f,fn);
			rb_hash_aset(fourccs,fn,name);
		}
		rb_hash_aset(codecs,name,f);
	}
	rb_ivar_set(rself,SI(@codecs),codecs);
	rb_ivar_set(rself,SI(@fourccs),fourccs);
#endif
}

\end class FormatQuickTime
