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

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "grid.h"

#include "../config.h"
#include <assert.h>
#include <limits.h>

/* number of #send_out calls allowed at once (not used yet) */
#define GF_STACK_DEPTH 1024
struct GFStackFrame {
	GridObject *self;
};
GFStackFrame gf_stack[GF_STACK_DEPTH];
int gf_stack_n = 0;

Ruby mGridFlow; /* not the same as jMax's gridflow_module */
Ruby cFObject;
static Ruby sym_outlets=0;

static void default_post(const char *fmt, ...) {
	va_list args;
	va_start(args,fmt);
	vfprintf(stderr,fmt,args);
	va_end(args);
}

GFBridge gf_bridge = {
	send_out: 0,
	class_install: 0,
	post: default_post,
	post_does_ln: false,
	clock_tick: 10.0,
};

/* ---------------------------------------------------------------- */
/* GridFlow::FObject */

Ruby rb_ary_fetch(Ruby rself, int i) {
	Ruby argv[] = { INT2NUM(i) };
	return rb_ary_aref(COUNT(argv),argv,rself);
}

const char *rb_sym_name(Ruby sym) {return rb_id2name(SYM2ID(sym));}

static void FObject_mark (void *z) {
/*
	GridObject *self = (GridObject *)z;
//	fprintf(stderr,"marking FObject c++=%p ruby=%p\n",self,self->peer);
	self->mark();
*/
}

static int object_count=0;

static void FObject_free (void *foo) {
	GridObject *self = (GridObject *)foo;
//	fprintf(stderr,"Say farewell to %08x\n",(int)self);
	if (!self->rself) {
		fprintf(stderr,"attempt to free object that has no rself\n");
		abort();
	}
	self->rself = 0; /* paranoia */
	delete self;
	/* a silly bug was on this line before. watch out. */
//	object_count -= 1; fprintf(stderr,"object_count=%d\n",object_count);
}

static void FObject_prepare_message(int &argc, Ruby *&argv, Ruby &sym, int &outlet) {
	if (argc<1) RAISE("not enough args");
	outlet = INT(*argv);
	if (outlet<0 || outlet>9 /*|| outlet>real_outlet_max*/)
		RAISE("invalid outlet number");
	argc--, argv++;
	if (argc<1) {
		sym = SYM(bang);
	} else if (argc>1 && !SYMBOL_P(*argv)) {
		sym = SYM(list);
	} else if (INTEGER_P(*argv)) {
		sym = SYM(int);
	} else if (FLOAT_P(*argv)) {
		sym = SYM(float);
	} else if (SYMBOL_P(*argv)) {
		sym = *argv;
		argc--, argv++;
	} else {
		RAISE("bad message");
	}
}

Ruby FObject_send_out(int argc, Ruby *argv, Ruby rself) {
	Ruby sym;
	int outlet;
	FObject_prepare_message(argc,argv,sym,outlet);
	Ruby noutlets2 = rb_ivar_get(rb_obj_class(rself),SYM2ID(SYM(@noutlets)));
	if (TYPE(noutlets2)!=T_FIXNUM) {
		IEVAL(rself,"STDERR.puts inspect");
		RAISE("don't know how many outlets this has");
	}
	int noutlets = INT(noutlets2);
	if (outlet<0 || outlet>=noutlets) RAISE("outlet %d does not exist",outlet);
	if (gf_bridge.send_out && FObject_peer(rself))
		gf_bridge.send_out(argc,argv,sym,outlet,rself);
	Ruby ary = rb_ivar_defined(rself,SYM2ID(sym_outlets)) ?
		rb_ivar_get(rself,SYM2ID(sym_outlets)) : Qnil;
	if (ary==Qnil) return Qnil;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	ary = rb_ary_fetch(ary,outlet);
	if (ary==Qnil) return Qnil;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	int n = RARRAY(ary)->len;
	for (int i=0; i<n; i++) {
		Ruby conn = rb_ary_fetch(ary,i);
		Ruby rec = rb_ary_fetch(conn,0);
		int inl = INT(rb_ary_fetch(conn,1));
		char buf[256];
		sprintf(buf,"_%d_%s",inl,rb_sym_name(sym));
		rb_funcall2(rec,rb_intern(buf),argc,argv);
	}
	return Qnil;
}

Ruby FObject_delete(Ruby argc, Ruby *argv, Ruby rself) {
	Ruby keep = rb_ivar_get(mGridFlow, SI(@fobjects_set));
	rb_funcall(keep,SI(delete),1,rself);
//	fprintf(stderr,"del: @fobjects_set.size: %ld\n",INT(rb_funcall(keep,SI(size),0)));
	return Qnil;
}

Ruby FObject_s_new(Ruby argc, Ruby *argv, Ruby qlass) {
	Ruby gc2 = rb_ivar_defined(qlass,SI(@grid_class)) ?
		rb_ivar_get(qlass,SI(@grid_class)) : Qnil;
	GridClass *gc = (gc2==Qnil ? 0 : FIX2PTR(GridClass,gc2));
	Ruby keep = rb_ivar_get(mGridFlow, SI(@fobjects_set));
	GridObject *c_peer = gc ? (GridObject *)gc->allocate() : new GridObject();
	c_peer->foreign_peer = 0;
	Ruby rself = Data_Wrap_Struct(qlass, FObject_mark, FObject_free, c_peer);
	c_peer->rself = rself;
	c_peer->grid_class = gc;
	rb_hash_aset(keep,rself,Qtrue); /* prevent sweeping */
	rb_funcall2(rself,SI(initialize),argc,argv);
//	object_count += 1; fprintf(stderr,"object_count=%d\n",object_count);
	return rself;
}

Ruby FObject_s_install(Ruby rself, Ruby name, Ruby inlets2, Ruby outlets2) {
	int inlets, outlets;
	Ruby name2;
	if (SYMBOL_P(name)) {
		name2 = rb_funcall(name,SI(to_str),0);
	} else if (TYPE(name) == T_STRING) {
		name2 = rb_funcall(name,SI(dup),0);
	} else {
		RAISE("expect symbol or string");
	}
	inlets  =  INT(inlets2); if ( inlets<0 ||  inlets>9) RAISE("...");
	outlets = INT(outlets2); if (outlets<0 || outlets>9) RAISE("...");
	rb_ivar_set(rself,SI(@ninlets),INT2NUM(inlets));
	rb_ivar_set(rself,SI(@noutlets),INT2NUM(outlets));
	rb_ivar_set(rself,SI(@foreign_name),name2);
	rb_hash_aset(rb_ivar_get(mGridFlow,SI(@fclasses_set)),
		name2, rself);
	if (gf_bridge.class_install)
		gf_bridge.class_install(rself,RSTRING(name2)->ptr);
	return Qnil;
}

/* begin Ruby 1.6 compatibility */

static uint64 num2ull(Ruby val) {
    if (FIXNUM_P(val)) return (uint64)FIX2INT(val);
	if (TYPE(val)!=T_BIGNUM) RAISE("type error");
	uint64 v =
		(uint64)NUM2UINT(rb_funcall(val,SI(>>),1,INT2FIX(32))) << 32;
	return v + NUM2UINT(rb_funcall(val,SI(&),1,UINT2NUM(0xffffffff)));
}

static Ruby ull2num(uint64 val) {
	return rb_funcall(rb_funcall(UINT2NUM((uint32)val),SI(<<),1,INT2FIX(32)),
		SI(+),1,UINT2NUM(val>>32));
}

/* end */

Ruby FObject_profiler_cumul(Ruby rself) {
	DGS(GridObject);
	return ull2num(self->profiler_cumul);
}

Ruby FObject_profiler_cumul_set(Ruby rself, Ruby arg) {
	DGS(GridObject);
	self->profiler_cumul = num2ull(arg);
	return arg;
}

/* ---------------------------------------------------------------- */
/* C++<->Ruby bridge for classes/functions in base/number.c */

static Ruby String_swap32_f (Ruby rself) {
	int n = rb_str_len(rself)/4;
	swap32(n,Pt<uint32>((uint32 *)rb_str_ptr(rself),n));
	return rself;
}

static Ruby String_swap16_f (Ruby rself) {
	int n = rb_str_len(rself)/2;
	swap16(n,Pt<uint16>((uint16 *)rb_str_ptr(rself),n));
	return rself;
}

/* **************************************************************** */

METHOD3(BitPacking,initialize) {
	return Qnil;
}

METHOD3(BitPacking,pack2) {
	if (argc!=1 || TYPE(argv[0])!=T_STRING) RAISE("bad args");
	if (argc==2 && TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int n = rb_str_len(argv[0]) / sizeof(int32) / size;
	Pt<int32> in = Pt<int32>((int32 *)rb_str_ptr(argv[0]),rb_str_len(argv[0]));
	int bytes2 = n*bytes;
	Ruby out = argc==2 ? rb_str_resize(argv[1],bytes2) : rb_str_new("",bytes2);
	rb_str_modify(out);
	pack(n,Pt<int32>(in,n),Pt<uint8>((uint8 *)rb_str_ptr(out),bytes2));
	return out;
}

METHOD3(BitPacking,unpack2) {
	if (argc<1 || argc>2 || TYPE(argv[0])!=T_STRING) RAISE("bad args");
	if (argc==2 && TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int n = rb_str_len(argv[0]) / bytes;
	Pt<uint8> in = Pt<uint8>((uint8 *)rb_str_ptr(argv[0]),rb_str_len(argv[0]));
	int bytes2 = n*size*sizeof(int32);
	Ruby out = argc==2 ? rb_str_resize(argv[1],bytes2) : rb_str_new("",bytes2);
	rb_str_modify(out);
	memset(rb_str_ptr(out),255,n*4*size);
	unpack(n,Pt<uint8>((uint8 *)in,bytes2),Pt<int32>((int32 *)rb_str_ptr(out),n));
//	memcpy(rb_str_ptr(out),in,n);
	return out;
}

void BitPacking_free (void *foo) {}

static Ruby BitPacking_s_new(Ruby argc, Ruby *argv, Ruby qlass) {
	Ruby keep = rb_ivar_get(mGridFlow, rb_intern("@fobjects_set"));
	BitPacking *c_peer;

	if (argc!=3) RAISE("bad args");
	if (TYPE(argv[2])!=T_ARRAY) RAISE("bad mask");

	int endian = INT(argv[0]);
	int bytes = INT(argv[1]);
	Ruby *masks = rb_ary_ptr(argv[2]);
	uint32 masks2[4];
	int size = rb_ary_len(argv[2]);
	if (size<1) RAISE("not enough masks");
	if (size>4) RAISE("too many masks (%d)",size);
	for (int i=0; i<size; i++) masks2[i] = NUM2UINT(masks[i]);
	c_peer = new BitPacking(endian,bytes,size,masks2);
	
	Ruby rself = Data_Wrap_Struct(qlass, 0, BitPacking_free, c_peer);
	rb_hash_aset(keep,rself,Qtrue); /* prevent sweeping (leak) */
	rb_funcall2(rself,SI(initialize),argc,argv);
	return rself;
}

GRCLASS(BitPacking,"",inlets:0,outlets:0,startup:0,
LIST(),
	DECL(BitPacking,initialize),
	DECL(BitPacking,pack2),
	DECL(BitPacking,unpack2));



/* ---------------------------------------------------------------- */
/* The setup part */

#define DECL_SYM2(_sym_) Ruby sym_##_sym_ = 0xDeadBeef;
#define DEF_SYM(_sym_) sym_##_sym_ = SYM(_sym_);

DECL_SYM2(grid)
DECL_SYM2(bang)
DECL_SYM2(int)
DECL_SYM2(list)

typedef void (*Callback)(void*);
static Ruby GridFlow_exec (Ruby rself, Ruby data, Ruby func) {
	void *data2 = FIX2PTR(void,data);
	Callback func2 = (Callback) FIX2PTR(void,func);
	func2(data2);
	return Qnil;
}

/* this is the entry point for all of the above */

/* **************************************************************** */
/* Procs of somewhat general utility */

void define_many_methods(Ruby rself, int n, MethodDecl *methods) {
	for (int i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		const char *buf =
			strcmp(md->selector,"del")==0 ? "delete" :
			md->selector;
		rb_define_method(rself,buf,(RMethod)md->method,-1);
		rb_enable_super(rself,buf);
	}
}

NumberTypeIndex NumberType_find (Ruby sym) {
	if (TYPE(sym)!=T_SYMBOL) RAISE("expected symbol");
	if (sym==SYM(uint8)) return uint8_type_i;
	if (sym==SYM(int16)) return int16_type_i;
	if (sym==SYM(int32)) return int32_type_i;
	if (sym==SYM(float32)) return float32_type_i;
	RAISE("unknown element type \"%s\"", rb_sym_name(sym));
}

Ruby GridFlow_clock_tick (Ruby rself) {
	return rb_float_new(gf_bridge.clock_tick);
}

Ruby GridFlow_clock_tick_set (Ruby rself, Ruby tick) {
	if (TYPE(tick)!=T_FLOAT) RAISE("expecting Float");
	gf_bridge.clock_tick = RFLOAT(tick)->value;
	return tick;
}

#ifndef ULL2NUM
#define ULL2NUM(x) \
	rb_funcall(rb_funcall(UINT2NUM((x)>>32),SI(<<),1,INT2NUM(32)), \
		SI(+),1,UINT2NUM(((uint32)x)))
#endif

Ruby GridFlow_rdtsc (Ruby rself) {
	return ULL2NUM(rdtsc());
}

void MainLoop_add(void *data, void (*func)(void*)) {
	rb_funcall(EVAL("$tasks"),SI([]=), 2, PTR2FIX(data), PTR2FIX(func));
}

void MainLoop_remove(void *data) {
	rb_funcall(EVAL("$tasks"),SI(delete), 1, PTR2FIX(data));
}

void gfpost(const char *fmt, ...) {
	va_list args;
	int length;
	va_start(args,fmt);
	const int n=2000;
	char post_s[n];
	length = vsnprintf(post_s,n,fmt,args);
	if (length<0 || length>=n) sprintf(post_s+n-6,"[...]"); /* safety */
	va_end(args);
	if (rb_respond_to(mGridFlow,SI(gfpost2))) {
		rb_funcall(mGridFlow,SI(gfpost2),2,
			rb_str_new2(fmt),rb_str_new2(post_s));
	} else {
		default_post("%s",post_s);
	}
}

Ruby gf_post_string (Ruby rself, Ruby s) {
	if (TYPE(s) != T_STRING) RAISE("not a String");
	char *p = rb_str_ptr(s);
//	bool has_ln = p[rb_str_len(p)-1]=='\n';
	gf_bridge.post(gf_bridge.post_does_ln?"%s":"%s\n",p);
	return Qnil;
}

Ruby ruby_c_install(GridClass *gc, Ruby super) {
	Ruby rself = rb_define_class_under(mGridFlow, gc->name, super);
	rb_ivar_set(rself,SI(@grid_class),PTR2FIX(gc));
	define_many_methods(rself,gc->methodsn,gc->methods);
//remember to take care of delete
	rb_funcall(rself,SI(install),3,
		rb_str_new2(gc->jname),
		INT2NUM(gc->inlets),
		INT2NUM(gc->outlets));
	GridObject_conf_class(rself,gc);
	gc->rubyclass = rself;
	if (gc->startup) gc->startup(gc);
	return Qnil;
}

/*
  The following code is not used. It was supposed to exist for passing
  buffer-pointers and function-pointers of C++, but in the end I can't
  make this work with the limitations of PureData.
*/
Ruby cPointer;

Ruby Pointer_new (void *ptr) {
	return Data_Wrap_Struct(rb_eval_string("GridFlow::Pointer"), 0, 0, ptr);
}

void *Pointer_get (Ruby self) {
	void *p;
	Data_Get_Struct(self,void *,p);
	return p;
}

/* ---------------------------------------------------------------- */

void gfmemcopy(uint8 *out, const uint8 *in, int n) {
	while (n>16) {
		((int32*)out)[0] = ((int32*)in)[0];
		((int32*)out)[1] = ((int32*)in)[1];
		((int32*)out)[2] = ((int32*)in)[2];
		((int32*)out)[3] = ((int32*)in)[3];
		in+=16; out+=16; n-=16;
	}
	while (n>4) { *(int32*)out = *(int32*)in; in+=4; out+=4; n-=4; }
	while (n) { *out = *in; in++; out++; n--; }
}

/* ---------------------------------------------------------------- */

#include <signal.h>

void startup_number();
void startup_grid();
void startup_flow_objects();
void startup_formats();
void startup_cpu();

#define SDEF2(a,b,c) rb_define_singleton_method(mGridFlow,a,(RMethod)b,c)

/* Ruby's entrypoint. */
void Init_gridflow () {
	signal(11,SIG_DFL);
//	setenv("RUBY_VERBOSE_GC","yes",1);

	DEF_SYM(grid);
	DEF_SYM(bang);
	DEF_SYM(int);
	DEF_SYM(list);
	sym_outlets=SYM(@outlets);

	mGridFlow = rb_define_module("GridFlow");
	SDEF2("clock_tick",GridFlow_clock_tick,0);
	SDEF2("clock_tick=",GridFlow_clock_tick_set,1);
	SDEF2("exec",GridFlow_exec,2);
	SDEF2("post_string",gf_post_string,1);
	SDEF2("rdtsc",GridFlow_rdtsc,0);

	rb_ivar_set(mGridFlow, SI(@fobjects_set), rb_hash_new());
	rb_ivar_set(mGridFlow, SI(@fclasses_set), rb_hash_new());
	rb_define_const(mGridFlow, "GF_VERSION", rb_str_new2(GF_VERSION));
	rb_define_const(mGridFlow, "GF_COMPILE_TIME", rb_str_new2(GF_COMPILE_TIME));
	cPointer = rb_define_class_under(mGridFlow, "Pointer", rb_cObject);
	DEF(Pointer, get, 0);

	cFObject = rb_define_class_under(mGridFlow, "FObject", rb_cObject);
	DEF(FObject, send_out, -1);
	DEF(FObject, delete, -1);
	DEF(FObject, profiler_cumul, 0);
	DEF2(FObject, "profiler_cumul=", profiler_cumul_set, 1);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, -1);

	Ruby rb_cData = EVAL("Data");
	ID bi = SI(gridflow_bridge_init);
	if (rb_respond_to(rb_cData,bi)) {
		rb_funcall(rb_cData,bi,1,PTR2FIX(&gf_bridge));
	}

	Ruby cBitPacking =
		rb_define_class_under(mGridFlow, "BitPacking", rb_cObject);
	define_many_methods(cBitPacking,
		ciBitPacking.methodsn,
		ciBitPacking.methods);
	SDEF(BitPacking,new,-1);
	rb_define_method(rb_cString, "swap32!", (RMethod)String_swap32_f, 0);
	rb_define_method(rb_cString, "swap16!", (RMethod)String_swap16_f, 0);

	/* run startup of every source file */

	startup_number();
	startup_grid();
	startup_flow_objects();

	EVAL("for f in %w(gridflow/base/main.rb gridflow/format/main.rb) do "
//	EVAL("for f in %w(gridflow/base/main.rb) do "
		"require f end rescue STDERR.puts \"can't load: #{$!}\n$: = #{$:}; "
		"backtrace: #{$!.backtrace}\"");

	startup_formats();

#ifdef HAVE_MMX
	if (!getenv("NO_MMX")) startup_cpu();
#endif

	signal(11,SIG_DFL); /* paranoia */
}
