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

extern "C" {
#include "bundled/pdp_imagebase.h"
};
#include "gridflow.hxx.fcs"
#include "bundled/m_imp.h"
#include "colorspace.hxx"
int slowclip (int x) {return clip(x,0,255);}
#define fastclip slowclip
//------------------------------------------------------------------------
\class GridToPdp : FObject {
	\attr bool scale;
	\attr bool shift;
	\attr t_symbol *colorspace;
	\constructor () {scale=0; shift=7; colorspace=gensym("rgb");}
	\grin 0 int
};
GRID_INLET(0) {
	if (in.dim.n!=3) RAISE("expecting 3 dimensions: rows,columns,channels");
	//if (in.dim[2]!=3 && in.dim[2]!=4) RAISE("expecting 3 or 4 channels: red,green,blue,ignored (got %d)",in.dim[2]);
	if (in.dim[2]!=3) RAISE("expecting 3 4 channels: red,green,blue (got %d)",in.dim[2]);
	if (in.dim[0]&1) RAISE("can't have odd number of rows");
	if (in.dim[1]&1) RAISE("can't have odd number of columns");
	in.set_chunk(0);
} GRID_FLOW {
	int sy=in.dim[0];
	int sx=in.dim[1];
	int packet = pdp_packet_new_image(PDP_IMAGE_YV12,in.dim[1],in.dim[0]);
	t_pdp *header = pdp_packet_header(packet);
	if (!header) RAISE("can't allocate packet");
	short *tada = (short *)pdp_packet_data(packet);
	T *d;
	d=data;	for (int y=0; y<sy; y++          ) for (int x=0; x<sx; x++ ,d+=3) *tada++ =  RGB2Y_(d[0],d[1],d[2]     )<<7;
	d=data; for (int y=0; y<sy; y+=2, d+=3*sx) for (int x=0; x<sx; x+=2,d+=6) *tada++ = (RGB2V_(d[0],d[1],d[2])-128)<<8;
	d=data; for (int y=0; y<sy; y+=2, d+=3*sx) for (int x=0; x<sx; x+=2,d+=6) *tada++ = (RGB2U_(d[0],d[1],d[2])-128)<<8;

	pdp_packet_pass_if_valid(outlets[0],&packet);
} GRID_FINISH {
} GRID_END
\end class {install("#to_pdp",1,1);}

//------------------------------------------------------------------------
static t_class *pdpproxy_class;
struct GridFromPDP;
struct pdpproxy : t_pdp_imagebase {GridFromPDP *daddy;};
static void pdpproxy_process(pdpproxy *self) {
	int p0 = pdp_base_get_packet(self,0);
	t_pdp *head = pdp_packet_header(p0);
	unsigned sy = head->info.image.height;
	unsigned sx = head->info.image.width;
	short *datay = (short *)pdp_packet_data(p0);
	short *datav = datay + sy*sx;
	short *datau = datav + (sy/2)*(sx/2);
	int32 tada2[sy*sx*3]; int32 *tada = tada2;
	for (unsigned y=0; y<sy; y++, datau-=sx/2*(y&1), datav-=sx/2*(y&1)) {
		for (unsigned x=0; x<sx; x+=2, datay+=2, datau++, datav++) {
			int u = 128 + (datau[0]>>8);
			int v = 128 + (datav[0]>>8);
			*tada++ = YUV2R(datay[0]>>7,u,v);
			*tada++ = YUV2G(datay[0]>>7,u,v);
			*tada++ = YUV2B(datay[0]>>7,u,v);
			*tada++ = YUV2R(datay[1]>>7,u,v);
			*tada++ = YUV2G(datay[1]>>7,u,v);
			*tada++ = YUV2B(datay[1]>>7,u,v);
		}
	}
	GridOutlet o((FObject *)self->daddy,0,Dim(sy,sx,3),int32_e); // why (FObject *) cast ???
	o.send(sy*sx*3,tada2);
}
static void pdpproxy_free(pdpproxy *x) {pdp_imagebase_free(x);}
static void *pdpproxy_new () {
	pdpproxy *self = (pdpproxy *)pd_new(pdpproxy_class);
	pdp_imagebase_init(self);
	pdp_base_set_process_method(self, (t_pdp_method)pdpproxy_process);
	return self;
}
\class GridFromPDP : FObject {
	\attr bool scale;
	\attr bool shift;
	pdpproxy *bitch;
	\attr t_symbol *colorspace;
	\constructor () {
		scale=0; shift=7; colorspace=gensym("rgb");
		pd_typedmess(&pd_objectmaker,gensym("#to_pdp_proxy"),0,0);
		bitch = (pdpproxy *)pd_newest();
		if (!bitch) RAISE("bitchless");
		bitch->daddy = this;
		//inlet_new((t_object *)bself,(t_pd *)bitch,0,0);
	}
	~GridFromPDP () {
		//pd_free((t_pd *)bitch); // crashes
	}
	\decl 0 pdp (...) {typedmess((t_pd *)bitch,gensym("pdp"),argc,argv);}
};
\end class {install("#from_pdp",1,1);}
//------------------------------------------------------------------------

extern "C" void gridflow_pdp_setup() {
	\startall
	pdpproxy_class =
	  class_new(gensym("#to_pdp_proxy"), (t_newmethod)pdpproxy_new, (t_method)pdpproxy_free, sizeof(pdpproxy),0,A_NULL);
	pdp_imagebase_setup(pdpproxy_class);
}
