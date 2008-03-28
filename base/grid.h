/*
	$Id$

	GridFlow
	Copyright (c) 2001-2007 by Mathieu Bouchard

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
#include "../gridflow2.h"

#define GF_VERSION "0.9.0"

#ifndef IS_BRIDGE
#include <stdio.h>
extern "C" void *gfmalloc(size_t n);
extern "C" void gffree(void *p);
#if __GNUC__<4
#define ALLOCPREFIX static inline
#else
#define ALLOCPREFIX inline
#endif
#include "./new.h"
ALLOCPREFIX void *operator new   (size_t n)          throw (std::bad_alloc) {return gfmalloc(n);}
ALLOCPREFIX void *operator new[] (size_t n)          throw (std::bad_alloc) {return gfmalloc(n);}
ALLOCPREFIX void *operator new   (size_t n, const std::nothrow_t&) throw () {return gfmalloc(n);}
ALLOCPREFIX void *operator new[] (size_t n, const std::nothrow_t&) throw () {return gfmalloc(n);}
ALLOCPREFIX void operator delete  (void*p) throw() {gffree(p);}
ALLOCPREFIX void operator delete[](void*p) throw() {gffree(p);}
ALLOCPREFIX void operator delete  (void*p, const std::nothrow_t&) throw() {gffree(p);}
ALLOCPREFIX void operator delete[](void*p, const std::nothrow_t&) throw() {gffree(p);}
#endif

#ifdef USE_RUBY
extern "C" {
#include <ruby.h>
#include <rubyio.h>
#include <version.h>
#ifndef RUBY_H
#error "Can't do anything without ruby.h"
#endif
};
#endif

#ifdef __WIN32__
#define INT winINT
#define random rand
#undef send
#undef close
#define sigjmp_buf jmp_buf
#define siglongjmp longjmp
#endif

//#define _L_ post("%s:%d in %s",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define _L_ fprintf(stderr,"%s:%d in %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);

#ifdef IS_BRIDGE
#define RAISE(args...) rb_raise(rb_eArgError,"[rubypd] "args)
#else
#define RAISE(args...) throw new Barf(__FILE__,__LINE__,__PRETTY_FUNCTION__,args)
#endif
#define RAISE2(string) RAISE(string)

#ifdef USE_RUBY
typedef VALUE Ruby;
#else
typedef long Ruby;
#endif

extern "C" {
void rb_raise0(
const char *file, int line, const char *func, Ruby exc, const char *fmt, ...)
__attribute__ ((noreturn));
};

#define VA int argc, Ruby *argv
#define SI(_sym_) (rb_intern(#_sym_))
#define SYM(_sym_) (ID2SYM(SI(_sym_)))
#define DGS(_class_) _class_ *self; Data_Get_Struct(rself,_class_,self);

// returns the size of a statically defined array
#define COUNT(_array_) ((int)(sizeof(_array_) / sizeof((_array_)[0])))

#ifdef USE_RUBY
#ifdef RARRAY_LEN
#if RUBY_VERSION_MAJOR > 1 || RUBY_VERSION_MINOR >= 9
// T_SYMBOL stops existing when RARRAY_LEN was introduced in 1.9 mid-2006 ?
// but at the same time you can't do this with 1.8.5 which has RARRAY_LEN too
#undef T_SYMBOL
#define T_SYMBOL T_STRING
#endif // 1.9.0
static inline long  rb_str_len(Ruby s) {return RSTRING_LEN(s);}
static inline char *rb_str_ptr(Ruby s) {return RSTRING_PTR(s);}
static inline long  rb_ary_len(Ruby s) {return  RARRAY_LEN(s);}
static inline Ruby *rb_ary_ptr(Ruby s) {return  RARRAY_PTR(s);}
#else
static inline long  rb_str_len(Ruby s) {return RSTRING(s)->len;}
static inline char *rb_str_ptr(Ruby s) {return RSTRING(s)->ptr;}
static inline long  rb_ary_len(Ruby s) {return  RARRAY(s)->len;}
static inline Ruby *rb_ary_ptr(Ruby s) {return  RARRAY(s)->ptr;}
#endif // RARRAY_LEN
static inline const char *rb_sym_name(Ruby sym) {return rb_id2name(SYM2ID(sym));}
#else // USE_RUBY
#endif // USE_RUBY

#define IEVAL(_self_,s) rb_funcall(_self_,SI(instance_eval),1,rb_str_new2(s))
#define EVAL(s) rb_eval_string(s)

#define WATCH(n,ar) { \
	char foo[16*1024], *p=foo; p += sprintf(p,"%s: ",#ar); \
	for (int q=0; q<n; q++) p += sprintf(p,"%lld ",(long long)ar[q]); \
	post("%s",foo);}

#ifdef USE_RUBY
static inline Ruby PTR2FIX (const void *ptr) {
	long p = (long)ptr;
	if ((p&3)!=0) RAISE("unaligned pointer: %p\n",ptr);
	return LONG2NUM(p>>2);
}
#define FIX2PTR(T,ruby) ((T *)(TO(long,ruby)<<2))
#define INT2PTR(T,   v) ((T *)(          (v)<<2))
#endif

//****************************************************************
// my own little Ruby <-> C++ layer

#ifdef USE_RUBY
static inline bool INTEGER_P(Ruby x) {return FIXNUM_P(x)||TYPE(x)==T_BIGNUM;}
static inline bool FLOAT_P(Ruby x)   {return TYPE(x)==T_FLOAT;}
typedef Ruby Symbol, Array, String, Integer;
static Ruby convert(Ruby x, Ruby *bogus) { return x; }
typedef Ruby (*RMethod)(...); /* !@#$ fishy */
#else
typedef t_symbol *Symbol;
//typedef std::string String;
typedef const char *String;
//#define Qnil 4
#define Qnil 0
typedef void *(*RMethod)(...); /* !@#$ fishy */
#endif // USE_RUBY

#define BUILTIN_SYMBOLS(MACRO) \
	MACRO(_grid,"grid") MACRO(_bang,"bang") MACRO(_float,"float") \
	MACRO(_list,"list") MACRO(_sharp,"#") \
	MACRO(iv_ninlets,"@ninlets") MACRO(iv_noutlets,"@noutlets")

extern struct BuiltinSymbols {
#define FOO(_sym_,_str_) Symbol _sym_;
BUILTIN_SYMBOLS(FOO)
#undef FOO
} bsym;

struct Numop;
struct Pointer;
extern Ruby cPointer;
#ifdef USE_RUBY
typedef struct R {
	VALUE r;
	R() {r=Qnil;}
	R(int           x) {r=  INT2NUM(x);}
	R(unsigned      x) {r= UINT2NUM(x);}
	R(long          x) {r= LONG2NUM(x);}
	R(unsigned long x) {r=ULONG2NUM(x);}
	R(double     x) {r=rb_float_new(x);}
#ifdef HAVE_GCC64
	R( int64 x) {r= LONG2NUM(x);}
	R(uint64 x) {r=ULONG2NUM(x);}
#else
	R( int64 x) {
		r = rb_funcall( INT2NUM(( int32)(x>>32)),SI(<<),1,INT2FIX(32));
		r = rb_funcall(r,SI(+),1,UINT2NUM((uint32)x));}
	R(uint64 x) {
		r = rb_funcall(UINT2NUM((uint32)(x>>32)),SI(<<),1,INT2FIX(32));
		r = rb_funcall(r,SI(+),1,UINT2NUM((uint32)x));}
#endif
	R(Numop *x);
	R(t_symbol *x) {r = ID2SYM(rb_intern(x->s_name));}

	operator bool () const {
		if (r==Qtrue) return true;
		if (r==Qfalse) return false;
		switch (TYPE(r)) {
			case T_FIXNUM: case T_BIGNUM: case T_FLOAT: return !!NUM2INT(r);
			default: RAISE("can't convert to bool");}}
/*	operator bool () { // added a non-const copy so that gcc stops complaining. C++ is weird
		if (r==Qtrue) return true;
		if (r==Qfalse) return false;
		switch (TYPE(r)) {
			case T_FIXNUM: case T_BIGNUM: case T_FLOAT: return !!NUM2INT(r);
			default: RAISE("can't convert to bool");}}
*/	operator uint8 () const {return INT2NUM(r);}
	operator int16 () const {
		int v = (int32)*this;
		if (v<-0x8000 || v>=0x8000) RAISE("value %d is out of range",v);
		return v;}
	operator uint16 () const {
		int v = (int32)*this;
		if (v<0 || v>=0x10000) RAISE("value %d is out of range",v);
		return v;}
	operator int32 () const {
		// not using plain NUM2INT because Ruby can convert Symbol to int (compat Ruby1.4)
		if (INTEGER_P(r)) return NUM2INT(r);
		if (FLOAT_P(r)) return NUM2INT(rb_funcall(r,SI(round),0));
		volatile Ruby meuh = rb_funcall(r,SI(inspect),0);
		RAISE("expected Integer or Float (got %s)",rb_str_ptr(meuh));}
	operator long () const {
		return sizeof(long)==sizeof(int32) ? (int32)*this : (int64)*this;}

#ifdef HAVE_GCC64
	operator uint64 () const {return NUM2ULONG(r);}
	operator  int64 () const {return NUM2ULONG(r);}
#else
	operator uint64 () const {
		if (FIXNUM_P(r)) return (uint64)FIX2LONG(r);
		if (TYPE(r)!=T_BIGNUM) RAISE("type error");
		uint64 v = (uint64)NUM2UINT(rb_funcall(r,SI(>>),1,INT2FIX(32))) << 32;
		return v + NUM2UINT(rb_funcall(r,SI(&),1,UINT2NUM(0xffffffff)));}
	operator  int64 () const {
		if (FIXNUM_P(r)) return (int64)FIX2LONG(r);
		if (TYPE(r)!=T_BIGNUM) RAISE("type error");
		int64 v = (int64)NUM2INT(rb_funcall(r,SI(>>),1,INT2FIX(32))) << 32;
		return v + NUM2UINT(rb_funcall(r,SI(&),1,UINT2NUM(0xffffffff)));}
#endif
	operator float32 () const {return (float64)*this;}
	operator float64 () const {
		if (INTEGER_P(r)) return (float64)(int32)*this;
		if (TYPE(r)!=T_FLOAT) RAISE("not a Float");
		return ((RFloat*)r)->value;}
	operator t_symbol * () const {
		if (TYPE(r)==T_SYMBOL) return gensym((char *)rb_sym_name(r));
		if (TYPE(r)==T_STRING) return gensym((char *)rb_str_ptr(r));
 		RAISE("want Symbol or String");
	}
	operator Pointer * () const {
		if (CLASS_OF(r)!=cPointer) RAISE("not a Pointer");
		return (Pointer *)NUM2ULONG(rb_funcall(r,SI(ptr),0));
	}
	static R value(VALUE r) {R x; x.r=r; return x;}
#define FOO(As,Op) \
	R &operator As (int x) {r=rb_funcall(r, SI(Op),1,INT2NUM(x)); return *this;}
	FOO(+=,+) FOO(-=,-) FOO(*=,*) FOO(/=,/) FOO(%=,%)
	FOO(&=,&) FOO(|=,|) FOO(^=,^) FOO(<<=,<<) FOO(>>=,>>)
#undef FOO
//	bool operator  == (int x) {return rb_funcall(r,SI(==),1,INT2NUM(x));}
#define FOO(Op) \
	R operator Op (R x)   const {return rb_funcall(r,SI(Op),1,x.r);} \
	R operator Op (int x) const {return rb_funcall(r,SI(Op),1,INT2NUM(x));}
	FOO(+) FOO(-) FOO(*) FOO(/) FOO(%)
	FOO(&) FOO(|) FOO(^) FOO(<<) FOO(>>)
	FOO(<) FOO(>) FOO(<=) FOO(>=) FOO(==) FOO(!=)
#undef FOO
#define FOO(Op) \
	R operator Op () const {return rb_funcall(r,SI(Op),0);}
	FOO(-) FOO(~)
#undef FOO
} ruby;
#define INT(x)  convert(x,(int32*)0)
#define TO(T,x) convert(x,(T*)0)
template <class T> T convert(Ruby x, T *foo) {R r; r.r = x; return (T)r;}
static R operator -(int a, R b) {return rb_funcall(a,SI(Op),1,INT2NUM(b.r));}
static inline R   ipow(R a, R b) {return R::value(rb_funcall(a.r,SI(**),1,b.r));}
static inline R gf_abs(R a)      {return R::value(rb_funcall(a.r,SI(abs),0));}
static inline R    cmp(R a, R b) {return R::value(rb_funcall(a.r,SI(<=>),1,b.r));}
#endif // USE_RUBY

//****************************************************************

//template <class T> class P : T * {};
//a reference counting pointer class
template <class T> class P {
public:
#define INCR if (p) p->refcount++;
#define DECR if (p) {p->refcount--; if (!p->refcount) delete p;}
	T *p;
	P() : p(0) {}
	P(T *_p)            { p = _p; INCR; }
	P(const P<T> &_p) {
//fprintf(stderr,"350: &_p=%08x\n",(long)&_p); fprintf(stderr,"350: _p.p=%08x\n",(long)_p.p);
 p = _p.p; INCR; }
	P<T> &operator =(T *  _p) { DECR; p=_p;   INCR; return *this; }
	P<T> &operator =(P<T> _p) { DECR; p=_p.p; INCR; return *this; }
	bool operator ==(P<T> _p) { return p == _p.p; }
	bool operator !=(P<T> _p) { return p != _p.p; }
	~P() { DECR; }
	bool operator !(){ return  !p; }
	operator bool()  { return !!p; }
	T &operator *()  { return  *p; }
	T *operator ->() { return   p; }
//#undef INCR
//#undef DECR
};

void gfmemcopy(uint8 *out, const uint8 *in, long n);
template <class T> inline void COPY(T *dest, T *src, long n) {
	gfmemcopy((uint8*)dest,(const uint8*)src,n*sizeof(T));
}
template <class T> inline void CLEAR(T *dest, long n) {
	memset(dest,0,n*sizeof(T));
}
template <class T> static void memswap (T *a, T *b, long n) {
	T c[n]; COPY(c,a,n); COPY(a,b,n); COPY(b,c,n);
}

//****************************************************************
// CObject is the base class for C++ classes that get exported to Ruby.
// BTW: It's quite convenient to have virtual-methods in the base class
// because otherwise the vtable* isn't at the beginning of the object
// and that's pretty confusing to a lot of people, especially when simple
// casting causes a pointer to change its value.

struct CObject {
	int32 refcount;
	Ruby rself; // point to Ruby peer
	CObject() : refcount(0), rself(0) {}
	virtual ~CObject() {}
};
void CObject_free (void *);

// you shouldn't use MethodDecl directly (used by source_filter.rb)
struct MethodDecl { const char *selector; RMethod method; };
void define_many_methods(Ruby rself, int n, MethodDecl *methods);
extern Ruby mGridFlow, cFObject, cGridObject, cFormat;

#undef check

//****************************************************************
// a Dim is a list of dimensions that describe the shape of a grid
typedef int32 Card; /* should be switched to long int soon */
\class Dim : CObject {
	static const Card MAX_DIM=16; // maximum number of dimensions in a grid
	Card n;
	Card v[MAX_DIM]; // real stuff
	void check(); // test invariants
	Dim(Card n, Card *v){this->n=n; COPY(this->v,v,n); check();}
	Dim()                    {n=0;                     check();}
	Dim(Card a)              {n=1;v[0]=a;              check();}
	Dim(Card a,Card b)       {n=2;v[0]=a;v[1]=b;       check();}
	Dim(Card a,Card b,Card c){n=3;v[0]=a;v[1]=b;v[2]=c;check();}
	Dim(Dim *a, Dim *b, Dim *c=0) {
		n=a->n+b->n; if(c) n+=c->n;
		if (n>Dim::MAX_DIM) RAISE("too many dims");
		COPY(v     ,a->v,a->n);
		COPY(v+a->n,b->v,b->n);
		if(c) COPY(v+a->n+b->n,c->v,c->n);
	}
	Card count() {return n;}
	Card get(Card i) {return v[i];}
/*	Dim *range(Card i, Card j) {return new Dim(...);} */
	Card prod(Card start=0, Card end=-1) {
		if (end<0) end+=n;
		Card tot=1;
		for (Card i=start; i<=end; i++) tot *= v[i];
		return tot;
	}
	char *to_s();
	bool equal(P<Dim> o) {
		if (n!=o->n) return false;
		for (Card i=0; i<n; i++) if (v[i]!=o->v[i]) return false;
		return true;
	}
};
\end class Dim

//****************************************************************
//NumberTypeE is a very small int identifying the type of the (smallest) elements of a grid

#define NT_UNSIGNED (1<<0)
#define NT_FLOAT    (1<<1)
#define NT_UNIMPL   (1<<2)
#define NUMBER_TYPE_LIMITS(T,a,b,c) \
	inline T nt_smallest(T *bogus) {return a;} \
	inline T nt_greatest(T *bogus) {return b;} \
	inline T nt_all_ones(T *bogus) {return c;}

NUMBER_TYPE_LIMITS(  uint8,0,255,255)
NUMBER_TYPE_LIMITS(  int16,-0x8000,0x7fff,-1)
NUMBER_TYPE_LIMITS(  int32,-0x80000000,0x7fffffff,-1)
NUMBER_TYPE_LIMITS(  int64,-0x8000000000000000LL,0x7fffffffffffffffLL,-1)
NUMBER_TYPE_LIMITS(float32,-HUGE_VAL,+HUGE_VAL,(RAISE2("all_ones"),0))
NUMBER_TYPE_LIMITS(float64,-HUGE_VAL,+HUGE_VAL,(RAISE2("all_ones"),0))

#ifdef HAVE_LITE
#define NT_NOTLITE NT_UNIMPL
#define NONLITE(x...)
#else
#define NT_NOTLITE 0
#define NONLITE(x...) x
#endif
#define NUMBER_TYPES(MACRO) \
	MACRO(uint8, 8,NT_UNSIGNED, "u8,b") \
	MACRO(int16,16,0,           "i16,s") \
	MACRO(int32,32,0,           "i32,i") \
	MACRO(int64,64,NT_NOTLITE,  "i64,l") \
	MACRO(float32,32,NT_FLOAT,  "f32,f") \
	MACRO(float64,64,NT_NOTLITE|NT_FLOAT, "f64,d")

enum NumberTypeE {
#define FOO(_sym_,args...) _sym_##_e,
NUMBER_TYPES(FOO)
#undef FOO
number_type_table_end
};

#define FOO(_type_) \
inline NumberTypeE NumberTypeE_type_of(_type_ *x) { \
	return _type_##_e; }
EACH_NUMBER_TYPE(FOO)
#undef FOO

struct NumberType : CObject {
	const char *name;
	int size;
	int flags;
	const char *aliases;
	NumberTypeE index;
	NumberType(const char *name_, int size_, int flags_, const char *aliases_) :
		name(name_), size(size_), flags(flags_), aliases(aliases_) {}
};

NumberTypeE NumberTypeE_find (Symbol sym);

#define TYPESWITCH(T,C,E) switch (T) { \
  case uint8_e:   C(uint8) break;         case int16_e: C(int16) break; \
  case int32_e:   C(int32) break;   NONLITE(case int64_e: C(int64) break;) \
  case float32_e: C(float32) break; NONLITE(case float64_e: C(float64) break;) \
  default: E; RAISE("type '%s' not available here",number_type_table[T].name);}
#define TYPESWITCH_JUSTINT(T,C,E) switch (T) { \
  case uint8_e: C(uint8) break; case int16_e: C(int16) break; \
  case int32_e: C(int32) break;   NONLITE(case int64_e: C(int64) break;) \
  default: E; RAISE("type '%s' not available here",number_type_table[T].name);}

//****************************************************************
//BitPacking objects encapsulate optimised loops of conversion
struct BitPacking;
// those are the types of the optimised loops of conversion 
// inputs are const
struct Packer {
#define FOO(S) void (*as_##S)(BitPacking *self, long n, S *in, uint8 *out);
EACH_INT_TYPE(FOO)
#undef FOO
};
struct Unpacker {
#define FOO(S) void (*as_##S)(BitPacking *self, long n, uint8 *in, S *out);
EACH_INT_TYPE(FOO)
#undef FOO
};

\class BitPacking : CObject {
	Packer   *  packer;
	Unpacker *unpacker;
	unsigned int endian; // 0=big, 1=little, 2=same, 3=different
	int bytes;
	int size;
	uint32 mask[4];
	BitPacking(){::abort();} // don't call, but don't remove. sorry.
	BitPacking(int endian, int bytes, int size, uint32 *mask,
		Packer *packer=0, Unpacker *unpacker=0);
	bool is_le();
	bool eq(BitPacking *o);
	\decl void initialize(Ruby foo1, Ruby foo2, Ruby foo3);
	\decl String   pack2(String ins, String outs=Qnil);
	\decl String unpack2(String ins, String outs=Qnil);
	\decl void     pack3(long n, long inqp, long outqp, NumberTypeE nt);
	\decl void   unpack3(long n, long inqp, long outqp, NumberTypeE nt);
	\decl String to_s();
// main entrances to Packers/Unpackers
	template <class T> void   pack(long n, T *in, uint8 *out);
	template <class T> void unpack(long n, uint8 *in, T *out);
};
\end class

int high_bit(uint32 n);
int  low_bit(uint32 n);
void swap16(long n, uint16 *data);
void swap32(long n, uint32 *data);
void swap64(long n, uint64 *data);
inline void swap_endian(long n,   uint8 *data) {}
inline void swap_endian(long n,   int16 *data) {swap16(n,(uint16 *)data);}
inline void swap_endian(long n,   int32 *data) {swap32(n,(uint32 *)data);}
inline void swap_endian(long n,   int64 *data) {swap64(n,(uint64 *)data);}
inline void swap_endian(long n, float32 *data) {swap32(n,(uint32 *)data);}
inline void swap_endian(long n, float64 *data) {swap64(n,(uint64 *)data);}

//****************************************************************
// Numop objects encapsulate optimised loops of simple operations

enum LeftRight { at_left, at_right };

template <class T>
struct NumopOn {
	// Function Vectorisations
	typedef void (*Map )(         long n, T *as, T  b ); Map  map;
	typedef void (*Zip )(         long n, T *as, T *bs); Zip  zip;
	typedef void (*Fold)(long an, long n, T *as, T *bs); Fold fold;
	typedef void (*Scan)(long an, long n, T *as, T *bs); Scan scan;
	// Algebraic Properties (those involving simply numeric types)
	typedef bool (*AlgebraicCheck)(T x, LeftRight side);
	// neutral: right: forall y {f(x,y)=x}; left: forall x {f(x,y)=y};
	// absorbent: right: exists a forall y {f(x,y)=a}; ...
	void (*neutral)(T *,LeftRight); // default neutral: e.g. 0 for addition, 1 for multiplication
	AlgebraicCheck is_neutral, is_absorbent;
	NumopOn(Map m, Zip z, Fold f, Scan s,
	void (*neu)(T *,LeftRight), AlgebraicCheck n, AlgebraicCheck a) :
		map(m), zip(z), fold(f), scan(s), neutral(neu), is_neutral(n), is_absorbent(a) {}
	NumopOn() : map(0),zip(0),fold(0),scan(0),neutral(0),is_neutral(0),is_absorbent(0) {}
	NumopOn(const NumopOn &z) {
		map=z.map; zip=z.zip; fold=z.fold; scan=z.scan;
		is_neutral = z.is_neutral; neutral = z.neutral;
		is_absorbent = z.is_absorbent; }
};

// semigroup property: associativity: f(a,f(b,c))=f(f(a,b),c)
#define OP_ASSOC (1<<0)
// abelian property: commutativity: f(a,b)=f(b,a)
#define OP_COMM (1<<1)

struct Numop {
	const char *name;
	int flags;
	int size; // numop=1; vecop>1
#define FOO(T) NumopOn<T> on_##T; \
  NumopOn<T> *on(T &foo) { \
    if (!on_##T.map) RAISE("operator %s does not support type "#T,name); \
    return &on_##T;}
EACH_NUMBER_TYPE(FOO)
#undef FOO
	template <class T> inline void map(long n, T *as, T b) {
		on(*as)->map(n,(T *)as,b);}
	template <class T> inline void zip(long n, T *as, T *bs) {
		on(*as)->zip(n,(T *)as,(T *)bs);}
	template <class T> inline void fold(long an, long n, T *as, T *bs) {
		typename NumopOn<T>::Fold f = on(*as)->fold;
		if (!f) RAISE("operator %s does not support fold",name);
		f(an,n,(T *)as,(T *)bs);}
	template <class T> inline void scan(long an, long n, T *as, T *bs) {
		typename NumopOn<T>::Scan f = on(*as)->scan;
		if (!f) RAISE("operator %s does not support scan",name);
		f(an,n,(T *)as,(T *)bs);}
	void map_m  (NumberTypeE nt,          long n, String as, String b);
	void zip_m  (NumberTypeE nt,          long n, String as, String bs);
	void fold_m (NumberTypeE nt, long an, long n, String as, String bs);
	void scan_m (NumberTypeE nt, long an, long n, String as, String bs);

	Numop(Symbol sym_, const char *name_,
#define FOO(T) NumopOn<T> op_##T, 
EACH_NUMBER_TYPE(FOO)
#undef FOO
	int flags_, int size_) : name(name_), flags(flags_), size(size_) {
#define FOO(T) on_##T = op_##T;
EACH_NUMBER_TYPE(FOO)
#undef FOO
	}
};
#ifdef USE_RUBY
inline R::R(Numop *x) {r=ID2SYM(rb_intern(x->name));}
#endif

extern NumberType number_type_table[];

extern Ruby number_type_dict; // GridFlow.@number_type_dict={}
extern Ruby op_dict; // GridFlow.@op_dict={}

#ifdef USE_RUBY
static inline NumberTypeE convert(Ruby x, NumberTypeE *bogus) {return NumberTypeE_find(x);}
#endif

#ifdef USE_RUBY
#ifndef IS_BRIDGE
static Numop *convert(Ruby x, Numop **bogus) {
	Ruby s = rb_hash_aref(rb_ivar_get(mGridFlow,SI(@op_dict)),x);
	if (s==Qnil) {
		s = rb_hash_aref(rb_ivar_get(mGridFlow,SI(@vop_dict)),x);
		if (s==Qnil) RAISE("expected two-input-operator, not %s",
			rb_str_ptr(rb_funcall(x,SI(inspect),0)));
//		post("warning: using vecop !!!");
	}
	return FIX2PTR(Numop,s);
}
#endif
#endif

// ****************************************************************
struct Grid : CObject {
	P<Dim> dim;
	NumberTypeE nt;
	void *data;
	int state; /* 0:borrowing 1:owning -1:expired(TODO) */
	Grid(P<Dim> dim, NumberTypeE nt, bool clear=false) {
		state=1; 
		if (!dim) RAISE("hell");
		init(dim,nt);
		if (clear) {long size = bytes(); CLEAR((char *)data,size);}
	}
	Grid(Ruby x) { state=1; init_from_ruby(x); }
	Grid(int n, Ruby *a, NumberTypeE nt_=int32_e) {state=1; init_from_ruby_list(n,a,nt_);}
	template <class T> Grid(P<Dim> dim, T *data) {
		state=0; this->dim=dim;
		this->nt=NumberTypeE_type_of((T *)0);
		this->data = data;
	}
	// parens are necessary to prevent overflow at 1/16th of the word size (256 megs)
	long bytes() { return dim->prod()*(number_type_table[nt].size/8); }
	P<Dim> to_dim () { return new Dim(dim->prod(),(int32 *)*this); }
#define FOO(T) operator T *() { return (T *)data; }
EACH_NUMBER_TYPE(FOO)
#undef FOO
	Grid *dup () { /* always produce an owning grid even if from a borrowing grid */
		Grid *foo=new Grid(dim,nt);
		memcpy(foo->data,data,bytes());
		return foo;
	}
	~Grid() {if (state==1 && data) free(data);}
private:
	void init(P<Dim> dim, NumberTypeE nt) {
		this->dim = dim;
		this->nt = nt;
		data = 0;
		if (dim) {
			data = memalign(16,bytes()+16);
			if (!data) RAISE("out of memory (?) trying to get %ld bytes",bytes());
		}
		//fprintf(stderr,"rdata=%p data=%p align=%d\n",rdata,data,align);
	}
	void init_from_ruby(Ruby x);
	void init_from_ruby_list(int n, Ruby *a, NumberTypeE nt=int32_e);
};

static inline Grid *convert (Ruby r, Grid **bogus) {return r?new Grid(r):0;}

// DimConstraint interface:
// return if d is acceptable
// else RAISE with proper descriptive message
typedef void (*DimConstraint)(P<Dim> d);

struct PtrGrid : public P<Grid> {
	DimConstraint dc;
	void constrain(DimConstraint dc_) { dc=dc_; }
	P<Grid> next;
	PtrGrid()                  : P<Grid>(), dc(0), next(0) {}
	PtrGrid(const PtrGrid &_p) : P<Grid>(), dc(0), next(0) {dc=_p.dc; p=_p.p; INCR;}
//	PtrGrid(       P<Grid> _p) : P<Grid>(), dc(0), next(0) {dc=_p.dc; p=_p.p; INCR;}
	PtrGrid(         Grid *_p) : P<Grid>(), dc(0), next(0) {
// fprintf(stderr,"&_p=%08x\n",(long)&_p);
// fprintf(stderr,"_p=%08x\n",(long)_p);
          p=_p;
INCR;}
	PtrGrid &operator =(  Grid *_p) {if(dc&&_p)dc(_p->dim);
// fprintf(stderr,"741: this=%08x\n",(long)this);
// fprintf(stderr,"742: dc=%08x &_p=%08x\n",(long)this,&_p); fprintf(stderr,"742: _p=%08x\n",(long)_p);
DECR; p=_p; INCR;
return *this;}
	PtrGrid &operator =(P<Grid> _p) {if(dc&&_p)dc(_p->dim); DECR; p=_p.p; INCR; return *this;}
	PtrGrid &operator =(PtrGrid _p) {if(dc&&_p)dc(_p->dim); DECR; p=_p.p; INCR; return *this;}
};
#undef INCR
#undef DECR

#ifndef IS_BRIDGE
static inline P<Dim> convert(Ruby x, P<Dim> *foo) {
	Grid *d = convert(x,(Grid **)0);
	if (!d) RAISE("urgh");
	if (d->dim->n!=1) RAISE("dimension list must have only one dimension itself");
	return new Dim(d->dim->v[0],(int32 *)(d->data));
}

static inline PtrGrid convert(Ruby x, PtrGrid *foo) {
	return PtrGrid(convert(x,(Grid **)0));
}
#endif

//****************************************************************
// GridInlet represents a grid-aware inlet

// four-part macro for defining the behaviour of a gridinlet in a class
// C:Class I:Inlet
#define GRID_INLET(C,I) \
	template <class T> void C::grinw_##I (GridInlet *in, long n, T *data) { \
		((C*)(in->parent))->grin_##I(in,n,data); } \
	template <class T> void  C::grin_##I (GridInlet *in, long n, T *data) { \
	if (n==-1)
#define GRID_ALLOC  else if (n==-3)
#define GRID_FLOW   else if (n>=0)
#define GRID_FINISH else if (n==-2)
#define GRID_END }

/* macro for defining a gridinlet's behaviour as just storage (no backstore) */
// V is a PtrGrid instance-var
#define GRID_INPUT(C,I,V) \
GRID_INLET(C,I) { V=new Grid(in->dim,NumberTypeE_type_of(data)); } \
GRID_FLOW { COPY((T *)*(V)+in->dex, data, n); } GRID_FINISH

// macro for defining a gridinlet's behaviour as just storage (with backstore)
// V is a PtrGrid instance-var
#define GRID_INPUT2(C,I,V) \
	GRID_INLET(C,I) { \
		if (is_busy_except(in)) { \
			V.next = new Grid(in->dim,NumberTypeE_type_of(data)); \
		} else V=        new Grid(in->dim,NumberTypeE_type_of(data)); \
	} GRID_FLOW { \
		COPY(((T *)*(V.next?V.next.p:&*V.p))+in->dex, data, n); \
	} GRID_FINISH

typedef struct GridInlet GridInlet;
typedef struct GridHandler {
#define FOO(T) \
	void (*flow_##T)(GridInlet *in, long n, T *data); \
	void flow(GridInlet *in, long n, T *data) const {flow_##T(in,n,data);}
EACH_NUMBER_TYPE(FOO)
#undef FOO
} GridHandler;

struct GridObject;
\class GridInlet : CObject {
	GridObject *parent;
	const GridHandler *gh;
private:
	GridObject *sender;
public:
	P<Dim> dim;
	NumberTypeE nt;
	long dex;
private:
	PtrGrid buf;// factor-chunk buffer
	long bufi;   // buffer index: how much of buf is filled
public:
	int mode; // 0=ignore; 4=ro; 6=rw
//	long allocfactor,allocmin,allocmax,allocn;
//	uint8 *alloc;

	GridInlet(GridObject *parent_, const GridHandler *gh_) :
		parent(parent_), gh(gh_), sender(0),
		dim(0), nt(int32_e), dex(0), bufi(0), mode(4) {}
	~GridInlet() {}
	void set_factor(long factor); // obsolete
	void set_chunk(long whichdim);
	void set_mode(int mode_) { mode=mode_; }
	int32 factor() {return buf?buf->dim->prod():1;}
	Ruby begin(int argc, Ruby *argv);
	void finish(); /* i thought this should be private but maybe it shouldn't. */

	// n=-1 is begin, and n=-2 is finish; GF-0.9 may have n=-3 meaning alloc (?).
	template <class T> void flow(int mode, long n, T *data);
	void from_ruby_list(VA, NumberTypeE nt=int32_e) {
		Grid t(argc,argv,nt); from_grid(&t);
	}
	void from_ruby(VA) {Grid t(argv[0]); from_grid(&t);}
	void from_grid(Grid *g);
	bool supports_type(NumberTypeE nt);
private:
	template <class T> void from_grid2(Grid *g, T foo);
};
\end class GridInlet

//****************************************************************
// for use by source_filter.rb ONLY (for \grin and \classinfo)
#ifndef HAVE_LITE
#define GRIN(TB,TS,TI,TL,TF,TD,TR) {TB,TS,TI,TL,TF,TD}
#else
#define GRIN(TB,TS,TI,TL,TF,TD,TR) {TB,TS,TI,TF}
#endif // HAVE_LITE
struct FClass {
	void *(*allocator)(); // returns a new C++ object
	void (*startup)(Ruby rself); // initializer for the Ruby class
	const char *name; // C++/Ruby name (not PD name)
	int methodsn; MethodDecl *methods; // C++ -> Ruby methods
};

//****************************************************************
// GridOutlet represents a grid-aware outlet
\class GridOutlet : CObject {
// number of (minimum,maximum) BYTES to send at once
// starting with version 0.8, this is amount of BYTES, not amount of NUMBERS.
	static const long MIN_PACKET_SIZE = 1<<8;
	static const long MAX_PACKET_SIZE = 1<<12;
// those are set only once
	GridObject *parent; // not a P<> because of circular refs
	P<Dim> dim; // dimensions of the grid being sent
	NumberTypeE nt;
	std::vector<GridInlet *> inlets; // which inlets are we connected to
// those are updated during transmission
	long dex;  // how many numbers were already sent in this connection

	GridOutlet(GridObject *parent_, int woutlet, P<Dim> dim_, NumberTypeE nt_=int32_e) :
	parent(parent_), dim(dim_), nt(nt_), dex(0), frozen(false), bufi(0) {
		//int ntsz = number_type_table[nt].size;
		buf=new Grid(new Dim(MAX_PACKET_SIZE/*/ntsz*/), nt);
		begin(woutlet,dim,nt);
	}
	~GridOutlet() {}
	void callback(GridInlet *in);

	// send/send_direct: data belongs to caller, may be stack-allocated,
	// receiver doesn't modify the data; in send(), there is buffering;
	// in send_direct(), there is not. When switching from buffered to
	// unbuffered mode, flush() must be called
	template <class T> void send(long n, T *data);
	void flush(); // goes with send();

	// give: data must be dynamically allocated as a whole: the data
	// will be deleted eventually, and should not be used by the caller
	// beyond the call to give().
	template <class T> void give(long n, T *data);

	// third way to send (upcoming, in GF-0.8.??) is called "ask".
	template <class T> void ask(int &n, T * &data, long factor, long min, long max);

private:
	bool frozen; // is the "begin" phase finished?
	PtrGrid buf; // temporary buffer
	long bufi; // number of bytes used in the buffer
	void begin(int woutlet, P<Dim> dim, NumberTypeE nt=int32_e);
	template <class T> void send_direct(long n, T *data);
	void finish() {
		flush();
		for (uint32 i=0; i<inlets.size(); i++) inlets[i]->finish();
		dim=0;
	}
};
\end class GridOutlet

//****************************************************************

struct BFProxy;
struct BFObject : t_object {
#ifdef HAVE_GEM
	void *gemself; // a CPPExtern * for binary-compat with GEM's Obj_header class
#endif
	Ruby rself;
	int nin,nout;   // per object settings (not class)
	BFProxy  **in;  // direct access to  inlets (not linked lists)
	t_outlet **out; // direct access to outlets (not linked lists)
	t_canvas *mom;
	void  ninlets_set(int n);
	void noutlets_set(int n);
};

// represents objects that have inlets/outlets
\class FObject : CObject {
	BFObject *bself; // point to PD peer
	FObject() : bself(0) {}
	\decl void send_in (...);
	\decl void send_out (...);
	template <class T> void send_out(int outlet, int argc, T *argv) {
		t_atom foo[argc];
		for (int i=0; i<argc; i++) SETFLOAT(&foo[i],argv[i]);
		outlet_list(bself->out[outlet],&s_list,argc,foo);
	}
	\decl void delete_m ();
};
\end class FObject

\class GridObject : FObject {
	std::vector<P<GridInlet> > in;
	P<GridOutlet> out;
	// Make sure you distinguish #close/#delete, and C++'s delete. The first
	// two are quite equivalent and should never make an object "crashable".
	// C++'s delete is called by Ruby's garbage collector or by PureData's delete.
	GridObject() {}
	~GridObject() {}
	bool is_busy_except(P<GridInlet> gin) {
		for (uint32 i=0; i<in.size(); i++)
			if (in[i] && in[i]!=gin && in[i]->dim) return true;
		return false;
	}
	\decl Ruby method_missing(...);
	\decl Array inlet_dim(int inln);
	\decl Symbol inlet_nt(int inln);
	\decl void inlet_set_factor(int inln, long factor);
	\decl void inlet_set_chunk(int inln, long chunk);
	\decl void send_out_grid_begin (int outlet, Array dim,        NumberTypeE nt=int32_e);
	\decl void send_out_grid_flow  (int outlet, String buf,       NumberTypeE nt=int32_e);
	\decl void send_out_grid_flow_3(int outlet, long n, long buf, NumberTypeE nt=int32_e);
};
\end class GridObject

uint64 gf_timeofday();
extern "C" void Init_gridflow ();
extern Numop *op_add,*op_sub,*op_mul,*op_div,*op_mod,*op_shl,*op_and,*op_put;

#define ARGS(OBJ) rb_str_ptr(rb_funcall(OBJ->rself,SI(args),0))
#define NOTEMPTY(_a_) if (!(_a_)) RAISE("in [%s], '%s' is empty",ARGS(this), #_a_);
#define SAME_TYPE(_a_,_b_) \
	if ((_a_)->nt != (_b_)->nt) RAISE("%s: same type please (%s has %s; %s has %s)", ARGS(this), \
		#_a_, number_type_table[(_a_)->nt].name, \
		#_b_, number_type_table[(_b_)->nt].name);
static void SAME_DIM(int n, P<Dim> a, int ai, P<Dim> b, int bi) {
	if (ai+n > a->n) RAISE("left hand: not enough dimensions");
	if (bi+n > b->n) RAISE("right hand: not enough dimensions");
	for (int i=0; i<n; i++) {
		if (a->v[ai+i] != b->v[bi+i]) {
			RAISE("mismatch: left dim #%d is %d, right dim #%d is %d",
				ai+i, a->v[ai+i],
				bi+i, b->v[bi+i]);}}}

\class Pointer : CObject {
	void *p;
	Pointer() { RAISE("trying to construct a (ruby) Pointer without an argument"); }
	Pointer(void *_p) : p(_p) {}
	\decl Ruby ptr ();
};
\end class Pointer

void suffixes_are (const char *name, const char *suffixes);

#define install(name,ins,outs) rb_funcall(rself,SI(install),3,rb_str_new2(name),INT2NUM(ins),INT2NUM(outs))
#define install_format(name,mode,suffixes) do {install(name,1,1); suffixes_are(name,suffixes);} while(0)

#define SUPER rb_call_super(argc,argv);

\class Format : GridObject {
	int mode;
	int fd;
	FILE *f;
	NumberTypeE cast;
	long frame;
	Format() : mode(0), fd(-1), f(0), cast(int32_e), frame(0) {}
	\decl void initialize (t_symbol *mode, ...);
	\decl 0 open (String mode, String filename);
	\decl 0 close ();
	\decl 0 cast (NumberTypeE nt);
	\decl 0 seek(int frame);
	\decl 0 rewind ();
};

#endif // __GF_GRID_H
