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

//#define INFO(foo) rb_str_ptr(rb_funcall(parent->peer,SI(args),0))
#define INFO(foo) "!@#$"

/* **************** Grid ****************************************** */

void Grid::init(Dim *dim, NumberTypeIndex nt) {
	this->nt = nt;
	this->dim = dim;
	this->data = dim ? NEW(char,dim->prod()*number_type_table[nt].size/8) : 0;
}

void Grid::init_from_ruby_list(int n, VALUE *a) {
		int dims = 1;
		VALUE delim = SYM(#);
		init(new Dim(0,0));
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
		Number *p = as_int32();
		for (int i=0; i<n; i++) p[i] = INT(a[i]);
		for (int i=n; i<nn; i+=n) COPY(p+i,p,min(n,nn-i));
}

void Grid::init_from_ruby(VALUE x) {
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

void Grid::del() {
	FREE(dim);
	FREE(data);
}

/* **************** GridInlet ************************************* */

GridInlet::GridInlet(GridObject *parent, const GridHandler *gh) {
	this->parent = parent;
	this->gh    = gh;
	assert(gh->begin && gh->flow && gh->end);
	dim   = 0;
	dex   = 0;
	factor= 1;
	buf   = 0;
	bufn  = 0;
}

GridInlet::~GridInlet() {
	FREE(dim);
	FREE(buf);
}

void GridInlet::abort() {
	if (dim) {
		if (dim->prod())
		gfpost("%s: aborting grid: %d of %d", INFO(this), dex, dim->prod());
		FREE(dim);
		FREE(buf);
	}
	dex = 0;
}

bool GridInlet::is_busy() {
	assert(this);
	return !!dim;
}

bool GridInlet::is_busy_verbose(const char *where) {
	assert(this);
	if (!dim) {
		gfpost("%s: (%s): no dim", INFO(this), where);
	} else if (!gh->flow) {
		gfpost("%s: (%s): no flow()", INFO(this), where);
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
	FREE(buf);
	if (factor > 1) {
		buf = NEW(Number,factor);
		bufn = 0;
	}
}

static VALUE GridInlet_begin$1(GridInlet *$) {
	$->gh->begin($->parent->peer,(GridObject *)$->parent,$);
	return Qnil;
}

static VALUE GridInlet_begin$2(GridInlet *$) {
	$->dim = 0; /* hack */
//	gfpost("ensure: %p,%p",$,$->dim);
	return (VALUE) 0;
}

void GridInlet::begin(int argc, VALUE *argv) {
	GridOutlet *back_out = (GridOutlet *) FIX2PTRAB(argv[0],argv[1]);
//	fprintf(stderr,"back_out=%p\n",back_out);
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
	int r = rb_ensure(
		(RFunc)GridInlet_begin$1,(VALUE)this,
		(RFunc)GridInlet_begin$2,(VALUE)this);
	if (!r) {abort(); return;}

	this->dim = dim;
	back_out->callback(this,gh->mode);
}

void GridInlet::flow(int argc, VALUE *argv) {
//	gfpost("inlet=%08p dex=%06d prod=%06d",this,dex,dim->prod());
	int n = NUM2INT(argv[0]);
	int mode = NUM2INT(argv[2]);
	if (n==0) return;
	if (mode==4) {
		const Number *data = (Number *) FIX2PTR(argv[1]);
		if (!is_busy_verbose("flow")) return;
		assert(n>0);
		int d = dex + bufn;
		if (d+n > dim->prod()) {
			gfpost("%s: grid input overflow: %d of %d",INFO($), d+n, dim->prod());
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
					Number *data2 = NEW2(Number,factor);
					COPY(data2,buf,factor);
					((GridFlow2)gh->flow)(parent->peer,parent,this,factor,data2);
				} else {
					gh->flow(parent->peer,parent,this,factor,buf);
				}
				dex = newdex;
				bufn = 0;
			}
		}
		int m = (n / factor) * factor;
		if (m) {
			int newdex = dex + m;
			if (gh->mode==6) {
				Number *data2 = NEW2(Number,m);
				COPY(data2,data,m);
				((GridFlow2)gh->flow)(parent->peer,parent,this,m,data2);
			} else {
				gh->flow(parent->peer,parent,this,m,data);
			}
			dex = newdex;
		}
		data += m;
		n -= m;
		if (buf) COPY(buf+bufn,data,n), bufn+=n;
	} else if (mode==6) {
		Number *data = (Number *)FIX2PTR(argv[1]);
		if (!is_busy_verbose("flow")) return;
		assert(n>0);
		assert(factor==1);
		int newdex = dex + n;
		if (gh->mode==6) {
			((GridFlow2)gh->flow)(parent->peer,parent,this,n,data);
		} else if (gh->mode==4) {
			gh->flow(parent->peer,parent,this,n,data);
			FREE(data);
		}
		dex = newdex;
	} else {
		assert(0);
	}
}

void GridInlet::end(int argc, VALUE *argv) {
	if (!is_busy_verbose("end")) return;
/*	gfpost("%s: GridInlet_end()", INFO(this)); */
	if (dim->prod() != dex) {
		gfpost("%s: incomplete grid: %d of %d", INFO(this),
			dex, dim->prod());
	}
	gh->end(parent->peer,parent,this);
	FREE(dim);
	dex = 0;
}

void GridInlet::list(int argc, VALUE *argv) {
	Grid t;
//	for (int i=0; i<argc; i++) gfpost("%08x.inlet_list: %02d: %08x\n",this,i,argv[i]);
	t.init_from_ruby_list(argc,argv);
	assert(gh);
	dim = t.dim->dup();
	int n = t.dim->prod();
	gh->begin(parent->peer,parent,this);
	if (n>0) gh->flow( parent->peer,parent,this,n,t.as_int32());
	gh->end(  parent->peer,parent,this);
	//!@#$ add error handling.
	/* rescue; GridInlet_abort($); */
	FREE(dim);
	dex = 0;
}

void GridInlet::int_(int argc, VALUE *argv) {
	Grid t;
	t.init_from_ruby(argv[0]);
	assert(gh);
	dim = t.dim->dup();
	gh->begin(parent->peer,parent,this);
	gh->flow( parent->peer,parent,this,t.dim->prod(),t.as_int32());
	gh->end(  parent->peer,parent,this);
	//!@#$ add error handling.
	/* rescue; GridInlet_abort($); */
	FREE(dim);
	dex = 0;
}

void GridInlet::float_(int argc, VALUE *argv) {int_(argc,argv);}

/* **************** GridOutlet ************************************ */

GridOutlet::GridOutlet(GridObject *parent, int woutlet) {
	this->parent = parent;
	this->woutlet = woutlet;
	dim = 0;
	dex = 0;
	buf = NEW2(Number,gf_max_packet_length);
	bufn = 0;
	frozen = 0;
	ron = 0; ro = 0;
	rwn = 0; rw = 0;
}

GridOutlet::~GridOutlet() {
	FREE(dim);
	FREE(buf);
}

bool GridOutlet::is_busy() {
	assert(this);
	return !!dim;
}

void GridOutlet::abort() {
	assert(this);
	if (!is_busy()) return;
	FREE(dim);
	dex = 0;
//	LEAVE_P;
	VALUE a[] = { INT2NUM(woutlet), sym_grid_end };
	FObject_send_out(COUNT(a),a,parent->peer);
//	ENTER_P;
}

void GridOutlet::end() {
	assert(this);
	assert(is_busy());
	flush();
//	LEAVE_P;
	VALUE a[] = { INT2NUM(woutlet), sym_grid_end };
	FObject_send_out(COUNT(a),a,parent->peer);
//	ENTER_P;
	FREE(dim);
	dim = 0;
	dex = 0;
}

void GridOutlet::begin(Dim *dim) {
	assert(this);

	int n = dim->count();
	dim = dim->dup(); /* leak */

	/* if (GridOutlet_busy($)) GridOutlet_abort($); */

//	fprintf(stderr,"this outlet = %p\n",this);

	this->dim = dim;
	dex = 0;
	frozen = 0;
	ron = 0; FREE(ro); ro = new GridInlet*[8];
	rwn = 0; FREE(rw); rw = new GridInlet*[8];
	VALUE a[n+4];
	a[0] = INT2NUM(woutlet);
	a[1] = sym_grid_begin;
	a[2] = PTR2FIXA(this);
	a[3] = PTR2FIXB(this);
	for(int i=0; i<n; i++) a[4+i] = INT2NUM(dim->get(i));
//	LEAVE_P;
	FObject_send_out(COUNT(a),a,parent->peer);
//	ENTER_P;
	frozen = 1;
/*	gfpost("$ = %p; $->ron = %d; $->rwn = %d", $, $->ron, $->rwn); */
}

void GridOutlet::send_direct(int n, const Number *data) {
	assert(is_busy());
	while (n>0) {
		int pn = min(n,gf_max_packet_length);
		VALUE a[] = { INT2NUM(pn), PTR2FIX(data), INT2NUM(4) };
//		LEAVE_P;
		for (int i=0; i<ron; i++) ro[i]->flow(COUNT(a),a);
		for (int i=0; i<rwn; i++) rw[i]->flow(COUNT(a),a);
//		ENTER_P;
		data += pn;
		n -= pn;
	}
}

void GridOutlet::send(int n, const Number *data) {
	assert(is_busy());
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
void GridOutlet::send8(int n, const uint8 *data) {
	int bs = gf_max_packet_length;
	Number data2[bs];
	for (;n>=bs;n-=bs) {
		for (int i=0; i<bs; i++) data2[i] = *data++;
		send(bs,data2);
	}
	for (int i=0; i<n; i++) data2[i] = *data++;
	send(n,data2);
}

void GridOutlet::give(int n, Number *data) {
	assert(is_busy());
	dex += n;
	assert(dex <= dim->prod());
	flush();
	if (ron == 0 && rwn == 1) {
		/* this is the copyless buffer passing */
		VALUE a[] = { INT2NUM(n), PTR2FIX(data), INT2NUM(6) };
//		LEAVE_P;
		rw[0]->flow(COUNT(a),a);
//		ENTER_P;
	} else {
		/* normal stuff */
		send_direct(n,data);
		FREE(data);
	}
}

void GridOutlet::flush() {
	assert(is_busy());
	send_direct(bufn,buf);
	bufn = 0;
}

void GridOutlet::callback(GridInlet *in, int mode) {
	assert(is_busy());
	assert(!frozen);
	assert(mode==6 || mode==4);
	assert(ron<8 && rwn<8);
	/* gfpost("callback: outlet=%p, inlet=%p, mode=%d",$,in,mode); */
	/* not using ->ro, ->rw yet */
	if (mode==4) ro[ron++]=in;
	if (mode==6) rw[rwn++]=in;
}

/* **************** GridObject ************************************ */
/*
  abstract class for an FTS Object that has Grid-aware inlets/outlets
*/

void GridObject::mark() {}

METHOD(GridObject,init) {
	int i;
	for (i=0; i<MAX_INLETS;  i++) $->in[i]  = 0;
	for (i=0; i<MAX_OUTLETS; i++) $->out[i] = 0;
	/* Dict_put(gf_object_set,$,0); */
	$->profiler_cumul = 0;

	GridClass *cl = $->grid_class;
	//gfpost("cl=%p\n",cl);
	for (i=0; i<cl->handlersn; i++) {
		GridHandler *gh = &cl->handlers[i];
		$->in[gh->winlet] = new GridInlet($,gh);
	}
	for (i=0; i<cl->outlets; i++) $->out[i] = new GridOutlet($,i);
//	for (i=0; i<MAX_INLETS; i++) gfpost("$=%p i=%d $->in[i]=%p",$,i,$->in[i]);
	rb_call_super(0,0);
}

/* category: input */

GRID_BEGIN_(GridObject,GridObject_r_begin) {
	rb_funcall(rself,SI(_0_rgrid_begin),0); // hack
}

GRID_FLOW_(GridObject,GridObject_r_flow) {
	VALUE buf = rb_str_new((char *)data,n*sizeof(Number));
	rb_funcall(rself,SI(_0_rgrid_flow),1,buf); // hack
}

GRID_END_(GridObject,GridObject_r_end) {
	rb_funcall(rself,SI(_0_rgrid_end),0); // hack
}

METHOD2(GridObject,inlet_dim) {
	GridInlet *in = $->in[INT(argv[0])];
	if (!in) RAISE("no such inlet #%d");
	if (!in->dim) return Qnil;
	int n=in->dim->count();
	VALUE a = rb_ary_new2(n);
//	gfpost("inlet_dim: %p,%p",in,in->dim);
	for(int i=0; i<n; i++) rb_ary_push(a,INT2NUM(in->dim->v[i]));
	return a;
}

METHOD(GridObject,send_out_grid_begin) {
	if (argc!=2 || TYPE(argv[1])!=T_ARRAY) RAISE("bad args");
	int outlet = INT(argv[0]);
	int n = rb_ary_len(argv[1]);
	int v[n];
	VALUE *p = rb_ary_ptr(argv[1]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
	for (int i=0; i<n; i++) v[i] = INT(p[i]);
	$->out[outlet]->begin(new Dim(n,v));
}

METHOD(GridObject,send_out_grid_flow) {
	if (argc!=2 || TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int outlet = INT(argv[0]);
	int n = rb_str_len(argv[1]) / sizeof(Number);
	Number *p = (Number *)rb_str_ptr(argv[1]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
	$->out[outlet]->send(n,p);
}

METHOD(GridObject,send_out_grid_end) {
	int outlet = INT(argv[0]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
	$->out[outlet]->end();
}

static VALUE GridObject_s_install_rgrid(int argc, VALUE *argv, VALUE rself) {
	GridHandler *gh = NEW(GridHandler,1);
	GridClass *gc = NEW(GridClass,1);
	if (argc!=1) RAISE("er...");
	if (INT(argv[0])!=0) RAISE("not yet");
	gh->winlet = INT(argv[0]);
	gh->begin = GridObject_r_begin;
	gh->flow = GridObject_r_flow;
	gh->end = GridObject_r_end;
	gh->mode = 4;
	gc->objectsize = 256; /* hack */
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

static VALUE GridObject_s_instance_methods(int argc, VALUE *argv, VALUE rself) {
	static const char *names[] = {"grid_begin","grid_end","list","int","float"};
//	VALUE list = rb_call_super(argc,argv);
	VALUE list = rb_class_instance_methods(argc,argv,rself);
	VALUE v = rb_ivar_get(rself,SI(@grid_class));
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

METHOD(GridObject,method_missing) {
	static const char *names[] = {"grid_begin","grid_end","list","int","float"};
//	gfpost("argc=%d,argv=%p,self=%p,rself=%p",argc,argv,self,rself);
	if (argc<1) RAISE("not enough arguments");
	if (!SYMBOL_P(argv[0])) RAISE("expected symbol");
//	rb_p(argv[0]);
	const char *name = rb_sym_name(argv[0]);
	//rb_funcall2(rb_cObject,SI(p),argc,argv);
	if (strlen(name)>3 && name[0]=='_' && name[2]=='_' && isdigit(name[1])) {
		int i = name[1]-'0';
		GridInlet *inl = $->in[i];
		//gfpost("$=%p; inl=%p",$,inl);
		int m;
		for (m=0; m<COUNT(names); m++)
			if (strcmp(name+3,names[m])==0) break;
		if (m==COUNT(names)) {
			rb_call_super(argc,argv);
			return;
		}
		if (!inl) RAISE("inlet #%d missing for object %s",i,$->args());
		argc--, argv++;
		switch(m) {
		case 0: return inl->begin( argc,argv);
		case 1: return inl->end(   argc,argv);
		case 2: return inl->list(  argc,argv);
		case 3: return inl->int_(  argc,argv);
		case 4: return inl->float_(argc,argv);
		}
	}
}

METHOD(GridObject,delete) {
	GridObject *parent=$;
	gfpost("%s: GridObject#delete",INFO(foo));
	for (int i=0; i<MAX_INLETS;  i++)
		if ($->in[i])  { delete $->in[i]; $->in[i]=0; }
	for (int i=0; i<MAX_OUTLETS; i++)
		if ($->out[i]) { delete $->out[i]; $->out[i]=0; }
	rb_call_super(argc,argv);
}

GRCLASS(GridObject,"GridObject",inlets:0,outlets:0,
LIST(),
	DECL(GridObject,init),
	DECL(GridObject,delete),
	DECL(GridObject,send_out_grid_begin),
	DECL(GridObject,send_out_grid_flow),
	DECL(GridObject,send_out_grid_end),
	DECL(GridObject,inlet_dim),
	DECL(GridObject,method_missing))

void GridObject_conf_class(VALUE $, GridClass *grclass) {
	/* define in Ruby-metaclass */
	rb_define_singleton_method($,"instance_methods",(RFunc)GridObject_s_instance_methods,-1);
	rb_define_singleton_method($,"install_rgrid",(RFunc)GridObject_s_install_rgrid,-1);
	rb_enable_super(rb_singleton_class($),"instance_methods");
}  

/* **************** Format **************************************** */
/* this is an abstract base class for file formats, network protocols, etc */

#ifdef FORMAT_LIST
extern GridClass FORMAT_LIST( ,_classinfo);
extern FormatInfo FORMAT_LIST( ,_formatinfo);
GridClass *format_classes[] = { FORMAT_LIST(&,_classinfo) };
FormatInfo *format_infos[] = { FORMAT_LIST(&,_formatinfo) };
#else
GridClass *format_classes[] = {};
FormatInfo *format_infos[] = {};
#endif

METHOD(Format,init) {
	int flags = INT(rb_ivar_get(CLASS_OF(rself),SI(@flags)));
	rb_call_super(argc,argv);
	$->mode = argv[0];
	$->parent = 0;
	$->bit_packing = 0;
	$->dim = 0;
	//gfpost("flags=%d",flags);
	/* FF_W, FF_R, FF_RW */
	if ($->mode==SYM(in)) {
		if ((flags & 4)==0) goto err;
	} else if ($->mode==SYM(out)) {
		if ((flags & 2)==0) goto err;
	} else {
		RAISE("Format opening mode is incorrect");
	}
	return;
err:
	RAISE("Format %s does not support mode '%s'",
		RSTRING(rb_ivar_get(rself,SI(@symbol_name)))->ptr, rb_sym_name($->mode));
}

METHOD(Format,option) {
	if (argc<1) RAISE("not enough arguments");
	RAISE("option %s not supported",rb_sym_name(argv[0]));
}

METHOD(Format,close) {
	FREE($->bit_packing);
	FREE($->dim);
	FREE($);
}

METHOD(Format,open_file) {
	if (argc<1) RAISE("expect filename");
	rb_ivar_set(rself,SI(@stream),
		rb_funcall(rb_eval_string("File"),SI(open),2,
			rb_funcall(GridFlow_module,SI(find_file),1,
			rb_any_to_s(argv[0])),
			rb_str_new2($->mode==4?"r":$->mode==2?"w":(RAISE("argh"),""))));
}

GRCLASS(Format,"Format",inlets:0,outlets:0,
LIST(),
	DECL(Format,init),
	DECL(Format,option),
	DECL(Format,open_file),
	DECL(Format,close))

/* **************************************************************** */

VALUE Format_class;
VALUE GridObject_class;

void startup_grid () {
	int i;
	ruby_c_install(&GridObject_classinfo, FObject_class);
	GridObject_class = rb_const_get(GridFlow_module,SI(GridObject));
	ruby_c_install(&Format_classinfo, GridObject_class);
	Format_class = rb_const_get(GridFlow_module,SI(Format));
	rb_ivar_set(GridFlow_module,SI(@formats),rb_hash_new());
	for (i=0; i<COUNT(format_classes); i++) {
		FormatInfo *fi = format_infos[i];
		GridClass *gc = format_classes[i];
		VALUE qlass;
		ruby_c_install(gc, Format_class);
		qlass = rb_const_get(GridFlow_module,rb_intern(gc->name));
		rb_ivar_set(qlass,SI(@flags),INT2NUM(fi->flags));
		rb_ivar_set(qlass,SI(@symbol_name),rb_str_new2(fi->symbol_name));
		rb_ivar_set(qlass,SI(@description),rb_str_new2(fi->description));
		rb_hash_aset(rb_ivar_get(GridFlow_module,SI(@formats)),
			ID2SYM(rb_intern(fi->symbol_name)), qlass);
	}
}
