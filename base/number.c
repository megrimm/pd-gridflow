/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

#include "grid.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define CONVERT1 t = \
	(((in[0] << hb[0]) >> 7) & mask[0]) | \
	(((in[1] << hb[1]) >> 7) & mask[1]) | \
	(((in[2] << hb[2]) >> 7) & mask[2])

#define CONVERT2 \
	for (t=0,i=0; i<self->size; i++) t |= (((in[i] << hb[i]) >> 7) & mask[i]);

#define CONVERT3 \
	for (t=0,i=0; i<self->size; i++) { \
		t |= ((in[i]>>(7-hb[i]))|(in[i]<<(hb[i]-7))) & mask[i]; \
	}

#define WRITE_LE \
	for (int bytes = self->bytes; bytes; bytes--, t>>=8) *out++ = t;

#define WRITE_BE { int bytes; \
	for (bytes = self->bytes; bytes; bytes--, t>>=8) out[bytes] = t; \
	out += self->bytes; }

/* this macro would be faster if the _increment_
   was done only once every loop. or maybe gcc does it, i dunno */
#define NTIMES(_x_) \
	for (; n>3; n-=4) { _x_ _x_ _x_ _x_ } \
	for (; n; n--) { _x_ }

template <class T>
static void default_pack(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	uint32 t;
	int i;
	int hb[4];
	uint32 mask[4];
	int sameorder = self->endian==2 || self->endian==::is_le();
	int size = self->size;

	for (i=0; i<self->size; i++) hb[i] = highest_bit(self->mask[i]);
	memcpy(mask,self->mask,size*sizeof(uint32));

	if (sameorder && size==3) {
		switch(self->bytes) {
		case 2:
			NTIMES(t=CONVERT1; *((int16 *)out)=t; out+=2; in+=3;)
			return;
		case 4:
			NTIMES(t=CONVERT1; *((int32 *)out)=t; out+=4; in+=3;)
			return;
		}
	}
	
	if (self->is_le()) {
		if (size==3)
			for (; n--; in+=3) {CONVERT1; WRITE_LE;}
		else if (size==4) {
			for (; n--; in+=4) {CONVERT3; WRITE_LE;}
		} else
			for (; n--; in+=size) {CONVERT2; WRITE_LE;}
	} else {
		if (size==3)	
			for (; n--; in+=3) {CONVERT1; WRITE_BE;}
		else if (size==4)
			for (; n--; in+=4) {CONVERT3; WRITE_BE;}
		else
			for (; n--; in+=size) {CONVERT2; WRITE_BE;}
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
static void default_unpack(BitPacking *self, int n, Pt<uint8> in, Pt<T> out) {
	int hb[4];
	for (int i=0; i<self->size; i++) hb[i] = highest_bit(self->mask[i]);

	if (is_le()) {
		/* smallest byte first */
		LOOP_UNPACK(
			for(; self->bytes>bytes; bytes++, in++) temp |= *in<<(8*bytes);
		)
	} else {
		/* biggest byte first */
		LOOP_UNPACK(
			bytes=self->bytes; for (; bytes; bytes--, in++) temp=(temp<<8)|*in;
		)
	}
}

/* **************************************************************** */

template <class T>
static void pack2_565(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	const int hb[3] = {15,10,4};
	const uint32 mask[3] = {0x0000f800,0x000007e0,0x0000001f};
	uint32 t;
	NTIMES( t=CONVERT1; *((short *)out)=t; out+=2; in+=3; )
}

template <class T>
static void pack3_888(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}

static uint32 bp_masks[][4] = {
	{0x0000f800,0x000007e0,0x0000001f,0},
	{0x00ff0000,0x0000ff00,0x000000ff,0},
};

static Packer bp_packers[] = {
	{default_pack, default_pack, default_pack},
	{pack2_565, pack2_565, pack2_565},
	{pack3_888, pack3_888, pack3_888},
};

static Unpacker bp_unpackers[] = {
	{default_unpack, default_unpack, default_unpack},
};	

static BitPacking builtin_bitpackers[] = {
	BitPacking(2, 2, 3, bp_masks[0], &bp_packers[1], &bp_unpackers[0]),
	BitPacking(1, 3, 3, bp_masks[1], &bp_packers[2], &bp_unpackers[0]),
};

/* **************************************************************** */

bool BitPacking::eq(BitPacking *o) {
	if (!(bytes == o->bytes)) return false;
	if (!(size == o->size)) return false;
	for (int i=0; i<size; i++) {
		if (!(mask[0] == o->mask[0])) return false;
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
	this->packer = &bp_packers[0];
	this->unpacker = &bp_unpackers[0];

	for (int i=0; i<(int)(sizeof(builtin_bitpackers)/sizeof(BitPacking)); i++) {
		BitPacking *bp = builtin_bitpackers+i;
		if (this->eq(bp)) {
			this->packer = bp->packer;
			this->unpacker = bp->unpacker;
			//::gfpost("Bitpacking: will be using special packer (#%d)",i);
		}
	}
}

void BitPacking::gfpost() {
	static const char *colour_name[] = {"red","green","blue","alpha"};
	::gfpost("BitPacking:");
	::gfpost("    bytes: %d", bytes);
	for (int i=0;i<size;i++)
		::gfpost("    mask[%5s]: %08x (bits from %2d up to %2d)",
			colour_name[i], mask[i], lowest_bit(mask[i]), highest_bit(mask[i]));
}

bool BitPacking::is_le() {
	return endian==1 || (endian ^ ::is_le())==3;
}

template <class T>
void BitPacking::pack(int n, Pt<T> in, Pt<uint8> out) {
	switch (NumberTypeE_type_of(*in)) {
	case uint8_type_i: packer->as_uint8(this,n,(Pt<uint8>)in,out); break;
	case int16_type_i: packer->as_int16(this,n,(Pt<int16>)in,out); break;
	case int32_type_i: packer->as_int32(this,n,(Pt<int32>)in,out); break;
	default: RAISE("argh");
	}
}

template <class T>
void BitPacking::unpack(int n, Pt<uint8> in, Pt<T> out) {
	switch (NumberTypeE_type_of(*out)) {
	case uint8_type_i: unpacker->as_uint8(this,n,in,(Pt<uint8>)out); break;
	case int16_type_i: unpacker->as_int16(this,n,in,(Pt<int16>)out); break;
	case int32_type_i: unpacker->as_int32(this,n,in,(Pt<int32>)out); break;
	default: RAISE("argh");
	}
}

/* this could be faster (use asm) */
void swap32 (int n, Pt<uint32> data) {
	NTIMES({
		uint32 x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	})
}

/* this could be faster (use asm or do it in int32 chunks) */
void swap16 (int n, Pt<uint16> data) {
	NTIMES({ uint16 x = *data; *data++ = (x<<8) | (x>>8); })
}

/* **************************************************************** */

#define MAX_INDICES 16*1024*1024
void Dim::check() {
	if (n>MAX_DIMENSIONS) RAISE("too many dimensions");
	for (int i=0; i<n; i++)
		if (v[i]<0 || v[i]>MAX_INDICES)
			RAISE("dim[%d]=%d is out of range",i,v[i]);
}


/* returns a string like "Dim[240,320,3]" */
char *Dim::to_s() {
	/* if you blow 256 chars it's your own fault */
	char buf[256];
	char *s = buf;
	s += sprintf(s,"Dim[");
	for(int i=0; i<n; i++) s += sprintf(s,"%s%ld", ","+!i, v[i]);
	s += sprintf(s,"]");
	return strdup(buf);
}

/* **************************************************************** */

NumberType number_type_table[] = {
#define FOO(_sym_,_size_,_flags_,args...) { 0, #_sym_, _size_, _flags_, args },
NUMBER_TYPES(FOO)
#undef FOO
};

/* **************************************************************** */

template <class T>
class Op1 {public: static T f(T); };

template <class O>
class Op1Loops {
public:
	template <class T>
	static void op_map (int n, T *as) {
		for (; (n&3)!=0; n--, as++) *as=O::f(*as);
		for (; n; as+=4, n-=4) {
			as[0]=O::f(as[0]);
			as[1]=O::f(as[1]);
			as[2]=O::f(as[2]);
			as[3]=O::f(as[3]);
		} 
	}
};

#define DEF_OP1(op,expr) \
	template <class T> class Y1##op : Op1<T> { public: \
		inline static T f(T a) { return expr; } };

#define DECL_OP1ON(base,op,type) { \
	&base<Y1##op<type> >::op_map }

#define DECL_OP1(op,sym,props) { 0, sym, \
	DECL_OP1ON(Op1Loops,op,uint8), \
	DECL_OP1ON(Op1Loops,op,int16), \
	DECL_OP1ON(Op1Loops,op,int32), \
	DECL_OP1ON(Op1Loops,op,int64), \
	DECL_OP1ON(Op1Loops,op,float32), \
	DECL_OP1ON(Op1Loops,op,float64), \
}

#define DECL_OP1_NOU(op,sym,props) { 0, sym, \
	{0}, \
	DECL_OP1ON(Op1Loops,op,int16), \
	DECL_OP1ON(Op1Loops,op,int32), \
	DECL_OP1ON(Op1Loops,op,int64), \
	DECL_OP1ON(Op1Loops,op,float32), \
	DECL_OP1ON(Op1Loops,op,float64), \
}

DEF_OP1(abs,  a>=0 ? a : -a)
DEF_OP1(sqrt, (T)(0+floor(sqrt(a))))
/*DEF_OP1(rand, (random()*(long long)a)/RAND_MAX)*/
DEF_OP1(rand, a==0 ? 0 : random()%(int32)a)
DEF_OP1(sq, a*a)

Operator1 op1_table[] = {
	DECL_OP1_NOU(abs, "abs",""),
	DECL_OP1(sqrt,"sqrt",""), 
	DECL_OP1(rand,"rand",""),
	DECL_OP1(sq,"sq",""),
};

/* **************************************************************** */
/*
  those are bogus classes. this design is to make C++ happy with
  templating, because it doesn't do template-by-function nor
  template-of-typedef. I think the STL has that kind of pattern in it,
  but I don't use the STL so I couldn't tell exactly.
*/

template <class T>
class Op2 {public: static T f(T,T); };

template <class O>
class Op2Loops {
public:
	template <class T>
	static void op_map (int n, T *as, T b) {
		for (; (n&3)!=0; as++, n--) *as=O::f(*as,b);
		for (; n; as+=4, n-=4) {
			as[0]=O::f(as[0],b);
			as[1]=O::f(as[1],b);
			as[2]=O::f(as[2],b);
			as[3]=O::f(as[3],b);
		}
	}
	template <class T>
	static void op_zip (int n, T *as, T *bs) {
		for (; (n&3)!=0; as++,bs++,n-- ) *as=O::f(*as,*bs);
		for (; n; as+=4, bs+=4, n-=4) {
			as[0]= O::f(as[0],bs[0]);
			as[1]= O::f(as[1],bs[1]);
			as[2]= O::f(as[2],bs[2]);
			as[3]= O::f(as[3],bs[3]);
		}
	}
	template <class T>
	static void op_fold (int an, int n, T *as, T *bs) {
		if (an==1) {
			for (; (n&3)!=0; bs++, n--) *as=O::f(*as,*bs);
			for (; n; bs+=4, n-=4)
				*as=O::f(O::f(O::f(O::f(*as,bs[0]),bs[1]),bs[2]),bs[3]);
		} else {
			for (; n--; ) {
				int i=0;
				for (; i<(an&-4); i+=4, bs+=4) {
					as[i+0]=O::f(as[i+0],bs[0]);
					as[i+1]=O::f(as[i+1],bs[1]);
					as[i+2]=O::f(as[i+2],bs[2]);
					as[i+3]=O::f(as[i+3],bs[3]);
				}
				for (; i<an; i++, bs++) as[i] = O::f(as[i],*bs);
			}
		}
	}
	template <class T>
	static void op_scan (int an, int n, T *as, T *bs) {
		for (; n--; as=bs-an) {
			for (int i=0; i<an; i++, as++, bs++) *bs=O::f(*as,*bs);
		}
	}
};

template <class O>
class Op2LoopsBitwise : Op2Loops<O> {
public:
/*
	template <class T>
	static void op_map (int n, T *as, T b) {
		for (; (n&3)!=0; as++, n--) *as=O::f(*as,b);
		for (; n; as+=4, n-=4) {
			as[0]=O::f(as[0],b);
			as[1]=O::f(as[1],b);
			as[2]=O::f(as[2],b);
			as[3]=O::f(as[3],b);
		}
	}
*/
};

#define DEF_OP2(op,expr) \
	template <class T> class Y##op : Op2<T> { public: \
		inline static T f(T a, T b) { return expr; } };

#define DEF_OP2F(op,expr,expr2) \
	template <class T> class Y##op : Op2<T> { public: \
		inline static T f(T a, T b) { return expr; } }; \
	class Y##op<float32> : Op2<float32> { public: \
		inline static float32 f(float32 a, float32 b) { return expr2; } }; \
	class Y##op<float64> : Op2<float64> { public: \
		inline static float64 f(float64 a, float64 b) { return expr2; } };

#define DECL_OP2ON(base,op,type) { \
	&base<Y##op<type> >::op_map, \
	&base<Y##op<type> >::op_zip, \
	&base<Y##op<type> >::op_fold, \
	&base<Y##op<type> >::op_scan }

#define DECL_OP2(op,sym,props) { 0, sym, \
	DECL_OP2ON(Op2Loops,op,uint8), \
	DECL_OP2ON(Op2Loops,op,int16), \
	DECL_OP2ON(Op2Loops,op,int32), \
	DECL_OP2ON(Op2Loops,op,int64), \
	DECL_OP2ON(Op2Loops,op,float32), \
	DECL_OP2ON(Op2Loops,op,float64), \
}

/*
#define DECL_OP2_BITWISE(op,sym,props) { 0, sym, \
	DECL_OP2ON(Op2LoopsBitwise,op,uint8), \
	DECL_OP2ON(Op2LoopsBitwise,op,int16), \
	DECL_OP2ON(Op2LoopsBitwise,op,int32), \
	DECL_OP2ON(Op2LoopsBitwise,op,int64), \
	DECL_OP2ON(Op2LoopsBitwise,op,float32), \
	DECL_OP2ON(Op2LoopsBitwise,op,float64), \
}
*/

#define DECL_OP2_NOFLOAT(op,sym,props) { 0, sym, \
	DECL_OP2ON(Op2Loops,op,uint8), \
	DECL_OP2ON(Op2Loops,op,int16), \
	DECL_OP2ON(Op2Loops,op,int32), \
	DECL_OP2ON(Op2Loops,op,int64), \
	{0,0,0,0}, \
	{0,0,0,0}, \
}

template <class T>
static inline T gf_floor (T a) {
	return (T) floor((double)a); }

template <class T>
static inline T gf_trunc (T a) {
	return (T) floor(abs((double)a)) * (a<0?-1:1); }

DEF_OP2(ignore, a)
DEF_OP2(put, b)

DEF_OP2(add, a+b)
DEF_OP2(sub, a-b)
DEF_OP2(bus, b-a)

DEF_OP2(mul, a*b)
DEF_OP2(mulshr8, (a*b)>>8)
DEF_OP2(div, b==0 ? 0 : a/b)
DEF_OP2(div2, b==0 ? 0 : div2(a,b))
DEF_OP2(vid, a==0 ? 0 : b/a)
DEF_OP2(vid2, a==0 ? 0 : div2(b,a))
DEF_OP2F(mod, b==0 ? 0 : mod(a,b), b==0 ? 0 : a-b*gf_floor(a/b))
DEF_OP2F(dom, a==0 ? 0 : mod(b,a), a==0 ? 0 : b-a*gf_floor(b/a))

/*
DEF_OP2F(rem, b==0 ? 0 : a%b, b==0 ? 0 : a-b*gf_trunc(a/b))
DEF_OP2F(mer, a==0 ? 0 : b%a, a==0 ? 0 : b-a*gf_trunc(b/a))
*/
DEF_OP2(rem, b==0 ? 0 : a%b)
DEF_OP2(mer, a==0 ? 0 : b%a)

DEF_OP2(gcd, gcd(a,b))
DEF_OP2(lcm, a==0 || b==0 ? 0 : lcm(a,b))

DEF_OP2F(or , a|b, (float32)((int32)a | (int32)b))
DEF_OP2F(xor, a^b, (float32)((int32)a ^ (int32)b))
DEF_OP2F(and, a&b, (float32)((int32)a & (int32)b))
DEF_OP2F(shl, a<<b, a*pow(2.0,+b))
DEF_OP2F(shr, a>>b, a*pow(2.0,-b))

DEF_OP2(sc_and, a ? b : a)
DEF_OP2(sc_or,  a ? a : b)

DEF_OP2(min, min(a,b))
DEF_OP2(max, max(a,b))

DEF_OP2(cmp, cmp(a,b))
DEF_OP2(eq,  a == b)
DEF_OP2(ne,  a != b)
DEF_OP2(gt,  a >  b)
DEF_OP2(le,  a <= b)
DEF_OP2(lt,  a <  b)
DEF_OP2(ge,  a >= b)

DEF_OP2(sin, (T)(b * sin(a * (M_PI / 18000))))
DEF_OP2(cos, (T)(b * cos(a * (M_PI / 18000))))
DEF_OP2(atan, (T)(atan2(a,b) * (18000 / M_PI)))
DEF_OP2(tanh, (T)(b * tanh(a * (M_PI / 18000))))
DEF_OP2(gamma, b<=0 ? 0 : (T)(0+floor(pow(a/256.0,256.0/b)*256.0)))
DEF_OP2(pow, ipow(a,b))
DEF_OP2(log, (T)(a==0 ? 0 : b * log(abs(a))))

/*
Algebraic Properties (not used yet)

  RN: right neutral: { e in G | f(x,RN)=x }
  LN: left neutral: { e in G | f(x,RN)=x } (left neutral)
  N: both sides neutral: N=LN=RN

  LINV: left inverse: each a has a b like f(a,b) = the left neutral
  RINV: right inverse: each b has a a like f(a,b) = the right neutral
  INV: both sides inverse

  ASSO: associative: f(a,f(b,c))=f(f(a,b),c)
  COMM: commutative: f(a,b)=f(b,a)

*/
Operator2 op2_table[] = {
	DECL_OP2(ignore, "ignore", "ASSO RN=all"),
	DECL_OP2(put, "put", "ASSO LN=all"),

	DECL_OP2(add, "+", "N=0 ASSO COMM"), /* LINV=sub */
	DECL_OP2(sub, "-", "RN=0"),
	DECL_OP2(bus, "inv+", "LN=0"),

	DECL_OP2(mul, "*", "N=1 ASSO"),
	DECL_OP2_NOFLOAT(mulshr8, "*>>8", "N=256 ASSO"),
	DECL_OP2(div, "/", "RN=1"),
	DECL_OP2_NOFLOAT(div2, "div", "RN=1"),
	DECL_OP2(vid, "inv*", "LN=1"),
	DECL_OP2_NOFLOAT(vid2, "swapdiv", "LN=1"),
	DECL_OP2_NOFLOAT(mod, "%", ""),
	DECL_OP2_NOFLOAT(dom, "swap%", ""),
	DECL_OP2_NOFLOAT(rem, "rem", ""),
	DECL_OP2_NOFLOAT(mer, "swaprem", ""),
	DECL_OP2_NOFLOAT(gcd, "gcd", "ASSO COMM"),
	DECL_OP2_NOFLOAT(lcm, "lcm", "ASSO COMM"),

	DECL_OP2(or , "|", "N=0 ASSO COMM"),
	DECL_OP2(xor, "^", "N=0 ASSO COMM"),
	DECL_OP2(and, "&", "N=-1 ASSO COMM"),
	DECL_OP2(shl, "<<", "RN=0"),
	DECL_OP2(shr, ">>", "RN=0"),

	DECL_OP2(sc_and,"&&", ""),
	DECL_OP2(sc_or, "||", ""), /* N!=0 */

	DECL_OP2(min, "min", "ASSO COMM"), /* N = greatest possible number */
	DECL_OP2(max, "max", "ASSO COMM"), /* N = smallest possible number */

	DECL_OP2(eq,  "==", ""),
	DECL_OP2(ne,  "!=", ""),
	DECL_OP2(gt,  ">", ""),
	DECL_OP2(le,  "<=", ""),
	DECL_OP2(lt,  "<", ""),
	DECL_OP2(ge,  ">=", ""),
	DECL_OP2(cmp, "cmp", ""),

	DECL_OP2(sin, "sin*", ""),
	DECL_OP2(cos, "cos*", ""),
	DECL_OP2(atan, "atan", ""),
	DECL_OP2(tanh, "tanh*", ""),
	DECL_OP2(gamma, "gamma", "RN=256"),
	DECL_OP2(pow, "**", "RN=1"),
	DECL_OP2(log, "log*", ""),
};

template <class T>
static void quick_mod_map (int n, T *as, T b) {
	if (!b) return;
	for (; (n&3)!=0; n--, as++) *as=mod(*as,b);
	for (; n; as+=4, n+=4) {
		as[0]=mod(as[0],b);
		as[1]=mod(as[1],b);
		as[2]=mod(as[2],b);
		as[3]=mod(as[3],b);
	}
}

template <class T>
static void quick_ign_map (int n, T *as, T b) {
}

template <class T>
static void quick_ign_zip (int n, T *as, T *bs) {
}

template <class T>
static void quick_put_zip (int n, T *as, T *bs) {
	gfmemcopy(as,bs,n);
}

Ruby op1_dict = Qnil;
Ruby op2_dict = Qnil;
Ruby number_type_dict = Qnil;

#define INIT_TABLE(_dict_,_table_) { \
	rb_ivar_set(mGridFlow,rb_intern("@" #_dict_),_dict_=rb_hash_new()); \
	for(int i=0; i<COUNT(_table_); i++) { \
		_table_[i].sym = ID2SYM(rb_intern(_table_[i].name)); \
		rb_hash_aset(_dict_,_table_[i].sym,PTR2FIX((_table_+i))); \
	}}

void startup_number () {
	INIT_TABLE(op1_dict,op1_table)
	INIT_TABLE(op2_dict,op2_table)
	INIT_TABLE(number_type_dict,number_type_table)

	for (int i=0; i<COUNT(number_type_table); i++) {
		number_type_table[i].index = (NumberTypeE) i;
		char a[64];
		strcpy(a,number_type_table[i].aliases);
		char *b = strchr(a,',');
		if (b) {
			*b=0;
			rb_hash_aset(number_type_dict, ID2SYM(rb_intern(b+1)),
				PTR2FIX(&number_type_table[i]));
		}
		rb_hash_aset(number_type_dict, ID2SYM(rb_intern(a)),
				PTR2FIX(&number_type_table[i]));
	}

#define OVERRIDE_INT(_name_,_mode_,_func_) { \
	Operator2 *foo = FIX2PTR(Operator2,rb_hash_aref(op2_dict,SYM(_name_))); \
	foo->on_uint8.op_##_mode_ = _func_; \
	foo->on_int16.op_##_mode_ = _func_; \
	foo->on_int32.op_##_mode_ = _func_; }

/*
	OVERRIDE_INT(ignore,map,quick_ign_map);
	OVERRIDE_INT(ignore,zip,quick_ign_zip);
	OVERRIDE_INT(put,map,quick_put_zip);
*/

	/* !@#$ does that make an improvement at all? (suspect) */
//	OVERRIDE_INT(mod,map,quick_mod_map);
}

/* i'm sorry... see the end of grid.c for an explanation... */
//static
void make_hocus_pocus () {
//	exit(1);
#define FOO(S) \
	((BitPacking*)0)->pack(0,Pt<S>(),Pt<uint8>()); \
	((BitPacking*)0)->unpack(0,Pt<uint8>(),Pt<S>());
EACH_NUMBER_TYPE(FOO)
#undef FOO
}
