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

extern FileFormatClass FormatPPM;

Dim *FormatPPM_frame (FileFormat *$, int frame) {
	char buf[256];
	int metrics[6],n=0;

	if (frame != -1) return 0;
	fgets(buf,256,$->stream);
	if (feof($->stream)) {
		fseek($->stream,0,SEEK_SET);
		fgets(buf,256,$->stream);
	}

	if(strcmp("P6\n",buf)!=0) {
		whine("Wrong format (needing PPM P6)");
		goto err;
	}
	while (n<3) {
		fgets(buf,256,$->stream);
		if (*buf=='#') continue; /* skipping a comment line */
		n += sscanf(buf,"%d%d%d",metrics+n,metrics+n+1,metrics+n+2);
		if (feof($->stream)) {
			whine("unexpected end of file in header");
			goto err;
		}
	}
	whine("File metrics: %d %d %d",metrics[0],metrics[1],metrics[2]);
	if(metrics[2] != 255) {
		whine("Wrong color depth (max_value=%d instead of 255)",metrics[2]);
		goto err;
	}
	{
		int v[] = { metrics[1], metrics[0], 3 };
		$->dim = Dim_new(3,v);
		$->left = Dim_prod($->dim);
		return $->dim;
	}
err:
	return 0;
}

Number *FormatPPM_read (FileFormat *$, int n) {
	int i;
	int bs = $->left;
	int bs2;
	uint8 b1[n];
	Number *b2 = NEW2(Number,n);

	if (!$->dim) return 0;
	if (bs > n) bs = n;
	bs2 = (int) fread(b1,1,bs,$->stream);
	if (bs2 < bs) {
		whine("unexpected end of file: bs=%d; bs2=%d",bs,bs2);
	}
	for (i=0; i<bs; i++) b2[i] = b1[i];
	n -= bs;
	$->left -= bs;
	return b2;
}

void FormatPPM_begin (FileFormat *$, Dim *dim) {
	$->dim = dim;
	fprintf($->stream,
		"P6\n"
		"# generated using Video4jmax " VIDEO4JMAX_VERSION "\n"
		"%d %d\n"
		"255\n",
		Dim_get($->dim,1),
		Dim_get($->dim,0));

	fflush($->stream);
}

void FormatPPM_flow (FileFormat *$, int n, const Number *data) {
	uint8 data2[n];
	int i;
	for (i=0; i<n; i++) data2[i] = data[i];
	fwrite(data2,1,n,$->stream);
}

void FormatPPM_end (FileFormat *$) {
	fflush($->stream);
	fseek($->stream,0,SEEK_SET);
}

void FormatPPM_close (FileFormat *$) {
	if ($->stream) fclose($->stream);
	FREE($);
}

FileFormat *FormatPPM_open (const char *filename, int mode) {
	const char *modestr;
	FileFormat *$ = NEW(FileFormat,1);
	$->qlass  = &FormatPPM;
	$->frames = 0;
	$->frame  = FormatPPM_frame;
	$->size   = 0;
	$->read   = FormatPPM_read;
	$->begin  = FormatPPM_begin;
	$->flow   = FormatPPM_flow;
	$->end    = FormatPPM_end;
	$->color  = 0;
	$->option = 0;
	$->close  = FormatPPM_close;

	$->stuff = NEW(int,3); /* huh? */
	$->stream = 0;

	switch(mode) {
	case 4: case 2: break;
	default: whine("unsupported mode (#%d)", mode); goto err;
	}

	$->stream = v4j_file_fopen(filename,mode);
	if (!$->stream) {
		whine("can't open file `%s': %s", filename, strerror(errno));
		goto err;
	}
	return $;
err:
	$->close($);
	return 0;
}

FileFormatClass FormatPPM = {
	"ppm", "Portable PixMap", (FileFormatFlags)0,
	FormatPPM_open, 0, 0,
};
