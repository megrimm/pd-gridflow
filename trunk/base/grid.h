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
#define GF_VERSION "0.7.0"
#define GF_COMPILE_TIME __DATE__ ", " __TIME__

#include <new>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>

extern "C" {
#include <ruby.h>
#include <rubyio.h>
};

#ifndef RUBY_H
#error "Can't do anything without ruby.h"
#endif

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
#define IEVAL(_self_,s) rb_funcall(_self_,SI(instance_eval),1,rb_str_new2(s))
#define EVAL(s) rb_eval_string(s)
#define rassert(_p_) if (!(_p_)) RAISE(#_p_);

/* because of older versions of Ruby (1.6.???) */
#define rb_obj_class(o) rb_funcall((o),SI(class),0)

#define WATCH(n,ar) { \
	char foo[16*1024], *p=foo; \
	p += sprintf(p,"%s: ",#ar); \
	for (int q=0; q<n; q++) p += sprintf(p,"%ld ",(long)ar[q]); \
	gfpost("%s",foo); \
}

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

#ifdef HAVE_DEBUG
//#define HAVE_DEBUG_HARDER
#endif

#ifdef HAVE_TSC_PROFILING
#define HAVE_PROFILING
#endif

#ifdef HAVE_PROFILING
#define ENTER(_self_) { _self_->profiler_last = rdtsc(); }
#define LEAVE(_self_) { \
	if (_self_->profiler_last) \
		_self_->profiler_cumul += rdtsc() - _self_->profiler_last; \
	_self_->profiler_last = 0; }
#else
#define ENTER(_self_)
#define LEAVE(_self_)
#endif

#define PTR2FIX(ptr) INT2NUM(((long)(int32*)ptr)>>2)
#define FIX2PTR(type,ruby) ((type *)(INT(ruby)<<2))
//#define PTR2FIX(ptr) Pointer_new((void *)ptr)
//#define FIX2PTR(v) Pointer_get(v)
#define PTR2FIXA(ptr) INT2NUM(((long)(int32*)ptr)&0xffff)
#define PTR2FIXB(ptr) INT2NUM((((long)(int32*)ptr)>>16)&0xffff)
#define FIX2PTRAB(type,v1,v2) ((type *)(INT(v1)+(INT(v2)<<16)))

#define DEF(_class_,_name_,_argc_) \
	rb_define_method(c##_class_,#_name_,(RMethod)_class_##_##_name_,_argc_)
#define DEF2(_class_,_name2_,_name_,_argc_) \
	rb_define_method(c##_class_,_name2_,(RMethod)_class_##_##_name_,_argc_)
#define SDEF(_class_,_name_,_argc_) \
	rb_define_singleton_method(c##_class_,#_name_,(RMethod)_class_##_s_##_name_,_argc_)

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
template <class T> /* where T is comparable */
static inline T cmp(T a, T b) { return a < b ? -1 : a > b; }

/*
  a remainder function such that div2(a,b)*b+mod(a,b) = a
  and for which mod(a,b) is in [0;b) or (b;0].
  in contrast to C-language builtin a%b,
  this one has uniform behaviour around zero.
*/
static inline int mod(int a, int b) {
int c=a%b; c+=b&-(c&&(a<0)^(b<0)); return c;}

/* counterpart of mod(a,b), just like a/b and a%b are counterparts */
static inline int div2(int a, int b) {
	int c=a<0;
	return (a/b)-(c&&!!(a%b));
}

static inline int32   abs(  int32 a) { return  abs(a); }
static inline float32 abs(float32 a) { return fabs(a); }

/* integer powers in log(b) time */
template <class T> /* where T is integral type */
static inline T ipow(T a, T b) {
	T r=1;
	for(;;) {
		if (b&1) r*=a;
		b>>=1;
		if (!b) return r;
		a*=a;
	}
}	

/* kludge */
static inline float32 ipow(float32 a, float32 b) { return pow(a,b); }

#undef min
#undef max

/* minimum/maximum functions */

template <class T> /* where T is comparable */
static inline T min(T a, T b) { return a<b?a:b; }

template <class T> /* where T is comparable */
static inline T max(T a, T b) { return a>b?a:b; }

/*
static inline int min(int a, int b) { int c = -(a<b); return (a&c)|(b&~c); }
static inline int max(int a, int b) { int c = -(a>b); return (a&c)|(b&~c); }
static inline int min(int a, int b) { int c = (a-b)>>31; return (a&c)|(b&~c); }
static inline int max(int a, int b) { int c = (a-b)>>31; return (a&c)|(b&~c); }
*/

int highest_bit(uint32 n);
int lowest_bit(uint32 n);

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

/* !@#$ */
typedef Ruby (*RMethod)(...);
//typedef Ruby (*RMethod)();


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

#ifdef HAVE_PENTIUM
static inline uint64 rdtsc() {
	uint64 x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;}
#else
static inline uint64 rdtsc() {
	RAISE("Toto, we're not in Pentium(tm) anymore");}
#endif

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
	operator float32 *() { return (float32 *)p; }
	operator Pt<uint8>() { return Pt<uint8>((uint8 *)p,n*sizeof(T)/1,(uint8 *)start); }
	operator Pt<int16>() { return Pt<int16>((int16 *)p,n*sizeof(T)/2,(int16 *)start); }
	operator Pt<int32>() { return Pt<int32>((int32 *)p,n*sizeof(T)/4,(int32 *)start); }
//	operator T *() { return p; }
//	operator Pt<const T>() { return Pt((const T *)p); }
	int operator-(Pt x) { return p-x.p; }
	template <class U> Pt operator+(U x) { return Pt(p+x,n,start); }
	template <class U> Pt operator-(U x) { return Pt(p-x,n,start); }
};

//template <class T> Pt<T>::operator Pt<const T>() { return Pt<const T>(*this); }

#define STACK_ARRAY(_type_,_name_,_count_) \
	_type_ _name_##_foo[_count_]; Pt<_type_> _name_(_name_##_foo,_count_);

#define ARRAY_NEW(_type_,_count_) \
	(Pt<_type_>((_type_ *)new _type_[_count_],_count_))

/* **************************************************************** */
/* some basic memory handling */

#define COPY(_dest_,_src_,_n_) gfmemcopy((uint8*)(_dest_),(uint8*)(_src_),(_n_)*sizeof(*(_dest_)))
#define CLEAR(_dest_,_n_) memset((_dest_),0,(_n_)*sizeof(*(_dest_)))
void gfmemcopy(uint8 *out, const uint8 *in, int n);
template <class T> static void memswap (T *a, T *b, int n) {
	T c[n];
	COPY(c,a,n);
	COPY(a,b,n);
	COPY(b,c,n);
}

/* **************************************************************** */

#define DECL_SYM(_sym_) extern "C" Ruby/*Symbol*/ sym_##_sym_;

DECL_SYM(grid)
DECL_SYM(bang)
DECL_SYM(int)
DECL_SYM(list)

/* maximum number of dimensions in an array */
#define MAX_DIMENSIONS 16

/* a Dim is a list of dimensions that describe the shape of a grid */
struct Dim {
	int n;
	int32 v[MAX_DIMENSIONS];

	void check();

	Dim(int n, int32 *v=0) {
		this->n = n;
		if (v) memcpy(this->v,v,n*sizeof(int)), check();
	}

	Dim *dup() { return new Dim(n,v); }
	int count() {return n;}

	int get(int i) {
		assert(i>=0);
		assert(i<n);
		return v[i];
	}
	int32 prod(int start=0, int end=-1) {
		if (end<0) end+=n;
		int32 tot=1;
		for (int i=start; i<=end; i++) tot *= v[i];
		return tot;
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
struct Packer {
	void (*as_uint8  )(BitPacking *self, int n, Pt<uint8> in,   Pt<uint8> out);
	void (*as_int16  )(BitPacking *self, int n, Pt<int16> in,   Pt<uint8> out);
	void (*as_int32  )(BitPacking *self, int n, Pt<int32> in,   Pt<uint8> out);
//	void (*as_float32)(BitPacking *self, int n, Pt<float32> in, Pt<uint8> out);
};
struct Unpacker {
	void (*as_uint8  )(BitPacking *self, int n, Pt<uint8> in, Pt<uint8> out);
	void (*as_int16  )(BitPacking *self, int n, Pt<uint8> in, Pt<int16> out);
	void (*as_int32  )(BitPacking *self, int n, Pt<uint8> in, Pt<int32> out);
//	void (*as_float32)(BitPacking *self, int n, Pt<uint8> in, Pt<float32> out);
};

struct BitPacking {
	Packer *packer;
	Unpacker *unpacker;
	unsigned int endian; /* 0=big, 1=little, 2=same, 3=different */
	int bytes;
	int size;
	uint32 mask[4];

	BitPacking(){abort();} /* don't call, but don't remove. sorry. */
	BitPacking(int endian, int bytes, int size, uint32 *mask,
		Packer *packer=0, Unpacker *unpacker=0);
	void gfpost();
	bool is_le();
	bool eq(BitPacking *o);
	DECL3(initialize);

/* main entrances to Packers/Unpackers */
	template <class T> void pack(  int n, Pt<T> in, Pt<uint8> out);
	template <class T> void unpack(int n, Pt<uint8> in, Pt<T> out);
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
	DECL_TYPE(   float64, 64),
	number_type_table_end
};

#undef DECL_TYPE

#define NTI_MAKE(_type_) \
inline NumberTypeIndex NumberTypeIndex_type_of(_type_ &x) { \
	return _type_##_type_i; }
NTI_MAKE(uint8)
NTI_MAKE(int16)
NTI_MAKE(int32)
NTI_MAKE(float32)
#undef NTI_MAKE

struct NumberType {
	Ruby /*Symbol*/ sym;
	const char *name;
	int size;
};

NumberTypeIndex NumberTypeIndex_find (Ruby sym);

/* Operator objects encapsulate optimised loops of simple operations */

template <class T>
struct Operator1On {
	void   (*op_map)(int,T*);
};

struct Operator1 {
	Ruby /*Symbol*/ sym;
	const char *name;
//private:
#define MAKE_TYPE_ENTRY(type) \
		Operator1On<type> on_##type; \
		Operator1On<type> *on(type &foo) { \
			if (!on_##type.op_map) RAISE("operator does not support this type"); \
			return &on_##type;}
	MAKE_TYPE_ENTRY(uint8)
	MAKE_TYPE_ENTRY(int16)
	MAKE_TYPE_ENTRY(int32)
	MAKE_TYPE_ENTRY(float32)
#undef MAKE_TYPE_ENTRY
//public:
	template <class T> inline void map(int n, Pt<T> a) {
		on(*a)->op_map(n,(T *)a);}
};

template <class T>
struct Operator2On {
	void (*op_map)(int,T*,T);
	void (*op_zip)(int,T*,T*);
	T    (*op_fold)(T,int,T*);
	void (*op_fold2)(int,T*,int,T*);
	void (*op_scan)(T,int,T*);
	void (*op_scan2)(int,T*,int,T*);
};

struct Operator2 {
	Ruby /*Symbol*/ sym;
	const char *name;
//private:
#define MAKE_TYPE_ENTRY(type) \
		Operator2On<type> on_##type; \
		Operator2On<type> *on(type &foo) { \
			if (!on_##type.op_map) RAISE("operator does not support this type"); \
			return &on_##type;}
	MAKE_TYPE_ENTRY(uint8)
	MAKE_TYPE_ENTRY(int16)
	MAKE_TYPE_ENTRY(int32)
	MAKE_TYPE_ENTRY(float32)
#undef MAKE_TYPE_ENTRY
//public:
	template <class T> inline void map(int n, Pt<T> a, T b) {
		on(*a)->op_map(n,(T *)a,b);}
	template <class T> inline void zip(int n, Pt<T> a, Pt<T> b) {
		on(*a)->op_zip(n,(T *)a,(T *)b);}
	template <class T> inline T fold(T a, int n, Pt<T> b) {
		return on(a)->op_fold(a,n,(T *)b);}
	template <class T> inline void fold2(int an, Pt<T> as, int n, Pt<T> bs) {
		on(*as)->op_fold2(an,(T *)as,n,(T *)bs);}
	template <class T> inline T scan(T a, int n, Pt<T> b) {
		on(a)->op_scan(a,n,(T *)b);}
	template <class T> inline void scan2(int an, Pt<T> as, int n, Pt<T> bs) {
		on(*as)->op_scan2(an,(T *)as,n,(T *)bs);}
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
	Grid *next;
	DimConstraint dc;
	Dim *dim;
	NumberTypeIndex nt;
	void *data;
	/*GridType *constraint; */

	Grid() {
		next = this;
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
#define DEFCAST(type) \
	operator type *() { return (type *)data; } \
	operator Pt<type>() { return Pt<type>((type *)data,dim->prod()); }
	DEFCAST(uint8)
	DEFCAST(int16)
	DEFCAST(int32)
	DEFCAST(float32)
#undef DEFCAST
	inline bool is_empty() { return !dim; }
	Dim *to_dim ();
};

/* **************************************************************** */
/* GridInlet represents a grid-aware inlet */
/*
   sorry for the header file pollution.
   with all the new C++ templating (v0.7) i don't exactly know what
   should go where.
*/

/* macro for declaring an inlet inside a class{} block */
#define GRINLET3(_inlet_) \
	template <class T> \
	void grid_inlet_##_inlet_(GridInlet *in, int n, Pt<T> data); \

/* macro for declaring an inlet inside GRCLASS() */
#define GRINLET(_class_,i,mode) {i, mode, \
	0, \
	0, \
	_class_##_grid_inlet_##i, \
	0, }

/* same for inlets that support more than just int32. */
#define GRINLET2(_class_,i,mode) {i, mode, \
	_class_##_grid_inlet_##i, \
	_class_##_grid_inlet_##i, \
	_class_##_grid_inlet_##i, \
	0, }

/* four-part macro for defining the behaviour of a gridinlet in a class */
#define GRID_INLET(_cl_,_inlet_) \
	template <class T> \
	static void _cl_##_grid_inlet_##_inlet_\
	(GridInlet *in, int n, Pt<T> data) { \
		((_cl_*)in->parent)->grid_inlet_##_inlet_(in,n,data); } \
	template <class T> \
	void _cl_::grid_inlet_##_inlet_ \
	(GridInlet *in, int n, Pt<T> data) { \
	if (n==-1)
#define GRID_FLOW else if (n>=0)
#define GRID_FINISH else
#define GRID_END }

/* macro for defining a gridinlet's behaviour as just storage */
#define GRID_INPUT(_class_,_inlet_,_member_) \
	GRID_INLET(_class_,_inlet_) { \
		if (is_busy()) { \
			if (_member_.next == &_member_) { \
				gfpost("object busy (backstoring data)"); \
				_member_.next = new Grid(); \
				_member_.next->dc = _member_.dc; \
			} else { \
				gfpost("object busy and backstore busy (aborting)"); \
				in->abort(); \
			} \
		} \
		_member_.next->init(in->dim->dup(),NumberTypeIndex_type_of(*data)); } \
	GRID_FLOW { \
		COPY(&((Pt<T>)*_member_.next)[in->dex], data, n); } \
	GRID_FINISH

typedef struct GridInlet GridInlet;
typedef struct GridHandler {
	int winlet;
	int mode; /* 0=ignore; 4=ro; 6=rw; 8=dump; 8 is not implemented yet */
	/* It used to be three different function pointers here (begin,flow,end)
	but now, n=-1 is begin, and n=-2 is _finish_. "end" is now used as an
	end-marker for inlet definitions... sorry for the confusion */
#define DEFFLOW(_type_) \
	void (*flow_##_type_)(GridInlet *in, int n, Pt<_type_> data); \
	void flow(GridInlet *in, int n, Pt<_type_> data) const { \
		assert(flow_##_type_); \
		flow_##_type_(in,n,data); }
	DEFFLOW(uint8);
	DEFFLOW(int16);
	DEFFLOW(int32);
	DEFFLOW(float32);
#undef DEFFLOW
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
/*	Pt<int32> (*get_target)(GridInlet *self); */
	int factor; /* flow's n will be multiple of self->factor */
	Grid buf; /* factor-chunk buffer */
	int bufi; /* buffer index: how much of buf is filled */

/* extra */
	GridObject *sender;

/* methods */
	GridInlet(GridObject *parent, const GridHandler *gh);
	~GridInlet();
	void abort();
	void set_factor(int factor);
	bool is_busy();
	void begin( int argc, Ruby *argv);
	template <class T>
	void flow(int mode, int n, Pt<T> data);
	void end();
	void list(  int argc, Ruby *argv);
	void int_(  int argc, Ruby *argv);
	void float_(int argc, Ruby *argv);
	void grid(Grid *g);
	bool supports_type(NumberTypeIndex nt);

private:
	template <class T>
	void grid2(Grid *g, T foo);
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
	NumberTypeIndex nt;
	Dim *dim; /* dimensions of the grid being sent */
	Grid buf; /* temporary buffer */
	bool frozen; /* is the "begin" phase finished? */
	Pt<GridInlet *> inlets; /* which inlets are we connected to */
	int ninlets; /* how many of them */

/* those are updated during transmission */
	int dex; /* how many numbers were already sent in this connection */
	int bufi; /* number of bytes used in the buffer */

/* transmission accelerator */

/* methods */
	GridOutlet(GridObject *parent, int woutlet);
	~GridOutlet();
	bool is_busy() { return !!dim; }

	void begin(Dim *dim, NumberTypeIndex nt=int32_type_i);
	void abort();

	/* give: data must be dynamic. it should not be used by the caller
	   beyond the call to give() */
	template <class T>
	void GridOutlet::give(int n, Pt<T> data);

	/* send/send_direct: data belongs to caller, may be stack-allocated,
	   receiver doesn't modify the data; in send(), there is buffering;
	   in send_direct(), there is not. When switching from buffered to
	   unbuffered mode, flush() must be called */
	template <class T>
	void send(int n, Pt<T> data);

	void flush() {
		if (!bufi) return;
		switch(buf.nt) {
		case uint8_type_i: send_direct(bufi,(Pt<uint8>)buf); break;
		case int16_type_i: send_direct(bufi,(Pt<int16>)buf); break;
		case int32_type_i: send_direct(bufi,(Pt<int32>)buf); break;
		case float32_type_i: send_direct(bufi,(Pt<float32>)buf); break;
		default: RAISE("argh");
		}
		bufi = 0;
	}
	void callback(GridInlet *in);
private:
	template <class T>
	void send_direct(int n, Pt<T> data);
	void end() {
		flush();
		for (int i=0; i<ninlets; i++) inlets[i]->end();
		delete dim; dim = 0; dex = 0;
	}
};

/* **************************************************************** */
/* the <Ruby/C++/Other> mapping of GridObjects is not too clear, sorry */

typedef struct BFObject BFObject; /* fts_object_t or something */

struct FObject {
	Ruby /*GridFlow::FObject*/ rself; /* point to Ruby peer */
	GridClass *grid_class;
	BFObject *foreign_peer; /* point to jMax/PD peer */
	uint64 profiler_cumul;
	uint64 profiler_last;
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

	bool is_busy_except(GridInlet *gin) {
		for (int i=0; i<MAX_INLETS; i++)
			if (in[i] && in[i]!=gin && in[i]->is_busy()) return true;
		return false;
	}

	/* for Formats */
	Ruby mode () { return rb_ivar_get(rself,SI(@mode)); }

	DECL3(initialize);
	DECL3(inlet_dim);
	DECL3(inlet_set_factor);
	DECL3(method_missing);
	DECL3(del);
	DECL3(send_out_grid_begin);
	DECL3(send_out_grid_flow);
	DECL3(send_out_grid_abort);
};

inline BFObject *FObject_peer(Ruby rself) {
	DGS(GridObject); return self->foreign_peer;
}

void GridObject_conf_class(Ruby rself, GridClass *grclass);

/* **************************************************************** */

/* **************************************************************** */

typedef struct GridObject Format;

#define FF_W   (1<<1)
#define FF_R   (1<<2)

struct GFBridge {
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
};

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

extern "C" GFBridge gf_bridge;
extern "C" void Init_gridflow () /*throws Exception*/;

#endif /* __GF_GRID_H */
