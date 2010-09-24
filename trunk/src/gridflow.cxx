/*
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
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#ifndef __WIN32__
	#include <sys/resource.h>
#endif
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
//#include <cstdlib>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>

extern "C" {
#include "bundled/g_canvas.h"
#include "bundled/m_imp.h"
extern t_class *text_class;
};

/* for exception-handling in 0.9.x and 9.x... Linux-only */
#if !defined(MACOSX) && !defined(__WIN32__)
#include <exception>
#include <execinfo.h>
#endif
#undef check

#ifdef __GNUC__
#include <cxxabi.h>
#endif

map<t_symbol *,FClass *> fclasses;
map<t_class *,FClass *> fclasses_pd;
t_symbol *gridflow_folder;

//using namespace std;

#define FOO(SYM,NAME) t_symbol *s_##SYM;
BUILTIN_SYMBOLS(FOO)
#undef FOO

string t_atom2::to_s() const {ostringstream os; os << *this; return os.str();}

//#define ARRET kill(getpid(),SIGSTOP);
#define ARRET /**/

Barf::Barf(const char *s, ...) {
    ARRET
    ostringstream os; va_list ap; va_start(ap,s  ); voprintf(os,s  ,ap);
    va_end(ap); text = os.str();}
Barf::Barf(const char *file, int line, const char *func, const char *fmt, ...) {
    ARRET
    ostringstream os; va_list ap; va_start(ap,fmt); voprintf(os,fmt,ap);
    // oprintf(os,"\n%s:%d:in `%s'",file,line,func);
    //oprintf(os,"\n%s",short_backtrace(2,7));
    va_end(ap); text = os.str();}

void Barf::error(BFObject *bself, int winlet, t_symbol *sel) {
	//post("----------------------------------------------------------------");
	if (!bself) ::error("[???] inlet %d method %s: %s"                              ,winlet,sel?sel->s_name:"(???)",text.data());
	else    pd_error(bself,"%s inlet %d method %s: %s",bself->binbuf_string().data(),winlet,sel?sel->s_name:"(???)",text.data());
}
void Barf::error(t_symbol *s, int argc, t_atom *argv) {
	//post("----------------------------------------------------------------");
	ostringstream os; os << s->s_name;
	for (int i=0; i<argc; i++) os << (i ? " " : " ") << argv[i];
        ::error("[%s]: %s",os.str().data(),text.data());
}

// [#expr]
#define A_OPEN  t_atomtype(0x1000) /* only between next() and parse() */
#define A_CLOSE t_atomtype(0x1001) /* only between next() and parse() */
#define A_VAR   t_atomtype(0x1002) /* for $f1-style variables, not other variables */
#define A_OP1   t_atomtype(0x1003) /* operator: unary prefix */
#define A_OP    t_atomtype(0x1004) /* operator: binary infix, or not parsed yet */

#define Z(T) case T: return #T; break;
string atomtype_to_s (t_atomtype t) {
  switch (int(t)) {
    Z(A_FLOAT)Z(A_SYMBOL)Z(A_POINTER)Z(A_LIST)Z(A_GRID)Z(A_GRIDOUT)
    Z(A_DOLLAR)Z(A_DOLLSYM)Z(A_COMMA)Z(A_SEMI)
    Z(A_CANT)Z(A_NULL)
    Z(A_OP)Z(A_OP1)Z(A_VAR)Z(A_OPEN)Z(A_CLOSE)
    default: ostringstream os; oprintf(os,"unknown:%d",int(t)); return os.str();
  }
}
#undef Z

void pd_oprint (ostream &o, int argc, t_atom *argv) {
	for (int i=0; i<argc; i++) {
		t_atomtype t = argv[i].a_type;
		if (t==A_FLOAT) o << argv[i].a_float;
		else if (t==A_SYMBOL) o << argv[i].a_symbol->s_name;
		else if (t==A_POINTER) o << "(pointer)";
 		else if (t==A_COMMA) o << ",";
 		else if (t==A_SEMI) o << ";";
		else if (t==A_LIST) {
			t_binbuf *b = (t_binbuf *)argv[i].a_gpointer;
			o << "[";
			pd_oprint(o,binbuf_getnatom(b),binbuf_getvec(b));
			o << "]";
		} else o << "(atom of type " << atomtype_to_s(t) << ")";
		if (i!=argc-1) o << " ";
	}
}

void pd_post (const char *s, int argc, t_atom *argv) {
	ostringstream os;
	if (s) os << s << ": ";
	pd_oprint(os,argc,argv);
	post("%s",os.str().data());
}

// currently not supporting *m$ ... see man page
// will not support length modifiers at all (hh h l ll L q j z t)
// currently not supporting 'formats' p n m
static float     eatfloat  (int argc, t_atom *argv, int &i) {if (!argc) RAISE("not enough args");
	if (argv[i].a_type != A_FLOAT) RAISE("expected float in $%d",i+1);	return argv[i++].a_float;}
static t_symbol *eatsymbol (int argc, t_atom *argv, int &i) {if (!argc) RAISE("not enough args");
	if (argv[i].a_type != A_SYMBOL) RAISE("expected symbol in $%d",i+1);	return argv[i++].a_symbol;}
#define EATFLOAT   eatfloat(argc,argv,i)
#define EATSYMBOL eatsymbol(argc,argv,i)
void pd_oprintf (ostream &o, const char *s, int argc, t_atom *argv) {
	int i=0;
	const char *t;
	for (; *s; s++) {
		if (*s!='%') {o << (char)*s; continue;}
		t=s;
		int n=0; // number of args to eat before the actual value
		s++; // skip the %
		while (strchr("#0- +'I",*s)) s++; // skip flags
		if (*s>='1' && *s<='9') {do {s++;} while (*s>='0' && *s<='9');} // skip field width
		else if (*s=='*') {s++; n++;}
		if (*s=='.') { // precision
			s++;
			if (*s>='1' && *s<='9') {do {s++;} while (*s>='0' && *s<='9');} // skip field width
			else if (*s=='*') {s++; n++;}
		}
		if (strchr("hlLqjzt",*s)) RAISE("can't use length modifier '%c' in the context of pd",*s);
		char form[t-s+2]; sprintf(form,"%.*s",int(s-t+1),t);
		int k[2]={0,0}; // fake initialisation just to shut up gcc warning
		if (n>0) k[0]=int(EATFLOAT);
		if (n>1) k[1]=int(EATFLOAT);
		if (strchr("cdiouxX",*s)) {switch (n) {
			case 0: oprintf(o,form,          int(EATFLOAT)); break;
			case 1: oprintf(o,form,k[0],     int(EATFLOAT)); break;
			case 2: oprintf(o,form,k[0],k[1],int(EATFLOAT)); break;
		}} else if (strchr("eEfFgGaA",*s)) {switch (n) {
			case 0: oprintf(o,form,          EATFLOAT); break;
			case 1: oprintf(o,form,k[0],     EATFLOAT); break;
			case 2: oprintf(o,form,k[0],k[1],EATFLOAT); break;
		}} else if (strchr("s",*s)) {switch (n) {
			case 0: oprintf(o,form,          EATSYMBOL->s_name); break;
			case 1: oprintf(o,form,k[0],     EATSYMBOL->s_name); break;
			case 2: oprintf(o,form,k[0],k[1],EATSYMBOL->s_name); break;
		}} else if (strchr("%",*s)) o << "%";
		else RAISE("sorry, the format character '%c' is not supported yet, in \"%.*s\"",*s,s+1-t,t);
	}
}

// backslash is doubled here because Pd doesn't do it and Tcl's eval interprets a double backslash as a single.
ostream &operator << (ostream &self, const t_atom &a) {
	switch (int(a.a_type)) {
		case A_FLOAT:   self << a.a_float; break;
		case A_SYMBOL:  self << a.a_symbol->s_name; break; // i would rather show backslashes here...
		case A_DOLLSYM: self << a.a_symbol->s_name; break; // for real, it's the same thing as A_SYMBOL in pd >= 0.40
		case A_POINTER: self << "\\\\p(0x" << std::hex << (uintptr_t)a.a_gpointer << std::dec << ")"; break;
		case A_COMMA:   self << ","; break;
		case A_SEMI:    self << ";"; break;
		case A_DOLLAR:  self << "$" << a.a_w.w_index; break;
		case A_LIST: {
			t_list *b = (t_list *)a.a_gpointer;
			int argc = binbuf_getnatom(b);
			t_atom *argv = binbuf_getvec(b);
			self << "(";
			for (int i=0; i<argc; i++) {self << argv[i]; if (i<argc-1) self << " ";}
			self << ")";
			break;
		}
		case A_NULL: case A_CANT: case A_OPEN: case A_CLOSE:
			   self << "\\\\a(" << atomtype_to_s(a.a_type)                                                << ")"; break;
		case A_VAR:self << "\\\\a(" << atomtype_to_s(a.a_type) << " " << std::hex << char(a.a_index>>8) << int(a.a_index&255) << std::dec << ")"; break;
		case A_OP:
		case A_OP1:self << "\\\\a(" << atomtype_to_s(a.a_type) << " " << std::hex << a.a_symbol   << std::dec << ")"; break;
		default:   self << "\\\\a(" << atomtype_to_s(a.a_type) << " " << std::hex << a.a_gpointer << std::dec << ")"; break;
	}
	return self;
}

// adapted from desiredata/src/kernel.c
void PtrOutlet::operator () (t_atom &a) {
    if      (a.a_type==A_FLOAT  ) (*this)(a.a_float);
    else if (a.a_type==A_SYMBOL ) (*this)(a.a_symbol);
    else if (a.a_type==A_POINTER) (*this)(a.a_gpointer);
    else if (a.a_type==A_LIST   ) (*this)((t_list *)a.a_gpointer);
    else if (a.a_type==A_BLOB   ) (*this)((t_blob *)a.a_gpointer);
    else {string s = atomtype_to_s(a.a_type); RAISE("can't send atom whose type is %s",s.data());}
    //self(1,av);
}

bool t_atom2::operator == (const t_atom2 &b) {
	const t_atom2 &a = *this;
	if (a.a_type!=b.a_type) return false;
	if (a.a_type==A_FLOAT)   return a.a_float   ==b.a_float;
	if (a.a_type==A_SYMBOL)  return a.a_symbol  ==b.a_symbol;
	if (a.a_type==A_POINTER) return a.a_gpointer==b.a_gpointer; // not deep
	if (a.a_type==A_LIST)    return a.a_gpointer==b.a_gpointer; // not deep
	if (a.a_type==A_BLOB)    return a.a_gpointer==b.a_gpointer; // not deep
	string s = atomtype_to_s(a.a_type);
	RAISE("don't know how to compare elements of type %s",s.data());
}
//----------------------------------------------------------------
// Dim

void Dim::check() {
	if (n>MAX_DIM) RAISE("too many dimensions");
	for (int i=0; i<n; i++) if (v[i]<0) RAISE("Dim: negative dimension");
}

// !@#$ big leak machine?
// returns a string like "Dim[240,320,3]"
char *Dim::to_s() const {
	// if you blow 256 chars it's your own fault
	char buf[256];
	char *s = buf;
	s += sprintf(s,"Dim[");
	for(int i=0; i<n; i++) s += sprintf(s,"%s%d", ","+!i, v[i]);
	s += sprintf(s,"]");
	return strdup(buf);
}

NumberTypeE NumberTypeE_find (string s) {
	if (number_type_dict.find(s)==number_type_dict.end()) RAISE("unknown number type \"%s\"", s.data());
	return number_type_dict[s]->index;
}

NumberTypeE NumberTypeE_find (const t_atom &x) {
	if (x.a_type!=A_SYMBOL) RAISE("expected number-type (as symbol)");
	return NumberTypeE_find(string(x.a_symbol->s_name));
}

// don't touch.
static void gfmemcopy32(int32 *as, int32 *bs, long n) {
	ptrdiff_t ba = bs-as;
#define FOO(I) as[I] = (as+ba)[I];
		UNROLL_8(FOO,n,as)
#undef FOO
}

void gfmemcopy(uint8 *out, const uint8 *in, long n) {
	for (; n>16; in+=16, out+=16, n-=16) {
		((int32*)out)[0] = ((int32*)in)[0];
		((int32*)out)[1] = ((int32*)in)[1];
		((int32*)out)[2] = ((int32*)in)[2];
		((int32*)out)[3] = ((int32*)in)[3];
	}
	for (; n>4; in+=4, out+=4, n-=4) *(int32*)out = *(int32*)in;
	for (; n; in++, out++, n--) *out=*in;
}

//----------------------------------------------------------------

uint64 gf_timeofday () {
	timeval t;
	gettimeofday(&t,0);
	return t.tv_sec*1000000+t.tv_usec;
}

#define CONVERT0(z) (((uint32)in[z] >> chop[z]) << slide[z])
#define CONVERT1 t = CONVERT0(0) | CONVERT0(1) | CONVERT0(2)
#define CONVERT2 for (t=0,i=0; i<self->size; i++) t |= CONVERT0(i);

#define WRITE_LE \
	for (int bytes = self->bytes; bytes; bytes--, t>>=8) *out++ = t;

#define WRITE_BE {int bytes; \
	bytes = self->bytes; \
	while (bytes--) {out[bytes] = t; t>>=8;}\
	out += self->bytes;}

/* this macro would be faster if the _increment_
   was done only once every loop. or maybe gcc does it, i dunno */
#define NTIMES(_x_) \
	for (; n>=4; n-=4) {_x_ _x_ _x_ _x_} \
	for (; n; n--) {_x_}

/* this could be faster (use asm) */
void swap64 (long n, uint64 *data) {
	NTIMES({
		uint64 x = *data;
		x = (x<<32) | (x>>32);
		x = ((x&0x0000ffff0000ffffLL)<<16) | ((x>>16)&0x0000ffff0000ffffLL);
		x = ((x&0x00ff00ff00ff00ffLL)<< 8) | ((x>> 8)&0x00ff00ff00ff00ffLL);
		*data++ = x;
	})
}

/* this could be faster (use asm) */
void swap32 (long n, uint32 *data) {
	NTIMES({
		uint32 x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	})
}

/* this could be faster (use asm or do it in int32 chunks) */
void swap16 (long n, uint16 *data) {NTIMES({ uint16 x = *data; *data++ = (x<<8) | (x>>8); })}

/* **************************************************************** */

//#define DEBUG 1
#ifdef DEBUG
#define TRACE static int use=0; use++; if ((use%10000)==0) post("%s",__PRETTY_FUNCTION__);
#else
#define TRACE
#endif

#define   PACKER(NAME) template <class T> static void NAME (BitPacking *self, long n, T *in, uint8 *out)
#define UNPACKER(NAME) template <class T> static void NAME (BitPacking *self, long n, uint8 *in, T *out)

PACKER(default_pack) {TRACE
	uint32 t;
	int i;
	int sameorder = self->endian==2 || self->endian==::is_le();
	int size = self->size;
	uint32  mask[4]; memcpy(mask,self->mask,size*sizeof(uint32));
	uint32    hb[4]; for (i=0; i<size; i++) hb[i] = highest_bit(mask[i]);
	uint32  span[4]; for (i=0; i<size; i++) span[i] = hb[i] - lowest_bit(mask[i]);
	uint32  chop[4]; for (i=0; i<size; i++) chop[i] = 7-span[i];
	uint32 slide[4]; for (i=0; i<size; i++) slide[i] = hb[i]-span[i];
	
	if (sameorder && size==3) {
		switch(self->bytes) {
		case 2:	NTIMES(CONVERT1; *((int16 *)out)=t; out+=2; in+=3;) return;
		case 4:	NTIMES(CONVERT1; *((int32 *)out)=t; out+=4; in+=3;) return;
		}
	}
	if (self->is_le()) {
		switch (size) {
		case 3: for (; n--; in+=3) {CONVERT1; WRITE_LE;} break;
		case 4:	for (; n--; in+=4) {CONVERT1; WRITE_LE;} break;
		default:for (; n--; in+=size) {CONVERT2; WRITE_LE;}}
	} else {
		switch (size) {
		case 3: for (; n--; in+=3) {CONVERT1; WRITE_BE;} break;
		case 4: for (; n--; in+=4) {CONVERT1; WRITE_BE;} break;
		default:for (; n--; in+=size) {CONVERT2; WRITE_BE;}}
	}
}

#define LOOP_UNPACK(_reader_) \
	for (; n; n--) { \
		int bytes=0; uint32 temp=0; _reader_; \
		for (int i=0; i<self->size; i++, out++) { \
			uint32 t=temp&self->mask[i]; \
			*out = (t<<(7-hb[i]))|(t>>(hb[i]-7));}}

UNPACKER(default_unpack) {TRACE
	int hb[4];
	for (int i=0; i<self->size; i++) hb[i] = highest_bit(self->mask[i]);
	if (is_le()) { // smallest byte first
		LOOP_UNPACK(
			for(; self->bytes>bytes; bytes++, in++) temp |= *in<<(8*bytes);
		)
	} else { // biggest byte first
		LOOP_UNPACK(
			bytes=self->bytes; for (; bytes; bytes--, in++) temp=(temp<<8)|*in;
		)
	}
}

/* **************************************************************** */

PACKER(pack2_565) {TRACE
	uint32 chop[3] = {3,2,3};
	uint32 slide[3] = {11,5,0};
	uint32 t;
	NTIMES(CONVERT1; *((short *)out)=t; out+=2; in+=3;)
}

PACKER(pack3_888) {TRACE
	int32 *o32 = (int32 *)out;
	while (n>=4) {
		o32[0] = (in[5]<<24) | (in[ 0]<<16) | (in[ 1]<<8) | in[2];
		o32[1] = (in[7]<<24) | (in[ 8]<<16) | (in[ 3]<<8) | in[4];
		o32[2] = (in[9]<<24) | (in[10]<<16) | (in[11]<<8) | in[6];
		o32+=3; in+=12;
		n-=4;
	}
	out = (uint8 *)o32;
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}
UNPACKER(  unpack3_888) {TRACE NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2];           out+=3; in+=3; )}
  PACKER(   pack3_888c) {TRACE NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out[3]=0; out+=4; in+=3; )}
  PACKER(   pack3_888d) {TRACE NTIMES( out[0]=in[0]; out[1]=in[1]; out[2]=in[2]; out[3]=0; out+=4; in+=3; )}
UNPACKER( unpack3_888d) {TRACE NTIMES( out[0]=in[0]; out[1]=in[1]; out[2]=in[2];           out+=3; in+=4; )}
PACKER(pack3_bgrn8888b) {TRACE NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out[3]=0; out+=4; in+=4; )}

PACKER(pack3_888b) {TRACE
	int32 *o32 = (int32 *)out;
	while (n>=4) {
		o32[0] = (in[0]<<16) | (in [1]<<8) | in [2];
		o32[1] = (in[3]<<16) | (in [4]<<8) | in [5];
		o32[2] = (in[6]<<16) | (in [7]<<8) | in [8];
		o32[3] = (in[9]<<16) | (in[10]<<8) | in[11];
		o32+=4; in+=12;
		n-=4;
	}
	NTIMES( o32[0] = (in[0]<<16) | (in[1]<<8) | in[2]; o32++; in+=3; )
}

// (R,G,B,?) -> B:8,G:8,R:8,0:8
PACKER(pack3_bgrn8888) {TRACE /* NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=4; in+=4; ) */
	uint32 *i32 = (uint32 *)in, *o32 = (uint32 *)out;
	while (n>=4) {
		o32[0] = ((i32[0]&0xff)<<16) | (i32[0]&0xff00) | ((i32[0]>>16)&0xff);
		o32[1] = ((i32[1]&0xff)<<16) | (i32[1]&0xff00) | ((i32[1]>>16)&0xff);
		o32[2] = ((i32[2]&0xff)<<16) | (i32[2]&0xff00) | ((i32[2]>>16)&0xff);
		o32[3] = ((i32[3]&0xff)<<16) | (i32[3]&0xff00) | ((i32[3]>>16)&0xff);
		o32+=4; i32+=4; n-=4;
	}
	NTIMES( o32[0] = ((i32[0]&0xff)<<16) | (i32[0]&0xff00) | ((i32[0]>>16)&0xff); o32++; i32++; )
}

static uint32 bp_masks[][4] = {
	{0x0000f800,0x000007e0,0x0000001f,0},
	{0x00ff0000,0x0000ff00,0x000000ff,0},
	{0x000000ff,0x0000ff00,0x00ff0000,0},
};

#define ANYCASE(a) {a,a,a}
static Packer bp_packers[] = {
	ANYCASE(default_pack),
	ANYCASE(pack2_565),
	ANYCASE(pack3_888),
	{pack3_888b, default_pack, default_pack}, /* {pack3_888c, pack3_888c, pack3_888c}, not tested */
	{pack3_bgrn8888, pack3_bgrn8888b, pack3_bgrn8888b},
	ANYCASE(pack3_888d),
};

static Unpacker bp_unpackers[] = {
	ANYCASE(default_unpack),
	ANYCASE(unpack3_888),
	{pack3_bgrn8888, default_unpack, default_unpack},
	ANYCASE(unpack3_888d),
};	

static BitPacking builtin_bitpackers[] = {
	BitPacking(2, 2, 3, bp_masks[0], &bp_packers[1], &bp_unpackers[0]),
	BitPacking(1, 3, 3, bp_masks[1], &bp_packers[2], &bp_unpackers[1]),
	BitPacking(1, 4, 3, bp_masks[1], &bp_packers[3], &bp_unpackers[0]),
	BitPacking(1, 4, 4, bp_masks[1], &bp_packers[4], &bp_unpackers[2]),
	BitPacking(1, 4, 3, bp_masks[2], &bp_packers[5], &bp_unpackers[3]),
};

/* **************************************************************** */

bool BitPacking::eq(BitPacking *o) {
	if (!(bytes == o->bytes)) return false;
	if (!(size == o->size)) return false;
	for (int i=0; i<size; i++) {
		if (!(mask[i] == o->mask[i])) return false;
	}
	if (endian==o->endian) return true;
	/* same==little on a little-endian; same==big on a big-endian */
	return (endian ^ o->endian ^ ::is_le()) == 2;
}

void post_BitPacking(BitPacking *b) {
	::post("Bitpacking: endian=%d bytes=%d size=%d packer=%d unpacker=%d",
		b->endian,b->bytes,b->size,int(b->packer-bp_packers),int(b->unpacker-bp_unpackers));
	::post("  mask=[0x%08x,0x%08x,0x%08x,0x%08x]",b->mask[0],b->mask[1],b->mask[2],b->mask[3]);
}

BitPacking::BitPacking(int endian, int bytes, int size, uint32 *mask, Packer *packer, Unpacker *unpacker) {
	this->endian = endian;
	this->bytes = bytes;
	this->size = size;
	for (int i=0; i<size; i++) this->mask[i] = mask[i];
	if (packer) {this->packer = packer; this->unpacker = unpacker; return;}
	int packeri=-1;
	this->packer = &bp_packers[0];
	this->unpacker = &bp_unpackers[0];
	for (int i=0; i<(int)(sizeof(builtin_bitpackers)/sizeof(BitPacking)); i++) {
		BitPacking *bp = &builtin_bitpackers[i];
		if (this->eq(bp)) {
			this->  packer = bp->  packer;
			this->unpacker = bp->unpacker;
			packeri=i;
			goto end;
		}
	}
end:;
}

bool BitPacking::is_le() {return endian==1 || (endian ^ ::is_le())==3;}

/*
#undef TRACE
#ifdef DEBUG
#define TRACE static int use=0; use++; if ((use%10000)==0) post_BitPacking(this);
#else
#define TRACE
#endif
*/

#ifdef __WIN32__
#define lstat stat
#endif

vector<string> gf_data_path;
string gf_find_file (string x) {
	if (strchr(x.data(),'/')) return x;
	int n = gf_data_path.size();
	struct stat dummy;
	for (int i=0; i<n; i++) {
		string s = gf_data_path[i]+"/"+x;
		if (lstat(s.data(),&dummy)==0) return s;
	}
	return x;
}

/* **************************************************************** */

#undef pd_class
#define pd_class(x) (*(t_pd *)x)
#define pd_classname(x) (fclasses_pd[pd_class(x)]->name->s_name)

// when winlet==0  : the message came to the receiver of the object (by left inlet or by receive-symbol)
// when winlet>0   : the message came through a proxy (non-left inlet)
// when winlet==-1 : \decl n ... for multi-inlet methods
// when winlet==-2 : \decl void anything : matches anything in all inlets (as a fallback)
//		     \decl ___get : attribute lister defined by source_filter
//                   other : attribute readers
static FMethod method_lookup (FClass *fc, int winlet, t_symbol *sel) {
    for (; fc; fc=fc->super) {
	typeof(fc->methods) &m = fc->methods;
	typeof(m.begin()) it = m.find(insel(winlet,sel)); if (it!=m.end()) return it->second;
	if (sel==&s_bang || sel==&s_float || sel==&s_symbol || sel==&s_pointer) {
		it = m.find(insel(winlet,&s_list)); if (it!=m.end()) return it->second;
	}
    }
    return 0;
}
static FMethod method_lookup (BFObject *bself, int winlet, t_symbol *sel) {
	return method_lookup(fclasses_pd[pd_class(bself)],winlet,sel);
}

void call_super(int argc, t_atom *argv) {/* unimplemented */}

//****************************************************************
// BFObject

struct BFProxy : t_object {
	BFObject *parent;
	t_inlet *inlet;
	int id;
};

static t_class *BFProxy_class;
static t_symbol *s_loadbang;

static void BFObject_loadbang (BFObject *bself) {
    try {FMethod m = method_lookup(bself,0,s_loadbang); m(bself->self,0,0);}
    catch (Barf &oozy) {oozy.error(bself,0,s_loadbang);}
}

static void BFObject_anything (BFObject *bself, int winlet, t_symbol *s, int ac, t_atom2 *at) {
    #ifdef DES_BUGS
	post("BFObject_anything bself=%08lx magic=%08lx self=%08lx",long(bself),bself->magic,long(bself->self));
    #endif
    try {
	t_atom2 argv[ac+2]; for (int i=0; i<ac; i++) argv[i+2] = at[i];
	int argc = handle_parens(ac,argv+2); // ought to be removed
	FMethod m;
	FObject *self = bself->self;
	m=method_lookup(bself,winlet,      s); if (m) {                           m(self,argc+0,argv+2); return;}
	m=method_lookup(bself,-1,          s); if (m) {argv[1]=winlet;            m(self,argc+1,argv+1); return;}
	m=method_lookup(bself,-2,&s_anything); if (m) {argv[0]=winlet; argv[1]=s; m(self,argc+2,argv+0); return;}

	if (s==&s_list) {
	    if (argc==0) {
						m=method_lookup(bself,winlet,&s_bang   ); if(m){m(self,argc,argv+2); return;}
	    } else if (argc==1) {
		if (argv[2].a_type==A_FLOAT)   {BFObject_anything(bself,winlet,&s_float  ,argc,argv+2); return;}
		if (argv[2].a_type==A_SYMBOL)  {BFObject_anything(bself,winlet,&s_symbol ,argc,argv+2); return;}
		if (argv[2].a_type==A_POINTER) {BFObject_anything(bself,winlet,&s_pointer,argc,argv+2); return;}
	    } else if (winlet==0) {
		for (int i=min(argc,self->ninlets)-1; i>=0; i--) { // not exactly same order as pd's obj_list...
		    if (argv[2+i].a_type==A_FLOAT)   BFObject_anything(bself,i,&s_float  ,1,argv+2+i);
		    if (argv[2+i].a_type==A_SYMBOL)  BFObject_anything(bself,i,&s_symbol ,1,argv+2+i);
		    if (argv[2+i].a_type==A_POINTER) BFObject_anything(bself,i,&s_pointer,1,argv+2+i);
		}
		return;
	    }
	}
	pd_error((t_pd *)bself, "method '%s' not found for inlet %d in class '%s'",s->s_name,winlet,pd_classname(bself));
    } catch (Barf &oozy) {oozy.error(bself,winlet,s);}
}
static void BFObject_blob (BFObject *bself, int winlet, t_blob *b) {
	t_atom2 a[] = {b};
	BFObject_anything(bself,winlet,gensym("blob"),1,a);
}

static void BFObject_anything0 (BFObject *self, MESSAGE)   {BFObject_anything(self        ,0       ,MESSAGE2);}
static void  BFProxy_anything  ( BFProxy *self, MESSAGE)   {BFObject_anything(self->parent,self->id,MESSAGE2);}
static void BFObject_blob0     (BFObject *self, t_blob *b) {BFObject_blob(    self        ,0       ,b);}
static void  BFProxy_blob      ( BFProxy *self, t_blob *b) {BFObject_blob(    self->parent,self->id,b);}

FObject::FObject (BFObject *bself, MESSAGE) {
	this->bself = bself;
	bself->self = this;
	string name = string(sel->s_name);
	mom = (t_canvas *)canvas_getcurrent();
	ninlets  = 1;
	noutlets = 0;
	inlets = new  BFProxy*[1];
	out    = new PtrOutlet[1];
	FClass *fc = fclasses[sel];
	inlets[0] = 0; // inlet 0 of this table is not in use
	ninlets_set( fc->ninlets ,false);
	noutlets_set(fc->noutlets,false);
}
FObject::~FObject () {
	ninlets_set(1,false);
	delete[] inlets;
	delete[] out;
}

static void dont_handle_parens (int ac, t_atom2 *av) {for (int i=0; i<ac; i++) if (av[i]==s_comma) SETCOMMA(&av[i]);}

static void *BFObject_new (t_symbol *classsym, int ac, t_atom *at) {
    string name = string(classsym->s_name);
    if (fclasses.find(classsym)==fclasses.end()) {post("GF: class not found: '%s'",classsym->s_name); return 0;}
    FClass *qlass = fclasses[classsym];
    BFObject *bself = (BFObject *)pd_new(qlass->bfclass);
    try {
	int argc = ac;
	t_atom2 argv[argc];
	for (int i=0; i<argc; i++) argv[i] = at[i];
	if (qlass->flags&CLASS_NOPARENS) dont_handle_parens(argc,argv); else argc = handle_parens(argc,argv);
	int j;
	for (j=0; j<argc; j++) if (argv[j].a_type==A_COMMA) break;
	bself->self = 0;
	t_allocator alloc = fclasses[classsym]->allocator;
	bself->te_binbuf = 0; //HACK: supposed to be 0 already (why this hack ? I don't recall)
	#ifdef DES_BUGS
		bself->magic = 0x66666600;
	#endif
	bself->self = alloc(bself,classsym,j,(t_atom2 *)argv);
	while (j<argc) {
		j++;
		int k=j;
		for (; j<argc; j++) if (argv[j].a_type==A_COMMA) break;
		if (argv[k].a_type==A_SYMBOL) pd_typedmess((t_pd *)bself,argv[k],j-k-1,argv+k+1);
		//else post("oupse");
	}
        #ifdef DES_BUGS
	    post("BFObject_new..... bself=%08lx magic=%08lx self=%08lx",long(bself),bself->magic,long(bself->self));
        #endif
	return bself;
    } catch (Barf &oozy) {oozy.error(classsym,ac,at); return 0;}
}

static void BFObject_delete (BFObject *bself) {
	try {
	     delete bself->self;
	     bself->self = (FObject *)0xdeadbeef;
	} catch (Barf &oozy) {oozy.error(bself,-1,gensym("destructor"));}
}

//****************************************************************

static void BFObject_undrawio (BFObject *bself) {
	t_canvas *mom = bself->self->mom;
	if (!mom || !glist_isvisible(mom)) return;
	t_rtext *rt = glist_findrtext(mom,bself);
	if (!rt) return;
	glist_eraseiofor(mom,bself,rtext_gettag(rt));
}

static void BFObject_redraw (BFObject *bself) {
	t_canvas *mom = bself->self->mom;
	if (!mom || !glist_isvisible(mom)) return;
	t_rtext *rt = glist_findrtext(mom,bself);
	if (!rt) return;
	gobj_vis((t_gobj *)bself,mom,0);
	gobj_vis((t_gobj *)bself,mom,1);
	canvas_fixlinesfor(mom,(t_text *)bself);
}

/* warning: deleting inlets that are connected will cause pd to crash */
void FObject::ninlets_set (int n, bool draw) {
	if (!bself->te_binbuf) draw=false;
	if (n<1) RAISE("ninlets_set: n=%d must be at least 1",n);
	if (draw) BFObject_undrawio(bself);
	if (ninlets<n) {
		BFProxy **noo = new BFProxy*[n];
		memcpy(noo,inlets,ninlets*sizeof(BFProxy*));
		delete[] inlets;
		inlets = noo;
		while (ninlets<n) {
			BFProxy *p = inlets[ninlets] = (BFProxy *)pd_new(BFProxy_class);
			p->parent = bself;
			p->id = ninlets;
			p->inlet = inlet_new(bself,&p->ob_pd,0,0);
			ninlets++;
		}
	} else {
		while (ninlets>n) {
			ninlets--;
			inlet_free(inlets[ninlets]->inlet);
			pd_free((t_pd *)inlets[ninlets]);
		}
	}
	if (draw) BFObject_redraw(bself);
}
/* warning: deleting outlets that are connected will cause pd to crash */
void FObject::noutlets_set (int n, bool draw) {
	if (!bself->te_binbuf) draw=false;
	if (n<0) RAISE("noutlets_set: n=%d must be at least 0",n);
	if (draw) BFObject_undrawio(bself);
	if (noutlets<n) {
		PtrOutlet*noo = new PtrOutlet[n>0?n:1];
		memcpy(noo,out,noutlets*sizeof(t_outlet*));
		delete[] out;
		out = noo;
		while (noutlets<n) out[noutlets++].p = outlet_new(bself,&s_anything);
	} else {
		while (noutlets>n) outlet_free(out[--noutlets]);
	}
	if (draw) BFObject_redraw(bself);
}

string BFObject::binbuf_string () {
	t_binbuf *b = te_binbuf;
	if (!b) return "[???]";
	ostringstream s;
	int n = binbuf_getnatom(b);
	t_atom2 *at = (t_atom2 *)binbuf_getvec(b);
	for (int i=0; i<n; i++) s << (i ? at[i].a_type==A_COMMA ? "" : " " : "[") << at[i];
	s << "]";
	return s.str();
}

void add_creator2(FClass *fclass, const char *name) {
	fclasses[gensym(name)] = fclass;
	class_addcreator((t_newmethod)BFObject_new,gensym((char *)name),A_GIMME,0);
}

void add_creator3(FClass *fclass, const char *name) {
	fclasses[gensym(name)] = fclass;
	t_class *c = pd_objectmaker;
	t_symbol *want = gensym(name);
	t_gotfn old;
	for (int i=c->c_nmethod-1; i>=0; i--) {
		t_methodentry *m = c->c_methods+i;
		if (m->me_name==want) {
			old = m->me_fun;
			m->me_fun = t_gotfn(BFObject_new);
			m->me_arg[0]=A_GIMME;
			m->me_arg[1]=A_NULL;
			// big hack : I can't assume A_GIMME here, but so far, this is only for [print], so it's ok
			class_addcreator(t_newmethod(old),symprintf("pd/%s",name),A_GIMME,0);
			return;
		}
	}
}

//****************************************************************

struct t_namelist;
extern t_namelist *sys_searchpath, *sys_helppath;
extern "C" t_namelist *namelist_append_files(t_namelist *, char *);
static void add_to_path(char *dir) {
	static bool debug = false;
	char bof[1024];
	if (debug) post("gridflow was found in %s",dir);
	gf_data_path.push_back(string(dir)+"/images");
	if (debug) post("adding gf_data_path %s/images",dir);
	sprintf(bof,"%s/abstractions",dir);        sys_searchpath = namelist_append_files(sys_searchpath,bof);
	if (debug) post("adding -path %s",bof);
	sprintf(bof,"%s/deprecated",dir);          sys_searchpath = namelist_append_files(sys_searchpath,bof);
	if (debug) post("adding -path %s",bof);
	sprintf(bof,"%s/doc/flow_classes",dir);    sys_helppath   = namelist_append_files(sys_helppath,  bof);
	if (debug) post("adding -helppath %s",bof);
}

//----------------------------------------------------------------

t_list *list_new (int argc, t_atom *argv) {
	t_list *b = binbuf_new();
	binbuf_add(b,argc,argv);
	return b;
}
void list_free (t_list *self) {binbuf_free(self);}

//----------------------------------------------------------------

void fclass_install(FClass *fclass, FClass *super) {
	fclass->super = super;
	if (fclass->startup) fclass->startup(fclass);
}

void install2(FClass *fc, const char *name, int ninlets, int noutlets, int flags) {
	fc->ninlets = ninlets;
	fc->noutlets = noutlets;
	fc->name = gensym(name);
	fc->flags = flags;
	// we _have_ to use A_GIMME if we want to check for too many args.
	t_class *bc = fc->bfclass = class_new(gensym((char *)name), (t_newmethod)BFObject_new, (t_method)BFObject_delete,
		sizeof(BFObject), flags, A_GIMME,0);
	fclasses[gensym(name)] = fclasses_pd[bc] = fc;
	class_addanything(bc,t_method(BFObject_anything0));
// blob has to be explicitly registered, because it has a default method, and isn't caught by 'anything'.
	//class_addblob(bc,t_method(BFObject_blob0));
// loadbang has to be explicitly registered, because it is called by zgetfn.
	FMethod m = method_lookup(fc,0,s_loadbang);
	if (m) class_addmethod(bc,t_method(BFObject_loadbang),s_loadbang,A_NULL);
}

//static inline const t_atom &convert(const t_atom &x, const t_atom *foo) {return x;}

/* This code handles nested lists because PureData doesn't do it */
int handle_parens(int ac, t_atom *av_) {
	t_atom2 *av = (t_atom2 *)av_;
	int stack[16];
	int stackn=0;
	int j=0;
	t_binbuf *buf = binbuf_new();
	for (int i=0; i<ac; ) {
		int close=0;
		if (av[i].a_type==A_SYMBOL) {
			const char *s = av[i].a_symbol->s_name, *os=s;
			while (*s=='(') {
				if (stackn==16) {binbuf_free(buf); RAISE("too many nested lists (>16)");}
				stack[stackn++]=j;
				s++;
			}
			const char *se = s+strlen(s), *ose=se;
			while (se>s && se[-1]==')') {se--; close++;}
			if (os==s && ose==se) {
				if (av[i].a_symbol==s_comma) av[i].a_type=A_COMMA; /* wtf */
				av[j++]=av[i];
			} else if (s!=se) {
				binbuf_text(buf,(char *)s,se-s);
				if ((binbuf_getnatom(buf)==1 && binbuf_getvec(buf)[0].a_type==A_FLOAT)
				 || binbuf_getvec(buf)[0].a_type==A_COMMA) {
					av[j++] = binbuf_getvec(buf)[0];
				} else {
					char ss[MAXPDSTRING];
					int n = min(long(se-s),long(MAXPDSTRING-1));
					sprintf(ss,"%.*s",n,s);
					av[j++]=gensym(ss);
				}
			}
		} else av[j++]=av[i];
		i++;
		while (close--) {
			if (!stackn) {binbuf_free(buf); RAISE("close-paren without open-paren");}
			t_binbuf *a2 = binbuf_new(); /* leak because there is no deallocation mechanism whatsoever */
			int j2 = stack[--stackn];
			binbuf_add(a2,j-j2,av+j2);
			j=j2;
			av[j]=a2;
			j++;
		}
	}
	binbuf_free(buf);
	if (stackn) RAISE("too many open-paren (%d)",stackn);
	return j;
}

// foreach macro from desiredata:
#define foreach(ITER,COLL) for(typeof(COLL.begin()) ITER = COLL.begin(); ITER != (COLL).end(); ITER++)

\class FObject
\def 0 get (t_symbol *s=0) {
	FClass *fc = fclasses_pd[pd_class(bself)];
	if (!s) {
		foreach(attr,fc->attrs) {
			t_atom2 a[1] = {gensym((char *)attr->second->name->s_name)};
			pd_typedmess((t_pd *)bself,gensym("get"),1,a);
		}
	} else {
		t_atom2 a[1] = {s};
		FMethod m = method_lookup(bself,-2,gensym("___get"));
		if (!m) RAISE("missing ___get");
		m(this,1,a);
	}
}
\end class {fclass->name = gensym("<FObject>");}

void blargh () {
  fprintf(stderr,"begin blargh()");
#if defined(MACOSX) || defined(__WIN32__)
  fprintf(stderr,"unhandled exception\n");
#else
  void *array[25];
  int nSize = backtrace(array,25);
  char **symbols = backtrace_symbols(array, nSize);
  for (int i=0; i<nSize; i++) fprintf(stderr,"%d: %s\n",i,symbols[i]);
  free(symbols);
#endif
  fprintf(stderr,"end blargh()");
}

/* for debugging... linux-only */
#if !defined(MACOSX) && !defined(__WIN32__)
char *short_backtrace (int start/*=3*/, int end/*=4*/) {
	static char buf[1024]; buf[0]=0;
	void *array[end];
	int nSize = backtrace(array,end);
	char **symbols = backtrace_symbols(array, nSize);
	char *demangled = (char *)malloc(1024); size_t length=1024; int status;
	for (int i=start,j=0; i<nSize; i++) {
		char *a = strchr(symbols[i],'(');
		char *b = strchr(symbols[i],'+');
		if (a&&b) {
			char mangled[1024]; sprintf(mangled,"%.*s",int(b-a-1),a+1);
			if (abi::__cxa_demangle(mangled,demangled,&length,&status))
				j+=sprintf(buf+j,"%s%.*s",i>start?", \n  ":"[",int(length),demangled);
			else
			j+=sprintf(buf+j,"%s%s",  i>start?", \n  ":"[",symbols[i]);
		}
		else    j+=sprintf(buf+j,"%s%s",  i>start?", \n  ":"[",symbols[i]);
	}
	free(demangled);
	sprintf(buf+strlen(buf),"]");
	return buf;
}
#endif

static t_gobj *canvas_last (t_canvas *self) {
	t_gobj *g = self->gl_list;
	while (g->g_next) g=g->g_next;
	return g;
}

static void canvas_else (t_canvas *self, t_symbol *s, int argc, t_atom2 *argv) {
	t_gobj *g = canvas_last(self); if (!g) {pd_error(self,"canvas else: there is no last object"); return;}
	if (pd_newest()) return;
	glist_delete(self,g);
	if (argc<1 || argv[0].a_type!=A_SYMBOL) {error("$1 must be a symbol"); return;}
	pd_typedmess((t_pd *)self,argv[0],argc-1,argv+1);
}

struct _inlet {t_pd i_pd; struct _inlet *i_next;};
extern int propertiesfn_offset;
static t_pd *text_firstinlet (t_object *self) {
	//int i = propertiesfn_offset+sizeof(long)+sizeof(int)+2;
	int n = obj_ninlets(self);
	for (t_inlet *in = self->ob_inlet; in; in = in->i_next) n--;
	//post("firstin=%d n=%d offsetof=%d",((char *)self)[i],n,offsetof(t_class,c_firstin));
	return n ? (t_pd *)self : (t_pd *)self->ob_inlet;
}
static void canvas_tolast (t_canvas *self, t_symbol *s, int argc, t_atom2 *argv) {
	t_gobj *g = canvas_last(self); if (!g) {pd_error(self,"canvas last: there is no last object"); return;}
	if (argc<1 || argv[0].a_type!=A_SYMBOL) {error("$1 must be a symbol"); return;}
	//post("tolast: class=%s",pd_class(g)->c_name->s_name);
	t_pd *r = text_firstinlet((t_text *)g);
	pd_typedmess(r,argv[0],argc-1,argv+1);
}
static void canvas_last_activate (t_canvas *self, t_symbol *s, int argc, t_atom *argv) {
	t_gobj *g = canvas_last(self); if (!g) {pd_error(self,"canvas last: there is no last object"); return;}
	glist_select(self,g);
        gobj_activate(g,self,1);
}

// those are not really leaks but deleting them make them disappear from valgrind
// however, there's still a problem doing it, so, we won't do it.
static void gridflow_unsetup () {
/*
	foreach(iter,fclasses_pd) {
		FClass *fc = iter->second;
		foreach(iter2,fc->attrs) delete iter2->second;
		fc->FClass::~FClass();
	}
*/
}

void allow_big_stack () {
#ifndef __WIN32__
  struct rlimit happy;
  if (0>getrlimit(RLIMIT_STACK,&happy))
    error("GF: getrlimit: %s",strerror(errno));
  happy.rlim_cur = happy.rlim_max;
  if (0>setrlimit(RLIMIT_STACK,&happy))
    error("GF: setting stack size to %ld: %s",happy.rlim_cur,strerror(errno));
  else
    post( "GF: setting stack size to %ld",happy.rlim_cur);
#endif
}

extern "C" void sys_load_lib(t_canvas *,const char *);

void startup_number();
void startup_classes1();
void startup_classes3();
void startup_classes2();
void startup_classes4();
void startup_classes_gui();
void startup_format();
STARTUP_LIST(void)

// note: contrary to what m_pd.h says, pd_getfilename() and pd_getdirname()
// don't exist; also, canvas_getcurrentdir() isn't available during setup
// (segfaults), in addition to libraries not being canvases ;-)
// AND ALSO, CONTRARY TO WHAT m_pd.h SAYS, open_via_path()'s args are reversed!!!
extern "C" void gridflow_setup () {
    s_loadbang = gensym("loadbang");
    post("GridFlow " GF_VERSION ", Copyright (c) 2001-2010 Mathieu Bouchard");
    post("GridFlow was compiled on "__DATE__", "__TIME__);
    //std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
    std::set_terminate(blargh);
    allow_big_stack();
    try {
	char *dirname   = new char[MAXPDSTRING];
	char *dirresult = new char[MAXPDSTRING];
	char *nameresult;
	char *zz=getcwd(dirname,MAXPDSTRING); /* zz only exists because gcc 4.3.3 gives me a bogus warning otherwise. */
	if (zz<0) {post("AAAARRRRGGGGHHHH!"); exit(69);}
	int       fd=open_via_path(dirname,"gridflow/gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd<0) fd=open_via_path(dirname,         "gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd>=0) close(fd); else post("%s was not found via the -path!","gridflow"PDSUF);
	/* nameresult is only a pointer in dirresult space so don't delete[] it. */
	gridflow_folder = gensym(dirresult);
	add_to_path(dirresult);

	BFProxy_class = class_new(gensym("gf.proxy"),0,0,sizeof(BFProxy),CLASS_PD|CLASS_NOINLET, A_NULL);
	class_addanything(BFProxy_class,BFProxy_anything);
	//void **fumble = (void **)BFProxy_class;
	//while (*fumble != (void *)BFProxy_anything) fumble++;
	//post("BFProxy_blob = %p",BFProxy_blob);        post("fumble[-2] = %p",fumble[-2]);
	//class_addblob(    BFProxy_class,BFProxy_blob); post("fumble[-2] = %p",fumble[-2]);
	//fumble[-2] = (void *)BFProxy_blob;
	#ifndef __WIN32__
        srandom(rdtsc());
    #endif
#define FOO(SYM,NAME) s_##SYM = gensym(NAME);
BUILTIN_SYMBOLS(FOO)
#undef FOO
	startup_number();
	\startall
	startup_classes1();
	startup_classes3();
	startup_classes2();
	startup_classes4();
	startup_classes_gui();
	startup_format();
	STARTUP_LIST()
	// avoid linking directly to those parts (cross-platform optional-linkage)
	sys_load_lib(0,"gridflow/gridflow_gem_loader");
	sys_load_lib(0,"gridflow/gridflow_pdp");     
	sys_load_lib(0,"gridflow/gridflow_unicorn");

	//sys_gui("bind . <Motion> {puts %W}\n");
        sys_vgui("set gfdir {%s}\n",dirresult);
	sys_gui("proc gf_menu_open {parent} {\n"
		"set z $::pd_opendir; set ::pd_opendir $::gfdir/examples;"
		"if {[catch {menu_open}]} {menu_open $parent};"
		"set ::pd_opendir $z}\n");
	sys_gui("proc gridflow_add_to_help {menu} {\n"
		  "$menu add separator\n"
		  "$menu add command -label {GridFlow About}      -command {pd pd open about.pd $::gfdir/doc \\;}\n"
		  "$menu add command -label {GridFlow Help Index} -command {pd pd open index.pd $::gfdir/doc \\;}\n"
		  "$menu add command -label {GridFlow Examples}   -command {gf_menu_open .}\n"
		"}\n"
		"proc gridflow_add_to_put {menu} {\n"
		  "set c [regsub .m.put $menu \"\"]\n"
 		  "$menu add separator\n"
		  "$menu add command -label {Display} -command [list pd $c put display \\;]\n" //  -accelerator [accel_munge Shift+Ctrl+p]
		  "$menu add command -label {GridSee} -command [list pd $c put \\#see  \\;]\n"
		"}\n"
		"catch {gridflow_add_to_help .mbar.help}\n"
		"catch {gridflow_add_to_help $::pd_menus::menubar.help; proc pd {args} {pdsend [join $args " "]}}\n"
		"catch {rename menu_addstd menu_addstd_old\n"
		  "proc menu_addstd {mbar} {menu_addstd_old $mbar; gridflow_add_to_help $mbar.help\n"
		    "gridflow_add_to_put $mbar.put"
		"}}\n"
		/*"catch {rename pdtk_canvas_ctrlkey pdtk_canvas_ctrlkey_old\n"
		  "proc pdtk_canvas_ctrlkey {name key shift} {"
		    "if {$shift && ($key==\"p\"||$key==\"P\"} {}"
		    "pdtk_canvas_ctrlkey_old $name $key $shift"
		"}\n"*/
		);
	delete[] dirresult;
	delete[] dirname;
    } catch (Barf &oozy) {oozy.error(0,-3,(t_symbol *)0);}
    atexit(gridflow_unsetup);
    void canvas_iemguis(t_canvas *x, t_symbol *s);
    extern t_class *canvas_class;
    class_addmethod(canvas_class,(t_method)canvas_else,         gensym("else")         ,A_GIMME,0);
    class_addmethod(canvas_class,(t_method)canvas_tolast,       gensym("last")         ,A_GIMME,0);
    class_addmethod(canvas_class,(t_method)canvas_last_activate,gensym("last_activate"),A_GIMME,0);
    class_addmethod(canvas_class,(t_method)canvas_iemguis,      gensym("put"),          A_SYMBOL,0);
}
