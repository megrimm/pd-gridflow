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
*/

#include "../base/grid.h"
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
/* hope for a multiple of 4 */
static bool FormatGrid_frame3 (FormatGrid *$, int n, char *buf) {
	GridOutlet *out = $->parent->out[0];
	int nn = n*8/$->bpv;
//	whine("out->dex = %d",out->dex);
	if ($->bpv==8) {
		int i;
		Number buf2[n];
		for (i=0; i<n; i++) buf2[i] = ((uint8 *)buf)[i];
		GridOutlet_send(out,n,buf2);
	} else if ($->bpv==32) {
		Number *data = (Number *)buf;
		if ($->is_le != is_le()) swap32(nn,data);
		GridOutlet_send(out,nn,data);
	}
	FREE(buf);
	if (out->dex == Dim_prod(out->dim)) {
		GridOutlet_end(out);
		/* how do i fix this? */
		//Dict_del(gf_timer_set,$->st);
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
//	whine("there are %d dimensions",n_dim);
	for (i=0; i<n_dim; i++) {
		v[i] = ((int*)buf)[i];
//		whine("dimension %d is %d indices",i,v[i]);
		COERCE_INT_INTO_RANGE(v[i],0,MAX_INDICES);
	}
	$->dim = Dim_new(n_dim,v);
	prod = Dim_prod($->dim);
	if (prod <= 0 || prod > MAX_NUMBERS)
		RAISE("dimension list: invalid prod (%d)",prod);
	FREE(buf);
	GridOutlet_begin(out, $->dim);
	Stream_on_read_do($->st,bufsize($,out),(OnRead)FormatGrid_frame3,$);
	return true;
err: return false;
}

/* the header */
static bool FormatGrid_frame1 (FormatGrid *$, int n, char *buf) {
	GridOutlet *out = $->out[0];
	int n_dim;
/*
	if (is_le())
		whine("we are smallest digit first");
	else
		whine("we are biggest digit first");
*/

	if (strncmp("\x7fGRID",buf,5)==0) {
		$->is_le = false; //whine("this file is biggest digit first");
	} else if (strncmp("\x7fgrid",buf,5)==0) {
		$->is_le = true; //whine("this file is smallest digit first");
	} else {
		RAISE("grid header: invalid");
	}

	$->bpv = buf[5];
	n_dim  = buf[7];
	switch ($->bpv) {
		case 8: case 32: break;
		default:
		RAISE("unsupported bpv (%d)", $->bpv);
	}
	if (buf[6] != 0) RAISE("reserved field is not zero");
	if (n_dim > MAX_DIMENSIONS) RAISE("too many dimensions (%d)",n_dim);

	//whine("bpv=%d; res=%d; n_dim=%d", $->bpv, buf[6], n_dim);
	FREE(buf);
	Stream_on_read_do($->st,n_dim*sizeof(int),(OnRead)FormatGrid_frame2,$);
	return true;
}

/* rewinding and starting */
METHOD(FormatGrid,frame) {
	int fd = Stream_get_fd($->st);
	if (Stream_is_waiting($->st))
		RAISE("frame: Stream %p in ff %p is already waiting.",$->st,$);
	//whine("frame: $->is_socket: %d",$->is_socket);

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
	if ($->is_socket) {
		/* non-blocking */
		MainLoop_add($->st,(void(*)())Stream_try_read);
	} else {
		/* blocking */
		while (Stream_is_waiting($->st)) Stream_try_read($->st);
		whine("frame: finished");
	}
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
		buf[5]=$->bpv;
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
	int i;
	if ($->bpv==8) {
		uint8 buf[n];
		for (i=0; i<n; i++) { buf[i] = data[i]; }
		write(fd,(char *)buf,n);
	} else if ($->bpv==32) {
		write(fd,data,n*sizeof(Number));
	}
}

GRID_END(FormatGrid,0) {
	int fd = Stream_get_fd($->st);
	lseek(fd,0,SEEK_SET); /* it's bad to rewind here */
}

METHOD(FormatGrid,close) {
	if ($->st) Stream_close($->st);
	if ($->is_socket) MainLoop_remove($);
	rb_call_super(argc,argv);
}

METHOD(FormatGrid,option) {
	int fd = Stream_get_fd($->st);
	VALUE sym = argv[0];
	if (sym == SYM(type)) {
		/* bug: should not be able to modify this _during_ a transfer */
		VALUE value = argv[1];
		if (value==SYM(uint8)) { $->bpv=8; }
		else if (value==SYM(int32)) { $->bpv=32; }
		whine("$->bpv = %d",$->bpv);
	} else {
		rb_call_super(argc,argv);
	}
}

/* **************************************************************** */

static bool FormatGrid_open_file (FormatGrid *$, int argc, VALUE *argv) {
	const char *filename;
	$->is_socket = false;

	if (argc<1) RAISE("not enough arguments");

	if (TYPE(argv[0])!=T_SYMBOL) RAISE("bad argument");
	filename = rb_sym_name(argv[0]);
	$->st = Stream_open_file(filename,$->mode);
	if (!$->st) RAISE("can't open file `%s': %s", filename, strerror(errno));
	return true;
}

/* **************************************************************** */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

static bool FormatGrid_open_tcp (FormatGrid *$, int argc, VALUE *argv) {
	int stream = -1;
	struct sockaddr_in address;
	$->is_socket = true;

	if (argc<2) RAISE("not enough arguments");

	if (TYPE(argv[0])!=T_SYMBOL || TYPE(argv[1])!=T_FIXNUM) RAISE("bad arguments");

	stream = socket(AF_INET,SOCK_STREAM,0);

	address.sin_family = AF_INET;
	address.sin_port = htons(NUM2INT(argv[1]));

	{
		struct hostent *h = gethostbyname(rb_id2name(SYM2ID(argv[0])));
		if (!h) RAISE("open_tcp(gethostbyname): %s",strerror(errno));
		memcpy(&address.sin_addr.s_addr,h->h_addr_list[0],h->h_length);
	}

	if (0>connect(stream,(struct sockaddr *)&address,sizeof(address)))
		RAISE("open_tcp(connect): %s",strerror(errno));

	$->st = Stream_open_fd(stream,$->mode);
	if ($->mode==4 && $->is_socket) Stream_nonblock($->st);
	return true;
}

static bool FormatGrid_open_tcpserver (FormatGrid *$, int argc, VALUE
*argv) {
	int stream = -1;
	struct sockaddr_in address;
	$->is_socket = true;

	if (argc<1) RAISE("not enough arguments");

	if (TYPE(argv[0])!=T_FIXNUM) RAISE("bad arguments");

	$->listener = socket(AF_INET,SOCK_STREAM,0);

	address.sin_family = AF_INET;
	address.sin_port = htons(NUM2INT(argv[0]));
	address.sin_addr.s_addr = INADDR_ANY;  /* whatever */

	if (0> bind($->listener,(struct sockaddr *)&address,sizeof(address)))
		RAISE("open_tcpserver(bind): %s\n", strerror(errno));

	if (0> listen($->listener,0)) RAISE("open_tcpserver(listen): %s\n", strerror(errno));

	stream = accept($->listener,0,0);
	if (0> stream) RAISE("open_tcpserver(accept): %s\n", strerror(errno));

	close($->listener);
	$->listener = -1;

	$->st = Stream_open_fd(stream,$->mode);

	if ($->mode==4) Stream_nonblock($->st);
	return true;
}

/* **************************************************************** */

METHOD(FormatGrid,init) {
	rb_call_super(argc,argv);
	argv++, argc--;
	$->bpv = 32;

	if (argc<1) RAISE("not enough arguments");

	{
		int result;
		VALUE sym = argv[0];
		argv++, argc--;
		if (sym == SYM(file)) {
			result = FormatGrid_open_file($,argc,argv);
		} else if (sym == SYM(tcp)) {
			result = FormatGrid_open_tcp($,argc,argv);
		} else if (sym == SYM(tcpserver)) {
			result = FormatGrid_open_tcpserver($,argc,argv);
		} else {
			RAISE("unknown access method '%s'",sym);
		}
	}
}

/* **************************************************************** */

FMTCLASS(FormatGrid,"grid","GridFlow file format",FF_R|FF_W,
inlets:1,outlets:1,LIST(GRINLET(FormatGrid,0)),
DECL(FormatGrid,init),
DECL(FormatGrid,frame),
DECL(FormatGrid,option),
DECL(FormatGrid,close))
