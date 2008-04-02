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

