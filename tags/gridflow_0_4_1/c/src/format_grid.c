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
#include <fcntl.h>

static fts_alarm_t *grid_alarm;
static Dict *format_grid_object_set = 0;

extern FormatClass class_FormatGrid;

typedef struct FormatGrid FormatGrid;
typedef bool (*FGWaiter)(FormatGrid*,GridOutlet*,int n,char *buf);

struct FormatGrid {
	Format_FIELDS;

/* properties of currently recv'd image */
	bool is_le; /* little endian: smallest digit first, like i386 */
	int bpv;    /* bits per value: 32 only */
	bool is_socket;

/* socket stuff */
	int listener;

/* async stuff */
	char *buf;
	int buf_i;
	int buf_n;
	FGWaiter do_stuff;
	fts_alarm_t *alarm;
};

static void read_do(FormatGrid *$, int n, FGWaiter stuff) {
	FREE($->buf);
	$->buf = NEW(char,n);
	$->buf_i = 0;
	$->buf_n = n;
	$->do_stuff = stuff;
}

static bool try_read(FormatGrid *$) {
	if (!$->buf) {
//		whine("frame: try_read (nothing to do)");
		return true;
	}
	whine("frame: try read");
	while ($->buf) {
		int n = read($->stream,$->buf+$->buf_i,$->buf_n-$->buf_i);
		if (n<0) {
			whine("try_read: %s", strerror(errno));
			/* fix this */
		} else {
			$->buf_i += n;
		}
		if ($->buf_i == $->buf_n) {
			char *buf = $->buf;
			$->buf = 0;
			if (!$->do_stuff($,$->parent->out[0],$->buf_n,buf)) return false;
		} else {
			return true;
		}
	}
	return true;
}

static void swap32 (int n, uint32 *data) {
	while(n--) {
		uint32 x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	}
}

static int bufsize (FormatGrid *$, GridOutlet *out) {
	int n = Dim_prod_start($->dim,1);
	int k = 1 * gf_max_packet_length / n;
	if (k<1) k=1;
	return k*n*$->bpv/8;
}

/* for each slice of the body */
bool FormatGrid_frame3 (FormatGrid *$, GridOutlet *out, int n, char *buf) {
	int nn = n*8/$->bpv;
	Number *data = (Number *)buf;
//	whine("out->dex = %d",out->dex);
	if ($->is_le != is_le()) swap32(nn,data);
	GridOutlet_send(out,nn,data);
	FREE(buf);
	if (out->dex == Dim_prod(out->dim)) {
		GridOutlet_end(out);
	} else {
		read_do($,bufsize($,out),FormatGrid_frame3);
	}
	return true;
}

/* the dimension list */
bool FormatGrid_frame2 (FormatGrid *$, GridOutlet *out, int n, char *buf) {
	int prod;
	int n_dim = n/sizeof(int);
	int v[n_dim];
	int i;
	if ($->is_le != is_le()) swap32(n_dim,(uint32*)buf);
	whine("there are %d dimensions",n_dim);
	for (i=0; i<n_dim; i++) {
		v[i] = ((int*)buf)[i];
		whine("dimension %d is %d indices",i,v[i]);
		COERCE_INT_INTO_RANGE(v[i],0,MAX_INDICES);
	}
	$->dim = Dim_new(n_dim,v);
	prod = Dim_prod($->dim);
	if (prod <= 0 || prod > MAX_NUMBERS) {
		whine("dimension list: invalid prod (%d)",prod);
		goto err;
	}
	FREE(buf);
	GridOutlet_begin(out, $->dim);
	read_do($,bufsize($,out),FormatGrid_frame3);
	return true;
err: return false;
}

/* the header */
bool FormatGrid_frame1 (FormatGrid *$, GridOutlet *out, int n, char *buf) {
	int n_dim;
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
	whine("bpv=%d; res=%d; n_dim=%d", $->bpv, buf[6], n_dim);
	FREE(buf);
	read_do($,n_dim*sizeof(int),FormatGrid_frame2);
	return true;
err: return false;
}

/* rewinding and starting */
bool FormatGrid_frame (FormatGrid *$, GridOutlet *out, int frame) {
	if (frame!=-1) return 0;
	whine("frame: $->is_socket: %d",$->is_socket);
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

	read_do($,8,FormatGrid_frame1);
	if (!$->is_socket) {
		while ($->buf) try_read($);
		whine("frame: finished");
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

void FormatGrid_close (FormatGrid *$) {
	if ($->is_socket) Dict_del(gf_alarm_set,$);
/*	if ($->bstream) fclose($->bstream); */
	$->bstream = 0;
	if (0 <= $->stream) close($->stream);
	FREE($);
}

/* **************************************************************** */

bool FormatGrid_open_file (FormatGrid *$, int mode, ATOMLIST) {
	const char *filename;
	$->is_socket = false;

	if (ac<1) { whine("not enough arguments"); goto err; }

	if (!fts_is_symbol(at+0)) {
		whine("bad argument"); goto err;
	}

	filename = fts_symbol_name(fts_get_symbol(at+0));
	$->bstream = gf_file_fopen(filename,mode);
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

static void nonblock (int fd) {
	int flags = fcntl(fd,F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd,F_SETFL,flags);
}

bool FormatGrid_open_tcp (FormatGrid *$, int mode, ATOMLIST) {
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

	if (0>connect($->stream,(struct sockaddr *)&address,sizeof(address))) {
		whine("open_tcp(connect): %s",strerror(errno));
		goto err;
	}

	if ($->mode==4 && $->is_socket) nonblock($->stream);
	return true;
err:
	if (0<= $->stream) close($->stream);
	return false;
}

bool FormatGrid_open_tcpserver (FormatGrid *$, int mode, ATOMLIST) {
	struct sockaddr_in address;
	struct sockaddr address2;
	$->is_socket = true;

	if (ac<1) { whine("not enough arguments"); goto err; }

	if (!fts_is_int(at+0)) {
		whine("bad arguments"); goto err;
	}

	$->listener = socket(AF_INET,SOCK_STREAM,0);

	address.sin_family = AF_INET;
	address.sin_port = htons(fts_get_int(at+0));
	address.sin_addr.s_addr = INADDR_ANY;  /* whatever */

	if (0> bind($->listener,(struct sockaddr *)&address,sizeof(address))) {
		printf("open_tcpserver(bind): %s\n", strerror(errno));
		goto err;
	}

	if (0> listen($->listener,0)) {
		printf("open_tcpserver(listen): %s\n", strerror(errno));
		goto err;
	}

	$->stream = accept($->listener,0,0);
	if (0> $->stream) {
		$->stream = -1;
		printf("open_tcpserver(accept): %s\n", strerror(errno));
		goto err;
	}

	close($->listener);
	$->listener = -1;

	if ($->mode==4) nonblock($->stream);
	return true;
err:
	if ($->listener>0) {
		close($->listener);
		$->listener = -1;
	}
	return false;
}

/* **************************************************************** */

Format *FormatGrid_open (FormatClass *qlass, GridObject *parent, int mode, ATOMLIST) {
	FormatGrid *$ = (FormatGrid *)Format_open(&class_FormatGrid,parent,mode);
	const char *filename;

	if (!$) return 0;
	$->buf = 0;

	if (ac<1) { whine("not enough arguments"); goto err; }

	{
		int result;
		fts_symbol_t sym = fts_get_symbol(at+0);
		if (sym == SYM(file)) {
			result = FormatGrid_open_file($,mode,ac-1,at+1);
		} else if (sym == SYM(tcp)) {
			result = FormatGrid_open_tcp($,mode,ac-1,at+1);
		} else if (sym == SYM(tcpserver)) {
			result = FormatGrid_open_tcpserver($,mode,ac-1,at+1);
		} else {
			whine("unknown access method '%s'",fts_symbol_name(sym));
			goto err;
		}
		if (!result) goto err;
		whine("open: $->is_socket = %d", $->is_socket);
	}

	if ($->is_socket) Dict_put(gf_alarm_set,$,try_read);

	return (Format *)$;
err:
	$->cl->close((Format *)$);
	return 0;
}

/* **************************************************************** */

FormatClass class_FormatGrid = {
	object_size: sizeof(FormatGrid),
	symbol_name: "grid",
	long_name: "Grid",
	flags: FF_R|FF_W,

	open: FormatGrid_open,
	frames: 0,
	frame:  (Format_frame_m)FormatGrid_frame,
	begin:  (GRID_BEGIN_(Format,(*)))GRID_BEGIN_PTR(FormatGrid,0),
	flow:    (GRID_FLOW_(Format,(*))) GRID_FLOW_PTR(FormatGrid,0),
	end:      (GRID_END_(Format,(*)))  GRID_END_PTR(FormatGrid,0),
	option: 0,
	close:  (Format_close_m)FormatGrid_close,
};