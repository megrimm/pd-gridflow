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

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "grid.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include "grid.h"

int gf_max_packet_length = 1024;

#define INFO(foo) "!@#$"

/* **************** GridInlet ************************************* */

GridInlet *GridInlet_new(GridObject *parent, const GridHandler *gh) {
	GridInlet *$ = NEW(GridInlet,1);
	$->parent = parent;
	$->gh    = gh;
	$->dim   = 0;
	$->dex   = 0;
	$->factor= 1;
	$->buf   = 0;
	$->bufn  = 0;
	return $;
}

void GridInlet_delete(GridInlet *$) {
	FREE($->dim);
	FREE($->buf);
}

GridObject *GridInlet_parent(GridInlet *$) {
	return $->parent;
}

void GridInlet_abort(GridInlet *$) {
	if ($->dim) {
		whine("%s: aborting grid: %d of %d", INFO($), $->dex, Dim_prod($->dim));
		FREE($->dim);
		FREE($->buf);
	}
	$->dex = 0;
}

bool GridInlet_busy(GridInlet *$) {
	assert($);
	return !!$->dim;
}

bool GridInlet_busy_verbose(GridInlet *$, const char *where) {
	assert($);
	if (!$->dim) {
		whine("%s: (%s): no dim", INFO($), where);
	} else if (!$->gh->flow) {
		whine("%s: (%s): no flow()", INFO($), where);
	} else {
		return 1;
	}
	return 0;
}

void GridInlet_set_factor(GridInlet *$, int factor) {
	assert(factor > 0);
	assert(Dim_prod($->dim) % factor == 0);
	$->factor = factor;
	FREE($->buf);
	if (factor > 1) {
		$->buf = NEW(Number,factor);
		$->bufn = 0;
	}
}

void GridInlet_begin(GridInlet *$, int argc, VALUE *argv) {
	int i;
	int *v = NEW(int,argc-1);
	GridOutlet *back_out = (GridOutlet *) FIX2PTR(argv[0]);
	argc--, argv++;

	if (GridInlet_busy($)) {
		whine("grid inlet busy (aborting previous grid)");
		GridInlet_abort($);
	}

	if (argc-1>MAX_DIMENSIONS) {
		whine("too many dimensions (aborting grid)");
		goto err;
	}

	for (i=0; i<argc; i++) v[i] = NUM2INT(argv[i]);
	$->dim = Dim_new(argc,v);
	FREE(v);

	$->dex = 0;
	assert($->gh->begin);
	if (!$->gh->begin($->parent->peer,(GridObject *)$->parent,$)) goto err;

	GridOutlet_callback((GridOutlet *)back_out,$,$->gh->mode);
	return;
err:
	GridInlet_abort($);
}

void GridInlet_flow(GridInlet *$, int argc, VALUE *argv) {
	int n = NUM2INT(argv[0]);
	int mode = NUM2INT(argv[2]);

if (mode==4) {

	const Number *data = (Number *) FIX2PTR(argv[1]);
	if (!GridInlet_busy_verbose($,"flow")) return;
	assert(n>0);
	{
		int d = $->dex + $->bufn;
		if (d+n > Dim_prod($->dim)) {
			whine("%s: grid input overflow: %d of %d",
				INFO($), d+n, Dim_prod($->dim));
			n = Dim_prod($->dim) - d;
			if (n<=0) return;
		}
		if ($->buf && $->bufn) {
			while (n && $->bufn<$->factor) { $->buf[$->bufn++] = *data++; n--; }
			if ($->bufn == $->factor) {
				int newdex = $->dex + $->factor;
				if ($->gh->mode==6 && $->gh->flow) {
					Number *data2 = NEW2(Number,$->factor);
					memcpy(data2,$->buf,$->factor*sizeof(Number));
					((GridFlow2)$->gh->flow)($->parent->peer,(GridObject *)$->parent,$,$->factor,data2);
				} else {
					$->gh->flow($->parent->peer,(GridObject *)$->parent,$,$->factor,$->buf);
				}
				$->dex = newdex;
				$->bufn = 0;
			}
		}
		{
			int m = (n / $->factor) * $->factor;
			int newdex = $->dex + m;
			if (m) {
				if ($->gh->mode==6 && $->gh->flow) {
					Number *data2 = NEW2(Number,m);
					memcpy(data2,data,m*sizeof(Number));
					((GridFlow2)$->gh->flow)($->parent->peer,(GridObject *)$->parent,$,m,data2);
				} else {
					$->gh->flow($->parent->peer,(GridObject *)$->parent,$,m,data);
				}
			}
			$->dex = newdex;
			data += m;
			n -= m;
		}
		if ($->buf) { while (n) { $->buf[$->bufn++] = *data++; n--; }}
	}
} else if (mode==6) {
	Number *data = FIX2PTR(argv[1]);
	if (!GridInlet_busy_verbose($,"flow")) return;
	assert(n>0);
	assert($->factor==1);
	assert($->gh->flow);
	if ($->gh->mode==6) {
		((GridFlow2)$->gh->flow)($->parent->peer,(GridObject *)$->parent,$,n,data);
	} else if ($->gh->mode==4) {
		$->gh->flow($->parent->peer,(GridObject *)$->parent,$,n,data);
		FREE(data);
	}
} else {
	assert(0);
}}

void GridInlet_end(GridInlet *$, int argc, VALUE *argv) {
	if (!GridInlet_busy_verbose($,"end")) return;
/*	whine("%s: GridInlet_end()", INFO($)); */
	if (Dim_prod($->dim) != $->dex) {
		whine("%s: incomplete grid: %d of %d", INFO($),
			$->dex, Dim_prod($->dim));
	}
	if ($->gh->end) { $->gh->end($->parent->peer,(GridObject *)$->parent,$); }
	FREE($->dim);
	$->dex = 0;
}

void GridInlet_list(GridInlet *$, int argc, VALUE *argv) {
	int i;
	Number *v = NEW(Number,argc);
	int n = argc;
	whine("argc=%d",n);
	for (i=0; i<argc; i++) {
		whine("argv[%d]=%ld",i,argv[i],
			RSTRING(rb_funcall(argv[i],rb_intern("inspect"),0))->ptr);
		v[i] = NUM2INT(argv[i]);
		whine("v[%d]=%ld",i,v[i]);
	}
	$->dim = Dim_new(1,&n);

	assert($->gh->begin);
	if ($->gh->begin($->parent->peer,(GridObject *)$->parent,$)) {
		$->gh->flow($->parent->peer,(GridObject *)$->parent,$,n,v);
		if ($->gh->end) { $->gh->end($->parent->peer,(GridObject *)$->parent,$); }
	} else {
		GridInlet_abort($);
	}

	FREE(v);
	FREE($->dim);
	$->dex = 0;
}

/* **************** GridOutlet ************************************ */

GridOutlet *GridOutlet_new(GridObject *parent, int woutlet) {
	GridOutlet *$ = NEW(GridOutlet,1);
	$->parent = parent;
	$->woutlet = woutlet;
	$->dim = 0;
	$->dex = 0;
	$->buf = NEW2(Number,gf_max_packet_length);
	$->bufn = 0;
	$->frozen = 0;
	$->ron = 0;
	$->ro = 0;
	$->rwn = 0;
	$->rw = 0;
	return $;
}

void GridOutlet_delete(GridOutlet *$) {
	FREE($->dim);
	FREE($->buf);
}

GridObject *GridOutlet_parent(GridOutlet *$) {
	assert($);
	return $->parent;
}

bool GridOutlet_busy(GridOutlet *$) {
	assert($);
	return !!$->dim;
}

void GridOutlet_abort(GridOutlet *$) {
	assert($);
	if (!GridOutlet_busy($)) return;
	FREE($->dim);
	$->dim = 0;
	$->dex = 0;
	LEAVE_P;
	{
		VALUE a[] = { INT2NUM($->woutlet), sym_grid_end };
		FObject_send_out(COUNT(a),a,$->parent->peer);
	}
	ENTER_P;
}

void GridOutlet_end(GridOutlet *$) {
	assert($);
	assert(GridOutlet_busy($));
	GridOutlet_flush($);
	LEAVE_P;
	{
		VALUE a[] = { INT2NUM($->woutlet), sym_grid_end };
		FObject_send_out(COUNT(a),a,$->parent->peer);
	}
	ENTER_P;
	FREE($->dim);
	$->dim = 0;
	$->dex = 0;
}

void GridOutlet_begin(GridOutlet *$, Dim *dim) {
	int i;
	int n = Dim_count(dim);
	VALUE a[MAX_DIMENSIONS+3];

	assert($);

	/* if (GridOutlet_busy($)) GridOutlet_abort($); */

	$->dim = dim;
	$->dex = 0;
	$->frozen = 0;
	$->ron = 0;
	$->ro  = 0;
	$->rwn = 0;
	$->rw  = 0;
	a[0] = INT2NUM($->woutlet);
	a[1] = sym_grid_begin;
	a[2] = PTR2FIX($);
	for(i=0; i<n; i++) a[3+i] = INT2NUM(Dim_get($->dim,i));
	LEAVE_P;
	FObject_send_out(n+3,a,$->parent->peer);
	ENTER_P;
	$->frozen = 1;
/*	whine("$ = %p; $->ron = %d; $->rwn = %d", $, $->ron, $->rwn); */
}

void GridOutlet_send_direct(GridOutlet *$, int n, const Number *data) {
	assert(GridOutlet_busy($));
	while (n>0) {
		int pn = min(n,gf_max_packet_length);
		VALUE a[] = {
			INT2NUM($->woutlet),
			sym_grid_flow,
			INT2NUM(pn),
			PTR2FIX(data), /* explicitly removing const */
			INT2NUM(4), /* mode ro */
		};
		LEAVE_P;
		FObject_send_out(COUNT(a),a,$->parent->peer);
		ENTER_P;
		data += pn;
		n -= pn;
	}
}

void GridOutlet_send(GridOutlet *$, int n, const Number *data) {
	assert(GridOutlet_busy($));
	$->dex += n;
	assert($->dex <= Dim_prod($->dim));
	if (n > gf_max_packet_length/2 || $->bufn + n > gf_max_packet_length) {
		GridOutlet_flush($);
	}
	if (n > gf_max_packet_length/2) {
		GridOutlet_send_direct($,n,data);
	} else {
		memcpy(&$->buf[$->bufn],data,n*sizeof(Number));
		$->bufn += n;
	}
}

void GridOutlet_give(GridOutlet *$, int n, Number *data) {
	assert(GridOutlet_busy($));
	$->dex += n;
	assert($->dex <= Dim_prod($->dim));
	GridOutlet_flush($);
	if ($->ron == 0 && $->rwn == 1) {
		/* this is the copyless buffer passing */
		VALUE a[] = {
			INT2NUM(0),
			sym_grid_flow,
			INT2NUM(n),
			PTR2FIX(data),
			INT2NUM(6), /* mode rw */
		};
		LEAVE_P;
		FObject_send_out(COUNT(a),a,$->parent->peer);
		ENTER_P;
	} else {
		/* normal stuff */
		GridOutlet_send_direct($,n,data);
		FREE(data);
	}
}

void GridOutlet_flush(GridOutlet *$) {
	assert(GridOutlet_busy($));
	GridOutlet_send_direct($,$->bufn,$->buf);
	$->bufn = 0;
}

void GridOutlet_callback(GridOutlet *$, GridInlet *in, int mode) {
	assert(GridOutlet_busy($));
	assert(!$->frozen);
	assert(mode==6 || mode==4);
	/* whine("callback: outlet=%p, inlet=%p, mode=%d",$,in,mode); */
	/* not using ->ro, ->rw yet */
	if (mode==4) $->ron += 1;
	if (mode==6) $->rwn += 1;
}

/* **************** GridObject ************************************ */
/*
  abstract class for an FTS Object that has Grid-aware inlets/outlets
*/

void GridObject_init(GridObject *$) {
	int i;
	for (i=0; i<MAX_INLETS;  i++) $->in[i]  = 0;
	for (i=0; i<MAX_OUTLETS; i++) $->out[i] = 0;
	/* Dict_put(gf_object_set,$,0); */
	$->profiler_cumul = 0;

	{
		GridClass *cl = $->grid_class;
		whine("cl=%p",cl);
		for (i=0; i<cl->handlersn; i++) {
			GridHandler *gh = &cl->handlers[i];
			$->in[gh->winlet] = GridInlet_new($,gh);
		}
		for (i=0; i<cl->outlets; i++) {
			$->out[i] = GridOutlet_new($,i);
		}
	}
}

/* category: input */

METHOD(GridObject,method_missing) {
	char *name;
	whine("argc=%d,argv=%p,self=%p",argc,argv,self);
	if (argc<1) RAISE("not enough arguments");
	if (!SYMBOL_P(argv[0])) RAISE("expected symbol");
	whine("method_missing: %s",rb_sym_name(argv[0]));
	name = rb_sym_name(argv[0]);
	rb_funcall2(rb_cObject,rb_intern("p"),argc,argv);
	if (strlen(name)>3 && name[0]=='_' && name[2]=='_' && isdigit(name[1])) {
		int i = name[1]-'0';
		GridInlet *inl = $->in[i];
		argc--, argv++;
		if      (strcmp(name+3,"grid_begin")==0) return GridInlet_begin(inl,argc,argv);
		else if (strcmp(name+3,"grid_flow" )==0) return GridInlet_flow( inl,argc,argv);
		else if (strcmp(name+3,"grid_end"  )==0) return GridInlet_end(  inl,argc,argv);
		else if (strcmp(name+3,"list"      )==0) return GridInlet_list( inl,argc,argv);
		argc++, argv--;
	}
	rb_call_super(argc,argv);
}

void GridObject_delete(GridObject *$) {
	int i;
	for (i=0; i<MAX_INLETS;  i++) {
		if ($->in[i]) {
			GridInlet_delete($->in[i]);
			FREE($->in[i]);
		}
	}
	for (i=0; i<MAX_OUTLETS; i++) {
		if ($->out[i]) {
			GridOutlet_delete($->out[i]);
			FREE($->out[i]);
		}
	}
/*
	Dict_del(gf_object_set,$);
	if (Dict_size(gf_object_set)==0) {
		whine("all objects freed");
		qdump();
	}
*/
}

void GridObject_conf_class(VALUE $, int winlet) {
/*	MethodDecl methods[] = {
		DECL(GridObject,winlet,grid_begin,"spi+"),
		DECL(GridObject,winlet,grid_flow, "sip"),
		DECL(GridObject,winlet,grid_end,  ""),
		DECL(GridObject,winlet,list,      "l"),
	};
*/
	MethodDecl methods[] = {
		DECL(GridObject,-1,method_missing,"+"),
	};
	define_many_methods($,COUNT(methods),methods);
}

void GridObject_conf_class2(VALUE $, GridClass *grclass) {
	int i;
	define_many_methods($,grclass->methodsn,grclass->methods);
	for (i=0; i<grclass->handlersn; i++) {
		GridObject_conf_class($,grclass->handlers[i].winlet);
	}
}  


/* **************** Stream **************************************** */

Stream *Stream_open_file(const char *filename, int mode) {
	Stream *$;
	int fd = fileno(fopen(filename,mode==4?"r":mode==2?"w":""));
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
VALUE format_classes_dex = Qnil;

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

/* **************************************************************** */

void startup_grid (void) {
	int i;
	whine("format-list-begin");
	rb_define_readonly_variable("$format_classes_dex",&format_classes_dex);
	format_classes_dex = rb_hash_new();
	for (i=0; i<COUNT(format_classes); i++) {
		FormatClass *fc = format_classes[i];
		whine("  %10s %s",fc->symbol_name, fc->long_name);
		rb_hash_aset(format_classes_dex, ID2SYM(rb_intern(fc->symbol_name)),
			PTR2FIX(fc));
	}
	whine("format-list-end");
}
