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

#include "../base/grid.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern FormatClass class_FormatPPM;

typedef Format FormatPPM;

METHOD(FormatPPM,frame) {
	char buf[256];
	int metrics[6],n=0;
	FILE *f = Stream_get_file($->st);

	fgets(buf,256,f);
	if (feof(f)) {
		fseek(f,0,SEEK_SET);
		fgets(buf,256,f);
	}

	if(strcmp("P6\n",buf)!=0) RAISE("Wrong format (needing PPM P6)");
	while (n<3) {
		fgets(buf,256,f);
		if (*buf=='#') continue; /* skipping a comment line */
		n += sscanf(buf,"%d%d%d",metrics+n,metrics+n+1,metrics+n+2);
		if (feof(f)) RAISE("unexpected end of file in header");
	}
/*	whine("File metrics: %d %d %d",metrics[0],metrics[1],metrics[2]); */
	if(metrics[2] != 255) RAISE("Wrong color depth (max_value=%d instead of 255)",metrics[2]);
	{
		int v[] = { metrics[1], metrics[0], 3 };
		GridOutlet_begin($->out[0], Dim_new(3,v));
	}

	{
		int y;
		int bs = Dim_prod_start($->out[0]->dim,1);
		uint8 b1[bs];
		Number b2[bs];
		for (y=0; y<metrics[1]; y++) {
			int i;
			int bs2 = (int) fread(b1,1,bs,f);
			if (bs2 < bs) {
				whine("unexpected end of file: bs=%d; bs2=%d",bs,bs2);
			}
			/* should use bitpacking instead... */
			for (i=0; i<bs; i++) b2[i] = b1[i];
			GridOutlet_send($->out[0],bs,b2);
		}
	}
	GridOutlet_end($->out[0]);
}

GRID_BEGIN(FormatPPM,0) {
	FILE *f = Stream_get_file($->st);
	fprintf(f,
		"P6\n"
		"# generated using GridFlow " GF_VERSION "\n"
		"%d %d\n"
		"255\n",
		Dim_get(in->dim,1),
		Dim_get(in->dim,0));

	fflush(f);
	return true;
}

/* !@#$ use BitPacking here */
GRID_FLOW(FormatPPM,0) {
	FILE *f = Stream_get_file($->st);
	uint8 data2[n];
	int i;
	for (i=0; i<n; i++) data2[i] = data[i];
	fwrite(data2,1,n,f);
}

GRID_END(FormatPPM,0) {
	FILE *f = Stream_get_file($->st);
	fflush(f);
	fseek(f,0,SEEK_SET);
}

METHOD(FormatPPM,close) {
	if ($->st) Stream_close($->st);
	rb_call_super(argc,argv);
}

METHOD(FormatPPM,init) {
	const char *filename;
	rb_call_super(argc,argv);
	argv++, argc--;

	if (argc!=2 || argv[0] != SYM(file)) RAISE("usage: ppm file <filename>");
	filename = rb_sym_name(argv[1]);

	$->st = Stream_open_file(filename,$->mode);
	if (!$->st) RAISE("can't open file `%s': %s", filename, strerror(errno));
}

FMTCLASS(FormatPPM,"ppm","Portable PixMap",FF_R|FF_W,
inlets:1,outlets:1,LIST(GRINLET(FormatPPM,0)),
DECL(FormatPPM,init),
DECL(FormatPPM,frame),
DECL(FormatPPM,close))
