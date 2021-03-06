/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

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

#ifndef __GRIDFLOW_H
#define __GRIDFLOW_H

#define GF_VERSION "9.14"
#define GF_VERSION_A  9
#define GF_VERSION_B 14
#define GF_VERSION_C  0

#include "bundled/m_pd.h"
#include "config.h"
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <ext/hash_map>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#if defined(__APPLE__) || defined(__WIN32__)
static inline void *memalign (size_t a, size_t n) {return malloc(n);}
#else
#include <malloc.h>
#endif

using std::string;
using std::map;
using __gnu_cxx::hash_map; // no match for call to (const __gnu_cxx::hash<string>) (const std::string &)
using std::vector;
using std::ostream;
using std::ostringstream;
using std::pair;
typedef pair<int,t_symbol *> insel; // inlet-selector compound

#define a_index    a_w.w_index
#define a_float    a_w.w_float
#define a_symbol   a_w.w_symbol
#define a_gpointer a_w.w_gpointer

#ifndef PD_BLOBS
#define A_BLOB    t_atomtype(12) /* t_blob * */
typedef struct _blob t_blob;
#endif
#define A_LIST    t_atomtype(13) /* (t_binbuf *) */
#define A_GRID    t_atomtype(14) /* (Grid *)    */
#define A_GRIDOUT t_atomtype(15) /* (GridOut *) */

string atomtype_to_s (t_atomtype t);

#define gensym(s) gensym(const_cast<char *>(s))
#define sys_vgui(FMT,ARGS...) sys_vgui(const_cast<char *>(FMT),ARGS)
#define sys_gui(s) sys_gui(const_cast<char *>(s))

#ifdef DES_BUGS
#define DEF_IN(CONTEXT) static const char *context = CONTEXT; context=context; post("> self=%08x %s",long(self),context);
#else
#define DEF_IN(CONTEXT) static const char *context = CONTEXT; context=context;
#endif
#define DEF_OUT

/*
#ifdef __WIN32__
#ifdef LIBGF
#define DLL __declspec(dllexport) extern
#else
#define DLL __declspec(dllimport) extern
#endif
#else
#define DLL extern
#endif
*/

typedef t_binbuf t_list;
struct Grid;
struct GridOut;

t_list *list_new (int argc, t_atom *argv);
void list_free (t_list *self);

typedef char       int8; typedef unsigned char      uint8;
typedef short     int16; typedef unsigned short     uint16;
typedef int       int32; typedef unsigned int       uint32;
typedef long long int64; typedef unsigned long long uint64;
typedef float   float32;
typedef double  float64;

// three-way comparison (T is assumed Comparable)
template <class T> static inline T cmp(T a, T b) {return a<b ? -1 : a>b;}

// a remainder function such that div2(a,b)*b+mod(a,b) = a and for which mod(a,b) is in [0;b) or (b;0].
// in contrast to C-language builtin a%b, this one has uniform behaviour around zero,
// that is, the same as around any other whole number.
static inline int mod(int a, int b) {int c=a%b; c+=b&-(c&&(a<0)^(b<0)); return c;}

// counterpart of mod(a,b), just like a/b and a%b are counterparts
static inline int div2(int a, int b) {return (a/b)-((a<0)&&!!(a%b));}

static inline int32   gf_abs(  int32 a) {return a>0?a:-a;}
static inline int64   gf_abs(  int64 a) {return a>0?a:-a;}
static inline float32 gf_abs(float32 a) {return fabs(a);}
static inline float64 gf_abs(float64 a) {return fabs(a);}

// integer powers in log(b) time. T is assumed Integer
template <class T> static inline T ipow(T a, T b) {T r=1; if (b<0) return 0; for(;;) {if (b&1) r*=a; b>>=1; if (!b) return r; a*=a;}}
static inline float32 ipow(float32 a, float32 b) {return pow(a,b);}
static inline float64 ipow(float64 a, float64 b) {return pow(a,b);}

#undef min
#undef max
// minimum/maximum functions; T is assumed to be Comparable
template <class T> static inline T min(T a, T b) {return a<b?a:b;}
template <class T> static inline T max(T a, T b) {return a>b?a:b;}
template <class T> static inline T clip(T a, T lower, T upper) {return a<lower?lower:a>upper?upper:a;}
//template <class T> inline T min(T a, T b) { T c = (a-b)>>31; return (a&c)|(b&~c); }

// greatest common divisor, by euclid's algorithm
// this runs in log(a+b) number operations
template <class T> static T gcd (T a, T b) {
	while (b) {T c=mod(a,b); a=b; b=c;}
	return a;
}

// greatest common divisor, the binary algorithm. haven't tried yet.
template <class T> static T gcd2 (T a, T b) {
	int s=0;
	while (((a|b)&1)==0) { a>>=1; b>>=1; s++; }
	while (a) {
		if ((a&1)==0) a>>=1;
		else if ((b&1)==0) b>>=1;
		else {T t=abs(a-b); if (a<b) b=t; else a=t;}
	}
	return b<<s;
}

// least common multiple; this runs in log(a+b) like gcd.
template <class T> static inline T lcm (T a, T b) {return (int64)a*b/gcd(a,b);}

// returns the position (0..63) of highest bit set in a word, or 0 if none.
#define Z(N) if ((x>>N)&(((typeof(x))1<<N)-1)) { x>>=N; i+=N; }
static int highest_bit(uint8  x) {int i=0;              Z(4)Z(2)Z(1)return i;}
static int highest_bit(uint16 x) {int i=0;          Z(8)Z(4)Z(2)Z(1)return i;}
static int highest_bit(uint32 x) {int i=0;     Z(16)Z(8)Z(4)Z(2)Z(1)return i;}
static int highest_bit(uint64 x) {int i=0;Z(32)Z(16)Z(8)Z(4)Z(2)Z(1)return i;}
#undef Z
// returns the position (0..63) of lowest bit set in a word, or 0 if none.
template <class T> static int lowest_bit(T n) { return highest_bit((~n+1)&n); }

static double drand() { return 1.0*rand()/(RAND_MAX+1.0); }

// is little-endian
static inline bool is_le() {int x=1; return ((char *)&x)[0];}

static inline uint64 rdtsc()
#if defined(HAVE_PENTIUM)
{uint64 x; __asm__ volatile (".byte 0x0f, 0x31":"=A"(x)); return x;}
#else
{return 0;}
#endif

#ifdef HAVE_LITE
#define EACH_INT_TYPE(MACRO) MACRO(uint8) MACRO(int16) MACRO(int32)
#define EACH_FLOAT_TYPE(MACRO) MACRO(float32)
#else
#define EACH_INT_TYPE(MACRO) MACRO(uint8) MACRO(int16) MACRO(int32) MACRO(int64)
#define EACH_FLOAT_TYPE(MACRO) MACRO(float32) MACRO(float64)
#endif
#define EACH_NUMBER_TYPE(MACRO) EACH_INT_TYPE(MACRO) EACH_FLOAT_TYPE(MACRO)

// note: loop unrolling macros assume N!=0
// btw this may cause alignment problems when 8 does not divide N
#define UNROLL_8(MACRO,N,PTR,ARGS...) \
	int n__=(-N)&7; PTR-=n__; N+=n__; \
	switch (n__) { start: \
		case 0:MACRO(0); case 1:MACRO(1); case 2:MACRO(2); case 3:MACRO(3); \
		case 4:MACRO(4); case 5:MACRO(5); case 6:MACRO(6); case 7:MACRO(7); \
		PTR+=8; N-=8; ARGS; if (N) goto start; }
#define UNROLL_4(MACRO,N,PTR,ARGS...) \
	int n__=(-N)&3; PTR-=n__; N+=n__; \
	switch (n__) { start: \
		case 0:MACRO(0); case 1:MACRO(1); case 2:MACRO(2); case 3:MACRO(3); \
		PTR+=4; N-=4; ARGS; if (N) goto start; }

struct FObject;
struct BFObject;
struct Barf {
  string text;
  Barf(                                              const char *fmt, ...) __attribute__((format(printf,2,3)));
  Barf(const char *file, int line, const char *func, const char *fmt, ...) __attribute__((format(printf,5,6)));
  void error(BFObject *self, int winlet, t_symbol *selector);
  void error(t_symbol *s, int argc, t_atom *argv);
  ~Barf() {}
};

#define NEWBUF(T,N) (new T[N])
#define DELBUF(A) (delete[] A)

#ifdef __WIN32__
#undef INT
#define random rand
#undef send
#undef close
#define sigjmp_buf jmp_buf
#define siglongjmp longjmp
#endif

extern char *short_backtrace (int start=3, int end=4);

//#define _L_ post("%s:%d in %s",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define _L_ fprintf(stderr,"%s:%d in %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define RAISE(args...) throw Barf(__FILE__,__LINE__,__PRETTY_FUNCTION__,args)
#define VA int argc, t_atom2 *argv
// returns the size of a statically defined array
#define COUNT(_array_) ((int)(sizeof(_array_) / sizeof((_array_)[0])))
#define WATCH(n,ar) { \
	char foo[16*1024], *p=foo; p += sprintf(p,"%s: ",#ar); \
	for (int q=0; q<n; q++) p += sprintf(p,"%lld ",(long long)ar[q]); \
	post("%s",foo);}

#define  EXACTARGS(FUDGE,N      ) if (argc!=(N)             ) RAISE("got %d args instead of %d"                     ,argc-(FUDGE),(N)-(FUDGE));
#define    MINARGS(FUDGE,MIN    ) if (argc<(MIN)            ) RAISE("got %d args instead of at least %d"            ,argc-(FUDGE),(MIN)-(FUDGE));
#define MINMAXARGS(FUDGE,MIN,MAX) if (argc<(MIN)||argc>(MAX)) RAISE("got %d args instead of at least %d, at most %d",argc-(FUDGE),(MIN)-(FUDGE),(MAX)-(FUDGE));

#define CLASSINFO(THISCLASS) \
	static void THISCLASS##_startup (FClass *fclass); \
	static FObject *THISCLASS##_allocator (BFObject *bself, MESSAGE) {return new THISCLASS(bself,sel,argc,argv);} \
	FClass ci##THISCLASS = {THISCLASS##_allocator,THISCLASS##_startup};

//****************************************************************

struct FObject;
struct t_atom2;
typedef void (*FMethod)(FObject *, int, t_atom2 *);

#define BUILTIN_SYMBOLS(MACRO) MACRO(grid,"grid") MACRO(sharp,"#") MACRO(comma,",") MACRO(semi,";")
	
#define FOO(_sym_,_str_) extern t_symbol *s_##_sym_;
BUILTIN_SYMBOLS(FOO)
#undef FOO

struct Numop;
struct Pointer;
#define TO(T,x) convert(x,(T*)0)

// trick to be able to define methods in t_atom
struct t_atom2 : t_atom {
	// does not do recursive comparison of lists.
	bool operator == (const t_atom2 &b);
	bool operator != (const t_atom2 &b) {return !(*this==b);}
	bool operator == (t_symbol *b) {return this->a_type==A_SYMBOL && this->a_symbol==b;}
	bool operator != (t_symbol *b) {return !(*this==b);}
	operator float32 () const {if (a_type!=A_FLOAT) RAISE("expected float, got %s",to_s().data()); return a_float;}
#define TYPECASTER(T,A,B) operator T () const { \
  float f = round(float32(*this)); if (f<A || f>=B) RAISE("value %f is out of range",f); return (T)f;}
	TYPECASTER(   bool,           0  ,          2  )
	TYPECASTER(  uint8,           0  ,      0x100  )
	TYPECASTER(  int16,     -0x8000  ,     0x8000  )
	TYPECASTER( uint16,           0  ,    0x10000  )
	TYPECASTER(  int32, -0x80000000LL, 0x80000000LL)
	TYPECASTER( uint32,           0  ,0x100000000LL)
#undef TYPECASTER
	operator  uint64 () const {if (a_type!=A_FLOAT) RAISE("expected float, got %s",to_s().data()); return (uint64)round(a_float);}
	operator   int64 () const {if (a_type!=A_FLOAT) RAISE("expected float, got %s",to_s().data()); return  (int64)round(a_float);}
	operator float64 () const {if (a_type!=A_FLOAT) RAISE("expected float, got %s",to_s().data()); return               a_float ;}

#define TYPECASTER2(T,A,B,C) operator T () const {if (a_type!=A) RAISE("expected "B", got %s",to_s().data()); return C;}
	TYPECASTER2(string      ,A_SYMBOL ,"symbol"     ,string(a_symbol->s_name))
	//TYPECASTER2(const char *,A_SYMBOL ,"symbol"     ,       a_symbol->s_name ) //"ambiguous" ?
	TYPECASTER2(t_symbol   *,A_SYMBOL ,"symbol"     ,       a_symbol         )
	TYPECASTER2(void       *,A_POINTER,"pointer"    ,              a_gpointer)
	TYPECASTER2(t_binbuf   *,A_LIST   ,"nested list",  (t_binbuf *)a_gpointer)
	TYPECASTER2(Grid       *,A_GRID   ,"grid"       ,      (Grid *)a_gpointer)
	TYPECASTER2(GridOut    *,A_GRIDOUT,"grid outlet",   (GridOut *)a_gpointer)
#undef TYPECASTER2

	template <class T> t_atom2 &operator = (T value) {set_atom(this,value); return *this;};
	template <class T> t_atom2             (T value) {set_atom(this,value);              };
	t_atom2 ()                            {a_type = A_NULL; a_gpointer = 0;}
	t_atom2 (t_atomtype t, int i)         {a_type = t; a_index    = i;}
	t_atom2 (t_atomtype t, float f)       {a_type = t; a_float    = f;}
	t_atom2 (t_atomtype t, t_symbol *s)   {a_type = t; a_symbol   = s;}
	t_atom2 (t_atomtype t, t_gpointer *p) {a_type = t; a_gpointer = p;}
	string to_s () const;
};

template <class T> static inline T convert(const t_atom2 &x, T *foo) {return (T)x;}
static inline const char *convert(const t_atom2 &x, const char **foo) {return ((t_symbol *)x)->s_name;}

void gfmemcopy(uint8 *out, const uint8 *in, long n);
template <class T> inline void COPY  (T *dest, const T *src, long n=1) {gfmemcopy((uint8*)dest,(const uint8*)src,n*sizeof(T));}
template <class T> inline void CLEAR (T *dest,               long n=1) {memset(dest,0,n*sizeof(T));}
template <class T> static void memswap (T *a,  T *b,         long n=1) {T c[n]; COPY(c,a,n); COPY(a,b,n); COPY(b,c,n);}

//****************************************************************

struct CObject {
	int32 refcount;
	CObject() : refcount(0) {}
	virtual ~CObject() {}
};

#undef check

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
NUMBER_TYPE_LIMITS(float32,-HUGE_VAL,+HUGE_VAL,(RAISE("all_ones"),0))
NUMBER_TYPE_LIMITS(float64,-HUGE_VAL,+HUGE_VAL,(RAISE("all_ones"),0))

#ifdef HAVE_LITE
#define NT_NOTLITE NT_UNIMPL
#define NONLITE(x...)
#else
#define NT_NOTLITE 0
#define NONLITE(x...) x
#endif
#define NUMBER_TYPES(MACRO) \
	MACRO(b,uint8, 8,NT_UNSIGNED) \
	MACRO(s,int16,16,0          ) \
	MACRO(i,int32,32,0          ) \
	MACRO(l,int64,64,NT_NOTLITE ) \
	MACRO(f,float32,32,NT_FLOAT ) \
	MACRO(d,float64,64,NT_NOTLITE|NT_FLOAT)

enum NumberTypeE {
#define FOO(ABBR,SYM,SIZE,FLAGS) SYM##_e,
NUMBER_TYPES(FOO)
#undef FOO
number_type_table_end
};

#define FOO(_type_) \
inline NumberTypeE NumberTypeE_type_of(_type_ *x) {return _type_##_e;}
EACH_NUMBER_TYPE(FOO)
#undef FOO

struct NumberType {
	const char *alias;
	const char *name;
	int size;
	int flags;
	NumberTypeE index;
	NumberType(const char *alias_, const char *name_, int size_, int flags_) :
		alias(alias_), name(name_), size(size_), flags(flags_) {}
};

NumberTypeE NumberTypeE_find (string sym);
NumberTypeE NumberTypeE_find (const t_atom &sym);

#define TYPESWITCH(T,C,E) switch (T) { \
  case uint8_e:   C(uint8)   break;         case   int16_e: C(int16)   break; \
  case int32_e:   C(int32)   break; NONLITE(case   int64_e: C(int64)   break;) \
  case float32_e: C(float32) break; NONLITE(case float64_e: C(float64) break;) \
  default: E; RAISE("type '%s' not available here",number_type_table[T].name);}
#define TYPESWITCH_JUSTINT(T,C,E) switch (T) { \
  case uint8_e: C(uint8) break;           case int16_e: C(int16) break; \
  case int32_e: C(int32) break;   NONLITE(case int64_e: C(int64) break;) \
  default: E; RAISE("type '%s' not available here",number_type_table[T].name);}

//****************************************************************
// a Dim is a list of dimensions that describe the shape of a grid
struct Dim {
	//typedef intptr_t T; // this is signed, to match the fact that most int types are signed in Grid.
	typedef int32 T;
	static const int MAX_DIM=15; // maximum number of dimensions in a grid
	T n;
	T v[MAX_DIM]; // real stuff
	void check(); // test invariants
	Dim(T n, T *v){this->n=n; COPY(this->v,v,n); check();}
	Dim()            {n=0;                     check();}
	Dim(T a)         {n=1;v[0]=a;              check();}
	Dim(T a,T b)     {n=2;v[0]=a;v[1]=b;       check();}
	Dim(T a,T b,T c) {n=3;v[0]=a;v[1]=b;v[2]=c;check();}
	Dim &operator = (const Dim &a) {n=a.n; COPY(&*v,&*a.v,n); check(); return *this;}
	Dim(Dim *a, Dim *b, Dim *c=0) {
		n=a->n+b->n; if(c) n+=c->n;
		if (n>Dim::MAX_DIM) RAISE("too many dims");
		COPY(v     ,a->v,a->n);
		COPY(v+a->n,b->v,b->n);
		if(c) COPY(v+a->n+b->n,c->v,c->n);
	}
	T count() {return n;}
	T operator[](T i) const {return v[i];}
/*	Dim *range(T i, T j) {return new Dim(...);} */
	T prod(T start=0, T end=-1) const {
		if (start<0) start+=n;
		if (end  <0) end  +=n;
		T tot=1;
		for (T i=start; i<=end; i++) tot *= v[i];
		return tot;
	}
	char *to_s() const; // should be string
	bool operator==(const Dim &o) const {
		if (n!=o.n) return false;
		for (T i=0; i<n; i++) if (v[i]!=o[i]) return false;
		return true;
	}
	bool operator!=(const Dim &o) const {return !operator==(o);}
};

//****************************************************************
// GridConstraint interface: (only a GRID_BEGIN time constraint)
// return if d and nt are acceptable, else RAISE with proper descriptive message
#define CONSTRAINT(NAME) void NAME (const Dim &d, NumberTypeE nt)
typedef CONSTRAINT((*GridConstraint));
CONSTRAINT(expect_any);

//template <class T> class P : T * {};
//a reference counting pointer class
//note: T <= CObject
//used mostly as P<Grid>, P<BitPacking>
template <class T> struct P {
public:
	//#define INCR if (p) {p->refcount++; if (p->refcount>1) post("refcount++ to %d at %s",p->refcount,short_backtrace(3,6));}
	//#define DECR if (p) {if (p->refcount>1) post("refcount-- from %d at %s",p->refcount,short_backtrace()); p->refcount--; if (!p->refcount) delete p;}
	#define INCR if (p) {p->refcount++;}
	#define DECR if (p) {p->refcount--; if (!p->refcount) delete p;}
	T *p;
	GridConstraint dc; // is not really related to the general P template, but it saves some trouble later
	void but (GridConstraint dc) {this->dc=dc;}
	P()               {dc=0; p=0;}
	P(T *_p)          {dc=0; p=_p  ; INCR;}
	P(const P<T> &_p) {dc=0; p=_p.p; INCR;}
	P<T> &operator = (T *  _p) {DECR; p=_p;   INCR; return *this;}
	P<T> &operator = (P<T> _p) {DECR; p=_p.p; INCR; return *this;}
	bool operator == (P<T> _p) {return p==_p.p;}
	bool operator != (P<T> _p) {return p!=_p.p;}
	~P() {DECR;}
	bool operator !() {return  !p;}
	operator bool()   {return !!p;}
	T &operator *()   {return  *p;}
	T *operator ->()  {return   p;}
	operator T *() {return p;}
	#undef INCR
	#undef DECR
};

//****************************************************************
//BitPacking objects encapsulate optimised loops of bitfield conversion (mostly for I/O)
struct BitPacking;
// those are the types of the optimised loops of conversion; inputs are const
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

struct BitPacking : CObject {
	Packer   *  packer;
	Unpacker *unpacker;
	unsigned int endian; // 0=big, 1=little, 2=same, 3=different
	int bytes;
	int size;
	uint32 mask[4];
	BitPacking(){::abort();} // don't call, but don't remove. sorry.
	BitPacking(int endian, int bytes, int size, uint32 *mask, Packer *packer=0, Unpacker *unpacker=0);
	bool is_le();
	bool eq(BitPacking *o);
// main entrances to Packers/Unpackers
	#define FOO(A) case A##_e:   packer->as_##A(this,n,(A *)in,out); break;
	template <class T> void   pack(long n, T *in, uint8 *out) {
		switch (NumberTypeE_type_of( in)) {FOO(uint8) FOO(int16) FOO(int32) default: RAISE("argh");}}
	#undef FOO
	#define FOO(A) case A##_e: unpacker->as_##A(this,n,in,(A *)out); break;
	template <class T> void unpack(long n, uint8 *in, T *out) {
		switch (NumberTypeE_type_of(out)) {FOO(uint8) FOO(int16) FOO(int32) default: RAISE("argh");}}
	#undef FOO
};

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

struct Numop {
	const char *name;
	t_symbol *sym;
	int size; // 1 means regular numop; more than 1 means vecop
	virtual int arity ();
	virtual ~Numop() {} // never used, only to shut up warning
};

struct Numop1 : Numop {
	template <class T> struct On {
		// Function Vectorisations
		typedef void (*Map)(         long n, T *as       ); Map map;
		On(Map m) : map(m) {}
		On()      : map(0) {}
		On(const On &z) {map=z.map;}
	};
	virtual int arity () {return 1;}
	#define FOO(T) On<T> on_##T; On<T> *on(T &foo) { \
		if (!on_##T.map) RAISE("operator %s does not support type "#T,name); else return &on_##T;}
	EACH_NUMBER_TYPE(FOO)
	#undef FOO
	template <class T> inline void map(long n, T *as) {on(*as)->map(n,(T *)as);}
	Numop1(const char *name_,
		#define FOO(T) On<T> op_##T, 
		EACH_NUMBER_TYPE(FOO)
		#undef FOO
	int size_) {
		name=name_; size=size_;
		#define FOO(T) on_##T = op_##T;
		EACH_NUMBER_TYPE(FOO)
		#undef FOO
		sym=gensym((char *)name);
	}
};

struct Numop2 : Numop {
	template <class T> struct On {
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
		On(Map m, Zip z, Fold f, Scan s,
		void (*neu)(T *,LeftRight), AlgebraicCheck n, AlgebraicCheck a) :
			map(m),zip(z),fold(f),scan(s),neutral(neu),is_neutral(n),is_absorbent(a) {}
		On() : map(0),zip(0),fold(0),scan(0),neutral(0)  ,is_neutral(0),is_absorbent(0) {}
		On(const On &z) {
			map=z.map; zip=z.zip; fold=z.fold; scan=z.scan;
			is_neutral = z.is_neutral; neutral = z.neutral;
			is_absorbent = z.is_absorbent; }
	};
	virtual int arity () {return 2;}
	int flags;
		#define OP_ASSOC (1<<0) /* semigroup property: associativity: f(a,f(b,c))=f(f(a,b),c) */
		#define OP_COMM (1<<1)  /* abelian property: commutativity: f(a,b)=f(b,a) */
	#define FOO(T) On<T> on_##T; On<T> *on(T &foo) { \
		if (!on_##T.map) RAISE("operator %s does not support type "#T,name); else return &on_##T;}
	EACH_NUMBER_TYPE(FOO)
	#undef FOO
	template <class T> inline void map(long n, T *as, T b)   {on(*as)->map(n,(T *)as,     b );}
	template <class T> inline void zip(long n, T *as, T *bs) {on(*as)->zip(n,(T *)as,(T *)bs);}
	template <class T> inline void fold(long an, long n, T *as, T *bs) {typename On<T>::Fold f = on(*as)->fold;
		if (f) f(an,n,as,bs); else RAISE("operator %s does not support fold",name);}
	template <class T> inline void scan(long an, long n, T *as, T *bs) {typename On<T>::Scan f = on(*as)->scan;
		if (f) f(an,n,as,bs); else RAISE("operator %s does not support scan",name);}

	Numop2(const char *name_,
		#define FOO(T) On<T> op_##T, 
		EACH_NUMBER_TYPE(FOO)
		#undef FOO
	int flags_, int size_) {
		name=name_; size=size_; flags=flags_;
		#define FOO(T) on_##T = op_##T;
		EACH_NUMBER_TYPE(FOO)
		#undef FOO
		sym=gensym((char *)name);
	}
};

namespace __gnu_cxx {
	template <> struct hash<t_symbol*> {
		size_t operator()(const t_symbol *a) const {size_t b = size_t(a); return b^(b>>2);}
	};
	template <> struct hash<pair<int,t_symbol *> > {
		size_t operator()(const pair<int, t_symbol *>a) const {size_t b = size_t(a.second); return a.first^b^(b>>2);}
	};
}
extern NumberType number_type_table[];
extern map<t_symbol *,NumberType *> number_type_dict;
//extern map<t_symbol *,Numop *> op_dict;
extern hash_map<t_symbol *,Numop *> op_dict;

static inline NumberTypeE convert(const t_atom2 &x, NumberTypeE *bogus) {
	if (x.a_type!=A_SYMBOL) RAISE("expected number-type, got %s",x.to_s().data()); return NumberTypeE_find(string(x.a_symbol->s_name));}

       Numop  *convert(const t_atom2 &x, Numop  **bogus);
static Numop1 *convert(const t_atom2 &x, Numop1 **bogus) {
	Numop *nu = convert(x,(Numop **)0); if (nu->arity()!=1) RAISE("this Numop doesn't have one argument" );
	return (Numop1 *)nu;}
static Numop2 *convert(const t_atom2 &x, Numop2 **bogus) {
	Numop *nu = convert(x,(Numop **)0); if (nu->arity()!=2) RAISE("this Numop doesn't have two arguments");
	return (Numop2 *)nu;}

// ****************************************************************
struct Grid : CObject {
	Dim dim;
	NumberTypeE nt;
	void *data;
	int state; /* 0:borrowing 1:owning -1:expired(TODO) */
	P<Grid> next; /* used for reentrancy */
	Grid(const Dim &dim=Dim(), NumberTypeE nt=int32_e, bool clear=false) {
		data=0; state=1;
		init(dim,nt);
		if (clear) CLEAR((char *)data,bytes());
	}
	Grid(const t_atom &x) {data=0; state=1; init_from_atom(x);}
	Grid(int n, t_atom2 *a, NumberTypeE nt_=int32_e) {data=0; state=1; init_from_list(n,a,nt_);}
	Grid(int n, t_atom  *a, NumberTypeE nt_=int32_e) {data=0; state=1; init_from_list(n,a,nt_);}
	static Grid *from_msg(int n, t_atom2 *a) {Grid *self = new Grid; self->init_from_msg(n,a); return self;}
	static Grid *from_msg(int n, t_atom  *a) {Grid *self = new Grid; self->init_from_msg(n,a); return self;}
	template <class T> Grid(const Dim &dim, T *data) {
		state=0; this->dim=dim;
		this->nt=NumberTypeE_type_of((T *)0);
		this->data = data;
	}
	// parens are necessary to prevent overflow at 1/16th of the word size (256 megs)
	long bytes() {return dim.prod()*(number_type_table[nt].size/8);}
	Dim to_dim() {return Dim(dim.prod(),(int32 *)*this);}
#define FOO(T) operator T *() { \
	/*NumberTypeE tnt = NumberTypeE_type_of((T *)0);*/\
	/*if (nt!=tnt) post("cast from %s to %s",number_type_table[nt].name, number_type_table[tnt].name);*/ \
	return (T *)data;}
EACH_NUMBER_TYPE(FOO)
#undef FOO
	Grid *dup () { /* always produce an owning grid even if from a borrowing grid */
		Grid *foo=new Grid(dim,nt);
		memcpy(foo->data,data,bytes());
		return foo;
	}
	~Grid() {if (state==1 && data) free(data);}
	void init(const Dim &dim=Dim(), NumberTypeE nt=int32_e) {
		this->dim = dim;
		this->nt = nt;
		if (data) free(data);
		data = memalign(16,bytes()+16);
		if (!data) RAISE("out of memory (?) trying to get %ld bytes",bytes());
		//fprintf(stderr,"rdata=%p data=%p align=%d\n",rdata,data,align);
	}
	void init_from_atom(const t_atom &x);
	void init_from_list(int n, t_atom *a, NumberTypeE nt=int32_e);
	void init_from_msg (int n, t_atom *a, NumberTypeE nt=int32_e);
};
static inline Grid *convert (const t_atom2 &r, Grid **bogus) {return new Grid(r);}

static inline Dim convert(const t_atom2 &x, Dim *foo) {
	Grid *d = convert(x,(Grid **)0);
	if (!d) RAISE("urgh");
	if (d->dim.n!=1) RAISE("dimension list must have only one dimension itself");
	return Dim(d->dim[0],(int32 *)d->data);
}
static inline P<Grid> convert(const t_atom2 &x, P<Grid> *foo) {return P<Grid>(convert(x,(Grid **)0));}

//****************************************************************
// GridInlet represents a grid-aware inlet

#define GRIDHANDLER_ARGS(T) GridInlet &in, long n, T *data

// four-part macro for defining the behaviour of a gridinlet in a class
// C:Class I:Inlet
#define GRID_INLET(I) \
	template <class T> void THISCLASS::grinw_##I (GRIDHANDLER_ARGS(T)) {((THISCLASS*)in.parent)->grin_##I(in,n,data);}\
	template <class T> void THISCLASS::grin_##I  (GRIDHANDLER_ARGS(T)) {if (n==-1)
#define GRID_FLOW   else if (n>=0)
#define GRID_FINISH else if (n==-2)
#define GRID_END }

/* macros for defining a gridinlet's behaviour as just storage (without and with backstore, respectively) */
// V is a P<Grid> instance-var
#define GRID_INPUT(I,V) \
	GRID_INLET(I) {V=new Grid(in.dim,NumberTypeE_type_of(data));} GRID_FLOW {COPY((T *)*(V)+in.dex,data,n);} GRID_FINISH
#define GRID_INPUT2(I,V) GRID_INLET(I) {V->next = new Grid(in.dim,NumberTypeE_type_of(data));} \
	GRID_FLOW {COPY(((T *)*(V->next?V->next.p:&*V.p))+in.dex,data,n);} GRID_FINISH

typedef struct GridInlet GridInlet;
typedef struct GridHandler {
#define FOO(T) void (*flow_##T)(GRIDHANDLER_ARGS(T)); \
	       void   flow     (GRIDHANDLER_ARGS(T)) const {flow_##T(in,n,data);}
EACH_NUMBER_TYPE(FOO)
#undef FOO
} GridHandler;

struct FObject;
struct GridInlet : CObject {
	// set once ever
	FObject *parent; const GridHandler *gh;
	// set once per transmission
	GridOut *sender; Dim dim; NumberTypeE nt; /* nt shouldn't need to exist */
	// modified continually
	long dex; P<Grid> buf; /* factor-chunk buffer */ long bufi; /* buffer index: how much of buf is filled */

	GridInlet(FObject *parent_, const GridHandler *gh_) :
		parent(parent_), gh(gh_), sender(0), dim(0), nt(int32_e), dex(0), bufi(0) {}
	~GridInlet() {}
	void set_chunk(long whichdim);
	int32 factor() {return buf?buf->dim.prod():1;} // which is usually not the same as this->dim->prod(chunk)
	void begin(GridOut *sender);
	void finish();
	template <class T> void flow(long n, T *data); // n=-1 is begin, and n=-2 is finish.
	void from_list(VA, NumberTypeE nt=int32_e) {Grid t(argc,argv,nt); from_grid(&t);}
	void from_atom(VA) {Grid t(argv[0]); from_grid(&t);}
	void from_float(float f) {t_atom2 a[1]; SETFLOAT(a,f); from_atom(1,a);}
	void from_grid(Grid *g);
	bool supports_type(NumberTypeE nt);
private:
	template <class T> void from_grid2(Grid *g, T foo);
};

//****************************************************************
// for use by source_filter.rb ONLY (for \grin)
#ifndef HAVE_LITE
#define GRIN(TB,TS,TI,TL,TF,TD) {TB,TS,TI,TL,TF,TD}
#else
#define GRIN(TB,TS,TI,TL,TF,TD) {TB,TS,TI,TF}
#endif // HAVE_LITE

#define GRIN_all(G)     GRIN(G,G,G,G,G,G)
#define GRIN_int(G)     GRIN(G,G,G,G,0,0)
#define GRIN_int32(G)   GRIN(0,0,G,0,0,0)
#define GRIN_float(G)   GRIN(0,0,0,0,G,G)
#define GRIN_float32(G) GRIN(0,0,0,0,G,0)
#define GRIN_float64(G) GRIN(0,0,0,0,0,G)

#define GRINDECL(I) \
	template <class T>        void grin_##I  (GRIDHANDLER_ARGS(T)); \
	template <class T> static void grinw_##I (GRIDHANDLER_ARGS(T)); \
	static GridHandler grid_##I##_hand;

// for use by source_filter.rb ONLY (for \decl \def \constructor)
#define MESSAGE t_symbol *sel, int argc, t_atom2 *argv
#define MESSAGE2 sel,argc,argv
#define MESSAGE3 t_symbol *, int, t_atom2 *
// you shouldn't use MethodDecl directly (used by source_filter.rb)
struct MethodDecl {const char *selector; FMethod method;};
struct AttrDecl {
	t_symbol *name;
	string type;
	AttrDecl(t_symbol *name_, string type_) {name=name_; type=type_;}
};
typedef FObject *(*t_allocator)(BFObject *,MESSAGE3);
#define CLASS_NOPARENS (1<<16)
#define CLASS_NOCOMMA  (1<<17)
struct FClass {
	t_allocator allocator; // returns a new C++ object
	void (*startup)(FClass *);
	FClass *super;
	int ninlets;
	int noutlets;
	t_class *bfclass;
	t_symbol *name;
	// Note that hash_map for methods saved 20 ns or 60 ns per call when I compared.
	hash_map<pair<int,t_symbol *>,FMethod> methods; // (inlet,selector) -> method.
	map<t_symbol *,AttrDecl *> attrs;
	int flags; // flags: any combination of: CLASS_NOINLET, CLASS_NOPARENS, CLASS_NOCOMMA
};

void fclass_install(FClass *fc, FClass *super);

//****************************************************************
// GridOut represents a grid transmission from the perspective of the sender
struct GridOut : CObject {
	// number of (minimum,maximum) BYTES to send at once (probably shouldn't be in the .h at all...?)
	static const long MIN_PACKET_SIZE = 1<<8;
	static const long MAX_PACKET_SIZE = 1<<12;

	// read-only (set-once)
	FObject *parent; Dim dim; NumberTypeE nt; vector<GridInlet *> inlets;
	GridOut *sender; // dummy (because of P<Dim> to Dim)

	// continually changed
	long dex; // how many numbers were already sent in this connection
	Grid buf; // temporary buffer
	long bufi; // number of bytes used in the buffer
	bool fresh; /* 0 = buf was inited */

	GridOut(FObject *parent_, int woutlet, const Dim &dim_, NumberTypeE nt_=int32_e);
	~GridOut();
	void callback(GridInlet &in);

	// send/send_direct: data belongs to caller, may be stack-allocated,
	// receiver doesn't modify the data; in send(), there is buffering;
	// in send_direct(), there is not. When switching from buffered to
	// unbuffered mode, flush() must be called
	// the macro is probably a hack to force correct behaviour in some special cases,
	// in a way that I couldn't have had by using templates.
	#define FOO(T) void send(long n, T *data) {send_2(n,data);}
	EACH_NUMBER_TYPE(FOO)
	#undef FOO
	template <class T> void send_2(long n, T *data);
	template <class T> void send_direct(long n, T *data);
	void flush(); // goes with send();
	void finish();
	void create_buf();
};

//****************************************************************

#define CHECK_GRIN(class,i) \
	if (in.size()<=i) in.resize(i+1); \
	if (!in[i]) in[i]=new GridInlet((FObject *)this,&class##_grid_##i##_hand);

#define CHECK_ALIGN16(d,nt) \
	{int bytes = 16; \
	int align = ((uintptr_t)(void*)d)%bytes; \
	if (align) {_L_;post("%s(%s): Alignment Warning: %s=%p is not %d-aligned: %d", \
		ARGS(this), __PRETTY_FUNCTION__,#d,(void*)d,bytes,align);}}

struct BFProxy;
struct BFObject : t_object {
#ifdef DES_BUGS
	long magic;
#endif
	FObject *self;
	string binbuf_string ();
};

/* not planning to use or rely on those fields at all. it's more about C++ifying the normal interface to it. */
/*struct _outlet {
    t_object *o_owner;
    struct _outlet *o_next;
    t_outconnect *o_connections;
    t_symbol *o_sym;
    //void send () {outlet_bang(
};*/

// the use of w_gpointer here is fake for non-A_POINTER, just because there's no suitable member in the union
static inline void set_atom (t_atom *a, uint8       v) {SETFLOAT(a,(float)v);}
static inline void set_atom (t_atom *a, int16       v) {SETFLOAT(a,(float)v);}
static inline void set_atom (t_atom *a, int32       v) {SETFLOAT(a,(float)v);}
static inline void set_atom (t_atom *a, uint32      v) {SETFLOAT(a,(float)v);}
static inline void set_atom (t_atom *a, long        v) {SETFLOAT(a,(float)v);}
static inline void set_atom (t_atom *a, float32     v) {SETFLOAT(a,v);}
static inline void set_atom (t_atom *a, float64     v) {SETFLOAT(a,v);}
static inline void set_atom (t_atom *a, string      v) {SETSYMBOL(a,gensym(v.data()));}
static inline void set_atom (t_atom *a, t_symbol   *v) {SETSYMBOL(a,v);}
static inline void set_atom (t_atom *a, Numop      *v) {SETSYMBOL(a,v->sym);}
static inline void set_atom (t_atom *a, t_gpointer *v) {SETPOINTER(a,v);}
static inline void set_atom (t_atom *a, t_blob     *b) {a->a_type = A_BLOB;    a->a_gpointer = (t_gpointer *)b;}
static inline void set_atom (t_atom *a, t_binbuf   *b) {a->a_type = A_LIST;    a->a_gpointer = (t_gpointer *)b;}
//static inline void set_atom (t_atom *a, Grid     *g) {a->a_type = A_GRID;    a->a_gpointer = (t_gpointer *)g;} // future use
static inline void set_atom (t_atom *a, GridOut    *g) {a->a_type = A_GRIDOUT; a->a_gpointer = (t_gpointer *)g;}
static inline void set_atom (t_atom *a)              {a->a_type = A_NULL;    a->a_gpointer = 0;}
static inline void set_atom (t_atom *a, const t_atom &v) {*a=v;}

struct PtrOutlet {
	t_outlet *p;
	operator t_outlet * () {return p;}
	void operator () ()              {outlet_bang(   p  );}
	void operator () (int f)         {outlet_float(  p,f);}
	void operator () (long f)        {outlet_float(  p,f);}
	void operator () (size_t f)      {outlet_float(  p,f);}
	void operator () (float f)       {outlet_float(  p,f);}
	void operator () (double f)      {outlet_float(  p,f);}
	void operator () (t_symbol *s)   {outlet_symbol( p,s);}
	void operator () (t_gpointer *g) {outlet_pointer(p,g);}
//	void operator () (t_blob *g)     {outlet_blob   (p,g);} // can't use this with vanille
	void operator () (t_blob *g)     {t_atom2 a[] = {g}; outlet_anything(p,gensym("blob"),1,a);}
	void operator () (t_binbuf *g)   {t_atom2 a[] = {g}; outlet_anything(p,gensym("binbuf"),1,a);}
	void operator () (             int argc, t_atom *argv) {outlet_list(p,&s_list,argc,argv);}
	void operator () (t_symbol *s, int argc, t_atom *argv) {outlet_anything(p,s,  argc,argv);}
	void operator () (t_atom &a);
};

// represents objects that have inlets/outlets
\class FObject {
	virtual ~FObject ();
	virtual void changed (t_symbol *s=0) {}
	BFObject *bself; // point to PD peer
	int ninlets,noutlets; // per object settings (not class)
	BFProxy  **inlets; // direct access to  inlets (not linked lists)
	PtrOutlet *out;    // direct access to outlets (not linked lists)
	t_canvas *mom;
	void  ninlets_set(int n, bool draw=true);
	void noutlets_set(int n, bool draw=true);
	vector<P<GridInlet> > in;
	P<GridOut> go;
	FObject(BFObject *bself, MESSAGE);
	template <class T> void send_out(int outlet, int argc, T *argv) {
		t_atom foo[argc];
		for (int i=0; i<argc; i++) SETFLOAT(&foo[i],argv[i]);
		outlet_list(out[outlet],&s_list,argc,foo);
	}
	\decl 0 get (t_symbol *s=0);
};
\end class

uint64 gf_timeofday();
extern "C" void Init_gridflow ();
extern Numop2 *op_add,*op_sub,*op_mul,*op_div,*op_mod,*op_shl,*op_and,*op_put;

#undef ARGS
#define ARGS(OBJ) ((OBJ) ? (OBJ)->bself->binbuf_string().data() : "[???]")
#define SAME_TYPE(A,B) if ((A).nt != (B)->nt) RAISE("same type please (%s has %s; %s has %s)", \
	#A, number_type_table[(A).nt].name, \
	#B, number_type_table[(B)->nt].name);
static void SAME_DIM(int n, const Dim &a, int ai, const Dim &b, int bi) {
	if (ai+n > a.n) RAISE("left hand: not enough dimensions");
	if (bi+n > b.n) RAISE("right hand: not enough dimensions");
	for (int i=0; i<n; i++) if (a[ai+i] != b[bi+i])
			RAISE("mismatch: left dim #%d is %d, right dim #%d is %d", ai+i, a[ai+i], bi+i, b[bi+i]);}

void suffixes_are (const char *name, const char *suffixes);
#define install(A,B,C...) install2(fclass,A,B,C)
void install2(FClass *fclass, const char *name, int inlets, int outlets, int flags=CLASS_DEFAULT);
#define add_creator(name) add_creator2(fclass,name)
void add_creator2(FClass *fclass, const char *name);
void add_creator3(FClass *fclass, const char *name);
#define install_format(name,mode,suffixes) do {install(name,1,1); suffixes_are(name,suffixes);} while(0)
void call_super(int argc, t_atom *argv);

\class Format : FObject {
	int mode;
	int fd;
	FILE *f;
	NumberTypeE cast;
	long frame;
	Format(BFObject *, MESSAGE);
	\decl 0 open (t_symbol *mode, string filename);
	\decl 0 close ();
	\decl 0 cast (NumberTypeE nt);
	\decl 0 seek(int frame);
	\decl 0 rewind ();
	~Format ();
};
\end class

extern vector<string> gf_data_path;
string gf_find_file (string x);
void pd_oprintf (ostream &o, const char *s, int argc, t_atom *argv);
void pd_oprint (ostream &o, int argc, t_atom *argv);
void pd_post (const char *s, int argc, t_atom *argv);

extern map<t_symbol *,FClass *> fclasses;
int handle_parens(int ac, t_atom *av);

extern FClass ciFObject, ciFormat;

#ifdef __WIN32__
static inline int vasprintf(char **buf, const char *s, va_list args) {
	int n = vsnprintf(0,0,s,args);
	*buf = (char *)malloc(n+1);
	vsnprintf(*buf,n+1,s,args);
	return n;
}
#endif

/* all those three printf are copied from desiredata */
static inline int voprintf(ostream &buf, const char *s, va_list args) {
    char *b;
    int n = vasprintf(&b,s,args);
    buf << b;
    free(b);
    return n;
}
static inline int oprintf(ostream &buf, const char *s, ...) {
    va_list args;
    va_start(args,s);
    int n = voprintf(buf,s,args);
    va_end(args);
    return n;
}
inline t_symbol *symprintf(const char *s, ...) {
    char *buf;
    va_list args;
    va_start(args,s);
    if (vasprintf(&buf,s,args)<0) {/*rien*/}
    va_end(args);
    t_symbol *r = gensym(buf);
    free(buf);
    return r;
}

              ostream &operator << (ostream &self, const t_atom &a);
static inline ostream &operator << (ostream &self, t_symbol *s) {self << s->s_name; return self;}

static inline string join (int argc, t_atom *argv, string sep=" ", string term="") {
	ostringstream os;
	for (int i=0; i<argc; i++) os << argv[i] << (i==argc-1 ? term : sep);
	return os.str();
}

// from pd/src/g_canvas.c
struct _canvasenvironment {
    t_symbol *ce_dir;   /* directory patch lives in */
    int ce_argc;        /* number of "$" arguments */
    t_atom *ce_argv;    /* array of "$" arguments */
    int ce_dollarzero;  /* value of "$0" */
};

#endif // __GF_GRIDFLOW_H
