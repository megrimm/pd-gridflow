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

#ifndef __GRIDFLOW_GRID_H
#define __GRIDFLOW_GRID_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "config.h"
#include "lang.h"

/* current version number as string literal */
#define GRIDFLOW_VERSION "0.4.0"
#define GRIDFLOW_COMPILE_TIME __DATE__ ", " __TIME__

#ifdef HAVE_SPEED
#define NO_ASSERT
#define NO_DEADBEEF
#endif

#ifdef HAVE_TSC_PROFILING
#define HAVE_PROFILING
#endif

/* **************************************************************** */
/* jMax macros */

/* we're gonna override this, so load it first, to avoid conflicts */
#include <assert.h>

/*
  This file section defines a few macros of general use. The goal of these is
  to work more at the level of jmax's ideas instead of copy+pasting
  boilerplate code (and having to read that afterwards). In short, they
  should not hinder understanding of the code, but exactly the reverse.

  The set of macros should remain small so that it stays easy to
  use and understand. They should map to concepts commonly used in the
  jMax extensions and/or user interface.

  Please point out any inappropriate definitions and possible
  replacements for those.
*/

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

/*
  lists the arguments suitable for any method of a given class.
  this includes a class-specific class name so you don't have to
  write inane stuff like
  
  SomeClass* self = (SomeClass *) o;
  
  at the beginning of every method body. This is a trade-off because
  it means you have to cast to (fts_object_t *) sometimes.
  see METHOD_PTR, METHOD, OBJ

*/

#define ATOMLIST int ac, const fts_atom_t *at

#define METHOD_ARGS(_class_) \
	_class_ *self, int winlet, fts_symbol_t selector, ATOMLIST

/*
  now (0.3.1) using a separate macro for some things like decls
  and for methods that should not be profiled.
*/

#define METHOD2(_class_,_name_) \
	void _class_##_##_name_(METHOD_ARGS(_class_))

/*
  a header for a given method in a given class.
*/
#ifdef HAVE_PROFILING
#define METHOD(_class_,_name_) \
	void _class_##_##_name_(METHOD_ARGS(_class_)); \
	void _class_##_##_name_##_wrap(METHOD_ARGS(_class_)) { \
		ENTER; \
		_class_##_##_name_(self,winlet,selector,ac,at); \
		LEAVE; \
	} \
	void _class_##_##_name_(METHOD_ARGS(_class_))
#else
#define METHOD(_class_,_name_) METHOD2(_class_,_name_)
#endif

/* this is old-style METHOD_PTR */
#define METHOD2PTR(_class_,_name_) \
	((void(*)(METHOD_ARGS(fts_object_t))) _class_##_##_name_)

/*
  returns a pointer to a method of a class as an fts_object_t* instead
  of a pointer to an object of your class. This is because in C, structs
  cannot inherit from another struct, and so a pointer of one type cannot be
  considered as a pointer to a similar, more elementary type.
*/
#ifdef HAVE_PROFILING
#define METHOD_PTR(_class_,_name_) \
	((void(*)(METHOD_ARGS(fts_object_t))) _class_##_##_name_##_wrap)
#else
#define METHOD_PTR(_class_,_name_) METHOD2PTR(_class_,_name_)
#endif

/* a header for the class constructor */
#define CLASS(_name_) \
	fts_status_t _name_ ## _class_init (fts_class_t *class, ATOMLIST)

/* returns the size of a statically defined array */
#define COUNT(_array_) \
	((int)(sizeof(_array_) / sizeof((_array_)[0])))

/*
  casts a pointer to any object, into a fts_object_t*.
*/
#define OBJ(_object_) \
	((fts_object_t *)(_object_))

/*
  get arg or default value
  this works with int, float, symbol (fts_symbol_t),
  string (const char *), ptr (void *)
*/
#define GET(_i_,_type_,_default_) \
	fts_get_##_type_##_arg (ac,at,_i_,_default_)

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
		post("warning: %s=%d was smaller than %d\n", #_var_, (_var_), (_lower_)); \
		(_var_) = (_lower_); \
	} else if ((_var_) > (_upper_)) { \
		post("warning: %s=%d was bigger than %d\n", #_var_, (_var_), (_upper_)); \
		(_var_) = (_upper_); \
	}

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

#define SYM(_sym_) (fts_new_symbol(#_sym_))

#ifdef NO_ASSERT
	/* disabling assertion checking */
	#undef assert
	#define assert(_foo_)
	#undef assert_range
	#define assert_range(_foo_,_a_,_b_)
#endif

/* **************************************************************** */
/* other general purpose stuff */

/*
  a remainder function such that floor(a/b)*b+mod(a,b) = a
  in contrast to C-language builtin a%b,
  this one has uniform behaviour around zero.
*/
static inline int mod(int a, int b) { if (a<0) a += b * (1-(a/b)); return a%b; }

/*
  integer powers.
  very bad algorithm, but still avoids using floats.
  (most useful values of b are 2 and 3)
*/
static inline int ipow(int a, int b) {
	int r=1;
	if (b>10) return (int)(0+floor(pow(a,b)));
	while(b-->0) r *= a;
	return r;
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

/* three-way comparison */
static inline int cmp(int a, int b) { return a < b ? -1 : a > b; }

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
int    gf_file_open(const char *name, int mode);
FILE *gf_file_fopen(const char *name, int mode); 

typedef struct MethodDecl MethodDecl;
struct MethodDecl {
	int winlet;
	fts_symbol_t selector;
	void (*method)(METHOD_ARGS(fts_object_t));
	int n_args;
	fts_type_t *args;
	int min_args;
};

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
	extern fts_symbol_t sym_##_sym_;

DECL_SYM(open)
DECL_SYM(close)
DECL_SYM(reset)
DECL_SYM(autodraw)
DECL_SYM(grid_begin)
DECL_SYM(grid_flow)
DECL_SYM(grid_flow2)
DECL_SYM(grid_end)

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

	Dim *Dim_new(int n, int *v);
	Dim *Dim_dup(Dim *$);
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
	BitPacking *BitPacking_new(int bytes, uint32 r, uint32 g, uint32 b);
	void    BitPacking_whine(BitPacking *$);
	uint8  *BitPacking_pack(BitPacking *$, int n, const Number *data, uint8 *target);
	Number *BitPacking_unpack(BitPacking *$, int n, const uint8 *in, Number *out);
	int     BitPacking_bytes(BitPacking *$);

extern int builtin_bitpacks_n;
extern BitPacking builtin_bitpacks[];

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
	fts_symbol_t sym;
	const char *name;
	int size;
} NumberType;

typedef struct Operator1 {
	fts_symbol_t sym;
	const char *name;
	Number (*op)(Number);
	void   (*op_array)(int,Number*);
} Operator1;

	Operator1 *op1_table_find(fts_symbol_t sym);
	extern Operator1 op1_table[];

typedef struct Operator2 {
	fts_symbol_t sym;
	const char *name;
	Number (*op)(Number,Number);
	void   (*op_array)(int,Number *,Number);
	void   (*op_array2)(int,Number *, const Number *);
	Number (*op_fold)(Number,int,const Number *);
} Operator2;

	Operator2 *op2_table_find(fts_symbol_t sym);
	extern Operator2 op2_table[];

/* **************************************************************** */
/* grid.c (part 1: inlet objects) */

/* GridInlet represents a grid-aware jmax inlet */

typedef struct GridInlet  GridInlet;
typedef struct GridOutlet GridOutlet;
typedef struct GridObject GridObject;
/* typedef struct GridHandler GridHandler; */

#define GRID_BEGIN_(_class_,_name_) bool _name_(_class_ *$, GridInlet *in)
#define  GRID_FLOW_(_class_,_name_) void _name_(_class_ *$, GridInlet *in, int n, const Number *data)
#define GRID_FLOW2_(_class_,_name_) void _name_(_class_ *$, GridInlet *in, int n, Number *data)
#define   GRID_END_(_class_,_name_) void _name_(_class_ *$, GridInlet *in)

#define GRID_BEGIN_PTR(_class_,_inlet_) ((GRID_BEGIN_(_class_,(*)))_class_##_##_inlet_##_begin)
#define  GRID_FLOW_PTR(_class_,_inlet_)  ((GRID_FLOW_(_class_,(*)))_class_##_##_inlet_##_flow)
#define GRID_FLOW2_PTR(_class_,_inlet_) ((GRID_FLOW2_(_class_,(*)))_class_##_##_inlet_##_flow2)
#define   GRID_END_PTR(_class_,_inlet_)   ((GRID_END_(_class_,(*)))_class_##_##_inlet_##_end)

typedef GRID_BEGIN_(GridObject, (*GridBegin));
typedef  GRID_FLOW_(GridObject, (*GridFlow));
typedef GRID_FLOW2_(GridObject, (*GridFlow2));
typedef   GRID_END_(GridObject, (*GridEnd));

#define GRID_BEGIN(_class_,_inlet_) GRID_BEGIN_(_class_,_class_##_##_inlet_##_begin)
#define  GRID_FLOW(_class_,_inlet_)  GRID_FLOW_(_class_,_class_##_##_inlet_##_flow)
#define GRID_FLOW2(_class_,_inlet_) GRID_FLOW2_(_class_,_class_##_##_inlet_##_flow2)
#define   GRID_END(_class_,_inlet_)   GRID_END_(_class_,_class_##_##_inlet_##_end)

struct GridInlet {
/* context information */
	GridObject *parent;
	int winlet;

/* grid progress info */
	Dim *dim;
	int dex;

/* grid receptor */
	GridBegin begin;
	GridFlow  flow;
	GridFlow2 flow2;
	GridEnd   end;

	int factor; /* flow's n will be multiple of $->factor */
	int bufn;
	Number *buf; /* factor-chunk buffer */
	int mode;
};

	GridInlet *GridInlet_new(GridObject *parent, int winlet,
		GridBegin b, GridFlow f, GridEnd e, int mode);
	void GridInlet_delete(GridInlet *$);
	GridObject *GridInlet_parent(GridInlet *$);
	void GridInlet_abort(GridInlet *$);
	void GridInlet_set_factor(GridInlet *$, int factor);
	bool GridInlet_idle(GridInlet *$);

#define GridInlet_NEW3(_parent_,_class_,_winlet_) \
	GridInlet_new((GridObject *)_parent_, _winlet_, \
		((GridBegin)_class_##_##_winlet_##_begin), \
		 ((GridFlow)_class_##_##_winlet_##_flow), \
		  ((GridEnd)_class_##_##_winlet_##_end), 4)

#define GridInlet_NEW2(_parent_,_class_,_winlet_) \
	GridInlet_new((GridObject *)_parent_, _winlet_, \
		((GridBegin)_class_##_##_winlet_##_begin), \
		(/*(GridFlow2)*/(GridFlow)_class_##_##_winlet_##_flow2), \
		  ((GridEnd)_class_##_##_winlet_##_end), 6)

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
	bool GridOutlet_idle   (GridOutlet *$);

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
	fts_object_t o;  /* extends fts object */ \
	uint64 profiler_cumul, profiler_last; \
	GridInlet  * in[MAX_INLETS]; \
	GridOutlet *out[MAX_OUTLETS];

struct GridObject {
	GridObject_FIELDS;
};

	void GridObject_conf_class(fts_class_t *class, int winlet);

	METHOD2(GridObject,init);
	METHOD2(GridObject,grid_begin);
	METHOD2(GridObject,grid_flow);
	METHOD2(GridObject,grid_end);
	void GridObject_delete(GridObject *$);

/* **************************************************************** */
/* grid.c (part 4: formats) */

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
	GRID_BEGIN_(Format,(*begin));
	GRID_FLOW_( Format,(*flow));
	GRID_END_(  Format,(*end));

/* for both */
	/* for misc options */
	Format_option_m option;
	Format_close_m close;
};

#define Format_FIELDS \
	FormatClass *cl; \
	GridObject *parent; \
	Format *chain; \
	int mode; \
	int stream; \
	FILE *bstream; \
	BitPacking *bit_packing; \
	Dim *dim;

struct Format {
	Format_FIELDS;
};

extern FormatClass FORMAT_LIST( ,class_);
extern FormatClass *format_classes[];
FormatClass *FormatClass_find(const char *name);

Format *Format_open(FormatClass *qlass, GridObject *parent, int mode);

/* **************************************************************** */

extern Dict *gridflow_object_set;
extern Dict *gridflow_alarm_set;
extern fts_alarm_t *gridflow_alarm;
extern const char *whine_header;

#endif /* __GRIDFLOW_GRID_H */
