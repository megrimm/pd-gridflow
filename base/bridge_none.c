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

#include "grid.h"
#include <stdarg.h>

void FObject_mark (VALUE *$) {}
void FObject_sweep (VALUE *$) {}

VALUE FObject_send_out_2(int argc, VALUE *argv, VALUE $) {
	return Qnil;
}

VALUE FObject_s_install(VALUE $, VALUE name, VALUE inlets, VALUE outlets) {
	struct { int inlets, outlets; } kludge;
	VALUE name2;
	if (SYMBOL_P(name)) {
		name2 = rb_funcall(name,rb_intern("dup"),0);
	} else if (TYPE(name) == T_STRING) {
		name2 = rb_funcall(name,rb_intern("to_str"),0);
	} else {
		EARG("expect symbol or string");
	}
	whine("class will be called: %s",RSTRING(name2)->ptr);
	kludge.inlets = NUM2INT(inlets);
	if (kludge.inlets<0 || kludge.inlets>9) EARG("...");
	kludge.outlets = NUM2INT(outlets);
	if (kludge.outlets<0 || kludge.outlets>9) EARG("...");
	rb_ivar_set($,rb_intern("@inlets"),inlets);
	rb_ivar_set($,rb_intern("@outlets"),outlets);
	rb_ivar_set($,rb_intern("@foreign_name"),name2);
	return Qnil;
}

void gf_install_bridge(void) {
	rb_define_singleton_method(GridFlow_module, "post_string", gf_post_string, 1);
}

void post(const char *fmt, ...) {
	va_list args;
	va_start(args,fmt);
	vfprintf(stderr,fmt,args);
}
