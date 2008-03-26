/*
	$Id$

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "../base/grid.h.fcs"

/* This is the Grid format I defined: */
struct GridHeader {
	char magic[5]; // = "\7fgrid" on little endian, "\x7fGRID" on big endian
	uint8 type; // number of bits.
		   // the original doc said: "plus one of: 1:unsigned 2:float" but i don't recall what this means.
	uint8 reserved; // set this to 0 all of the time.
	uint8 dimn; // number of dimensions supported: at least 0..4)
	// int32 dimv[dimn]; // number of elements in each dimension. (in the file's endianness!)
	// raw data goes after that
};

\class FormatGrid < Format {
	GridHeader head;
	int fd;
	FILE *f;
	int endian;
	NumberTypeE nt;
	P<Dim> headerless; // if null: headerful; if Dim: it is the assumed dimensions of received grids
	\grin 0
	\decl void initialize(Symbol mode, String filename);
	\decl 0 bang ();
	\decl 0 headerless_m (...);
	\decl 0 headerful ();
	\decl 0 type (NumberTypeE nt);
	\decl 0 close();
	\decl void raw_open_gzip_in(String filename);
	\decl void raw_open_gzip_out(String filename);
	\decl void raw_open(Symbol mode, String filename);
};

\def void initialize(Symbol mode, String filename) {
	rb_call_super(argc,argv);
	strncpy(head.magic,is_le()?"\7fgrid":"\7fGRID",5);
	head.type = 32;
	rb_funcall(rself,SI(raw_open),3,mode,filename);
	Ruby stream = rb_ivar_get(rself,SI(@stream));
	fd = NUM2INT(rb_funcall(stream,SI(fileno),0));
	f = fdopen(fd,mode==SYM(in)?"r":"w");
}
\def 0 bang () {
	P<Dim> dim;
	if (feof(f)) {outlet_bang(bself->out[1]); return;}
	if (headerless) {
		dim = headerless;
	} else {
		fread(&head,1,8,f);
		uint8 *m = (uint8 *)head.magic;
		if (strncmp((char *)m,"\7fgrid",5)==0) endian=1; else
		if (strncmp((char *)m,"\7fGRID",5)==0) endian=1; else
		RAISE("unknown header, can't read grid from file: "
			"%02x %02x %02x %02x %02x %02x %02x %02x",
			m[0],m[1],m[2],m[3],m[4],m[5],m[6],m[7]);
		if (head.type!=32) RAISE("unsupported grid type %d in file",head.type);
		// apparently, head.type 8 and 16 worked too.
		if (head.reserved!=0) RAISE("unsupported grid reserved field %d in file",head.reserved);
		if (head.dimn>16) RAISE("unsupported grid number of dimensions %d in file",head.dimn);
		int32 dimv[head.dimn];
		fread(dimv,head.dimn,4,f);
		if (endian != is_le()) swap32(head.dimn,(uint32 *)dimv);
		dim = new Dim(head.dimn,dimv);
	}
	GridOutlet out(this,0,dim,nt);
	long nn = dim->prod();
#define FOO(T) {T data[nn]; fread(data,nn,sizeof(T),f); out.send(nn,(T *)data);}
TYPESWITCH(nt,FOO,)
#undef FOO
	SUPER;
}

GRID_INLET(FormatGrid,0) {
	if (!headerless) {
		fwrite(&head,1,8,f);
		fwrite(in->dim->v,in->dim->n,4,f); // forgot the endian here
	}
} GRID_FLOW {
#define FOO(T) {T data2[n]; for(int i=0; i<n; i++) data2[i]=(T)data[i]; \
		if (endian!=is_le()) swap_endian(n,data2); \
		fwrite(data2,n,sizeof(T),f);}
TYPESWITCH(nt,FOO,)
#undef FOO
} GRID_FINISH {
	fflush(f);
} GRID_END

\def 0 headerless_m (...) {
	if (argc>=0 && TYPE(argv[0])==T_ARRAY) {argc = rb_ary_len(argv[0]); argv = rb_ary_ptr(argv[0]);}
	int v[argc];
	for (int i=0; i<argc; i++) v[i] = NUM2INT(argv[i]);
	headerless = new Dim(argc,v);
}
\def 0 headerful () { headerless = 0; }
//#!@#$ method name conflict ?
\def 0 type (NumberTypeE nt) {
	//!@#$ bug: should not be able to modify this _during_ a transfer
	switch (nt) {
	case uint8_e: head.type= 8; break;
	case int16_e: head.type=16; break;
	case int32_e: head.type=32; break;
	default: RAISE("unsupported type");
	}
	this->nt = nt;
}
\def void raw_open_gzip_in(String filename) {
	RAISE("temporarily disabled. ask matju.");
	//r,w = IO.pipe
	//if (pid=fork) {GridFlow.subprocesses[pid]=true; w.close; @stream = r;}
	//else {r.close; STDOUT.reopen w; STDIN.reopen filename, "r"; exec "gzip", "-dc";}
}
\def void raw_open_gzip_out(String filename) {
	RAISE("temporarily disabled. ask matju.");
	//r,w = IO.pipe
	//if (pid=fork) {GridFlow.subprocesses[pid]=true; r.close; @stream = w;}
	//else {w.close; STDIN.reopen r; STDOUT.reopen filename, "w"; exec "gzip", "-c";}
}
\def void raw_open(Symbol mode, String filename) {
	const char *fmode;
	if (mode==SYM(in))  fmode="r"; else
	if (mode==SYM(out)) fmode="w"; else
	RAISE("bad mode");
	//@stream.close if @stream
	if (mode==SYM(in)) {filename = rb_funcall(mGridFlow,SI(find_file),1,filename);}
	RAISE("raw_open code missing here");
	//@stream = File.open(filename,fmode); break;
//	case gzfile:
//		if (mode==SYM(in)) {filename = GridFlow.find_file(filename);}
//		if (mode==:in) {raw_open_gzip_in filename; else raw_open_gzip_out filename;}
//		def self.rewind() raw_open(*@raw_open_args); @frame = 0 end unless @rewind_redefined
//		@rewind_redefined = true
}
\def 0 close () {
	//@stream.close if @stream
	//GridFlow.hunt_zombies
}
\end class FormatGrid {install_format("#io:grid",1,1,6,"grid");}

void startup_format () {
	\startall
}
