/*
	$Id$

	GridFlow
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../../COPYING for further informations on licensing terms.

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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../config.h"

/* current version number as string literal */
#define GF_VERSION "0.5.0"
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

/*
  "COUNT(array), array", a shortcut because often in jmax you pass
  first the size of an array followed by a pointer to that array.
*/
#define ARRAY(_array_) \
	COUNT(_array_), _array_

/*
  make sure an int is not too small nor too big;
  warn and adjust outside values
*/
#define COERCE_INT_INTO_RANGE(_var_,_lower_,_upper_) \
	if ((_var_) < (_lower_)) { \
		whine("warning: %s=%d was smaller than %d\n", #_var_, (_var_), (_lower_)); \
		(_var_) = (_lower_); \
	} else if ((_var_) > (_upper_)) { \
		whine("warning: %s=%d was bigger than %d\n", #_var_, (_var_), (_upper_)); \
		(_var_) = (_upper_); \
	}

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

#define ATOMLIST int ac, const Var *at

/*
  lists the arguments suitable for any method of a given class.
  this includes a class-specific class name so you don't have to
  write inane stuff like SomeClass* self = (SomeClass *) o;
  at the beginning of every method body. This is a trade-off because
  it means you have to cast to (FObject *) sometimes.
  see METHOD_PTR, METHOD, OBJ
*/

#define METHOD_ARGS(_class_) \
	_class_ *self, int winlet, Symbol selector, ATOMLIST

/* 0.5.0 */
#define RAISE(args...) return whine(args),0;
#define RAISE2(args...) {fts_object_set_error(OBJ($),args);return;}

/* 0.5.0: shortcuts for MethodDecl */

#define DECL(_cl_,_inlet_,_sym_,args...) \
	{_inlet_,#_sym_,METHOD_PTR(_cl_,_sym_),args}
#define DECL2(_cl_,_inlet_,_sym_,_sym2_,args...) \
	{_inlet_,#_sym_,METHOD_PTR(_cl_,_sym2_),args}

/*
  METHOD is a header for a given method in a given class.
  METHOD_PTR returns a pointer to a method of a class as if it were
  in the FObject class. this is because C's type system is broken.
*/
#ifdef HAVE_PROFILING
#define METHOD(_class_,_name_) \
	static void _class_##_##_name_(METHOD_ARGS(_class_)); \
	static void _class_##_##_name_##_wrap(METHOD_ARGS(_class_)) { \
		ENTER; \
		_class_##_##_name_(self,winlet,selector,ac,at); \
		LEAVE; \
	} \
	static void _class_##_##_name_(METHOD_ARGS(_class_))
#define METHOD_PTR(_class_,_name_) \
	((FTSMethod) _class_##_##_name_##_wrap)
#else
#define METHOD(_class_,_name_) \
	static void _class_##_##_name_(METHOD_ARGS(_class_))
#define METHOD_PTR(_class_,_name_) \
	((FTSMethod) _class_##_##_name_)
#endif

typedef void(*FTSMethod)(METHOD_ARGS(FObject));

/* class constructor */

#define GRCLASS(_name_,_inlets_,_outlets_,_handlers_,args...) \
	static MethodDecl _name_ ## _methods[] = { args }; \
	static GridHandler _name_ ## _handlers[] = { _handlers_ }; \
	GridClass _name_ ## _class = { \
		sizeof(_name_), \
		ARRAY(_name_##_methods),\
		_inlets_,_outlets_,ARRAY(_name_##_handlers) }; \
	static fts_status_t _name_ ## _class_init (fts_class_t *class, ATOMLIST) {\
		GridObject_conf_class2(class,&_name_##_class); \
		return fts_Success;}

/*
  casts a pointer to any object, into a FObject *.
*/
#define OBJ(_object_) \
	((FObject *)(_object_))

/*
  get arg or default value
  this works with int, float, symbol,
  string (const char *), ptr (void *)
*/
#define GET(_i_,_type_,_default_) \
	(_i_>=0 && _i_<ac ? Var_get_##_type_ (at+_i_) : _default_)

#define PUT(_i_,_type_,_value_) \
	Var_put_##_type_ (at+_i_,_value_)

#define SYM(_sym_) (Symbol_new(#_sym_))

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
/* general purpose but jMax specific */

void whine(char *fmt, ...);
void whine_time(const char *s);

typedef struct MethodDecl {
	int winlet;
	const char *selector;
	void (*method)(METHOD_ARGS(FObject));
	const char *signature;
} MethodDecl;

void define_many_methods(fts_class_t *class, int n, MethodDecl *methods);

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
	extern Symbol sym_##_sym_;

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
/*	Dim *Dim_new(int n, int *v); */
#define Dim_new(_n_,_v_) Dim_new2(_n_,_v_,__FILE__,__LINE__)
	Dim *Dim_dup2(Dim *$, const char *file, int line);
/*	Dim *Dim_dup(Dim *$); */
#define Dim_dup(_$_) Dim_dup2(_$_,__FILE__,__LINE__)
	int Dim_count(Dim *$);
	int Dim_get(Dim *$, int i);
	int Dim_prod(Dim *$);
	int Dim_prod_start(Dim *$, int start);

	char *Dim_to_s(Dim *$);
	int Dim_equal_verbose(Dim *$, Dim *other);
	int Dim_equal_verbose_hwc(Dim *$, Dim *other);
	int Dim_dex_add(Dim *$, int n, int *dex);
	int Dim_calc_dex(Dim *$, int *v);

/* **************************************************************** */
/* bitpacking.c */

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
/* operator.c */

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
	Symbol sym;
	const char *name;
	int size;
} NumberType;

typedef struct Operator1 {
	Symbol sym;
	const char *name;
	Number (*op)(Number);
	void   (*op_array)(int,Number*);
} Operator1;

	extern Operator1 op1_table[];
	extern Dict *op1_dict;

typedef struct Operator2 {
	Symbol sym;
	const char *name;
	Number (*op)(Number,Number);
	void   (*op_array)(int,Number *,Number);
	void   (*op_array2)(int,Number *, const Number *);
	Number (*op_fold)(Number,int,const Number *);
} Operator2;

	extern Operator2 op2_table[];
	extern Dict *op2_dict;

/* **************************************************************** */
/* grid.c (part 1: inlet objects) */

/* GridInlet represents a grid-aware jmax inlet */

typedef struct GridInlet  GridInlet;
typedef struct GridOutlet GridOutlet;
typedef struct GridObject GridObject;

#define GRID_BEGIN_(_cl_,_name_) bool _name_(_cl_ *$, GridInlet *in)
#define  GRID_FLOW_(_cl_,_name_) void _name_(_cl_ *$, GridInlet *in, int n, const Number *data)
#define GRID_FLOW2_(_cl_,_name_) void _name_(_cl_ *$, GridInlet *in, int n, Number *data)
#define   GRID_END_(_cl_,_name_) void _name_(_cl_ *$, GridInlet *in)

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
	int       mode; /* 4=r=flow; 6=rw=flow2 */
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
} GridClass;

#define LIST(args...) args

#define GRINLET(_class_,_winlet_) {_winlet_,\
	((GridBegin)_class_##_##_winlet_##_begin), \
	 ((GridFlow)_class_##_##_winlet_##_flow), \
	  ((GridEnd)_class_##_##_winlet_##_end), 4 }

#define GRINLET2(_class_,_winlet_) {_winlet_,\
	((GridBegin)_class_##_##_winlet_##_begin), \
	 ((GridFlow)_class_##_##_winlet_##_flow), \
	  ((GridEnd)_class_##_##_winlet_##_end), 6 }

/* **************************************************************** */
/* grid.c (part 2: outlet objects) */
/* GridOutlet represents a grid-aware jmax outlet */

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
	int ron;
	GridInlet *ro; /* want (const Number *) shown to */
	int rwn;
	GridInlet *rw; /* want (Number *) given to */
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
/* grid.c (part 3: processor objects) */

#define GridObject_FIELDS \
	FObject _o; /* extends FObject */ \
	uint64 profiler_cumul, profiler_last; \
	GridInlet  * in[MAX_INLETS]; \
	GridOutlet *out[MAX_OUTLETS];

struct GridObject {
	GridObject_FIELDS;
};

	void GridObject_init(GridObject *$);
	void GridObject_delete(GridObject *$);
	void GridObject_conf_class(fts_class_t *class, int winlet);
	void GridObject_conf_class2(fts_class_t *class, GridClass *grclass);

/* **************************************************************** */
/* io.c (part 1: streams) */

typedef bool (*OnRead)(void *target,int n,char *buf);

typedef struct Stream {
	Object _o; /* inherit */
	int fd; /* kernel interface (unbuffered) */
	FILE *file; /* stdio.h interface (buffered) */
/* async stuff */
	char *buf;
	int buf_i;
	int buf_n;
	OnRead on_read;
	void *target;
} Stream;

Stream *Stream_open_file(const char *name, int mode);
Stream *Stream_open_fd(int fd, int mode);

void Stream_nonblock(Stream *$);
int Stream_get_fd(Stream *$);
FILE *Stream_get_file(Stream *$);
int Stream_read(Stream *$, int n, char *buf);
void Stream_on_read_do(Stream *$,  int n, OnRead on_read, void *target);
bool Stream_try_read(Stream *$);
bool Stream_is_waiting(Stream *$);
void Stream_close(Stream *$); /* does free too */
/*int Stream_write(Stream *$, int n, char *buf);*/

/* **************************************************************** */
/* io.c (part 2: formats) */

#define FF_W   (1<<1)
#define FF_R   (1<<2)
#define FF_RW  (1<<3)
extern const char *format_flags_names[];

typedef struct FormatClass FormatClass;
typedef struct Format Format;

/* methods on objects of this class */
/* for reading, to outlet */
	/* frames - how many frames there are (optional) */
	typedef int (*Format_frames_m)(Format *$);
	/*
	  read a frame, sending to outlet
	  if frame number is -1, pick up next frame (reloop after last)
	  (if frames() is not implemented, only -1 is valid)
	*/
	typedef bool (*Format_frame_m)(Format *$, GridOutlet *out, int frame);
/* both in read and write mode */
	/* all additional functionality */
	typedef void (*Format_option_m)(Format *$, ATOMLIST);
	/* destroys this object */
	typedef void (*Format_close_m)(Format *$);

struct FormatClass {
	int object_size;
	const char *symbol_name; /* short identifier */
	const char *long_name; /* long identifier */
	int flags;

	/*
	  mode=4 is reading; mode=2 is writing;
	  other values are not used yet (not even 6)
	*/
	Format *(*open)(FormatClass *$, GridObject *parent, int mode, ATOMLIST);

	Format_frames_m frames;
	Format_frame_m frame;

/* for writing, from inlet */
	GridHandler *handler;

/* for both */
	/* for misc options */
	Format_option_m option;
	Format_close_m close;
};

#define Format_FIELDS \
	Object _o; \
	FormatClass *cl; \
	GridObject *parent; \
	Format *chain; \
	int mode; \
	Stream *st; \
	BitPacking *bit_packing; \
	Dim *dim;

struct Format {
	Format_FIELDS;
};

extern FormatClass FORMAT_LIST( ,class_);
extern FormatClass *format_classes[];
extern Dict *format_classes_dex;

Format *Format_open(FormatClass *qlass, GridObject *parent, int mode);
void Format_close(Format *$);

/* **************************************************************** */

extern Dict *gf_object_set;
extern Dict *gf_timer_set;
extern Timer *gf_timer;
extern const char *whine_header;

char *FObject_to_s(FObject *$);

uint64 RtMetro_now(void);
void gf_install_bridge(void);

#endif /* __GF_GRID_H */
