/*
	$Id$

	Video4jmax
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
#ifndef __GRID_PROTOCOL_H
#define __GRID_PROTOCOL_H

#include <pthread.h>
#include <assert.h>
#include "config.h"
#include "macros.h"

/* current version number as string literal */
#define VIDEO4JMAX_VERSION "0.2.1"
#define VIDEO4JMAX_COMPILE_TIME __DATE__ ", " __TIME__

/* **************************************************************** */
/* general purpose stuff */

#ifdef NO_ASSERT
	/* disabling assertion checking */
	#undef assert
	#define assert(_foo_)
#endif

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
typedef char  int8;
typedef short int16;
typedef long  int32;
typedef float  float32;
typedef double float64;

int high_bit(unsigned long n);
int low_bit(unsigned long n);

void *qalloc(          size_t n); /*0xdeadbeef*/
void qfree(void *data, size_t n); /*0xfadedf00*/

/*
  a remainder function such that floor(a/b)*b+mod(a,b) = a
  in contrast to C-language builtin a%b,
  this one has uniform behaviour around zero.
*/
static inline int mod(int a, int b) { if (a<0) a += b * (1-(a/b)); return a%b; }

/* **************************************************************** */
/* jMax specific */

void whine(char *fmt, ...);
int    v4j_file_open(const char *name, int mode);
FILE *v4j_file_fopen(const char *name, int mode); 

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
/* Video4jmax specific */

/* not used for now
typedef enum {
	t_uint8,
	t_uint16,
	t_uint32,
unsupported:
	t_int8,
	t_int16,
	t_int32,
	t_float32,
	t_float64,
	t_complex64,
	t_complex128,
} ArrayType;

typedef struct {
	int type;
	void *buffer;
} Array;
*/

/* **************************************************************** */

#define DECL_SYM(_sym_) \
	extern fts_symbol_t sym_##_sym_;

DECL_SYM(open)
DECL_SYM(close)
DECL_SYM(reset)
DECL_SYM(autodraw)
DECL_SYM(grid_begin)
DECL_SYM(grid_flow)
DECL_SYM(grid_end)


/* used as maximum width, maximum height, etc. */
#define MAX_INDICES 2000

/* maximum number of dimensions in an array */
#define MAX_DIMENSIONS 4

/* 1 + maximum id of last grid-aware inlet/outlet */
#define MAX_INLETS 2
#define MAX_OUTLETS 2

/*
  what kind of number a Grid contains.
  note that on the other hand, indexing and dimensioning of Grids is
  still done with explicit ints.
*/
typedef long Number;

/* number of (maximum,ideal) Numbers to send at once */
#define PACKET_LENGTH (1*1024)

/* options */

#if 0
  /* don't use this, doesn't work yet */
  #define VIDEO_OUT_SHM
  /* don't use this, does not work yet. */
  #define GRID_USE_INLINE
#endif

/* #define NO_ASSERT */
/* #define NO_DEADBEEF */

/* **************************************************************** */

/*
  a const array that holds dimensions of a grid
  and can do some calculations on positions in that grid.
*/

typedef struct Dim Dim;
struct Dim {
	int n;
	int v[1];
};

	Dim *Dim_new(int n, int *v);
	int Dim_count(Dim *$);
	int Dim_get(Dim *$, int i);
	int Dim_prod(Dim *$);
	int Dim_prod_start(Dim *$, int start);

	char *Dim_to_s(Dim *$);
	int Dim_equal_verbose(Dim *$, Dim *other);
	int Dim_equal_verbose_hwc(Dim *$, Dim *other);
	int Dim_dex_add(Dim *$, int n, int *dex);
	int Dim_calc_dex(Dim *$, int *v);

/*
	#ifdef GRID_USE_INLINE
		#define PROC static inline
		#include "dim.c"
		#undef PROC		
	#endif
*/

/* GridInlet represents a grid-aware jmax inlet */

typedef struct GridInlet GridInlet;
typedef struct GridObject GridObject;

#define GRID_BEGIN_(_class_,_name_) void _name_(_class_ *parent, GridInlet *$)
#define  GRID_FLOW_(_class_,_name_) void _name_(_class_ *parent, GridInlet *$, int n, const Number *data)
#define   GRID_END_(_class_,_name_) void _name_(_class_ *parent, GridInlet *$)

typedef GRID_BEGIN_(GridObject, (*GridBegin));
typedef  GRID_FLOW_(GridObject, (*GridFlow));
typedef   GRID_END_(GridObject, (*GridEnd));

#define GRID_BEGIN(_class_,_inlet_) GRID_BEGIN_(_class_,_class_##_##_inlet_##_begin)
#define  GRID_FLOW(_class_,_inlet_)  GRID_FLOW_(_class_,_class_##_##_inlet_##_flow)
#define   GRID_END(_class_,_inlet_)   GRID_END_(_class_,_class_##_##_inlet_##_end)

struct GridInlet {
	GridObject *parent;
	int winlet;

	Dim *dim;
	int dex;

//	Number *buf; /* packet buffer */

	/* grid reception functions */
	GridBegin begin;
	GridFlow  flow;
	GridEnd   end;

	int count; /* how many Numbers transferred */
};

	GridInlet *GridInlet_new(GridObject *parent, int winlet,
		GridBegin a, GridFlow p);
	GridObject *GridInlet_parent(GridInlet *$);
	void GridInlet_abort(GridInlet *$);

/* **************************************************************** */
/* GridOutlet represents a grid-aware jmax outlet */

typedef struct GridOutlet GridOutlet;

struct GridOutlet {
	GridObject *parent;
	int woutlet;

	Dim *dim;
	int dex;

	Number *buf; /* packet buffer */
	int bufn;

	int count; /* how many Numbers transferred */
};

	GridOutlet *GridOutlet_new(GridObject *parent, int woutlet);
	GridObject *GridOutlet_parent(GridOutlet *$);
	int  GridOutlet_idle   (GridOutlet *$);
	void GridOutlet_abort  (GridOutlet *$);
	void GridOutlet_begin  (GridOutlet *$, Dim *dim);
	int  GridOutlet_send   (GridOutlet *$, int n, const Number *data);
	int  GridOutlet_send_buffered(GridOutlet *$, int n, const Number *data);
	void GridOutlet_flush  (GridOutlet *$);
	void GridOutlet_end    (GridOutlet *$);

/* **************************************************************** */
/* GridPacket
	** for future use **
	represents a smart buffer bound to a GridOutlet and N GridInlets
*/

struct GridPacket {
	GridOutlet *parent;
	pthread_mutex_t *mutex;
	Number *(*request)(void);
	Number *buf;
	int bufn;
};

/*
	GridPacket *GridPacket_new(GridOutlet *parent, int n, Number *data);
	int GridPacket_size(GridPacket *$);
	Number *GridPacket_data(GridPacket *$);
	void GridPacket_delete(GridPacket *$);
*/

/* **************************************************************** */

#define GridObject_FIELDS \
	fts_object_t o;  /* extends fts object */ \
	GridInlet  * in[MAX_INLETS]; \
	GridOutlet *out[MAX_OUTLETS];

struct GridObject {
	GridObject_FIELDS;
};

	void GridObject_conf_class(fts_class_t *class, int winlet);

	METHOD(GridObject,init);
	METHOD(GridObject,grid_begin);
	METHOD(GridObject,grid_flow);
	METHOD(GridObject,grid_end);

/* **************************************************************** */

typedef enum FileFormatFlags {dummy_dummy} FileFormatFlags;
typedef struct FileFormatClass FileFormatClass;
typedef struct FileFormat FileFormat;

struct FileFormatClass {
	const char *symbol_name; /* short identifier */
	const char *long_name; /* long identifier */
	FileFormatFlags flags; /* future use (set to 0 please) */

	/*
	  mode=4 is reading; mode=2 is writing;
	  other values are not used yet (not even 6)
	*/
	FileFormat *(*open)(const char *filename, int mode);

	/*
	  for future use
	*/
	FileFormat *(*connect)(const char *proto, const char *target, int mode);
	FileFormat *(*chain_to)(FileFormat *other);
};

/*
	The Read aspect of a format:
		frames - how many frames are there
			(optional method)
		frame - select a frame, prepare for reading,
			get a dimensions descriptor.
			if frame number is -1, pick up next frame; reloop after last.
			if frames() is not implemented, you must use -1 as
			a frame number.
		size - decide which (height,width) should incoming images have
			(optional method)
		read - read actual data: n Numbers.
		
	The Write aspect of a format:
		begin - similar to GridInlet's
		flow  - similar to GridInlet's
		end   - similar to (future) GridInlet's

	Common aspect:
		close
*/

#define FileFormat_FIELDS \
	FileFormatClass *qlass; \
	int stream_raw; \
	FILE *stream; \
	FileFormat *chain; \
	Dim *dim; \
	int left; \
	void *stuff; \
	\
	int    (*frames) (FileFormat *$); \
	Dim   *(*frame)  (FileFormat *$, int frame); \
	void   (*size)   (FileFormat *$, int height, int width); \
	Number*(*read)   (FileFormat *$, int n); \
	\
	void   (*begin)(FileFormat *$, Dim *dim); \
	void   (*flow) (FileFormat *$, int n, const Number *data); \
	void   (*end)  (FileFormat *$); \
	\
	void   (*config) (FileFormat *$, fts_symbol_t *sym, int value); \
	void   (*close)  (FileFormat *$);

struct FileFormat {
	FileFormat_FIELDS;
};

extern FileFormatClass FILE_FORMAT_LIST( );
extern FileFormatClass *file_format_classes[];
FileFormatClass *FileFormatClass_find(const char *name);

#endif /* __GRID_PROTOCOL_H */
