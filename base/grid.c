/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002 by Mathieu Bouchard

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
#include "grid.h"
#include <ctype.h>

/* maximum number of grid cords per outlet per cord type */
#define MAX_CORDS 8

/* number of (minimum,maximum) numbers to send at once */
#define MIN_PACKET_SIZE (1<<9)
#define MAX_PACKET_SIZE (1<<11)

/* result should be printed immediately as the GC may discard it anytime */
static const char *INFO(GridObject *foo) {
	if (!foo) return "(nil GridObject!?)";
	Ruby z = rb_funcall(foo->rself,SI(args),0);
/*	if (TYPE(z)==T_ARRAY) z = rb_funcall(z,SI(inspect),0); */
	if (z==Qnil) return "(nil args!?)";
	return rb_str_ptr(z);
}

#define CHECK_TYPE(d) \
	if (NumberTypeIndex_type_of(d)!=nt) \
		RAISE("%s(%s): " \
		"type mismatch during transmission (%s instead of %s)", \
			INFO(parent), \
			__PRETTY_FUNCTION__, \
			number_type_table[NumberTypeIndex_type_of(d)].name, \
			number_type_table[nt].name);

#define CHECK_BUSY(s) \
	if (!is_busy()) RAISE("%s: " #s " not busy",INFO(parent));


/* **************** Grid ****************************************** */

void Grid::init(Dim *dim, NumberTypeIndex nt) {
	if (dc && dim) dc(dim);
	del();
	this->nt = nt;
	this->dim = dim;
	this->data = dim ? new char[dim->prod()*number_type_table[nt].size/8] : 0;
}

void Grid::init_from_ruby_list(int n, Ruby *a) {
		NumberTypeIndex nt = int32_type_i;
		int dims = 1;
		Ruby delim = SYM(#);
		del();
		for (int i=0; i<n; i++) {
			if (a[i] == delim) {
				int32 v[i];
				if (i!=0 && TYPE(a[i-1])==T_SYMBOL) {
					i--;
					nt = NumberTypeIndex_find(a[i]);
				}
				for (int j=0; j<i; j++) v[j] = INT(a[j]);
				init(new Dim(i,v),nt);
				if (a[i] != delim) i++;
				i++; a+=i; n-=i;
				goto fill;
			}
		}
		{
			if (n!=0 && TYPE(a[0])==T_SYMBOL) {
				nt = NumberTypeIndex_find(a[0]);
				a++;
				n--;
			}
			int32 v[1]={n};
			init(new Dim(1,v),nt);
		}
		fill:
		int nn = dim->prod();
		n = min(n,nn);
#define FOO(type) { \
		Pt<type> p = (Pt<type>)*this; \
		for (int i=0; i<n; i++) p[i] = INT(a[i]); \
		for (int i=n; i<nn; i+=n) COPY(p+i,p,min(n,nn-i)); }
		switch (nt) {
		case uint8_type_i: FOO(uint8); break;
		case int16_type_i: FOO(int16); break;
		case int32_type_i: FOO(int32); break;
		case float32_type_i: FOO(float32); break;
		default: RAISE("argh");
		}
#undef FOO		
}

void Grid::init_from_ruby(Ruby x) {
	if (TYPE(x)==T_ARRAY) {
		init_from_ruby_list(rb_ary_len(x),rb_ary_ptr(x));
	} else if (INTEGER_P(x) || FLOAT_P(x)) {
		init(new Dim(0,0));
		((Pt<int32>)*this)[0] = INT(x);
	} else {
		rb_funcall(
		EVAL("proc{|x| raise \"can't convert to grid: #{x.inspect}\"}"),
		SI(call),1,x);
	}
}

Dim *Grid::to_dim() {
	return new Dim(dim->prod(),(int32 *)(Pt<int32>)*this);
}

void Grid::del() {
	if (dim) delete dim;
	if (data) delete[] (uint8 *)data;
	dim=0;
	data=0;
}

Grid::~Grid() {
	del();
}

/* **************** GridInlet ************************************* */

GridInlet::GridInlet(GridObject *parent, const GridHandler *gh) {
	this->parent = parent;
	this->gh    = gh;
	assert(gh->flow);
	dim   = 0;
	nt    = int32_type_i;
	dex   = 0;
	factor= 1;
	bufi  = 0;
	sender = 0;
}

GridInlet::~GridInlet() {
	delete dim;
}

bool GridInlet::is_busy() {
	assert(this);
	return !!dim;
}

void GridInlet::set_factor(int factor) {
	assert(dim);
	assert(factor > 0);
	if (dim->prod() && dim->prod() % factor)
		RAISE("%s: set_factor: expecting divisor",INFO(parent));
	this->factor = factor;
	if (factor > 1) {
		int32 v[] = {factor};
		buf.init(new Dim(1,v));
		bufi = 0;
	} else {
		buf.del();
	}
}

static Ruby GridInlet_begin_1(GridInlet *self) {
	LEAVE(self->sender);
	ENTER(self->parent);
	switch (self->nt) {
	case uint8_type_i: self->gh->flow(self,-1,Pt<uint8>()); break;
	case int16_type_i: self->gh->flow(self,-1,Pt<int16>()); break;
	case int32_type_i: self->gh->flow(self,-1,Pt<int32>()); break;
	case float32_type_i: self->gh->flow(self,-1,Pt<float32>()); break;
	default: RAISE("argh");
	}
	LEAVE(self->parent);
	ENTER(self->sender);
	return Qnil;
}

static Ruby GridInlet_begin_2(GridInlet *self) {
	self->dim = 0; /* hack */
	return (Ruby) 0;
}

bool GridInlet::supports_type(NumberTypeIndex nt) {
	switch (nt) {
	case uint8_type_i: return !! gh->flow_uint8;
	case int16_type_i: return !! gh->flow_int16;
	case int32_type_i: return !! gh->flow_int32;
	case float32_type_i: return !! gh->flow_float32;
	default: return false;
	}
}

void GridInlet::begin(int argc, Ruby *argv) {
	GridOutlet *back_out = FIX2PTRAB(GridOutlet,argv[0],argv[1]);
	nt = (NumberTypeIndex) INT(argv[2]);
	sender = back_out->parent;
	argc-=3, argv+=3;

	if (is_busy()) {
		gfpost("grid inlet busy (aborting previous grid)");
		abort();
	}

	if ((int)nt<0 || (int)nt>=(int)number_type_table_end)
		RAISE("%s: inlet: unknown number type",INFO(parent));

	if (!supports_type(nt))
		RAISE("%s: number type %s not supported here",
			INFO(parent), number_type_table[nt].name);

	if (argc>MAX_DIMENSIONS)
		RAISE("%s: too many dimensions (aborting grid)",INFO(parent));

	int32 v[argc];
	for (int i=0; i<argc; i++) v[i] = NUM2INT(argv[i]);
	Dim *dim = this->dim = new Dim(argc,v);

	dex = 0;
	factor = 1;
	int r = rb_ensure(
		(RMethod)GridInlet_begin_1,(Ruby)this,
		(RMethod)GridInlet_begin_2,(Ruby)this);
	if (!r) {abort(); return;}

	this->dim = dim;
	back_out->callback(this);
}

template <class T>
void GridInlet::flow(int mode, int n, Pt<T> data) {
	CHECK_BUSY(inlet);
	CHECK_TYPE(*data);
	LEAVE(sender);
	ENTER(parent);
	if (gh->mode==0) {
		dex += n;
		return; /* ignore data */
	}
	if (n==0) return;
	if (mode==4) {
		int d = dex + bufi;
		if (d+n > dim->prod()) {
			gfpost("grid input overflow: %d of %d from %s to %s",
				d+n, dim->prod(), INFO(sender), INFO(parent));
			n = dim->prod() - d;
			if (n<=0) return;
		}
		if (factor>1 && bufi) {
			int k = min(n,factor-bufi);
			COPY((T *)buf+bufi,data,k);
			bufi+=k; data+=k; n-=k;
			if (bufi == factor) {
				int newdex = dex + factor;
				if (gh->mode==6) {
					Pt<T> data2 = ARRAY_NEW(T,factor);
					COPY(data2,buf,factor);
					gh->flow(this,factor,data2);
				} else {
					gh->flow(this,factor,(Pt<T>)buf);
				}
				dex = newdex;
				bufi = 0;
			}
		}
		int m = (n / factor) * factor;
		if (m) {
			int newdex = dex + m;
			if (gh->mode==6) {
				Pt<T> data2 = ARRAY_NEW(T,m);
				COPY(data2,data,m);
				gh->flow(this,m,data2);
			} else {
				gh->flow(this,m,data);
			}
			dex = newdex;
		}
		data += m;
		n -= m;
		if (factor>1 && n>0) COPY((T *)buf+bufi,data,n), bufi+=n;
	} else if (mode==6) {
		assert(factor==1);
		int newdex = dex + n;
		gh->flow(this,n,data);
		if (gh->mode==4) delete[] (T *)data;
		dex = newdex;
	} else if (mode==0) {
		/* nothing happens */
	} else {
		RAISE("%s: unknown inlet mode",INFO(parent));
	}
	LEAVE(parent);
	ENTER(sender);
}

/* !@#$ this is not correct */
void GridInlet::abort() {
	if (dim) {
		if (dim->prod())
		gfpost("aborting grid: %d of %d from %s to %s",
			dex, dim->prod(), INFO(sender), INFO(parent));
		delete dim; dim=0;
	}
	buf.del();
	dex = 0;
}

void GridInlet::end() {
	if (!is_busy()) RAISE("%s: inlet not busy",INFO(parent));
	if (dim->prod() != dex) {
		gfpost("incomplete grid: %d of %d from %s to %s",
			dex, dim->prod(), INFO(sender), INFO(parent));
	}
	LEAVE(sender);
	ENTER(parent);
	switch (nt) {
	case uint8_type_i: gh->flow(this,-2,Pt<uint8>()); break;
	case int16_type_i: gh->flow(this,-2,Pt<int16>()); break;
	case int32_type_i: gh->flow(this,-2,Pt<int32>()); break;
	case float32_type_i: gh->flow(this,-2,Pt<float32>()); break;
	default: RAISE("argh");
	}
	LEAVE(parent);
	ENTER(sender);
	if (dim) {delete dim; dim=0;}
	buf.del();
	dex = 0;
}

#define WATCH2(n,ar) { \
	char foo[16*1024], *p=foo; \
	p += sprintf(p,"%s: ",#ar); \
	for (int i=0; i<n; i++) p += sprintf(p,"%ld ",ar[i]); \
	gfpost("%s",foo); \
}

template <class T>
void GridInlet::grid2(Grid *g, T foo) {
	assert(gh);
	nt = g->nt;
	dim = g->dim->dup();
	int n = g->dim->prod();
	gh->flow(this,-1,Pt<T>());
	if (n>0 && gh->mode!=0) {
		Pt<T> data = (Pt<T>)*g;
		int size = g->dim->prod();
		if (gh->mode==6) {
			Pt<T> d = data;
			data = ARRAY_NEW(T,size);
			COPY(data,d,size);
			gh->flow(this,n,data);
		} else {
			int m = MAX_PACKET_SIZE/factor;
			if (!m) m++;
			m *= factor;
			while (n) {
				if (m>n) m=n;
				gh->flow(this,m,data);
				data+=m; n-=m;
			}			
		}
	}
	gh->flow(this,-2,Pt<T>());
	//!@#$ add error handling.
	// rescue; abort(); end ???
	delete dim;
	dim = 0;
	dex = 0;

	/* hack */
/*
	GridObject
	GridOutlet *go = new GridOutlet(parent,0);
	go->begin(g->dim->dup(),g->nt);
	go->send(g->dim->prod(),(Pt<T>)*g);
*/
}

void GridInlet::grid(Grid *g) {
	if (!supports_type(g->nt))
		RAISE("%s: number type %s not supported here",
			INFO(parent), number_type_table[g->nt].name);

	switch (g->nt) {
	case uint8_type_i: grid2(g,(uint8)0); break;
	case int16_type_i: grid2(g,(int16)0); break;
	case int32_type_i: grid2(g,(int32)0); break;
	case float32_type_i: grid2(g,(float32)0); break;
	default: RAISE("argh");
	}
}

void GridInlet::list(int argc, Ruby *argv) {
	Grid t;
	t.init_from_ruby_list(argc,argv);
	grid(&t);
}

void GridInlet::int_(int argc, Ruby *argv) {
	Grid t;
	t.init_from_ruby(argv[0]);
	grid(&t);
}

void GridInlet::float_(int argc, Ruby *argv) {int_(argc,argv);}

/* **************** GridOutlet ************************************ */

GridOutlet::GridOutlet(GridObject *parent, int woutlet) {
	this->parent = parent;
	this->woutlet = woutlet;
	nt = int32_type_i;
	dim = 0;
	dex = 0;
	int32 v[] = {MAX_PACKET_SIZE};
	buf.init(new Dim(1,v), nt);
	bufi = 0;
	frozen = 0;
	inlets = Pt<GridInlet *>();
	ninlets = 0;
}

GridOutlet::~GridOutlet() {
	if (dim) delete dim;
	if (inlets) delete[] inlets.p;
}

void GridOutlet::abort() {
	if (!is_busy()) RAISE("%s: outlet not busy",INFO(parent));
	for (int i=0; i<ninlets; i++) inlets[i]->abort();
	delete dim; dim = 0; dex = 0;
}

void GridOutlet::begin(Dim *dim, NumberTypeIndex nt) {
	int n = dim->count();
	/* if (is_busy()) abort(); */
	this->nt = nt;
	this->dim = dim;
	dex = 0;
	frozen = 0;
	ninlets = 0;
	if (nt != buf.nt) {
		int32 v[] = {MAX_PACKET_SIZE};
		buf.init(new Dim(1,v), nt);
	}
	if (inlets) delete[] inlets.p;
	inlets = ARRAY_NEW(GridInlet *,MAX_CORDS);
	Ruby a[n+5];
	a[0] = INT2NUM(woutlet);
	a[1] = sym_grid;
	a[2] = PTR2FIXA(this);
	a[3] = PTR2FIXB(this);
	a[4] = INT2NUM(nt);
	for(int i=0; i<n; i++) a[5+i] = INT2NUM(dim->get(i));
	FObject_send_out(COUNT(a),a,parent->rself);
	frozen = 1;
}

template <class T>
void GridOutlet::send_direct(int n, Pt<T> data) {
	CHECK_BUSY(outlet); assert(frozen);
	CHECK_TYPE(*data);
	while (n>0) {
		int pn = min(n,MAX_PACKET_SIZE);
		for (int i=0; i<ninlets; i++) inlets[i]->flow(4,pn,data);
		data += pn;
		n -= pn;
	}
}

template <class T, class S>
static void convert_number_type(int n, Pt<T> out, Pt<S> in) {
	for (int i=0; i<n; i++) out[i]=(T)in[i];
}

#define SEND_IN_CHUNKS(type) { \
	STACK_ARRAY(type,data2,bs); \
	for (;n>=bs;n-=bs,data+=bs) { \
		convert_number_type(bs,data2,data); send(bs,data2);} \
	convert_number_type(n,data2,data); \
	send(n,data2); }
			
/* buffering in outlet still is 8x faster...? */
/* should use BitPacking for conversion...? */

template <class T>
void GridOutlet::send(int n, Pt<T> data) {
	CHECK_BUSY(outlet); assert(frozen);
	if (NumberTypeIndex_type_of(*data)!=nt) {
		int bs = MAX_PACKET_SIZE;
/*
		gfpost("HAVE TO CONVERT %s INTO %s",
			number_type_table[NumberTypeIndex_type_of(*data)].name,
			number_type_table[nt].name);
*/
		switch (nt) {
		case uint8_type_i:   SEND_IN_CHUNKS(uint8); break;
		case int16_type_i:   SEND_IN_CHUNKS(int16); break;
		case int32_type_i:   SEND_IN_CHUNKS(int32); break;
		case float32_type_i: SEND_IN_CHUNKS(float32); break;
		default: RAISE("huh?");	
		}
	} else {
		dex += n;
		assert(dex <= dim->prod());
		if (n > MIN_PACKET_SIZE || bufi + n > MAX_PACKET_SIZE) flush();
		if (n > MIN_PACKET_SIZE) {
			send_direct(n,data);
		} else {
			COPY((T *)buf+bufi,data,n);
			bufi += n;
		}
		if (dex==dim->prod()) end();
	}
}

template <class T>
void GridOutlet::give(int n, Pt<T> data) {
	CHECK_BUSY(outlet)
	assert(frozen);
	dex += n;
	assert(dex <= dim->prod());
	flush();
	if (NumberTypeIndex_type_of(*data)!=nt && ninlets==1 &&
	inlets[0]->gh->mode == 6) {
		/* this is the copyless buffer passing */
		inlets[0]->flow(6,n,data);
	} else {
		/* normal stuff */
		send_direct(n,data);
		delete[] (T *)data;
	}
	if (dex==dim->prod()) end();
}

void GridOutlet::callback(GridInlet *in) {
	CHECK_BUSY(outlet)
	int mode = in->gh->mode;
	assert(!frozen);
	assert(mode==6 || mode==4 || mode==0);
	assert(ninlets<MAX_CORDS);
	inlets[ninlets++]=in;
}

/* **************** GridObject ************************************ */
/*
  abstract class for an FTS Object that has Grid-aware inlets/outlets
*/

void GridObject::mark() {}

GridObject::GridObject() {
	freed = false;
	profiler_cumul = 0;
	profiler_last  = 0;
	for (int i=0; i<MAX_INLETS;  i++) in[i]  = 0;
	for (int i=0; i<MAX_OUTLETS; i++) out[i] = 0;
}

GridObject::~GridObject() {
//	fprintf(stderr,"GridObject::~GridObject: %08x\n",(int)this);
	if (freed) fprintf(stderr,"GridObject being deleted twice???\n");
	freed=true;
	for (int i=0; i<MAX_INLETS;  i++) {
		//fprintf(stderr,"GridObject::~GridObject: inlet #%d=%08x\n",i,(int)in[i]);
		if (in[i]) delete in[i], in[i]=0;
	}
	for (int i=0; i<MAX_OUTLETS; i++) {
		//fprintf(stderr,"GridObject::~GridObject: outlet #%d=%08x\n",i,(int)out[i]);
		if (out[i]) delete out[i], out[i]=0;
	}
}

METHOD3(GridObject,initialize) {
	if (!grid_class) RAISE("C++ grid_class is null in Ruby class %s",
		rb_str_ptr(rb_funcall(rself,SI(inspect),0)));
	for (int i=0; i<grid_class->handlersn; i++) {
		GridHandler *gh = &grid_class->handlers[i];
		in[gh->winlet] = new GridInlet(this,gh);
	}
	for (int i=0; i<grid_class->outlets; i++) out[i] = new GridOutlet(this,i);
	rb_call_super(argc,argv);
	return Qnil;
}

/* category: input */

//!@#$ does not handle types properly
//!@#$ most possibly a big hack
template <class T>
void GridObject_r_flow(GridInlet *in, int n, Pt<T> data) {
	GridObject *self = in->parent;
	if (n==-1) {
		rb_funcall(self->rself,SI(_0_rgrid_begin),0);
	} else if (n>=0) {
		Ruby buf = rb_str_new((char *)((uint8 *)data),n*sizeof(T));
		rb_funcall(self->rself,SI(_0_rgrid_flow),1,buf); // hack
	} else {
		rb_funcall(self->rself,SI(_0_rgrid_end),0);
	}
}

METHOD3(GridObject,inlet_dim) {
	int inln = INT(argv[0]);
	if (inln<0 || inln>=MAX_INLETS) RAISE("bad inlet number");
	GridInlet *inl = in[inln];
	if (!inl) RAISE("no such inlet #%d",inln);
	if (!inl->is_busy()) return Qnil;
	int n=inl->dim->count();
	Ruby a = rb_ary_new2(n);
	for(int i=0; i<n; i++) rb_ary_push(a,INT2NUM(inl->dim->v[i]));
	return a;
}

METHOD3(GridObject,inlet_set_factor) {
	int inln = INT(argv[0]);
	if (inln<0 || inln>=MAX_INLETS) RAISE("bad inlet number");
	GridInlet *inl = in[inln];
	if (!inl) RAISE("no such inlet #%d",inln);
	if (!inl->is_busy()) RAISE("inlet #%d not active",inln);
	inl->set_factor(INT(argv[1]));
	return Qnil;
}

METHOD3(GridObject,send_out_grid_begin) {
	if (argc<2 || argc>3 || TYPE(argv[1])!=T_ARRAY) RAISE("bad args");
	int outlet = INT(argv[0]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet number");
	int n = rb_ary_len(argv[1]);
	Ruby *p = rb_ary_ptr(argv[1]);
	NumberTypeIndex nt = argc<3 ? int32_type_i : NumberTypeIndex_find(argv[2]);
	int32 v[n];
	for (int i=0; i<n; i++) v[i] = INT(p[i]);
	out[outlet]->begin(new Dim(n,v),nt);
	return Qnil;
}

template <class T>
void send_out_grid_flow_2(GridOutlet *go, Ruby s, T bogus) {
	int n = rb_str_len(s) / sizeof(T);
	Pt<T> p = rb_str_pt(s,T);
	go->send(n,p);
}

METHOD3(GridObject,send_out_grid_flow) {
	if (argc<2 || argc>3 || TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int outlet = INT(argv[0]);
	NumberTypeIndex nt = argc<3 ? int32_type_i : NumberTypeIndex_find(argv[2]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet number");
	GridOutlet *go = out[outlet];
	switch (nt) {
	case uint8_type_i: send_out_grid_flow_2(go,argv[1],(uint8)0); break;
	case int16_type_i: send_out_grid_flow_2(go,argv[1],(int16)0); break;
	case int32_type_i: send_out_grid_flow_2(go,argv[1],(int32)0); break;
	case float32_type_i: send_out_grid_flow_2(go,argv[1],(float32)0); break;
	default: RAISE("wham");
	}
	return Qnil;
}

METHOD3(GridObject,send_out_grid_abort) {
	int outlet = INT(argv[0]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet number");
	out[outlet]->abort();
	return Qnil;
}

static void *GridObject_allocate ();

static Ruby GridObject_s_install_rgrid(int argc, Ruby *argv, Ruby rself) {
	GridHandler *gh = new GridHandler;
	GridClass *gc = new GridClass;
	if (argc!=1) RAISE("er...");
	if (INT(argv[0])!=0) RAISE("not yet");
	gh->winlet = INT(argv[0]);
	gh->mode = 4;
	gh->flow_uint8 = 0;
	gh->flow_int16 = 0;
	gh->flow_int32 = GridObject_r_flow;
	gh->flow_float32 = 0;
	gc->objectsize = sizeof(GridObject);
	gc->allocate = GridObject_allocate;
	gc->methodsn = 0;
	gc->methods = 0;
	gc->inlets = MAX_INLETS; /* hack */
	gc->outlets = MAX_OUTLETS; /* hack */
	gc->handlersn = 1; /* hack, crack */
	gc->handlers = gh;
	gc->name = "hello";
	rb_ivar_set(rself,SI(@grid_class),PTR2FIX(gc));
	return Qnil;
}

static Ruby GridObject_s_instance_methods(int argc, Ruby *argv, Ruby rself) {
	static const char *names[] = {"grid","list","int","float"};
	Ruby list = rb_class_instance_methods(argc,argv,rself);
	Ruby v = rb_ivar_get(rself,SI(@grid_class));
	if (v==Qnil) return list;
	GridClass *grid_class = FIX2PTR(GridClass,v);
	for (int i=0; i<grid_class->handlersn; i++) {
		GridHandler *gh = &grid_class->handlers[i];
		char buf[256];
		int inl = gh->winlet;
		for (int j=0; j<COUNT(names); j++) {
			sprintf(buf,"_%d_%s",inl,names[j]);
			rb_ary_push(list,rb_str_new2(buf));
		}
	}
	return list;
}

METHOD3(GridObject,method_missing) {
	static const char *names[] = {"grid","list","int","float"};
	if (argc<1) RAISE("not enough arguments");
	if (!SYMBOL_P(argv[0])) RAISE("expected symbol");
	const char *name = rb_sym_name(argv[0]);
	if (strlen(name)>3 && name[0]=='_' && name[2]=='_' && isdigit(name[1])) {
		int i = name[1]-'0';
		GridInlet *inl = in[i];
		int m;
		for (m=0; m<COUNT(names); m++)
			if (strcmp(name+3,names[m])==0) break;
		if (m==COUNT(names)) goto hell;
		if (!inl) RAISE("inlet #%d missing for object %s",i,args());
		argc--, argv++;
		switch(m) {
		case 0: return inl->begin(  argc,argv), Qnil;
		case 1: return inl->list(   argc,argv), Qnil;
		case 2: return inl->int_(   argc,argv), Qnil;
		case 3: return inl->float_( argc,argv), Qnil;
		}
		return Qnil;
	}
	hell: return rb_call_super(argc,argv);
}

/* moved the stuff to the other destructor */
METHOD3(GridObject,del) {
	rb_call_super(argc,argv);
	return Qnil;
}

GRCLASS(GridObject,"GridObject",inlets:0,outlets:0,startup:0,
LIST(),
	DECL(GridObject,initialize),
	DECL(GridObject,del),
	DECL(GridObject,send_out_grid_begin),
	DECL(GridObject,send_out_grid_flow),
	DECL(GridObject,send_out_grid_abort),
	DECL(GridObject,inlet_dim),
	DECL(GridObject,inlet_set_factor),
	DECL(GridObject,method_missing))

void GridObject_conf_class(Ruby rself, GridClass *grclass) {
	/* define in Ruby-metaclass */
	rb_define_singleton_method(rself,"instance_methods",(RMethod)GridObject_s_instance_methods,-1);
	rb_define_singleton_method(rself,"install_rgrid",(RMethod)GridObject_s_install_rgrid,-1);
	rb_enable_super(rb_singleton_class(rself),"instance_methods");
}  

/* **************** Format **************************************** */
/* this is an abstract base class for file formats, network protocols, etc */

#ifdef FORMAT_LIST
extern GridClass FORMAT_LIST( ,ci,);
GridClass *format_classes[] = { FORMAT_LIST(&,ci,) };
#else
GridClass *format_classes[] = {};
#endif

/* **************************************************************** */

Ruby cFormat;
Ruby cGridObject;

void startup_formats () {
	for (int i=0; i<COUNT(format_classes); i++) {
		ruby_c_install(format_classes[i], cFormat);
	}
}

void startup_grid () {
	ruby_c_install(&ciGridObject, cFObject);
	EVAL(
	"module GridFlow; "
	"class Format < GridObject; end; "
	"def Format.conf_format(flags,symbol_name,description)"
	"@flags = flags;"
	"@symbol_name = symbol_name;"
	"@description = description;"
	"GridFlow.instance_eval{@formats}[symbol_name.intern]=self;"
	"end;end");
	cGridObject = rb_const_get(mGridFlow,SI(GridObject));
	cFormat = rb_const_get(mGridFlow,SI(Format));
	rb_ivar_set(mGridFlow,SI(@formats),rb_hash_new());
}

/*
  do not call this.
  I don't understand how to use templates properly
  so this is a hack to make some things work.
*/
static void make_gimmick () {
	exit(1); /* i warned you. */
	GridOutlet foo(0,0);
	foo.give(0,Pt<uint8>());
	foo.give(0,Pt<int16>());
	foo.give(0,Pt<int32>());
	foo.give(0,Pt<float32>());
}
