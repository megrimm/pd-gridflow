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
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../config.h"

#include <stdlib.h>

#define $ self

#define NEW(_type_,_count_) \
	((_type_ *)qalloc(sizeof(_type_)*(_count_),__FILE__,__LINE__))

#define NEW2(_type_,_count_) \
	((_type_ *)qalloc2(sizeof(_type_)*(_count_),__FILE__,__LINE__))

#define NEW3(_type_,_count_,_file_,_line_) \
	((_type_ *)qalloc2(sizeof(_type_)*(_count_),_file_,_line_))

#define FREE(_var_) \
	_var_ ? (qfree(_var_), _var_=0) : 0

#define REALLOC(_val_,_count_) \
	(qrealloc(_val_,_count_))

void *qalloc2(size_t n, const char *file, int line);
void *qalloc(size_t n, const char *file, int line); /*0xdeadbeef*/
void qfree2(void *data);
void qfree(void *data); /*0xfadedf00*/
void *qrealloc(void *data, int n);
void qdump(void);

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
typedef unsigned long long uint64;
typedef char  int8;
typedef short int16;
typedef long  int32;
typedef float  float32;
typedef double float64;

#ifndef __cplusplus
typedef enum { false, true } bool;
#endif

void gf_lang_init (void);

/* three-way comparison */
static inline int cmp(int a, int b) { return a < b ? -1 : a > b; }

/* **************************************************************** */
/* other general purpose stuff */

/*
  a remainder function such that floor(a/b)*b+mod(a,b) = a
  in contrast to C-language builtin a%b,
  this one has uniform behaviour around zero.
*/
static inline int mod(int a, int b) { if (a<0) a += b * (1-(a/b)); return a%b; }

/* integer powers in log(b) time */
static inline int ipow(int a, int b) {
	int r=1;
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
*/

/* current version number as string literal */
#define GF_VERSION "0.6.1"
#define GF_COMPILE_TIME __DATE__ ", " __TIME__

#ifdef HAVE_SPEED
#define NO_ASSERT
#define NO_DEADBEEF
#endif

#ifdef HAVE_TSC_PROFILING
#define HAVE_PROFILING
#endif

/* lots of macros follow */
/* i'm not going to explain to you why macros *ARE* a good thing. */
/* **************************************************************** */

/*
  we're gonna override this,
  so load it first, to avoid conflicts
*/
#include <assert.h>

#define assert_range(_var_,_lower_,_upper_) \
	if ((_var_) < (_lower_) || (_var_) > (_upper_)) { \
		fprintf(stderr, "%s:%d: assertion failed: %s=%d not in (%d..%d)\n", \
			__FILE__, __LINE__, #_var_, (_var_), (_lower_), (_upper_)); \
		abort(); }

#undef assert
#define assert(_expr_) \
	if (!(_expr_)) { \
		fprintf(stderr, "%s:%d: assertion failed: %s is false\n", \
			__FILE__, __LINE__, #_expr_); \
		abort(); }

#ifdef NO_ASSERT
	/* disabling assertion checking */
	#undef assert
	#define assert(_foo_)
	#undef assert_range
	#define assert_range(_foo_,_a_,_b_)
#endif

/* **************************************************************** */

/* returns the size of a statically defined array */
#define COUNT(_array_) \
	((int)(sizeof(_array_) / sizeof((_array_)[0])))

/* **************************************************************** */

/* those are helpers for profiling. */
#ifdef HAVE_PROFILING
#define ENTER $->profiler_last = rdtsc();
#define LEAVE $->profiler_cumul += rdtsc() - $->profiler_last;
#define ENTER_P $->parent->profiler_last = rdtsc();
#define LEAVE_P $->parent->profiler_cumul += rdtsc()-$->parent->profiler_last;
#else
#define ENTER
#define LEAVE
#define ENTER_P
#define LEAVE_P
#endif

/* **************************************************************** */

#define METHOD_ARGS(_class_) int argc, VALUE *argv, VALUE rself
#define RAISE(args...) rb_raise(rb_eArgError,args)

/* 0.5.0: shortcuts for MethodDecl */

#define DECL(_cl_,_sym_) \
	{#_sym_,METHOD_PTR(_cl_,_sym_)}

#define DGS(_class_) _class_ *$; Data_Get_Struct(rself,_class_,$);

/*
  METHOD is a header for a given method in a given class.
  METHOD_PTR returns a pointer to a method of a class as if it were
  in the FObject class. this is because C's type system is broken.
*/
#define METHOD(_class_,_name_) \
	static void _class_##_##_name_(_class_ *$, VALUE rself, int argc, VALUE *argv); \
	static VALUE _class_##_##_name_##_wrap(METHOD_ARGS(_class_)) { \
		VALUE result; \
		DGS(_class_); \
		ENTER; \
		result = Qnil; _class_##_##_name_($,rself,argc,argv); \
		LEAVE; \
		return result; \
	} \
	static void _class_##_##_name_(_class_ *$, VALUE rself, int argc, VALUE *argv)

/* should be merged with the preceding one someday */
#define METHOD2(_class_,_name_) \
	static VALUE _class_##_##_name_(_class_ *$, VALUE rself, int argc, VALUE *argv); \
	static VALUE _class_##_##_name_##_wrap(METHOD_ARGS(_class_)) { \
		VALUE result; \
		DGS(_class_); \
		result = _class_##_##_name_($,rself,argc,argv); \
		return result; \
	} \
	static VALUE _class_##_##_name_(_class_ *$, VALUE rself, int argc, VALUE *argv)

#define METHOD_PTR(_class_,_name_) \
	((RMethod) _class_##_##_name_##_wrap)

typedef VALUE (*RMethod)(VALUE $, ...); /* !@#$ */

/* class constructor */

#define GRCLASS(_name_,_inlets_,_outlets_,_handlers_,args...) \
	static MethodDecl _name_ ## _methods[] = { args }; \
	static GridHandler _name_ ## _handlers[] = { _handlers_ }; \
	GridClass _name_ ## _classinfo = { \
		sizeof(_name_), \
		COUNT(_name_##_methods),\
		_name_##_methods,\
		_inlets_,_outlets_,COUNT(_name_##_handlers),_name_##_handlers, \
		#_name_ };

#define FMTCLASS(_name_,symbol_name,description,flags,_inlets_,_outlets_,_handlers_,args...) \
	FormatInfo _name_ ## _formatinfo = { symbol_name,description,flags }; \
	static MethodDecl _name_ ## _methods[] = { args }; \
	static GridHandler _name_ ## _handlers[] = { _handlers_ }; \
	GridClass _name_ ## _classinfo = { \
		sizeof(_name_), \
		COUNT(_name_##_methods),\
		_name_##_methods,\
		_inlets_,_outlets_,COUNT(_name_##_handlers),_name_##_handlers, \
		#_name_ };

#define SI(_sym_) (rb_intern(#_sym_))
#define SYM(_sym_) (ID2SYM(SI(_sym_)))

/* */
static inline uint64 rdtsc(void) {
  uint64 x;
  __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
  return x;}

/* is little-endian */
static inline bool is_le(void) {
	int x=1;
	return ((char *)&x)[0];
}

/* **************************************************************** */
/* general purpose but Ruby/jMax specific */

void whine(const char *fmt, ...);
void whine_time(const char *s);

typedef struct MethodDecl {
	const char *selector;
	RMethod method;
} MethodDecl;

void define_many_methods(VALUE/*Class*/ $, int n, MethodDecl *methods);

/* **************************************************************** */
/* limits */

/* maximum size of any grid (256 megs in 32-bit, 64 megs in 8-bit) */
#define MAX_NUMBERS 64*1024*1024

/* used as maximum width, maximum height, etc. */
#define MAX_INDICES 32767

/* maximum number of dimensions in an array */
#define MAX_DIMENSIONS 16

/* 1 + maximum id of last grid-aware inlet/outlet */
#define MAX_INLETS 4
#define MAX_OUTLETS 2

/* number of (maximum,ideal) Numbers to send at once */
/* this should remain a constant throughout execution
   because code still expect it to be constant. */
extern int gf_max_packet_length;

/*
  what kind of number a Grid contains.
  note that on the other hand, indexing and dimensioning of Grids is
  still done with explicit ints.
*/
typedef long Number;

/* **************************************************************** */

#define DECL_SYM(_sym_) \
	extern VALUE/*Symbol*/ sym_##_sym_;

DECL_SYM(grid_begin)
DECL_SYM(grid_flow)
DECL_SYM(grid_end)
DECL_SYM(bang)
DECL_SYM(int)
DECL_SYM(list)

/* **************************************************************** */
/* dim.c */

/*
  a const array that holds dimensions of a grid
  and can do some calculations on positions in that grid.
*/

typedef struct Dim {
	int n;
	int v[1];
} Dim;

Dim *Dim_new2(int n, int *v, const char *file, int line);
#define Dim_new(_n_,_v_) Dim_new2(_n_,_v_,__FILE__,__LINE__)
Dim *Dim_dup2(Dim *$, const char *file, int line);
#define Dim_dup(_$_) Dim_dup2(_$_,__FILE__,__LINE__)
int Dim_count(Dim *$);
int Dim_get(Dim *$, int i);
int Dim_prod(Dim *$);
int Dim_prod_start(Dim *$, int start);
char *Dim_to_s(Dim *$);
int Dim_calc_dex(Dim *$, int *v);

/* **************************************************************** */
/* BitPacking objects encapsulate optimised loops of conversion */

typedef struct BitPacking BitPacking;

int high_bit(uint32 n);
int low_bit(uint32 n);
BitPacking *BitPacking_new(int endian, int bytes, uint32 r, uint32 g, uint32 b);
void    BitPacking_whine(BitPacking *$);
uint8  *BitPacking_pack(BitPacking *$, int n, const Number *data, uint8 *target);
Number *BitPacking_unpack(BitPacking *$, int n, const uint8 *in, Number *out);
int     BitPacking_bytes(BitPacking *$);
bool    BitPacking_is_le(BitPacking *$);

extern int builtin_bitpacks_n;
extern BitPacking builtin_bitpacks[];

void swap32 (int n, uint32 *data);
void swap16 (int n, uint16 *data);

/* **************************************************************** */
/* Operator objects encapsulate optimised loops of simple operations */

#define DECL_TYPE(_name_,_size_) \
	_name_##_type_i

typedef enum NumberTypeIndex {
	DECL_TYPE(     uint8,  8),
	DECL_TYPE(      int8,  8),
	DECL_TYPE(    uint16, 16),
	DECL_TYPE(     int16, 16),
/*	DECL_TYPE(    uint32, 32), */
	DECL_TYPE(     int32, 32),
} NumberTypeIndex;

#undef DECL_TYPE

typedef struct NumberType {
	VALUE /*Symbol*/ sym;
	const char *name;
	int size;
} NumberType;

typedef struct Operator1 {
	VALUE /*Symbol*/ sym;
	const char *name;
	Number (*op)(Number);
	void   (*op_array)(int,Number*);
} Operator1;

typedef struct Operator2 {
	VALUE /*Symbol*/ sym;
	const char *name;
	Number (*op)(Number,Number);
	void   (*op_array)(int,Number *,Number);
	void   (*op_array2)(int,Number *, const Number *);
	Number (*op_fold)(Number,int,const Number *);
} Operator2;

extern Operator1 op1_table[];
extern Operator2 op2_table[];
extern VALUE /*Hash*/ op1_dict;
extern VALUE /*Hash*/ op2_dict;

/* **************************************************************** */

/* future use:
typedef struct Grid {
	Dim *dim;
	Number *data;
	// add: number type
	// add: producer function
} Grid;
*/

/* **************************************************************** */
/* GridInlet represents a grid-aware inlet */

typedef struct GridInlet  GridInlet;
typedef struct GridOutlet GridOutlet;
typedef struct GridObject GridObject;

#define GRID_BEGIN_(_cl_,_name_) bool _name_(VALUE rself,_cl_ *$, GridInlet *in)
#define  GRID_FLOW_(_cl_,_name_) void _name_(VALUE rself,_cl_ *$, GridInlet *in, int n, const Number *data)
#define GRID_FLOW2_(_cl_,_name_) void _name_(VALUE rself,_cl_ *$, GridInlet *in, int n, Number *data)
#define   GRID_END_(_cl_,_name_) void _name_(VALUE rself,_cl_ *$, GridInlet *in)

typedef GRID_BEGIN_(GridObject, (*GridBegin));
typedef  GRID_FLOW_(GridObject, (*GridFlow));
typedef GRID_FLOW2_(GridObject, (*GridFlow2));
typedef   GRID_END_(GridObject, (*GridEnd));

#define GRID_BEGIN(_cl_,_inlet_) static GRID_BEGIN_(_cl_,_cl_##_##_inlet_##_begin)
#define  GRID_FLOW(_cl_,_inlet_) static  GRID_FLOW_(_cl_,_cl_##_##_inlet_##_flow)
#define GRID_FLOW2(_cl_,_inlet_) static GRID_FLOW2_(_cl_,_cl_##_##_inlet_##_flow)
#define   GRID_END(_cl_,_inlet_) static   GRID_END_(_cl_,_cl_##_##_inlet_##_end)

typedef struct GridHandler {
	int       winlet;
	GridBegin begin;
	GridFlow  flow; /* or GridFlow2 */
	GridEnd   end;
	int       mode; /* 4=ro=flow; 6=rw=flow2 */
} GridHandler;

struct GridInlet {
/* context information */
	GridObject *parent;
	const GridHandler *gh;

/* grid progress info */
	Dim *dim;
	int dex;

/* grid receptor */

	int factor; /* flow's n will be multiple of $->factor */
	int bufn;
	Number *buf; /* factor-chunk buffer */
};

GridInlet *GridInlet_new(GridObject *parent, const GridHandler *gh);
void GridInlet_delete(GridInlet *$);
GridObject *GridInlet_parent(GridInlet *$);
void GridInlet_abort(GridInlet *$);
void GridInlet_set_factor(GridInlet *$, int factor);
bool GridInlet_busy(GridInlet *$);

typedef struct GridClass {
	int objectsize;
	int methodsn;
	MethodDecl *methods;
	int inlets;
	int outlets;
	int handlersn;
	GridHandler *handlers;
	const char *name;
} GridClass;

#define LIST(args...) args

/* should merge those two together */
#define GRINLET(_class_,_winlet_) {_winlet_,\
	((GridBegin)_class_##_##_winlet_##_begin), \
	 ((GridFlow)_class_##_##_winlet_##_flow), \
	  ((GridEnd)_class_##_##_winlet_##_end), 4 }

#define GRINLET2(_class_,_winlet_) {_winlet_,\
	((GridBegin)_class_##_##_winlet_##_begin), \
	 ((GridFlow)_class_##_##_winlet_##_flow), \
	  ((GridEnd)_class_##_##_winlet_##_end), 6 }

/* **************************************************************** */
/* GridOutlet represents a grid-aware outlet */

struct GridOutlet {
/* context information */
	GridObject *parent;
	int woutlet;

/* grid progress info */
	Dim *dim;
	int dex;

/* buffering */
	Number *buf;
	int bufn;

/* transmission accelerator */
	int frozen;
	int ron; GridInlet **ro; /* want (const Number *) shown to */
	int rwn; GridInlet **rw; /* want (Number *) given to */
};

GridOutlet *GridOutlet_new(GridObject *parent, int woutlet);
void GridOutlet_delete(GridOutlet *$);
GridObject *GridOutlet_parent(GridOutlet *$);
bool GridOutlet_busy   (GridOutlet *$);
void GridOutlet_begin  (GridOutlet *$, Dim *dim);
void GridOutlet_abort  (GridOutlet *$);
void GridOutlet_give       (GridOutlet *$, int n,       Number *data);
void GridOutlet_send       (GridOutlet *$, int n, const Number *data);
void GridOutlet_send_direct(GridOutlet *$, int n, const Number *data);
void GridOutlet_flush  (GridOutlet *$);
void GridOutlet_end    (GridOutlet *$);
void GridOutlet_callback(GridOutlet *$, GridInlet *in, int mode);

/* **************************************************************** */
/* GridFlow::GridObject inherits from GridFlow::FObject */

#define GridObject_FIELDS \
	VALUE /*GridFlow::FObject*/ peer; /* point to Ruby peer */ \
	GridClass *grid_class; \
	void *foreign_peer; /* point to jMax peer */ \
	uint64 profiler_cumul, profiler_last; \
	GridInlet  * in[MAX_INLETS]; \
	GridOutlet *out[MAX_OUTLETS];

struct GridObject {
	GridObject_FIELDS;
};

void GridObject_conf_class(VALUE $, GridClass *grclass);

/* **************************************************************** */
/* GridFlow::Format inherits from GridFlow::GridObject */

#define FF_W   (1<<1)
#define FF_R   (1<<2)
#define FF_RW  (1<<3)

typedef struct FormatInfo FormatInfo;
typedef struct Format Format;

struct FormatInfo {
	const char *symbol_name; /* short identifier */
	const char *description; /* long identifier */
	int flags;
};

#define Format_FIELDS \
	GridObject_FIELDS; \
	GridObject *parent; \
	VALUE /*Symbol*/ mode; \
	BitPacking *bit_packing; \
	Dim *dim;

struct Format {
	Format_FIELDS;
};

extern GridClass *format_classes[];

/*
	NEW FORMAT API
	mode is :in or :out
	def initialize(mode,*args) : open a file handler (do it via .new of class)
	def frame() : read one frame, send through outlet 0
	def seek(Integer i) : select one frame to be read next (by number)
	def length() : ^Integer number of frames
	def option(Symbol name, *args) : miscellaneous options
	def close() : close a handler
*/

/* **************************************************************** */

extern VALUE /*Hash*/ gf_object_set;
extern const char *whine_header;

/* **************************************************************** */
/* 0.6.0 */

typedef struct BFObject BFObject; /* fts_object_t or something */

typedef struct GFBridge {
	VALUE (*send_out)(int argc, VALUE *argv, VALUE sym, int outlet, VALUE $);
	VALUE (*class_install)(VALUE $, char *name2, VALUE inlets2, VALUE outlets2);
	void (*post)(const char *, ...);
} GFBridge;

extern GFBridge gf_bridge;
extern VALUE GridFlow_module;
extern VALUE FObject_class;
extern VALUE FObject_class;
extern VALUE GridObject_class;
extern VALUE Format_class;

uint64 RtMetro_now(void);

VALUE gf_post_string (VALUE $, VALUE s);
void FObject_mark (VALUE *$);
void FObject_sweep (VALUE *$);
VALUE FObject_send_out(int argc, VALUE *argv, VALUE $);
VALUE FObject_s_install(VALUE $, VALUE name, VALUE inlets, VALUE outlets);
VALUE FObject_s_new(VALUE argc, VALUE *argv, VALUE qlass);
char *rb_sym_name(VALUE sym);
void post(const char *,...);

/* keyed on data */
void MainLoop_add(void *data, void (*func)());
void MainLoop_remove(void *data);

void gf_init (void);
#define PTR2FIX(ptr) ((((long)ptr)&3?RAISE("pointer alignment error"):0),INT2NUM(((int)ptr)/4))
#define FIX2PTR(value) (void *)(FIX2INT(value)*4)

#define DEF(_class_,_name_,_argc_) \
	rb_define_method(_class_##_class,#_name_,_class_##_##_name_,_argc_)

#define SDEF(_class_,_name_,_argc_) \
	rb_define_singleton_method(_class_##_class,#_name_,_class_##_s_##_name_,_argc_)

#define INTEGER_P(_value_) (FIXNUM_P(_value_) || TYPE(_value_)==T_BIGNUM)
#define FLOAT_P(_value_) (TYPE(_value_)==T_FLOAT)
#define EARG(_reason_...) rb_raise(rb_eArgError, _reason_)

#define INT(x) (INTEGER_P(x) ? NUM2INT(x) : (RAISE("expected Integer"),0))
#define INSTALL(jname,rname) \
	ruby_c_install(jname, #rname, &rname##_classinfo, GridObject_class);

VALUE ruby_c_install(const char *jname, const char *rname, GridClass *gc,
VALUE super);

#undef post
#define post(args...) gf_bridge.post(args)

#define EVAL(s) rb_eval_string(s)

#define rb_str_len(s) (RSTRING(s)->len)
#define rb_str_ptr(s) (RSTRING(s)->ptr)
#define rb_ary_len(s) (RARRAY(s)->len)
#define rb_ary_ptr(s) (RARRAY(s)->ptr)

typedef VALUE (*RFunc)();

#ifdef __cplusplus
};
#endif

#endif /* __GF_GRID_H */
