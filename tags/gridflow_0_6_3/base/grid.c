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

int gf_max_packet_length = 1024*2;

#define INFO(foo) rb_str_ptr(rb_funcall(foo->peer,SI(args),0))

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
//		init(new Dim(0,0));
		del();
		for (int i=0; i<n; i++) {
			if (a[i] == delim) {
				int v[i];
				for (int j=0; j<i; j++) v[j] = INT(a[j]);
				init(new Dim(i,v));
				i++; a+=i; n-=i;
				goto fill;
			}
		}
		init(new Dim(1,&n));
		fill:
		int nn = dim->prod();
		n = min(n,nn);
		Pt<Number> p = as_int32();
		for (int i=0; i<n; i++) p[i] = INT(a[i]);
		for (int i=n; i<nn; i+=n) COPY(p+i,p,min(n,nn-i));
}

void Grid::init_from_ruby(Ruby x) {
	if (TYPE(x)==T_ARRAY) {
		init_from_ruby_list(rb_ary_len(x),rb_ary_ptr(x));
	} else if (INTEGER_P(x) || FLOAT_P(x)) {
		init(new Dim(0,0));
		as_int32()[0] = INT(x);
	} else {
		rb_funcall(
		EVAL("proc{|x| raise \"can't convert to grid: #{x.inspect}\"}"),
		SI(call),1,x);
	}
}

Dim *Grid::to_dim() {
	return new Dim(dim->prod(),(int *)(Number *)as_int32());
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
	assert(gh->begin && gh->flow && gh->end);
	dim   = 0;
	dex   = 0;
	factor= 1;
	buf   = Pt<Number>();
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

bool GridInlet::is_busy_verbose(const char *where) {
	assert(this);
	if (!dim) {
		gfpost("%s: (%s): no grid is being transferred (!)",
			INFO(parent), where);
	} else {
		return 1;
	}
	return 0;
}

void GridInlet::set_factor(int factor) {
	assert(dim);
	assert(factor > 0);
	assert(dim->prod() % factor == 0);
	this->factor = factor;
	if (buf) {delete[] (Number *)buf; buf=Pt<Number>();}
	if (factor > 1) {
		buf = Pt<Number>(new Number[factor],factor);
		bufn = 0;
	}
}

static Ruby GridInlet_begin$1(GridInlet *$) {
	$->gh->begin($->parent,$);
	return Qnil;
}

static Ruby GridInlet_begin$2(GridInlet *$) {
	$->dim = 0; /* hack */
	return (Ruby) 0;
}

void GridInlet::begin(int argc, Ruby *argv) {
	GridOutlet *back_out = (GridOutlet *) FIX2PTRAB(argv[0],argv[1]);
	sender = back_out->parent;
	argc-=2, argv+=2;

	if (is_busy()) {
		gfpost("grid inlet busy (aborting previous grid)");
		abort();
	}

	if (argc>MAX_DIMENSIONS) RAISE("too many dimensions (aborting grid)");

	int v[argc];
	for (int i=0; i<argc; i++) v[i] = NUM2INT(argv[i]);
	Dim *dim = this->dim = new Dim(argc,v);

	dex = 0;
	factor = 1;
	int r = rb_ensure(
		(RFunc)GridInlet_begin$1,(Ruby)this,
		(RFunc)GridInlet_begin$2,(Ruby)this);
	if (!r) {abort(); return;}

	this->dim = dim;
	back_out->callback(this,gh->mode);
}

void GridInlet::flow(int argc, Ruby *argv) {
	int n = NUM2INT(argv[0]);
	int mode = NUM2INT(argv[2]);
	if (n==0) return;
	Pt<Number> data = Pt<Number>((Number *)FIX2PTR(argv[1]),n);
	if (!is_busy_verbose("flow")) return;
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
					Pt<Number> data2 = Pt<Number>(new Number[factor],factor);
					COPY(data2,buf,factor);
					gh->flow(parent,this,factor,data2);
				} else {
					gh->flow(parent,this,factor,buf);
				}
				dex = newdex;
				bufn = 0;
			}
		}
		int m = (n / factor) * factor;
		if (m) {
			int newdex = dex + m;
			if (gh->mode==6) {
				Pt<Number> data2 = Pt<Number>(new Number[m],m);
				COPY(data2,data,m);
				gh->flow(parent,this,m,data2);
			} else {
				gh->flow(parent,this,m,data);
			}
			dex = newdex;
		}
		data += m;
		n -= m;
		if (buf) COPY(buf+bufn,data,n), bufn+=n;
	} else if (mode==6) {
		assert(factor==1);
		int newdex = dex + n;
		gh->flow(parent,this,n,data);
		if (gh->mode==4) delete[] (int32 *)data;
		dex = newdex;
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

void GridInlet::end(int argc, Ruby *argv) {
	if (!is_busy_verbose("end")) return;
/*	gfpost("%s: GridInlet_end()", INFO(parent)); */
	if (dim->prod() != dex) {
		gfpost("incomplete grid: %d of %d from %s to %s",
			dex, dim->prod(), INFO(sender), INFO(parent));
	}
	gh->end(parent,this);
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

void GridInlet::list(int argc, Ruby *argv) {
	Grid t;
	t.init_from_ruby_list(argc,argv);
	assert(gh);
	dim = t.dim->dup();
	int n = dim->prod();
	gh->begin(parent,this);
	if (n>0) {
		Pt<Number> data = t.as_int32();
		if (gh->mode==6) {
			Pt<Number> d = data;
			int size = t.dim->prod()*number_type_table[t.nt].size/8;
			data = Pt<Number>((Number *)new char[size],t.dim->prod());
			memcpy(data,d,size);
		}
		gh->flow(parent,this,n,data);
	}
	gh->end(parent,this);
	//!@#$ add error handling.
	/* rescue; GridInlet_abort($); */
	delete dim;
	dim = 0;
	dex = 0;
}

void GridInlet::int_(int argc, Ruby *argv) {
	Grid t;
	t.init_from_ruby(argv[0]);
	assert(gh);
	dim = t.dim->dup();
	gh->begin(parent,this);
	gh->flow( parent,this,t.dim->prod(),t.as_int32());
	gh->end(  parent,this);
	//!@#$ add error handling.
	/* rescue; GridInlet_abort($); */
	delete dim;
	dim = 0;
	dex = 0;
}

void GridInlet::float_(int argc, Ruby *argv) {int_(argc,argv);}

/* **************** GridOutlet ************************************ */

GridOutlet::GridOutlet(GridObject *parent, int woutlet) {
	this->parent = parent;
	this->woutlet = woutlet;
	dim = 0;
	dex = 0;
	buf = Pt<Number>(new Number[gf_max_packet_length],gf_max_packet_length);
	bufn = 0;
	frozen = 0;
	ron = 0; ro = 0;
	rwn = 0; rw = 0;
}

GridOutlet::~GridOutlet() {
	if (dim) delete dim;
	if (buf) delete[] (Number *)buf;
	if (ro) delete[] ro;
	if (rw) delete[] rw;
}

bool GridOutlet::is_busy() {
	assert(this);
	return !!dim;
}

void GridOutlet::abort() {
	assert(this);
	if (!is_busy()) return;
	delete dim;
	dim = 0;
	dex = 0;
	Ruby a[] = { INT2NUM(woutlet), sym_grid_end };
	FObject_send_out(COUNT(a),a,parent->peer);
}

void GridOutlet::end() {
	assert(this);
	assert(is_busy());
	flush();
	Ruby a[] = { INT2NUM(woutlet), sym_grid_end };
	FObject_send_out(COUNT(a),a,parent->peer);
	delete dim;
	dim = 0;
	dex = 0;
}

void GridOutlet::begin(Dim *dim) {
	assert(this);
	int n = dim->count();

	/* if (GridOutlet_busy($)) GridOutlet_abort($); */

	this->dim = dim;
	dex = 0;
	frozen = 0;
	ron = 0; if (ro) delete[] ro; ro = new GridInlet*[MAX_CORDS];
	rwn = 0; if (rw) delete[] rw; rw = new GridInlet*[MAX_CORDS];
	Ruby a[n+4];
	a[0] = INT2NUM(woutlet);
	a[1] = sym_grid_begin;
	a[2] = PTR2FIXA(this);
	a[3] = PTR2FIXB(this);
	for(int i=0; i<n; i++) a[4+i] = INT2NUM(dim->get(i));
	FObject_send_out(COUNT(a),a,parent->peer);
	frozen = 1;
/*	gfpost("$ = %p; $->ron = %d; $->rwn = %d", $, $->ron, $->rwn); */
}

void GridOutlet::send_direct(int n, Pt<Number> data) {
	assert(is_busy());
	assert(frozen);
	while (n>0) {
		int pn = min(n,gf_max_packet_length);
		Ruby a[] = { INT2NUM(pn), PTR2FIX(data), INT2NUM(4) };
		for (int i=0; i<ron; i++) ro[i]->flow(COUNT(a),a);
		for (int i=0; i<rwn; i++) rw[i]->flow(COUNT(a),a);
		data += pn;
		n -= pn;
	}
}

void GridOutlet::send(int n, Pt<Number> data) {
	assert(is_busy());
	assert(frozen);
	dex += n;
	assert(dex <= dim->prod());
	if (n > gf_max_packet_length/2 || bufn + n > gf_max_packet_length) {
		flush();
	}
	if (n > gf_max_packet_length/2) {
		send_direct(n,data);
	} else {
		COPY(buf+bufn,data,n);
		bufn += n;
	}
}

/* should use BitPacking */
void GridOutlet::send(int n, Pt<uint8> data) {
	int bs = gf_max_packet_length;
	STACK_ARRAY(Number,data2,bs);
	for (;n>=bs;n-=bs) {
		for (int i=0; i<bs; i++) data2[i] = *data++;
		send(bs,data2);
	}
	for (int i=0; i<n; i++) data2[i] = *data++;
	send(n,data2);
}

void GridOutlet::give(int n, Pt<Number> data) {
	assert(is_busy());
	assert(frozen);
	dex += n;
	assert(dex <= dim->prod());
	flush();
	if (ron == 0 && rwn == 1) {
		/* this is the copyless buffer passing */
		Ruby a[] = { INT2NUM(n), PTR2FIX(data), INT2NUM(6) };
		rw[0]->flow(COUNT(a),a);
	} else {
		/* normal stuff */
		send_direct(n,data);
		delete[] (Number *)data;
	}
}

void GridOutlet::flush() {
	assert(is_busy());
	assert(frozen);
	send_direct(bufn,buf);
	bufn = 0;
}

void GridOutlet::callback(GridInlet *in, int mode) {
	assert(is_busy());
	assert(!frozen);
	assert(mode==6 || mode==4);
	assert(ron<MAX_CORDS && rwn<MAX_CORDS);
	/* gfpost("callback: outlet=%p, inlet=%p, mode=%d",$,in,mode); */
	if (mode==4) ro[ron++]=in;
	if (mode==6) rw[rwn++]=in;
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

METHOD3(GridObject,init) {
	for (int i=0; i<grid_class->handlersn; i++) {
		GridHandler *gh = &grid_class->handlers[i];
		in[gh->winlet] = new GridInlet(this,gh);
	}
	for (int i=0; i<grid_class->outlets; i++) out[i] = new GridOutlet(this,i);
	rb_call_super(0,0);
	return Qnil;
}

/* category: input */

void GridObject_r_begin(GridObject *$, GridInlet *in) {
	rb_funcall($->peer,SI(_0_rgrid_begin),0); // hack
}

void GridObject_r_flow(GridObject *$, GridInlet *in, int n, Pt<Number>data) {
	Ruby buf = rb_str_new((char *)((uint8 *)data),n*sizeof(Number));
	rb_funcall($->peer,SI(_0_rgrid_flow),1,buf); // hack
}

void GridObject_r_end(GridObject *$, GridInlet *in) {
	rb_funcall($->peer,SI(_0_rgrid_end),0); // hack
}

METHOD3(GridObject,inlet_dim) {
	int inln = INT(argv[0]);
	if (inln<0 || inln>=MAX_INLETS) RAISE("bad inlet number");
	GridInlet *inl = in[inln];
	if (!inl) RAISE("no such inlet #%d");
	if (!inl->dim) return Qnil;
	int n=inl->dim->count();
	Ruby a = rb_ary_new2(n);
	for(int i=0; i<n; i++) rb_ary_push(a,INT2NUM(inl->dim->v[i]));
	return a;
}

METHOD3(GridObject,send_out_grid_begin) {
	if (argc!=2 || TYPE(argv[1])!=T_ARRAY) RAISE("bad args");
	int outlet = INT(argv[0]);
	int n = rb_ary_len(argv[1]);
	Ruby *p = rb_ary_ptr(argv[1]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
	int v[n];
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

METHOD3(GridObject,send_out_grid_end) {
	int outlet = INT(argv[0]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
	out[outlet]->end();
	return Qnil;
}

static void *GridObject_allocate ();

static Ruby GridObject_s_install_rgrid(int argc, Ruby *argv, Ruby rself) {
	GridHandler *gh = new GridHandler;
	GridClass *gc = new GridClass;
	if (argc!=1) RAISE("er...");
	if (INT(argv[0])!=0) RAISE("not yet");
	gh->winlet = INT(argv[0]);
	gh->begin = GridObject_r_begin;
	gh->flow = GridObject_r_flow;
	gh->end = GridObject_r_end;
	gh->mode = 4;
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
	static const char *names[] = {"grid_begin","grid_end","list","int","float"};
	Ruby list = rb_class_instance_methods(argc,argv,rself);
	Ruby v = rb_ivar_get(rself,SI(@grid_class));
	if (v==Qnil) return list;
	GridClass *grid_class = (GridClass *)FIX2PTR(v);
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
	static const char *names[] = {"grid_begin","grid_end","list","int","float"};
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
		case 1: return inl->end(    argc,argv), Qnil;
		case 2: return inl->list(   argc,argv), Qnil;
		case 3: return inl->int_(   argc,argv), Qnil;
		case 4: return inl->float_( argc,argv), Qnil;
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
	DECL(GridObject,init),
	DECL(GridObject,del),
	DECL(GridObject,send_out_grid_begin),
	DECL(GridObject,send_out_grid_flow),
	DECL(GridObject,send_out_grid_end),
	DECL(GridObject,inlet_dim),
	DECL(GridObject,method_missing))

void GridObject_conf_class(Ruby $, GridClass *grclass) {
	/* define in Ruby-metaclass */
	rb_define_singleton_method($,"instance_methods",(RFunc)GridObject_s_instance_methods,-1);
	rb_define_singleton_method($,"install_rgrid",(RFunc)GridObject_s_install_rgrid,-1);
	rb_enable_super(rb_singleton_class($),"instance_methods");
}  

/* **************** Format **************************************** */
/* this is an abstract base class for file formats, network protocols, etc */

#ifdef FORMAT_LIST
extern GridClass FORMAT_LIST( ,ci,);
GridClass *format_classes[] = { FORMAT_LIST(&,ci,) };
#else
GridClass *format_classes[] = {};
#endif

METHOD3(Format,init) {
	int flags = INT(rb_ivar_get(rb_obj_class(peer),SI(@flags)));
//	printf("self=%08x\n",(int)this);
	rb_call_super(argc,argv);
//	printf("self=%08x\n",(int)this);
//	fprintf(stderr,"index(mode)=%08x\n",(char*)(&mode)-(char*)this);
	mode = argv[0]; /* VG: Invalid write of size 4 */
	parent = 0;
	/* FF_W, FF_R, FF_RW */
	if (mode==SYM(in)) {
		if ((flags & 4)==0) goto err;
	} else if (mode==SYM(out)) {
		if ((flags & 2)==0) goto err;
	} else {
		RAISE("Format opening mode is incorrect");
	}
	return Qnil;
err:
	RAISE("Format %s does not support mode '%s'",
		RSTRING(rb_ivar_get(peer,SI(@symbol_name)))->ptr, rb_sym_name(mode));
}

METHOD3(Format,option) {
	if (argc<1) RAISE("not enough arguments");
	RAISE("option %s not supported",rb_sym_name(argv[0]));
	return Qnil;
}

METHOD3(Format,close) {
	delete this; // caution...
	return Qnil;
}

METHOD3(Format,open_file) {
	if (argc<1) RAISE("expect filename");
	rb_ivar_set(peer,SI(@stream),
		rb_funcall(EVAL("File"),SI(open),2,
			rb_funcall(mGridFlow,SI(find_file),1,
			rb_any_to_s(argv[0])),
			rb_str_new2(mode==4?"r":mode==2?"w":(RAISE("argh"),""))));
	return Qnil;
}

GRCLASS(Format,"Format",inlets:0,outlets:0,startup:0,
LIST(),
	DECL(Format,init),
	DECL(Format,option),
	DECL(Format,open_file),
	DECL(Format,close))

/* **************************************************************** */

Ruby cFormat;
Ruby cGridObject;

void startup_grid () {
	ruby_c_install(&ciGridObject, cFObject);
	cGridObject = rb_const_get(mGridFlow,SI(GridObject));
	ruby_c_install(&ciFormat, cGridObject);
	cFormat = rb_const_get(mGridFlow,SI(Format));
	rb_ivar_set(mGridFlow,SI(@formats),rb_hash_new());

	EVAL(
	"module GridFlow; def Format.conf_format(flags,symbol_name,description)"
	"@flags = flags;"
	"@symbol_name = symbol_name;"
	"@description = description;"
	"GridFlow.instance_eval{@formats}[symbol_name.intern]=self;"
	"end;end");

	for (int i=0; i<COUNT(format_classes); i++) {
		ruby_c_install(format_classes[i], cFormat);
	}
}