/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003,2004 by Mathieu Bouchard

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

#include "grid.h.fcs"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

/* ---------------------------------------------------------------- */

NumberType number_type_table[] = {
#define FOO(_sym_,_size_,_flags_,args...) NumberType( #_sym_, _size_, _flags_, args ),
NUMBER_TYPES(FOO)
#undef FOO
};

/* ---------------------------------------------------------------- */

template <class T>
class Op1 {public: static T f(T); };

template <class O>
class Op1Loops {
public:
	template <class T>
	static void op_map (int n, T *as) {
#define FOO(I) as[I]=O::f(as[I]);
	UNROLL_8(FOO,n,as)
#undef FOO
	}
};

#define DEF_OP1(op,expr) \
	template <class T> class Y1##op : Op1<T> { public: \
		inline static T f(T a) { return expr; } };

#define DECL_OP1ON(base,op,T) Numop1On<T>( \
	&base<Y1##op<T> >::op_map )

#ifdef HAVE_LITE
#define DECL_OP1(op,sym,props) Numop1( 0, sym, \
	DECL_OP1ON(Op1Loops,op,uint8), DECL_OP1ON(Op1Loops,op,int16), \
	DECL_OP1ON(Op1Loops,op,int32))
#else
#define DECL_OP1(op,sym,props) Numop1( 0, sym, \
	DECL_OP1ON(Op1Loops,op,uint8), DECL_OP1ON(Op1Loops,op,int16), \
	DECL_OP1ON(Op1Loops,op,int32), DECL_OP1ON(Op1Loops,op,int64), \
	DECL_OP1ON(Op1Loops,op,float32), DECL_OP1ON(Op1Loops,op,float64))
#endif

DEF_OP1(abs, a>=0 ? a : -a)
DEF_OP1(sqrt, (T)(0+floor(sqrt(a))))
/*DEF_OP1(rand, (random()*(long long)a)/RAND_MAX)*/
DEF_OP1(rand, a==0 ? 0 : random()%(int32)a)
DEF_OP1(sq, a*a)

Numop1 op1_table[] = {
	DECL_OP1(abs, "abs",""),
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
class Op2 {
public:
	static T f(T a, T b);
	static bool is_neutral(T x, LeftRight side) {}
	static bool is_absorbent(T x, LeftRight side) {}
};

template <class O>
class Op2Loops {
public:
	template <class T>
	static void op_map (int n, T *as, T b) {
		if (!n) return;
#define FOO(I) as[I]=O::f(as[I],b);
	UNROLL_8(FOO,n,as)
#undef FOO
	}
	template <class T>
	static void op_zip (int n, T *as, T *bs) {
		if (!n) return;
		int ba=bs-as; // really!
#define FOO(I) as[I]=O::f(as[I],as[ba+I]);
	UNROLL_8(FOO,n,as)
#undef FOO
	}
	// disabled
	template <class T>
	static void op_zip2 (int n, T *as, T *bs, T *cs) {
		if (!n) return;
		int ba=bs-as, ca=cs-as;
#define FOO(I) as[ca+I]=O::f(as[I],as[ba+I]);
	UNROLL_8(FOO,n,as)
#undef FOO
	}
	template <class T>
	static void op_fold (int an, int n, T *as, T *bs) {
		switch (an) {
		case 1:
			for (; (n&3)!=0; bs++, n--) *as=O::f(*as,*bs);
			for (; n; bs+=4, n-=4)
				*as=O::f(O::f(O::f(O::f(*as,bs[0]),bs[1]),bs[2]),bs[3]);
		break;
		case 2:
			for (; (n&3)!=0; bs+=2, n--) {
				as[0]=O::f(as[0],bs[0]);
				as[1]=O::f(as[1],bs[1]); }
			for (; n; bs+=8, n-=4) {
				as[0]=O::f(O::f(O::f(O::f(as[0],bs[0]),bs[2]),bs[4]),bs[6]);
				as[1]=O::f(O::f(O::f(O::f(as[1],bs[1]),bs[3]),bs[5]),bs[7]); }
		break;
		case 3:
			for (; (n&3)!=0; bs+=3, n--) {
				as[0]=O::f(as[0],bs[0]);
				as[1]=O::f(as[1],bs[1]);
				as[2]=O::f(as[2],bs[2]); }
			for (; n; bs+=12, n-=4) {
				as[0]=O::f(O::f(O::f(O::f(as[0],bs[0]),bs[3]),bs[6]),bs[9]);
				as[1]=O::f(O::f(O::f(O::f(as[1],bs[1]),bs[4]),bs[7]),bs[10]);
				as[2]=O::f(O::f(O::f(O::f(as[2],bs[2]),bs[5]),bs[8]),bs[11]); }
		break;
		case 4:
			for (; (n&3)!=0; bs+=4, n--) {
				as[0]=O::f(as[0],bs[0]);
				as[1]=O::f(as[1],bs[1]);
				as[2]=O::f(as[2],bs[2]);
				as[3]=O::f(as[3],bs[3]); }
			for (; n; bs+=16, n-=4) {
				as[0]=O::f(O::f(O::f(O::f(as[0],bs[0]),bs[4]),bs[ 8]),bs[12]);
				as[1]=O::f(O::f(O::f(O::f(as[1],bs[1]),bs[5]),bs[ 9]),bs[13]);
				as[2]=O::f(O::f(O::f(O::f(as[2],bs[2]),bs[6]),bs[10]),bs[14]);
				as[3]=O::f(O::f(O::f(O::f(as[3],bs[3]),bs[7]),bs[11]),bs[15]); }
		break;
		default:
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

template <class T>
static void quick_mod_map (int n, T *as, T b) {
	if (!b) return;
#define FOO(I) as[I]=mod(as[I],b);
	UNROLL_8(FOO,n,as)
#undef FOO
}

template <class T>
static void quick_ign_map (int n, T *as, T b) {}
template <class T>
static void quick_ign_zip (int n, T *as, T *bs) {}
template <class T>
static void quick_put_map (int n, T *as, T b) {
#define FOO(I) as[I]=b;
	UNROLL_8(FOO,n,as)
#undef FOO
}
void quick_put_map (int n, int16 *as, int16 b) {
	if (n&1!=0 && (long)as&4!=0) { *as++=b; n--; }
	quick_put_map (n>>1, (int32 *)as, (int32)(b<<16)+b);
	if (n&1!=0) *as++=b;
}
void quick_put_map (int n, uint8 *as, uint8 b) {
	while (n&3!=0 && (long)as&4!=0) { *as++=b; n--; }
	int32 c=(b<<8)+b; c+=c<<16;
	quick_put_map (n>>2, (int32 *)as, c);
	while (n&3!=0) *as++=b;
}
template <class T>
static void quick_put_zip (int n, T *as, T *bs) {
	gfmemcopy((uint8 *)as, (uint8 *)bs, n*sizeof(T));
}

/* classic two-input operator */
#define DEF_OP2(op,expr,neutral,absorbent) \
	template <class T> class Y##op : Op2<T> { public: \
		inline static T f(T a, T b) { return expr; } \
		inline static bool is_neutral(T x, LeftRight side) { return neutral; } \
		inline static bool is_absorbent(T x, LeftRight side) { return absorbent; } };

/* this macro is for operators that have different code for the float version */
#define DEF_OP2F(op,expr,expr2,neutral,absorbent) \
	DEF_OP2( op,expr,      neutral,absorbent) \
	class Y##op<float32> : Op2<float32> { public: \
		inline static float32 f(float32 a, float32 b) { return expr2; } \
		inline static bool is_neutral(float32 x, LeftRight side) { return neutral; } \
		inline static bool is_absorbent(float32 x, LeftRight side) { return absorbent; } }; \
	class Y##op<float64> : Op2<float64> { public: \
		inline static float64 f(float64 a, float64 b) { return expr2; } \
		inline static bool is_neutral(float64 x, LeftRight side) { return neutral; } \
		inline static bool is_absorbent(float64 x, LeftRight side) { return absorbent; } }; \

#define DECL_OP2ON(base,op,T) Numop2On<T>( \
	&base<Y##op<T> >::op_map, &base<Y##op<T> >::op_zip, \
	&base<Y##op<T> >::op_fold, &base<Y##op<T> >::op_scan, \
	&Y##op<T>::is_neutral, &Y##op<T>::is_absorbent)

#define DECL_OP2ON_NOFOLD(base,op,T) Numop2On<T>( \
	&base<Y##op<T> >::op_map, &base<Y##op<T> >::op_zip, 0,0, \
	&Y##op<T>::is_neutral, &Y##op<T>::is_absorbent)

#ifdef HAVE_LITE
#define DECL_OP2(op,sym,flags) Numop2(0, sym, \
	DECL_OP2ON(Op2Loops,op,uint8), DECL_OP2ON(Op2Loops,op,int16), \
	DECL_OP2ON(Op2Loops,op,int32), flags)
#define DECL_OP2_NOFLOAT(op,sym,flags) Numop2(0, sym, \
	DECL_OP2ON(Op2Loops,op,uint8), DECL_OP2ON(Op2Loops,op,int16), \
	DECL_OP2ON(Op2Loops,op,int32), flags)
#define DECL_OP2_NOFOLD(op,sym,flags) Numop2(0, sym, \
	DECL_OP2ON_NOFOLD(Op2Loops,op,uint8), DECL_OP2ON_NOFOLD(Op2Loops,op,int16), \
	DECL_OP2ON_NOFOLD(Op2Loops,op,int32), flags)
#else
#define DECL_OP2(op,sym,flags) Numop2(0, sym, \
	DECL_OP2ON(Op2Loops,op,uint8), DECL_OP2ON(Op2Loops,op,int16), \
	DECL_OP2ON(Op2Loops,op,int32), DECL_OP2ON(Op2Loops,op,int64), \
	DECL_OP2ON(Op2Loops,op,float32), DECL_OP2ON(Op2Loops,op,float64), \
	flags)
#define DECL_OP2_NOFLOAT(op,sym,flags) Numop2(0, sym, \
	DECL_OP2ON(Op2Loops,op,uint8), DECL_OP2ON(Op2Loops,op,int16), \
	DECL_OP2ON(Op2Loops,op,int32), DECL_OP2ON(Op2Loops,op,int64), \
	Numop2On<float32>(0,0,0,0,0,0), Numop2On<float64>(0,0,0,0,0,0), \
	flags)
#define DECL_OP2_NOFOLD(op,sym,flags) Numop2(0, sym, \
	DECL_OP2ON_NOFOLD(Op2Loops,op,uint8), DECL_OP2ON_NOFOLD(Op2Loops,op,int16), \
	DECL_OP2ON_NOFOLD(Op2Loops,op,int32), DECL_OP2ON_NOFOLD(Op2Loops,op,int64), \
	DECL_OP2ON_NOFOLD(Op2Loops,op,float32), DECL_OP2ON_NOFOLD(Op2Loops,op,float64), \
	flags)
#endif

template <class T>
static inline T gf_floor (T a) {
	return (T) floor((double)a); }

template <class T>
static inline T gf_trunc (T a) {
	return (T) floor(abs((double)a)) * (a<0?-1:1); }

/*
Algebraic Properties
  Note: N,A are now coded into DEF_OP2A
	
  RN: right neutral: { e in G | f(x,RN)=x }
  LN: left neutral: { e in G | f(x,RN)=x } (left neutral)
  N: both sides neutral: N=LN=RN
  RA,LA,A: absorbence

  LINV: left inverse: each a has a b like f(a,b) = the left neutral
  RINV: right inverse: each b has a a like f(a,b) = the right neutral
  INV: both sides inverse
*/
	
DEF_OP2(ignore, a, side==at_right, side==at_left)
DEF_OP2(put, b, side==at_left, side==at_right)

DEF_OP2(add, a+b, x==0, false)
DEF_OP2(sub, a-b, side==at_right && x==0, false)
DEF_OP2(bus, b-a, side==at_left  && x==0, false)

DEF_OP2(mul, a*b, x==1, x==0)
DEF_OP2(mulshr8, ((int32)a*(int32)b)>>8, (int64)x==256, x==0) //!@#$ bug with int64
DEF_OP2(div, b==0 ? 0 : a/b, side==at_right && x==1, false)
DEF_OP2(div2, b==0 ? 0 : div2(a,b), side==at_right && x==1, false)
DEF_OP2(vid, a==0 ? 0 : b/a, side==at_left && x==1, false)
DEF_OP2(vid2, a==0 ? 0 : div2(b,a), side==at_left && x==1, false)
DEF_OP2F(mod, b==0 ? 0 : mod(a,b), b==0 ? 0 : a-b*gf_floor(a/b),
	false, side==at_left && x==0 || side==at_right && x==1)
DEF_OP2F(dom, a==0 ? 0 : mod(b,a), a==0 ? 0 : b-a*gf_floor(b/a),
	false, side==at_left && x==0 || side==at_right && x==1)

/*
DEF_OP2F(rem, b==0 ? 0 : a%b, b==0 ? 0 : a-b*gf_trunc(a/b))
DEF_OP2F(mer, a==0 ? 0 : b%a, a==0 ? 0 : b-a*gf_trunc(b/a))
*/
DEF_OP2(rem, b==0 ? 0 : a%b,
	false, side==at_left && x==0 || side==at_right && x==1)
DEF_OP2(mer, a==0 ? 0 : b%a,
	false, side==at_left && x==0 || side==at_right && x==1)

DEF_OP2(gcd, gcd(a,b), x==0, x==1)
DEF_OP2(gcd2, gcd2(a,b), x==0, x==1) // should test those and pick one of the two
DEF_OP2(lcm, a==0 || b==0 ? 0 : lcm(a,b), x==1, x==0)

DEF_OP2F(or , a|b, (float32)((int32)a | (int32)b), x==0, x==nt_all_ones(&x))
DEF_OP2F(xor, a^b, (float32)((int32)a ^ (int32)b), x==0, false)
DEF_OP2F(and, a&b, (float32)((int32)a & (int32)b), x==nt_all_ones(&x), x==0)
DEF_OP2F(shl, a<<b, a*pow(2.0,+b), side==at_right && x==0, false)
DEF_OP2F(shr, a>>b, a*pow(2.0,-b), side==at_right && x==0, false)

DEF_OP2(sc_and, a ? b : a, side==at_left && x!=0, side==at_left && x==0)
DEF_OP2(sc_or,  a ? a : b, side==at_left && x==0, side==at_left && x!=0)

DEF_OP2(min, min(a,b), x==nt_greatest(&x), x==nt_smallest(&x))
DEF_OP2(max, max(a,b), x==nt_smallest(&x), x==nt_greatest(&x))

DEF_OP2(cmp, cmp(a,b), false, false)
DEF_OP2(eq,  a == b, false, false)
DEF_OP2(ne,  a != b, false, false)
DEF_OP2(gt,  a >  b, false, side==at_left&&x==nt_smallest(&x)||side==at_right&&x==nt_greatest(&x))
DEF_OP2(le,  a <= b, false, side==at_left&&x==nt_smallest(&x)||side==at_right&&x==nt_greatest(&x))
DEF_OP2(lt,  a <  b, false, side==at_left&&x==nt_greatest(&x)||side==at_right&&x==nt_smallest(&x))
DEF_OP2(ge,  a >= b, false, side==at_left&&x==nt_greatest(&x)||side==at_right&&x==nt_smallest(&x))

DEF_OP2(sin, (T)(b * sin(a * (M_PI / 18000))), false, false) // "LN=9000+36000n RA=0 LA=..."
DEF_OP2(cos, (T)(b * cos(a * (M_PI / 18000))), false, false) // "LN=36000n RA=0 LA=..."
DEF_OP2(atan, (T)(atan2(a,b) * (18000 / M_PI)), false, false) // "LA=0"
DEF_OP2(tanh, (T)(b * tanh(a * (M_PI / 18000))), false, x==0)
DEF_OP2(gamma, b<=0 ? 0 : (T)(0+floor(pow(a/256.0,256.0/b)*256.0)), false, false) // "RN=256"
DEF_OP2(pow, ipow(a,b), false, false) // "RN=1"
DEF_OP2(log, (T)(a==0 ? 0 : b * log(abs(a))), false, false) // "RA=0"

// 0.7.8
//DEF_OP2(clipadd, min(nt_greatest(&a),max(nt_smallest(&a),a+b)), x==0, false)
//DEF_OP2(clipsub, min(nt_greatest(&a),max(nt_smallest(&a),a-b)), side==at_right && x==0, false)
DEF_OP2(abssub,  abs(a-b), false, false)
DEF_OP2(sqsub, (a-b)*(a-b), false, false)
DEF_OP2(avg, (a-b)/2, false, false)
//DEF_OP2(erf,"erf*", 0)
DEF_OP2(hypot, (T)(0+floor(sqrt(a*a+b*b))), false, false)

Numop2 op2_table[] = {
	DECL_OP2(ignore, "ignore", OP2_ASSOC),
	DECL_OP2(put, "put", OP2_ASSOC),

	DECL_OP2(add, "+", OP2_ASSOC|OP2_COMM), // "LINV=sub"
	DECL_OP2(sub, "-", 0),
	DECL_OP2(bus, "inv+", 0),

	DECL_OP2(mul, "*", OP2_ASSOC|OP2_COMM),
	DECL_OP2_NOFLOAT(mulshr8, "*>>8", OP2_ASSOC|OP2_COMM),
	DECL_OP2(div, "/", 0),
	DECL_OP2_NOFLOAT(div2, "div", 0),
	DECL_OP2(vid, "inv*", 0),
	DECL_OP2_NOFLOAT(vid2, "swapdiv", 0),
	DECL_OP2_NOFLOAT(mod, "%", 0),
	DECL_OP2_NOFLOAT(dom, "swap%", 0),
	DECL_OP2_NOFLOAT(rem, "rem", 0),
	DECL_OP2_NOFLOAT(mer, "swaprem", 0),
	DECL_OP2_NOFLOAT(gcd, "gcd", OP2_ASSOC|OP2_COMM),
	DECL_OP2_NOFLOAT(gcd2, "gcd2", OP2_ASSOC|OP2_COMM),
	DECL_OP2_NOFLOAT(lcm, "lcm", OP2_ASSOC|OP2_COMM),

	DECL_OP2(or , "|", OP2_ASSOC|OP2_COMM),
	DECL_OP2(xor, "^", OP2_ASSOC|OP2_COMM),
	DECL_OP2(and, "&", OP2_ASSOC|OP2_COMM),
	DECL_OP2_NOFOLD(shl, "<<", 0),
	DECL_OP2_NOFOLD(shr, ">>", 0),

	DECL_OP2_NOFOLD(sc_and,"&&", 0),
	DECL_OP2_NOFOLD(sc_or, "||", 0),

	DECL_OP2(min, "min", OP2_ASSOC|OP2_COMM),
	DECL_OP2(max, "max", OP2_ASSOC|OP2_COMM),

	DECL_OP2_NOFOLD(eq,  "==", OP2_COMM),
	DECL_OP2_NOFOLD(ne,  "!=", OP2_COMM),
	DECL_OP2_NOFOLD(gt,  ">", 0),
	DECL_OP2_NOFOLD(le,  "<=", 0),
	DECL_OP2_NOFOLD(lt,  "<", 0),
	DECL_OP2_NOFOLD(ge,  ">=", 0),
	DECL_OP2_NOFOLD(cmp, "cmp", 0),

	DECL_OP2_NOFOLD(sin, "sin*", 0),
	DECL_OP2_NOFOLD(cos, "cos*", 0),
	DECL_OP2_NOFOLD(atan, "atan", 0),
	DECL_OP2_NOFOLD(tanh, "tanh*", 0),
	DECL_OP2_NOFOLD(gamma, "gamma", 0),
	DECL_OP2_NOFOLD(pow, "**", 0),
	DECL_OP2_NOFOLD(log, "log*", 0),
// 0.7.8
	//DECL_OP2(clipadd,"clip+", OP2_ASSOC|OP2_COMM),
	//DECL_OP2(clipsub,"clip-", 0),
	DECL_OP2_NOFOLD(abssub,"abs-", OP2_COMM),
	DECL_OP2_NOFOLD(sqsub,"sq-", OP2_COMM),
	DECL_OP2_NOFOLD(avg,"avg", OP2_COMM),
	DECL_OP2_NOFOLD(hypot,"hypot", OP2_COMM),
	//DECL_OP2_NOFOLD(erf,"erf*", 0),
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
	Numop2 *foo = FIX2PTR(Numop2,rb_hash_aref(op2_dict,SYM(_name_))); \
	foo->on_uint8.op_##_mode_ = _func_; \
	foo->on_int16.op_##_mode_ = _func_; \
	foo->on_int32.op_##_mode_ = _func_; }

	OVERRIDE_INT(ignore,map,quick_ign_map);
	OVERRIDE_INT(ignore,zip,quick_ign_zip);
#if 0
	OVERRIDE_INT(put,map,quick_put_map);
	OVERRIDE_INT(put,zip,quick_put_zip);
	OVERRIDE_INT(%,map,quick_mod_map); // !@#$ does that make an improvement at all?
#endif
}

