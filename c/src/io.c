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

/* ---------------------------------------------------------------- */

/* return and complain when file not open */
#define CHECK_FILE_OPEN \
	if (!$->ff) { whine("can't do that: file not open"); return; }

/* same with false return */
#define CHECK_FILE_OPEN2 \
	if (!$->ff) RAISE("can't do that: file not open");

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
	const char *format = fts_symbol_name(GET(0,symbol,SYM(ppm)));
	FormatClass *qlass = FormatClass_find(format);

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
	if (GridOutlet_busy($->out[0])) {
		whine("ignorine frame request: already waiting for a frame");
		return;
	}

	if (! $->ff->cl->frame($->ff,$->out[0],-1)) {
		whine("file format says: no, no, no");
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
	$->out[0] = GridOutlet_new((GridObject *)$, 0);
	$->ff = 0;
	$->timelog = 0; /* not used in @in yet */
	$->framecount = 0;
	gettimeofday(&$->tv,0);
}

METHOD(GridIn,delete) {
	if ($->ff) $->ff->cl->close($->ff);
	GridObject_delete((GridObject *)$);
}

CLASS(GridIn,
	DECL(GridIn,-1,init,  "s"),
	DECL(GridIn,-1,delete,""),
	DECL(GridIn, 0,bang,  "s"),
	DECL(GridIn, 0,reset, "s"),
	DECL(GridIn, 0,open,  "ssl"),
	DECL(GridIn, 0,close, ""),
//	DECL(GridIn, 0,frame, "si"),
	DECL(GridIn, 0,option,"ssi"))
{
	fts_class_init(class, sizeof(GridIn), 1, 1, 0);
	define_many_methods(class,ARRAY(GridIn_methods));
	return fts_Success;
}

/* ---------------------------------------------------------------- */

GRID_BEGIN(GridOut,0) {
	if (Dim_count(in->dim) != 3)
		RAISE("supports only exactly three dimensions");
	{
		CHECK_FILE_OPEN2
		in->dex=0;
		return $->ff->cl->begin($->ff, in);
	}
}

GRID_FLOW(GridOut,0) {
	Format *f = $->ff;
	CHECK_FILE_OPEN
	$->ff->cl->flow($->ff, in, n, data);
}

GRID_END(GridOut,0) {
	$->ff->cl->end($->ff,in);
	LEAVE;
	fts_outlet_send(OBJ($),0,fts_s_bang,0,0);
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
	int i;
/*
	sprintf_atoms(buf,ac,at);
	whine("open args = %s",buf);
	for (i=0; i<ac; i++) {
		sprintf_atoms(buf,1,at+i);
		whine("open arg %d = %s %s", i, fts_symbol_name((at+i)->type), buf);
	}
	return;
*/
	const char *format = fts_symbol_name(GET(0,symbol,SYM(ppm)));
	FormatClass *qlass = FormatClass_find(format);

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
	$->in[0] = GridInlet_NEW3($,GridOut,0);
	if (ac>1) {
		fts_atom_t at2[3];
		fts_set_symbol(at2+0,SYM(x11));
		fts_set_symbol(at2+1,SYM(here));
		GridOut_open($,winlet,selector,2,at2);
		fts_set_symbol(at2+0,SYM(out_size));
		fts_set_int(at2+1,GET(1,int,0));
		fts_set_int(at2+2,GET(2,int,0));
		GridOut_option($,winlet,selector,3,at2);
	}
}

METHOD(GridOut,delete) {
	if ($->ff) $->ff->cl->close($->ff);
	GridObject_delete((GridObject *)$);
}

CLASS(GridOut,
	DECL(GridOut,-1,init,  "s;ii"),
	DECL(GridOut,-1,delete,""),
	DECL(GridOut, 0,open,  "s;sss"),
	DECL(GridOut, 0,close, ""),
//	DECL(GridOut, 0,frame, "si"),
	DECL(GridOut, 0,option,"ssii"))
{
	fts_class_init(class, sizeof(GridOut), 1, 1, 0);
	GridObject_conf_class(class,0);
	define_many_methods(class,ARRAY(GridOut_methods));
	return fts_Success;
}

/* ---------------------------------------------------------------- */

#define INSTALL(_sym_,_name_) \
	fts_class_install(fts_new_symbol(_sym_),_name_##_class_init)

void startup_io (void) {
	INSTALL("@in",GridIn);
	INSTALL("@out",GridOut);
}
