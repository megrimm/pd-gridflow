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
//#include <fcntl.h>

extern FormatClass class_FormatGrid;

typedef struct FormatGrid FormatGrid;

struct FormatGrid {
	Format_FIELDS;

/* properties of currently recv'd image */
	bool is_le; /* little endian: smallest digit first, like i386 */
	int bpv;    /* bits per value: 32 only */
	bool is_socket;

/* socket stuff */
	int listener;
};

static int bufsize (FormatGrid *$, GridOutlet *out) {
	int n = Dim_prod_start($->dim,1);
	int k = 1 * gf_max_packet_length / n;
	if (k<1) k=1;
	return k*n*$->bpv/8;
}

/* for each slice of the body */
static bool FormatGrid_frame3 (FormatGrid *$, int n, char *buf) {
	GridOutlet *out = $->parent->out[0];
	int nn = n*8/$->bpv;
	Number *data = (Number *)buf;
//	whine("out->dex = %d",out->dex);
	if ($->is_le != is_le()) swap32(nn,data);
	GridOutlet_send(out,nn,data);
	FREE(buf);
	if (out->dex == Dim_prod(out->dim)) {
		GridOutlet_end(out);
	} else {
		Stream_on_read_do($->st,bufsize($,out),(OnRead)FormatGrid_frame3,$);
	}
	return true;
}

/* the dimension list */
static bool FormatGrid_frame2 (FormatGrid *$, int n, char *buf) {
	GridOutlet *out = $->parent->out[0];
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
	Stream_on_read_do($->st,bufsize($,out),(OnRead)FormatGrid_frame3,$);
	return true;
err: return false;
}

/* the header */
static bool FormatGrid_frame1 (FormatGrid *$, int n, char *buf) {
	GridOutlet *out = $->parent->out[0];
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
	Stream_on_read_do($->st,n_dim*sizeof(int),(OnRead)FormatGrid_frame2,$);
	return true;
err: return false;
}

/* rewinding and starting */
static bool FormatGrid_frame (FormatGrid *$, GridOutlet *out, int frame) {
	int fd = Stream_get_fd($->st);
	if (frame!=-1) return 0;
	whine("frame: $->is_socket: %d",$->is_socket);
	/* rewind when at end of file. */
	if (!$->is_socket) {
		off_t thispos = lseek(fd,0,SEEK_CUR);
		off_t lastpos = lseek(fd,0,SEEK_END);
		whine("thispos = %d", thispos);
		whine("lastpos = %d", lastpos);
		if (thispos == lastpos) thispos = 0;
		{
			off_t nextpos = lseek(fd,thispos,SEEK_SET);
			whine("nextpos = %d", nextpos);
		}
	}

	Stream_on_read_do($->st,8,(OnRead)FormatGrid_frame1,$);
	if (!$->is_socket) {
		while (Stream_is_waiting($->st)) Stream_try_read($->st);
		whine("frame: finished");
	}

	return true;
err: return false;
}

GRID_BEGIN(FormatGrid,0) {
	int fd = Stream_get_fd($->st);
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
		if (8>write(fd,buf,8)) {
			whine("grid header: write error: %s",strerror(errno));
		}

	}

	/* dimension list */
	{
		int n = sizeof(int)*Dim_count($->dim);
		if (n>write(fd,$->dim->v,n)) {
			whine("dimension list: write error: %s",strerror(errno));
		}
	}	
	return true;
}

GRID_FLOW(FormatGrid,0) {
	int fd = Stream_get_fd($->st);
	write(fd,data,n*sizeof(Number));
}

GRID_END(FormatGrid,0) {
	int fd = Stream_get_fd($->st);
	lseek(fd,0,SEEK_SET);
}

void FormatGrid_close (FormatGrid *$) {
	int fd = Stream_get_fd($->st);
	close(fd);
	if ($->is_socket) Dict_del(gf_timer_set,$);
	$->st = 0;
	FREE($);
}

/* **************************************************************** */

static bool FormatGrid_open_file (FormatGrid *$, int mode, ATOMLIST) {
	const char *filename;
	$->is_socket = false;

	if (ac<1) { whine("not enough arguments"); goto err; }

	if (!Var_has_symbol(at+0)) {
		whine("bad argument"); goto err;
	}

	filename = Symbol_name(Var_get_symbol(at+0));
	$->st = Stream_open_file(filename,mode);
	if (!$->st) {
		whine("can't open file `%s': %s", filename, strerror(errno));
		goto err;
	}
	return true;
err: return false;
}

/* **************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

static bool FormatGrid_open_tcp (FormatGrid *$, int mode, ATOMLIST) {
	int stream = -1;
	struct sockaddr_in address;
	$->is_socket = true;

	if (ac<2) { whine("not enough arguments"); goto err; }

	if (!Var_has_symbol(at+0) || !Var_has_int(at+1)) {
		whine("bad arguments"); goto err;
	}

	stream = socket(AF_INET,SOCK_STREAM,0);

	address.sin_family = AF_INET;
	address.sin_port = htons(Var_get_int(at+1));

	{
		struct hostent *h = gethostbyname(Symbol_name(Var_get_symbol(at+0)));
		if (!h) {
			whine("open_tcp(gethostbyname): %s",strerror(errno));
			goto err;
		}
		memcpy(&address.sin_addr.s_addr,h->h_addr_list[0],h->h_length);
	}

	if (0>connect(stream,(struct sockaddr *)&address,sizeof(address))) {
		whine("open_tcp(connect): %s",strerror(errno));
		goto err;
	}

	$->st = Stream_open_fd(stream,$->mode);
	if ($->mode==4 && $->is_socket) Stream_nonblock($->st);
	return true;
err:
	if (0<= stream) close(stream);
	return false;
}

static bool FormatGrid_open_tcpserver (FormatGrid *$, int mode, ATOMLIST) {
	int stream = -1;
	struct sockaddr_in address;
	$->is_socket = true;

	if (ac<1) { whine("not enough arguments"); goto err; }

	if (!Var_has_int(at+0)) {
		whine("bad arguments"); goto err;
	}

	$->listener = socket(AF_INET,SOCK_STREAM,0);

	address.sin_family = AF_INET;
	address.sin_port = htons(Var_get_int(at+0));
	address.sin_addr.s_addr = INADDR_ANY;  /* whatever */

	if (0> bind($->listener,(struct sockaddr *)&address,sizeof(address))) {
		printf("open_tcpserver(bind): %s\n", strerror(errno));
		goto err;
	}

	if (0> listen($->listener,0)) {
		printf("open_tcpserver(listen): %s\n", strerror(errno));
		goto err;
	}

	stream = accept($->listener,0,0);
	if (0> stream) {
		printf("open_tcpserver(accept): %s\n", strerror(errno));
		goto err;
	}

	close($->listener);
	$->listener = -1;

	$->st = Stream_open_fd(stream,mode);

	if ($->mode==4) Stream_nonblock($->st);
	return true;
err:
	if ($->listener>0) {
		close($->listener);
		$->listener = -1;
	}
	return false;
}

/* **************************************************************** */

static Format *FormatGrid_open (FormatClass *qlass, GridObject *parent, int mode, ATOMLIST) {
	FormatGrid *$ = (FormatGrid *)Format_open(&class_FormatGrid,parent,mode);

	if (!$) return 0;

	if (ac<1) { whine("not enough arguments"); goto err; }

	{
		int result;
		Symbol sym = Var_get_symbol(at+0);
		if (sym == SYM(file)) {
			result = FormatGrid_open_file($,mode,ac-1,at+1);
		} else if (sym == SYM(tcp)) {
			result = FormatGrid_open_tcp($,mode,ac-1,at+1);
		} else if (sym == SYM(tcpserver)) {
			result = FormatGrid_open_tcpserver($,mode,ac-1,at+1);
		} else {
			whine("unknown access method '%s'",Symbol_name(sym));
			goto err;
		}
		if (!result) goto err;
		whine("open: $->is_socket = %d", $->is_socket);
	}

	if ($->is_socket) Dict_put(gf_timer_set,$,Stream_try_read);

	return (Format *)$;
err:
	$->cl->close((Format *)$);
	return 0;
}

/* **************************************************************** */

static GridHandler FormatGrid_handler = GRINLET(FormatGrid,0);
FormatClass class_FormatGrid = {
	object_size: sizeof(FormatGrid),
	symbol_name: "grid",
	long_name: "GridFlow file format",
	flags: FF_R|FF_W,

	open: FormatGrid_open,
	frames: 0,
	frame:  (Format_frame_m)FormatGrid_frame,
	handler: &FormatGrid_handler,
	option: 0,
	close:  (Format_close_m)FormatGrid_close,
};
