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

#define CHECK_ALIGN(d,nt) \
	{int bytes = 16; \
	int align = ((ulong)(void*)d)%bytes; \
	if (align) {_L_;gfpost("%s(%s): Alignment Warning: %s=%p is not %d-aligned: %d", \
		INFO(this), __PRETTY_FUNCTION__,#d,(void*)d,bytes,align);}}

\class GridFFT < GridObject
struct GridFFT : GridObject {
	fftwf_plan plan;
	P<Dim> lastdim; /* of last input */
	\attr int sign; /* -1 or +1 */
	\attr int skip; /* 0.. */
	\decl void initialize ();
	\grin 0 float
};

\def void _0_sign (int sign) {
	if (sign!=-1 && sign!=1) RAISE("sign should be -1 or +1");
	this->sign=sign;
	fftwf_destroy_plan(plan);
}

\def void _0_skip (int skip) {
	if (skip<0 || skip>1) RAISE("skip should be 0 or 1");
	this->skip=skip;
	fftwf_destroy_plan(plan);
}

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
	CHECK_ALIGN(data,in->nt)
	CHECK_ALIGN(buf, in->nt)
	if (plan && lastdim && lastdim!=in->dim) fftwf_destroy_plan(plan);
	fftwf_complex *ip = (fftwf_complex *)data;
	fftwf_complex *op = (fftwf_complex *)buf;
	int v[] = {in->dim->v[0],in->dim->v[1]};
	if (skip==0) plan = fftwf_plan_dft_2d(v[0],v[1],ip,op,sign,0);
	if (skip==1) plan = fftwf_plan_many_dft(1,&v[1],v[0],ip,0,1,v[1],op,0,1,v[1],sign,0);
	fftwf_execute_dft(plan,ip,op);
	GridOutlet out(this,0,in->dim,in->nt);
	out.send(n,buf);
	delete[] buf;
} GRID_END

\def void initialize () {
	rb_call_super(argc,argv);
	sign = -1;
	plan = 0;
	lastdim = 0;
	skip = 0;
}

\classinfo {
	IEVAL(rself,"install '#fft',1,1;");
}
\end class GridFFT

void startup_fftw () {
	\startall
}

