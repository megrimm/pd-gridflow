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

#include "grid.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern FormatClass class_FormatPPM;

typedef Format FormatPPM;

bool FormatPPM_frame (Format *$, GridOutlet *out, int frame) {
	char buf[256];
	int metrics[6],n=0;

	if (frame!=-1) return 0;
	fgets(buf,256,$->bstream);
	if (feof($->bstream)) {
		fseek($->bstream,0,SEEK_SET);
		fgets(buf,256,$->bstream);
	}

	if(strcmp("P6\n",buf)!=0) {
		whine("Wrong format (needing PPM P6)");
		goto err;
	}
	while (n<3) {
		fgets(buf,256,$->bstream);
		if (*buf=='#') continue; /* skipping a comment line */
		n += sscanf(buf,"%d%d%d",metrics+n,metrics+n+1,metrics+n+2);
		if (feof($->bstream)) {
			whine("unexpected end of file in header");
			goto err;
		}
	}
/*	whine("File metrics: %d %d %d",metrics[0],metrics[1],metrics[2]); */
	if(metrics[2] != 255) {
		whine("Wrong color depth (max_value=%d instead of 255)",metrics[2]);
		goto err;
	}
	{
		int v[] = { metrics[1], metrics[0], 3 };
		GridOutlet_begin(out, Dim_new(3,v));
	}

	{
		int y;
		int bs = Dim_prod_start(out->dim,1);
		uint8 b1[bs];
		Number b2[bs];
		for (y=0; y<metrics[1]; y++) {
			int i;
			int bs2 = (int) fread(b1,1,bs,$->bstream);
			if (bs2 < bs) {
				whine("unexpected end of file: bs=%d; bs2=%d",bs,bs2);
			}
			for (i=0; i<bs; i++) b2[i] = b1[i];
			GridOutlet_send(out,bs,b2);
		}
	}
	GridOutlet_end(out);
	return true;
err:
	return false;
}

GRID_BEGIN(FormatPPM,0) {
	fprintf($->bstream,
		"P6\n"
		"# generated using Video4jmax " VIDEO4JMAX_VERSION "\n"
		"%d %d\n"
		"255\n",
		Dim_get(in->dim,1),
		Dim_get(in->dim,0));

	fflush($->bstream);
	return true;
}

GRID_FLOW(FormatPPM,0) {
	uint8 data2[n];
	int i;
	for (i=0; i<n; i++) data2[i] = data[i];
	fwrite(data2,1,n,$->bstream);
}

GRID_END(FormatPPM,0) {
	fflush($->bstream);
	fseek($->bstream,0,SEEK_SET);
}

void FormatPPM_close (Format *$) {
	if ($->bstream) fclose($->bstream);
	FREE($);
}

Format *FormatPPM_open (FormatClass *qlass, const char *filename, int mode) {
	Format *$ = NEW(Format,1);
	$->cl     = &class_FormatPPM;

	$->stream = 0;
	$->bstream = 0;

	switch(mode) {
	case 4: case 2: break;
	default: whine("unsupported mode (#%d)", mode); goto err;
	}

	$->bstream = v4j_file_fopen(filename,mode);
	if (!$->bstream) {
		whine("can't open file `%s': %s", filename, strerror(errno));
		goto err;
	}
	return $;
err:
	$->cl->close($);
	return 0;
}

FormatClass class_FormatPPM = {
	symbol_name: "ppm",
	long_name: "Portable PixMap",
	flags: (FormatFlags)0,

	open: FormatPPM_open,
	connect: 0,
	chain_to: 0,

	frames: 0,
	frame:  FormatPPM_frame,

	begin:  GRID_BEGIN_PTR(FormatPPM,0),
	flow:    GRID_FLOW_PTR(FormatPPM,0),
	end:      GRID_END_PTR(FormatPPM,0),

	size:   0,
	color:  0,
	option: 0,
	close:  FormatPPM_close,
};