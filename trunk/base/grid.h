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

#ifndef __GF_GRID_H
#define __GF_GRID_H

/* current version number as string literal */
#define GF_VERSION "0.6.5"
#define GF_COMPILE_TIME __DATE__ ", " __TIME__

#include <new>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include "../config.h"

/* lots of macros follow */
/* i'm not going to explain to you why macros *ARE* a good thing. */
/* **************************************************************** */
/* general-purpose macros */

#define LIST(args...) args
#define RAISE(args...) rb_raise(rb_eArgError,args)
#define DGS(_class_) _class_ *self; Data_Get_Struct(rself,_class_,self);
#define INSTALL(rname) ruby_c_install(&ci##rname, cGridObject);
#define L fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);
#define SI(_sym_) (rb_intern(#_sym_))
#define SYM(_sym_) (ID2SYM(SI(_sym_)))

/* returns the size of a statically defined array */
#define COUNT(_array_) ((int)(sizeof(_array_) / sizeof((_array_)[0])))

/* these could be inline functions */
#define rb_str_len(s) (RSTRING(s)->len)
#define rb_str_ptr(s) (RSTRING(s)->ptr)
#define rb_str_pt(s,t) Pt<t>((t*)rb_str_ptr(s),rb_str_len(s))
#define rb_ary_len(s) (RARRAY(s)->len)
#define rb_ary_ptr(s) (RARRAY(s)->ptr)
#define COPY(_dest_,_src_,_n_) memcpy((_dest_),(_src_),(_n_)*sizeof(*(_dest_)))
#define CLEAR(_dest_,_n_) memset((_dest_),0,(_n_)*sizeof(*(_dest_)))
#define IEVAL(_self_,s) rb_funcall(_self_,SI(instance_eval),1,rb_str_new2(s))
#define EVAL(s) rb_eval_string(s)

/* we're gonna override assert, so load it first, to avoid conflicts */
#include <assert.h>

#undef assert
#define assert(_expr_) \
	if (!(_expr_)) { \
		fprintf(stderr, "%s:%d: assertion failed: %s is false\n", \
			__FILE__, __LINE__, #_expr_); \
		abort(); }

/* disabling assertion checking? */
#ifndef HAVE_DEBUG
#undef assert
#define assert(_foo_)
#endif

#ifdef HAVE_TSC_PROFILING
#define HAVE_PROFILING
#endif

/* those are helpers for profiling. OBSOLETE */
#ifdef HAVE_PROFILING
#define ENTER self->profiler_last = rdtsc();
#define LEAVE self->profiler_cumul += rdtsc() - self->profiler_last;
#define ENTER_P self->parent->profiler_last = rdtsc();
#define LEAVE_P self->parent->profiler_cumul += rdtsc()-self->parent->profiler_last;
#else
#define ENTER
#define LEAVE
#define ENTER_P
#define LEAVE_P
#endif

#define PTR2FIX(ptr) INT2NUM(((long)(int32*)ptr)>>2)
#define FIX2PTR(type,ruby) ((type *)(INT(ruby)<<2))
//#define PTR2FIX(ptr) Pointer_new((void *)ptr)
//#define FIX2PTR(v) Pointer_get(v)
#define PTR2FIXA(ptr) INT2NUM(((long)(int32*)ptr)&0xffff)
#define PTR2FIXB(ptr) INT2NUM((((long)(int32*)ptr)>>16)&0xffff)
#define FIX2PTRAB(type,v1,v2) ((type *)(INT(v1)+(INT(v2)<<16)))

#define DEF(_class_,_name_,_argc_) \
	rb_define_method(c##_class_,#_name_,(RFunc)_class_##_##_name_,_argc_)
#define DEF2(_class_,_name2_,_name_,_argc_) \
	rb_define_method(c##_class_,_name2_,(RFunc)_class_##_##_name_,_argc_)
#define SDEF(_class_,_name_,_argc_) \
	rb_define_singleton_method(c##_class_,#_name_,(RFunc)_class_##_s_##_name_,_argc_)

#define INTEGER_P(_Ruby_) (FIXNUM_P(_Ruby_) || TYPE(_Ruby_)==T_BIGNUM)
#define FLOAT_P(_Ruby_) (TYPE(_Ruby_)==T_FLOAT)

#define INT(x) (INTEGER_P(x) ? NUM2INT(x) : \
	FLOAT_P(x) ? NUM2INT(rb_funcall(x,SI(round),0)) : \
	(RAISE("expected Integer or Float"),0))

/* **************************************************************** */

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
typedef unsigned long long uint64;
typedef char  int8;
typedef short int16;
typedef long  int32;
typedef float  float32;
typedef double float64;

typedef /*volatile*/ VALUE Ruby;

/* three-way comparison */
static inline int cmp(int a, int b) { return a < b ? -1 : a > b; }

/*
  a remainder function such that floor(a/b)*b+mod(a,b) = a
  and for which mod(a,b) is in [0;b) or (b;0]
  in contrast to C-language builtin a%b,
  this one has uniform behaviour around zero.
*/
static inline int mod(int a, int b) {
int c=a%b; c+=b&-(c&&(a<0)^(b<0)); return c;}

/* integer powers in log(b) time */
static inline int ipow(int32 a, uint32 b) {
	int32 r=1;
	while(1) {
		if (b&1) r*=a;
		b>>=1;
		if (!b) return r;
		a*=a;
	}
}	

#undef min
#undef max

/* minimum/maximum functions */

static inline int min(int a, int b) { return a<b?a:b; }
static inline int max(int a, int b) { return a>b?a:b; }
/*
static inline int min(int a, int b) { int c = -(a<b); return (a&c)|(b&~c); }
static inline int max(int a, int b) { int c = -(a>b); return (a&c)|(b&~c); }
static inline int min(int a, int b) { int c = (a-b)>>31; return (a&c)|(b&~c); }
static inline int max(int a, int b) { int c = (a-b)>>31; return (a&c)|(b&~c); }
*/

/* **************************************************************** */

/* DECL/DECL3/METHOD3/GRCLASS : Ruby<->C++ bridge */

#define DECL(_class_,_name_) \
	MethodDecl(#_class_,#_name_,(RMethod) _class_##_##_name_##_wrap)

#define DECL3(_name_) \
	Ruby _name_(int argc, Ruby *argv);

#define METHOD3(_class_,_name_) \
	static Ruby _class_##_##_name_##_wrap(int argc, Ruby *argv, Ruby rself) { \
		DGS(_class_); return self->_name_(argc,argv); } \
	Ruby _class_::_name_(int argc, Ruby *argv)

typedef Ruby (*RMethod)(Ruby rself, ...); /* !@#$ */

#define GRCLASS(_name_,_jname_,_inlets_,_outlets_,_startup_,_handlers_,args...) \
	static void *_name_##_allocate () { return new _name_; } \
	static MethodDecl _name_ ## _methods[] = { args }; \
	static GridHandler _name_ ## _handlers[] = { _handlers_ }; \
	GridClass ci ## _name_ = { \
		0, sizeof(_name_), _name_##_allocate, _startup_, \
		COUNT(_name_##_methods),\
		_name_##_methods,\
		_inlets_,_outlets_,COUNT(_name_##_handlers),_name_##_handlers, \
		#_name_, _jname_ };

/* pentium-only, wtf? */
static inline uint64 rdtsc() {
  uint64 x;
  __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
  return x;}

/* is little-endian */
static inline bool is_le() {
	int x=1;
	return ((char *)&x)[0];
}

/* **************************************************************** */

void gfpost(const char *fmt, ...);

typedef struct MethodDecl {
	const char *qlass;
	const char *selector;
	RMethod method;
//	MethodDecl *next;
//	static MethodDecl *gf_all_methods;

	MethodDecl() {} /* unused but required. new C++ makes no sense */
	MethodDecl(const char *qlass, const char *selector, RMethod method) {
		this->qlass = qlass;
		this->selector = selector;
		this->method = method;
//		this->next = gf_all_methods;
//		gf_all_methods = this;
	}
} MethodDecl;

void define_many_methods(Ruby/*Class*/ rself, int n, MethodDecl *methods);

/* **************************************************************** */
/* maximum number of dimensions in an array */
#define MAX_DIMENSIONS 16
/* **************************************************************** */

/*
  what kind of number a Grid contains.
  note that on the other hand, indexing and dimensioning of Grids is
  still done with explicit ints.
*/
typedef long Number;

/*
  hook into pointer manipulation.
  will help find memory corruption bugs.
*/
template <class T> class Pt {
public:
	T *p;
	T *start;
	int n;

	Pt() : p(0), start(0), n(0) {}
	Pt(T *q, int _n) : p(q), start(q), n(_n) {}
	Pt(T *q, int _n, T *_start) : p(q), start(_start), n(_n) {}
//	Pt(char *q) : p((T *)q) {}
	T &operator *() { return *p; }
	Pt operator++(     ) { p++;  return *this; }
	Pt operator+=(int i) { p+=i; return *this; }
	Pt operator++(int  ) { Pt f(*this); ++*this; return f; }
	T &operator[](int i) {
#ifdef HAVE_DEBUG_HARDER
		if (!(p+i>=start && p+i<start+n)) {
			fprintf(stderr,
				"BUFFER OVERFLOW: 0x%08lx is not in 0x%08lx..0x%08lx\n",
				(long)p,(long)start,(long)start+n);
			raise(11);
		}
#endif
		return p[i];
	}
	operator bool   () { return (bool   )p; }
	operator void  *() { return (void  *)p; }
	operator uint8 *() { return (uint8 *)p; }
	operator int8  *() { return (int8  *)p; }
	operator int16 *() { return (int16 *)p; }
	operator int32 *() { return (int32 *)p; }
//	operator Pt<const T>() { return Pt((const T *)p); }
	Pt operator-(Pt x) { return Pt(p-x.p,n,start); }
	Pt operator+(int x) { return Pt(p+x,n,start); }
	Pt operator-(int x) { return Pt(p-x,n,start); }
};

//template <class T> Pt<T>::operator Pt<const T>() { return Pt<const T>(*this); }

#define STACK_ARRAY(_type_,_name_,_count_) \
	_type_ _name_##_foo[_count_]; Pt<_type_> _name_(_name_##_foo,_count_);

#define ARRAY_NEW(_type_,_count_) \
	(Pt<_type_>((_type_ *)new _type_[_count_],_count_))

/* **************************************************************** */

#define DECL_SYM(_sym_) extern "C" Ruby/*Symbol*/ sym_##_sym_;

DECL_SYM(grid)
DECL_SYM(bang)
DECL_SYM(int)
DECL_SYM(list)

/* a Dim is a list of dimensions that describe the shape of a grid */
struct Dim {
	int n;
	int v[MAX_DIMENSIONS];

	void check();

	Dim(int n) {
		this->n = n;
	}

	Dim(int n, int *v) {
		this->n = n;
		memcpy(this->v,v,n*sizeof(int));
		check();
	}

	Dim *dup() { return new Dim(n,v); }
	int count() {return n;}

	int get(int i) {
		assert(i>=0);
		assert(i<n);
		return v[i];
	}
	int prod(int start=0,int end=-1) {
		if (end<0) end+=n;
		int tot=1;
		for (int i=start; i<=end; i++) tot *= v[i];
		return tot;
	}
	int calc_dex(int *v,int end=-1) {
		if (end<0) end+=n;
		int dex=0;
		for (int i=0; i<=end; i++) dex = dex * this->v[i] + v[i];
		return dex;
	}

	/* big leak machine */
	char *to_s();

	bool equal(Dim *o) {
		if (n!=o->n) return false;
		for (int i=0; i<n; i++) if (v[i]!=o->v[i]) return false;
		return true;
	}
};

/*
struct DimPattern {
	const char *s;
	DimPattern(const char *_s) { s=_s; }
};
*/

/* **************************************************************** */

/* BitPacking objects encapsulate optimised loops of conversion */
struct BitPacking;

/* those are the types of the optimised loops of conversion */ 
/* inputs are const */
typedef void (*  Packer)(BitPacking *self, int n, Pt<Number> in, Pt< uint8> out);
typedef void (*Unpacker)(BitPacking *self, int n, Pt< uint8> in, Pt<Number> out);

struct BitPacking {
	Packer packer;
	Unpacker unpacker;
	unsigned int endian; /* 0=big, 1=little, 2=same, 3=different */
	int bytes;
	int size;
	uint32 mask[4];

	BitPacking(){abort();} /* don't call, but don't remove. sorry. */
	BitPacking(int endian, int bytes, int size, uint32 *mask,
		Packer packer=0, Unpacker unpacker=0);
	void gfpost();
	bool is_le();
	bool eq(BitPacking *o);
	DECL3(initialize);

/* main entrances to Packers/Unpackers */
	void pack(  int n, Pt<Number> in, Pt< uint8> out);
	void unpack(int n, Pt< uint8> in, Pt<Number> out);
	DECL3(pack2);
	DECL3(unpack2);
};

int high_bit(uint32 n);
int low_bit(uint32 n);
void swap32 (int n, Pt<uint32> data);
void swap16 (int n, Pt<uint16> data);

#define DECL_TYPE(_name_,_size_) \
	_name_##_type_i

enum NumberTypeIndex {
	DECL_TYPE(     uint8,  8),
	DECL_TYPE(      int8,  8),
	DECL_TYPE(    uint16, 16),
	DECL_TYPE(     int16, 16),
	DECL_TYPE(    uint32, 32),
	DECL_TYPE(     int32, 32),
	DECL_TYPE(   float32, 32),
	DECL_TYPE(   float64, 32),
	number_type_table_end
};

#undef DECL_TYPE

struct NumberType {
	Ruby /*Symbol*/ sym;
	const char *name;
	int size;
};

NumberTypeIndex NumberType_find (Ruby sym);

/* Operator objects encapsulate optimised loops of simple operations */

template <class T>
struct Operator1On {
/*
	bool has_neutral;
	T neutral;
*/
	Number (*op)(T);
	void   (*op_array)(int,Pt<T>);
};

struct Operator1 {
	Ruby /*Symbol*/ sym;
	const char *name;
	Operator1On<int32> on_int32;
};

template <class T>
struct Operator2On {
/*
	bool has_neutral;
	Number neutral;
*/
	T (*op)(T,T);
	void   (*op_array)(int,Pt<T>,T);
	void   (*op_array2)(int,Pt<T>, Pt</*const*/ T>);
	T (*op_fold)(T,int,Pt</*const*/ T>);
	void   (*op_fold2)(int,Pt<T>,int,Pt</*const*/ T>);
	void   (*op_scan)(T,int,Pt<T>);
	void   (*op_scan2)(int,Pt</*const*/ T>,int,Pt<T>);
};

struct Operator2 {
	Ruby /*Symbol*/ sym;
	const char *name;
	Operator2On<int32> on_int32;
};

extern NumberType number_type_table[];
extern Ruby number_type_dict; /* GridFlow.@number_type_dict={} */
extern Ruby op1_dict; /* GridFlow.@op1_dict={} */
extern Ruby op2_dict; /* GridFlow.@op2_dict={} */

/* **************************************************************** */

/* will RAISE with proper message if invalid */
/* DimConstraint functions should be replaced by something more
   sophisticated, but not now.
*/
typedef void (*DimConstraint)(Dim *d);

struct Grid {
	DimConstraint dc;
	Dim *dim;
	NumberTypeIndex nt;
	void *data;
	/*GridType *constraint; */

	Grid() {
		dc = 0;
		dim = 0;
		nt = int32_type_i;
		data = 0;
	}

	void constrain(DimConstraint dc_) { dc=dc_; }
	void init(Dim *dim, NumberTypeIndex nt=int32_type_i);
	void init_from_ruby(Ruby x);
	void init_from_ruby_list(int n, Ruby *a);
	void del();
	~Grid();
	operator Pt<int32>() { return Pt<int32>((int32 *)data,dim->prod()); }
	operator Pt<uint8>() { return Pt<uint8>((uint8 *)data,dim->prod()); }
	inline bool is_empty() { return !dim; }
	Dim *to_dim ();
};

/* **************************************************************** */
/* GridInlet represents a grid-aware inlet */

/* macro for declaring an inlet inside a class{} block */
#define GRINLET3(_inlet_) \
	void _##_inlet_##_flow(GridInlet *in, int n, Pt<Number>data); \

/* macro for declaring an inlet inside GRCLASS() */
#define GRINLET(_class_,_winlet_,_mode_) {_winlet_, _mode_, \
	(void (*)(GridInlet *, int, Pt<int32>)) \
	 _class_##__##_winlet_##_flow }

/* four-part macro for defining the behaviour of a gridinlet in a class */
#define GRID_INLET(_cl_,_inlet_) \
	static void _cl_##__##_inlet_##_flow\
	(GridInlet *in, int n, Pt<Number> data) { \
		((_cl_*)in->parent)->_##_inlet_##_flow(in,n,data); } \
	void _cl_::_##_inlet_##_flow \
	(GridInlet *in, int n, Pt<Number> data) { \
	if (n==-1)
#define GRID_FLOW else if (n>=0)
#define GRID_FINISH else
#define GRID_END }

/* macro for defining a gridinlet's behaviour as just storage */
#define GRID_INPUT(_class_,_inlet_,_member_) \
	GRID_INLET(_class_,_inlet_) { \
		_member_.del(); _member_.init(in->dim->dup(),int32_type_i); } \
	GRID_FLOW { \
		COPY(&((Pt<int32>)_member_)[in->dex], data, n); } \
	GRID_FINISH

typedef struct  GridInlet  GridInlet;
typedef struct GridHandler {
	int winlet;
	int mode; /* 0=ignore; 4=ro; 6=rw */
	/* It used to be three different function pointers here (begin,flow,end)
	but now, n=-1 is begin, and n=-2 is _finish_. "end" is now used as an
	end-marker for inlet definitions... sorry for the confusion */

	void  (*flow)(GridInlet *, int n, Pt<Number>data);
} GridHandler;

typedef struct  GridObject GridObject;
struct GridInlet {
/* context information */
	GridObject *parent;
	const GridHandler *gh;

/* grid progress info */
	Dim *dim;
	NumberTypeIndex nt;
	int dex;

/* grid receptor */
	int factor; /* flow's n will be multiple of self->factor */
	int bufn;
	Pt<Number> buf; /* factor-chunk buffer */

/* extra */
	GridObject *sender;

/* methods */
	GridInlet(GridObject *parent, const GridHandler *gh);
	~GridInlet();
	void abort();
	void set_factor(int factor);
	bool is_busy();
	void begin( int argc, Ruby *argv);
	void flow(int mode, int n, Pt<Number> data);
	void end();
	void list(  int argc, Ruby *argv);
	void int_(  int argc, Ruby *argv);
	void float_(int argc, Ruby *argv);
	void grid(Grid *g);
};

struct FClass {
};

/* !@#$ should move some of this stuff over to FClass */
/* but can't right now (C++ weirdness) */
struct GridClass /*: FClass */ {
	Ruby rubyclass;
	int objectsize;
	void *(*allocate)();
	void (*startup)(GridClass *);
	int methodsn;
	MethodDecl *methods;
	int inlets;
	int outlets;
	int handlersn;
	GridHandler *handlers;
	const char *name;
	const char *jname;
};

/* **************************************************************** */
/* GridOutlet represents a grid-aware outlet */

struct GridOutlet {
/* these are set only once, at outlet creation */
	GridObject *parent;
	int woutlet;

/* those are set at every beginning of a transmission */
	Dim *dim; /* dimensions of the grid being sent */
	Pt<Number>buf; /* temporary buffer */
	bool frozen; /* is the "begin" phase finished? */
	Pt<GridInlet *> inlets; /* which inlets are we connected to */
	int ninlets; /* how many of them */

/* those are updated during transmission */
	int dex; /* how many numbers were already sent in this connection */
	int bufn; /* number of bytes used in the buffer */

/* transmission accelerator */

/* methods */
	GridOutlet(GridObject *parent, int woutlet);
	~GridOutlet();
	bool is_busy();
	void begin(Dim *dim);
	void abort();
	void give(int n, Pt<Number>data);
	void send(int n, Pt</*const*/ uint8>data);
	void send(int n, Pt</*const*/ Number>data);
	void send_direct(int n, Pt</*const*/ Number>data);
	void flush();
	void end();
	void callback(GridInlet *in);
};

/* **************************************************************** */
/* the <Ruby/C++/Other> mapping of GridObjects is not too clear, sorry */

typedef struct BFObject BFObject; /* fts_object_t or something */

struct FObject {
	Ruby /*GridFlow::FObject*/ rself; /* point to Ruby peer */
	GridClass *grid_class;
	BFObject *foreign_peer; /* point to jMax/PD peer */
	uint64 profiler_cumul, profiler_last;
};

/* 1 + maximum id of last grid-aware inlet/outlet */
#define MAX_INLETS 4
#define MAX_OUTLETS 4

struct GridObject : FObject {
	bool freed; /* paranoia */
	GridInlet  * in[MAX_INLETS];
	GridOutlet *out[MAX_OUTLETS];

	/* Make sure you distinguish #close/#delete, and C++'s delete. The first
	two are quite equivalent and should never make an object "crashable".
	C++ is called by Ruby's garbage collector or by jMax/PureData's delete.
    */

	GridObject();
	virtual void mark(); /* not used for now */
	virtual ~GridObject();

	const char *args() {
		Ruby s=rb_funcall(rself,SI(args),0);
		if (s==Qnil) return 0;
		return rb_str_ptr(s);
	}

	Ruby mode () { return rb_ivar_get(rself,SI(@mode)); }

	DECL3(initialize);
	DECL3(inlet_dim);
	DECL3(inlet_set_factor);
	DECL3(method_missing);
	DECL3(del);
	DECL3(send_out_grid_begin);
	DECL3(send_out_grid_flow);
	DECL3(send_out_grid_end);
};

typedef struct GridObject Format;

inline BFObject *FObject_peer(Ruby rself) {
	DGS(GridObject); return self->foreign_peer;
}

void GridObject_conf_class(Ruby rself, GridClass *grclass);

#define FF_W   (1<<1)
#define FF_R   (1<<2)

typedef struct GFBridge {
	/* send message */
	/* pre: outlet number is valid; self has a foreign_peer */
	Ruby (*send_out)(int argc, Ruby *argv, Ruby sym, int outlet, Ruby rself);
	/* add new class */
	Ruby (*class_install)(Ruby rself, char *name);
	/* to write to the console */
	void (*post)(const char *, ...);
	/* PD adds a newline; jMax doesn't. */
	bool post_does_ln;
	/* milliseconds between refreshes of x11, rtmetro, tcp */
	float clock_tick;
} GFBridge;

extern "C" GFBridge gf_bridge;
extern Ruby mGridFlow;
extern Ruby cFObject;
extern Ruby cGridObject;
extern Ruby cFormat;

uint64 RtMetro_now();

Ruby FObject_send_out(int argc, Ruby *argv, Ruby rself);
Ruby FObject_s_install(Ruby rself, Ruby name, Ruby inlets, Ruby outlets);
Ruby FObject_s_new(Ruby argc, Ruby *argv, Ruby qlass);
const char *rb_sym_name(Ruby sym);

/* keyed on data */
void MainLoop_add(void *data, void (*func)(void*));
void MainLoop_remove(void *data);

Ruby Pointer_new (void *ptr);
void *Pointer_get (Ruby self);

Ruby ruby_c_install(GridClass *gc, Ruby super);

typedef Ruby (*RFunc)(...);

extern "C" void Init_gridflow () /*throws Exception*/;

#endif /* __GF_GRID_H */
