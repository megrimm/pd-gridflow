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

/* number of (maximum,ideal) Numbers to send at once */
/* this should remain a constant throughout execution
   because code still expect it to be constant. */
static int max_packet_size = 1024*2;

/* result should be printed immediately as the GC may discard it anytime */
static const char *INFO(GridObject *foo) {
	Ruby z = rb_funcall(foo->rself,SI(args),0);
/*	if (TYPE(z)==T_ARRAY) z = rb_funcall(z,SI(inspect),0); */
	if (z==Qnil) return "(argh)";
	return rb_str_ptr(z);
}

/* **************** Grid ****************************************** */

void Grid::init(Dim *dim, NumberTypeIndex nt) {
	if (dc && dim) dc(dim);
	del();
	this->nt = nt;
	this->dim = dim;
	this->data = dim ? new char[dim->prod()*number_type_table[nt].size/8] : 0;
}

void Grid::init_from_ruby_list(int n, Ruby *a) {
		int dims = 1;
		Ruby delim = SYM(#);
		del();
		for (int i=0; i<n; i++) {
			if (a[i] == delim) {
				int32 v[i];
				for (int j=0; j<i; j++) v[j] = INT(a[j]);
				init(new Dim(i,v));
				i++; a+=i; n-=i;
				goto fill;
			}
		}
		{int32 v[1]={n}; init(new Dim(1,v));}
		fill:
		int nn = dim->prod();
		n = min(n,nn);
		Pt<int32> p = (Pt<int32>)*this;
		for (int i=0; i<n; i++) p[i] = INT(a[i]);
		for (int i=n; i<nn; i+=n) COPY(p+i,p,min(n,nn-i));
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
	buf   = Pt<int32>();
	bufn  = 0;
	sender = 0;
}

GridInlet::~GridInlet() {
	delete dim;
	delete[] (Number *)buf;
}

bool GridInlet::is_busy() {
	assert(this);
	return !!dim;
}

void GridInlet::set_factor(int factor) {
	assert(dim);
	assert(factor > 0);
	if (dim->prod() && dim->prod() % factor)
		RAISE("set_factor: expecting divisor");
	this->factor = factor;
	if (buf) {delete[] (Number *)buf; buf=Pt<Number>();}
	if (factor > 1) {
		buf = ARRAY_NEW(Number,factor);
		bufn = 0;
	}
}

static Ruby GridInlet_begin_1(GridInlet *self) {
	self->gh->flow(self,-1,Pt<int32>());
	return Qnil;
}

static Ruby GridInlet_begin_2(GridInlet *self) {
	self->dim = 0; /* hack */
	return (Ruby) 0;
}

void GridInlet::begin(int argc, Ruby *argv) {
	GridOutlet *back_out = FIX2PTRAB(GridOutlet,argv[0],argv[1]);
	nt = (NumberTypeIndex) INT(argv[2]);
	sender = back_out->parent;
	argc-=3, argv+=3;

	if ((int)nt<0 || (int)nt>=(int)number_type_table_end)
		RAISE("unknown number type");

	if (is_busy()) {
		gfpost("grid inlet busy (aborting previous grid)");
		abort();
	}

	if (argc>MAX_DIMENSIONS) RAISE("too many dimensions (aborting grid)");

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

void GridInlet::flow(int mode, int n, Pt<Number> data) {
	if (!is_busy()) RAISE("inlet not busy");
	if (gh->mode==0) {
		dex += n;
		return; /* ignore data */
	}
	if (n==0) return;
	if (mode==4) {
		int d = dex + bufn;
		if (d+n > dim->prod()) {
			gfpost("grid input overflow: %d of %d from %s to %s",
				d+n, dim->prod(), INFO(sender), INFO(parent));
			n = dim->prod() - d;
			if (n<=0) return;
		}
		if (buf && bufn) {
			int k = min(n,factor-bufn);
			COPY(buf+bufn,data,k);
			bufn+=k; data+=k; n-=k;
			if (bufn == factor) {
				int newdex = dex + factor;
				if (gh->mode==6) {
					Pt<Number> data2 = ARRAY_NEW(Number,factor);
					COPY(data2,buf,factor);
					gh->flow(this,factor,data2);
				} else {
					gh->flow(this,factor,buf);
				}
				dex = newdex;
				bufn = 0;
			}
		}
		int m = (n / factor) * factor;
		if (m) {
			int newdex = dex + m;
			if (gh->mode==6) {
				Pt<Number> data2 = ARRAY_NEW(Number,m);
				COPY(data2,data,m);
				gh->flow(this,m,data2);
			} else {
				gh->flow(this,m,data);
			}
			dex = newdex;
		}
		data += m;
		n -= m;
		if (buf) COPY(buf+bufn,data,n), bufn+=n;
	} else if (mode==6) {
		assert(factor==1);
		int newdex = dex + n;
		gh->flow(this,n,data);
		if (gh->mode==4) delete[] (int32 *)data;
		dex = newdex;
	} else if (mode==0) {
		/* nothing happens */
	} else {
		assert(0);
	}
}

void GridInlet::abort() {
	if (dim) {
		if (dim->prod())
		gfpost("aborting grid: %d of %d from %s to %s",
			dex, dim->prod(), INFO(sender), INFO(parent));
		delete dim; dim=0;
	}
	if (buf) {delete[] (Number *)buf; buf=Pt<Number>();}
	dex = 0;
}

void GridInlet::end() {
	if (!is_busy()) RAISE("inlet not busy");
	if (dim->prod() != dex) {
		gfpost("incomplete grid: %d of %d from %s to %s",
			dex, dim->prod(), INFO(sender), INFO(parent));
	}
	gh->flow(this,-2,Pt<int32>());
	if (dim) {delete dim; dim=0;}
	if (buf) {delete[] (Number *)buf; buf=Pt<Number>();}
	dex = 0;
}

#define WATCH2(n,ar) { \
	char foo[16*1024], *p=foo; \
	p += sprintf(p,"%s: ",#ar); \
	for (int i=0; i<n; i++) p += sprintf(p,"%ld ",ar[i]); \
	gfpost("%s",foo); \
}

/* !@#$ something wrong with nt here */
void GridInlet::grid(Grid *g) {
	assert(gh);
	int n = g->dim->prod();
	dim = g->dim->dup();
	gh->flow(this,-1,Pt<int32>());
	if (n>0 && gh->mode!=0) {
		Pt<Number> data = (Pt<int32>)*g;
		if (gh->mode==6) {
			Pt<Number> d = data;
			int size = g->dim->prod()*number_type_table[g->nt].size/8;
			data = Pt<Number>((Number *)new char[size],g->dim->prod());
			memcpy(data,d,size);
		}
		gh->flow(this,n,data);
	}
	gh->flow(this,-2,Pt<int32>());
	//!@#$ add error handling.
	/* rescue; GridInlet_abort(self); */
	delete dim;
	dim = 0;
	dex = 0; /* why? */
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
	dim = 0;
	dex = 0;
	buf = ARRAY_NEW(Number,max_packet_size);
	bufn = 0;
	frozen = 0;
	inlets = Pt<GridInlet *>();
	ninlets = 0;
}

GridOutlet::~GridOutlet() {
	if (dim) delete dim;
	if (buf) delete[] (Number *)buf;
	if (inlets) delete[] inlets.p;
}

void GridOutlet::abort() {
	if (!is_busy()) RAISE("outlet not busy");
	for (int i=0; i<ninlets; i++) inlets[i]->abort();
	delete dim; dim = 0; dex = 0;
}

void GridOutlet::begin(Dim *dim) {
	int n = dim->count();
	/* if (is_busy()) abort(); */
	this->dim = dim;
	dex = 0;
	frozen = 0;
	ninlets = 0;
	if (inlets) delete[] inlets.p;
	inlets = ARRAY_NEW(GridInlet *,MAX_CORDS);
	Ruby a[n+5];
	a[0] = INT2NUM(woutlet);
	a[1] = sym_grid;
	a[2] = PTR2FIXA(this);
	a[3] = PTR2FIXB(this);
	a[4] = int32_type_i;
	for(int i=0; i<n; i++) a[5+i] = INT2NUM(dim->get(i));
	FObject_send_out(COUNT(a),a,parent->rself);
	frozen = 1;
}

void GridOutlet::send_direct(int n, Pt<Number> data) {
	if (!is_busy()) RAISE("outlet not busy");
	assert(frozen);
	while (n>0) {
		int pn = min(n,max_packet_size);
		for (int i=0; i<ninlets; i++) inlets[i]->flow(4,pn,data);
		data += pn;
		n -= pn;
	}
}

/* buffering in outlet still is 8x faster...? */
void GridOutlet::send(int n, Pt<Number> data) {
	assert(is_busy());
	assert(frozen);
	dex += n;
	assert(dex <= dim->prod());
	if (n > max_packet_size/2 || bufn + n > max_packet_size) flush();
	if (n > max_packet_size/2) {
		send_direct(n,data);
	} else {
		COPY(buf+bufn,data,n);
		bufn += n;
	}
	if (dex==dim->prod()) end();
}

/* should use BitPacking? */
void GridOutlet::send(int n, Pt<uint8> data) {
	int bs = max_packet_size;
	STACK_ARRAY(Number,data2,bs);
	for (;n>=bs;n-=bs) {
		for (int i=0; i<bs; i++) data2[i] = *data++;
		send(bs,data2);
	}
	for (int i=0; i<n; i++) data2[i] = *data++;
	send(n,data2);
}

void GridOutlet::give(int n, Pt<Number> data) {
	if (!is_busy()) RAISE("outlet not busy");
	assert(frozen);
	dex += n;
	assert(dex <= dim->prod());
	flush();
	if (ninlets==1 && inlets[0]->gh->mode == 6) {
		/* this is the copyless buffer passing */
		inlets[0]->flow(6,n,data);
	} else {
		/* normal stuff */
		send_direct(n,data);
		delete[] (Number *)data;
	}
	if (dex==dim->prod()) end();
}

void GridOutlet::callback(GridInlet *in) {
	int mode = in->gh->mode;
	if (!is_busy()) RAISE("outlet not busy");
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
	rb_call_super(0,0);
	return Qnil;
}

/* category: input */

// most possibly a big hack
void GridObject_r_flow(GridInlet *in, int n, Pt<Number>data) {
	GridObject *self = in->parent;
	if (n==-1) {
		rb_funcall(self->rself,SI(_0_rgrid_begin),0);
	} else if (n>=0) {
		Ruby buf = rb_str_new((char *)((uint8 *)data),n*sizeof(Number));
		rb_funcall(self->rself,SI(_0_rgrid_flow),1,buf); // hack
	} else {
		rb_funcall(self->rself,SI(_0_rgrid_end),0);
	}
}

METHOD3(GridObject,inlet_dim) {
	int inln = INT(argv[0]);
	if (inln<0 || inln>=MAX_INLETS) RAISE("bad inlet number");
	GridInlet *inl = in[inln];
	if (!inl) RAISE("no such inlet #%d");
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
	if (!inl) RAISE("no such inlet #%d");
	if (!inl->is_busy()) RAISE("inlet not active");
	inl->set_factor(INT(argv[1]));
	return Qnil;
}

METHOD3(GridObject,send_out_grid_begin) {
	if (argc!=2 || TYPE(argv[1])!=T_ARRAY) RAISE("bad args");
	int outlet = INT(argv[0]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
	int n = rb_ary_len(argv[1]);
	Ruby *p = rb_ary_ptr(argv[1]);
	int32 v[n];
	for (int i=0; i<n; i++) v[i] = INT(p[i]);
	out[outlet]->begin(new Dim(n,v));
	return Qnil;
}

METHOD3(GridObject,send_out_grid_flow) {
	if (argc!=2 || TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int outlet = INT(argv[0]);
	int n = rb_str_len(argv[1]) / sizeof(Number);
	Pt<Number> p = rb_str_pt(argv[1],Number);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
	out[outlet]->send(n,p);
	return Qnil;
}

METHOD3(GridObject,send_out_grid_abort) {
	int outlet = INT(argv[0]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
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
	gh->flow = GridObject_r_flow;
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

void gfmemcopy(uint8 *out, const uint8 *in, int n) {
	while (n>16) {
		((int32*)out)[0] = ((int32*)in)[0];
		((int32*)out)[1] = ((int32*)in)[1];
		((int32*)out)[2] = ((int32*)in)[2];
		((int32*)out)[3] = ((int32*)in)[3];
		in+=16; out+=16; n-=16;
	}
	while (n>4) { *(int32*)out = *(int32*)in; in+=4; out+=4; n-=4; }
	while (n) { *out = *in; in++; out++; n--; }
}
