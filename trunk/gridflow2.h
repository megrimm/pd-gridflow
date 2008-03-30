/*
	$Id$

	GridFlow
	Copyright (c) 2001-2007 by Mathieu Bouchard

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

/* this file should contain the things moved from grid.h that are expected to stick around in GridFlow 2, plus some extra things. */
/* some of those things might still be in grid.h instead though. */
#ifndef __GRIDFLOW2_H
#define __GRIDFLOW2_H
#include "m_pd.h"
#undef T_OBJECT
#include "config.h"
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#ifdef __APPLE__
static inline void *memalign (size_t a, size_t n) {return malloc(n);}
#else
#include <malloc.h>
#endif

typedef char       int8; typedef unsigned char      uint8;
typedef short     int16; typedef unsigned short     uint16;
typedef int       int32; typedef unsigned int       uint32;
typedef long long int64; typedef unsigned long long uint64;
typedef float   float32;
typedef double  float64;

// three-way comparison (T is assumed Comparable)
template <class T> static inline T cmp(T a, T b) { return a<b ? -1 : a>b; }

// a remainder function such that div2(a,b)*b+mod(a,b) = a and for which
// mod(a,b) is in [0;b) or (b;0]. in contrast to C-language builtin a%b,
// this one has uniform behaviour around zero.
static inline int mod(int a, int b) {return a<0 ? b-((-a)%b) : a%b;}

// counterpart of mod(a,b), just like a/b and a%b are counterparts
static inline int div2(int a, int b) {return (a/b)-((a<0)&&!!(a%b));}

static inline int32   gf_abs(  int32 a) { return a>0?a:-a; }
static inline int64   gf_abs(  int64 a) { return a>0?a:-a; }
static inline float32 gf_abs(float32 a) { return fabs(a); }
static inline float64 gf_abs(float64 a) { return fabs(a); }

// integer powers in log(b) time. T is assumed Integer
template <class T> static inline T ipow(T a, T b) {
	for(T r=1;;) {if (b&1) r*=a; b>>=1; if (!b) return r; a*=a;}
}
static inline float32 ipow(float32 a, float32 b) { return pow(a,b); }
static inline float64 ipow(float64 a, float64 b) { return pow(a,b); }

#undef min
#undef max
// minimum/maximum functions; T is assumed to be Comparable
template <class T> static inline T min(T a, T b) { return a<b?a:b; }
template <class T> static inline T max(T a, T b) { return a>b?a:b; }
//template <class T> inline T min(T a, T b) { T c = (a-b)>>31; return (a&c)|(b&~c); }

// greatest common divisor, by euclid's algorithm
// this runs in log(a+b) number operations
template <class T> static T gcd (T a, T b) {
	while (b) {T c=mod(a,b); a=b; b=c;}
	return a;
}

// greatest common divisor, the binary algorithm. haven't tried yet.
template <class T> static T gcd2 (T a, T b) {
	int s=0;
	while ((a|b)&1==0) { a>>=1; b>>=1; s++; }
	while (a) {
		if (a&1==0) a>>=1;
		else if (b&1==0) b>>=1;
		else {T t=abs(a-b); if (a<b) b=t; else a=t;}
	}
	return b<<s;
}

// least common multiple; this runs in log(a+b) like gcd.
template <class T> static inline T lcm (T a, T b) {return a*b/gcd(a,b);}

// returns the position (0..63) of highest bit set in a word, or 0 if none.
#define Z(N) if ((x>>N)&(((typeof(x))1<<N)-1)) { x>>=N; i+=N; }
static int highest_bit(uint8  x) {int i=0;              Z(4)Z(2)Z(1)return i;}
static int highest_bit(uint16 x) {int i=0;          Z(8)Z(4)Z(2)Z(1)return i;}
static int highest_bit(uint32 x) {int i=0;     Z(16)Z(8)Z(4)Z(2)Z(1)return i;}
static int highest_bit(uint64 x) {int i=0;Z(32)Z(16)Z(8)Z(4)Z(2)Z(1)return i;}
#undef Z
// returns the position (0..63) of lowest bit set in a word, or 0 if none.
template <class T> static int lowest_bit(T n) { return highest_bit((~n+1)&n); }

static double drand() { return 1.0*rand()/(RAND_MAX+1.0); }

// is little-endian
static inline bool is_le() {int x=1; return ((char *)&x)[0];}

static inline uint64 rdtsc()
#if defined(HAVE_PENTIUM)
{uint64 x; __asm__ volatile (".byte 0x0f, 0x31":"=A"(x)); return x;}
#else
{return 0;}
#endif

#ifdef HAVE_LITE
#define EACH_INT_TYPE(MACRO) MACRO(uint8) MACRO(int16) MACRO(int32)
#define EACH_FLOAT_TYPE(MACRO) MACRO(float32)
#else
#define EACH_INT_TYPE(MACRO) MACRO(uint8) MACRO(int16) MACRO(int32) MACRO(int64)
#define EACH_FLOAT_TYPE(MACRO) MACRO(float32) MACRO(float64)
#endif
#define EACH_NUMBER_TYPE(MACRO) EACH_INT_TYPE(MACRO) EACH_FLOAT_TYPE(MACRO)

// note: loop unrolling macros assume N!=0
// btw this may cause alignment problems when 8 does not divide N
#define UNROLL_8(MACRO,N,PTR,ARGS...) \
	int n__=(-N)&7; PTR-=n__; N+=n__; \
	switch (n__) { start: \
		case 0:MACRO(0); case 1:MACRO(1); case 2:MACRO(2); case 3:MACRO(3); \
		case 4:MACRO(4); case 5:MACRO(5); case 6:MACRO(6); case 7:MACRO(7); \
		PTR+=8; N-=8; ARGS; if (N) goto start; }
#define UNROLL_4(MACRO,N,PTR,ARGS...) \
	int n__=(-N)&3; PTR-=n__; N+=n__; \
	switch (n__) { start: \
		case 0:MACRO(0); case 1:MACRO(1); case 2:MACRO(2); case 3:MACRO(3); \
		PTR+=4; N-=4; ARGS; if (N) goto start; }

class Barf {
public:
  char *text;
  Barf(const char *s, ...);
  Barf(const char *file, int line, const char *func, const char *s, ...);
  ~Barf() {free(text);}
};

#endif /* __GRIDFLOW2_H */
