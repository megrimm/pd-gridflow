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
  NOTE: most essential code of BitPacking is used very often;
  the rest of the related code is special-case accelerations of it.
*/

#include "grid.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint8 *(*Packer)(BitPacking *$, int n, const Number *in, uint8 *out);
typedef Number *(*Unpacker)(BitPacking *$, int n, const uint8 *in, Number *out);

struct BitPacking {
	Packer pack;
	unsigned int endian; /* 0=big, 1=little, 2=same, 3=different */
	int bytes;
	uint32 mask[3];
};

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

/*
  This piece of code is little-endian specific, sorry.
  Will fix it when I get asked for it.
*/
#define CONVERT1 \
	(((in[0] << hb[0]) >> 7) & mask[0]) | \
	(((in[1] << hb[1]) >> 7) & mask[1]) | \
	(((in[2] << hb[2]) >> 7) & mask[2])

/* those macros would be faster if the _increment_
   were done only once every loop. or maybe gcc does it, i dunno */
#define FOURTIMES(_x_) _x_ _x_ _x_ _x_

#define NTIMES(_x_) \
	while (n>3) { FOURTIMES(_x_) n-=4; } \
	while (n--) { _x_ }

uint8 *default_pack(BitPacking *$, int n, const Number *in, uint8 *out) {
	register uint32 t;
	int hb[3] = {
		high_bit($->mask[0]),
		high_bit($->mask[1]),
		high_bit($->mask[2])};
	int mask[3];
	int sameorder = $->endian==2 || $->endian==is_le();
	memcpy(mask,$->mask,3*sizeof(int));
	if (sameorder && $->bytes==2) {
		NTIMES(t=CONVERT1; *((short *)out)=t; out+=2; in+=3;)
	} else if (sameorder && $->bytes==3) {
		NTIMES(t=CONVERT1; *((short *)out)=t; out[2]=t>>16; out+=3; in+=3;)
	} else if (sameorder && $->bytes==4) {
		NTIMES(t=CONVERT1; *((long *)out)=t; out+=4; in+=3;)
	} else if (BitPacking_is_le($)) {
		/* smallest byte first */
		whine("hello!");
		while (n--) {
			int bytes = $->bytes;
			t = CONVERT1;
			while (bytes--) { *out++ = t; t >>= 8; }
			in+=3;
		}
	} else {
		/* largest byte first */
		while (n--) {
			int bytes = $->bytes;
			t = CONVERT1;
			while (bytes--) { out[bytes] = t; t >>= 8; }
			out += $->bytes;
			in+=3;
		}
	}
	return out;
}

/* **************************************************************** */

uint8 *pack_5652(BitPacking *$, int n, const Number *in, uint8 *out) {
	const int hb[3] = {15,10,4};
	const int mask[3] = {0x0000f800,0x000007e0,0x0000001f};
	register uint32 t;
	while (n>3) {
		FOURTIMES( t=CONVERT1; *((short *)out)=t; out+=2; in+=3; )
		n-=4;
	}
	while (n--) {
		t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
	}
	return out;
}

uint8 *pack_8883(BitPacking *$, int n, const Number *in, uint8 *out) {
	while (n>3) {
		FOURTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
		n-=4;
	}
	while (n--) {
		out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3;
	}
	return out;
}

BitPacking builtin_bitpacks[] = {
	{ pack_5652, 2, 2, {0x0000f800,0x000007e0,0x0000001f} },
	{ pack_8883, 1, 3, {0x00ff0000,0x0000ff00,0x000000ff} },
};

/* **************************************************************** */

static bool BitPacking_eq(BitPacking *$, BitPacking *o) {
	if (!($->bytes == o->bytes)) return false;
	if (!($->mask[0] == o->mask[0] &&
		$->mask[1] == o->mask[1] &&
		$->mask[2] == o->mask[2])) return false;
	if ($->endian==o->endian) return true;
	/* same==little on a little-endian; same==big on a big-endian */
	return ($->endian ^ o->endian ^ is_le()) == 2;
}

BitPacking *BitPacking_new(int endian, int bytes, uint32 r, uint32
g, uint32 b) {
	int i;
	BitPacking *$ = NEW(BitPacking,1);
	$->endian = endian;
	$->bytes = bytes;
	$->mask[0] = r;
	$->mask[1] = g;
	$->mask[2] = b;
	$->pack = default_pack;

	for (i=0; i<(int)(sizeof(builtin_bitpacks)/sizeof(BitPacking)); i++) {
		BitPacking *bp = builtin_bitpacks+i;
		if (BitPacking_eq($,bp)) {
			$->pack = bp->pack;
			whine("Bitpacking: will be using special packer (#%d)",i);
		}
	}
	return $;
}

void BitPacking_whine(BitPacking *$) {
	int i;
	whine("BitPacking:");
	whine("    bytes: %d", $->bytes);
	for (i=0;i<3;i++) {
		static const char *colour_name[] = {"red","green","blue"};
		whine("    mask[%5s]: %08x (bits from %2d up to %2d)",
			colour_name[i],
			$->mask[i],
			low_bit($->mask[i]),
			high_bit($->mask[i]));
	}
}

int  BitPacking_bytes(BitPacking *$) { return $->bytes; }
bool BitPacking_is_le(BitPacking *$) {
	return $->endian==1 || ($->endian ^ is_le())==3; }

uint8 *BitPacking_pack(BitPacking *$, int n, const Number *in, uint8 *out) {
	return $->pack($,n,in,out);
}

Number *BitPacking_unpack(BitPacking *$, int n, const uint8 *in, Number *out) {
	int hb[3];
	hb[0] = high_bit($->mask[0]);
	hb[1] = high_bit($->mask[1]);
	hb[2] = high_bit($->mask[2]);
#define LOOP_UNPACK(_reader_) \
	while (n--) { \
		int bytes=0, temp=0; \
		_reader_; \
		*out++ = ((temp & $->mask[0]) << 7) >> hb[0]; \
		*out++ = ((temp & $->mask[1]) << 7) >> hb[1]; \
		*out++ = ((temp & $->mask[2]) << 7) >> hb[2]; \
	}

	if (BitPacking_is_le($)) {
		/* smallest byte first */
		LOOP_UNPACK(while($->bytes>bytes) { temp |= *in++ << (bytes++)*8; })
	} else {
		/* biggest byte first */
		LOOP_UNPACK(bytes=$->bytes;while(bytes--) { temp = (temp<<8) | *in++; })
	}
	return out;
}

/* this could be faster (unroll loops; use asm) */
void swap32 (int n, uint32 *data) {
	while(n--) {
		uint32 x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	}
}

/* this could be faster (unroll loops; use asm) */
void swap16 (int n, uint16 *data) {
	while(n--) {
		uint16 x = *data;
		*data++ = (x<<8) | (x>>8);
	}
}

/* **************************************************************** */

METHOD2(BitPacking,init) {
	return Qnil;
}

METHOD2(BitPacking,pack2) {
	if (argc!=1 || TYPE(argv[0])!=T_STRING) RAISE("bad args");
{
	int n = rb_str_len(argv[0]) / sizeof(Number) / 3;
	Number *in = (Number *)rb_str_ptr(argv[0]);
	VALUE out = rb_str_new("",n*BitPacking_bytes($));
	rb_str_modify(out);
	BitPacking_pack($,n,in,(uint8 *)rb_str_ptr(out));
	return out;
}}

METHOD2(BitPacking,unpack2) {
	if (argc!=1 || TYPE(argv[0])!=T_STRING) RAISE("bad args");
{
	int n = rb_str_len(argv[0]) / BitPacking_bytes($);
	uint8 *in = (uint8 *)rb_str_ptr(argv[0]);
	VALUE out = rb_str_new("",n*3*sizeof(Number));
	rb_str_modify(out);
	memset(rb_str_ptr(out),255,n*12);
	BitPacking_unpack($,n,in,(Number *)rb_str_ptr(out));
//	memcpy(rb_str_ptr(out),in,n);
	return out;
}}

void BitPacking_mark (VALUE *$) {}
void BitPacking_sweep (VALUE *$) {fprintf(stderr,"sweeping BitPacking %p\n",$);}

VALUE BitPacking_s_new(VALUE argc, VALUE *argv, VALUE qlass) {
	VALUE keep = rb_ivar_get(GridFlow_module, rb_intern("@fobjects_set"));
	BitPacking *c_peer;
	VALUE $; /* ruby_peer */

	if (argc!=3) RAISE("bad args");
	if (TYPE(argv[2])!=T_ARRAY) RAISE("bad mask");

	{
		int endian = INT(argv[0]);
		int bytes = INT(argv[1]);
		VALUE *masks;
		masks = rb_ary_ptr(argv[2]);
		c_peer = BitPacking_new(INT(argv[0]),INT(argv[1]),
			INT(masks[0]),INT(masks[1]),INT(masks[2]));
	}
	
	$ = Data_Wrap_Struct(qlass, BitPacking_mark, BitPacking_sweep, c_peer);
	rb_hash_aset(keep,$,Qtrue); /* prevent sweeping (leak) */
	rb_funcall2($,SI(initialize),argc,argv);
	return $;
}

GRCLASS(BitPacking,inlets:0,outlets:0,
LIST(),
	DECL(BitPacking,init),
	DECL(BitPacking,pack2),
	DECL(BitPacking,unpack2));

/* **************************************************************** */

#define Dim_invariant(_self_) \
	assert(_self_); \
	assert_range(_self_->n,0,MAX_DIMENSIONS);

Dim *Dim_new2(int n, int *v, const char *file, int line) {
	Dim *$ = (Dim *) NEW3(int,n+1,file,line);

	int i;
	assert_range(n,0,MAX_DIMENSIONS);
	assert(v);
	$->n = n;
	for (i=0; i<n; i++) {
		assert_range(v[i],0,MAX_INDICES);
		$->v[i] = v[i];
	}
	Dim_invariant($);
	return $;
}

Dim *Dim_dup2(Dim *$, const char *file, int line) {
	return Dim_new2($->n,$->v,file,line);
}

int Dim_count(Dim *$) {
	Dim_invariant($);
	return $->n;
}

int Dim_get(Dim *$, int i) {
	Dim_invariant($);
	assert_range(i,0,$->n-1);
	assert_range($->v[i],0,MAX_INDICES);
	return $->v[i];
}

int Dim_prod(Dim *$) {
	int v=1;
	int i;
	Dim_invariant($);
	for (i=0; i<$->n; i++) v *= $->v[i];
	return v;
}

int Dim_prod_start(Dim *$, int start) {
	int v=1;
	int i;
	Dim_invariant($);
	for (i=start; i<$->n; i++) v *= $->v[i];
	return v;
}

int Dim_calc_dex(Dim *$, int *v) {
	int dex=0;
	int i;
	for (i=0; i<$->n; i++) {
		dex = dex * $->v[i] + v[i];
	}
	return dex;
}

/* ******************************************************** */

/* returns a string like "Dim(240,320,3)" */
char *Dim_to_s(Dim *$) {
	/* if you blow 256 chars it's your own fault */
	char *bottom = NEW(char,256);
	char *s = bottom;
	int i=0;
	Dim_invariant($);
	s += sprintf(s,"Dim(");
	for(; i<$->n; i++) s += sprintf(s,"%s%d", ","+!i, $->v[i]);
	s += sprintf(s,")");
	return (char *)REALLOC(bottom,strlen(bottom)+1);
}

/* **************************************************************** */

#define DECL_TYPE(_name_,_size_) \
	{ 0, #_name_, _size_ }

static NumberType numeric_type_table[] = {
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

//	static void op_array_##_name_ (int n, Number *as, Number b) {
//		while (n--) { Number a = *as; *as++ = _expr_; } }

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
		while (n--) { Number b = *bs++; a = _expr_; } return a; }

DEF_OP2(add, a+b)
DEF_OP2(sub, a-b)
DEF_OP2(bus, b-a)

DEF_OP2(mul, a*b)
DEF_OP2(div, b==0 ? 0 : a/b)
DEF_OP2(vid, a==0 ? 0 : b/a)
DEF_OP2(mod, b==0 ? 0 : a%b)
DEF_OP2(dom, a==0 ? 0 : b%a)

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
	&op_##_name_, &op_array_##_name_, &op_array2_##_name_, &op_fold_##_name_ }

Operator2 op2_table[] = {
	DECL_OP2(add, "+"),
	DECL_OP2(sub, "-"),
	DECL_OP2(bus, "inv+"),

	DECL_OP2(mul, "*"),
	DECL_OP2(div, "/"),
	DECL_OP2(vid, "inv*"),
	DECL_OP2(mod, "%"),
	DECL_OP2(dom, "swap%"),

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

void startup_number (void) {
	int i;
	rb_define_readonly_variable("$op1_dict",&op1_dict);
	rb_define_readonly_variable("$op2_dict",&op2_dict);
	op1_dict = rb_hash_new();
	op2_dict = rb_hash_new();
	for(i=0; i<COUNT(op1_table); i++) {
		op1_table[i].sym = ID2SYM(rb_intern(op1_table[i].name));
		rb_hash_aset(op1_dict,op1_table[i].sym,PTR2FIX(&op1_table[i]));
	} 
	for(i=0; i<COUNT(op2_table); i++) {
		op2_table[i].sym = ID2SYM(rb_intern(op2_table[i].name));
		rb_hash_aset(op2_dict,op2_table[i].sym,PTR2FIX(&op2_table[i]));
	} 
	{
		VALUE BitPacking_class =
			rb_define_class_under(GridFlow_module, "BitPacking", rb_cObject);
		define_many_methods(BitPacking_class,
			BitPacking_classinfo.methodsn,
			BitPacking_classinfo.methods);
		SDEF(BitPacking,new,-1);
	}
}
