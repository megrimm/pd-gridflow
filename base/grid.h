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

#ifndef __GF_GRID_H
#define __GF_GRID_H

/* current version number as string literal */
#define GF_VERSION "0.7.6"
#define GF_COMPILE_TIME __DATE__ ", " __TIME__

#include <new>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <math.h>
#include <version.h>

extern "C" {
#include <ruby.h>
#include <rubyio.h>
};

#ifndef RUBY_H
#error "Can't do anything without ruby.h"
#endif

#include "../config.h"

/* !@#$ what am I going to do about this? should this be changed? */
/* should I wrap all of the Ruby API for C++-style convenience? */
typedef VALUE Ruby;
/* typedef struct Ruby { VALUE x }; */

#define LIST(args...) args

#ifdef IS_BRIDGE
#define RAISE(args...) rb_raise(rb_eArgError,args)
#else
#define RAISE(args...) rb_raise0(__FILE__,__LINE__,__PRETTY_FUNCTION__,rb_eArgError,args)
#endif

/* avoid ruby warning */
#ifndef rb_enable_super
#define rb_enable_super(a,b) \
	if (RUBY_RELEASE_CODE < 20030716) rb_enable_super(a,b)
#endif

/* undocumented function from Ruby that is one thing we need to fix a very elusive bug
that manifests itself when embedding ruby inside a plugin of another app. This exists
for all versions of Ruby up to now, and I don't know when it gets fixed. */
extern "C" void Init_stack(VALUE *addr);

extern "C" {
void rb_raise0(
const char *file, int line, const char *func, VALUE exc, const char *fmt, ...)
__attribute__ ((noreturn));
};
#define L fprintf(stderr,"%s:%d in %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define SI(_sym_) (rb_intern(#_sym_))
#define SYM(_sym_) (ID2SYM(SI(_sym_)))
#define DGS(_class_) \
	_class_ *self; \
	Data_Get_Struct(rself,_class_,self); \
	self->check_magic();

/* returns the size of a statically defined array */
/* warning: does not work with STACK_ARRAY() */
#define COUNT(_array_) ((int)(sizeof(_array_) / sizeof((_array_)[0])))

/* !@#$ could these be inline functions? */
#define rb_str_len(s) (RSTRING(s)->len)
#define rb_str_ptr(s) (RSTRING(s)->ptr)
#define rb_str_pt(s,t) Pt<t>((t*)rb_str_ptr(s),rb_str_len(s))
#define rb_ary_len(s) (RARRAY(s)->len)
#define rb_ary_ptr(s) (RARRAY(s)->ptr)
#define IEVAL(_self_,s) rb_funcall(_self_,SI(instance_eval),1,rb_str_new2(s))
#define EVAL(s) rb_eval_string(s)
#define rassert(_p_) if (!(_p_)) RAISE(#_p_);

/* because of older versions of Ruby (1.6.?) */
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
		::abort(); }

/* disabling assertion checking? */
#ifndef HAVE_DEBUG
#undef assert
#define assert(_foo_)
#endif

#ifdef HAVE_TSC_PROFILING
#define HAVE_PROFILING
#endif


#ifdef HAVE_PROFILING
#define PROF(_self_) for (GFStackMarker gf_marker(_self_);gf_marker.once();)
#else
#define PROF(_self_)
#endif /* HAVE_PROFILING */

static inline Ruby PTR2FIX (const void *ptr) {
	long p = (long)ptr;
	//fprintf(stderr,"pointer&3==%d\n",p&3);
	if ((p&3)!=0) {
		fprintf(stderr,"unaligned pointer: %08x\n",(int)(ptr));
		::raise(11);//assert(!"segfault ignored???");
	}
	return INT2NUM(p>>2);
}
#define PTR2FIXA(ptr) INT2NUM(((long)(int32*)ptr)&0xffff)
#define PTR2FIXB(ptr) INT2NUM((((long)(int32*)ptr)>>16)&0xffff)
#define FIX2PTR(type,ruby) ((type *)(INT(ruby)<<2))
#define FIX2PTRAB(type,v1,v2) ((type *)(INT(v1)+(INT(v2)<<16)))

const char *rb_sym_name(Ruby sym);

/* **************************************************************** */

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
typedef unsigned long long uint64;
typedef char  int8;
typedef short int16;
typedef long  int32;
typedef long long int64;
typedef float  float32;
typedef double float64;

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
int c=a<0; return (a/b)-(c&&!!(a%b));}

static inline int32   abs(  int32 a) { return a>0?a:-a; }
static inline int64   abs(  int64 a) { return a>0?a:-a; }
static inline float32 abs(float32 a) { return fabs(a); }
static inline float64 abs(float64 a) { return fabs(a); }

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
static inline float64 ipow(float64 a, float64 b) { return pow(a,b); }

#undef min
#undef max

/* minimum/maximum functions */

template <class T> /* where T is comparable */
static inline T min(T a, T b) { return a<b?a:b; }

template <class T> /* where T is comparable */
static inline T max(T a, T b) { return a>b?a:b; }

/*
static inline int min(int a, int b) { int c = (a-b)>>31; return (a&c)|(b&~c); }
static inline int max(int a, int b) { int c = (a-b)>>31; return (a&c)|(b&~c); }
*/

/* greatest common divisor, by euclid's algorithm */
/* this runs in log(a+b) number operations */
template <class T>
static T gcd (T a, T b) {
	while (b) {
		T c=mod(a,b);
		a=b;
		b=c;
	}
	return a;
}

/* greatest common divisor, the binary algorithm. haven't tried yet. */
template <class T>
static T gcd2 (T a, T b) {
	int s=0;
	while ((a|b)&1==0) { a>>=1; b>>=1; s++; }
	while (a) {
		if (a&1==0) a>>=1;
		else if (b&1==0) b>>=1;
		else {
			T t=abs(a-b);
			if (a<b) b=t; else a=t;
		}
	}
	return b<<s;
}

/* least common multiple */
/* this runs in log(a+b) */
template <class T>
static inline T lcm (T a, T b) {
	return a*b/gcd(a,b);
}

// returns the highest bit set in a word, or 0 if none.
template <class T>
int highest_bit(T n) {
	int i=0;
	if (n&0xffff0000) { n>>=16; i+=16; }
	if (n&0x0000ff00) { n>>= 8; i+= 8; }
	if (n&0x000000f0) { n>>= 4; i+= 4; }
	if (n&0x0000000c) { n>>= 2; i+= 2; }
	if (n&0x00000002) { n>>= 1; i+= 1; }
	return i;
}

// returns the lowest bit set in a word, or 0 if none.
template <class T>
int lowest_bit(T n) {
	return highest_bit((~n+1)&n);
}

static double drand() { return 1.0*rand()/(RAND_MAX+1.0); }

/* **************************************************************** */

#ifdef HAVE_PENTIUM
static inline uint64 rdtsc() {
	uint64 x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;}
#else
/* Toto, we're not in Pentium(tm) anymore */
static inline uint64 rdtsc() {return 0;}
#endif

/* is little-endian */
static inline bool is_le() {
	int x=1;
	return ((char *)&x)[0];
}

/* **************************************************************** */

#define EACH_INT_TYPE(MACRO) MACRO(uint8) MACRO(int16) MACRO(int32) MACRO(int64)
#define EACH_FLOAT_TYPE(MACRO) MACRO(float32) MACRO(float64)
#define EACH_NUMBER_TYPE(MACRO) EACH_INT_TYPE(MACRO) EACH_FLOAT_TYPE(MACRO)

// note: loop unrolling macros assume N!=0
#define UNROLL_8(MACRO,N,PTR,ARGS...) \
	int n__=(-N)&7; PTR-=n__; N+=n__; \
	switch (n__) { start: \
		case 0: MACRO(0); \
		case 1: MACRO(1); \
		case 2: MACRO(2); \
		case 3: MACRO(3); \
		case 4: MACRO(4); \
		case 5: MACRO(5); \
		case 6: MACRO(6); \
		case 7: MACRO(7); \
		PTR+=8; N-=8; ARGS; if (N) goto start; }

#define UNROLL_4(MACRO,N,PTR,ARGS...) \
	int n__=(-N)&3; PTR-=n__; N+=n__; \
	switch (n__) { start: \
		case 0: MACRO(0); \
		case 1: MACRO(1); \
		case 2: MACRO(2); \
		case 3: MACRO(3); \
		PTR+=4; N-=4; ARGS; if (N) goto start; }

/* **************************************************************** */
/*
  hook into pointer manipulation.
  will help find memory corruption bugs.
*/
template <class T> class Pt {
public:
	T *p;
#ifdef HAVE_DEBUG
	T *start;
	int n;
	Pt() : p(0), start(0), n(0) {}
	Pt(T *q, int _n) : p(q), start(q), n(_n) {
/* should this be >= or > ? */
#ifdef HAVE_DEBUG_HARDER
/*
		if (p<start || p>start+n) {
			fprintf(stderr,
				"%s\nBUFFER OVERFLOW: 0x%08lx is not in 0x%08lx..0x%08lx\n",
				__PRETTY_FUNCTION__, (long)p,(long)start,(long)(start+n));
			::raise(11);//assert(!"segfault ignored???");
		}		
*/
		#endif
	}
	Pt(T *q, int _n, T *_start) : p(q), start(_start), n(_n) {
#ifdef HAVE_DEBUG_HARDER
/*
		if (p<start || p>start+n) {
			fprintf(stderr,
				"%s\nBUFFER OVERFLOW: 0x%08lx is not in 0x%08lx..0x%08lx\n",
				__PRETTY_FUNCTION__, (long)p,(long)start,(long)(start+n));
			::raise(11);//assert(!"segfault ignored???");
		}		
*/
		#endif
	}
#else
	Pt() : p(0) {}
	Pt(T *q, int _n, T *_start=0) : p(q) {}
#endif

	T &operator *() { return *p; }
	Pt operator+=(int i) { p+=i; return *this; }
	Pt operator-=(int i) { p-=i; return *this; }
	Pt operator++(     ) { p++;  return *this; }
	Pt operator--(     ) { p--;  return *this; }
	Pt operator++(int  ) { Pt f(*this); ++*this; return f; }
	Pt operator--(int  ) { Pt f(*this); --*this; return f; }
	T &operator[](int i) {
#ifdef HAVE_DEBUG_HARDER
		if (!(p+i>=start && p+i<start+n)) {
			fprintf(stderr, "%s\nBUFFER OVERFLOW: 0x%08lx[%ld]=0x%08lx is not in 0x%08lx..0x%08lx\n",
				__PRETTY_FUNCTION__, (long)p, (long)i, (long)(p+i),(long)start,(long)(start+n));
			::raise(11);//assert(!"segfault ignored???");
		}
#endif
		return p[i];
	}

	void will_use(int k) {
		if (k==0) return;
#ifdef HAVE_DEBUG_HARDER
		if (!(p>=start && p<start+n)) {
			fprintf(stderr,
				"%s\nBUFFER OVERFLOW: 0x%08lx is not in 0x%08lx..0x%08lx\n",
				__PRETTY_FUNCTION__, (long)p,(long)start,(long)(start+n));
			::raise(11);//assert(!"segfault ignored???");
		}
		T *q = p+k-1;
		if (!(q>=start && q<start+n)) {
			fprintf(stderr,
				"%s\nBUFFER OVERFLOW: 0x%08lx is not in 0x%08lx..0x%08lx\n",
				__PRETTY_FUNCTION__, (long)q,(long)start,(long)(start+n));
			::raise(11);//assert(!"segfault ignored???");
		}
#endif
	}

	operator bool   () { return (bool   )p; }
	operator void  *() { return (void  *)p; }
	operator int8  *() { return (int8  *)p; }
#define FOO(S) operator S *() { return (S *)p; }
EACH_NUMBER_TYPE(FOO)
#undef FOO
	int operator-(Pt x) { return p-x.p; }

#ifdef HAVE_DEBUG
#define FOO(S) operator Pt<S>() { return Pt<S>((S *)p,n*sizeof(T)/1,(S *)start); }
EACH_NUMBER_TYPE(FOO)
#undef FOO
	template <class U> Pt operator+(U x) { return Pt(p+x,n,start); }
	template <class U> Pt operator-(U x) { return Pt(p-x,n,start); }
#else
#define FOO(S) operator Pt<S>() { return Pt<S>((S *)p,0); }
EACH_NUMBER_TYPE(FOO)
#undef FOO
	template <class U> Pt operator+(U x) { return Pt(p+x,0); }
	template <class U> Pt operator-(U x) { return Pt(p-x,0); }
#endif
};

#define STACK_ARRAY(_type_,_name_,_count_) \
	_type_ _name_##_foo[_count_]; Pt<_type_> _name_(_name_##_foo,_count_);

#define ARRAY_NEW(_type_,_count_) \
	(Pt<_type_>((_type_ *)new _type_[_count_],_count_))

/* **************************************************************** */
/* some basic memory handling */

template <class T>
inline void COPY(Pt<T> dest, Pt<T> src, int n) {
	src.will_use(n);
	dest.will_use(n);
	gfmemcopy((uint8*)dest,(uint8*)src,n*sizeof(T));
}

template <class T>
inline void CLEAR(Pt<T> dest, int n) {
	dest.will_use(n);
	memset(dest,0,n*sizeof(T));
}

void gfmemcopy(uint8 *out, const uint8 *in, int n);

template <class T> static void memswap (Pt<T> a, Pt<T> b, int n) {
	STACK_ARRAY(T,c,n); COPY(c,a,n); COPY(a,b,n); COPY(b,c,n);
}

/* **************************************************************** */
/* my own little Ruby <-> C++ layer */

struct Arg {
	Ruby a;
};

struct ArgList {
	int n;
	Pt<Arg> v;
};

static inline bool INTEGER_P(Ruby x) {
	return FIXNUM_P(x) || TYPE(x)==T_BIGNUM;
}

static inline bool FLOAT_P(Ruby x) {
	return TYPE(x)==T_FLOAT;
}

static inline int32 INT(Ruby x) {
	if (INTEGER_P(x)) return NUM2INT(x);
	if (FLOAT_P(x)) return NUM2INT(rb_funcall(x,SI(round),0));
	RAISE("expected Integer or Float (got %s)",
		rb_str_ptr(rb_funcall(x,SI(inspect),0)));
}

static short convert(Ruby x, short *foo) {
	int v = INT(x);
	if (v<-0x8000 || v>=0x8000) RAISE("value is out of range");
	return v;
}

static uint16 convert(Ruby x, uint16 *foo) {
	int v = INT(x);
	if (v<0 || v>=0x10000) RAISE("value is out of range");
	return v;
}

static int   convert(Ruby x, int   *foo) { return INT(x); }
static int32 convert(Ruby x, int32 *foo) { return INT(x); }
static bool  convert(Ruby x, bool  *foo) {
	switch (TYPE(x)) {
		case T_FIXNUM: return INT(x);
		/*...*/
		default: RAISE("can't convert to bool");
	}
}

typedef Ruby Symbol;
typedef Ruby Array;
typedef Ruby String;
typedef Ruby Integer;
static Ruby convert(Ruby x, Ruby *bogus) {
	return x;
}


typedef Ruby (*RMethod)(...); /* !@#$ fishy */

/* you shouldn't use MethodDecl directly */
struct MethodDecl {
	const char *selector;
	RMethod method;
};

/* **************************************************************** */

#define BUILTIN_SYMBOLS(MACRO) \
	MACRO(_grid,"grid") \
	MACRO(_bang,"bang") \
	MACRO(_int,"int") \
	MACRO(_float,"float") \
	MACRO(_list,"list") \
	MACRO(_lparen,"(") \
	MACRO(_rparen,")") \
	MACRO(_lbrace,"{") \
	MACRO(_rbrace,"}") \
	MACRO(_sharp,"#") \
	MACRO(iv_outlets,"@outlets") \
	MACRO(iv_ninlets,"@ninlets") \
	MACRO(iv_noutlets,"@noutlets")

extern struct BuiltinSymbols {
#define FOO(_sym_,_str_) Ruby _sym_;
BUILTIN_SYMBOLS(FOO)
#undef FOO
} bsym;

/* **************************************************************** */

#define OBJECT_MAGIC 1618033989

/* base class for C++ classes that get exported to Ruby */
struct Object {
	int32 magic;
	Ruby rself; /* point to Ruby peer */

	Object() : magic(OBJECT_MAGIC), rself(0) {}

	void check_magic () {
		if (magic != OBJECT_MAGIC) {
			fprintf(stderr,"Object memory corruption! (ask the debugger)\n");
			for (int i=-2; i<=2; i++) {
				fprintf(stderr,"this[%d]=0x%08x\n",i,((int*)this)[i]);
			}
			raise(11);
		}
	}

	/*
	  part of the purpose of this is to cause the virtual-table-field
	  to be created in FObject and not GridObject, because else gdb pretends
	  the virtual table is after FObject when it's actually before, and then
	  ((GridObject *)a)->foo != ((FObject *)a)->foo
	  PS: I've heard this is not a hack, this is the only way, as in:
	  Even dynamic_cast<> relies on this.
	*/

	virtual ~Object() { magic = 0xDEADBEEF; }
	virtual void mark() {} /* not used for now */
};

void Object_free (void *);
void define_many_methods(Ruby rself, int n, MethodDecl *methods);

/* **************************************************************** */

/* maximum number of dimensions in an array */
#define MAX_DIMENSIONS 16

/* a Dim is a list of dimensions that describe the shape of a grid */
struct Dim : Object {
	int n;
	Pt<int32> v; /* safe pointer */
	int32 v2[MAX_DIMENSIONS]; /* real stuff */
	
	void check();

	Dim(int n, Pt<int32> v) {
		this->v = Pt<int32>(v2,MAX_DIMENSIONS);
		this->n = n;
		COPY(this->v,v,n); check();
	}

	Dim(int n, int32* v) {
		this->v = Pt<int32>(v2,MAX_DIMENSIONS);
		this->n = n;
		COPY(this->v,Pt<int32>(v,n),n); check();
	}

	Dim *dup() { return new Dim(n,v); }
	int count() {return n;}
	int get(int i) { return v[i]; }

	int32 prod(int start=0, int end=-1) {
		if (end<0) end+=n;
		int32 tot=1;
		for (int i=start; i<=end; i++) tot *= v[i];
		return tot;
	}

	char *to_s();

	bool equal(Dim *o) {
		if (n!=o->n) return false;
		for (int i=0; i<n; i++) if (v[i]!=o->v[i]) return false;
		return true;
	}
};

/* **************************************************************** */

/* BitPacking objects encapsulate optimised loops of conversion */
struct BitPacking;

/* those are the types of the optimised loops of conversion */ 
/* inputs are const */
struct Packer {
#define FOO(S) void (*as_##S)(BitPacking *self, int n, Pt<S> in,   Pt<uint8> out);
EACH_INT_TYPE(FOO)
#undef FOO
};
struct Unpacker {
#define FOO(S) void (*as_##S)(BitPacking *self, int n, Pt<uint8> in, Pt<S> out);
EACH_INT_TYPE(FOO)
#undef FOO
};

\class BitPacking < Object
struct BitPacking : Object {
	Packer *packer;
	Unpacker *unpacker;
	unsigned int endian; /* 0=big, 1=little, 2=same, 3=different */
	int bytes;
	int size;
	uint32 mask[4];

	BitPacking(){::abort();} /* don't call, but don't remove. sorry. */
	BitPacking(int endian, int bytes, int size, uint32 *mask,
		Packer *packer=0, Unpacker *unpacker=0);
	bool is_le();
	bool eq(BitPacking *o);
	\decl void initialize(Ruby foo1, Ruby foo2, Ruby foo3);
	\decl String pack2(String ins, String outs=Qnil);
	\decl String unpack2(String ins, String outs=Qnil);

	//new
	\decl String to_s();

/* main entrances to Packers/Unpackers */
	template <class T> void pack(  int n, Pt<T> in, Pt<uint8> out);
	template <class T> void unpack(int n, Pt<uint8> in, Pt<T> out);
};
\end class

int high_bit(uint32 n);
int low_bit(uint32 n);
void swap32 (int n, Pt<uint32> data);
void swap16 (int n, Pt<uint16> data);

#define NT_UNSIGNED (1<<0)
#define NT_FLOAT (1<<1)
#define NT_UNSUPPORTED (1<<2)

#define NUMBER_TYPE_LIMITS(T,a,b,c) \
	inline T nt_smallest(T *bogus) {return a;} \
	inline T nt_greatest(T *bogus) {return b;} \
	inline T nt_all_ones(T *bogus) {return c;}

NUMBER_TYPE_LIMITS(uint8,0,255,255)
NUMBER_TYPE_LIMITS(int16,0x8000,0x7fff,-1)
NUMBER_TYPE_LIMITS(int32,0x80000000,0x7fffffff,-1)
NUMBER_TYPE_LIMITS(int64,(int64)0x8000000000000000LL,(int64)0x7fffffffffffffffLL,-1)
NUMBER_TYPE_LIMITS(float32,-HUGE_VAL,+HUGE_VAL,(RAISE("all_ones"),0))
NUMBER_TYPE_LIMITS(float64,-HUGE_VAL,+HUGE_VAL,(RAISE("all_ones"),0))

#define NUMBER_TYPES(MACRO) \
	MACRO(     uint8,  8, NT_UNSIGNED, "u8,b") \
	MACRO(      int8,  8, NT_UNSUPPORTED, "i8") \
	MACRO(    uint16, 16, NT_UNSIGNED | NT_UNSUPPORTED, "u16") \
	MACRO(     int16, 16, 0, "i16,s") \
	MACRO(    uint32, 32, NT_UNSIGNED | NT_UNSUPPORTED, "u32") \
	MACRO(     int32, 32, 0, "i32,i") \
	MACRO(    uint64, 64, NT_UNSIGNED | NT_UNSUPPORTED, "u64") \
	MACRO(     int64, 64, NT_UNSUPPORTED, "i64,l") \
	MACRO(   float32, 32, NT_FLOAT, "f32,f") \
	MACRO(   float64, 64, NT_FLOAT | NT_UNSUPPORTED, "f64,d")

enum NumberTypeE {
#define FOO(_sym_,args...) _sym_##_type_i,
NUMBER_TYPES(FOO)
#undef FOO
	number_type_table_end
};

#define FOO(_type_) \
inline NumberTypeE NumberTypeE_type_of(_type_ &x) { \
	return _type_##_type_i; }
EACH_NUMBER_TYPE(FOO)
#undef FOO

\class NumberType < Object
struct NumberType : Object {
	Ruby /*Symbol*/ sym;
	const char *name;
	int size;
	int flags;
	const char *aliases;
	NumberTypeE index;
	
	NumberType(const char *name_, int size_, int flags_, const char *aliases_) :
		name(name_), size(size_), flags(flags_), aliases(aliases_) {}
	
	//new
	\decl Symbol sym_m();
	\decl int size_m();
	\decl int flags_m();
	\decl int index_m();
};
\end class

NumberTypeE NumberTypeE_find (Ruby sym);

#define TYPESWITCH(T,C,E) switch (T) { \
	case uint8_type_i: C(uint8) break; \
	case int16_type_i: C(int16) break; \
	case int32_type_i: C(int32) break; \
	case int64_type_i: C(int64) break; \
	case float32_type_i: C(float32) break; \
	case float64_type_i: C(float64) break; \
	default: E; RAISE("type '%s' not available here",number_type_table[T].sym);}

#define TYPESWITCH_NOFLOAT(T,C,E) switch (T) { \
	case uint8_type_i: C(uint8) break; \
	case int16_type_i: C(int16) break; \
	case int32_type_i: C(int32) break; \
	case int64_type_i: C(int64) break; \
	default: E; RAISE("type '%s' not available here",number_type_table[T].sym);}

/* Operator objects encapsulate optimised loops of simple operations */

template <class T>
struct Numop1On : Object {
	typedef void (*Map)(int,T*);
	Map op_map;
	Numop1On(Map m) : op_map(m) {}
	Numop1On() {}
	Numop1On(const Numop1On &z) {
		op_map = z.op_map; }
};

\class Numop1 < Object
struct Numop1 : Object {
	Ruby /*Symbol*/ sym;
	const char *name;
//private:
#define FOO(T) \
		Numop1On<T> on_##T; \
		Numop1On<T> *on(T &foo) { \
			if (!on_##T.op_map) RAISE("operator does not support this type"); \
			return &on_##T;}
EACH_NUMBER_TYPE(FOO)
#undef FOO
//public:
	template <class T> inline void map(int n, Pt<T> as) {
		as.will_use(n);
		on(*as)->op_map(n,(T *)as);}

	Numop1(Ruby /*Symbol*/ sym_, const char *name_,
#define FOO(T) Numop1On<T> op_##T, 
EACH_NUMBER_TYPE(FOO)
#undef FOO
	bool bogosity=0) : sym(sym_), name(name_) {
#define FOO(T) on_##T = op_##T;
EACH_NUMBER_TYPE(FOO)
#undef FOO
	}

	\decl Symbol sym_m ();
	\decl void map_m (NumberTypeE nt, int n, String as);
};
\end class

enum LeftRight { at_left, at_right };

template <class T>
struct Numop2On : Object {
	/* Function Vectorisations */
	typedef void (*Map)(int n, T *as, T b);
	typedef void (*Zip)(int n, T *as, T *bs);
	//typedef void (*Zip2)(int n, T *as, T *bs, T *cs);
	typedef void (*Fold)(int an, int n, T *as, T *bs);
	typedef void (*Scan)(int an, int n, T *as, T *bs);
	Map op_map;
	Zip op_zip;
	Fold op_fold;
	Scan op_scan;

	/* Algebraic Properties (those involving numeric types) */
	typedef bool (*AlgebraicCheck)(T x, LeftRight side);
	/* neutral: right: forall y {f(x,y)=x}; left: forall x {f(x,y)=y}; */
	AlgebraicCheck is_neutral;
	/* absorbent: right: exists a forall y {f(x,y)=a}; ... */
	AlgebraicCheck is_absorbent;
	
	/* Constructors */
	Numop2On(Map m, Zip z, Fold f, Scan s,
		AlgebraicCheck n,
		AlgebraicCheck a) :
			op_map(m), op_zip(z), op_fold(f), op_scan(s),
			is_neutral(n), is_absorbent(a) {}
	Numop2On() {}
	Numop2On(const Numop2On &z) {
		op_map = z.op_map;
		op_zip = z.op_zip;
		op_fold = z.op_fold;
		op_scan = z.op_scan;
		is_neutral = z.is_neutral;
		is_absorbent = z.is_absorbent; }
};

/* semigroup property: associativity: f(a,f(b,c))=f(f(a,b),c) */
#define OP2_ASSOC (1<<0)
/* abelian property: commutativity: f(a,b)=f(b,a) */
#define OP2_COMM (1<<1)

\class Numop2 < Object
struct Numop2 : Object {
	Ruby /*Symbol*/ sym;
	const char *name;
	int flags;
//private:
#define FOO(T) \
		Numop2On<T> on_##T; \
		Numop2On<T> *on(T &foo) { \
			if (!on_##T.op_map) RAISE("operator does not support this type"); \
			return &on_##T;}
EACH_NUMBER_TYPE(FOO)
#undef FOO
//public:
	template <class T> inline void map(int n, Pt<T> as, T b) {
		as.will_use(n);
		on(*as)->op_map(n,(T *)as,b);}
	template <class T> inline void zip(int n, Pt<T> as, Pt<T> bs) {
		as.will_use(n);
		bs.will_use(n);
		on(*as)->op_zip(n,(T *)as,(T *)bs);}
/*
		template <class T> inline void zip2(int n, Pt<T> as, Pt<T> bs, Pt<T> cs) {
		as.will_use(n);
		bs.will_use(n);
		cs.will_use(n);
		on(*as)->op_zip2(n,(T *)as,(T *)bs,(T *)cs);}
*/
	template <class T> inline void fold(int an, int n, Pt<T> as, Pt<T> bs) {
		as.will_use(an);
		bs.will_use(an*n);
		on(*as)->op_fold(an,n,(T *)as,(T *)bs);}
	template <class T> inline void scan(int an, int n, Pt<T> as, Pt<T> bs) {
		as.will_use(an);
		bs.will_use(an*n);
		on(*as)->op_scan(an,n,(T *)as,(T *)bs);}

	\decl void map_m  (NumberTypeE nt, int n, String as, String b);
	\decl void zip_m  (NumberTypeE nt, int n, String as, String bs);
	\decl void fold_m (NumberTypeE nt, int n, String as, String bs);
	\decl void scan_m (NumberTypeE nt, int an, int n, String as, String bs);

	Numop2(Ruby /*Symbol*/ sym_, const char *name_,
#define FOO(T) Numop2On<T> op_##T, 
EACH_NUMBER_TYPE(FOO)
#undef FOO
	int flags_) : sym(sym_), name(name_), flags(flags_) {
#define FOO(T) on_##T = op_##T;
EACH_NUMBER_TYPE(FOO)
#undef FOO
	}
};
\end class

extern NumberType number_type_table[];
extern Ruby number_type_dict; /* GridFlow.@number_type_dict={} */
extern Ruby op1_dict; /* GridFlow.@op1_dict={} */
extern Ruby op2_dict; /* GridFlow.@op2_dict={} */

static inline NumberTypeE convert(Ruby x, NumberTypeE *bogus) {
	return NumberTypeE_find(x);
}

#ifndef IS_BRIDGE
static Numop1 *convert(Ruby x, Numop1 **bogus) {
	Ruby s = rb_hash_aref(op1_dict,x);
	if (s==Qnil) RAISE("expected one-input-operator");
	return FIX2PTR(Numop1,s);
}

static Numop2 *convert(Ruby x, Numop2 **bogus) {
	Ruby s = rb_hash_aref(op2_dict,x);
	if (s==Qnil) RAISE("expected two-input-operator");
	return FIX2PTR(Numop2,s);
}
#endif

/* **************************************************************** */

/* will RAISE with proper message if invalid */
/* DimConstraint functions should be replaced by something more
   sophisticated, but not now.
*/
typedef void (*DimConstraint)(Dim *d);

struct Grid : Object {
	Grid *next;
	DimConstraint dc;
	Dim *dim;
	NumberTypeE nt;
	void *data;

	Grid() : next(0), dc(0), dim(0), nt(int32_type_i), data(0) {}
	void constrain(DimConstraint dc_) { dc=dc_; }
	void init(Dim *dim, NumberTypeE nt);
	void init_clear(Dim *dim, NumberTypeE nt);
	void init_from_ruby(Ruby x);
	void init_from_ruby_list(int n, Ruby *a);
	void del();

	int32 bytes() { return dim->prod()*number_type_table[nt].size/8; }
	inline bool is_empty() { return !dim; }
	Dim *to_dim ();

#define FOO(type) \
	operator type *() { return (type *)data; } \
	operator Pt<type>() { return Pt<type>((type *)data,dim->prod()); }
EACH_NUMBER_TYPE(FOO)
#undef FOO
	
	void operator= (Grid &foo) {
		dc = foo.dc;
		init(foo.dim->dup(),foo.nt);
		COPY(Pt<uint8>((uint8 *)    data,bytes()),
		     Pt<uint8>((uint8 *)foo.data,bytes()),bytes());
	}

	Grid *dup () {
		Grid *foo = new Grid;
		*foo = *this;
		return foo;
	}

	// almost like assignment, but also destroys the grid being assigned.
	void swallow (Grid *victim) {
		if (dc) dc(victim->dim);
		if (dim) delete dim;
		if (data) delete[] (uint8 *)data;
		nt = victim->nt;
		dim = victim->dim;   victim->dim = 0;
		data = victim->data; victim->data = 0;
		delete victim;
	}

	~Grid();
};

static inline Grid *convert (Ruby r, Grid **bogus) {
	if (!r) return 0;
	Grid *foo = new Grid;
	foo->init_from_ruby(r);
	return foo;
}

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

/* macro for declaring an inlet inside GRCLASS() (int32 only) */
#define GRINLET(_class_,i,mode) {i, mode, \
	0, 0, \
	_class_##_grid_inlet_##i, 0, \
	0, 0 }

/* same for inlets that support all types */
#define GRINLET4(_class_,i,mode) {i, mode, \
	_class_##_grid_inlet_##i, _class_##_grid_inlet_##i, \
	_class_##_grid_inlet_##i, _class_##_grid_inlet_##i, \
	_class_##_grid_inlet_##i, _class_##_grid_inlet_##i }

/* same except float unsupported by that inlet */
#define GRINLET2(_class_,i,mode) {i, mode, \
	_class_##_grid_inlet_##i, _class_##_grid_inlet_##i, \
	_class_##_grid_inlet_##i, _class_##_grid_inlet_##i, \
	0, 0 }

/* same except int unsupported by that inlet */
#define GRINLETF(_class_,i,mode) {i, mode, \
	0, 0, 0, 0, \
	_class_##_grid_inlet_##i, _class_##_grid_inlet_##i }

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
_member_.init(in->dim->dup(),NumberTypeE_type_of(*data)); } \
GRID_FLOW { \
COPY((Pt<T>)(_member_)+in->dex, data, n); } \
GRID_FINISH

/* macro for defining a gridinlet's behaviour as just storage */
#define GRID_INPUT2(_class_,_inlet_,_member_) \
	GRID_INLET(_class_,_inlet_) { \
		if (is_busy_except(in)) { \
			if (_member_.next != &_member_) delete _member_.next; \
			_member_.next = new Grid(); \
			_member_.next->dc = _member_.dc; \
		} \
		(_member_.next ? _member_.next : &_member_) \
		->init(in->dim->dup(),NumberTypeE_type_of(*data)); } \
	GRID_FLOW { \
		COPY(((Pt<T>)* \
		(_member_.next ? _member_.next : &_member_) \
		)+in->dex, data, n); } \
	GRID_FINISH

typedef struct GridInlet GridInlet;
typedef struct GridHandler {
	int winlet;
	int mode; /* 0=ignore; 4=ro; 6=rw; 8=dump; 8 is not implemented yet */
	/* It used to be three different function pointers here (begin,flow,end)
	but now, n=-1 is begin, and n=-2 is _finish_. "end" is now used as an
	end-marker for inlet definitions... sorry for the confusion */
#define FOO(_type_) \
	void (*flow_##_type_)(GridInlet *in, int n, Pt<_type_> data); \
	void flow(GridInlet *in, int n, Pt<_type_> data) const { \
		assert(flow_##_type_); \
		flow_##_type_(in,n,data); }
EACH_NUMBER_TYPE(FOO)
#undef FOO
} GridHandler;

typedef struct  GridObject GridObject;
struct GridInlet {
/* context information */
	GridObject *parent;
	const GridHandler *gh;

/* grid progress info */
	Dim *dim;
	NumberTypeE nt;
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
	bool is_busy() { return !!dim; }
	void begin( int argc, Ruby *argv);
	template <class T> void flow(int mode, int n, Pt<T> data);
	void end();
	void from_ruby_list(int argc, Ruby *argv);
	void from_ruby(int argc, Ruby *argv);
	void from_grid(Grid *g);
	bool supports_type(NumberTypeE nt);

private:
	template <class T> void from_grid2(Grid *g, T foo);
};

/* **************************************************************** */
/* DECL/DECL3/METHOD3/GRCLASS : Ruby<->C++ bridge */

/*
  FClass is to be used only at initialization,
  starting with GF-0.7.1; relevant contents are now copied over
  to ruby classes. In any case you shouldn't use it directly.
*/
struct FClass {
	void *(*allocator)(); /* returns a new C++ object */
	void (*startup)(Ruby rself); /* initializer for the Ruby class */
	const char *name; /* C++/Ruby name (not jMax/PD name) */
	int methodsn; MethodDecl *methods; /* C++ -> Ruby methods */
	 /* GridFlow inlet handlers (!@#$ hack) */
	int handlersn; GridHandler *handlers;
};

/* RMethod,MethodDecl,DECL,DECL3 should go here */

#define GRCLASS(_name_,_handlers_,_args_...) \
	static void *_name_##_allocator () { return new _name_; } \
	static MethodDecl _name_ ## _methods[] = { _args_ }; \
	static GridHandler _name_ ## _handlers[] = { _handlers_ }; \
	static void _name_ ## _startup (Ruby rself); \
	FClass ci ## _name_ = { \
		_name_##_allocator, _name_ ## _startup, #_name_, \
		COUNT(_name_##_methods), _name_##_methods, \
		COUNT(_name_##_handlers),_name_##_handlers, \
	}; \
	static void _name_ ## _startup (Ruby rself)

/* **************************************************************** */
/* GridOutlet represents a grid-aware outlet */

struct GridOutlet {
/* these are set only once, at outlet creation */
	GridObject *parent;
	int woutlet;

/* those are set at every beginning of a transmission */
	NumberTypeE nt;
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
	void begin(Dim *dim, NumberTypeE nt=int32_type_i);
	void abort();
	/* give: data must be dynamic. it should not be used by the caller
	   beyond the call to give() */
	template <class T> void give(int n, Pt<T> data);
	/* send/send_direct: data belongs to caller, may be stack-allocated,
	   receiver doesn't modify the data; in send(), there is buffering;
	   in send_direct(), there is not. When switching from buffered to
	   unbuffered mode, flush() must be called */
	template <class T> void send(int n, Pt<T> data);
	void flush();
	void callback(GridInlet *in);
private:
	template <class T> void send_direct(int n, Pt<T> data);
	void end() {
		flush();
		for (int i=0; i<ninlets; i++) inlets[i]->end();
		delete dim; dim = 0; dex = 0;
	}
};

/* **************************************************************** */

typedef struct BFObject BFObject; /* fts_object_t or something */

/* represents objects that have inlets/outlets */
\class FObject < Object
struct FObject : Object {
	BFObject *bself; /* point to jMax/PD peer */
	uint64 total_time;

	FObject() : bself(0), total_time(0) {}

	const char *args() {
		Ruby s=rb_funcall(rself,SI(args),0);
		if (s==Qnil) return 0;
		return rb_str_ptr(s);
	}

	/* result should be printed or copied immediately as the GC may discard it anytime */
	const char *info();

	\decl Ruby total_time_get();
	\decl Ruby total_time_set(Ruby x);
	\decl void send_in (...);
	\decl void send_out (...);
	\decl void delete_m ();
};
\end class FObject

/* 1 + maximum id of last grid-aware inlet/outlet */
#define MAX_INLETS 4
#define MAX_OUTLETS 4

\class GridObject < FObject
struct GridObject : FObject {
	GridInlet  * in[MAX_INLETS];
	GridOutlet *out[MAX_OUTLETS];

	/* Make sure you distinguish #close/#delete, and C++'s delete. The first
	two are quite equivalent and should never make an object "crashable".
	C++ is called by Ruby's garbage collector or by jMax/PureData's delete.
	*/

	GridObject();
	~GridObject();

	bool is_busy_except(GridInlet *gin) {
		for (int i=0; i<MAX_INLETS; i++)
			if (in[i] && in[i]!=gin && in[i]->is_busy()) return true;
		return false;
	}

	/* for Formats */
	Ruby mode () { return rb_ivar_get(rself,SI(@mode)); }

\decl Ruby method_missing(...);
\decl void initialize();
\decl Array inlet_dim(int inln);
\decl Symbol inlet_nt(int inln);
\decl void inlet_set_factor(int inln, int factor);
\decl void delete_m();

\decl void send_out_grid_begin(int outlet, Array buf, NumberTypeE nt=int32_type_i);
\decl void send_out_grid_flow(int outlet, String buf, NumberTypeE nt=int32_type_i);
\decl void send_out_grid_abort(int outlet);
};
\end class GridObject

/* **************************************************************** */

typedef struct GridObject Format;

struct GFBridge {
	/* bridge identification string */
	const char *name;
	/* send message; pre: outlet number is valid; self has a bself */
	Ruby (*send_out)(int argc, Ruby *argv, Ruby sym, int outlet, Ruby rself);
	/* add new class */
	Ruby (*class_install)(Ruby rself, char *name);
	/* to write to the console */
	void (*post)(const char *, ...);
	/* PD adds a newline; jMax doesn't. */
	bool post_does_ln;
	/* milliseconds between refreshes of x11, rtmetro, tcp */
	float clock_tick;
	/* additional gimmickry: bridge-specific functions all-in-one */
	Ruby (*whatever)(int argc, Ruby *argv, Ruby rself);
};

extern Ruby mGridFlow;
extern Ruby cFObject;
extern Ruby cGridObject;
extern Ruby cFormat;

uint64 RtMetro_now();

/* keyed on data */
void MainLoop_add(void *data, void (*func)(void*));
void MainLoop_remove(void *data);

Ruby Pointer_new (void *ptr);
void *Pointer_get (Ruby self);

Ruby fclass_install(FClass *fc, Ruby super=0); /* super=cGridObject */

extern int gf_security; /* unused */

extern "C" GFBridge *gf_bridge;
extern "C" void Init_gridflow ();

void gfpost(const char *fmt, ...);

extern Numop2 *op2_add, *op2_sub, *op2_mul, *op2_div, *op2_mod;
extern Numop2 *op2_shl, *op2_and;

#define SAME_TYPE(_a_,_b_) \
	if ((_a_).nt != (_b_).nt) RAISE("%s: same type please (%s has %s; %s has %s)", \
		this->info(), \
		#_a_, number_type_table[(_a_).nt].name, \
		#_b_, number_type_table[(_b_).nt].name);

#define NOTEMPTY(_a_) if ((_a_).is_empty()) RAISE("in [%s], '%s' is empty",this->info(), #_a_);

static void SAME_DIM(int n, Dim *a, int ai, Dim *b, int bi) {
	if (ai+n > a->n) RAISE("left hand: not enough dimensions");
	if (bi+n > b->n) RAISE("right hand: not enough dimensions");
	for (int i=0; i<n; i++) {
		if (a->v[ai+i] != b->v[bi+i]) {
			RAISE("mismatch: left dim #%d is %d, right dim #%d is %d",
				ai+i, a->v[ai+i],
				bi+i, b->v[bi+i]);
		}
	}
}

/* a stack for the profiler, etc. */
#define GF_STACK_MAX 256
struct GFStack {
	struct GFStackFrame {
		FObject *o;
		void *bp; /* a pointer into system stack */
		uint64 time;
	};
	GFStackFrame s[GF_STACK_MAX];
	int n;
	GFStack() { n = 0; }
	void push (FObject *o) __attribute__((noinline));
	void pop () __attribute__((noinline));
};

extern GFStack gf_stack;

struct GFStackMarker {
	int n;
	bool flag;
	GFStackMarker(FObject *o) { n = gf_stack.n; gf_stack.push(o); flag=true; }
	~GFStackMarker() { while (gf_stack.n != n) gf_stack.pop(); }
	bool once () {
		if (flag) { flag=false; return true; } else return false;
	}
};

#endif /* __GF_GRID_H */
