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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "grid.h"
#include <fcntl.h>
#include <errno.h>

/* ---------------------------------------------------------------- */

/* return and complain when file not open */
#define CHECK_FILE_OPEN \
	if (!$->ff) { whine("can't do that: file not open"); return; }

/* same with false return */
#define CHECK_FILE_OPEN2 \
	if (!$->ff) RAISE("can't do that: file not open");

/* **************** Stream **************************************** */

Stream *Stream_open_file(const char *filename, int mode) {
	Stream *$;
	int fd = fts_file_open(filename,mode==4?"r":mode==2?"w":"");
	if (fd<0) return 0;
	$ = NEW(Stream,1);
	$->fd = fd;
	$->file = fdopen(fd,mode==4?"r":mode==2?"w":"");
	if (!$->file) return 0;
	$->buf = 0; $->buf_i = $->buf_n = 0;
	$->on_read = $->target = 0;
	return $;
}

Stream *Stream_open_fd(int fd, int mode) {
	Stream *$ = NEW(Stream,1);
	$->fd = fd;
	$->file = fdopen(fd,mode==4?"r":mode==2?"w":"");
	if (!$->file) return 0;
	$->buf = 0; $->buf_i = $->buf_n = 0;
	$->on_read = $->target = 0;
	return $;
}

void Stream_nonblock(Stream *$) {
	int flags = fcntl($->fd,F_GETFL);
	flags |= O_NONBLOCK;
	fcntl($->fd,F_SETFL,flags);
}

int Stream_get_fd(Stream *$) {
	return $->fd;
}

FILE *Stream_get_file(Stream *$) {
/*	if (!$->file) $->file = fdopen($->file,mode); */
	return $->file;
}

int Stream_read(Stream *$, int n, char *buf) {
	assert(0);
}

void Stream_on_read_do(Stream *$, int n, OnRead on_read, void *target) {
	FREE($->buf);
	$->buf = NEW(char,n);
	$->buf_i = 0;
	$->buf_n = n;
	$->on_read = on_read;
	$->target = target;
}

bool Stream_try_read(Stream *$) {
	if (!$->buf) {
		whine("Stream_try_read has nothing to do",$);
		return true;
	}
	while ($->buf) {
		int n = read($->fd,$->buf+$->buf_i,$->buf_n-$->buf_i);
		if (n<=0) {
			if (n==0 || errno==EAGAIN || errno==EWOULDBLOCK) {
				whine("(trying again later)");
			} else {
				whine("Stream_try_read: %s", strerror(errno));
			}
			return true;
		} else {
			$->buf_i += n;
		}
		if ($->buf_i == $->buf_n) {
			char *buf = $->buf;
			$->buf = 0;
			if (!$->on_read($->target,$->buf_n,buf)) return false;
		}
	}
	return true;
}

bool Stream_is_waiting(Stream *$) {
	return !!$->buf;
}

void Stream_close(Stream *$) {
	if ($->file) fclose($->file);
	else close($->fd);
	FREE($);
}

/* **************** Format **************************************** */
/* this is an abstract base class for file formats, network protocols, etc */

FormatClass *format_classes[] = { FORMAT_LIST(&,class_) };
Dict *format_classes_dex;

Format *Format_open(FormatClass *qlass, GridObject *parent, int mode) {
	Format *$ = (Format *) NEW(char,qlass->object_size);
	$->cl = qlass;
	$->parent = parent;
	$->chain = 0;
	$->mode = mode;
	$->st = 0;
	$->bit_packing = 0;
	$->dim = 0;

	/* FF_W, FF_R, FF_RW */
	if (mode==2 || mode==4 || mode==6) {
		if (! (qlass->flags & (1 << (mode/2)))) {
			whine("Format %s does not support mode '%s'",
				qlass->symbol_name,
				format_flags_names[mode/2]);
			return 0;
		}
	} else {
		whine("Format opening mode is incorrect");
		return 0;
	}

	return $;
}

void Format_close(Format *$) {
	FREE($->bit_packing);
	FREE($->dim);
	FREE($);
}

/* **************** FormatClass *********************************** */

const char *format_flags_names[] = {
	"(0<<1)",
	"FF_W: write",
	"FF_R: read",
	"FF_RW: read_write",
};

int format_flags_n = sizeof(format_flags_names)/sizeof(const char *);

/* ---------------------------------------------------------------- */
/* some data/type decls */

typedef struct GridIn {
	GridObject_FIELDS;
	Format *ff; /* a file reader object */
	bool timelog; /* future use */
	struct timeval tv; /* future use */
	int framecount;
} GridIn;

typedef struct GridOut {
	GridObject_FIELDS;
	Format *ff; /* a file writer object */
	bool timelog;
	struct timeval tv;   /* time of the last grid_end */
	int framecount;
} GridOut;

/* ---------------------------------------------------------------- */

static void GridIn_p_reset(GridIn *$) {
	GridOutlet_abort($->out[0]);
	CHECK_FILE_OPEN
	/* $->ff->cl->frame($->ff,-1); */
}

METHOD(GridIn,close) {
	CHECK_FILE_OPEN
	$->ff->cl->close($->ff);
	$->ff = 0;
}

METHOD(GridIn,reset) {
	GridIn_p_reset($);
}

METHOD(GridIn,open) {
	const char *format = Symbol_name(GET(0,symbol,SYM(ppm)));
	FormatClass *qlass = Dict_get(format_classes_dex,format);

	if (qlass) {
		whine("file format: %s (%s)",qlass->symbol_name, qlass->long_name);
	} else {
		whine("unknown file format identifier: %s", format);
		return;
	}

	if ($->ff) $->ff->cl->close($->ff);
	if (GridOutlet_busy($->out[0])) GridOutlet_abort($->out[0]);
	if (qlass->open) {
		$->ff = qlass->open(qlass,(GridObject *)$,4,ac-1,at+1);
	} else {
		whine("file format has no `open'");
	}
}

METHOD(GridIn,bang) {
	CHECK_FILE_OPEN
	/* this is not a sufficient check. see format_grid.c */
	if (GridOutlet_busy($->out[0])) {
		whine("ignoring frame request: already waiting for a frame");
		return;
	}

	if (! $->ff->cl->frame($->ff,$->out[0],-1)) {
		whine("file format package '%s' reported error",$->ff->cl->symbol_name);
		goto err;
	}
	return;
err:
	if (GridOutlet_busy($->out[0])) GridOutlet_abort($->out[0]);
	return;
}

METHOD(GridIn,option) {
	CHECK_FILE_OPEN
	if ($->ff->cl->option) {
		$->ff->cl->option($->ff,ac,at);
	} else {
		whine("this format has no options");
	}
}

METHOD(GridIn,init) {
	GridObject_init((GridObject *)$);
	$->ff = 0;
	$->timelog = 0; /* not used in @in yet */
	$->framecount = 0;
	gettimeofday(&$->tv,0);
}

METHOD(GridIn,delete) {
	if ($->ff) $->ff->cl->close($->ff);
	GridObject_delete((GridObject *)$);
}

GRCLASS(GridIn,inlets:1,outlets:1,
LIST(),
	DECL(GridIn,-1,init,  "s"),
	DECL(GridIn,-1,delete,""),
	DECL(GridIn, 0,bang,  "s"),
	DECL(GridIn, 0,reset, "s"),
	DECL(GridIn, 0,open,  "ssl"),
	DECL(GridIn, 0,close, ""),
//	DECL(GridIn, 0,frame, "si"),
	DECL(GridIn, 0,option,"ssi"))

/* ---------------------------------------------------------------- */

GRID_BEGIN(GridOut,0) {
	if (Dim_count(in->dim) != 3)
		RAISE("supports only exactly three dimensions");
	{
		CHECK_FILE_OPEN2
		in->dex=0;
		return $->ff->cl->handler->begin((GridObject *)($->ff),in);
	}
}

GRID_FLOW(GridOut,0) {
	CHECK_FILE_OPEN
	$->ff->cl->handler->flow((GridObject *)($->ff),in,n,data);
}

GRID_END(GridOut,0) {
	$->ff->cl->handler->end((GridObject *)($->ff),in);
	LEAVE;
	Object_send_thru(OBJ($),0,sym_bang,0,0);
	if (!$->timelog) return;
	{
		struct timeval t;
		gettimeofday(&t,0);
		whine("@out:0:end: frame#%03d time: %6d.%03d s; minus last: %5d ms)\n",
			$->framecount,
			t.tv_sec%1000000, t.tv_usec/1000,
			(t.tv_sec-$->tv.tv_sec)*1000 + (t.tv_usec-$->tv.tv_usec)/1000);
		memcpy(&$->tv,&t,sizeof(struct timeval));
	}
	$->framecount+=1;
	ENTER;
}

METHOD(GridOut,option) {
	Symbol sym = GET(0,symbol,SYM(foo));
	CHECK_FILE_OPEN
	if (sym == SYM(timelog)) {
		$->timelog = GET(1,int,0);
		COERCE_INT_INTO_RANGE($->timelog,0,1);
		whine("timelog = %d",$->timelog);
	} else if ($->ff->cl->option) {
		$->ff->cl->option($->ff,ac,at);
	} else {
		whine("this format has no options");
	}
}

METHOD(GridOut,close) {
	CHECK_FILE_OPEN
	if (GridOutlet_busy($->out[0])) GridOutlet_abort($->out[0]);
	$->ff->cl->close($->ff);
}

METHOD(GridOut,open) {
	char buf[256];
/*
	int i;
	sprintf_vars(buf,ac,at);
	whine("open args = %s",buf);
	for (i=0; i<ac; i++) {
		sprintf_vars(buf,1,at+i);
		whine("open arg %d = %s %s", i, Symbol_name((at+i)->type), buf);
	}
	return;
*/
	const char *format = Symbol_name(GET(0,symbol,SYM(ppm)));
	FormatClass *qlass = Dict_get(format_classes_dex,format);

	if (qlass) {
		whine("file format: %s (%s)",qlass->symbol_name, qlass->long_name);
	} else {
		whine("unknown file format identifier: %s", format);
		return;
	}

	if ($->ff) $->ff->cl->close($->ff);
	if (qlass->open) {
		$->ff = qlass->open(qlass,(GridObject *)$,2,ac-1,at+1);
	} else {
		whine("file format has no `open'");
	}
}

METHOD(GridOut,init) {
	$->framecount = 0;
	$->timelog = 0;
	$->ff = 0;
	gettimeofday(&$->tv,0);
	GridObject_init((GridObject *)$);
	if (ac>1) {
		Var at2[3];
		Var_put_symbol(at2+0,SYM(x11));
		Var_put_symbol(at2+1,SYM(here));
		GridOut_open($,winlet,selector,2,at2);
		Var_put_symbol(at2+0,SYM(out_size));
		Var_put_int(at2+1,GET(1,int,0));
		Var_put_int(at2+2,GET(2,int,0));
		GridOut_option($,winlet,selector,3,at2);
	}
}

METHOD(GridOut,delete) {
	if ($->ff) $->ff->cl->close($->ff);
	GridObject_delete((GridObject *)$);
}

GRCLASS(GridOut,inlets:1,outlets:1,
LIST(GRINLET(GridOut,0)),
/* outlet 0 not used for grids */
	DECL(GridOut,-1,init,  "s;ii"),
	DECL(GridOut,-1,delete,""),
	DECL(GridOut, 0,open,  "s;sss"),
	DECL(GridOut, 0,close, ""),
/*	DECL(GridOut, 0,frame, "si"), */
	DECL(GridOut, 0,option,"ssii"))

/* ---------------------------------------------------------------- */

#define INSTALL(_sym_,_name_) \
	fts_class_install(Symbol_new(_sym_),_name_##_class_init)

void startup_io (void) {
	int i;
	format_classes_dex = Dict_new((CompFunc)strcmp,HashFunc_string);
	for (i=0; i<COUNT(format_classes); i++) {
		Dict_put(format_classes_dex,
			format_classes[i]->symbol_name,
			format_classes[i]);
	}
	INSTALL("@in",GridIn);
	INSTALL("@out",GridOut);
}

