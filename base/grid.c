/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

//#define TRACE fprintf(stderr,"%s [%s:%d]\n",__PRETTY_FUNCTION__,__FILE__,__LINE__);
#define TRACE

/* maximum number of grid cords per outlet per cord type */
#define MAX_CORDS 8

/* number of (minimum,maximum) numbers to send at once */
#define MIN_PACKET_SIZE (1<<9)
#define MAX_PACKET_SIZE (1<<11)

#define CHECK_TYPE(d) \
	if (NumberTypeE_type_of(d)!=this->nt) \
		RAISE("%s(%s): " \
		"type mismatch during transmission (got %s expecting %s)", \
			parent->info(), \
			__PRETTY_FUNCTION__, \
			number_type_table[NumberTypeE_type_of(d)].name, \
			number_type_table[this->nt].name);

#define CHECK_BUSY(s) \
	if (!is_busy()) RAISE("%s: " #s " not busy",parent->info());


/* **************** Grid ****************************************** */

void Grid::init(Dim *dim, NumberTypeE nt) {
	if (dc && dim) dc(dim);
	del();
	if (!dim) RAISE("hell");
	this->nt = nt;
	this->dim = dim;
	data = dim ? new char[bytes()] : 0;
}

void Grid::init_clear(Dim *dim, NumberTypeE nt) {
	init(dim,nt);
	int size = bytes();
	CLEAR(Pt<char>((char *)data,size),size);
}

#define FOO(S) \
	static inline void NUM(Ruby x, S &y) { y = INT(x); }
EACH_INT_TYPE(FOO)
#undef FOO

#define FOO(S) \
static inline void NUM(Ruby x, S &y) { \
	if (TYPE(x)==T_FLOAT) y = RFLOAT(x)->value; \
	else if (INTEGER_P(x)) y = INT(x); \
	else RAISE("expected Float (or at least Integer)");}
EACH_FLOAT_TYPE(FOO)
#undef FOO

void Grid::init_from_ruby_list(int n, Ruby *a) {
		NumberTypeE nt = int32_type_i;
		int dims = 1;
		Ruby delim = SYM(#);
		del();
		for (int i=0; i<n; i++) {
			if (a[i] == delim) {
				STACK_ARRAY(int32,v,i);
				if (i!=0 && TYPE(a[i-1])==T_SYMBOL) {
					i--;
					nt = NumberTypeE_find(a[i]);
				}
				for (int j=0; j<i; j++) v[j] = INT(a[j]);
				init(new Dim(i,v),nt);
				if (a[i] != delim) i++;
				i++; a+=i; n-=i;
				goto fill;
			}
		}
		if (n!=0 && TYPE(a[0])==T_SYMBOL) {
			nt = NumberTypeE_find(a[0]);
			a++, n--;
		}
		{int32 v[1]={n}; init(new Dim(1,Pt<int32>(v,1)),nt);}
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
		STACK_ARRAY(int32,foo,1);
		init(new Dim(0,foo),int32_type_i);
		((Pt<int32>)*this)[0] = INT(x);
	} else {
		rb_funcall(
		EVAL("proc{|x| raise \"can't convert to grid: #{x.inspect}\"}"),
		SI(call),1,x);
	}
}

Dim *Grid::to_dim() {
	return new Dim(dim->prod(),(Pt<int32>)*this);
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
	this->gh = gh;
	dim = 0;
	nt = int32_type_i;
	dex = 0;
	factor = 1;
	bufi = 0;
	sender = 0;
}

GridInlet::~GridInlet() {
	delete dim;
}

/* must be set before the end of GRID_BEGIN phase, and so cannot be changed
afterwards. This is to allow some optimisations. Anyway there is no good reason
why this would be changed afterwards. */
void GridInlet::set_factor(int factor) {
	assert(dim);
	assert(factor > 0);
	if (dim->prod() && dim->prod() % factor)
		RAISE("%s: set_factor: expecting divisor",parent->info());
	this->factor = factor;
	if (factor > 1) {
		int32 v[] = {factor};
		buf.init(new Dim(1,Pt<int32>(v,1)), nt);
		bufi = 0;
	} else {
		buf.del();
	}
}

static Ruby GridInlet_begin_1(GridInlet *self) {
#define FOO(T) self->gh->flow(self,-1,Pt<T>()); break;
	TYPESWITCH(self->nt,FOO,)
#undef FOO
	return Qnil;
}

static Ruby GridInlet_begin_2(GridInlet *self) {
	self->dim = 0; /* hack */
	return (Ruby) 0;
}

bool GridInlet::supports_type(NumberTypeE nt) {
#define FOO(T) return !! gh->flow_##T;
	TYPESWITCH(nt,FOO,return false)
#undef FOO
}

void GridInlet::begin(int argc, Ruby *argv) {
	GridOutlet *back_out = FIX2PTRAB(GridOutlet,argv[0],argv[1]);
	nt = (NumberTypeE) INT(argv[2]);
	sender = back_out->parent;
	argc-=3, argv+=3;

	PROF(parent) {

	if (is_busy()) {
		gfpost("grid inlet busy (aborting previous grid)");
		abort();
	}

	if ((int)nt<0 || (int)nt>=(int)number_type_table_end)
		RAISE("%s: inlet: unknown number type",parent->info());

	if (!supports_type(nt))
		RAISE("%s: number type %s not supported here",
			parent->info(), number_type_table[nt].name);

	if (argc>MAX_DIMENSIONS)
		RAISE("%s: too many dimensions (aborting grid)",parent->info());

	STACK_ARRAY(int32,v,argc);
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

	} /* PROF */
}

template <class T>
void GridInlet::flow(int mode, int n, Pt<T> data) {
	CHECK_BUSY(inlet);
	CHECK_TYPE(*data);
	PROF(parent) {
	if (gh->mode==0) {
		dex += n;
		return; /* ignore data */
	}
	if (n==0) return;
	switch(mode) {
	case 4:{
		int d = dex + bufi;
		if (d+n > dim->prod()) {
			gfpost("grid input overflow: %d of %d from %s to %s",
				d+n, dim->prod(), sender->info(), parent->info());
			n = dim->prod() - d;
			if (n<=0) return;
		}
		if (factor>1 && bufi) {
			int k = min(n,factor-bufi);
			COPY((Pt<T>)buf+bufi,data,k);
			bufi+=k; data+=k; n-=k;
			if (bufi == factor) {
				int newdex = dex + factor;
				if (gh->mode==6) {
					Pt<T> data2 = ARRAY_NEW(T,factor);
					COPY(data2,(Pt<T>)buf,factor);
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
		if (factor>1 && n>0) COPY((Pt<T>)buf+bufi,data,n), bufi+=n;
	}break;
	case 6:{
		assert(factor==1);
		int newdex = dex + n;
		gh->flow(this,n,data);
		if (gh->mode==4) delete[] (T *)data;
		dex = newdex;
	}break;
	case 0: break; /* nothing happens */
	default: 
		RAISE("%s: unknown inlet mode",parent->info());
	}} /* PROF */
}

/* !@#$ this is not correct */
void GridInlet::abort() {
	if (dim) {
		if (dim->prod())
		gfpost("aborting grid: %d of %d from %s to %s",
			dex, dim->prod(), sender->info(), parent->info());
		delete dim; dim=0;
	}
	buf.del();
	dex = 0;
}

void GridInlet::end() {
	if (!is_busy()) RAISE("%s: inlet not busy",parent->info());
	if (dim->prod() != dex) {
		gfpost("incomplete grid: %d of %d from %s to %s",
			dex, dim->prod(), sender->info(), parent->info());
	}
	PROF(parent) {
#define FOO(T) gh->flow(this,-2,Pt<T>());
	TYPESWITCH(nt,FOO,)
#undef FOO
	} /* PROF */
	if (dim) {delete dim; dim=0;}
	buf.del();
	dex = 0;
}

template <class T> void GridInlet::from_grid2(Grid *g, T foo) {
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
				data+=m; n-=m; dex+=m;
			}			
		}
	}
	gh->flow(this,-2,Pt<T>());
	//!@#$ add error handling.
	// rescue; abort(); end ???
	delete dim;
	dim = 0;
	dex = 0;
}

void GridInlet::from_grid(Grid *g) {
	if (!supports_type(g->nt))
		RAISE("%s: number type %s not supported here",
			parent->info(), number_type_table[g->nt].name);
#define FOO(T) from_grid2(g,(T)0);
	TYPESWITCH(g->nt,FOO,)
#undef FOO
}

void GridInlet::from_ruby_list(int argc, Ruby *argv) {
	Grid t;	t.init_from_ruby_list(argc,argv); from_grid(&t);
}

void GridInlet::from_ruby(int argc, Ruby *argv) {
	Grid t; t.init_from_ruby(argv[0]); from_grid(&t);
}

/* **************** GridOutlet ************************************ */

GridOutlet::GridOutlet(GridObject *parent, int woutlet) {
	this->parent = parent;
	this->woutlet = woutlet;
	nt = int32_type_i;
	dim = 0;
	dex = 0;
	int32 v[] = {MAX_PACKET_SIZE};
	buf.init(new Dim(1,Pt<int32>(v,1)), nt);
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
	if (!is_busy()) RAISE("%s: outlet not busy",parent->info());
	for (int i=0; i<ninlets; i++) inlets[i]->abort();
	delete dim; dim = 0; dex = 0;
}

void GridOutlet::begin(Dim *dim, NumberTypeE nt) {
	TRACE;
//	fprintf(stderr,"%s, %s\n", dim->to_s(), number_type_table[nt].name);
	int n = dim->count();
	/* if (is_busy()) abort(); */
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
	if (dex==dim->prod()) end(); /* zero-sized grid? */

	int lcm_factor = 1;
	for (int i=0; i<ninlets; i++) lcm_factor = lcm(lcm_factor,inlets[i]->factor);

	if (nt != buf.nt) {
		/* biggest packet size divisible by lcm_factor */
		int32 v[] = {(MAX_PACKET_SIZE/lcm_factor)*lcm_factor};
		if (*v==0) *v=MAX_PACKET_SIZE; /* factor too big. don't have a choice. */
		buf.init(new Dim(1,Pt<int32>(v,1)), nt);
	}

/*	
	char buf[256];
	sprintf(buf,"%s: lcm{ ", parent->info());
	for (int i=0; i<ninlets; i++) {
		sprintf(buf+strlen(buf),"%d ",inlets[i]->factor);
	}
	sprintf(buf+strlen(buf),"} = %d",lcm_factor);
	gfpost("%s",buf);
*/
}

template <class T>
void GridOutlet::send_direct(int n, Pt<T> data) {
	TRACE; CHECK_BUSY(outlet); assert(frozen);
	CHECK_TYPE(*data);
	for (; n>0; ) {
		int pn = min(n,MAX_PACKET_SIZE);
		for (int i=0; i<ninlets; i++) inlets[i]->flow(4,pn,data);
		data+=pn, n-=pn;
	}
}

void GridOutlet::flush() {
	TRACE;
	if (!bufi) return;
#define FOO(T) send_direct(bufi,(Pt<T>)buf);
	TYPESWITCH(buf.nt,FOO,)
#undef FOO
	bufi = 0;
}

template <class T, class S>
static void convert_number_type(int n, Pt<T> out, Pt<S> in) {
	for (int i=0; i<n; i++) out[i]=(T)in[i];
}

/* buffering in outlet still is 8x faster...? */
/* should use BitPacking for conversion...? */

template <class T>
void GridOutlet::send(int n, Pt<T> data) {
	if (!n) return;
/*
	fprintf(stderr,"GridOutlet::send: sending %d elements (dex=%d/%d)\n",
		n,dex,dim?dim->prod():0);
	if (!n) RAISE("a-ha");
*/
	TRACE; CHECK_BUSY(outlet); assert(frozen);
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
			COPY((Pt<T>)buf+bufi,data,n);
			bufi += n;
		}
		if (dex==dim->prod()) end();
	}
}

template <class T>
void GridOutlet::give(int n, Pt<T> data) {
	TRACE; CHECK_BUSY(outlet); assert(frozen);
	dex += n;
	assert(dex <= dim->prod());
	flush();
	if (ninlets==1 && inlets[0]->gh->mode == 6) {
		/* this is the copyless buffer passing */
		inlets[0]->flow(6,n,data);
	} else {
		if (NumberTypeE_type_of(*data)!=nt) {
			send(n,data);
		} else {
			flush();
			send_direct(n,data);
		}
		delete[] (T *)data;
	}
	if (dex==dim->prod()) end();
}

void GridOutlet::callback(GridInlet *in) {
	TRACE; CHECK_BUSY(outlet); assert(!frozen);
	int mode = in->gh->mode;
	assert(mode==6 || mode==4 || mode==0);
	assert(ninlets<MAX_CORDS);
	inlets[ninlets++]=in;
}

/* **************** GridObject ************************************ */
/*
  abstract class for an FTS Object that has Grid-aware inlets/outlets
*/

GridObject::GridObject() {
	for (int i=0; i<MAX_INLETS;  i++) in[i]=0;
	for (int i=0; i<MAX_OUTLETS; i++) out[i]=0;
}

GridObject::~GridObject() {
	check_magic();
	for (int i=0; i<MAX_INLETS;  i++) if (in[i]) delete in[i], in[i]=0;
	for (int i=0; i<MAX_OUTLETS; i++) if (out[i]) delete out[i], out[i]=0;
}

\class GridObject < FObject

\def void initialize (...) {
	Ruby qlass = rb_obj_class(rself);
	if (rb_ivar_get(qlass,SI(@noutlets))==Qnil)
		RAISE("not a GridObject subclass ???");
	int noutlets = INT(rb_ivar_get(qlass,SI(@noutlets)));
	Ruby handlers = rb_ivar_get(qlass,SI(@handlers));
	if (handlers!=Qnil) {
		for (int i=0; i<rb_ary_len(handlers); i++) {
			GridHandler *gh = FIX2PTR(GridHandler,rb_ary_ptr(handlers)[i]);
			in[gh->winlet] = new GridInlet(this,gh);
		}
	}
	for (int i=0; i<noutlets; i++) out[i] = new GridOutlet(this,i);
	rb_call_super(argc,argv);
}

/* category: input */

//!@#$ does not handle types properly
//!@#$ most possibly a big hack
template <class T>
void GridObject_r_flow(GridInlet *in, int n, Pt<T> data) {
	GridObject *self = in->parent;
	int i;
	for (i=0; i<MAX_INLETS; i++) if (in==self->in[i]) break;
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
	GridInlet *inl = in[inln];
	if (!inl) RAISE("no such inlet #%d",inln);
	if (!inl->is_busy()) return Qnil;
	return number_type_table[inl->nt].sym;
}

\def Array inlet_dim (int inln) {
	if (inln<0 || inln>=MAX_INLETS) RAISE("bad inlet number");
	GridInlet *inl = in[inln];
	if (!inl) RAISE("no such inlet #%d",inln);
	if (!inl->is_busy()) return Qnil;
	int n=inl->dim->count();
	Ruby a = rb_ary_new2(n);
	for(int i=0; i<n; i++) rb_ary_push(a,INT2NUM(inl->dim->v[i]));
	return a;
}

\def void inlet_set_factor (int inln, int factor) {
	if (inln<0 || inln>=MAX_INLETS) RAISE("bad inlet number");
	GridInlet *inl = in[inln];
	if (!inl) RAISE("no such inlet #%d",inln);
	if (!inl->is_busy()) RAISE("inlet #%d not active",inln);
	inl->set_factor(factor);
}

\def void send_out_grid_begin (int outlet, Array buf, NumberTypeE nt=int32_type_i) {
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet number");
	int n = rb_ary_len(buf);
	Ruby *p = rb_ary_ptr(buf);
	STACK_ARRAY(int32,v,n);
	for (int i=0; i<n; i++) v[i] = INT(p[i]);
	if (!out[outlet]) RAISE("outlet not found");
	out[outlet]->begin(new Dim(n,v),nt);
}

template <class T>
void send_out_grid_flow_2(GridOutlet *go, Ruby s, T bogus) {
	int n = rb_str_len(s) / sizeof(T);
	Pt<T> p = rb_str_pt(s,T);
/*
	fprintf(stderr,"send_out_grid_flow_2: sending %d elements (dex=%d/%d)\n",
		n,go->dex,go->dim->prod());
*/
	go->send(n,p);
}

\def void send_out_grid_flow (int outlet, String buf, NumberTypeE nt=int32_type_i) {
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet number");
	GridOutlet *go = out[outlet];
#define FOO(T) send_out_grid_flow_2(go,argv[1],(T)0);
	TYPESWITCH(nt,FOO,)
#undef FOO
}

\def void send_out_grid_abort (int outlet) {
	if (outlet<0 || outlet>=MAX_OUTLETS) RAISE("bad outlet number");
	out[outlet]->abort();
}

static void *GridObject_allocate ();

/* install_rgrid(Integer inlet, Boolean multi_type? = true) */
static Ruby GridObject_s_install_rgrid(int argc, Ruby *argv, Ruby rself) {
	if (argc<1 || argc>2) RAISE("er...");
//	if (INT(argv[0])!=0) RAISE("not yet");
	GridHandler *gh = new GridHandler;
	gh->winlet = INT(argv[0]);
	bool mt = argc>1 ? argv[1]==Qtrue : 0; /* multi_type? */
	gh->mode = 4;
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
		case 0: return inl->begin(         argc,argv), Qnil;
		case 1: return inl->from_ruby_list(argc,argv), Qnil;
		case 2: return inl->from_ruby(     argc,argv), Qnil;
		case 3: return inl->from_ruby(     argc,argv), Qnil;
		}
		return Qnil;
	}
	hell: return rb_call_super(argc,argv);
}

/* moved the stuff to the other destructor */
\def void delete_m () {
	rb_call_super(argc,argv);
}

GRCLASS(GridObject,LIST(),
	\grdecl
){
	IEVAL(rself,"install 'GridObject',0,0");
	/* define in Ruby-metaclass */
	rb_define_singleton_method(rself,"instance_methods",(RMethod)GridObject_s_instance_methods,-1);
	rb_define_singleton_method(rself,"install_rgrid",(RMethod)GridObject_s_install_rgrid,-1);
	rb_enable_super(rb_singleton_class(rself),"instance_methods");
}

\end class GridObject

/* **************** Format **************************************** */
/* this is an abstract base class for file formats, network protocols, etc */

#ifdef FORMAT_LIST
extern FClass FORMAT_LIST( ,ci,);
FClass *format_classes[] = { FORMAT_LIST(&,ci,) };
#else
FClass *format_classes[] = {};
#endif

/* **************************************************************** */

Ruby cGridObject, cFormat;

void startup_formats () {
	for (int i=0; i<COUNT(format_classes); i++) {
		fclass_install(format_classes[i], cFormat);
	}
}

void startup_grid () {
	fclass_install(&ciGridObject, cFObject);
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

   GCC 3.2 optimises so much that i have to make the following
   kludge non-static and i can't even put exit(1) at the beginning of it.
*/
//static
void make_gimmick () {
//    exit(1); /* i warned you. */
	GridOutlet foo(0,0);
#define FOO(S) foo.give(0,Pt<S>());
EACH_NUMBER_TYPE(FOO)
#undef FOO
}
