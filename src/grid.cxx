/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

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
#include "gridflow.hxx.fcs"
#include <ctype.h>

//#define TRACEBUFS

#define CHECK_TYPE(d,NT) if (NumberTypeE_type_of(&d)!=NT) RAISE("(%s): " \
		"type mismatch during transmission (got %s expecting %s)", __PRETTY_FUNCTION__, \
		number_type_table[NumberTypeE_type_of(&d)].name, number_type_table[NT].name);
#define CHECK_BUSY1(s) if (!this->sender) RAISE(#s " not busy");
#define CHECK_BUSY(s)  if (!this->sender) RAISE(#s " not busy (wanting to write %ld values)",(long)n);
#define CHECK_ALIGN(d,nt) {int bytes = number_type_table[nt].size/8; int align = ((uintptr_t)(void*)d)%bytes; \
	if (align) {post("(%s): Alignment Warning: %p is not %d-aligned: %d", __PRETTY_FUNCTION__, (void*)d,bytes,align);}}

CONSTRAINT(expect_any) {}

// **************** Grid ******************************************

void Grid::init_from_list(int n, t_atom *aa, NumberTypeE nt) {
	t_atom2 *a = (t_atom2 *)aa;
	for (int i=0; i<n; i++) {
		if (a[i] == s_sharp) {
			int32 v[i];
			if (i!=0 && a[i-1].a_type==A_SYMBOL) nt=NumberTypeE_find(a[--i]);
			for (int j=0; j<i; j++) v[j] = (int32)a[j];
			init(Dim(i,v),nt);
			CHECK_ALIGN(data,nt);
			if (a[i] != s_sharp) i++;
			i++; a+=i; n-=i;
			goto fill;
		}
	}
	if (n!=0 && a[0].a_type==A_SYMBOL) {nt = NumberTypeE_find(a[0]); a++; n--;}
	init(Dim(n),nt);
	CHECK_ALIGN(data,nt);
	fill:
	int nn = dim.prod();
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
		init(Dim(),int32_e);
		CHECK_ALIGN(data,nt);
		((int32 *)*this)[0] = (int32)a.a_float;
	} else {
		ostringstream s; s << x;
		RAISE("can't convert to grid: %s",s.str().data());
	}
}

void Grid::init_from_msg(int argc, t_atom *argv, NumberTypeE nt) {
	if (argc==1) init_from_atom(argv[0]); // what do I do with nt here ? is nt being useful here anyway ?
	else         init_from_list(argc,argv,nt);
}

// **************** GridInlet *************************************

// must be set before the end of GRID_BEGIN phase, and so cannot be changed
// afterwards. This is to allow some optimisations. Anyway there is no good reason
// why this would be changed afterwards.
void GridInlet::set_chunk(long whichdim) {
	long n = dim.prod(whichdim);
	if (!n) n=1;
	if (n>1) {buf=new Grid(Dim(n), sender->nt); bufi=0;} else buf=0;
}

bool GridInlet::supports_type(NumberTypeE nt) {
#define FOO(T) return !! gh->flow_##T;
	TYPESWITCH(nt,FOO,return false)
#undef FOO
	return false; // avoid warning "may reach end of non-void function" with some versions of gcc
}

void GridInlet::begin(GridOut *sender) {
	if (this->sender) RAISE("grid inlet aborting from %s at %ld/%ld because of %s",
		ARGS(this->sender->parent),long(dex),long(dim.prod()),ARGS(sender->parent));
	this->sender = sender;
	if (!supports_type(sender->nt)) RAISE("number type %s not supported here", number_type_table[sender->nt].name);
	this->nt = sender->nt;
	this->dim = sender->dim;
	dex=0;
	buf=0;
	try {
#define FOO(T) gh->flow(*this,-1,(T *)0); break;
		TYPESWITCH(sender->nt,FOO,)
#undef FOO
	} catch (Barf &barf) {this->sender=0; throw;}
	this->dim = dim;
	sender->callback(*this);
#ifdef TRACEBUFS
	post("GridInlet:  %20s buf for recving from %p",dim->to_s(),sender);
#endif
}

#define CATCH_IT catch (Barf &slimy) {slimy.error(parent->bself,-1,s_grid);}

template <class T> void GridInlet::flow(long n, T *data) {
	CHECK_BUSY(inlet); CHECK_TYPE(*data,sender->nt); CHECK_ALIGN(data,sender->nt);
	if (!n) return; // no data
	long d = dex + bufi;
	if (d+n > dim.prod()) {
		post("grid input overflow: %ld of %ld from [%s] to [%s]", d+n, long(dim.prod()), ARGS(sender->parent), ARGS(parent));
		n = dim.prod() - d;
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
			CHECK_ALIGN(bufd,sender->nt);
			try {gh->flow(*this,bufn,bufd);} CATCH_IT;
			dex = newdex;
			bufi = 0;
		}
	}
	int m = (n/bufn)*bufn;
	if (m) {
		int newdex = dex + m;
		try {gh->flow(*this,m,data);} CATCH_IT;
		dex = newdex;
	}
	data += m;
	n -= m;
	if (buf && n>0) COPY((T *)*buf+bufi,data,n), bufi+=n;
}

void GridInlet::finish() {
	CHECK_BUSY1(inlet);
	if (dim.prod() != dex) post("%s: incomplete grid: %ld of %ld from [%s] to [%s]",
	    ARGS(parent),dex,long(dim.prod()),ARGS(sender->parent),ARGS(parent));
#define FOO(T) try {gh->flow(*this,-2,(T *)0);} CATCH_IT;
	TYPESWITCH(sender->nt,FOO,)
#undef FOO
	dim=0; buf=0; dex=0; sender=0;
}

template <class T> void GridInlet::from_grid2(Grid *g, T foo) {
	GridOut out(0,-1,g->dim,g->nt);
	begin(&out);
	size_t n = g->dim.prod();
	if (n) out.send(n,(T *)*g); else finish();
}

void GridInlet::from_grid(Grid *g) {
	if (!supports_type(g->nt)) RAISE("number type %s not supported here",number_type_table[g->nt].name);
#define FOO(T) from_grid2(g,(T)0);
	TYPESWITCH(g->nt,FOO,)
#undef FOO
}

/* **************** GridOut ************************************ */

GridOut::GridOut(FObject *parent_, int woutlet, const Dim &dim_, NumberTypeE nt_) {
	parent=parent_; dim=dim_; nt=nt_; dex=0; bufi=0; sender=this; fresh=true;
	if (parent) {t_atom2 a[1] = {this}; parent->out[woutlet](s_grid,1,a); if (!dim.prod()) finish();}
}

void GridOut::create_buf () {
	int32 lcm_factor = 1;
	for (uint32 i=0; i<inlets.size(); i++) lcm_factor = lcm(lcm_factor,inlets[i]->factor());
	if (!lcm_factor) {
		int32 lcm_factor = 1;
		for (uint32 i=0; i<inlets.size(); i++) {
			lcm_factor = lcm(lcm_factor,inlets[i]->factor());
			post("inlets[%d]->factor():%d, lcm_factor:%d",i,inlets[i]->factor(),lcm_factor);
		}
		RAISE("big trouble !");
	}
	//size_t ntsz = number_type_table[nt].size;
	// biggest packet size divisible by lcm_factor
	int32 v = (MAX_PACKET_SIZE/lcm_factor)*lcm_factor;
	if (v==0) v=MAX_PACKET_SIZE; // factor too big. don't have a choice.
	buf.init(Dim(v),nt);
#ifdef TRACEBUFS
	ostringstream text;
	oprintf(text,"GridOut: %20s buf for sending to  ",buf->dim->to_s());
	for (uint i=0; i<inlets.size(); i++) text << " " << (void *)inlets[i]->parent;
	post("%s",text.str().data());
#endif
	fresh=false;
}

// send modifies dex; send_direct doesn't
template <class T>
void GridOut::send_direct(long n, T *data) {
	CHECK_BUSY(outlet); CHECK_TYPE(*data,nt); CHECK_ALIGN(data,nt);
	while (n>0) {
		long pn = n;//min((long)n,MAX_PACKET_SIZE);
		for (uint32 i=0; i<inlets.size(); i++) try {inlets[i]->flow(pn,data);} CATCH_IT;
		data+=pn, n-=pn;
	}
}

void GridOut::flush() {
	if (fresh || !bufi) return;
#define FOO(T) send_direct(bufi,(T *)buf);
	TYPESWITCH(buf.nt,FOO,)
#undef FOO
	bufi = 0;
}

template <class T, class S>
static void convert_number_type(int n, T *out, S *in) {for (int i=0; i<n; i++) out[i]=(T)in[i];}

//!@#$ buffering in outlet still is 8x faster...?
// send modifies dex; send_direct doesn't
template <class T>
void GridOut::send_2(long n, T *data) {
	if (!n) return;
	CHECK_BUSY(outlet); CHECK_ALIGN(data,nt);
	if (NumberTypeE_type_of(data)!=nt) {
		int bs = MAX_PACKET_SIZE;
#define FOO(T) {T data2[bs]; \
	for (;n>=bs;n-=bs,data+=bs) {convert_number_type(bs,data2,data); send(bs,data2);} \
	convert_number_type(n,data2,data); send(n,data2);}
		TYPESWITCH(nt,FOO,)
#undef FOO
	} else {
		dex += n;
		if (n > MIN_PACKET_SIZE) {
			flush();
			//post("send_direct %d",n);
			send_direct(n,data);
		} else {
			//post("send_indirect %d",n);
			if (fresh) create_buf();
			int32 v = buf.dim.prod();
			if (bufi + n > v) flush();
			COPY((T *)buf+bufi,data,n);
			bufi += n;
		}
		if (dex==dim.prod()) finish();
	}
}

void GridOut::callback(GridInlet &in) {
	CHECK_BUSY1(outlet);
	inlets.push_back(&in);
}

void GridOut::finish () {
	flush();
	for (uint32 i=0; i<inlets.size(); i++) inlets[i]->finish();
	sender=0;
}

GridOut::~GridOut () {
	if (dex!=long(dim.prod())) post("~GridOut(): dex=%ld prod=%ld",dex,long(dim.prod()));
}

// never call this. this is a hack to make some things work.
// i'm trying to circumvent either a bug in the compiler or i don't have a clue. :-(
void make_gimmick () {
	GridOut foo(0,0,0);
#define FOO(S) foo.send(0,(S *)0);
EACH_NUMBER_TYPE(FOO)
#undef FOO
	//foo.send(0,(float64 *)0); // this doesn't work, when trying to fix the new link problem in --lite mode.
}
