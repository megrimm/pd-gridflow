/*
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

#ifdef HAVE_LIBV4L2
#include <libv4l1.h>
//#define open   v4l2_open
#define close  v4l2_close
#define ioctl  v4l2_ioctl
#define mmap   v4l2_mmap
#define munmap v4l2_munmap
#define read   v4l2_read
#warning Using libv4l2 !!!
#else
#warning NOT Using libv4l2 !!!
#define v4l2_open(a,b) (RAISE("this [#io.v4l2] wasn't compiled with libv4l2 support"),-1)
#endif

/* **************************************************************** */

#define FLAG(_num_,_name_,_desc_) #_name_,
#define  OPT(_num_,_name_,_desc_) #_name_,
#define WH(_field_,_spec_)             oprintf(buf, "%s=" _spec_ " ", #_field_, self->_field_);
#define WHYX(_name_,_fieldy_,_fieldx_) oprintf(buf, "%s=(%d %d) ", #_name_, self->_fieldy_, self->_fieldx_);
#define WHFLAGS(_field_,_table_)       oprintf(buf, "%s:%s " ,#_field_,flags_to_s( self->_field_,COUNT(_table_),_table_).data());
#define WHCHOICE(_field_,_table_)      oprintf(buf, "%s=%s; ",#_field_,choice_to_s(self->_field_,COUNT(_table_),_table_).data());
static string flags_to_s(int value, int n, const char **table) {
	ostringstream buf; buf << "("; bool empty=true;
	for(int i=0; i<n; i++) if ((value & (1<<i)) != 0) {buf << (empty ? "(" : " ") << table[i];}
	buf << ")"; return buf.str();}
static string choice_to_s(int value, int n, const char **table) {
	if (value < 0 || value >= n) {ostringstream buf; buf << "(Unknown #" << value << ")"; return buf.str();}
	else	return string(table[value]);}
static void gfpost(v4l2_input *self) {ostringstream buf; buf << "[v4l2_input] ";
	WH(index,"%u");
	WH(name,"\"%.32s\"");
	WH(type,"0x%08x");
	WH(audioset,"0x%08x");
	WH(tuner,"%u");
	WH(std,"0x%08x");
	WH(status,"0x%08x");
	post("%s",buf.str().data());}
static void gfpost(v4l2_tuner *self) {ostringstream buf; buf << "[v4l2_tuner] ";
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
static void gfpost(v4l2_window *self) {ostringstream buf; buf << "[v4l2_window] ";
	WHYX(pos,w.top,w.left);
	WHYX(size,w.height,w.width);
//	enum v4l2_field  	field;
	WH(chromakey,"0x%08x");
	WH(clipcount,"%u");
	//WH(global_alpha,"%d"); but is a uint8.
	post("%s",buf.str().data());}
static void gfpost(v4l2_format *self) {ostringstream buf; buf << "[v4l2_format]";
	WH(type,"%u");
	if (self->type!=V4L2_BUF_TYPE_VIDEO_CAPTURE) return;
	WHYX(size,fmt.pix.height,fmt.pix.width);
	WH(fmt.pix.pixelformat,"%u");
	WH(fmt.pix.field,"0x%08x");
	WH(fmt.pix.bytesperline,"0x%08x");
	WH(fmt.pix.sizeimage,"0x%08x");
	WH(fmt.pix.colorspace,"0x%08x");
	WH(fmt.pix.priv,"0x%08x");}
static void gfpost(v4l2_requestbuffers *self) {ostringstream buf; buf << "[v4l2_requestbuffers]";
	WH(count,"%u");
	WH(type,"%u"); // enum v4l2_buf_type
	WH(memory,"%u"); // enum v4l2_memory
};

static t_symbol *safe_gensym (const char *s, int n=32) {
	int i; char buf[n+1]; memcpy(buf,s,n); buf[n]=0;
	for (i=n-1; buf[i] && !isspace(buf[i]); i--) buf[i]=0;
	for (i=0; buf[i]; i++) {
		if (isspace(buf[i])) buf[i]='_';
		if (buf[i]=='(') buf[i]='[';
		if (buf[i]==')') buf[i]=']';
	}
	return gensym(buf);
}


#define DEBUG(args...) 42
//#define DEBUG(args...) post(args)

#define  IOCTL( F,NAME,ARG) \
  (DEBUG("fd%d.ioctl(0x%08x,0x%08x)",F,NAME,ARG), v4l2_ioctl(F,NAME,ARG))
#define WIOCTL( F,NAME,ARG) \
  (IOCTL(F,NAME,ARG)<0 && (error("ioctl %s: %s",#NAME,strerror(errno)),1))
#define WIOCTL2(F,NAME,ARG) \
  (IOCTL(F,NAME,ARG)<0 && (error("ioctl %s: %s",#NAME,strerror(errno)), RAISE("ioctl error"), 0))

/* **************************************************************** */

struct Frame {uint8 *p; size_t n;};

\class FormatV4L2 : Format {
	uint8 *image;
	Frame queue[8];
	int queuesize, queuemax, next_frame;
	int current_channel, current_tuner;
	P<BitPacking> bit_packing3, bit_packing4;
	Dim dim;
	int fd;
	int palette;
	v4l2_format         fmt;
	v4l2_buffer         buf;
	v4l2_requestbuffers req;
	v4l2_capability     cap;
	\constructor (string mode, string filename, bool use_libv4l=false) {
		queuesize=0; queuemax=2; next_frame=0; palette=-1;
		colorspace=gensym("none"); /* non-existent colorspace just to prevent crash in case of other problems */
		image=0;
		if (use_libv4l) fd = v4l2_open(filename.data(),0);
		else            fd =      open(filename.data(),0);
		if (fd<0) RAISE("can't open device '%s': %s",filename.data(),strerror(errno));
		f=0;
		//WIOCTL(fd, VIDIOCGCAP, &cap);
		WIOCTL(fd, VIDIOC_QUERYCAP, &cap);
		//_0_size(0,0,cap.maxheight,cap.maxwidth);
		_0_size(240,320);
		t_symbol *card = safe_gensym((char *)cap.card,sizeof(cap.card));
		//t_symbol *bus = safe_gensym((char *)cap.bus_info,sizeof(cap.bus_info));
		//this->name = symprintf("%s_on_%s",card->s_name,bus->s_name);
		this->name = card;
		//WIOCTL(fd, VIDIOCGPICT,&vp);
		//int checklist[] = {V4L2_PIX_FMT_RGB24,V4L2_PIX_FMT_YVU420};
		_0_colorspace(gensym("rgb"));
		_0_channel(0);
	}
	void frame_finished (uint8 *buf);

	void alloc_image ();
	void dealloc_image ();
	void frame_ask ();
	~FormatV4L2 () {
		if (image) dealloc_image();
		close(fd); fd=-1; /* can be v4l2_close, not same as in formats.cxx */
	}

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

\def 0 get (t_symbol *s=0) {
	FObject::_0_get(s);
	// size are abnormal attributes (does not use nested list)
	if (!s) {
		t_atom a[2];
		//SETFLOAT(a+0,cap.minheight); SETFLOAT(a+1,cap.minwidth); outlet_anything(outlets[0],gensym("minsize"),2,a);
		//SETFLOAT(a+0,cap.maxheight); SETFLOAT(a+1,cap.maxwidth); outlet_anything(outlets[0],gensym("maxsize"),2,a);
		SETFLOAT(a+0,dim[0]); SETFLOAT(a+1,dim[1]); outlet_anything(outlets[0],gensym("size"),2,a);
		SETSYMBOL(a,gensym("mmap")); outlet_anything(outlets[0],gensym("transfer"),1,a);
	}
}

\def 0 size (int sy, int sx) {
	// !@#$ bug here: won't flush the frame queue
	dim = Dim(sy,sx,3);
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
	palette = f.pixelformat;
	if (debug) gfpost(&fmt);
}

void FormatV4L2::dealloc_image () {
  for (int i=0; i<queuemax; i++) {
    if (munmap(queue[i].p,queue[i].n)<0) post("v4l2: can't munmap?");
  }
  queuesize=0;
  //image=???;
}
void FormatV4L2::alloc_image () {
	//CLEAR(req); // ???
        //req.count = 2;
        //req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        //req.memory = V4L2_MEMORY_MMAP;
        //WIOCTL2(fd, VIDIOC_REQBUFS, &req); gfpost(&req);
	struct v4l2_buffer vbuf;
	memset (&vbuf,0,sizeof(vbuf));
	vbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vbuf.memory = V4L2_MEMORY_MMAP;
	vbuf.index  = 2;
        WIOCTL2(fd, VIDIOC_QUERYBUF, &vbuf);
	for (int i=0; i<queuemax; i++) {
		queue[i].n = buf.length;
		queue[i].p = (uint8 *)v4l2_mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if (queue[i].p == MAP_FAILED) post("v4l2: can't mmap");
	}
}

void FormatV4L2::frame_ask () {
	if (queuesize>=queuemax) RAISE("queue is full (queuemax=%d)",queuemax);
	//if (queuesize>=vmbuf.frames) RAISE("queue is full (vmbuf.frames=%d)",vmbuf.frames);
	//vmmap.frame = queue[queuesize++] = next_frame;
	//vmmap.format = vp.palette;
	//vmmap.width  = dim[1];
	//vmmap.height = dim[0];
	struct v4l2_buffer vbuf;
	memset(&vbuf, 0, sizeof(buf)); //CLEAR(vbuf);
	vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vbuf.memory = V4L2_MEMORY_MMAP;
	//WIOCTL2(fd, VIDIOCMCAPTURE, &vmmap);
	WIOCTL2(fd, VIDIOC_QBUF, &vbuf);
	//gfpost(&vmmap);
	next_frame = (next_frame+1) % queuemax;
}

//static uint8 clip(int x) {return x<0?0 : x>255?255 : x;}

void FormatV4L2::frame_finished (uint8 *buf) {
	string cs = colorspace->s_name;
	int downscale = cs=="magic";
	/* picture is converted here. */
	int sy = dim[0];
	int sx = dim[1];
	int bs = dim.prod(1); if (downscale) bs/=2;
	uint8 b2[bs];
	//post("frame_finished sy=%d sx=%d bs=%d, vp.palette = %d; colorspace = %s",sy,sx,bs,vp.palette,cs.data());
	int palette = fmt.fmt.pix.pixelformat;
	if (palette==V4L2_PIX_FMT_YUYV) {
		uint8 *bufy = buf;
		GridOut go(this,0,cs=="magic"?Dim(sy>>downscale,sx>>downscale,3):dim,cast);
		if (cs=="y") {
		    for(int y=0; y<sy; bufy+=sx, y++) {
			for (int x=0,xx=0; x<sx; x+=2,xx+=4) {
				b2[xx+0]=YUV2Y(bufy[x+0],0,0);
				b2[xx+1]=YUV2Y(bufy[x+2],0,0);
			}
			go.send(bs,b2);
		    }
		} else if (cs=="rgb") {
		    for(int y=0; y<sy; bufy+=2*sx, y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx*2; x+=4,xx+=6) {GETYUYV(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+3,Y2,U,V);}
			go.send(bs,b2);}
		} else if (cs=="rgba") {
		    for(int y=0; y<sy; bufy+=2*sx, y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx*2; x+=4,xx+=8) {GETYUYV(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+4,Y2,U,V);
				b2[xx+3]=255; b2[xx+7]=255;}
			go.send(bs,b2);}
		} else if (cs=="yuv") {
		    for(int y=0; y<sy; bufy+=2*sx, y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx*2; x+=4,xx+=6) {GETYUYV(x); YUV2YUV(b2+xx,Y1,U,V); YUV2YUV(b2+xx+3,Y2,U,V);}
			go.send(bs,b2);}
		} else if (cs=="magic") {
		    int sx2 = sx/2;
		    for(int y=0; y<sy; bufy+=2*sx, y+=2) {
 			for (int x=0,xx=0; x<sx2; x+=3,xx+=3) {b2[xx+0]=bufy[x+0]; b2[xx+1]=bufy[x+1]; b2[xx+2]=bufy[x+3];}
			go.send(bs,b2);
		    }
		}
	} else if (palette==V4L2_PIX_FMT_YVU420) {
		uint8 *bufy = buf, *bufu = buf+sx*sy, *bufv = buf+sx*sy*5/4;
		GridOut go(this,0,cs=="magic"?Dim(sy>>downscale,sx>>downscale,3):dim,cast);
		if (cs=="y") {
		    go.send(sy*sx,buf);
		} else if (cs=="rgb") {
		    for(int y=0; y<sy; bufy+=sx, bufu+=(sx/2)*(y&1), bufv+=(sx/2)*(y&1), y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx; x+=2,xx+=6) {GET420P(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+3,Y2,U,V);}
			go.send(bs,b2);}
		} else if (cs=="rgba") {
		    for(int y=0; y<sy; bufy+=sx, bufu+=(sx/2)*(y&1), bufv+=(sx/2)*(y&1), y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx; x+=2,xx+=8) {GET420P(x); YUV2RGB(b2+xx,Y1,U,V); YUV2RGB(b2+xx+4,Y2,U,V);
			b2[xx+3]=255; b2[xx+7]=255;}
			go.send(bs,b2);}
		} else if (cs=="yuv") {
		    for(int y=0; y<sy; bufy+=sx, bufu+=(sx/2)*(y&1), bufv+=(sx/2)*(y&1), y++) {int Y1,Y2,U,V;
			for (int x=0,xx=0; x<sx; x+=2,xx+=6) {GET420P(x); YUV2YUV(b2+xx,Y1,U,V); YUV2YUV(b2+xx+3,Y2,U,V);}
			go.send(bs,b2);}
		} else if (cs=="magic") {
		    int sx2 = sx/2;
		    for(int y=0; y<sy/2; bufy+=2*sx, bufu+=sx2, bufv+=sx2, y++) {
 			for (int x=0,xx=0; x<sx2; x++,xx+=3) {b2[xx+0]=bufy[x+x]; b2[xx+1]=bufu[x]; b2[xx+2]=bufv[x];}
			go.send(bs,b2);
		    }
		}
	} else if (palette==V4L2_PIX_FMT_RGB24) {
		GridOut go(this,0,dim,cast);
		uint8 rgb[sx*4];
		uint8 b2[sx*3];
		if (cs=="y") {
			for(int y=0; y<sy; y++) {
			        bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
				for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
					b2[x+0] = (76*rgb[xx+0]+150*rgb[xx+1]+29*rgb[xx+2])>>8;
					b2[x+1] = (76*rgb[xx+3]+150*rgb[xx+4]+29*rgb[xx+5])>>8;
				}
				go.send(bs,b2);
			}
		} else if (cs=="rgb") {
			for(int y=0; y<sy; y++) {
			        bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
			 	go.send(bs,rgb);
			}
		} else if (cs=="rgba") {
			for(int y=0; y<sy; y++) {
				uint8 *buf2 = buf+y*sx*bit_packing4->bytes;
			        bit_packing4->unpack(sx,buf2,rgb);
			        for (int x=0; x<sx; x++) buf2[4*x+3]=255; /* i hope this is correct. not tested. */
			 	go.send(bs,rgb);
			}
		} else if (cs=="yuv") {
			for(int y=0; y<sy; y++) {
				bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
				for (int x=0,xx=0; x<sx; x++,xx+=3) {
					b2[xx+0] = RGB2Y(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
					b2[xx+1] = RGB2U(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
					b2[xx+2] = RGB2V(rgb[xx+0],rgb[xx+1],rgb[xx+2]);
				}
				go.send(bs,b2);
			}
		} else if (cs=="magic") RAISE("magic colorspace not supported with a RGB palette");
	} else {
		RAISE("unsupported palette %d",palette);
	}
}

/* these are factors for RGB to analog YUV */
// Y =   66*R + 129*G +  25*B
// U = - 38*R -  74*G + 112*B
// V =  112*R -  94*G -  18*B

\def 0 bang () {
	if (!image) alloc_image();
	while(queuesize<queuemax) frame_ask();
	struct v4l2_buffer vbuf;
	memset(&vbuf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	//uint64 t0 = gf_timeofday();
	WIOCTL2(fd, VIDIOC_DQBUF, &buf);
	//uint64 t1 = gf_timeofday();
	//if (t1-t0 > 100) gfpost("VIDIOC_DQBUF delay: %d us",t1-t0);
	frame_finished(queue[0].p);
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
	//int meuh; has_frequency = (v4l2_ioctl(fd, VIDIOC_G_FREQUENCY, &meuh)>=0);
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
	if (palette==V4L2_PIX_FMT_RGB24) {
		uint32 masks[3]={0xff0000,0x00ff00,0x0000ff};
		bit_packing3 = new BitPacking(is_le(),3,3,masks);
		bit_packing4 = new BitPacking(is_le(),3,4,masks);
	} // else RAISE("huh?");
	this->colorspace=gensym(c.data());
	dim = Dim(dim[0],dim[1],c=="y"?1:3);
}

\def int auto_gain()        {return v4l2_get_control(fd,V4L2_CID_AUTOGAIN);}
\def 0   auto_gain (int auto_gain) {v4l2_set_control(fd,V4L2_CID_AUTOGAIN,auto_gain);}

\def uint16 white_mode () {return v4l2_get_control(fd,V4L2_CID_AUTO_WHITE_BALANCE);}
\def 0 white_mode (uint16 white_mode) {v4l2_set_control(fd,V4L2_CID_AUTO_WHITE_BALANCE,white_mode);}

\def uint16 white_red()        {return v4l2_get_control(fd,V4L2_CID_RED_BALANCE);}
\def 0 white_red(uint16 white_red)    {v4l2_set_control(fd,V4L2_CID_RED_BALANCE,white_red);}
\def uint16 white_blue()       {return v4l2_get_control(fd,V4L2_CID_RED_BALANCE);}
\def 0 white_blue(uint16 white_blue)  {v4l2_set_control(fd,V4L2_CID_RED_BALANCE,white_blue);}

\end class FormatV4L2 {install_format("#io.v4l2",4,"");}
void startup_v4l2 () {
	\startall
}
