/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003,2004 by Mathieu Bouchard

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
#include "grid.h.fcs"
#include <ctype.h>

//#define TRACE fprintf(stderr,"%s %s [%s:%d]\n",parent->info(),__PRETTY_FUNCTION__,__FILE__,__LINE__);assert(this);
#define TRACE assert(this);

// maximum number of grid cords per outlet per cord type
#define MAX_CORDS 8

// number of (minimum,maximum) numbers to send at once
#define MIN_PACKET_SIZE (1<<9)
#define MAX_PACKET_SIZE (1<<11)

#define CHECK_TYPE(d) \
	if (NumberTypeE_type_of(d)!=this->nt) RAISE("%s(%s): " \
		"type mismatch during transmission (got %s expecting %s)", \
		parent->info(), \
		__PRETTY_FUNCTION__, \
		number_type_table[NumberTypeE_type_of(d)].name, \
		number_type_table[this->nt].name);

#define CHECK_BUSY(s) \
	if (!is_busy()) RAISE("%s: " #s " not busy",parent->info());

#define CHECK_ALIGN(d) \
	{int bytes = number_type_table[nt].size/8; \
	int align = ((long)(void*)d)%bytes; \
	if (align) {L;gfpost("%s(%s): Alignment Warning: %p is not %d-aligned: %d", \
		parent->info(), __PRETTY_FUNCTION__, (void*)d,bytes,align);}}

#define CHECK_ALIGN2(d,nt) \
	{int bytes = number_type_table[nt].size/8; \
	int align = ((long)(void*)d)%bytes; \
	if (align) {L;gfpost("Alignment Warning: %p is not %d-aligned: %d", \
		(void*)d,bytes,align);}}

// **************** Grid ******************************************

#define FOO(S) static inline void NUM(Ruby x, S &y) {y=convert(x,(int32*)0);}
EACH_INT_TYPE(FOO)
#undef FOO

#define FOO(S) \
static inline void NUM(Ruby x, S &y) { \
	if (TYPE(x)==T_FLOAT) y = RFLOAT(x)->value; \
	else if (INTEGER_P(x)) y = convert(x,(S*)0); \
	else RAISE("expected Float (or at least Integer)");}
EACH_FLOAT_TYPE(FOO)
#undef FOO

void Grid::init_from_ruby_list(int n, Ruby *a, NumberTypeE nt) {
	Ruby delim = SYM(#);
	for (int i=0; i<n; i++) {
		if (a[i] == delim) {
			STACK_ARRAY(int32,v,i);
			if (i!=0 && TYPE(a[i-1])==T_SYMBOL) nt=NumberTypeE_find(a[--i]);
			for (int j=0; j<i; j++) v[j] = convert(a[j],(int32*)0);
			init(new Dim(i,v),nt);
			CHECK_ALIGN2(this->data,nt);
			if (a[i] != delim) i++;
			i++; a+=i; n-=i;
			goto fill;
		}
	}
	if (n!=0 && TYPE(a[0])==T_SYMBOL) {
		nt = NumberTypeE_find(a[0]);
		a++, n--;
	}
	init(new Dim(n),nt);
	CHECK_ALIGN2(this->data,nt);
	fill:
	int nn = dim->prod();
	n = min(n,nn);
#define FOO(type) { \
	Pt<type> p = (Pt<type>)*this; \
	if (n==0) CLEAR(p,nn); \
	else { \
		for (int i=0; i<n; i++) NUM(a[i],p[i]); \
		for (int i=n; i<nn; i+=n) COPY(p+i,p,min(n,nn-i)); }}
	TYPESWITCH(nt,FOO,)
#undef FOO
}

void Grid::init_from_ruby(Ruby x) {
	if (TYPE(x)==T_ARRAY) {
		init_from_ruby_list(rb_ary_len(x),rb_ary_ptr(x));
	} else if (INTEGER_P(x) || FLOAT_P(x)) {
		init(new Dim(),int32_e);
		CHECK_ALIGN2(this->data,nt);
		((Pt<int32>)*this)[0] = INT(x);
	} else {
		rb_funcall(
		EVAL("proc{|x| raise \"can't convert to grid: #{x.inspect}\"}"),
		SI(call),1,x);
	}
}

// **************** GridInlet *************************************

// must be set before the end of GRID_BEGIN phase, and so cannot be changed
// afterwards. This is to allow some optimisations. Anyway there is no good reason
// why this would be changed afterwards.
void GridInlet::set_factor(int factor) {
	if(!dim) RAISE("huh?");
	if(factor<=0) RAISE("%s: factor=%d should be >= 1",parent->info(),factor);
	if (dim->prod() && dim->prod() % factor)
		RAISE("%s: set_factor: expecting divisor",parent->info());
	if (factor > 1) {
		buf=new Grid(new Dim(factor), nt);
		bufi=0;
	} else {
		buf=0;
	}
}

static Ruby GridInlet_begin_1(GridInlet *self) {
#define FOO(T) self->gh->flow(self,-1,Pt<T>()); break;
	TYPESWITCH(self->nt,FOO,)
#undef FOO
	return Qnil;
}

static Ruby GridInlet_begin_2(GridInlet *self) {
	self->dim = 0; // hack
	return (Ruby) 0;
}

bool GridInlet::supports_type(NumberTypeE nt) {
#define FOO(T) return !! gh->flow_##T;
	TYPESWITCH(nt,FOO,return false)
#undef FOO
}

Ruby GridInlet::begin(int argc, Ruby *argv) {TRACE;
	if (!argc) return PTR2FIX(this);
	GridOutlet *back_out = FIX2PTRAB(GridOutlet,argv[0],argv[1]);
	nt = (NumberTypeE) INT(argv[2]);
	argc-=3, argv+=3;
	PROF(parent) {
	if (is_busy()) {
		gfpost("%s: grid inlet conflict; aborting %s in favour of %s",
			parent->info(), sender->info(), back_out->parent->info());
		abort();
	}
	sender = back_out->parent;
	if ((int)nt<0 || (int)nt>=(int)number_type_table_end)
		RAISE("%s: inlet: unknown number type",parent->info());
	if (!supports_type(nt))
		RAISE("%s: number type %s not supported here",
			parent->info(), number_type_table[nt].name);
	STACK_ARRAY(int32,v,argc);
	for (int i=0; i<argc; i++) v[i] = NUM2INT(argv[i]);
	P<Dim> dim = this->dim = new Dim(argc,v);
	dex=0;
	buf=0;
	int r = rb_ensure(
		(RMethod)GridInlet_begin_1,(Ruby)this,
		(RMethod)GridInlet_begin_2,(Ruby)this);
	if (!r) {abort(); goto hell;}
	this->dim = dim;
	back_out->callback(this);
	hell:;} // PROF
	return Qnil;
}

template <class T>
void GridInlet::flow(int mode, int n, Pt<T> data) {TRACE;
	CHECK_BUSY(inlet);
	CHECK_TYPE(*data);
	CHECK_ALIGN(data);
	PROF(parent) {
	if (this->mode==0) {dex += n; return;} // ignore data
	if (n==0) return; // no data
	switch(mode) {
	case 4:{
		int d = dex + bufi;
		if (d+n > dim->prod()) {
			gfpost("grid input overflow: %d of %d from [%s] to [%s]",
				d+n, dim->prod(), sender->info(), 0);
			n = dim->prod() - d;
			if (n<=0) return;
		}
		int bufn = factor();
		if (buf && bufi) {
			Pt<T> bufd = (Pt<T>)*buf;
			int k = min(n,bufn-bufi);
			COPY(bufd+bufi,data,k);
			bufi+=k; data+=k; n-=k;
			if (bufi==bufn) {
				int newdex = dex+bufn;
				if (this->mode==6) {
					Pt<T> data2 = ARRAY_NEW(T,bufn);
					COPY(data2,bufd,bufn);
					CHECK_ALIGN(data2);
					gh->flow(this,bufn,data2);
				} else {
					CHECK_ALIGN(bufd);
					gh->flow(this,bufn,bufd);
				}
				dex = newdex;
				bufi = 0;
			}
		}
		int m = (n/bufn)*bufn;
		if (m) {
			int newdex = dex + m;
			if (this->mode==6) {
				Pt<T> data2 = ARRAY_NEW(T,m);
				COPY(data2,data,m);
				CHECK_ALIGN(data2);
				gh->flow(this,m,data2);
			} else {
				gh->flow(this,m,data);
			}
			dex = newdex;
		}
		data += m;
		n -= m;
		if (buf && n>0) COPY((Pt<T>)*buf+bufi,data,n), bufi+=n;
	}break;
	case 6:{
		assert(!buf);
		int newdex = dex + n;
		gh->flow(this,n,data);
		if (this->mode==4) delete[] (T *)data;
		dex = newdex;
	}break;
	case 0: break; // ignore data
	default: RAISE("%s: unknown inlet mode",parent->info());
	}} // PROF
}

void GridInlet::end() {TRACE;
	assert(this);
	if (!is_busy()) RAISE("%s: inlet not busy",parent->info());
	if (dim->prod() != dex) {
		gfpost("incomplete grid: %d of %d from [%s] to [%s]",
			dex, dim->prod(), sender->info(), parent->info());
	}
	PROF(parent) {
#define FOO(T) gh->flow(this,-2,Pt<T>());
	TYPESWITCH(nt,FOO,)
#undef FOO
	} // PROF
	dim=0;
	buf=0;
	dex=0;
}

template <class T> void GridInlet::from_grid2(Grid *g, T foo) {TRACE;
	nt = g->nt;
	dim = g->dim;
	int n = g->dim->prod();
	gh->flow(this,-1,Pt<T>());
	if (n>0 && this->mode!=0) {
		Pt<T> data = (Pt<T>)*g;
		CHECK_ALIGN(data);
		int size = g->dim->prod();
		if (this->mode==6) {
			Pt<T> d = data;
			data = ARRAY_NEW(T,size);  // problem with int64,float64 here.
			COPY(data,d,size);
			CHECK_ALIGN(data);
			gh->flow(this,n,data);
		} else {
			int m = MAX_PACKET_SIZE/factor();
			if (!m) m++;
			m *= factor();
			while (n) {
				if (m>n) m=n;
				CHECK_ALIGN(data);
				gh->flow(this,m,data);
				data+=m; n-=m; dex+=m;
			}			
		}
	}
	gh->flow(this,-2,Pt<T>());
	//!@#$ add error handling.
	// rescue; abort(); end ???
	dim = 0;
	dex = 0;
}

void GridInlet::from_grid(Grid *g) {TRACE;
	if (!supports_type(g->nt))
		RAISE("%s: number type %s not supported here",
			parent->info(), number_type_table[g->nt].name);
#define FOO(T) from_grid2(g,(T)0);
	TYPESWITCH(g->nt,FOO,)
#undef FOO
}

void GridInlet::from_ruby_list(int argc, Ruby *argv, NumberTypeE nt) {TRACE;
	Grid t(argc,argv,nt); from_grid(&t);
}

void GridInlet::from_ruby(int argc, Ruby *argv) {TRACE;
	Grid t(argv[0]); from_grid(&t);
}

/* **************** GridOutlet ************************************ */

GridOutlet::GridOutlet(GridObject *parent, int woutlet, P<Dim> dim, NumberTypeE nt) {TRACE;
	this->parent = parent;
	this->woutlet = woutlet;
	this->nt = int32_e;
	this->dim = 0;
	dex = 0;
	buf=new Grid(new Dim(MAX_PACKET_SIZE), nt);
	bufi = 0;
	frozen = 0;
	inlets = Pt<GridInlet *>();
	ninlets = 0;
	if (dim) begin(dim,nt);
}

GridOutlet::~GridOutlet() {TRACE; if (inlets) delete[] inlets.p;}

void GridOutlet::begin(P<Dim> dim, NumberTypeE nt) {TRACE;
	int n = dim->count();
	this->nt = nt;
	this->dim = dim;
	dex = 0;
	frozen = 0;
	ninlets = 0;
	if (inlets) delete[] inlets.p;
	inlets = ARRAY_NEW(GridInlet *,MAX_CORDS);
	Ruby a[n+5];
	a[0] = INT2NUM(woutlet);
	a[1] = bsym._grid;
	a[2] = PTR2FIXA(this);
	a[3] = PTR2FIXB(this);
	a[4] = INT2NUM(nt);
	for(int i=0; i<n; i++) a[5+i] = INT2NUM(dim->get(i));
	parent->send_out(COUNT(a),a);
	frozen = 1;
	if (!dim->prod()) {end(); return;}
	int32 lcm_factor = 1;
	for (int i=0; i<ninlets; i++) lcm_factor = lcm(lcm_factor,inlets[i]->factor());
	if (nt != buf->nt) {
		// biggest packet size divisible by lcm_factor
		int32 v = (MAX_PACKET_SIZE/lcm_factor)*lcm_factor;
		if (v==0) v=MAX_PACKET_SIZE; // factor too big. don't have a choice.
		buf=new Grid(new Dim(v),nt);
	}
}

// send modifies dex; send_direct doesn't
template <class T>
void GridOutlet::send_direct(int n, Pt<T> data) {TRACE;
	assert(data); assert(frozen);
	CHECK_BUSY(outlet); CHECK_TYPE(*data); CHECK_ALIGN(data);
	for (; n>0; ) {
		int pn = min(n,MAX_PACKET_SIZE);
		for (int i=0; i<ninlets; i++) inlets[i]->flow(4,pn,data);
		data+=pn, n-=pn;
	}
}

void GridOutlet::flush() {TRACE;
	if (!bufi) return;
#define FOO(T) send_direct(bufi,(Pt<T>)*buf);
	TYPESWITCH(buf->nt,FOO,)
#undef FOO
	bufi = 0;
}

template <class T, class S>
static void convert_number_type(int n, Pt<T> out, Pt<S> in) {
	for (int i=0; i<n; i++) out[i]=(T)in[i];
}

//!@#$ buffering in outlet still is 8x faster...?
//!@#$ should use BitPacking for conversion...?
// send modifies dex; send_direct doesn't
template <class T>
void GridOutlet::send(int n, Pt<T> data) {TRACE;
	assert(data); assert(frozen);
	if (!n) return;
	CHECK_BUSY(outlet); CHECK_ALIGN(data);
	if (NumberTypeE_type_of(*data)!=nt) {
		int bs = MAX_PACKET_SIZE;
#define FOO(T) { \
	STACK_ARRAY(T,data2,bs); \
	for (;n>=bs;n-=bs,data+=bs) { \
		convert_number_type(bs,data2,data); send(bs,data2);} \
	convert_number_type(n,data2,data); \
	send(n,data2); }
		TYPESWITCH(nt,FOO,)
#undef FOO
	} else {
		dex += n;
		assert(dex <= dim->prod());
		if (n > MIN_PACKET_SIZE || bufi + n > MAX_PACKET_SIZE) flush();
		if (n > MIN_PACKET_SIZE) {
			send_direct(n,data);
		} else {
			COPY((Pt<T>)*buf+bufi,data,n);
			bufi += n;
		}
		if (dex==dim->prod()) end();
	}
}

template <class T>
void GridOutlet::give(int n, Pt<T> data) {TRACE;
	assert(data); CHECK_BUSY(outlet); assert(frozen);
	assert(dex+n <= dim->prod()); CHECK_ALIGN(data);
	if (NumberTypeE_type_of(*data)!=nt) {
		send(n,data);
		delete[] (T *)data;
		return;
	}
	if (ninlets==1 && inlets[0]->mode == 6) {
		/* this is the copyless buffer passing */
		flush();
		inlets[0]->flow(6,n,data);
		dex += n;
	} else {
		flush();
		send_direct(n,data);
		dex += n;
		delete[] (T *)data;
	}
	if (dex==dim->prod()) end();
}

void GridOutlet::callback(GridInlet *in) {TRACE;
	CHECK_BUSY(outlet); assert(!frozen);
	int mode = in->mode;
	assert(mode==6 || mode==4 || mode==0);
	assert(ninlets<MAX_CORDS);
	inlets[ninlets++]=in;
}

/* **************** GridObject ************************************ */
/* abstract class for a FObject that has Grid-aware inlets/outlets */

GridObject::GridObject() {}
GridObject::~GridObject() {check_magic();}

\class GridObject < FObject

//!@#$ does not handle types properly
//!@#$ most possibly a big hack
template <class T>
void GridObject_r_flow(GridInlet *in, int n, Pt<T> data) {
	GridObject *self = in->parent;
	int i;
	for (i=0; i<MAX_INLETS; i++) if (in==self->in[i].p) break;
	if (i==MAX_INLETS) RAISE("inlet not found?");
	if (n==-1) {
		rb_funcall(self->rself,SI(send_in),2,INT2NUM(i),SYM(rgrid_begin));
	} else if (n>=0) {
		Ruby buf = rb_str_new((char *)((uint8 *)data),n*sizeof(T));
		rb_funcall(self->rself,SI(send_in),3,INT2NUM(i),SYM(rgrid_flow),buf);
	} else {
		rb_funcall(self->rself,SI(send_in),2,INT2NUM(i),SYM(rgrid_end));
	}
}

\def Symbol inlet_nt (int inln) {
	if (inln<0 || inln>=MAX_INLETS) RAISE("bad inlet number");
	P<GridInlet> inl = in[inln];
	if (!inl) RAISE("no such inlet #%d",inln);
	if (!inl->is_busy()) return Qnil;
	return number_type_table[inl->nt].sym;
}

\def Array inlet_dim (int inln) {
	if (inln<0 || inln>=MAX_INLETS) RAISE("bad inlet number");
	P<GridInlet> inl = in[inln];
	if (!inl) RAISE("no such inlet #%d",inln);
	if (!inl->is_busy()) return Qnil;
	int n=inl->dim->count();
	Ruby a = rb_ary_new2(n);
	for(int i=0; i<n; i++) rb_ary_push(a,INT2NUM(inl->dim->v[i]));
	return a;
}

\def void inlet_set_factor (int inln, int factor) {
	if (inln<0 || inln>=MAX_INLETS) RAISE("bad inlet number");
	P<GridInlet> inl = in[inln];
	if (!inl) RAISE("no such inlet #%d",inln);
	if (!inl->is_busy()) RAISE("inlet #%d not active",inln);
	inl->set_factor(factor);
}

\def void send_out_grid_begin (int outlet, Array buf, NumberTypeE nt=int32_e) {
	if (outlet<0) RAISE("bad outlet number");
	int n = rb_ary_len(buf);
	Ruby *p = rb_ary_ptr(buf);
	STACK_ARRAY(int32,v,n);
	for (int i=0; i<n; i++) v[i] = convert(p[i],(int32*)0);
	if (!out) out = new GridOutlet(this,outlet); // valgrind says leak?
	out->begin(new Dim(n,v),nt);
}

template <class T>
void send_out_grid_flow_2(P<GridOutlet> go, Ruby s, T bogus) {
	int n = rb_str_len(s) / sizeof(T);
	Pt<T> p = rb_str_pt(s,T);
	go->send(n,p);
}

\def void send_out_grid_flow (int outlet, String buf, NumberTypeE nt=int32_e) {
	if (outlet<0) RAISE("bad outlet number");
#define FOO(T) send_out_grid_flow_2(out,argv[1],(T)0);
	TYPESWITCH(nt,FOO,)
#undef FOO
}

static void *GridObject_allocate ();

// install_rgrid(Integer inlet, Boolean multi_type? = true)
static Ruby GridObject_s_install_rgrid(int argc, Ruby *argv, Ruby rself) {
	if (argc<1 || argc>2) RAISE("er...");
	GridHandler *gh = new GridHandler;
	gh->winlet = INT(argv[0]);
	bool mt = argc>1 ? argv[1]==Qtrue : 0; /* multi_type? */
	if (mt) {
#define FOO(S) gh->flow_##S = GridObject_r_flow;
EACH_NUMBER_TYPE(FOO)
#undef FOO
	} else {
#define FOO(S) gh->flow_##S = 0;
EACH_NUMBER_TYPE(FOO)
#undef FOO
	}
	gh->flow_int32 = GridObject_r_flow;
	Ruby handlers = rb_ary_new();
	rb_ary_push(handlers,PTR2FIX(gh));
	rb_ivar_set(rself,SI(@handlers),handlers);
	return Qnil;
}

static Ruby GridObject_s_instance_methods(int argc, Ruby *argv, Ruby rself) {
	static const char *names[] = {"grid","list","int","float"};
	Ruby list = rb_class_instance_methods(argc,argv,rself);
	Ruby handlers = rb_ivar_get(rself,SI(@handlers));
	if (handlers==Qnil) return list;
	for (int i=0; i<rb_ary_len(handlers); i++) {
		GridHandler *gh = FIX2PTR(GridHandler,rb_ary_ptr(handlers)[i]);
		char buf[256];
		int inl = gh->winlet;
		for (int j=0; j<COUNT(names); j++) {
			sprintf(buf,"_%d_%s",inl,names[j]);
			rb_ary_push(list,rb_str_new2(buf));
		}
	}
	return list;
}

\def Ruby method_missing (...) {
	if (argc<1) RAISE("not enough arguments");
	if (!SYMBOL_P(argv[0])) RAISE("expected symbol");
	const char *name = rb_sym_name(argv[0]);
	if (strlen(name)>3 && name[0]=='_' && name[2]=='_' && isdigit(name[1])) {
		int i = name[1]-'0';
		char foo[42];
		sprintf(foo,"_%d_grid",i);
		if (strcmp(name+3,"grid" )==0) goto hell;
		P<GridInlet> inl = FIX2PTR(GridInlet,rb_funcall(rself,rb_intern(foo),0));
		if (strcmp(name+3,"list" )==0) return inl->from_ruby_list(argc-1,argv+1), Qnil;
		if (strcmp(name+3,"int"  )==0) return inl->from_ruby     (argc-1,argv+1), Qnil;
		if (strcmp(name+3,"float")==0) return inl->from_ruby     (argc-1,argv+1), Qnil;
	}
	hell: return rb_call_super(argc,argv);
}

\classinfo {
	IEVAL(rself,"install 'GridObject',0,0");
	// define in Ruby-metaclass
	rb_define_singleton_method(rself,"instance_methods",(RMethod)GridObject_s_instance_methods,-1);
	rb_define_singleton_method(rself,"install_rgrid",(RMethod)GridObject_s_install_rgrid,-1);
	rb_enable_super(rb_singleton_class(rself),"instance_methods");
}
\end class GridObject

Ruby cGridObject;

void startup_grid () {
	fclass_install(&ciGridObject, cFObject);
	cGridObject = rb_const_get(mGridFlow,SI(GridObject));
}

// never call this. this is a hack to make some things work.
void make_gimmick () {
	GridOutlet foo(0,0);
#define FOO(S) foo.give(0,Pt<S>());
EACH_NUMBER_TYPE(FOO)
#undef FOO
}
