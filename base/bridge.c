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
#ifndef __BRIDGE_C
#define __BRIDGE_C

#include <sys/time.h>

#define rb_sym_name rb_sym_name_r4j
static const char *rb_sym_name(Ruby sym) {return rb_id2name(SYM2ID(sym));}

static BuiltinSymbols *syms;
static GFBridge *gf_bridge2;

/* can't even refer to the other mGridFlow because we don't link
   statically to the other gridflow.so */
static Ruby mGridFlow2=0;
static Ruby cPointer2=0;

static void count_tick () {
	static int count = 0;
	static int next = 500;
	if (count>=next) {
		gf_bridge2->post("survived to %d clock ticks%s",count,
			gf_bridge2->post_does_ln ? "" : "\n");
		//next = (next*15+9)/10; /* next notice when 50% bigger */
		next *= 2;
	}
	count++;
}

/*
static Ruby Pointer_new (void *ptr) {
	return Data_Wrap_Struct(EVAL("GridFlow::Pointer"), 0, 0, ptr);
}

static void *Pointer_get (Ruby rself) {
	void *p;
	Data_Get_Struct(rself,void *,p);
	return p;
}
*/

static Ruby make_error_message () {
//	EVAL("STDERR.puts $!.inspect");
//	Ruby error_array = EVAL("[\"ruby #{$!.class}: #{$!}\",*($!.backtrace)]");
	Ruby error_array = EVAL("[\"ruby #{$!.class}: #{$!}\",$!.backtrace[0]]");
//	Ruby error_array = EVAL("[\"ruby #{$!.class}: #{$!}\"]");
	return error_array;
}

static void bridge_common_init () {

}

static int ninlets_of (Ruby qlass) {
	if (!rb_ivar_defined(qlass,SYM2ID(syms->iv_ninlets))) RAISE("no inlet count");
	return INT(rb_ivar_get(qlass,SYM2ID(syms->iv_ninlets)));
}

static int noutlets_of (Ruby qlass) {
	if (!rb_ivar_defined(qlass,SYM2ID(syms->iv_noutlets))) RAISE("no outlet count");
	return INT(rb_ivar_get(qlass,SYM2ID(syms->iv_noutlets)));
}

static void gf_same_version () {
	Ruby ver = EVAL("GridFlow::GF_VERSION");
	if (strcmp(rb_str_ptr(ver), GF_VERSION) != 0) {
		RAISE("GridFlow version mismatch: "
			"main library is '%s'; bridge is '%s'",
			rb_str_ptr(ver), GF_VERSION);
	}
}

#endif /* __BRIDGE_C */
