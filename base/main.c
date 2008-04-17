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

#include <stdlib.h>
//#include <cstdlib>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "../config.h"
#include <limits.h>

//using namespace std;

#include "grid.h.fcs"

BuiltinSymbols bsym;
Ruby mGridFlow;
Ruby cFObject;

Barf::Barf(const char *s, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap,s);
    vsnprintf(buf,1024,s,ap);
    buf[1023]=0;
    va_end(ap);
    text = strdup(buf);
}
Barf::Barf(const char *file, int line, const char *func, const char *s, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap,s);
    int n = vsnprintf(buf,1024,s,ap);
    if (n<1024) snprintf(buf+n, 1024-n, "\n%s:%d:in `%s'", file, line, func);
    buf[1023]=0;
    va_end(ap);
    text = strdup(buf);
}

void pd_oprint (std::ostream &o, int argc, t_atom *argv) {
	for (int i=0; i<argc; i++) {
		t_atomtype t = argv[i].a_type;
		if (t==A_FLOAT) o << argv[i].a_w.w_float;
		else if (t==A_SYMBOL) o << argv[i].a_w.w_symbol->s_name;
		else if (t==A_POINTER) o << "(pointer)";
 		else if (t==A_COMMA) o << ",";
 		else if (t==A_SEMI) o << ";";
		else if (t==A_LIST) {
			t_binbuf *b = (t_binbuf *)argv[i].a_w.w_gpointer;
			o << "[";
			pd_oprint(o,binbuf_getnatom(b),binbuf_getvec(b));
			o << "]";
		} else o << "(atom of type " << t << ")";
		if (i!=argc-1) o << " ";
	}
}

void pd_post (const char *s, int argc, t_atom *argv) {
	std::ostringstream os;
	if (s) os << s << ": ";
	pd_oprint(os,argc,argv);
	post("%s",os.str().data());
}

void pd_oprintf (std::ostream &o, const char *s, int argc, t_atom *argv) {
	int i=0;
	for (; *s; s++) {
		if (*s!='%') {o << (char)*s; continue;}
		s++; // skip the %
		if (*s=='f') {
			if (!argc) RAISE("not enough args");
			if (argv[i].a_type != A_FLOAT) RAISE("expected float");
			o << argv[i].a_w.w_float;
		}
		if (*s=='s') {
			if (!argc) RAISE("not enough args");
			if (argv[i].a_type != A_SYMBOL) RAISE("expected symbol");
			o << argv[i].a_w.w_symbol->s_name;
		}
		if (*s=='_') {
			if (!argc) RAISE("not enough args");
			char buf[MAXPDSTRING];
			atom_string(&argv[i],buf,MAXPDSTRING);
			o << buf;
		}
		if (*s=='%') {
			o << "%";
			continue;
		}
		RAISE("sorry, this format string is not supported yet");
	}
}

//----------------------------------------------------------------
// Dim

void Dim::check() {
	if (n>MAX_DIM) RAISE("too many dimensions");
	for (int i=0; i<n; i++) if (v[i]<0) RAISE("Dim: negative dimension");
}

// !@#$ big leak machine?
// returns a string like "Dim[240,320,3]"
char *Dim::to_s() {
	// if you blow 256 chars it's your own fault
	char buf[256];
	char *s = buf;
	s += sprintf(s,"Dim[");
	for(int i=0; i<n; i++) s += sprintf(s,"%s%d", ","+!i, v[i]);
	s += sprintf(s,"]");
	return strdup(buf);
}

/* ---------------------------------------------------------------- */
/* C++<->Ruby bridge for classes/functions in base/number.c */

NumberTypeE NumberTypeE_find (string s) {
	if (number_type_dict.find(s)==number_type_dict.end()) RAISE("unknown number type \"%s\"", s.data());
	return number_type_dict[s]->index;
}

NumberTypeE NumberTypeE_find (Ruby sym) {
	if (TYPE(sym)!=T_STRING) sym=rb_funcall(sym,SI(to_s),0);
	return NumberTypeE_find(string(rb_str_ptr(sym)));
}

NumberTypeE NumberTypeE_find (const t_atom &x) {
	if (x.a_type!=A_SYMBOL) RAISE("expected number-type (as symbol)");
	return NumberTypeE_find(string(x.a_w.w_symbol->s_name));
}

/* ---------------------------------------------------------------- */

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
	for (; n>4; in+=4, out+=4, n-=4) { *(int32*)out = *(int32*)in; }
	for (; n; in++, out++, n--) { *out = *in; }
}

extern "C" {
void *gfmalloc(size_t n) {
	void *p = memalign(16,n);
	long align = (long)p & 15;
	if (align) fprintf(stderr,"malloc alignment = %ld mod 16\n",align);
	return p;
}
void gffree(void *p) {free(p);}
};

uint64 gf_timeofday () {
	timeval t;
	gettimeofday(&t,0);
	return t.tv_sec*1000000+t.tv_usec;
}

#define CONVERT0(z) ((in[z] >> chop[z]) << slide[z])
#define CONVERT1 t = CONVERT0(0) | CONVERT0(1) | CONVERT0(2)
#define CONVERT2 for (t=0,i=0; i<self->size; i++) t |= CONVERT0(i);

#define WRITE_LE \
	for (int bytes = self->bytes; bytes; bytes--, t>>=8) *out++ = t;

#define WRITE_BE { int bytes; \
	bytes = self->bytes; \
	while (bytes--) {out[bytes] = t; t>>=8;}\
	out += self->bytes; }

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

template <class T>
static void default_pack(BitPacking *self, long n, T * in, uint8 * out) {
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
			*out = (t<<(7-hb[i]))|(t>>(hb[i]-7)); \
		} \
	}
//			*out++ = ((temp & self->mask[i]) << 7) >> hb[i];

template <class T>
static void default_unpack(BitPacking *self, long n, uint8 * in, T * out) {
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

template <class T>
static void pack2_565(BitPacking *self, long n, T * in, uint8 * out) {
//	const int hb[3] = {15,10,4};
//	const uint32 mask[3] = {0x0000f800,0x000007e0,0x0000001f};
//	uint32 span[3] = {4,5,4};
	uint32 chop[3] = {3,2,3};
	uint32 slide[3] = {11,5,0};
	uint32 t;
	NTIMES(CONVERT1; *((short *)out)=t; out+=2; in+=3;)
}

template <class T>
static void pack3_888(BitPacking *self, long n, T *in, uint8 *out) {
	int32 * o32 = (int32 *)out;
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

template <class T>
static void unpack3_888(BitPacking *self, long n, uint8 *in, T *out) {
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}

/*
template <>
static void pack3_888(BitPacking *self, long n, uint8 * in, uint8 * out) {
	uint32 * o32 = uint32 *((uint32 *)out.p,n*3/4);
	uint32 * i32 = uint32 *((uint32 *)in.p,n*3/4);
	while (n>=4) {
#define Z(w,i) ((word##w>>(i*8))&255)
		uint32 word0 = i32[0];
		uint32 word1 = i32[1];
		uint32 word2 = i32[2];
		o32[0] = (Z(1,1)<<24) | (Z(0,0)<<16) | (Z(0,1)<<8) | Z(0,2);
		o32[1] = (Z(1,3)<<24) | (Z(2,0)<<16) | (Z(0,3)<<8) | Z(1,0);
		o32[2] = (Z(2,1)<<24) | (Z(2,2)<<16) | (Z(2,3)<<8) | Z(1,2);
		o32+=3; i32+=3;
		n-=4;
	}
#undef Z
	out = (uint8 *)o32;
	in  = (uint8 *)i32;
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}
*/

template <class T>
static void pack3_888b(BitPacking *self, long n, T * in, uint8 * out) {
	int32 * o32 = (int32 *)out;
	while (n>=4) {
		o32[0] = (in[0]<<16) | (in[1]<<8) | in[2];
		o32[1] = (in[3]<<16) | (in[4]<<8) | in[5];
		o32[2] = (in[6]<<16) | (in[7]<<8) | in[8];
		o32[3] = (in[9]<<16) | (in[10]<<8) | in[11];
		o32+=4; in+=12;
		n-=4;
	}
	NTIMES( o32[0] = (in[0]<<16) | (in[1]<<8) | in[2]; o32++; in+=3; )
}

// (R,G,B,?) -> B:8,G:8,R:8,0:8
// fishy
template <class T>
static void pack3_bgrn8888(BitPacking *self, long n, T * in, uint8 * out) {
/* NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=4; in+=4; ) */
	int32 * i32 = (int32 *)in;
	int32 * o32 = (int32 *)out;
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
};

static Packer bp_packers[] = {
	{default_pack, default_pack, default_pack},
	{pack2_565, pack2_565, pack2_565},
	{pack3_888, pack3_888, pack3_888},
	{pack3_888b, default_pack, default_pack},
	{pack3_bgrn8888, default_pack, default_pack},
};

static Unpacker bp_unpackers[] = {
	{default_unpack, default_unpack, default_unpack},
	{unpack3_888,  unpack3_888,  unpack3_888},
};	

static BitPacking builtin_bitpackers[] = {
	BitPacking(2, 2, 3, bp_masks[0], &bp_packers[1], &bp_unpackers[0]),
	BitPacking(1, 3, 3, bp_masks[1], &bp_packers[2], &bp_unpackers[1]),
	BitPacking(1, 4, 3, bp_masks[1], &bp_packers[3], &bp_unpackers[0]),
	BitPacking(1, 4, 4, bp_masks[1], &bp_packers[4], &bp_unpackers[0]),
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

BitPacking::BitPacking(int endian, int bytes, int size, uint32 *mask,
Packer *packer, Unpacker *unpacker) {
	this->endian = endian;
	this->bytes = bytes;
	this->size = size;
	for (int i=0; i<size; i++) this->mask[i] = mask[i];
	if (packer) {
		this->packer = packer;
		this->unpacker = unpacker;
		return;
	}
	int packeri=-1;
	this->packer = &bp_packers[0];
	this->unpacker = &bp_unpackers[0];

	for (int i=0; i<(int)(sizeof(builtin_bitpackers)/sizeof(BitPacking)); i++) {
		BitPacking *bp = &builtin_bitpackers[i];
		if (this->eq(bp)) {
			this->packer = bp->packer;
			this->unpacker = bp->unpacker;
			packeri=i;
			goto end;
		}
	}
end:;
#if 0
	::post("Bitpacking: endian=%d bytes=%d size=%d packeri=%d",
		endian, bytes, size, packeri);
	::post("  packer=0x%08x unpacker=0x%08x",this->packer,this->unpacker);
	::post("  mask=[0x%08x,0x%08x,0x%08x,0x%08x]",mask[0],mask[1],mask[2],mask[3]);
#endif
}

bool BitPacking::is_le() {
	return endian==1 || (endian ^ ::is_le())==3;
}

template <class T>
void BitPacking::pack(long n, T * in, uint8 * out) {
	switch (NumberTypeE_type_of(in)) {
	case uint8_e: packer->as_uint8(this,n,(uint8 *)in,out); break;
	case int16_e: packer->as_int16(this,n,(int16 *)in,out); break;
	case int32_e: packer->as_int32(this,n,(int32 *)in,out); break;
	default: RAISE("argh");
	}
}

template <class T>
void BitPacking::unpack(long n, uint8 * in, T * out) {
	switch (NumberTypeE_type_of(out)) {
	case uint8_e: unpacker->as_uint8(this,n,in,(uint8 *)out); break;
	case int16_e: unpacker->as_int16(this,n,in,(int16 *)out); break;
	case int32_e: unpacker->as_int32(this,n,in,(int32 *)out); break;
	default: RAISE("argh");
	}
}

// i'm sorry... see the end of grid.c for an explanation...
//static
void make_hocus_pocus () {
//	exit(1);
#define FOO(S) \
	((BitPacking*)0)->pack(0,(S *)0,(uint8 *)0); \
	((BitPacking*)0)->unpack(0,(uint8 *)0,(S *)0);
EACH_NUMBER_TYPE(FOO)
#undef FOO
}

std::vector<string> gf_data_path;
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
