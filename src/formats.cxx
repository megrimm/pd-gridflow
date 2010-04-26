/*
	$Id$

	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

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

#include "gridflow.hxx.fcs"
#include <string>
#include <map>
#include <algorithm>
#include <errno.h>
#define L _L_

/* API (version 0.9.3)
	mode is :in or :out
	def initialize(mode,*args) :
		open a file handler (do it via .new of class)
	attr_reader :description :
		a _literal_ (constant) string describing the format handler
	def self.info() optional :
		return a string describing the format handler differently
		than self.description(). in particular, it can list
		compile-time options and similar things. for example,
		quicktime returns a list of codecs.
	def 0 bang() :
		read one frame, send through outlet 0
		return values :
			Integer >= 0 : frame number of frame read.
			false : no frame was read : end of sequence.
			nil : a frame was read, but can't say its number.
		note that trying to read a nonexistent frame should no longer
		rewind automatically (@in handles that part), nor re-read the
		last frame (mpeg/quicktime used to do this)
	def 0 seek(Integer i) :     select one frame to be read next (by number)
	def 0 grid() : frame to write
	def 0 get (optional Symbol s) : get one attribute value or all of them
	def 0 ...() : options
	outlet 0 grid() frame just read
	outlet 0 ...() everything else
	destructor : close a handler
*/

std::map<std::string,std::string> suffix_table;
void suffixes_are (const char *name, const char *suffixes) {
	std::string name2 = name;
	char *suff2 = strdup(suffixes);
	char *suff3 = suff2+strlen(suff2);
	for (char *s=suff2; s<suff3; s++) if (*s==' ' || *s==',') *s=0;
	for (char *s=suff2; s<suff3; s+=strlen(s)+1) {
		std::string ss = s;
		suffix_table[ss]=name2;
	}
}

\class SuffixLookup : FObject {
  \constructor () {}
  \decl 0 symbol (t_symbol *str);
};
\def 0 symbol (t_symbol *str) {
	char *s = strdup(str->s_name);
	char *t = strrchr(s,'.');
	if (!t) outlet_symbol(outlets[2],gensym(s));
	else {
		*t = 0;
		for (char *u=t+1; *u; u++) *u=tolower(*u);
		outlet_symbol(outlets[1],gensym(t+1));
		std::map<std::string,std::string>::iterator u = suffix_table.find(std::string(t+1));
		if (u==suffix_table.end()) outlet_bang(outlets[0]);
		else outlet_symbol(outlets[0],gensym((char *)u->second.data()));
	}
	free(s);
}
\end class {install("gf.suffix_lookup",1,3);}

\class Format : FObject
Format::Format (BFObject *bself, MESSAGE) : FObject(bself,MESSAGE2) {
	mode=0; fd=-1; f=0; cast=int32_e; frame=0;
	if (argv[0]==gensym("out")) this->mode=2; else
	if (argv[0]==gensym("in"))  this->mode=4; else RAISE("unknown mode");
//	case mode
//	when  :in; flags[2]==1
//	when :out; flags[1]==1
//	else raise "Format opening mode is incorrect"
	//end or raise "Format '#{self.class.instance_eval{@symbol_name}}' does not support mode '#{mode}'"
}

\def 0 open(t_symbol *mode, string filename) {
	const char *fmode;
	#if __WIN32__
	if (mode==gensym("in"))  fmode="rb"; else
	if (mode==gensym("out")) fmode="wb"; else
	#else
	if (mode==gensym("in"))  fmode="r"; else
	if (mode==gensym("out")) fmode="w"; else
	#endif
	RAISE("bad mode");
	if (f) _0_close(0,0);
	if (mode==gensym("in")) {filename = gf_find_file(filename);}
	f = fopen(filename.data(),fmode);
	if (!f) RAISE("can't open file '%s': %s",filename.data(),strerror(errno));
	fd = fileno(f);
//	case gzfile:
//		if (mode==SYM(in)) {filename = GridFlow.find_file(filename);}
//		if (mode==:in) {raw_open_gzip_in filename; else raw_open_gzip_out filename;}
//		def self.rewind() raw_open(*@raw_open_args); @frame = 0 end unless @rewind_redefined
//		@rewind_redefined = true
}
\def 0 close() {
	//post("CLOSE! f=%p fd=%d",f,fd);
	if (f) {fclose(f); f=0;}
	if (fd>=0) {close(fd); fd=-1;} // ???
}
\def 0 cast(NumberTypeE nt) {cast = nt;}

\def 0 seek(int frame) {
	if (!frame) {_0_rewind(0,0); return;}
	RAISE("don't know how to seek for frame other than # 0");
}

// this is what you should use to rewind
// different file-sources may redefine this as something else
// (eg: gzip)
\def 0 rewind () {
	if (!f) RAISE("Nothing to rewind about...");
	fseek(f,0,SEEK_SET);
	frame = 0;
}

Format::~Format () {_0_close(0,0);}
\end class Format {}

/* This is the Grid format I defined: */
struct GridHeader {
	char magic[5]; // = "\x7fgrid" on little endian, "\x7fGRID" on big endian
	uint8 type; // supported: 8=int8 9=uint8 16=int16 32=int32
		    // unsupported: 34=float32 64=int64 66=float64
		   // (number of bits is multiple of 8; add 1 for unsigned; add 2 for float)
	uint8 reserved; // set this to 0 all of the time.
	uint8 dimn; // number of dimensions supported: at least 0..4)
	// int32 dimv[dimn]; // number of elements in each dimension. (in the file's endianness!)
	// raw data goes after that
};

\class FormatGrid : Format {
	GridHeader head;
	int endian;
	NumberTypeE nt;
	P<Dim> headerless_dim; // if null: headerful; if Dim: it is the assumed dimensions of received grids
	\grin 0
	\constructor (t_symbol *mode, string filename) {
		nt = int32_e;
		endian = is_le();
		_0_open(0,0,mode,filename);
	}
	\decl 0 bang ();
	\decl 0 headerless (...);
	\decl 0 headerful ();
	\decl 0 seek_byte (int64 bytepos);
	~FormatGrid() {
		//@stream.close if @stream
		//GridFlow.hunt_zombies
	}
//	\decl void raw_open_gzip_in(string filename);
//	\decl void raw_open_gzip_out(string filename);
};
\def 0 bang () {
	//post("#io.grid 0 bang: ftell=%ld",ftell(f));
	P<Dim> dim;
	if (feof(f)) {outlet_bang(bself->te_outlet); return;}
	//post("#in grid bang: offset %ld",ftell(f));
	if (headerless_dim) {
		dim = headerless_dim;
	} else {
		int r = fread(&head,1,8,f);
		if (feof(f)) {outlet_bang(bself->te_outlet); return;} /* damn */
		if (r<8) RAISE("can't read header: got %d bytes instead of 8",r);
		uint8 *m = (uint8 *)head.magic;
		if (strncmp((char *)m,"\x7fgrid",5)==0) endian=1; else
		if (strncmp((char *)m,"\x7fGRID",5)==0) endian=0; else
		RAISE("unknown header, can't read grid from file: "
			"%02x %02x %02x %02x %02x %02x %02x %02x",
			m[0],m[1],m[2],m[3],m[4],m[5],m[6],m[7]);
		switch (head.type) {
		case 8: nt=uint8_e; break; // sorry, was supposed to be signed.
		case 9: nt=uint8_e; break;
		case 16: nt=int16_e; break;
		case 32: nt=int32_e; break;
		default: RAISE("unsupported grid type %d in file",head.type);
		}
		// apparently, head.type 8 and 16 worked too.
		if (head.reserved!=0) RAISE("unsupported grid reserved field %d in file",head.reserved);
		if (head.dimn>16) RAISE("unsupported grid number of dimensions %d in file",head.dimn);
		int32 dimv[head.dimn];
		if (fread(dimv,1,head.dimn*4,f)<size_t(head.dimn*4)) RAISE("can't read dimension list");
		if (endian != is_le()) swap32(head.dimn,(uint32 *)dimv);
		dim = new Dim(head.dimn,dimv);
	}
	GridOutlet out(this,0,dim,nt);
	long nn = dim->prod();
	
#define FOO(T) {T data[nn]; size_t nnn = fread(data,1,nn*sizeof(T),f); \
	if (nnn<nn*sizeof(T)) pd_error(bself,"can't read grid data (body): %s", feof(f) ? "end of file" : strerror(ferror(f))); \
	CLEAR(data+nnn/sizeof(T),nn-nnn/sizeof(T)); \
	out.send(nn,(T *)data);}
TYPESWITCH(nt,FOO,)
#undef FOO
	SUPER;
}

GRID_INLET(0) {
	if (!headerless_dim) {
		strncpy(head.magic,is_le()?"\x7fgrid":"\x7fGRID",5);
		switch (in->nt) {
		case uint8_e: head.type = 9; break;
		case int16_e: head.type = 16; break;
		case int32_e: head.type = 32; break;
		default: RAISE("can't write that type of number to a file");
		}
		head.reserved = 0;
		head.dimn = in->dim->n;
		size_t sz = 4*in->dim->n;
#define FRAISE(funk,f) RAISE("can't "#funk": %s",ferror(f));
		if (fwrite(&head,1,8,f      )< 8) FRAISE(fwrite,f);
		if (fwrite(in->dim->v,1,sz,f)<sz) FRAISE(fwrite,f);
	}
} GRID_FLOW {
#define FOO(T) {T data2[n]; for(int i=0; i<n; i++) data2[i]=(T)data[i]; \
		if (endian!=is_le()) swap_endian(n,data2); \
		size_t sz = n*sizeof(T); \
		if (fwrite(data2,1,sz,f)<sz) FRAISE(fwrite,f);}
TYPESWITCH(in->nt,FOO,)
#undef FOO
} GRID_FINISH {
	fflush(f);
} GRID_END

\def 0 headerless (...) {
	if (argc>=0 && argv[0].a_type==A_LIST) {
		t_binbuf *b = (t_binbuf *)argv[0]; argc = binbuf_getnatom(b); argv = (t_atom2 *)binbuf_getvec(b);}
	int v[argc];
	for (int i=0; i<argc; i++) v[i] = argv[i];
	headerless_dim = new Dim(argc,v);
}
\def 0 headerful () {headerless_dim = 0;}
\def 0 seek_byte (int64 pos) {fseek(f,pos,SEEK_SET);}

//\def void raw_open_gzip_in(string filename) {
	//r,w = IO.pipe
	//if (pid=fork) {GridFlow.subprocesses[pid]=true; w.close; @stream = r;}
	//else {r.close; STDOUT.reopen w; STDIN.reopen filename, "r"; exec "gzip", "-dc";}
//\def void raw_open_gzip_out(string filename) {
	//r,w = IO.pipe
	//if (pid=fork) {GridFlow.subprocesses[pid]=true; r.close; @stream = w;}
	//else {w.close; STDIN.reopen r; STDOUT.reopen filename, "w"; exec "gzip", "-c";}

\end class FormatGrid {install_format("#io.grid",6,"grid");}

////////////////////////////////////////////////////////////////////////////////////////////////

\class FormatNetPBM : Format {
	\grin 0
	\constructor (t_symbol *mode, string filename) {
		Format::_0_open(0,0,mode,filename);
	}
	uint32 getuint() {
		uint32 i=0;
		char ch; do {
			ch = fgetc(f);
			if (ch=='#') do {ch=fgetc(f);} while (ch!='\n' && ch!='\r');
		} while (isspace(ch));
		if (!isdigit(ch)) RAISE("error reading header: expected uint");
		do {
			uint32 d = ch-'0';
			if (i > 0xffffffff/10 - d) RAISE("error reading header: uint too big");
			i = i * 10 + d;
			ch = fgetc(f);
		} while (isdigit(ch));
		return i;
	}
	\decl 0 bang ();
};
#define RERR RAISE("error reading header: %s",feof(f) ? "end of file" : strerror(ferror(f)))
\def 0 bang () {
	int a = fgetc(f); if (a==EOF) RAISE("error reading header magic");
	int b = fgetc(f); if (a==EOF) RAISE("error reading header magic");
	if (a!='P') RAISE("not a pbm/pgm/ppm/pam file (expected 'P')");
	if (b!='6') RAISE("expected a P6");
	int sx = getuint();
	int sy = getuint();
	int maxnum = getuint();
	if (maxnum!=255) RAISE("expected max to be 255 (8 bits per value)");
	int sc = 3;
	size_t sxc = sx*sc;
	GridOutlet out(this,0,new Dim(sy,sx,sc),cast);
	uint8 row[sx*3];
	for (int y=0; y<sy; y++) {
		if (fread(row,1,sxc,f)<sxc) RERR;
		out.send(sxc,row);
	}

}
GRID_INLET(0) {
	if (in->dim->n!=3) RAISE("need 3 dimensions");
	if (in->dim->v[2]!=3) RAISE("need 3 channels");
	fprintf(f,"P6\n");
	fprintf(f,"%d %d 255\n",in->dim->v[1],in->dim->v[0]);
	in->set_chunk(1);
} GRID_FLOW {
	int sx = in->dim->v[1];
	int sc = in->dim->v[2];
	size_t sxc = sx*sc;
	uint8 row[sx*3];
	while (n) {
		for (int i=0,j=0; i<sx; i++,j+=3) {
			row[j+0] = data[j+0];
			row[j+1] = data[j+1];
			row[j+2] = data[j+2];
		}
		if (fwrite(row,1,sxc,f)<sxc) RAISE("write error: %s",strerror(ferror(f)));
		data+=sxc; n-=sxc;
	}
} GRID_FINISH {
	fflush(f);
} GRID_END
\classinfo {install_format("#io.ppm",6,"ppm pgm pnm pam");}
\end class FormatNetPBM

////////////////////////////////////////////////////////////////////////////////////////////////

// this line goes with colorspace.hxx
int cliptab[1024];

void startup_format () {
	\startall
	for (int i=0; i<1024; i++) cliptab[i] = min(255,max(0,i-384));
}
