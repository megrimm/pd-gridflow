/*
	GridFlow
	Copyright (c) 2001-2006 by Mathieu Bouchard

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
#include <fftw3.h>

\class GridFFT < GridObject
struct GridFFT : GridObject {
	\decl void initialize ();
	\grin 0 float
};

GRID_INLET(GridFFT,0) {
	if (in->nt != float32_e)
		RAISE("expecting float32");
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2)!=2)
		RAISE("expecting 2 channels: real,imaginary (got %d)",in->dim->get(2));
	in->set_factor(in->dim->prod());
} GRID_FLOW {
	float32 *buf = new float32[n];
	fftwf_plan plan = fftwf_plan_dft_2d(
		in->dim->v[0],in->dim->v[1],
		(fftwf_complex *)data,
		(fftwf_complex *)buf,-1,0);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);
	GridOutlet out(this,0,in->dim,in->nt);
	out.send(n,buf);
	delete[] buf;
} GRID_END

\def void initialize () {
	rb_call_super(argc,argv);
}

\classinfo {
	IEVAL(rself,"install '#fft',1,1;");
}
\end class GridFFT

void startup_fftw () {
	\startall
}

