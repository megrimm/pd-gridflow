/*
	$Id$

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

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

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "../gridflow.h.fcs"
#include <ctype.h>

//#define TRACEBUFS

#define CHECK_TYPE(d) if (NumberTypeE_type_of(&d)!=this->nt) RAISE("(%s): " \
		"type mismatch during transmission (got %s expecting %s)", __PRETTY_FUNCTION__, \
		number_type_table[NumberTypeE_type_of(&d)].name, \
		number_type_table[this->nt].name);
#define CHECK_BUSY1(s) if (!dim) RAISE(#s " not busy");
#define CHECK_BUSY(s)  if (!dim) RAISE(#s " not busy (wanting to write %ld values)",(long)n);
#define CHECK_ALIGN(d)     {int bytes = number_type_table[nt].size/8; int align = ((long)(void*)d)%bytes; \
	if (align) {post("(%s): Alignment Warning: %p is not %d-aligned: %d", __PRETTY_FUNCTION__, (void*)d,bytes,align);}}
#define CHECK_ALIGN2(d,nt) {int bytes = number_type_table[nt].size/8; int align = ((long)(void*)d)%bytes; \
	if (align) {post("Alignment Warning: %p is not %d-aligned: %d", (void*)d,bytes,align);}}

// **************** Grid ******************************************

void Grid::init_from_list(int n, t_atom *aa, NumberTypeE nt) {
	t_atom2 *a = (t_atom2 *)aa;
	t_symbol *delim = gensym("#");
	for (int i=0; i<n; i++) {
		if (a[i] == delim) {
			int32 v[i];
			if (i!=0 && a[i-1].a_type==A_SYMBOL) nt=NumberTypeE_find(a[--i]);
			for (int j=0; j<i; j++) v[j] = convert(a[j],(int32*)0);
			init(new Dim(i,v),nt);
			CHECK_ALIGN2(this->data,nt);
			if (a[i] != delim) i++;
			i++; a+=i; n-=i;
			goto fill;
		}
	}
	if (n!=0 && a[0].a_type==A_SYMBOL) {nt = NumberTypeE_find(a[0]); a++; n--;}
	init(new Dim(n),nt);
	CHECK_ALIGN2(this->data,nt);
	fill:
	int nn = dim->prod();
	n = min(n,nn);
#define FOO(T) { \
	T *p = (T *)*this; \
	if (n==0) CLEAR(p,nn); else { \
		for (int i=0; i<n; i++) p[i] = a[i]; \
		for (int i=n; i<nn; i+=n) COPY(p+i,p,min(n,nn-i)); }}
	TYPESWITCH(nt,FOO,)
#undef FOO
}

void Grid::init_from_atom(const t_atom &x) {
	const t_atom2 &a = *(t_atom2 *)&x;
	if (a.a_type==A_LIST) {
		t_binbuf *b = a;
		init_from_list(binbuf_getnatom(b),binbuf_getvec(b));
	} else if (x.a_type==A_FLOAT) {
		init(new Dim(),int32_e);
		CHECK_ALIGN2(this->data,nt);
		((int32 *)*this)[0] = (int32)a.a_float;
	} else RAISE("can't convert to grid");
}

// **************** GridInlet *************************************

// must be set before the end of GRID_BEGIN phase, and so cannot be changed
// afterwards. This is to allow some optimisations. Anyway there is no good reason
// why this would be changed afterwards.
void GridInlet::set_chunk(long whichdim) {
	chunk = whichdim;
	long n = dim->prod(whichdim);
	if (!n) n=1;
	if(!dim) RAISE("huh?");
	if (n>1) {
		buf=new Grid(new Dim(n), nt);
		bufi=0;
	} else buf=0;
}

bool GridInlet::supports_type(NumberTypeE nt) {
#define FOO(T) return !! gh->flow_##T;
	TYPESWITCH(nt,FOO,return false)
#undef FOO
}

void GridInlet::begin(GridOutlet *back_out) {
	nt = back_out->nt;
	if (dim) RAISE("grid inlet aborting from %s at %ld/%ld because of %s",
			ARGS(sender),long(dex),long(dim->prod()),ARGS(back_out->parent));
	sender = back_out->parent;
	if ((int)nt<0 || (int)nt>=(int)number_type_table_end) RAISE("inlet: unknown number type");
	if (!supports_type(nt)) RAISE("number type %s not supported here", number_type_table[nt].name);
	P<Dim> dim = this->dim = back_out->dim;
	dex=0;
	buf=0;
	try {
#define FOO(T) gh->flow(this,-1,(T *)0); break;
		TYPESWITCH(this->nt,FOO,)
#undef FOO
	} catch (Barf &barf) {this->dim=0; throw;}
	this->dim = dim;
	back_out->callback(this);
#ifdef TRACEBUFS
	post("GridInlet:  %20s buf for recving from %p",dim->to_s(),sender);
#endif
}

#define CATCH_IT catch (Barf &slimy) {slimy.error(parent->bself);}

template <class T> void GridInlet::flow(long n, T *data) {
	CHECK_BUSY(inlet); CHECK_TYPE(*data); CHECK_ALIGN(data);
	if (!mode) {dex += n; return;} // ignore data
	if (!n) return; // no data
	long d = dex + bufi;
	if (d+n > dim->prod()) {
		post("grid input overflow: %d of %d from [%s] to [%s]", d+n, dim->prod(), ARGS(sender), 0);
		n = dim->prod() - d;
		if (n<=0) return;
	}
	int bufn = factor();
	if (buf && bufi) {
		T *bufd = *buf;
		long k = min((long)n,bufn-bufi);
		COPY(bufd+bufi,data,k);
		bufi+=k; data+=k; n-=k;
		if (bufi==bufn) {
			long newdex = dex+bufn;
			CHECK_ALIGN(bufd);
			try {gh->flow(this,bufn,bufd);} CATCH_IT;
			dex = newdex;
			bufi = 0;
		}
	}
	int m = (n/bufn)*bufn;
	if (m) {
		int newdex = dex + m;
		try {gh->flow(this,m,data);} CATCH_IT;
		dex = newdex;
	}
	data += m;
	n -= m;
	if (buf && n>0) COPY((T *)*buf+bufi,data,n), bufi+=n;
}

void GridInlet::finish() {
	if (!dim) RAISE("inlet not busy");
	if (dim->prod() != dex) post("%s: incomplete grid: %d of %d from [%s] to [%s]",ARGS(parent),dex,dim->prod(),ARGS(sender));
#define FOO(T) try {gh->flow(this,-2,(T *)0);} CATCH_IT;
	TYPESWITCH(nt,FOO,)
#undef FOO
	dim=0; buf=0; dex=0;
}

template <class T> void GridInlet::from_grid2(Grid *g, T foo) {
	nt = g->nt;
	dim = g->dim;
	int n = g->dim->prod();
	gh->flow(this,-1,(T *)0);
	if (n>0 && this->mode!=0) {
		T *data = (T *)*g;
		CHECK_ALIGN(data);
		//int ntsz = number_type_table[nt].size;
		int m = GridOutlet::MAX_PACKET_SIZE/*/ntsz*//factor();
		if (!m) m++;
		m *= factor();
		while (n) {
			if (m>n) m=n;
			CHECK_ALIGN(data);
			try {gh->flow(this,m,data);} CATCH_IT;
			data+=m; n-=m; dex+=m;
		}
	}
	try {gh->flow(this,-2,(T *)0);} CATCH_IT;
	//!@#$ add error handling.
	dim = 0;
	dex = 0;
}

void GridInlet::from_grid(Grid *g) {
	if (!supports_type(g->nt)) RAISE("number type %s not supported here",number_type_table[g->nt].name);
#define FOO(T) from_grid2(g,(T)0);
	TYPESWITCH(g->nt,FOO,)
#undef FOO
}

/* **************** GridOutlet ************************************ */

GridOutlet::GridOutlet(FObject *parent_, int woutlet, P<Dim> dim_, NumberTypeE nt_) {
	parent=parent_; dim=dim_; nt=nt_; dex=0; bufi=0; buf=0; begin(woutlet,dim,nt);
}

void GridOutlet::create_buf () {
	int32 lcm_factor = 1;
	for (uint32 i=0; i<inlets.size(); i++) lcm_factor = lcm(lcm_factor,inlets[i]->factor());
	//size_t ntsz = number_type_table[nt].size;
	// biggest packet size divisible by lcm_factor
	int32 v = (MAX_PACKET_SIZE/lcm_factor)*lcm_factor;
	if (v==0) v=MAX_PACKET_SIZE; // factor too big. don't have a choice.
	buf=new Grid(new Dim(v),nt);
#ifdef TRACEBUFS
	std::ostringstream text;
	oprintf(text,"GridOutlet: %20s buf for sending to  ",buf->dim->to_s());
	for (uint i=0; i<inlets.size(); i++) text << " " << (void *)inlets[i]->parent;
	post("%s",text.str().data());
#endif
}

void GridOutlet::begin(int woutlet, P<Dim> dim, NumberTypeE nt) {
	this->nt = nt;
	this->dim = dim;
	t_atom a[1];
	SETGRIDOUT(a,this);
	outlet_anything(parent->bself->outlets[woutlet],bsym._grid,1,a);
	if (!dim->prod()) {finish(); return;}
}

// send modifies dex; send_direct doesn't
template <class T>
void GridOutlet::send_direct(long n, T *data) {
	CHECK_BUSY(outlet); CHECK_TYPE(*data); CHECK_ALIGN(data);
	while (n>0) {
		long pn = n;//min((long)n,MAX_PACKET_SIZE);
		for (uint32 i=0; i<inlets.size(); i++) try {inlets[i]->flow(pn,data);} CATCH_IT;
		data+=pn, n-=pn;
	}
}

void GridOutlet::flush() {
	if (!buf) return;
	if (!bufi) return;
#define FOO(T) send_direct(bufi,(T *)*buf);
	TYPESWITCH(buf->nt,FOO,)
#undef FOO
	bufi = 0;
}

template <class T, class S>
static void convert_number_type(int n, T *out, S *in) {for (int i=0; i<n; i++) out[i]=(T)in[i];}

//!@#$ buffering in outlet still is 8x faster...?
//!@#$ should use BitPacking for conversion...?
// send modifies dex; send_direct doesn't
template <class T>
void GridOutlet::send(long n, T *data) {
	//if (inlets.size()==1 && inlets[0]->buf) post("GridOutlet::send(%ld), bufsize %ld",long(n),long(inlets[0]->buf->dim->prod()));
	if (!n) return;
	CHECK_BUSY(outlet); CHECK_ALIGN(data);
	if (NumberTypeE_type_of(data)!=nt) {
		int bs = MAX_PACKET_SIZE;
#define FOO(T) {T data2[bs]; \
	for (;n>=bs;n-=bs,data+=bs) {convert_number_type(bs,data2,data); send(bs,data2);} \
	convert_number_type(n,data2,data); send(n,data2);}
		TYPESWITCH(nt,FOO,)
#undef FOO
	} else {
		dex += n;
		if (n > MIN_PACKET_SIZE || bufi + n > MAX_PACKET_SIZE) flush();
		if (n > MIN_PACKET_SIZE) {
			//post("send_direct %d",n);
			send_direct(n,data);
		} else {
			//post("send_indirect %d",n);
			if (!buf) create_buf();
			COPY((T *)*buf+bufi,data,n);
			bufi += n;
		}
		if (dex==dim->prod()) finish();
	}
}

void GridOutlet::callback(GridInlet *in) {
	CHECK_BUSY1(outlet);
	if (!(in->mode==4 || in->mode==0)) RAISE("mode error");
	inlets.push_back(in);
}

void GridOutlet::finish () {
	flush();
	for (uint32 i=0; i<inlets.size(); i++) inlets[i]->finish();
	dim=0;
}

// never call this. this is a hack to make some things work.
// i'm trying to circumvent either a bug in the compiler or i don't have a clue. :-(
void make_gimmick () {
	GridOutlet foo(0,0,0);
#define FOO(S) foo.send(0,(S *)0);
EACH_NUMBER_TYPE(FOO)
#undef FOO
	//foo.send(0,(float64 *)0); // this doesn't work, when trying to fix the new link problem in --lite mode.
}
