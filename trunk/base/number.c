/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002 by Mathieu Bouchard

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

// returns the highest bit set in a word, or 0 if none.
int high_bit(uint32 n) {
	int i=0;
	if (n&0xffff0000) { n>>=16; i+=16; }
	if (n&0x0000ff00) { n>>= 8; i+= 8; }
	if (n&0x000000f0) { n>>= 4; i+= 4; }
	if (n&0x0000000c) { n>>= 2; i+= 2; }
	if (n&0x00000002) { n>>= 1; i+= 1; }
	return i;
}

// returns the lowest bit set in a word, or 0 if none.
int low_bit(uint32 n) {
	return high_bit((~n+1)&n);
}

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

#define WRITE_LE { \
	int bytes = self->bytes; \
	while (bytes--) { *out++ = t; t >>= 8; }}

#define WRITE_BE { \
	int bytes = self->bytes; \
	while (bytes--) { out[bytes] = t; t >>= 8; } \
	out += self->bytes; }

/* those macros would be faster if the _increment_
   were done only once every loop. or maybe gcc does it, i dunno */
#define FOURTIMES(_x_) _x_ _x_ _x_ _x_

#define NTIMES(_x_) \
	while (n>3) { FOURTIMES(_x_) n-=4; } \
	while (n--) { _x_ }

static void default_pack(BitPacking *self, int n, Pt<Number> in, Pt<uint8> out) {
	register uint32 t;
	int i;
	int hb[4];
	uint32 mask[4];
	int sameorder = self->endian==2 || self->endian==::is_le();
	int size = self->size;

	for (i=0; i<self->size; i++) hb[i] = high_bit(self->mask[i]);
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
			while (n--) {CONVERT1; WRITE_LE; in+=3;}
		else if (size==4) {
			while (n--) {CONVERT3; WRITE_LE; in+=4;}
		} else
			while (n--) {CONVERT2; WRITE_LE; in+=size;}
	} else {
		if (size==3)	
			while (n--) {CONVERT1; WRITE_BE; in+=3;}
		else if (size==4)
			while (n--) {CONVERT3; WRITE_BE; in+=4;}
		else
			while (n--) {CONVERT2; WRITE_BE; in+=size;}
	}
}

#define LOOP_UNPACK(_reader_) \
	while (n--) { int bytes=0; uint32 temp=0; _reader_; \
		for (int i=0; i<self->size; i++) { \
			uint32 t=temp&self->mask[i]; *out++ = (t<<(7-hb[i]))|(t>>(hb[i]-7));} \
	}

//			*out++ = ((temp & self->mask[i]) << 7) >> hb[i];

static void default_unpack(BitPacking *self, int n, Pt</*const*/ uint8> in,
Pt<Number> out) {
	int hb[4];
	for (int i=0; i<self->size; i++) hb[i] = high_bit(self->mask[i]);

	if (is_le()) {
		/* smallest byte first */
		LOOP_UNPACK(while(self->bytes>bytes) { temp |= *in++ << (bytes++)*8; })
	} else {
		/* biggest byte first */
		LOOP_UNPACK(bytes=self->bytes;while(bytes--) { temp = (temp<<8) | *in++; })
	}
}

/* **************************************************************** */

static void pack2_565(BitPacking *self, int n, Pt<Number> in, Pt<uint8> out) {
	const int hb[3] = {15,10,4};
	const uint32 mask[3] = {0x0000f800,0x000007e0,0x0000001f};
	register uint32 t;
	NTIMES( t=CONVERT1; *((short *)out)=t; out+=2; in+=3; )
}

static void pack3_888(BitPacking *self, int n, Pt<Number> in, Pt<uint8> out) {
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}

uint32 bitpacking_masks[][4] = {
	{0x0000f800,0x000007e0,0x0000001f,0},
	{0x00ff0000,0x0000ff00,0x000000ff,0},
};

static BitPacking builtin_bitpackers[] = {
	BitPacking(2, 2, 3, bitpacking_masks[0], pack2_565, default_unpack),
	BitPacking(1, 3, 3, bitpacking_masks[1], pack3_888, default_unpack),
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
Packer packer, Unpacker unpacker) {
	this->endian = endian;
	this->bytes = bytes;
	this->size = size;
	for (int i=0; i<size; i++) this->mask[i] = mask[i];
	if (packer) {
		this->packer = packer;
		this->unpacker = unpacker;
		return;
	}
	this->packer = default_pack;
	this->unpacker = default_unpack;

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
			colour_name[i], mask[i], low_bit(mask[i]), high_bit(mask[i]));
}

bool BitPacking::is_le() {
	return endian==1 || (endian ^ ::is_le())==3;
}

void BitPacking::pack(int n, Pt<Number> in, Pt<uint8> out) {
	return packer(this,n,in,out);
}

void BitPacking::unpack(int n, Pt<uint8> in, Pt<Number> out) {
	return unpacker(this,n,in,out);
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

static Ruby String_swap32_f (Ruby rself) {
	int n = rb_str_len(rself)/4;
	swap32(n,Pt<uint32>((uint32 *)rb_str_ptr(rself),n));
	return rself;
}

static Ruby String_swap16_f (Ruby rself) {
	int n = rb_str_len(rself)/2;
	swap16(n,Pt<uint16>((uint16 *)rb_str_ptr(rself),n));
	return rself;
}

/* **************************************************************** */

METHOD3(BitPacking,initialize) {
	return Qnil;
}

METHOD3(BitPacking,pack2) {
	if (argc!=1 || TYPE(argv[0])!=T_STRING) RAISE("bad args");
	if (argc==2 && TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int n = rb_str_len(argv[0]) / sizeof(Number) / size;
	Pt<Number> in = Pt<Number>((Number *)rb_str_ptr(argv[0]),rb_str_len(argv[0]));
	int bytes2 = n*bytes;
	Ruby out = argc==2 ? rb_str_resize(argv[1],bytes2) : rb_str_new("",bytes2);
	rb_str_modify(out);
	pack(n,Pt<Number>(in,n),Pt<uint8>((uint8 *)rb_str_ptr(out),bytes2));
	return out;
}

METHOD3(BitPacking,unpack2) {
	if (argc<1 || argc>2 || TYPE(argv[0])!=T_STRING) RAISE("bad args");
	if (argc==2 && TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int n = rb_str_len(argv[0]) / bytes;
	Pt<uint8> in = Pt<uint8>((uint8 *)rb_str_ptr(argv[0]),rb_str_len(argv[0]));
	int bytes2 = n*size*sizeof(Number);
	Ruby out = argc==2 ? rb_str_resize(argv[1],bytes2) : rb_str_new("",bytes2);
	rb_str_modify(out);
	memset(rb_str_ptr(out),255,n*4*size);
	unpack(n,Pt<uint8>((uint8 *)in,bytes2),Pt<Number>((Number *)rb_str_ptr(out),n));
//	memcpy(rb_str_ptr(out),in,n);
	return out;
}

void BitPacking_free (void *foo) {}

static Ruby BitPacking_s_new(Ruby argc, Ruby *argv, Ruby qlass) {
	Ruby keep = rb_ivar_get(mGridFlow, rb_intern("@fobjects_set"));
	BitPacking *c_peer;

	if (argc!=3) RAISE("bad args");
	if (TYPE(argv[2])!=T_ARRAY) RAISE("bad mask");

	int endian = INT(argv[0]);
	int bytes = INT(argv[1]);
	Ruby *masks = rb_ary_ptr(argv[2]);
	uint32 masks2[4];
	int size = rb_ary_len(argv[2]);
	if (size<1) RAISE("not enough masks");
	if (size>4) RAISE("too many masks (%d)",size);
	for (int i=0; i<size; i++) masks2[i] = NUM2UINT(masks[i]);
	c_peer = new BitPacking(endian,bytes,size,masks2);
	
	Ruby rself = Data_Wrap_Struct(qlass, 0, BitPacking_free, c_peer);
	rb_hash_aset(keep,rself,Qtrue); /* prevent sweeping (leak) */
	rb_funcall2(rself,SI(initialize),argc,argv);
	return rself;
}

GRCLASS(BitPacking,"",inlets:0,outlets:0,startup:0,
LIST(),
	DECL(BitPacking,initialize),
	DECL(BitPacking,pack2),
	DECL(BitPacking,unpack2));

/* **************************************************************** */

#define MAX_INDICES 1024*1024
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

#define DECL_TYPE(_name_,_size_) \
	{ 0, #_name_, _size_ }

NumberType number_type_table[] = {
	DECL_TYPE(     uint8,  8),
	DECL_TYPE(      int8,  8),
	DECL_TYPE(    uint16, 16),
	DECL_TYPE(     int16, 16),
	DECL_TYPE(    uint32, 32),
	DECL_TYPE(     int32, 32),
	DECL_TYPE(   float32, 32),
	DECL_TYPE(   float64, 64),
/*
	DECL_TYPE(    uint64, 64),
	DECL_TYPE(     int64, 64),
	DECL_TYPE( complex64, 64),
	DECL_TYPE(complex128,128),
*/
};

NumberTypeIndex NumberType_find (Ruby sym) {
	if (TYPE(sym)!=T_SYMBOL) RAISE("expected symbol");
	if (sym==SYM(int32)) return int32_type_i;
	if (sym==SYM(uint8)) return uint8_type_i;
	RAISE("unknown element type \"%s\"", rb_sym_name(sym));
}

/* **************************************************************** */

#define DEF_OP1(_name_,_expr_) \
	\
	template <class T> \
	static T op1_##_name_ (T a) { return _expr_; } \
	\
	template <class T> \
	static void op1_array_##_name_ (int n, Pt<T> as) { \
		while ((n&3)!=0) { T a = *as; *as++ = _expr_; n--; } \
		while (n) { \
			{ T a=as[0]; as[0]= _expr_; } \
			{ T a=as[1]; as[1]= _expr_; } \
			{ T a=as[2]; as[2]= _expr_; } \
			{ T a=as[3]; as[3]= _expr_; } \
		as+=4; n-=4; } }

DEF_OP1(abs,  a>=0 ? a : -a)
DEF_OP1(sqrt, (T)(0+floor(sqrt(a))))
/*DEF_OP1(rand, (random()*(long long)a)/RAND_MAX)*/
DEF_OP1(rand, a==0 ? 0 : random()%a)
DEF_OP1(sq, a*a)

#define DECL_OP1(_name_,_sym_) \
	{ 0, _sym_, { &op1_##_name_, &op1_array_##_name_ } }

Operator1 op1_table[] = {
	DECL_OP1(abs, "abs"),
	DECL_OP1(sqrt,"sqrt"), 
	DECL_OP1(rand,"rand"),
	DECL_OP1(sq,"sq"),
};

/* **************************************************************** */

#define DEF_OP2(_name_,_expr_) \
	\
	template <class T> \
	static T op_##_name_ (T a, T b) { return _expr_; } \
	\
	template <class T> \
	static void op_array_##_name_ (int n, Pt<T> as, T b) { \
		while ((n&3)!=0) { T a = *as; *as++ = _expr_; n--; } \
		while (n) { \
			{ T a=as[0]; as[0]= _expr_; } \
			{ T a=as[1]; as[1]= _expr_; } \
			{ T a=as[2]; as[2]= _expr_; } \
			{ T a=as[3]; as[3]= _expr_; } \
		as+=4; n-=4; } } \
	\
	template <class T> \
	static void op_array2_##_name_ (int n, Pt<T> as, Pt<T> bs) { \
		while ((n&3)!=0) { T a = *as, b = *bs++; *as++ = _expr_; n--; } \
		while (n) { \
			{ T a=as[0], b=bs[0]; as[0]= _expr_; } \
			{ T a=as[1], b=bs[1]; as[1]= _expr_; } \
			{ T a=as[2], b=bs[2]; as[2]= _expr_; } \
			{ T a=as[3], b=bs[3]; as[3]= _expr_; } \
		as+=4; bs+=4; n-=4; } } \
	\
	template <class T> \
	static T op_fold_##_name_ (T a, int n, Pt<T> bs) { \
		while (n--) { T b = *bs++; a = _expr_; } return a; } \
	\
	template <class T> \
	static void op_fold2_##_name_ (int an, Pt<T> as, int n, Pt<T> bs) {\
		while (n--) { \
			int i=0; \
			while (i<an) { \
				{ T a = as[i], b = *bs++; as[i] = _expr_; } i++; } } } \
	\
	template <class T> \
	static void op_scan_##_name_ (T a, int n, Pt<T> bs) { \
		while (n--) { T b = *bs; *bs++ = a = _expr_; } } \
	\
	template <class T> \
	static void op_scan2_##_name_ (int an, Pt<T> as, int n, Pt<T> bs) { \
		while (n--) { \
			for (int i=0; i<an; i++) { \
				T a = *as++, b = *bs; *bs++ = a = _expr_; } \
			as=bs-an; } }

DEF_OP2(add, a+b)
DEF_OP2(sub, a-b)
DEF_OP2(bus, b-a)

DEF_OP2(mul, a*b)
DEF_OP2(div, b==0 ? 0 : a/b)
DEF_OP2(vid, a==0 ? 0 : b/a)
DEF_OP2(mod, b==0 ? 0 : mod(a,b))
DEF_OP2(dom, a==0 ? 0 : mod(b,a))
DEF_OP2(rem, b==0 ? 0 : a%b)
DEF_OP2(mer, a==0 ? 0 : b%a)

DEF_OP2(or , a|b)
DEF_OP2(xor, a^b)
DEF_OP2(and, a&b)
DEF_OP2(shl, a<<b)
DEF_OP2(shr, a>>b)

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

DEF_OP2(sin, (T)(b * sin(a * M_PI / 18000)))
DEF_OP2(cos, (T)(b * cos(a * M_PI / 18000)))
DEF_OP2(atan, (T)(atan2(a,b) * 18000 / M_PI))
DEF_OP2(tanh, (T)(b * tanh(a * 2 * M_PI / 36000)))
DEF_OP2(gamma, b<=0 ? 0 : (T)(0+floor(pow(a/256.0,256.0/b)*256.0)))
DEF_OP2(pow, ipow(a,b))
DEF_OP2(log, (T)(a==0 ? 0 : b * log(abs(a))))

#define DECL_OP2(_name_,_sym_,_props_) { \
	0, _sym_, \
	{ &op_##_name_, \
	&op_array_##_name_, \
	&op_array2_##_name_, \
	&op_fold_##_name_, \
	&op_fold2_##_name_, \
	&op_scan_##_name_, \
	&op_scan2_##_name_ } }

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
	DECL_OP2(add, "+", "N=0 ASSO COMM"), /* LINV=sub */
	DECL_OP2(sub, "-", "RN=0"),
	DECL_OP2(bus, "inv+", "LN=0"),

	DECL_OP2(mul, "*", "N=1 ASSO"),
	DECL_OP2(div, "/", "RN=1"),
	DECL_OP2(vid, "inv*", "LN=1"),
	DECL_OP2(mod, "%", ""),
	DECL_OP2(dom, "swap%", ""),
	DECL_OP2(rem, "rem", ""),
	DECL_OP2(mer, "swaprem", ""),

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

	Ruby cBitPacking =
		rb_define_class_under(mGridFlow, "BitPacking", rb_cObject);
	define_many_methods(cBitPacking,
		ciBitPacking.methodsn,
		ciBitPacking.methods);
	SDEF(BitPacking,new,-1);
	rb_define_method(rb_cString, "swap32!", (RFunc)String_swap32_f, 0);
	rb_define_method(rb_cString, "swap16!", (RFunc)String_swap16_f, 0);
}
