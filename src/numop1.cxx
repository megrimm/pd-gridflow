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
#include <math.h>
#include <complex>

template <class T> class Op {
public:
	// I call abort() on those because I can't say they're purevirtual.
	static T f(T a) {abort();}
};

template <class O, class T> class OpLoops: public Numop1::On<T> {
public:
  static inline T f(T a) {return O::f(a);}
  #define FOO(I) as[I]=f(as[I]);
  static void _map (long n, T *as) {if (!n) return; UNROLL_8(FOO,n,as)}
  #undef FOO
};

using std::complex;
complex<float> operator / (const complex<float> &a, float b) {return complex<float>(a.real()/b,a.imag()/b);}

#define DEF_OP_COMMON(op,expr,T) inline static T f(T a) { return (T)(expr); }
#define DEF_OP(op,expr) template <class T> class Y##op : Op<T> { public: DEF_OP_COMMON(op,expr,T);};
#define DEF_OPFT(op,expr,T) template <> class Y##op<T> : Op<T> { public: DEF_OP_COMMON(op,expr,T);};
// this macro is for operators that have different code for the float version
#define DEF_OPF( op,expr,expr2) \
	DEF_OP(  op,expr      ) \
	DEF_OPFT(op,     expr2,float32) \
	DEF_OPFT(op,     expr2,float64)

#define  OL(O,T) OpLoops<Y##O<T>,T>
#define VOL(O,T) OpLoops<Y##O<complex<T> >,complex<T> >
#define DECL_OPON(L,O,T) Numop1::On<T>((Numop1::On<T>::Map) L(O,T)::_map)
#define DECLOP(        L,M,O,sym,dim) Numop1(sym,M(L,O,uint8),M(L,O,int16),M(L,O,int32) \
	NONLITE(,M(L,O,int64)),  M(L,O,float32)   NONLITE(,M(L,O,float64)),dim)
#define DECLOP_NOFLOAT(L,M,O,sym,dim) Numop1(sym,M(L,O,uint8),M(L,O,int16),M(L,O,int32) \
	NONLITE(,M(L,O,int64)),Numop1::On<float32>() NONLITE(,Numop1::On<float64>()),dim)
//	NONLITE(,M(L,O,int64),Numop1::On<float32>(),Numop1::On<float64>()),dim)
#define DECLOP_FLOAT(  L,M,O,sym,dim) Numop1(sym,Numop1::On<uint8>(),Numop1::On<int16>(),Numop1::On<int32>() \
	NONLITE(,Numop1::On<int64>()),M(L,O,float32) NONLITE(,M(L,O,float64)),dim)

#define DECL_OP(          O,sym)     DECLOP(         OL,DECL_OPON,O,sym,1)
#define DECL_OP_NOFLOAT(  O,sym)     DECLOP_NOFLOAT( OL,DECL_OPON,O,sym,1)
#define DECL_OP_FLOAT(    O,sym)     DECLOP_FLOAT(   OL,DECL_OPON,O,sym,1)
#define DECL_VOP(         O,sym,dim) DECLOP(        VOL,DECL_OPON,O,sym,dim)
#define DECL_VOP_NOFLOAT( O,sym,dim) DECLOP_NOFLOAT(VOL,DECL_OPON,O,sym,dim)
#define DECL_VOP_FLOAT(   O,sym,dim) DECLOP_FLOAT(  VOL,DECL_OPON,O,sym,dim)

DEF_OP(logic_not, !a);
DEF_OP(not, -1-a); // because ~a doesn't exist for T=float
DEF_OP(unary_minus, -a); // for use by #expr
DEF_OP(abs, a<0 ? -a : a);

DEF_OPF(sqrt,floor(sqrt(a)),sqrt(a))
DEF_OP(rand, a==0 ? (T)0 : (T)(random()%(int32)a))

DEF_OP(sin,  sin(a))		DEF_OP(cx_sin,  sin(a))
DEF_OP(cos,  cos(a))		DEF_OP(cx_cos,  cos(a))
DEF_OP(tan,  tan(a))
DEF_OP(asin, asin(a))
DEF_OP(acos, acos(a))
DEF_OP(atan, atan(a))
DEF_OP(sinh, sinh(a))
DEF_OP(cosh, cosh(a))
DEF_OP(tanh, tanh(a))		DEF_OP(cx_tanh, tanh(a))
DEF_OP(asinh, asinh(a))
DEF_OP(acosh, acosh(a))
DEF_OP(atanh, atanh(a))
DEF_OP(exp,  exp(a))		DEF_OP(cx_exp,  exp(a))
DEF_OP(log,  log(a))		DEF_OP(cx_log,  log(a))
DEF_OP(log10,log(a)/log(10))	DEF_OP(cx_log10,log(a)/log(10))
DEF_OP(log2, log(a)/log(2))	DEF_OP(cx_log2, log(a)/log(2))

DEF_OP(erf,  erf(a))
DEF_OP(erfc, erfc(a))
DEF_OP(cbrt, cbrt(a))
DEF_OP(expm1, expm1(a))
DEF_OP(log1p, log1p(a))
DEF_OP(floor, floor(a))
DEF_OP(ceil,  ceil(a))
DEF_OP(isinf, isinf(a))
DEF_OP(finite, finite(a))
DEF_OP(isnan, isnan(a))

static double fact (double x) {
	int sign=0;
	double y = exp(lgamma_r(x+1,&sign));
	if (1/y) return y*sign; else return 0/0.f;
}
DEF_OP(fact, fact(a))

Numop1 op_table_unary[] = {
// moved from numop2 at 9.12
	DECL_OP(sqrt, "sqrt"),
	DECL_OP(rand, "rand"),
	DECL_OP_FLOAT(sin,  "sin"),   DECL_VOP_FLOAT(cx_sin,  "C.sin",  2),
	DECL_OP_FLOAT(cos,  "cos"),   DECL_VOP_FLOAT(cx_cos,  "C.cos",  2),
	DECL_OP_FLOAT(tan, "tan"),
	DECL_OP_FLOAT(sinh, "sinh"),
	DECL_OP_FLOAT(cosh, "cosh"),
	DECL_OP_FLOAT(tanh, "tanh"),  DECL_VOP_FLOAT(cx_tanh, "C.tanh", 2),
	DECL_OP_FLOAT(exp,  "exp"),   DECL_VOP_FLOAT(cx_exp,  "C.exp",  2),
	DECL_OP_FLOAT(log,  "log"),   DECL_VOP_FLOAT(cx_log,  "C.log",  2),
// introduced at 9.12
	DECL_OP(unary_minus, "unary-"),
	DECL_OP(logic_not, "!"),
	DECL_OP(not, "~"),
	DECL_OP(abs, "abs"),
	DECL_OP_FLOAT(erf,  "erf"),
	DECL_OP_FLOAT(erfc, "erfc"),
	DECL_OP_FLOAT(cbrt, "cbrt"),
	DECL_OP_FLOAT(expm1, "expm1"),
	DECL_OP_FLOAT(log1p, "log1p"),
	DECL_OP_FLOAT(floor, "floor"),
	DECL_OP_FLOAT(ceil, "ceil"),
// 9.13
	DECL_OP(fact, "fact"),
	DECL_OP(log10, "log10"),	DECL_VOP_FLOAT(cx_log10,"C.log10",  2),
	DECL_OP(log2, "log2"),		DECL_VOP_FLOAT(cx_log2, "C.log2",   2),
//	DECL_OP_FLOAT(isinf, "isinf"),
//	DECL_OP_FLOAT(finite, "finite"),
//	DECL_OP_FLOAT(isnan, "isnan"),
	DECL_OP(asin, "asin"),
	DECL_OP(acos, "acos"),
	DECL_OP(atan, "atan"),
	DECL_OP(asin, "asinh"),
	DECL_OP(acos, "acosh"),
	DECL_OP(atan, "atanh"),
};
const long op_table_unary_n = COUNT(op_table_unary);

// D=dictionary, A=table, A##_n=table count.
#define INIT_TABLE(D,A) for(int i=0; i<A##_n; i++) D[string(A[i].name)]=&A[i];

void startup_numop1 () {
	INIT_TABLE(op_dict,op_table_unary)
}
