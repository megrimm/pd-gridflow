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

#define INFO(foo) "!@#$"

/* **************** GridInlet ************************************* */

GridInlet::GridInlet(GridObject *parent, const GridHandler *gh) {
	this->parent = parent;
	this->gh    = gh;
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
		whine("%s: aborting grid: %d of %d", INFO(this), dex, dim->prod());
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
		whine("%s: (%s): no dim", INFO(this), where);
	} else if (!gh->flow) {
		whine("%s: (%s): no flow()", INFO(this), where);
	} else {
		return 1;
	}
	return 0;
}

void GridInlet::set_factor(int factor) {
	assert(factor > 0);
	assert(Dim_prod($->dim) % factor == 0);
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
//	whine("ensure: %p,%p",$,$->dim);
	return (VALUE) 0;
}

void GridInlet::begin(int argc, VALUE *argv) {
	int i;
	int *v = NEW(int,argc-1);
	GridOutlet *back_out = (GridOutlet *) FIX2PTR(argv[0]);
	Dim *dim;
	argc--, argv++;

	if (is_busy()) {
		whine("grid inlet busy (aborting previous grid)");
		abort();
	}

	if (argc-1>MAX_DIMENSIONS) {
		whine("too many dimensions (aborting grid)");
		goto err;
	}

	for (i=0; i<argc; i++) v[i] = NUM2INT(argv[i]);
	dim = this->dim = new Dim(argc,v);
	FREE(v);

	dex = 0;
	assert(gh->begin);
	{
		int v = rb_ensure(
			(RFunc)GridInlet_begin$1,(VALUE)this,
			(RFunc)GridInlet_begin$2,(VALUE)this);
		if (!v) goto err;
	}

//	whine("setting back dim...");
	this->dim = dim;
	((GridOutlet *)back_out)->callback(this,gh->mode);
	return;
err:
	abort();
}

void GridInlet::flow(int argc, VALUE *argv) {
	int n = NUM2INT(argv[0]);
	int mode = NUM2INT(argv[2]);

if (mode==4) {

	const Number *data = (Number *) FIX2PTR(argv[1]);
	if (!is_busy_verbose("flow")) return;
	assert(n>0);
	{
		int d = dex + bufn;
		if (d+n > dim->prod()) {
			whine("%s: grid input overflow: %d of %d",INFO($), d+n, dim->prod());
			n = dim->prod() - d;
			if (n<=0) return;
		}
		if (buf && bufn) {
			while (n && bufn<factor) { buf[bufn++] = *data++; n--; }
			if (bufn == factor) {
				int newdex = dex + factor;
				if (gh->mode==6 && gh->flow) {
					Number *data2 = NEW2(Number,factor);
					memcpy(data2,buf,factor*sizeof(Number));
					((GridFlow2)gh->flow)(parent->peer,(GridObject *)parent,this,factor,data2);
				} else {
					gh->flow(parent->peer,(GridObject *)parent,this,factor,buf);
				}
				dex = newdex;
				bufn = 0;
			}
		}
		{
			int m = (n / factor) * factor;
			int newdex = dex + m;
			if (m) {
				if (gh->mode==6 && gh->flow) {
					Number *data2 = NEW2(Number,m);
					memcpy(data2,data,m*sizeof(Number));
					((GridFlow2)gh->flow)(parent->peer,(GridObject *)parent,this,m,data2);
				} else {
					gh->flow(parent->peer,(GridObject *)parent,this,m,data);
				}
			}
			dex = newdex;
			data += m;
			n -= m;
		}
		if (buf) { while (n) { buf[bufn++] = *data++; n--; }}
	}
} else if (mode==6) {
	Number *data = (Number *)FIX2PTR(argv[1]);
	if (!is_busy_verbose("flow")) return;
	assert(n>0);
	assert(factor==1);
	assert(gh->flow);
	if (gh->mode==6) {
		((GridFlow2)gh->flow)(parent->peer,(GridObject *)parent,this,n,data);
	} else if (gh->mode==4) {
		gh->flow(parent->peer,(GridObject *)parent,this,n,data);
		FREE(data);
	}
} else {
	assert(0);
}}

void GridInlet::end(int argc, VALUE *argv) {
	if (!is_busy_verbose("end")) return;
/*	whine("%s: GridInlet_end()", INFO(this)); */
	if (dim->prod() != dex) {
		whine("%s: incomplete grid: %d of %d", INFO(this),
			dex, dim->prod());
	}
	if (gh->end) { gh->end(parent->peer,(GridObject *)parent,this); }
	FREE(dim);
	dex = 0;
}

void GridInlet::list(int argc, VALUE *argv) {
	int i;
	Number *v = NEW(Number,argc);
	int n = argc;
/*	whine("$=%p argc=%d",this,n); */
	for (i=0; i<argc; i++) {
		/*
		whine("argv[%d]=%ld",i,argv[i],
			RSTRING(rb_funcall(argv[i],SI(inspect),0))->ptr);
		*/
		v[i] = NUM2INT(argv[i]);
		/*whine("v[%d]=%ld",i,v[i]);*/
	}
	dim = new Dim(1,&n);

	assert(gh->begin);
	gh->begin(parent->peer,(GridObject *)parent,this);
	gh->flow(parent->peer,(GridObject *)parent,this,n,v);
	if (gh->end) { gh->end(parent->peer,(GridObject *)parent,this); }

	/* rescue; GridInlet_abort($); */

	FREE(v);
	FREE(dim);
	dex = 0;
}

/* **************** GridOutlet ************************************ */

GridOutlet::GridOutlet(GridObject *parent, int woutlet) {
	this->parent = parent;
	this->woutlet = woutlet;
	dim = 0;
	dex = 0;
	buf = NEW2(Number,gf_max_packet_length);
	bufn = 0;
	frozen = 0;
	ron = 0;
	ro = 0;
	rwn = 0;
	rw = 0;
}

GridOutlet::~GridOutlet() {
	FREE(dim);
	FREE(buf);
}

bool GridOutlet::is_busy() {
	assert(self);
	return !!dim;
}

void GridOutlet::abort() {
	assert(self);
	if (!is_busy()) return;
	FREE(dim);
	dex = 0;
//	LEAVE_P;
	{
		VALUE a[] = { INT2NUM(woutlet), sym_grid_end };
		FObject_send_out(COUNT(a),a,parent->peer);
	}
//	ENTER_P;
}

void GridOutlet::end() {
	assert(self);
	assert(is_busy());
	flush();
//	LEAVE_P;
	{
		VALUE a[] = { INT2NUM(woutlet), sym_grid_end };
		FObject_send_out(COUNT(a),a,parent->peer);
	}
//	ENTER_P;
	FREE(dim);
	dim = 0;
	dex = 0;
}

void GridOutlet::begin(Dim *dim) {
	int i;
	int n = dim->count();

	assert($);

	dim = dim->dup(); /* leak */

	/* if (GridOutlet_busy($)) GridOutlet_abort($); */

	this->dim = dim;
	dex = 0;
	frozen = 0;
	ron = 0; ro  = 0;
	rwn = 0; rw  = 0;
	{
		VALUE a[n+3];
		a[0] = INT2NUM(woutlet);
		a[1] = sym_grid_begin;
		a[2] = PTR2FIX(this);
		for(i=0; i<n; i++) a[3+i] = INT2NUM(dim->get(i));
//		LEAVE_P;
		FObject_send_out(n+3,a,parent->peer);
//		ENTER_P;
	}
	frozen = 1;
/*	whine("$ = %p; $->ron = %d; $->rwn = %d", $, $->ron, $->rwn); */
}

void GridOutlet::send_direct(int n, const Number *data) {
	assert(is_busy());
	while (n>0) {
		int pn = min(n,gf_max_packet_length);
		VALUE a[] = {
			INT2NUM(woutlet),
			sym_grid_flow,
			INT2NUM(pn),
			PTR2FIX(data), /* explicitly removing const */
			INT2NUM(4), /* mode ro */
		};
//		LEAVE_P;
		FObject_send_out(COUNT(a),a,parent->peer);
//		ENTER_P;
		data += pn;
		n -= pn;
	}
}

void GridOutlet::send(int n, const Number *data) {
	assert(GridOutlet_busy($));
	dex += n;
	assert($->dex <= dim->prod());
	if (n > gf_max_packet_length/2 || bufn + n > gf_max_packet_length) {
		flush();
	}
	if (n > gf_max_packet_length/2) {
		send_direct(n,data);
	} else {
		memcpy(&buf[bufn],data,n*sizeof(Number));
		bufn += n;
	}
}

void GridOutlet::give(int n, Number *data) {
	assert(GridOutlet_busy($));
	dex += n;
	assert(dex <= dim->prod());
	flush();
	if (ron == 0 && rwn == 1) {
		/* this is the copyless buffer passing */
		VALUE a[] = {
			INT2NUM(0),
			sym_grid_flow,
			INT2NUM(n),
			PTR2FIX(data),
			INT2NUM(6), /* mode rw */
		};
//		LEAVE_P;
		FObject_send_out(COUNT(a),a,parent->peer);
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
	/* whine("callback: outlet=%p, inlet=%p, mode=%d",$,in,mode); */
	/* not using ->ro, ->rw yet */
	if (mode==4) ron += 1;
	if (mode==6) rwn += 1;
}

/* **************** GridObject ************************************ */
/*
  abstract class for an FTS Object that has Grid-aware inlets/outlets
*/

METHOD(GridObject,init) {
	int i;
	for (i=0; i<MAX_INLETS;  i++) $->in[i]  = 0;
	for (i=0; i<MAX_OUTLETS; i++) $->out[i] = 0;
	/* Dict_put(gf_object_set,$,0); */
	$->profiler_cumul = 0;

	{
		GridClass *cl = $->grid_class;
		//whine("cl=%p\n",cl);
		for (i=0; i<cl->handlersn; i++) {
			GridHandler *gh = &cl->handlers[i];
			$->in[gh->winlet] = new GridInlet($,gh);
		}
		for (i=0; i<cl->outlets; i++) {
			$->out[i] = new GridOutlet($,i);
		}
	}

//	for (i=0; i<MAX_INLETS; i++) whine("$=%p i=%d $->in[i]=%p",$,i,$->in[i]);
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
{
	int i, n=in->dim->count();
	VALUE a = rb_ary_new2(n);
//	whine("inlet_dim: %p,%p",in,in->dim);
	for(i=0; i<n; i++) rb_ary_push(a,INT2NUM(in->dim->v[i]));
	return a;
}}

METHOD(GridObject,send_out_grid_begin) {
	if (argc!=2 || TYPE(argv[1])!=T_ARRAY) RAISE("bad args");
{
	int outlet = INT(argv[0]);
	int n = rb_ary_len(argv[1]);
	int i;
	int v[n];
	VALUE *p = rb_ary_ptr(argv[1]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
	for (i=0; i<n; i++) v[i] = INT(p[i]);
	$->out[outlet]->begin(new Dim(n,v));
}}

METHOD(GridObject,send_out_grid_flow) {
	if (argc!=2 || TYPE(argv[1])!=T_STRING) RAISE("bad args");
{
	int outlet = INT(argv[0]);
	int n = rb_str_len(argv[1]) / sizeof(Number);
	Number *p = (Number *)rb_str_ptr(argv[1]);
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet");
	$->out[outlet]->send(n,p);
}}

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
	int i;
//	VALUE list = rb_call_super(argc,argv);
	VALUE list = rb_class_instance_methods(argc,argv,rself);
	GridClass *grid_class = (GridClass *)FIX2PTR(rb_ivar_get(rself,SI(@grid_class)));
	for (i=0; i<grid_class->handlersn; i++) {
		GridHandler *gh = &grid_class->handlers[i];
		char buf[256];
		int inl = gh->winlet;
		sprintf(buf,"_%d_grid_begin",inl); rb_ary_push(list,rb_str_new2(buf));
		sprintf(buf,"_%d_grid_flow",inl);  rb_ary_push(list,rb_str_new2(buf));
		sprintf(buf,"_%d_grid_end",inl);   rb_ary_push(list,rb_str_new2(buf));
		sprintf(buf,"_%d_list",inl);       rb_ary_push(list,rb_str_new2(buf));
	}
	/* fprintf(stderr,"%s: instance_methods: %s\n",
		grid_class->name,
		RSTRING(rb_funcall(list,SI(inspect),0))->ptr); */
	return list;
}

METHOD(GridObject,method_missing) {
	char *name;
	//whine("argc=%d,argv=%p,self=%p",argc,argv,self);
	if (argc<1) RAISE("not enough arguments");
	if (!SYMBOL_P(argv[0])) RAISE("expected symbol");
	//rb_p(argv[0]);
	name = rb_sym_name(argv[0]);
	//rb_funcall2(rb_cObject,SI(p),argc,argv);
	if (strlen(name)>3 && name[0]=='_' && name[2]=='_' && isdigit(name[1])) {
		int i = name[1]-'0';
		GridInlet *inl = $->in[i];
		//whine("$=%p; inl=%p",$,inl);
		assert(inl);
		argc--, argv++;
		if      (strcmp(name+3,"grid_begin")==0) return inl->begin(argc,argv);
		else if (strcmp(name+3,"grid_flow" )==0) return inl->flow( argc,argv);
		else if (strcmp(name+3,"grid_end"  )==0) return inl->end(  argc,argv);
		else if (strcmp(name+3,"list"      )==0) return inl->list( argc,argv);
		argc++, argv--;
	}
	rb_call_super(argc,argv);
}

METHOD(GridObject,delete) {
	int i;
	for (i=0; i<MAX_INLETS;  i++) {
		if ($->in[i])  { delete $->in[i]; $->in[i]=0; }
	}
	for (i=0; i<MAX_OUTLETS; i++) {
		if ($->out[i]) { delete $->out[i]; $->out[i]=0; }
	}
/*
	Dict_del(gf_object_set,$);
	if (Dict_size(gf_object_set)==0) {
		whine("all objects freed");
		qdump();
	}
*/
}

GRCLASS(GridObject,inlets:0,outlets:0,
LIST(),
	DECL(GridObject,init),
	DECL(GridObject,delete),
	DECL(GridObject,send_out_grid_begin),
	DECL(GridObject,send_out_grid_flow),
	DECL(GridObject,send_out_grid_end),
	DECL(GridObject,inlet_dim),
	DECL(GridObject,method_missing))

void GridObject_conf_class(VALUE $, GridClass *grclass) {
	{
		MethodDecl methods[] = {
			DECL(GridObject,method_missing),
		};
		define_many_methods($,COUNT(methods),methods);
	}

	rb_enable_super($,"method_missing");
//	ruby_c_install(gc->name, gc->name, gc, Format_class2);

	/* define in Ruby-metaclass */
	rb_define_singleton_method($,"instance_methods",(RFunc)GridObject_s_instance_methods,-1);
	rb_define_singleton_method($,"install_rgrid",(RFunc)GridObject_s_install_rgrid,-1);
//	rb_define_singleton_method($,"install_format",(RFunc)GridObject_s_install_format,6);
	rb_enable_super(rb_singleton_class($),"instance_methods");
}  

/* **************** Format **************************************** */
/* this is an abstract base class for file formats, network protocols, etc */

#define FC(s) ((FormatClass *)s->grid_class)

extern GridClass FORMAT_LIST( ,_classinfo);
extern FormatInfo FORMAT_LIST( ,_formatinfo);
GridClass *format_classes[] = { FORMAT_LIST(&,_classinfo) };
FormatInfo *format_infos[] = { FORMAT_LIST(&,_formatinfo) };

METHOD(Format,init) {
	int flags = INT(rb_ivar_get(CLASS_OF(rself),SI(@flags)));

	rb_call_super(argc,argv);

	$->mode = argv[0];
	$->parent = 0;
	$->bit_packing = 0;
	$->dim = 0;

	whine("flags=%d",flags);
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

GRCLASS(Format,inlets:0,outlets:0,
LIST(),
	DECL(Format,init),
	DECL(Format,option),
	DECL(Format,open_file),
	DECL(Format,close))

/* **************************************************************** */

VALUE Format_class;
VALUE GridObject_class;

void startup_grid (void) {
	int i;
	ruby_c_install("GridObject","GridObject", &GridObject_classinfo, FObject_class);
	GridObject_class = rb_const_get(GridFlow_module,SI(GridObject));
	ruby_c_install("Format","Format", &Format_classinfo, GridObject_class);
	Format_class = rb_const_get(GridFlow_module,SI(Format));
	rb_ivar_set(GridFlow_module,SI(@formats),rb_hash_new());
	for (i=0; i<COUNT(format_classes); i++) {
		FormatInfo *fi = format_infos[i];
		GridClass *gc = format_classes[i];
		VALUE qlass;
		ruby_c_install(gc->name, gc->name, gc, Format_class);
		qlass = rb_const_get(GridFlow_module,rb_intern(gc->name));
		rb_ivar_set(qlass,SI(@flags),INT2NUM(fi->flags));
		rb_ivar_set(qlass,SI(@symbol_name),rb_str_new2(fi->symbol_name));
		rb_ivar_set(qlass,SI(@description),rb_str_new2(fi->description));
		rb_hash_aset(rb_ivar_get(GridFlow_module,SI(@formats)),
			ID2SYM(rb_intern(fi->symbol_name)), qlass);
	}
}
