/*
	$Id$

	Video4jmax
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file LICENSE for further informations on licensing terms.

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

#include "video4jmax.h"

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

/* don't use this, doesn't work yet
	#define VIDEO_OUT_SHM */
/* don't use this, does not work yet.
	#define GRID_USE_INLINE */
/* #define NO_ASSERT */
/* #define NO_DEADBEEF */

/* **************************************************************** */

fts_type_t *make_dims_list (void);

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

typedef void (*GridBegin)(GridInlet *$);
typedef void (*GridFlow)(GridInlet *$, int n, const Number *data);
typedef void (*GridEnd)(GridInlet *$); /* not there yet! */

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
