/*
	$Id$

	Video4jmax
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

#include "grid.h"

#define DEF_OP1(_name_,_expr_) \
	static Number op1_##_name_ (Number a) { return _expr_; } \
	static void op1_array_##_name_ (int n, Number *as) { \
		while (n--) { Number a = *as; *as++ = _expr_; } }

DEF_OP1(abs, a>=0 ? a : -a)

#define DECL_OP1(_name_,_sym_) \
	{ 0, _sym_, &op1_##_name_, &op1_array_##_name_ }


Operator1 op1_table[] = {
	DECL_OP1(abs,"abs")
};

Operator1 *op1_table_find(fts_symbol_t sym) {
	int i;
	for(i=0; i<COUNT(op1_table); i++) {
		if (op1_table[i].sym == sym) return &op1_table[i];
	}
	return 0;
}

/* **************************************************************** */

#define DEF_OP2(_name_,_expr_) \
	static Number op_##_name_ (Number a, Number b) { return _expr_; } \
	static void op_array_##_name_ (int n, Number *as, Number b) { \
		while (n--) { Number a = *as; *as++ = _expr_; } } \
	static Number op_fold_##_name_ (Number a, int n, const Number *as) { \
		while (n--) { Number b = *as; a = _expr_; } return a; }

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

DEF_OP2(min, a<b?a:b)
DEF_OP2(max, a>b?a:b)

DEF_OP2(eq,  a == b)
DEF_OP2(ne,  a != b)
DEF_OP2(gt,  a >  b)
DEF_OP2(le,  a <= b)
DEF_OP2(lt,  a <  b)
DEF_OP2(ge,  a >= b)

DEF_OP2(sin, (int)(b * sin(a * 2 * M_PI / 36000)))
DEF_OP2(cos, (int)(b * cos(a * 2 * M_PI / 36000)))

#define DECL_OP2(_name_,_sym_) \
	{ 0, _sym_, &op_##_name_, &op_array_##_name_, &op_fold_##_name_ }

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

	DECL_OP2(min, "min"),
	DECL_OP2(max, "max"),

	DECL_OP2(eq,  "=="),
	DECL_OP2(ne,  "!="), //!@#$
	DECL_OP2(gt,  ">"),
	DECL_OP2(le,  "<="), //!@#$
	DECL_OP2(lt,  "<"),
	DECL_OP2(ge,  ">="), //!@#$

	DECL_OP2(sin, "sin*"),
	DECL_OP2(cos, "cos*"),
};

Operator2 *op2_table_find(fts_symbol_t sym) {
	int i;
	for(i=0; i<COUNT(op2_table); i++) {
		if (op2_table[i].sym == sym) return &op2_table[i];
	}
	return 0;
}

void startup_operator (void) {
	int i;
	for(i=0; i<COUNT(op1_table); i++) {
		op1_table[i].sym = fts_new_symbol(op1_table[i].name);
	} 
	for(i=0; i<COUNT(op2_table); i++) {
		op2_table[i].sym = fts_new_symbol(op2_table[i].name);
	} 
}
