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

/*
	This is the Grid format I defined:
	1 uint8: 0x7f
	4 uint8: "GRID" big endian | "grid" little endian
	1 uint8: bits per value (supported: 8, 32)
	1 uint8: reserved (supported: 0)
	1 uint8: number of dimensions N (supported: at least 0..4)
	N uint32: number of elements per dimension D[0]..D[N-1]
	raw data goes there.
*/

#include "grid.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

extern FormatClass class_FormatGrid;

typedef struct FormatGrid {
	Format_FIELDS;
	bool is_le; /* little endian: smallest digit first, like i386 */
	int bpv;    /* bits per value: 32 only */
} FormatGrid;

static void swap32 (int n, uint32 *data) {
	while(n--) {
		int x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	}
}

bool FormatGrid_frame (FormatGrid *$, GridOutlet *out, int frame) {
	int n_dim, prod;
	if (frame!=-1) return 0;

	/* header */
	{
		char buf[8];
		if (0>read($->stream,buf,sizeof(buf))) {
			whine("grid header: read error: %s",strerror(errno));
			goto err;
		}
		if (strncmp("\x7fGRID",buf,5)==0) {
			$->is_le = false; whine("biggest digit first");
		} else if (strncmp("\x7fgrid",buf,5)==0) {
			$->is_le = true; whine("smallest digit first");
		} else {
			whine("grid header: invalid");
			goto err;
		}
		$->bpv = buf[5];
		n_dim  = buf[7];
		if ($->bpv != 32) {
			whine("unsupported bpv (%d)", $->bpv);
			goto err;
		}
		if (buf[6] != 0) {
			whine("reserved field is not zero");
			goto err;
		}
		if (n_dim > 4) {
			whine("too many dimensions (%d)",n_dim);
			goto err;
		}
	}

	/* dimension list */
	{
		int v[n_dim];
		if (0>read($->stream,v,sizeof(v))) {
			whine("dimension list: read error: %s",strerror(errno));
			goto err;
		}
		if ($->is_le != is_le()) swap32(n_dim,(uint32 *)v);
		$->dim = Dim_new(n_dim,v);
		prod = Dim_prod($->dim);
		if (prod <= 0 || prod > MAX_NUMBERS) {
			whine("dimension list: invalid prod (%d)",n_dim,prod);
			goto err;
		}
	}

	/* body */
	{
		Number *data = NEW2(Number,prod);
		int i;
		if (0>read($->stream,data,prod)) {
			whine("body: read error: %s",strerror(errno));
			goto err;
		}
		if ($->is_le != is_le()) swap32(prod,data);

		GridOutlet_begin(out, $->dim);
		GridOutlet_send(out,prod,data);
		GridOutlet_end(out);
		FREE(data);
	}
	return true;
err:
	return false;
}

GRID_BEGIN(FormatGrid,0) {
	/* header */
	{
		char buf[8];
		if (is_le()) {
			strcpy(buf,"\x7fgrid");
		} else {
			strcpy(buf,"\x7fGRID");
		}
		buf[5]=32;
		buf[6]=0;
		buf[7]=Dim_count($->dim);
	}

	/* dimension list */
	{
		if (0>write($->stream,$->dim->v,Dim_count($->dim))) {
			whine("dimension list: write error: %s",strerror(errno));
		}
	}	
	return true;
}

GRID_FLOW(FormatGrid,0) {
	Number data2[n];
	memcpy(data2,data,n);
	if ($->is_le != is_le()) swap32(n,data2);
	write($->stream,data2,n);
}

GRID_END(FormatGrid,0) {
	/* nothing to do */
}

void FormatGrid_close (Format *$) {
	if ($->bstream) fclose($->bstream);
	FREE($);
}

Format *FormatGrid_open (FormatClass *qlass, int ac, const fts_atom_t *at, int mode) {
	Format *$ = NEW(Format,1);
	const char *filename;
	$->cl     = &class_FormatGrid;

	$->stream = 0;
	$->bstream = 0;

	if (ac!=2 || fts_get_symbol(at+0) != SYM(file)) {
		whine("usage: grid file <filename>"); goto err;
	}
	filename = fts_symbol_name(fts_get_symbol(at+1));

	switch(mode) {
	case 4: case 2: break;
	default: whine("unsupported mode (#%d)", mode); goto err;
	}

	$->bstream = v4j_file_fopen(filename,mode);
	$->stream = fileno($->bstream);

	if (!$->bstream) {
		whine("can't open file `%s': %s", filename, strerror(errno));
		goto err;
	}
	return $;
err:
	$->cl->close($);
	return 0;
}

/* **************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>

/*
Format *FormatGrid_connect (FormatClass *qlass, const char *dest, int mode) {
	// TCP Socket
	int sock = socket(AF_INET,SOCK_STREAM,0);
//	struct sockaddr_t address = { AF_INET, };
	

}
*/

/* **************************************************************** */

FormatClass class_FormatGrid = {
	symbol_name: "Grid",
	long_name: "Grid",
	flags: (FormatFlags)0,

	open: FormatGrid_open,

	frames: 0,
	frame:  FormatGrid_frame,

	begin:  GRID_BEGIN_PTR(FormatGrid,0),
	flow:    GRID_FLOW_PTR(FormatGrid,0),
	end:      GRID_END_PTR(FormatGrid,0),

	option: 0,
	close:  FormatGrid_close,
};
