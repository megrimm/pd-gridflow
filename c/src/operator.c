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

#include <math.h>
#include <stdlib.h>
#include "grid.h"

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

/*
typedef struct {
	int type;
	void *buffer;
} Array;
*/

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
		while ((n&3)!=0) { Number a = *as; *as++ = _expr_; } \
		while (n) { \
			{ Number a = as[0]; as[0] = _expr_; } \
			{ Number a = as[1]; as[1] = _expr_; } \
			{ Number a = as[2]; as[2] = _expr_; } \
			{ Number a = as[3]; as[3] = _expr_; } \
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

Dict *op1_dict;

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

Dict *op2_dict;

void startup_operator (void) {
	int i;
	op1_dict = Dict_new(0);
	op2_dict = Dict_new(0);
	for(i=0; i<COUNT(op1_table); i++) {
		op1_table[i].sym = Symbol_new(op1_table[i].name);
		Dict_put(op1_dict,(void *)(int)op1_table[i].sym,&op1_table[i]);
	} 
	for(i=0; i<COUNT(op2_table); i++) {
		op2_table[i].sym = Symbol_new(op2_table[i].name);
		Dict_put(op2_dict,(void *)(int)op2_table[i].sym,&op2_table[i]);
	} 
}
