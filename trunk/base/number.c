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
	for (t=0,i=0; i<$->size; i++) t |= (((in[i] << hb[i]) >> 7) & mask[i]);

#define WRITE_LE while (bytes--) { *out++ = t; t >>= 8; }

#define WRITE_BE while (bytes--) { out[bytes] = t; t >>= 8; } out += $->bytes;

/* those macros would be faster if the _increment_
   were done only once every loop. or maybe gcc does it, i dunno */
#define FOURTIMES(_x_) _x_ _x_ _x_ _x_

#define NTIMES(_x_) \
	while (n>3) { FOURTIMES(_x_) n-=4; } \
	while (n--) { _x_ }

static uint8 *default_pack(BitPacking *$, int n, const Number *in, uint8 *out) {
	register uint32 t;
	int i;
	int hb[4];
	int mask[4];
	int sameorder = $->endian==2 || $->endian==::is_le();

	for (i=0; i<$->size; i++) hb[i] = high_bit($->mask[i]);
	memcpy(mask,$->mask,3*sizeof(int));

	if (sameorder && $->size==3) {
		switch($->bytes) {
		case 2:
			NTIMES(t=CONVERT1; *((short *)out)=t; out+=2; in+=3;)
			break;
		case 3:
			NTIMES(t=CONVERT1; *((short *)out)=t; out[2]=t>>16; out+=3; in+=3;)
			break;
		case 4:
			NTIMES(t=CONVERT1; *((long *)out)=t; out+=4; in+=3;)
			break;
		}
	} else if ($->is_le()) {
		if ($->size==3)
			while (n--) {int bytes = $->bytes; CONVERT1; WRITE_LE; in+=3;}
		else
			while (n--) {int bytes = $->bytes; CONVERT2; WRITE_LE; in+=$->size;}
	} else {
		if ($->size==3)	
			while (n--) {int bytes = $->bytes; CONVERT1; WRITE_BE; in+=3;}
		else
			while (n--) {int bytes = $->bytes; CONVERT2; WRITE_BE; in+=$->size;}
	}
	return out;
}

/* **************************************************************** */

static uint8 *pack_5652(BitPacking *$, int n, const Number *in, uint8 *out) {
	const int hb[3] = {15,10,4};
	const int mask[3] = {0x0000f800,0x000007e0,0x0000001f};
	register uint32 t;
	NTIMES( t=CONVERT1; *((short *)out)=t; out+=2; in+=3; )
	return out;
}

static uint8 *pack_8883(BitPacking *$, int n, const Number *in, uint8 *out) {
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
	return out;
}

uint32 bitpacking_masks[][4] = {
	{0x0000f800,0x000007e0,0x0000001f,0},
	{0x00ff0000,0x0000ff00,0x000000ff,0},
};

BitPacking builtin_bitpacks[] = {
	BitPacking(2, 2, 3, bitpacking_masks[0], pack_5652),
	BitPacking(1, 3, 3, bitpacking_masks[1], pack_8883),
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
Packer packer=0) {
	this->endian = endian;
	this->bytes = bytes;
	this->size = size;
	for (int i=0; i<size; i++) this->mask[i] = mask[i];
	if (packer) {
		this->packer = packer;
		return;
	}
	this->packer = default_pack;

	for (int i=0; i<(int)(sizeof(builtin_bitpacks)/sizeof(BitPacking)); i++) {
		BitPacking *bp = builtin_bitpacks+i;
		if (this->eq(bp)) {
			this->packer = bp->packer;
			::gfpost("Bitpacking: will be using special packer (#%d)",i);
		}
	}
}

void BitPacking::gfpost() {
	::gfpost("BitPacking:");
	::gfpost("    bytes: %d", bytes);
	for (int i=0;i<size;i++) {
		static const char *colour_name[] = {"red","green","blue","alpha"};
		::gfpost("    mask[%5s]: %08x (bits from %2d up to %2d)",
			colour_name[i],
			mask[i],
			low_bit(mask[i]),
			high_bit(mask[i]));
	}
}

bool BitPacking::is_le() {
	return endian==1 || (endian ^ ::is_le())==3;
}

uint8 *BitPacking::pack(int n, const Number *data, uint8 *target) {
	return packer(this,n,data,target);
}

#define LOOP_UNPACK(_reader_) \
	while (n--) { \
		int bytes=0, temp=0; \
		_reader_; \
		for (int i=0; i<size; i++) \
			*out++ = ((temp & mask[i]) << 7) >> hb[i]; \
	}

Number *BitPacking::unpack(int n, const uint8 *in, Number *out) {
	int hb[4];
	for (int i=0; i<size; i++) hb[i] = high_bit(mask[i]);

	if (is_le()) {
		/* smallest byte first */
		LOOP_UNPACK(while(this->bytes>bytes) { temp |= *in++ << (bytes++)*8; })
	} else {
		/* biggest byte first */
		LOOP_UNPACK(bytes=this->bytes;while(bytes--) { temp = (temp<<8) | *in++; })
	}
	return out;
}

/* this could be faster (use asm) */
void swap32 (int n, uint32 *data) {
	NTIMES({
		uint32 x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	})
}

/* this could be faster (use asm) */
void swap16 (int n, uint16 *data) {
	NTIMES({ uint16 x = *data; *data++ = (x<<8) | (x>>8); })
}

static VALUE String_swap32_f (VALUE $) {
	swap32(rb_str_len($)/4,(uint32 *)rb_str_ptr($));
	return $;
}

static VALUE String_swap16_f (VALUE $) {
	swap16(rb_str_len($)/2,(uint16 *)rb_str_ptr($));
	return $;
}

/* **************************************************************** */

METHOD2(BitPacking,init) {
	return Qnil;
}

METHOD2(BitPacking,pack2) {
	if (argc!=1 || TYPE(argv[0])!=T_STRING) RAISE("bad args");
	if (argc==2 && TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int n = rb_str_len(argv[0]) / sizeof(Number) / $->size;
	Number *in = (Number *)rb_str_ptr(argv[0]);
	int bytes = n*$->bytes;
	VALUE out = argc==2 ? rb_str_resize(argv[1],bytes) : rb_str_new("",bytes);
	rb_str_modify(out);
	$->pack(n,in,(uint8 *)rb_str_ptr(out));
	return out;
}

METHOD2(BitPacking,unpack2) {
	if (argc<1 || argc>2 || TYPE(argv[0])!=T_STRING) RAISE("bad args");
	if (argc==2 && TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int n = rb_str_len(argv[0]) / $->bytes;
	uint8 *in = (uint8 *)rb_str_ptr(argv[0]);
	int bytes = n*$->size*sizeof(Number);
	VALUE out = argc==2 ? rb_str_resize(argv[1],bytes) : rb_str_new("",bytes);
	rb_str_modify(out);
	memset(rb_str_ptr(out),255,n*4*$->size);
	$->unpack(n,in,(Number *)rb_str_ptr(out));
//	memcpy(rb_str_ptr(out),in,n);
	return out;
}

void BitPacking_mark (VALUE *$) {}
void BitPacking_sweep (VALUE *$) {fprintf(stderr,"sweeping BitPacking %p\n",$);}

static VALUE BitPacking_s_new(VALUE argc, VALUE *argv, VALUE qlass) {
	VALUE keep = rb_ivar_get(GridFlow_module, rb_intern("@fobjects_set"));
	BitPacking *c_peer;
	VALUE $; /* ruby_peer */

	if (argc!=3) RAISE("bad args");
	if (TYPE(argv[2])!=T_ARRAY) RAISE("bad mask");

	int endian = INT(argv[0]);
	int bytes = INT(argv[1]);
	VALUE *masks = rb_ary_ptr(argv[2]);
	uint32 masks2[4];
	int size = rb_ary_len(argv[2]);
	if (size<1) RAISE("not enough masks");
	if (size>4) RAISE("too many masks (%d)",size);
	for (int i=0; i<size; i++) masks2[i] = INT(masks[i]);
	c_peer = NEW(BitPacking,(endian,bytes,size,masks2));
	
	$ = Data_Wrap_Struct(qlass, BitPacking_mark, BitPacking_sweep, c_peer);
	rb_hash_aset(keep,$,Qtrue); /* prevent sweeping (leak) */
	rb_funcall2($,SI(initialize),argc,argv);
	return $;
}

GRCLASS(BitPacking,"",inlets:0,outlets:0,
LIST(),
	DECL(BitPacking,init),
	DECL(BitPacking,pack2),
	DECL(BitPacking,unpack2));

/* **************************************************************** */

void Dim::check() {
	if (n>MAX_DIMENSIONS) RAISE("too many dimensions");
	for (int i=0; i<n; i++)
		if (v[i]<0 || v[i]>MAX_INDICES)
			RAISE("dim[%d]=%d is out of range",i,v[i]);
}


/* returns a string like "Dim(240,320,3)" */
char *Dim::to_s() {
	/* if you blow 256 chars it's your own fault */
	char buf[256];
	char *s = buf;
	s += sprintf(s,"Dim(");
	for(int i=0; i<n; i++) s += sprintf(s,"%s%d", ","+!i, v[i]);
	s += sprintf(s,")");
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
/*	DECL_TYPE(    uint32, 32), */
	DECL_TYPE(     int32, 32),
/*
	DECL_TYPE(    uint64, 64),
	DECL_TYPE(     int64, 64),
	DECL_TYPE(   float32, 32),
	DECL_TYPE(   float64, 64),
	DECL_TYPE(   float80, 80),
	DECL_TYPE( complex64, 64),
	DECL_TYPE(complex128,128),
	DECL_TYPE(complex160,160),
*/
};

/* **************************************************************** */

/* future use, put in grid.h */
/* would be for optimising out stuff like "* 0" in convolution matrices... */
#define RHAND_NULL 0
#define RHAND_IDENTITY 1
#define RHAND_OTHER 2

/* **************************************************************** */

#define DEF_OP1(_name_,_expr_) \
	static Number op1_##_name_ (Number a) { return _expr_; } \
	static void op1_array_##_name_ (int n, Number *as) { \
		while ((n&3)!=0) { Number a = *as; *as++ = _expr_; n--; } \
		while (n) { \
			{ Number a=as[0]; as[0]= _expr_; } \
			{ Number a=as[1]; as[1]= _expr_; } \
			{ Number a=as[2]; as[2]= _expr_; } \
			{ Number a=as[3]; as[3]= _expr_; } \
		as+=4; n-=4; } }

DEF_OP1(abs,  a>=0 ? a : -a)
DEF_OP1(sqrt, (Number)(0+floor(sqrt(a))))
/*DEF_OP1(rand, (random()*(long long)a)/RAND_MAX)*/
DEF_OP1(rand, a==0 ? 0 : random()%a)
DEF_OP1(sq, a*a)

#define DECL_OP1(_name_,_sym_) \
	{ 0, _sym_, &op1_##_name_, &op1_array_##_name_ }

Operator1 op1_table[] = {
	DECL_OP1(abs, "abs"),
	DECL_OP1(sqrt,"sqrt"),
	DECL_OP1(rand,"rand"),
	DECL_OP1(sq,"sq"),
};

/* **************************************************************** */

#define DEF_OP2(_name_,_expr_) \
	static Number op_##_name_ (Number a, Number b) { return _expr_; } \
	static void op_array_##_name_ (int n, Number *as, Number b) { \
		while ((n&3)!=0) { Number a = *as; *as++ = _expr_; n--; } \
		while (n) { \
			{ Number a=as[0]; as[0]= _expr_; } \
			{ Number a=as[1]; as[1]= _expr_; } \
			{ Number a=as[2]; as[2]= _expr_; } \
			{ Number a=as[3]; as[3]= _expr_; } \
		as+=4; n-=4; } } \
	static void op_array2_##_name_ (int n, Number *as, const Number *bs) { \
		while ((n&3)!=0) { Number a = *as, b = *bs++; *as++ = _expr_; n--; } \
		while (n) { \
			{ Number a=as[0], b=bs[0]; as[0]= _expr_; } \
			{ Number a=as[1], b=bs[1]; as[1]= _expr_; } \
			{ Number a=as[2], b=bs[2]; as[2]= _expr_; } \
			{ Number a=as[3], b=bs[3]; as[3]= _expr_; } \
		as+=4; bs+=4; n-=4; } } \
	static Number op_fold_##_name_ (Number a, int n, const Number *bs) { \
		while (n--) { Number b = *bs++; a = _expr_; } return a; } \
	static void op_fold2_##_name_ (int an, Number *as, int n, const Number *bs) {\
		while (n--) { \
			int i=0; \
			while (i<an) { \
				{ Number a = as[i], b = *bs++; as[i] = _expr_; } i++; } } } \
	static void op_scan_##_name_ (Number a, int n, Number *bs) { \
		while (n--) { Number b = *bs; *bs++ = a = _expr_; } } \
	static void op_scan2_##_name_ (int an, const Number *as, int n, Number *bs) { \
		while (n--) { \
			for (int i=0; i<an; i++) { \
				Number a = *as++, b = *bs; *bs++ = a = _expr_; } \
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

DEF_OP2(sin, (Number)(b * sin(a * M_PI / 18000)))
DEF_OP2(cos, (Number)(b * cos(a * M_PI / 18000)))
DEF_OP2(atan, (Number)(atan2(a,b) * 18000 / M_PI))
DEF_OP2(tanh, (Number)(b * tanh(a * 2 * M_PI / 36000)))
DEF_OP2(gamma, b<=0 ? 0 : (Number)(0+floor(pow(a/256.0,256.0/b)*256.0)))
DEF_OP2(pow, ipow(a,b))

#define DECL_OP2(_name_,_sym_) { 0, _sym_, \
	&op_##_name_, \
	&op_array_##_name_, \
	&op_array2_##_name_, \
	&op_fold_##_name_, \
	&op_fold2_##_name_, \
	&op_scan_##_name_, \
	&op_scan2_##_name_ }


Operator2 op2_table[] = {
	DECL_OP2(add, "+"),
	DECL_OP2(sub, "-"),
	DECL_OP2(bus, "inv+"),

	DECL_OP2(mul, "*"),
	DECL_OP2(div, "/"),
	DECL_OP2(vid, "inv*"),
	DECL_OP2(mod, "%"),
	DECL_OP2(dom, "swap%"),
	DECL_OP2(rem, "rem"),
	DECL_OP2(mer, "swaprem"),

	DECL_OP2(or , "|"),
	DECL_OP2(xor, "^"),
	DECL_OP2(and, "&"),
	DECL_OP2(shl, "<<"),
	DECL_OP2(shr, ">>"),

	DECL_OP2(sc_and,"&&"),
	DECL_OP2(sc_or, "||"),

	DECL_OP2(min, "min"),
	DECL_OP2(max, "max"),

	DECL_OP2(eq,  "=="),
	DECL_OP2(ne,  "!="),
	DECL_OP2(gt,  ">"),
	DECL_OP2(le,  "<="),
	DECL_OP2(lt,  "<"),
	DECL_OP2(ge,  ">="),
	DECL_OP2(cmp, "cmp"),

	DECL_OP2(sin, "sin*"),
	DECL_OP2(cos, "cos*"),
	DECL_OP2(atan, "atan"),
	DECL_OP2(tanh, "tanh"),
	DECL_OP2(gamma, "gamma"),
	DECL_OP2(pow, "**"),
};

VALUE op1_dict = Qnil;
VALUE op2_dict = Qnil;

void startup_number () {
	int foo = PTR2FIX("hello");

	for (int i=0; i<COUNT(number_type_table); i++) {
		number_type_table[i].sym = ID2SYM(rb_intern(number_type_table[i].name));
	}

	rb_ivar_set(GridFlow_module,SI(@op1_dict),op1_dict=rb_hash_new());
	for(int i=0; i<COUNT(op1_table); i++) {
		op1_table[i].sym = ID2SYM(rb_intern(op1_table[i].name));
		rb_hash_aset(op1_dict,op1_table[i].sym,PTR2FIX((op1_table+i)));
	} 

	rb_ivar_set(GridFlow_module,SI(@op2_dict),op2_dict=rb_hash_new());
	for(int i=0; i<COUNT(op2_table); i++) {
		op2_table[i].sym = ID2SYM(rb_intern(op2_table[i].name));
		rb_hash_aset(op2_dict,op2_table[i].sym,PTR2FIX((op2_table+i)));
	} 

	VALUE BitPacking_class =
		rb_define_class_under(GridFlow_module, "BitPacking", rb_cObject);
	define_many_methods(BitPacking_class,
		BitPacking_classinfo.methodsn,
		BitPacking_classinfo.methods);
	SDEF(BitPacking,new,-1);
	rb_define_method(rb_cString, "swap32!", (RFunc)String_swap32_f, 0);
	rb_define_method(rb_cString, "swap16!", (RFunc)String_swap16_f, 0);
}
