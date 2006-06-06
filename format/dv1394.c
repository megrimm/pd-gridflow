/*
	$Id$

	GridFlow
	Copyright (c) 2001-2006 by Mathieu Bouchard
	portions Copyright (c) 2002 Iohannes M Zmölnig (from GEM)

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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

/* including a kernel header from the kernel */
#include "dv1394.h"

/* GAAH!!!!! */
#define DV1394_WAIT_FRAMES    DV1394_IOC_WAIT_FRAMES
#define DV1394_GET_STATUS     DV1394_IOC_GET_STATUS
#define DV1394_RECEIVE_FRAMES DV1394_IOC_RECEIVE_FRAMES
#define DV1394_START_RECEIVE  DV1394_IOC_START_RECEIVE
#define DV1394_INIT           DV1394_IOC_INIT
#define DV1394_SHUTDOWN       DV1394_IOC_SHUTDOWN

/* ARGH!!!!! */
#define __dev_t_defined
#define __gid_t_defined
#define __mode_t_defined
#define __nlink_t_defined
#define __uid_t_defined
//#define _SYS_SELECT_H

#include "../base/grid.h.fcs"
#include <libdv/dv.h>

#define N_BUF 2 /*DV1394_MAX_FRAMES/4*/
#define PAL 0
#define NTSC 1

\class FormatDV1394 < Format
struct FormatDV1394 : Format {
	bool m_capturing, m_haveVideo;
	int m_width, m_height;
	int m_channel, m_norm, m_reqFormat;
	char*m_devicename; int m_devicenum; // specify either devicename XOR devicenum
	int m_quality;

	int dvfd;
	unsigned char *videobuf;
	unsigned char *decodedbuf;
	bool m_frame_ready;
	int  m_frame, m_lastframe;
	//  static void*capturing(void*);
	bool m_continue_thread;
	pthread_t m_thread_id;
	int m_framesize;
	unsigned char *m_mmapbuf;
	dv_decoder_t *m_decoder;

	\decl void initialize (Symbol mode);
	\decl void frame ();
	\decl void close ();
	void *capturing ();
	int openDevice(int format);
	int startTransfer();
	void stopTransfer();
	void setNorm(char*norm);
	void setDevice(int d);
	void setDevice(char*name);
	void setColor(int format);
	void setQuality(int quality);
};

\def void initialize(Symbol mode) {
	gfpost("DV1394: hello world");
	m_channel = 0;//0x63;
	m_devicenum  = 0;
	m_norm = PAL;
	//  m_decoder=NULL;
	m_frame_ready=false;
	m_quality = DV_QUALITY_BEST;
	decodedbuf = new unsigned char[720*576*3];
	m_haveVideo=false;
}

\def void frame () {
  if (!m_decoder) {gfpost("dv1394#frame SKIPPED (no m_decoder)"); return;}
  gfpost("dv1394#frame START");
  dv_parse_header(m_decoder, videobuf);
  //dv_parse_packs (m_decoder, videobuf);
  if(dv_frame_changed(m_decoder)) {
    int pitches[3] = {0,0,0};
    pitches[0]=m_decoder->width*3; // rgb
    GridOutlet out(this,0,new Dim(m_decoder->height,m_decoder->width,3),
	NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
    // decode the DV-data to something we can handle and that is similar to the wanted format
    //    dv_report_video_error(m_decoder, videobuf);  // do we need this ?
    // gosh, this(e_dv_color_rgb) is expansive:: the decoding is done in software only...
    //      dv_decode_full_frame(m_decoder, videobuf, ((m_reqFormat==GL_RGBA)?e_dv_color_rgb:e_dv_color_yuv), &decodedbuf, pitches);
    //dv_decode_full_frame(m_decoder, videobuf, e_dv_color_yuv, &decodedbuf, pitches);
      dv_decode_full_frame(m_decoder, videobuf, e_dv_color_rgb, &decodedbuf, pitches);
    //gfpost("sampling %d", m_decoder->sampling);
  }
  m_frame_ready = false;
  // oops, we don't have a frame at this point?? -- matju
  gfpost("dv1394#frame END");
}

static void *capturing (void *foo) {return ((FormatDV1394 *)foo)->capturing();}

void *FormatDV1394::capturing () {
  gfpost("dv1394#capturing START");
  int fd=dvfd;
  int framesize = m_framesize;
  struct dv1394_status dvst;
  int n_frames = N_BUF;
  unsigned char* mmapbuf = m_mmapbuf;

  // this will hang if no ieee1394-device is present, what to do about it ???
  m_haveVideo=false;
  if(ioctl(fd, DV1394_WAIT_FRAMES, 1)) {
    perror("error: ioctl WAIT_FRAMES");
    m_capturing=false; goto end;
  }
  if (ioctl(fd, DV1394_GET_STATUS, &dvst))   {
    perror("ioctl GET_STATUS");
    m_capturing=false; goto end;
  }
  m_haveVideo=true;
  m_capturing=true;

  while(m_continue_thread) {
    if(ioctl(fd, DV1394_WAIT_FRAMES, n_frames - 1)) {perror("error: ioctl WAIT_FRAMES"); m_capturing=false; goto end;}
    if (ioctl(fd, DV1394_GET_STATUS, &dvst))        {perror("ioctl GET_STATUS");         m_capturing=false; goto end;}
    // dvst.init dvst.active_frame dvst.first_clear_frame dvst.n_clear_frames dvst.dropped_frames
    if (dvst.dropped_frames > 0) {
      gfpost("dv1394: dropped at least %d frames.", dvst.dropped_frames);
    }
    //memcpy( g_current_fradata, (g_dv1394_map + (dvst.first_clear_frame * DV1394_PAL_FRAME_SIZE)), DV1394_PAL_FRAME_SIZE);
    videobuf = mmapbuf + (dvst.first_clear_frame * framesize);

    //gfpost("thread %d\t%x %x", frame, tvfd, vmmap);
    if (ioctl(fd, DV1394_RECEIVE_FRAMES, 1) < 0) perror("receiving...");
    m_lastframe=m_frame;
    m_frame++;
    m_frame%=N_BUF;
    m_frame_ready = true;
  }
  m_capturing=false;
end:
  gfpost("dv1394#capturing END");
  return NULL;
}

int FormatDV1394::openDevice(int format) {
  gfpost("dv1394#openDevice START");
  int fd = -1;
  char buf[255];
  struct dv1394_init init = {
    DV1394_API_VERSION,// api version 
    0x63,              // isochronous transmission channel
    N_BUF,             // number of frames in ringbuffer
    (m_norm==NTSC)?DV1394_NTSC:DV1394_PAL,// PAL or NTSC
    0,0,0              // default packet rate
  };
  m_framesize=(m_norm==NTSC)?DV1394_NTSC_FRAME_SIZE:DV1394_PAL_FRAME_SIZE;
  if(m_devicename) {
    if ((fd = open(m_devicename, O_RDWR)) < 0) {perror(buf); goto err;}
  } else {
    int devnum=(m_devicenum<0)?0:m_devicenum;
    if (devnum<0)devnum=0;
    sprintf(buf, "/dev/ieee1394/dv/host%d/%s/in", devnum, (m_norm==NTSC)?"NTSC":"PAL");
    if ((fd = open(buf, O_RDWR)) < 0) {
      if ((fd=open("/dev/dv1394", O_RDWR)) < 0) {perror(buf); goto err;}
    }
  }
  if (ioctl(fd, DV1394_INIT, &init) < 0) {perror("initializing"); ::close(fd); goto err;}
  m_mmapbuf = (unsigned char *) mmap( NULL, N_BUF*m_framesize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if(m_mmapbuf == MAP_FAILED) {perror("mmap frame buffers"); ::close(fd); goto err;}
  if(ioctl(fd, DV1394_START_RECEIVE, NULL)) {perror("dv1394 START_RECEIVE ioctl"); ::close(fd); goto err;}
  gfpost("dv1394#openDevice END %d",fd);
  return(fd);
err:
  gfpost("dv1394#openDevice END -1");
  return -1;
}

int FormatDV1394::startTransfer() {
  gfpost("dv1394#startTransfer START");
  int format=0; // use to be a method arg
  if ((dvfd=openDevice(format))<0) {gfpost("DV4L: closed"); goto err;}
  //new Dim(576,720,3);
  videobuf=NULL;
  m_frame_ready = false; 
  if(m_decoder!=NULL) dv_decoder_free(m_decoder);
  if (!(m_decoder=dv_decoder_new(true,true,true))) {gfpost("DV4L: unable to create DV-decoder...closing"); close(0,0); goto err;}
  m_decoder->quality=m_quality;
  gfpost("DV4L: DV decoding quality %d ", m_decoder->quality);
  m_continue_thread = true;
  pthread_create(&m_thread_id, 0, ::capturing, this);
  gfpost("dv1394#startTransfer END 1");
  return 1;
err:
  gfpost("dv1394#startTransfer END 0");
  return 0;

}

void FormatDV1394::stopTransfer() {
  gfpost("dv1394#stopTransfer START");
  // close the dv4l device and dealloc buffer; terminate thread if there is one
  m_continue_thread=false;
  int i=0;
  if(m_haveVideo){
    while(m_capturing){usleep(10);i++;}
    gfpost("DV4L: shutting down dv1394 after %d usec", i*10);
    ioctl(dvfd, DV1394_SHUTDOWN);
  }
  close(0,0);
  gfpost("dv1394#stopTransfer STOP");
}

void FormatDV1394::setNorm(char*norm) {
  int inorm = m_norm;
  switch(norm[0]){
  case 'N': case 'n': inorm=NTSC; break;
  case 'P': case 'p': inorm=PAL;  break;
  }
  if (inorm==m_norm) return;
  m_norm=inorm;
}

void FormatDV1394::setDevice(int d) {
  m_devicename=NULL;
  if (d==m_devicenum)return;
  m_devicenum=d;
  if(m_haveVideo) {stopTransfer(); startTransfer();}
}

void FormatDV1394::setDevice(char*name) {
  m_devicenum=-1;
  m_devicename=name;
  if(m_haveVideo) {stopTransfer(); startTransfer();}
}

void FormatDV1394::setColor(int format) {
  if (format<=0) return;
  m_reqFormat=format;
}

void FormatDV1394::setQuality(int quality) {
  if (quality<0 || quality>5) return;
  m_quality=quality;
  if(m_haveVideo) {stopTransfer(); startTransfer();}
}

\def void close () {
  if(m_mmapbuf!=NULL)munmap(m_mmapbuf, N_BUF*m_framesize);
  if(dvfd>=0) ::close(dvfd);
  m_haveVideo=false;
  if (m_haveVideo) stopTransfer();
  if (decodedbuf) delete[] decodedbuf;
  if (m_decoder!=NULL) dv_decoder_free(m_decoder);
}

\classinfo {
	IEVAL(rself,"install '#io:dv1394',1,1;@flags=4;@comment='FireWire (IEEE1394) DV'");
	//IEVAL(rself,ruby_code);
//	rb_funcall(rself,SI(instance_eval),3,rb_str_new2(ruby_code),
//		rb_str_new2(__FILE__),INT2NUM(ruby_lineno+3));
}
\end class FormatDV1394

void startup_dv1394 () {
	\startall
}

