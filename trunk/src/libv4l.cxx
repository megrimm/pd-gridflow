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
#include "colorspace.hxx"
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
static string flags_to_s(int value, int n, const char **table) {
	std::ostringstream buf; buf << "("; bool empty=true;
	for(int i=0; i<n; i++) if ((value & (1<<i)) != 0) {buf << (empty ? "(" : " ") << table[i];}
	buf << ")"; return buf.str();}
static string choice_to_s(int value, int n, const char **table) {
	if (value < 0 || value >= n) {std::ostringstream buf; buf << "(Unknown #" << value << ")"; return buf.str();}
	else	return string(table[value]);}
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
	WH(fmt.pix.priv,"0x%08x");}

/* **************************************************************** */

\class FormatLibV4L : Format {
	uint8 *image;
	int queue[8], queuesize, queuemax, next_frame;
	int current_channel, current_tuner;
	P<BitPacking> bit_packing;
	P<Dim> dim;
	int fd;
	v4l2_format         fmt;
	v4l2_buffer         buf;
	v4l2_requestbuffers req;

	\constructor (string mode, string filename) {
		queuesize=0; queuemax=2; next_frame=0; bit_packing=0; dim=0;
		colorspace=gensym("none"); /* non-existent colorspace just to prevent crash in case of other problems */
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
	\decl 0 size (int sy, int sx);
	\decl 0 transfer (string sym, int queuemax=2);

	\attr t_symbol *colorspace;
	\attr int32  frequency();
	\attr uint16 brightness();
	\attr uint16 hue();
	\attr uint16 colour();
	\attr uint16 contrast();

	\attr uint16 white_mode(); /* 0..1 */
	\attr uint16 white_red();
	\attr uint16 white_blue();
	\attr int    auto_gain();
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
        f.pixelformat = V4L2_PIX_FMT_RGB24;
        f.field       = V4L2_FIELD_NONE /* V4L2_FIELD_INTERLACED */;
	if (debug) gfpost(&fmt);
	WIOCTL(fd, VIDIOC_S_FMT, &fmt);
	if (int(f.width)!=sy || int(f.height)!=sx)
                post("note: camera driver rejected (%d %d) resolution in favour of (%d %d)",
			sy,sx,f.height,f.width);
	sy = f.height;
	sx = f.width;
	if (debug) gfpost(&fmt);
}

void FormatLibV4L::dealloc_image () {if (image) {munmap(image,vmbuf.size); image=0;}}
void FormatLibV4L::alloc_image () {
	CLEAR(req);
        req.count = 2;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        WIOCTL2(fd, VIDIOC_REQBUFS, &req); //gfpost(&req);
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

void FormatVideoDev::frame_finished (uint8 *buf) {
	string cs = colorspace->s_name;
	int downscale = cs=="magic";
	/* picture is converted here. */
	int sy = dim->get(0);
	int sx = dim->get(1);
	int bs = dim->prod(1); if (downscale) bs/=2;
	uint8 b2[bs];
	//post("frame_finished sy=%d sx=%d bs=%d, vp.palette = %d; colorspace = %s",sy,sx,bs,vp.palette,cs.data());
	if (vp.palette==V4L2_PIX_FMT_YUYV) {
		uint8 *bufy = buf;
		GridOutlet out(this,0,cs=="magic"?new Dim(sy>>downscale,sx>>downscale,3):(Dim *)dim,cast);
		if (cs=="y") {
		    for(int y=0; y<sy; bufy+=sx, y++) {
			for (int x=0,xx=0; x<sx; x+=2,xx+=4) {
				b2[xx+0]=YUV2Y(bufy[x+0],0,0);
				b2[xx+1]=YUV2Y(bufy[x+2],0,0);
			}
			out.send(bs,b2);
		    }
		} else if (cs=="rgb") {
		    for(int y=0; y<sy; bufy+=2*sx, y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx*2; x+=4,xx+=6) {GETYUYV(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+3,Y2,U,V);}
			out.send(bs,b2);}
		} else if (cs=="rgba") {
		    for(int y=0; y<sy; bufy+=2*sx, y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx*2; x+=4,xx+=8) {GETYUYV(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+4,Y2,U,V);
				b2[xx+3]=255; b2[xx+7]=255;}
			out.send(bs,b2);}
		} else if (cs=="yuv") {
		    for(int y=0; y<sy; bufy+=2*sx, y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx*2; x+=4,xx+=6) {GETYUYV(x); YUV2YUV(b2+xx,Y1,U,V); YUV2YUV(b2+xx+3,Y2,U,V);}
			out.send(bs,b2);}
		} else if (cs=="magic") {
		    int sx2 = sx/2;
		    for(int y=0; y<sy; bufy+=2*sx, y+=2) {
 			for (int x=0,xx=0; x<sx2; x+=3,xx+=3) {b2[xx+0]=bufy[x+0]; b2[xx+1]=bufy[x+1]; b2[xx+2]=bufy[x+3];}
			out.send(bs,b2);
		    }
		}
	} else if (vp.palette==V4L2_PIX_FMT_YVU420) {
		uint8 *bufy = buf, *bufu = buf+sx*sy, *bufv = buf+sx*sy*5/4;
		GridOutlet out(this,0,cs=="magic"?new Dim(sy>>downscale,sx>>downscale,3):(Dim *)dim,cast);
		if (cs=="y") {
		    out.send(sy*sx,buf);
		} else if (cs=="rgb") {
		    for(int y=0; y<sy; bufy+=sx, bufu+=(sx/2)*(y&1), bufv+=(sx/2)*(y&1), y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx; x+=2,xx+=6) {GET420P(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+3,Y2,U,V);}
			out.send(bs,b2);}
		} else if (cs=="rgba") {
		    for(int y=0; y<sy; bufy+=sx, bufu+=(sx/2)*(y&1), bufv+=(sx/2)*(y&1), y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx; x+=2,xx+=8) {GET420P(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+4,Y2,U,V);
			b2[xx+3]=255; b2[xx+7]=255;}
			out.send(bs,b2);}
		} else if (cs=="yuv") {
		    for(int y=0; y<sy; bufy+=sx, bufu+=(sx/2)*(y&1), bufv+=(sx/2)*(y&1), y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx; x+=2,xx+=6) {GET420P(x); YUV2YUV(b2+xx,Y1,U,V); YUV2YUV(b2+xx+3,Y2,U,V);}
			out.send(bs,b2);}
		} else if (cs=="magic") {
		    int sx2 = sx/2;
		    for(int y=0; y<sy/2; bufy+=2*sx, bufu+=sx2, bufv+=sx2, y++) {
 			for (int x=0,xx=0; x<sx2; x++,xx+=3) {b2[xx+0]=bufy[x+x]; b2[xx+1]=bufu[x]; b2[xx+2]=bufv[x];}
			out.send(bs,b2);
		    }
		}
	} else if (vp.palette==V4L2_PIX_FMT_RGB24) {
		GridOutlet out(this,0,dim,cast);
		uint8 rgb[sx*4];
		uint8 b2[sx*3];
		if (cs=="y") {
			for(int y=0; y<sy; y++) {
			        bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					b2[x+0] = (76*rgb[xx+0]+150*rgb[xx+1]+29*rgb[xx+2])>>8;
					b2[x+1] = (76*rgb[xx+3]+150*rgb[xx+4]+29*rgb[xx+5])>>8;
				}
				out.send(bs,b2);
			}
		} else if (cs=="rgb") {
			for(int y=0; y<sy; y++) {
			        bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
			 	out.send(bs,rgb);
			}
		} else if (cs=="rgba") {
			for(int y=0; y<sy; y++) {
				uint8 *buf2 = buf+y*sx*bit_packing4->bytes;
			        bit_packing4->unpack(sx,buf2,rgb);
			        for (int x=0; x<sx; x++) buf2[4*x+3]=255; /* i hope this is correct. not tested. */
			 	out.send(bs,rgb);
			}
		} else if (cs=="yuv") {
			for(int y=0; y<sy; y++) {
				bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
				for (int x=0,xx=0; x<sx; x++,xx+=3) {
					b2[xx+0] = RGB2Y(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
					b2[xx+1] = RGB2U(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
					b2[xx+2] = RGB2V(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
				}
				out.send(bs,b2);
			}
		} else if (cs=="magic") RAISE("magic colorspace not supported with a RGB palette");
	} else {
		RAISE("unsupported palette %d",vp.palette);
	}
}

/* these are factors for RGB to analog YUV */
// Y =   66*R + 129*G +  25*B
// U = - 38*R -  74*G + 112*B
// V =  112*R -  94*G -  18*B

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

\def 0 tuner (int value) {
	v4l2_tuner vtuner;
	vtuner.index = current_tuner = value;
	if (0> IOCTL(fd, VIDIOC_G_TUNER, &vtuner)) RAISE("no tuner #%d", value);
	gfpost(&vtuner);
	WIOCTL(fd, VIDIOC_S_TUNER, &vtuner);
	int meuh;
	has_frequency = (v4l2_ioctl(fd, VIDIOC_G_FREQUENCY, &meuh)>=0);
}
\def int tuner () {return current_tuner;}

#define warn(fmt,stuff...) post("warning: " fmt,stuff)

\def 0 channel (int value) {
	v4l2_input vchan;
	vchan.index = current_channel = value;
	if (0> IOCTL(fd, VIDIOC_G_INPUT, &vchan)) warn("no channel #%d", value);
	//gfpost(&vchan);
	WIOCTL(fd, VIDIOC_S_INPUT, &vchan);
}
\def int channel () {return current_channel;}

\def 0 transfer (string sym, int queuemax=2) {
	RAISE("de que c'est?");
/*
	if (sym!="mmap") RAISE("don't know that transfer mode");
	dealloc_image();
	alloc_image();
	queuemax=min(8,min(queuemax,vmbuf.frames));
	post("transfer mmap with queuemax=%d (max max is vmbuf.frames=%d)", queuemax,vmbuf.frames);
	this->queuemax=queuemax;
*/
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
	int palette = (palettes&(1<<V4L2_PIX_FMT_RGB24 )) ? V4L2_PIX_FMT_RGB24 : V4L2_PIX_FMT_YVU420;
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

\def int auto_gain()        {return v4l2_get_control(fd,V4L2_CID_AUTOGAIN);}
\def 0   auto_gain (int auto_gain) {v4l2_set_control(fd,V4L2_CID_AUTOGAIN,auto_gain);}

\def uint16 white_mode () {return v4l2_get_control(fd,V4L2_CID_AUTO_WHITE_BALANCE);}
\def 0 white_mode (uint16 white_mode) {v4l2_set_control(fd,V4L2_CID_AUTO_WHITE_BALANCE,white_mode);}

\def uint16 white_red()        {return v4l2_get_control(fd,V4L2_CID_RED_BALANCE);}
\def 0 white_red(uint16 white_red)    {v4l2_set_control(fd,V4L2_CID_RED_BALANCE,white_red);}
\def uint16 white_blue()       {return v4l2_get_control(fd,V4L2_CID_RED_BALANCE);}
\def 0 white_blue(uint16 white_blue)  {v4l2_set_control(fd,V4L2_CID_RED_BALANCE,white_blue);}

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
	int checklist[] = {V4L2_PIX_FMT_RGB24,V4L2_PIX_FMT_YVU420};
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
