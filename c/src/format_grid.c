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
	1 uint8: bits per value (supported: 32)
	1 uint8: reserved (supported: 0)
	1 uint8: number of dimensions N (supported: at least 0..4)
	N uint32: number of elements per dimension D[0]..D[N-1]
	raw data goes there.

	future plans:
	additional bpv values 1,2,4,8,16
	reserved field will become number type or something
*/

#include "grid.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

extern FormatClass class_FormatGrid;

typedef struct FormatGrid {
	Format_FIELDS;
	/* properties of currently recv'd image */
	bool is_le; /* little endian: smallest digit first, like i386 */
	int bpv;    /* bits per value: 32 only */
	bool is_socket; /* future use */
} FormatGrid;

static void swap32 (int n, uint32 *data) {
	while(n--) {
		uint32 x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	}
}

bool FormatGrid_frame (FormatGrid *$, GridOutlet *out, int frame) {
	int n_dim, prod;
	if (frame!=-1) return 0;

	whine("$->is_socket: %d",$->is_socket);

	/* rewind when at end of file. */
	if (!$->is_socket) {
		off_t thispos = lseek($->stream,0,SEEK_CUR);
		off_t lastpos = lseek($->stream,0,SEEK_END);
		whine("thispos = %d", thispos);
		whine("lastpos = %d", lastpos);
		if (thispos == lastpos) thispos = 0;
		{
			off_t nextpos = lseek($->stream,thispos,SEEK_SET);
			whine("nextpos = %d", nextpos);
		}
	}

	/* header */
	{
		char buf[8];
		if (8>read($->stream,buf,sizeof(buf))) {
			whine("grid header: read error: %s",strerror(errno));
			goto err;
		}
		if (is_le()) {
			whine("we are smallest digit first");
		} else {
			whine("we are biggest digit first");
		}
		if (strncmp("\x7fGRID",buf,5)==0) {
			$->is_le = false; whine("this file is biggest digit first");
		} else if (strncmp("\x7fgrid",buf,5)==0) {
			$->is_le = true; whine("this file is smallest digit first");
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
		int v[n_dim],i;
		if (sizeof(v)>read($->stream,v,sizeof(v))) {
			whine("dimension list: read error: %s",strerror(errno));
			goto err;
		}
		if ($->is_le != is_le()) swap32(n_dim,(uint32 *)v);
		whine("there are %d dimensions",n_dim);
		for (i=0; i<n_dim; i++) {
			whine("dimension %d is %d indices",i,v[i]);
			COERCE_INT_INTO_RANGE(v[i],0,MAX_INDICES);
		}
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
		char *data2 = (char *)data;
		int n=prod*sizeof(Number);
		int i=0;
		while (i<n) {
			int r = read($->stream,data2+i,n-i);
			if (r<0) {
				whine("body: read error: %s",strerror(errno));
				goto err;
			}
			i+=r;
			whine("got %d bytes; %d bytes left", r, n-i);
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
	$->dim = in->dim;

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
		if (8>write($->stream,buf,8)) {
			whine("grid header: write error: %s",strerror(errno));
		}

	}

	/* dimension list */
	{
		int n = sizeof(int)*Dim_count($->dim);
		if (n>write($->stream,$->dim->v,n)) {
			whine("dimension list: write error: %s",strerror(errno));
		}
	}	
	return true;
}

GRID_FLOW(FormatGrid,0) {
	write($->stream,data,n*sizeof(Number));
}

GRID_END(FormatGrid,0) {
	lseek($->stream,0,SEEK_SET);
}

void FormatGrid_close (Format *$) {
/*	if ($->bstream) fclose($->bstream); */
	$->bstream = 0;
	if ($->stream) close($->stream);
	FREE($);
}

/* **************************************************************** */

bool FormatGrid_open_file (FormatGrid *$, int ac, const fts_atom_t *at, int mode) {
	const char *filename;
	whine("open_file: $->is_socket = %d", $->is_socket);
	$->is_socket = false;
	whine("open_file: $->is_socket = %d", $->is_socket);

	if (ac<1) { whine("not enough arguments"); goto err; }

	if (!fts_is_symbol(at+0)) {
		whine("bad argument"); goto err;
	}

	filename = fts_symbol_name(fts_get_symbol(at+0));
	$->bstream = v4j_file_fopen(filename,mode);
	if (!$->bstream) {
		whine("can't open file `%s': %s", filename, strerror(errno));
		goto err;
	}
	$->stream = fileno($->bstream);
	return true;
err:
	return false;
}

/* **************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

bool FormatGrid_open_tcp (FormatGrid *$, int ac, const fts_atom_t *at, int mode) {
	struct sockaddr_in address;
	$->is_socket = true;

	if (ac<2) { whine("not enough arguments"); goto err; }

	if (!fts_is_symbol(at+0) || !fts_is_int(at+1)) {
		whine("bad arguments"); goto err;
	}

	$->stream = socket(AF_INET,SOCK_STREAM,0);

	address.sin_family = AF_INET;
	address.sin_port = htons(fts_get_int(at+1));

	{
		const char *hostname;
		int port;
		struct hostent *h = gethostbyname(fts_symbol_name(fts_get_symbol(at+0)));
		if (!h) {
			whine("open_tcp(gethostbyname): %s",strerror(errno));
			goto err;
		}
		memcpy(&address.sin_addr.s_addr,h->h_addr_list[0],h->h_length);
	}

	if (0>connect($->stream,&address,sizeof(address))) {
		whine("open_tcp(connect): %s",strerror(errno));
		goto err;
	}
	return true;
err:
	return false;
}

bool FormatGrid_open_tcpserver (FormatGrid *$, int ac, const fts_atom_t *at, int mode) {
	struct sockaddr_in address;
	$->is_socket = true;

	if (ac<1) { whine("not enough arguments"); goto err; }

	if (!fts_is_int(at+1)) {
		whine("bad arguments"); goto err;
	}

	$->stream = socket(AF_INET,SOCK_STREAM,0);

	/* add server side code here */
	return 42;
err:
	return false;
}

/* **************************************************************** */

Format *FormatGrid_open (FormatClass *qlass, int ac, const fts_atom_t *at, int mode) {
	FormatGrid *$ = NEW(FormatGrid,1);
	const char *filename;
	$->cl     = &class_FormatGrid;

	$->stream = 0;
	$->bstream = 0;

	if (ac<1) { whine("not enough arguments"); goto err; }

	switch(mode) {
	case 4: case 2: break;
	default: whine("unsupported mode (#%d)", mode); goto err;
	}

	{
		int result;
		if (fts_get_symbol(at+0) == SYM(file)) {
			result = FormatGrid_open_file($,ac-1,at+1,mode);
		} else if (fts_get_symbol(at+0) == SYM(tcp)) {
			result = FormatGrid_open_tcp($,ac-1,at+1,mode);
		} else {
			whine("unknown access method");
			goto err;
		}
		if (!result) goto err;
		whine("open: $->is_socket = %d", $->is_socket);
	}

	return $;
err:
	$->cl->close($);
	return 0;
}

/* **************************************************************** */

FormatClass class_FormatGrid = {
	symbol_name: "grid",
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
