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
#include <string>
#include <map>
#define L _L_

/* API (version 0.9.1)
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
	def 0 length() : ^Integer   returns number of frames (never implemented ?)
	def 0 grid() : frame to write
	def 0 ...() : options
	outlet 0 grid() frame just read
	outlet 1 ...() everything else
	destructor : close a handler
*/

std::map<std::string,std::string> suffix_table;
std::map<std::string,std::string> format_table;
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
  \decl void initialize ();
  \decl 0 symbol (t_symbol *str);
};
\def void initialize () {}
\def 0 symbol (t_symbol *str) {
	char *s = strdup(str->s_name);
	char *t = strrchr(s,'.');
	if (!t) outlet_symbol(bself->out[2],gensym(s));
	else {
		*t = 0;
		outlet_symbol(bself->out[1],gensym(t+1));
		std::map<std::string,std::string>::iterator u = suffix_table.find(std::string(t+1));
		if (u!=suffix_table.end()) outlet_symbol(bself->out[0],gensym((char *)u->second.data()));
	}
	free(s);
}
\end class SuffixLookup {install("gf.suffix_lookup",1,3);}

// not in use
\class FormatLookup : FObject {
  \decl void initialize ();
  \decl 0 symbol (string str);
};
\def void initialize () {}
\def 0 symbol (string str) {
	std::map<std::string,std::string>::iterator u = format_table.find(str);
	if (u!=format_table.end()) outlet_symbol(bself->out[0],gensym((char *)u->second.data()));
	else outlet_bang(bself->out[0]);
}
\end class FormatLookup {install("gf.format_lookup",1,1);}

\class Format : GridObject
\def void initialize (t_symbol *mode, ...) {
	SUPER;
	if (mode==gensym("out")) this->mode=2; else
	if (mode==gensym("in"))  this->mode=4; else
		RAISE("unknown mode");
	this->frame = 0;
//	case mode
//	when  :in; flags[2]==1
//	when :out; flags[1]==1
//	else raise "Format opening mode is incorrect"
//	//end or raise "Format '#{self.class.instance_eval{@symbol_name}}' does not support mode '#{mode}'"
}

\def 0 open(t_symbol *mode, string filename) {
	const char *fmode;
	if (mode==gensym("in"))  fmode="r"; else
	if (mode==gensym("out")) fmode="w"; else
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
\def 0 close() {if (f) {fclose(f); f=0; fd=-1;}}
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

// This is different from IO#eof, which waits until a read has failed
// doesn't work in nonblocking mode? (I don't recall why)
//\def 0 eof () {
//	thispos = (@stream.seek 0,IO::SEEK_CUR; @stream.tell)
//	lastpos = (@stream.seek 0,IO::SEEK_END; @stream.tell)
//	@stream.seek thispos,IO::SEEK_SET
//	return thispos == lastpos
//rescue Errno::ESPIPE # just ignore if seek is not possible
//	return false
//}

// "ideal" buffer size or something
// the buffer may be bigger than this but usually not by much.
// def self.buffersize; 16384 end
\end class Format {}

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

\class FormatGrid : Format {
	GridHeader head;
	int endian;
	NumberTypeE nt;
	P<Dim> headerless; // if null: headerful; if Dim: it is the assumed dimensions of received grids
	\grin 0
	\decl void initialize(t_symbol *mode, string filename);
	\decl 0 bang ();
	\decl 0 headerless_m (...);
	\decl 0 headerful ();
	\decl 0 type (NumberTypeE nt);
	~FormatGrid() {
		//@stream.close if @stream
		//GridFlow.hunt_zombies
	}
//	\decl void raw_open_gzip_in(string filename);
//	\decl void raw_open_gzip_out(string filename);
};

\def void initialize(t_symbol *mode, string filename) {
	SUPER;
	strncpy(head.magic,is_le()?"\7fgrid":"\7fGRID",5);
	head.type = 32;
	_0_open(0,0,mode,filename);
}
\def 0 bang () {
	P<Dim> dim;
	if (feof(f)) {outlet_bang(bself->te_outlet); return;}
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

//\def void raw_open_gzip_in(string filename) {
	//r,w = IO.pipe
	//if (pid=fork) {GridFlow.subprocesses[pid]=true; w.close; @stream = r;}
	//else {r.close; STDOUT.reopen w; STDIN.reopen filename, "r"; exec "gzip", "-dc";}
//\def void raw_open_gzip_out(string filename) {
	//r,w = IO.pipe
	//if (pid=fork) {GridFlow.subprocesses[pid]=true; r.close; @stream = w;}
	//else {w.close; STDIN.reopen r; STDOUT.reopen filename, "w"; exec "gzip", "-c";}

\end class FormatGrid {install_format("#io.grid",6,"grid");}

void startup_format () {
	\startall
}