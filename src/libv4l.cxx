/*
	$Id: videodev.c 4620 2009-11-01 21:16:58Z matju $

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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sstream>

//#define error post
static bool debug=1;

/* **************************************************************** */

#define FLAG(_num_,_name_,_desc_) #_name_,
#define  OPT(_num_,_name_,_desc_) #_name_,
#define WH(_field_,_spec_)             oprintf(buf, "%s=" _spec_ " ", #_field_, self->_field_);
#define WHYX(_name_,_fieldy_,_fieldx_) oprintf(buf, "%s=(%d %d) ", #_name_, self->_fieldy_, self->_fieldx_);
#define WHFLAGS(_field_,_table_)       oprintf(buf, "%s:%s " ,#_field_,flags_to_s( self->_field_,COUNT(_table_),_table_).data());
#define WHCHOICE(_field_,_table_)      oprintf(buf, "%s=%s; ",#_field_,choice_to_s(self->_field_,COUNT(_table_),_table_).data());
/*
static const char *video_mode_choice[] = {
	OPT( 0,PAL,  "pal")
	OPT( 1,NTSC, "ntsc")
	OPT( 2,SECAM,"secam")
	OPT( 3,AUTO, "auto")
};*/
static string flags_to_s(int value, int n, const char **table) {
	char foo[256];
	*foo = 0;
	for(int i=0; i<n; i++) {
		if ((value & (1<<i)) == 0) continue;
		if (*foo) strcat(foo," | ");
		strcat(foo,table[i]);
	}
	if (!*foo) strcat(foo,"0");
	return string(foo);
}
static string choice_to_s(int value, int n, const char **table) {
	if (value < 0 || value >= n) {
		char foo[64];
		sprintf(foo,"(Unknown #%d)",value);
		return string(foo);
	} else {
		return string(table[value]);
	}
}
static void gfpost(v4l2_input *self) {std::ostringstream buf; buf << "[v4l2_input] ";
	WH(index,"%u");
	WH(name,"\"%.32s\"");
	WH(type,"0x%08x");
	WH(audioset,"0x%08x");
	WH(tuner,"%u");
	WH(std,"0x%08x");
	WH(status,"0x%08x");
	post("%s",buf.str().data());}
static void gfpost(v4l2_tuner *self) {std::ostringstream buf; buf << "[v4l2_tuner] ";
	WH(index,"%u");
	WH(name,"\"%.32s\"");
	WH(type,"0x%08x");
	WH(capability,"0x%08x");
	WH(rangelow,"%lu");
	WH(rangehigh,"%lu");
	WH(rxsubchans,"0x%08x");
	WH(audmode,"0x%08x");
	WH(signal,"%d");
	WH(afc,"%d");}
static void gfpost(v4l2_window *self) {std::ostringstream buf; buf << "[v4l2_window] ";
	WHYX(pos,w.top,w.left);
	WHYX(size,w.height,w.width);
//	enum v4l2_field  	field;
	WH(chromakey,"0x%08x");
	WH(clipcount,"%u");
	//WH(global_alpha,"%d"); but is a uint8.
	post("%s",buf.str().data());}
static void gfpost(v4l2_format *self) {std::ostringstream buf; buf << "[v4l2_format]";
	WH(type,"%u");
	if (self->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE) return;
	WHYX(size,fmt.pix.height,fmt.pix.width);
	WH(fmt.pix.pixelformat,"%u");
	WH(fmt.pix.field,"0x%08x");
	WH(fmt.pix.bytesperline,"0x%08x");
	WH(fmt.pix.sizeimage,"0x%08x");
	WH(fmt.pix.colorspace,"0x%08x");
	WH(fmt.pix.priv,"0x%08x");
}

/* **************************************************************** */

\class FormatLibV4L : Format {
	uint8 *image;
	int queue[8], queuesize, queuemax, next_frame;
	int current_channel, current_tuner;
	P<BitPacking> bit_packing;
	P<Dim> dim;
	bool has_frequency, has_tuner, has_norm;
	int fd;
    v4l2_format              fmt;
    v4l2_buffer              buf;
    v4l2_requestbuffers      req;

	\constructor (string mode, string filename) {
		queuesize=0; queuemax=2; next_frame=0; bit_packing=0; dim=0;
		colorspace=gensym("none"); /* non-existent colorspace just to prevent crash in case of other problems */
		has_frequency=false;
		has_tuner=false;
		has_norm=false;
		image=0;
		fd = v4l2_open(filename.data(),0);
		if (fd<0) RAISE("can't open device '%s': %s",filename.data(),strerror(errno));
		f = fdopen(fd,"r+");
		initialize2();
	}
	void frame_finished (uint8 *buf);

	void alloc_image ();
	void dealloc_image ();
	void frame_ask ();
	void initialize2 ();
	~FormatLibV4L () {if (image) dealloc_image();}

	\decl 0 bang ();
	\grin 0 int

	\attr int channel();
	\attr int tuner();
	//\attr int norm();
	\decl 0 size (int sy, int sx);
	\decl 0 transfer (string sym, int queuemax=2);

	\attr t_symbol *colorspace;
	\attr int32  frequency();
	\attr uint16 brightness();
	\attr uint16 hue();
	\attr uint16 colour();
	\attr uint16 contrast();
	\attr uint16 whiteness();

	\attr uint16 framerate();
	\attr uint16 white_mode(); /* 0..1 */
	\attr uint16 white_red();
	\attr uint16 white_blue();
	\attr uint16 white_speed();
	\attr uint16 white_delay();
	\attr int    auto_gain();
	\attr int    noise_reduction(); /* 0..3 */
	\attr int    compression();     /* 0..3 */
	\attr t_symbol *name;

	\decl 0 get (t_symbol *s=0);
};

#define DEBUG(args...) 42
//#define DEBUG(args...) post(args)

#define  IOCTL( F,NAME,ARG) \
  (DEBUG("fd%d.ioctl(0x%08x,0x%08x)",F,NAME,ARG), v4l2_ioctl(F,NAME,ARG))
#define WIOCTL( F,NAME,ARG) \
  (IOCTL(F,NAME,ARG)<0 && (error("ioctl %s: %s",#NAME,strerror(errno)),1))
#define WIOCTL2(F,NAME,ARG) \
  (IOCTL(F,NAME,ARG)<0 && (error("ioctl %s: %s",#NAME,strerror(errno)), RAISE("ioctl error"), 0))

\def 0 get (t_symbol *s=0) {
	// this is abnormal for a get-function
	if (s==gensym("frequency") && !has_frequency  ) return;
	if (s==gensym("tuner")     && !has_tuner      ) return;
	if (s==gensym("norm")      && !has_norm       ) return;
	FObject::_0_get(argc,argv,s);
	// size are abnormal attributes (does not use nested list)
	if (!s) {
		t_atom a[2];
		//SETFLOAT(a+0,vcaps.minheight); SETFLOAT(a+1,vcaps.minwidth); outlet_anything(outlets[0],gensym("minsize"),2,a);
		//SETFLOAT(a+0,vcaps.maxheight); SETFLOAT(a+1,vcaps.maxwidth); outlet_anything(outlets[0],gensym("maxsize"),2,a);
		SETFLOAT(a+0,dim->v[0]); SETFLOAT(a+1,dim->v[1]); outlet_anything(outlets[0],gensym("size"),2,a);
		SETSYMBOL(a,gensym("mmap")); outlet_anything(outlets[0],gensym("transfer"),1,a);
	}
}

\def 0 size (int sy, int sx) {
	// !@#$ bug here: won't flush the frame queue
	dim = new Dim(sy,sx,3);
	WIOCTL(fd, VIDIOC_G_FMT, &fmt); if (debug) gfpost(&fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2_pix_format &f = fmt.fmt.pix;
	if (sy && sx) {f.height=sy; f.width=sx;}
	if (debug) gfpost(&fmt);
	WIOCTL(fd, VIDIOC_S_FMT, &fmt);
	WIOCTL(fd, VIDIOC_G_FMT, &fmt);
	if (debug) gfpost(&fmt);
}

void FormatLibV4L::dealloc_image () {
	if (!image) return;
	munmap(image, vmbuf.size);
	image=0;
}

void FormatLibV4L::alloc_image () {
	WIOCTL2(fd, VIDIOCGMBUF, &vmbuf);
	//gfpost(&vmbuf);
	//size_t size = vmbuf.frames > 4 ? vmbuf.offsets[4] : vmbuf.size;
	image = (uint8 *)mmap(0,vmbuf.size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if (((long)image)==-1) {image=0; RAISE("mmap: %s", strerror(errno));}
}

void FormatLibV4L::frame_ask () {
	if (queuesize>=queuemax) RAISE("queue is full (queuemax=%d)",queuemax);
	if (queuesize>=vmbuf.frames) RAISE("queue is full (vmbuf.frames=%d)",vmbuf.frames);
	vmmap.frame = queue[queuesize++] = next_frame;
	vmmap.format = vp.palette;
	vmmap.width  = dim->get(1);
	vmmap.height = dim->get(0);
	WIOCTL2(fd, VIDIOCMCAPTURE, &vmmap);
	//gfpost(&vmmap);
	next_frame = (next_frame+1) % vmbuf.frames;
}

static uint8 clip(int x) {return x<0?0 : x>255?255 : x;}

void FormatLibV4L::frame_finished (uint8 *buf) {
	string cs = colorspace->s_name;
	int downscale = cs=="magic";
	/* picture is converted here. */
	int sy = dim->get(0)>>downscale;
	int sx = dim->get(1)>>downscale;
	int bs = dim->prod(1)>>downscale;
	uint8 b2[bs];
	//post("sy=%d sx=%d bs=%d",sy,sx,bs);
	//post("frame_finished, vp.palette = %d; colorspace = %s",vp.palette,cs.data());
	if (vp.palette==VIDEO_PALETTE_YUV420P) {
		GridOutlet out(this,0,cs=="magic"?new Dim(sy,sx,3):(Dim *)dim,cast);
		if (cs=="y") {
			out.send(sy*sx,buf);
		} else if (cs=="rgb") {
			for(int y=0; y<sy; y++) {
				uint8 *bufy = buf+sx* y;
				uint8 *bufu = buf+sx*sy    +(sx/2)*(y/2);
				uint8 *bufv = buf+sx*sy*5/4+(sx/2)*(y/2);
				int Y1,Y2,U,V;
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					Y1=bufy[x]   - 16;
					Y2=bufy[x+1] - 16;
					U=bufu[x/2] - 128;
					V=bufv[x/2] - 128;
					b2[xx+0]=clip((298*Y1         + 409*V)>>8);
					b2[xx+1]=clip((298*Y1 - 100*U - 208*V)>>8);
					b2[xx+2]=clip((298*Y1 + 516*U        )>>8);
					b2[xx+3]=clip((298*Y2         + 409*V)>>8);
					b2[xx+4]=clip((298*Y2 - 100*U - 208*V)>>8);
					b2[xx+5]=clip((298*Y2 + 516*U        )>>8);
				}
				out.send(bs,b2);
			}
		} else if (cs=="yuv") {
			for(int y=0; y<sy; y++) {
				uint8 *bufy = buf+sx* y;
				uint8 *bufu = buf+sx*sy    +(sx/2)*(y/2);
				uint8 *bufv = buf+sx*sy*5/4+(sx/2)*(y/2);
				int U,V;
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					U=bufu[x/2];
					V=bufv[x/2];
					b2[xx+0]=clip(((bufy[x+0]-16)*298)>>8);
					b2[xx+1]=clip(128+(((U-128)*293)>>8));
					b2[xx+2]=clip(128+(((V-128)*293)>>8));
					b2[xx+3]=clip(((bufy[x+1]-16)*298)>>8);
					b2[xx+4]=clip(128+(((U-128)*293)>>8));
					b2[xx+5]=clip(128+(((V-128)*293)>>8));
				}
				out.send(bs,b2);
			}
		} else if (cs=="magic") {
			for(int y=0; y<sy; y++) {
				uint8 *bufy = buf        +4*sx*y;
				uint8 *bufu = buf+4*sx*sy+  sx*y;
				uint8 *bufv = buf+5*sx*sy+  sx*y;
				for (int x=0,xx=0; x<sx; x++,xx+=3) {
					b2[xx+0]=bufy[x+x];
					b2[xx+1]=bufu[x];
					b2[xx+2]=bufv[x];
				}
				out.send(bs,b2);
			}
		}
	} else if (vp.palette==V4L2_PIX_FMT_RGB24) {
		GridOutlet out(this,0,dim,cast);
		uint8 rgb[sx*3];
		uint8 b2[sx*3];
		if (cs=="y") {
			for(int y=0; y<sy; y++) {
			        bit_packing->unpack(sx,buf+y*sx*bit_packing->bytes,rgb);
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					b2[x+0] = (76*rgb[xx+0]+150*rgb[xx+1]+29*rgb[xx+2])>>8;
					b2[x+1] = (76*rgb[xx+3]+150*rgb[xx+4]+29*rgb[xx+5])>>8;
				}
				out.send(bs,b2);
			}
		} else if (cs=="rgb") {
			for(int y=0; y<sy; y++) {
			        bit_packing->unpack(sx,buf+y*sx*bit_packing->bytes,rgb);
				out.send(bs,rgb);
			}
		} else if (cs=="yuv") {
			for(int y=0; y<sy; y++) {
				bit_packing->unpack(sx,buf+y*sx*bit_packing->bytes,rgb);
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					b2[xx+0] = clip(    ((  76*rgb[xx+0] + 150*rgb[xx+1] +  29*rgb[xx+2])>>8));
					b2[xx+1] = clip(128+((- 44*rgb[xx+0] -  85*rgb[xx+1] + 108*rgb[xx+2])>>8));
					b2[xx+2] = clip(128+(( 128*rgb[xx+0] - 108*rgb[xx+1] -  21*rgb[xx+2])>>8));
					b2[xx+3] = clip(    ((  76*rgb[xx+3] + 150*rgb[xx+4] +  29*rgb[xx+5])>>8));
					b2[xx+4] = clip(128+((- 44*rgb[xx+3] -  85*rgb[xx+4] + 108*rgb[xx+5])>>8));
					b2[xx+5] = clip(128+(( 128*rgb[xx+3] - 108*rgb[xx+4] -  21*rgb[xx+5])>>8));
				}
				out.send(bs,b2);
			}
		} else if (cs=="magic") {
			RAISE("magic colorspace not supported with a RGB palette");
		}
	} else {
		RAISE("unsupported palette %d",vp.palette);
	}
}

/* these are factors for RGB to analog YUV */
// Y =   66*R + 129*G +  25*B
// U = - 38*R -  74*G + 112*B
// V =  112*R -  94*G -  18*B

// strange that read2 is not used and read3 is used instead
static int read2(int fd, uint8 *image, int n) {
	int r=0;
	while (n>0) {
		int rr=read(fd,image,n);
		if (rr<0) return rr; else {r+=rr; image+=rr; n-=rr;}
	}
	return r;
}

static int read3(int fd, uint8 *image, int n) {
	int r=read(fd,image,n);
	if (r<0) return r;
	return n;
}

\def 0 bang () {
	if (!image) alloc_image();
	while(queuesize<queuemax) frame_ask();
	vmmap.frame = queue[0];
	//uint64 t0 = gf_timeofday();
	WIOCTL2(fd, VIDIOCSYNC, &vmmap);
	//uint64 t1 = gf_timeofday();
	//if (t1-t0 > 100) gfpost("VIDIOCSYNC delay: %d us",t1-t0);
	frame_finished(image+vmbuf.offsets[queue[0]]);
	queuesize--;
	for (int i=0; i<queuesize; i++) queue[i]=queue[i+1];
	frame_ask();
}

GRID_INLET(0) {
	RAISE("can't write.");
} GRID_FLOW {
} GRID_FINISH {
} GRID_END

/*\def 0 norm (int value) {
	v4l2_tuner vtuner;
	vtuner.index = current_tuner;
	if (value<0 || value>3) RAISE("norm must be in range 0..3");
	if (0> IOCTL(fd, VIDIOC_G_TUNER, &vtuner)) {post("no tuner #%d", value); return;}
	vtuner.mode = value; gfpost(&vtuner); WIOCTL(fd, VIDIOC_S_TUNER, &vtuner);
}*/

/*\def int norm () {
	v4l2_tuner vtuner;
	vtuner.index = current_tuner;
	if (0> IOCTL(fd, VIDIOC_G_TUNER, &vtuner)) {post("no tuner #%d", current_tuner); return -1;}
	return vtuner.mode;
}*/

\def 0 tuner (int value) {
	v4l2_tuner vtuner;
	vtuner.index = current_tuner = value;
	if (0> IOCTL(fd, VIDIOC_G_TUNER, &vtuner)) RAISE("no tuner #%d", value);
	vtuner.mode = VIDEO_MODE_NTSC; //???
	gfpost(&vtuner);
	WIOCTL(fd, VIDIOC_S_TUNER, &vtuner);
	has_norm = (vtuner.mode<=3);
	int meuh;
	has_frequency = (v4l2_ioctl(fd, VIDIOC_G_FREQUENCY, &meuh)>=0);
}
\def int tuner () {return current_tuner;}

#define warn(fmt,stuff...) post("warning: " fmt,stuff)

\def 0 channel (int value) {
	v4l2_channel vchan;
	vchan.channel = value;
	current_channel = value;
	if (0> IOCTL(fd, VIDIOC_G_INPUT, &vchan)) warn("no channel #%d", value);
	//gfpost(&vchan);
	WIOCTL(fd, VIDIOC_S_INPUT, &vchan);
	if (vcaps.type & VID_TYPE_TUNER) _0_tuner(0,0,0);
	has_tuner = (vcaps.type & VID_TYPE_TUNER && vchan.tuners > 1);
}
\def int channel () {return current_channel;}

\def 0 transfer (string sym, int queuemax=2) {
	if (sym=="mmap") {
		dealloc_image();
		alloc_image();
		queuemax=min(8,min(queuemax,vmbuf.frames));
		post("transfer mmap with queuemax=%d (max max is vmbuf.frames=%d)", queuemax,vmbuf.frames);
		this->queuemax=queuemax;
	} else RAISE("don't know that transfer mode");
}

\def uint16 brightness ()                 {return v4l2_get_control(fd,V4L2_CID_BRIGHTNESS);}
\def 0      brightness (uint16 brightness){       v4l2_set_control(fd,V4L2_CID_BRIGHTNESS,brightness);}
\def uint16 hue        ()                 {return v4l2_get_control(fd,V4L2_CID_HUE);}
\def 0      hue        (uint16 hue)       {       v4l2_set_control(fd,V4L2_CID_HUE,hue);}
\def uint16 colour     ()                 {return v4l2_get_control(fd,V4L2_CID_SATURATION);}
\def 0      colour     (uint16 colour)    {       v4l2_set_control(fd,V4L2_CID_SATURATION,colour);}
\def uint16 contrast   ()                 {return v4l2_get_control(fd,V4L2_CID_CONTRAST);}
\def 0      contrast   (uint16 contrast)  {       v4l2_set_control(fd,V4L2_CID_CONTRAST,contrast);}
\def int32 frequency  () {
	int32 value;
	//if (ioctl(fd, VIDIOC_G_FREQUENCY, &value)<0) {has_frequency=false; return 0;}
	WIOCTL(fd, VIDIOC_G_FREQUENCY, &value);
	return value;
}
\def 0 frequency (int32 frequency) {
	long frequency_ = frequency;
	WIOCTL(fd, VIDIOC_S_FREQUENCY, &frequency_);
}

\def 0 colorspace (t_symbol *colorspace) { /* y yuv rgb magic */
	string c = colorspace->s_name;
	if (c=="y"    ) {} else
	if (c=="yuv"  ) {} else
	if (c=="rgb"  ) {} else
	if (c=="magic") {} else
	   RAISE("got '%s' but supported colorspaces are: y yuv rgb magic",c.data());
	WIOCTL(fd, VIDIOCGPICT, &vp);
	int palette = (palettes&(1<<V4L2_PIX_FMT_RGB24 )) ? V4L2_PIX_FMT_RGB24 : VIDEO_PALETTE_YUV420P;
	vp.palette = palette;
	WIOCTL(fd, VIDIOCSPICT, &vp);
	WIOCTL(fd, VIDIOCGPICT, &vp);
	if (vp.palette != palette) {
		post("this driver is unsupported: it wants palette %d instead of %d",vp.palette,palette);
		return;
	}
	if (palette==V4L2_PIX_FMT_RGB24) {uint32 masks[3]={0xff0000,0x00ff00,0x0000ff};
	    bit_packing = new BitPacking(is_le(),3,3,masks);} else this->colorspace=gensym(c.data());
	dim = new Dim(dim->v[0],dim->v[1],c=="y"?1:3);
}

\def uint16 framerate() {
	struct video_window vwin; WIOCTL(fd, VIDIOCGWIN, &vwin);
	return (vwin.flags & PWC_FPS_MASK) >> PWC_FPS_SHIFT;
}
\def 0 framerate(uint16 framerate) {
	struct video_window vwin; WIOCTL(fd, VIDIOCGWIN, &vwin);
	vwin.flags &= ~PWC_FPS_FRMASK;
	vwin.flags |= (framerate << PWC_FPS_SHIFT) & PWC_FPS_FRMASK;
	WIOCTL(fd, VIDIOCSWIN, &vwin);
}

\def int auto_gain() {int auto_gain=0; WIOCTL(fd, VIDIOCPWCGAGC, &auto_gain); return auto_gain;}
\def 0   auto_gain   (int auto_gain)  {WIOCTL(fd, VIDIOCPWCSAGC, &auto_gain);}

\def uint16 white_mode () {
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	if (pwcwb.mode==PWC_WB_AUTO)   return 0;
	if (pwcwb.mode==PWC_WB_MANUAL) return 1;
	return 2;
}
\def 0 white_mode (uint16 white_mode) {
	struct pwc_whitebalance pwcwb; WIOCTL(fd, VIDIOCPWCGAWB, &pwcwb);
	if      (white_mode==0) pwcwb.mode = PWC_WB_AUTO;
	else if (white_mode==1) pwcwb.mode = PWC_WB_MANUAL;
	else {error("unknown mode number %d", white_mode); return;}
	WIOCTL(fd, VIDIOCPWCSAWB, &pwcwb);}

\def uint16 white_red()        {return v4l2_get_control(fd,V4L2_CID_RED_BALANCE);}
\def 0 white_red(uint16 white_red)    {v4l2_get_control(fd,V4L2_CID_RED_BALANCE,white_red);}
\def uint16 white_blue()       {return v4l2_get_control(fd,V4L2_CID_RED_BALANCE);}
\def 0 white_blue(uint16 white_blue)  {v4l2_get_control(fd,V4L2_CID_RED_BALANCE,white_blue);}

\def uint16 white_speed() {struct pwc_wb_speed pwcwbs; WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs); return pwcwbs.control_speed;}
\def uint16 white_delay() {struct pwc_wb_speed pwcwbs; WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs); return pwcwbs.control_delay;}
\def 0 white_speed(uint16 white_speed) {struct pwc_wb_speed pwcwbs;         WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs);
					      pwcwbs.control_speed = white_speed; WIOCTL(fd, VIDIOCPWCSAWBSPEED, &pwcwbs);}
\def 0 white_delay(uint16 white_delay) {struct pwc_wb_speed pwcwbs;         WIOCTL(fd, VIDIOCPWCGAWBSPEED, &pwcwbs);
					      pwcwbs.control_delay = white_delay; WIOCTL(fd, VIDIOCPWCSAWBSPEED, &pwcwbs);}

\def int noise_reduction() {int noise_reduction; WIOCTL(fd, VIDIOCPWCGDYNNOISE, &noise_reduction); return noise_reduction;}
\def 0 noise_reduction(int noise_reduction) {    WIOCTL(fd, VIDIOCPWCSDYNNOISE, &noise_reduction);}
\def int compression()     {int compression;     WIOCTL(fd, VIDIOCPWCSCQUAL,    &compression    ); return compression;    }
\def 0 compression(int compression) {            WIOCTL(fd, VIDIOCPWCGCQUAL,    &compression    );}

void FormatLibV4L::initialize2 () {
	WIOCTL(fd, VIDIOCGCAP, &vcaps);
	_0_size(0,0,vcaps.maxheight,vcaps.maxwidth);
	char namebuf[33];
	memcpy(namebuf,vcaps.name,sizeof(vcaps.name));
	int i;
	for (i=32; i>=1; i--) if (!namebuf[i] || !isspace(namebuf[i])) break;
	namebuf[i]=0;
	while (--i>=0) if (isspace(namebuf[i])) namebuf[i]='_';
	name = gensym(namebuf);
	WIOCTL(fd, VIDIOCGPICT,&vp);
	palettes=0;
	int checklist[] = {V4L2_PIX_FMT_RGB24,VIDEO_PALETTE_YUV420P};
#if 1
	for (size_t i=0; i<sizeof(checklist)/sizeof(*checklist); i++) {
		int p = checklist[i];
#else
	for (size_t p=0; p<17; p++) {
#endif
		vp.palette = p;
		v4l2_ioctl(fd, VIDIOCSPICT,&vp);
		v4l2_ioctl(fd, VIDIOCGPICT,&vp);
		if (vp.palette == p) {
			palettes |= 1<<p;
			post("palette %d supported",p);
		}
	}
	_0_colorspace(0,0,gensym("rgb"));
	_0_channel(0,0,0);
}

\end class FormatLibV4L {install_format("#io.libv4l",4,"");}
void startup_videodev () {
	\startall
}
