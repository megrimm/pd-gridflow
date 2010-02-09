/*
	$Id$

	GridFlow
	Copyright (c) 2001-2009 by Mathieu Bouchard

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

#define NETPBM_INCLUDE_HERE
#include "gridflow.hxx.fcs"

\class FormatNetPBM : Format {
	struct pam inpam, outpam;
	\grin 0
	\constructor (t_symbol *mode, string filename) {
		Format::_0_open(0,0,mode,filename);
		memset(& inpam,0,sizeof(pam));
		memset(&outpam,0,sizeof(pam));
	}
	\decl 0 bang ();
};
\def 0 bang () {
	//inpam.allocation_depth = 3;
	pnm_readpaminit(f, &inpam, /*PAM_STRUCT_SIZE(tuple_type)*/ sizeof(struct pam));
	tuple *tuplerow = pnm_allocpamrow(&inpam);
	if (inpam.depth!=3) RAISE("image has %d channels instead of 3 channels",inpam.depth);
	GridOutlet out(this,0,new Dim(inpam.height,inpam.width,inpam.depth),cast);
	uint8 buf[inpam.width*3];
	for (int i=0; i<inpam.height; i++) {
		pnm_readpamrow(&inpam, tuplerow);
		for (int j=0; j<inpam.width; j++) {
			buf[j*3+0] = tuplerow[j][0];
			buf[j*3+1] = tuplerow[j][1];
			buf[j*3+2] = tuplerow[j][2];
		}
		out.send(inpam.width*inpam.depth,buf);
	}
	pnm_freepamrow(tuplerow);
}
GRID_INLET(0) {
	if (in->dim->n!=3) RAISE("need 3 dimensions");
	if (in->dim->v[2]!=3) RAISE("need 3 channels");
	outpam.size = sizeof(struct pam);
	outpam.len  = sizeof(struct pam);
	outpam.file = f;
	outpam.format = PPM_FORMAT;
	outpam.height = in->dim->v[0];
	outpam.width = in->dim->v[1];
	outpam.depth = in->dim->v[2];
	outpam.plainformat = false;
	outpam.maxval = 255;
	//outpam.allocation_depth = 3;
	strcpy(outpam.tuple_type,PAM_PPM_TUPLETYPE);
	pnm_writepaminit(&outpam);
	in->set_chunk(1);
} GRID_FLOW {
	tuple *tuplerow = pnm_allocpamrow(&outpam);
	int m = in->dim->v[1];
	for (int i=0; i<n; i+=in->dim->prod(1)) {
		for (int j=0; j<m; j++, data+=3) {
			tuplerow[j][0] = int(data[0]);
			tuplerow[j][1] = int(data[1]);
			tuplerow[j][2] = int(data[2]);
		}
		pnm_writepamrow(&outpam, tuplerow);
	}
	pnm_freepamrow(tuplerow);
} GRID_FINISH {
	fflush(f);
} GRID_END
/* was supposed to be "#io.netpbm" but there's backwards compat. */
\classinfo {install_format("#io.ppm",6,"ppm pgm pnm pam");}
\end class FormatNetPBM

void startup_netpbm () {
	pm_init(0,0);
	\startall
}
