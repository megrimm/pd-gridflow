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

BuiltinSymbols bsym;
Ruby mGridFlow; /* not the same as jMax's gridflow_module */
Ruby cFObject;

int gf_security = 1;

static void default_post(const char *fmt, ...) {
	va_list args;
	va_start(args,fmt);
	vfprintf(stderr,fmt,args);
	va_end(args);
}

Ruby bridge_whatever_default (int argc, Ruby *argv, Ruby rself) {
	RAISE("sorry, not available in this bridge");
}

GFBridge gf_bridge_default = {
	name: 0,
	send_out: 0,
	class_install: 0,
	post: default_post,
	post_does_ln: false,
	clock_tick: 10.0,
	whatever: bridge_whatever_default,
};

GFBridge *gf_bridge = &gf_bridge_default;

/* ---------------------------------------------------------------- */
/* GridFlow::FObject */

Ruby rb_ary_fetch(Ruby rself, int i) {
	Ruby argv[] = { INT2NUM(i) };
	return rb_ary_aref(COUNT(argv),argv,rself);
}

const char *rb_sym_name(Ruby sym) {return rb_id2name(SYM2ID(sym));}

static void FObject_mark (void *z) {
/*
	FObject *self = (FObject *)z;
//	fprintf(stderr,"marking FObject c++=%p ruby=%p\n",self,self->peer);
	self->mark();
*/
}

static int object_count=0;

static void FObject_free (void *foo) {
	FObject *self = (FObject *)foo;
	//gfpost("FObject_free: %08x",(int)self);
	self->check_magic();
	if (!self->rself) {
		fprintf(stderr,"attempt to free object that has no rself\n");
		abort();
	}
	self->rself = 0; /* paranoia */
	delete self;
	/* a silly bug was on this line before. watch out. */
//	object_count -= 1; fprintf(stderr,"object_count=%d\n",object_count);
}

static void FObject_prepare_message(int &argc, Ruby *&argv, Ruby &sym) {
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
	} else if (argc==1 && TYPE(*argv)==T_ARRAY) {
		sym = SYM(list);
		argc = rb_ary_len(*argv);
		argv = rb_ary_ptr(*argv);
	} else {
		RAISE("bad message: argc=%d; argv[0]=%s",argc,
			argc ? rb_str_ptr(rb_inspect(argv[0])) : "");
	}
}

METHOD3(FObject,send_in) {
	ENTER(this);
	Ruby sym;

	if (argc<1) RAISE("not enough args");
	int inlet = INT(*argv);
	if (inlet<0 || inlet>9 /*|| inlet>real_inlet_max*/)
		RAISE("invalid inlet number");
	argc--, argv++;

	Ruby foo;
	if (argc==1 && TYPE(argv[0])==T_STRING /* && argv[0] =~ / / */) {
		foo = rb_funcall(mGridFlow,SI(parse),1,argv[0]);
		argc = rb_ary_len(foo);
		argv = rb_ary_ptr(foo);
	}

	FObject_prepare_message(argc,argv,sym);

	if (rb_const_get(mGridFlow,SI(@verbose))==Qtrue) {
		/* GridFlow.post "%s",m.inspect if GridFlow.verbose */
	}

	char buf[256];
	sprintf(buf,"_%d_%s",inlet,rb_sym_name(sym));
	rb_funcall2(rself,rb_intern(buf),argc,argv);

	LEAVE(this);
	return Qnil;
}

METHOD3(FObject,send_out) {
	Ruby sym;

	if (argc<1) RAISE("not enough args");
	int outlet = INT(*argv);
	if (outlet<0 || outlet>9 /*|| outlet>real_outlet_max*/)
		RAISE("invalid outlet number");
	argc--, argv++;

	FObject_prepare_message(argc,argv,sym);
	Ruby noutlets2 = rb_ivar_get(rb_obj_class(rself),SYM2ID(SYM(@noutlets)));
	if (TYPE(noutlets2)!=T_FIXNUM) {
		IEVAL(rself,"STDERR.puts inspect");
		RAISE("don't know how many outlets this has");
	}
	int noutlets = INT(noutlets2);
	if (outlet<0 || outlet>=noutlets) RAISE("outlet %d does not exist",outlet);

	LEAVE(this);

	if (gf_bridge->send_out && bself)
		gf_bridge->send_out(argc,argv,sym,outlet,rself);
	Ruby ary = rb_ivar_defined(rself,SYM2ID(bsym.iv_outlets)) ?
		rb_ivar_get(rself,SYM2ID(bsym.iv_outlets)) : Qnil;
	if (ary==Qnil) goto end;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	ary = rb_ary_fetch(ary,outlet);
	if (ary==Qnil) goto end;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	{
	int n = rb_ary_len(ary);
	for (int i=0; i<n; i++) {
		Ruby conn = rb_ary_fetch(ary,i);
		Ruby rec = rb_ary_fetch(conn,0);
		int inl = INT(rb_ary_fetch(conn,1));

		Ruby argv2[argc+2];
		for (int i=0; i<argc; i++) argv2[2+i] = argv[i];
		argv2[0] = INT2NUM(inl);
		argv2[1] = sym;
		rb_funcall2(rec,SI(send_in),argc+2,argv2);
	}}
end:
	ENTER(this);
	return Qnil;
}

Ruby FObject_s_new(Ruby argc, Ruby *argv, Ruby qlass) {
	Ruby allocator = rb_ivar_defined(qlass,SI(@allocator)) ?
		rb_ivar_get(qlass,SI(@allocator)) : Qnil;
	/* !@#$ GridObject is in FObject constructor (ugly) */
	GridObject *self;
	if (allocator==Qnil) {
		/* this is a pure-ruby FObject/GridObject */
		self = new GridObject;
	} else {
		/* this is a C++ FObject/GridObject */
		self = (GridObject *)((void*(*)())FIX2PTR(void,allocator))();
	}
	Ruby keep = rb_ivar_get(mGridFlow, SI(@fobjects_set));
	self->bself = 0;
	Ruby rself = Data_Wrap_Struct(qlass, FObject_mark, FObject_free, self);
	self->rself = rself;
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
	if (gf_bridge->class_install)
		gf_bridge->class_install(rself,RSTRING(name2)->ptr);
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
	return rb_funcall(
		rb_funcall(UINT2NUM((uint32)(val>>32)),SI(<<),1,INT2FIX(32)),
		SI(+),1,UINT2NUM((uint32)val));
}

/* end */

METHOD3(FObject,profiler_cumul_get) {
	return ull2num(profiler_cumul);
}

METHOD3(FObject,profiler_cumul_set) {
	if (argc<1) RAISE("");
	profiler_cumul = num2ull(argv[0]);
	return argv[0];
}

METHOD3(FObject,del) {
	Ruby keep = rb_ivar_get(mGridFlow, SI(@fobjects_set));
	rb_funcall(keep,SI(delete),1,rself);
	return Qnil;
}

const char *FObject::info() {
	if (!this) return "(nil GridObject!?)";
	Ruby z = rb_funcall(this->rself,SI(args),0);
/*	if (TYPE(z)==T_ARRAY) z = rb_funcall(z,SI(inspect),0); */
	if (z==Qnil) return "(nil args!?)";
	return rb_str_ptr(z);
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
	if (argc==2 && argv[1]!=Qnil && TYPE(argv[1])!=T_STRING) RAISE("bad args");
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
	if (argc==2 && argv[1]!=Qnil && TYPE(argv[1])!=T_STRING) RAISE("bad args");
	int n = rb_str_len(argv[0]) / bytes;
	Pt<uint8> in = Pt<uint8>((uint8 *)rb_str_ptr(argv[0]),rb_str_len(argv[0]));
	int bytes2 = n*size*sizeof(int32);
	Ruby out = argc==2 ? rb_str_resize(argv[1],bytes2) : rb_str_new("",bytes2);
	rb_str_modify(out);
	unpack(n,Pt<uint8>((uint8 *)in,bytes2),Pt<int32>((int32 *)rb_str_ptr(out),n));
	return out;
}

void BitPacking_free (void *foo) {
	BitPacking *self = (BitPacking *)foo;
	//gfpost("BitPacking_free: %08x",(int)self);
	self->check_magic();
	delete self;
}

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

GRCLASS(BitPacking,LIST(),
	DECL(BitPacking,initialize),
	DECL(BitPacking,pack2),
	DECL(BitPacking,unpack2))
{}

/* this is the entry point for all of the above */

/* **************************************************************** */
/* Procs of somewhat general utility */

NumberTypeIndex NumberTypeIndex_find (Ruby sym) {
	if (TYPE(sym)!=T_SYMBOL) RAISE("expected symbol");
	if (sym==SYM(uint8)) return uint8_type_i;
	if (sym==SYM(int16)) return int16_type_i;
	if (sym==SYM(int32)) return int32_type_i;
	if (sym==SYM(float32)) return float32_type_i;
	RAISE("unknown element type \"%s\"", rb_sym_name(sym));
}

Ruby GridFlow_clock_tick (Ruby rself) {
	return rb_float_new(gf_bridge->clock_tick);
}

Ruby GridFlow_clock_tick_set (Ruby rself, Ruby tick) {
	if (TYPE(tick)!=T_FLOAT) RAISE("expecting Float");
	gf_bridge->clock_tick = RFLOAT(tick)->value;
	return tick;
}

Ruby GridFlow_security_set (Ruby rself, Ruby level) {
	int security = INT(level);
	if (security<0 || security>4) RAISE("expecting 0..4");
	gf_security = security;
	return INT2NUM(gf_security);
}

Ruby GridFlow_security (Ruby rself) {
	return INT2NUM(gf_security);
}

Ruby GridFlow_rdtsc (Ruby rself) { return ull2num(rdtsc()); }

Ruby GridFlow_bridge_name (Ruby rself) {
	return gf_bridge->name ? rb_str_new2(gf_bridge->name) : Qnil;
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
	gf_bridge->post(gf_bridge->post_does_ln?"%s":"%s\n",p);
	return Qnil;
}

void define_many_methods(Ruby rself, int n, MethodDecl *methods) {
	for (int i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		const char *buf =
			strcmp(md->selector,"del")==0 ? "delete" : md->selector;
		rb_define_method(rself,buf,(RMethod)md->method,-1);
		rb_enable_super(rself,buf);
	}
}

Ruby fclass_install(FClass *fc, Ruby super) {
	if (!super) super = cGridObject;
	Ruby rself = rb_define_class_under(mGridFlow, fc->name, super);
	Ruby handlers = rb_ary_new();
	rb_ivar_set(rself,SI(@handlers),handlers);
	for (int i=0; i<fc->handlersn; i++)
		rb_ary_push(handlers,PTR2FIX(&fc->handlers[i]));
	define_many_methods(rself,fc->methodsn,fc->methods);
	rb_ivar_set(rself,SI(@allocator),PTR2FIX(fc->allocator));
	if (fc->startup) fc->startup(rself);
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

typedef void (*Callback)(void*);
static Ruby GridFlow_exec (Ruby rself, Ruby data, Ruby func) {
	void *data2 = FIX2PTR(void,data);
	Callback func2 = (Callback) FIX2PTR(void,func);
	func2(data2);
	return Qnil;
}

static Ruby GridFlow_handle_braces(Ruby rself, Ruby argv) {
	int stack[16];
	int stackn=0;
	Ruby *av = rb_ary_ptr(argv);
	int ac = rb_ary_len(argv);
	int j=0;
	for (int i=0; i<ac; i++) {
		if (av[i]==bsym._lbrace || av[i]==bsym._lparen) {
			if (stackn==16) RAISE("too many nested lists (>16)");
			stack[stackn++]=j;
		} else if (av[i]==bsym._rbrace || av[i]==bsym._rparen) {
			if (!stackn) RAISE("unbalanced '}' or ')'",av[i]);
			Ruby a2 = rb_ary_new();
			int j2 = stack[--stackn];
			for (int k=j2; k<j; k++) rb_ary_push(a2,av[k]);
			j=j2;
			av[j++] = a2;
		} else {
			av[j++]=av[i];
		}
	}
	if (stackn) RAISE("unbalanced '{' or '('");
	RARRAY(argv)->len = j;
	return rself;
}

/* ---------------------------------------------------------------- */

static uint64 memcpy_calls = 0;
static uint64 memcpy_bytes = 0;
Ruby GridFlow_memcpy_calls (Ruby rself) { return ull2num(memcpy_calls); }
Ruby GridFlow_memcpy_bytes (Ruby rself) { return ull2num(memcpy_bytes); }
Ruby GridFlow_profiler_reset2 (Ruby rself) {
	memcpy_calls = 0;
	memcpy_bytes = 0;
	return Qnil;
}

void gfmemcopy(uint8 *out, const uint8 *in, int n) {
	memcpy_calls++;
	memcpy_bytes+=n;
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

MethodDecl FObject_methods[] = {
	DECL(FObject,profiler_cumul_get),
	DECL(FObject,profiler_cumul_set),
	DECL(FObject,send_out),
	DECL(FObject,send_in),
	DECL(FObject,del),
};

/* Ruby's entrypoint. */
void Init_gridflow () {
	signal(11,SIG_DFL);
//	setenv("RUBY_VERBOSE_GC","yes",1);

#define FOO(_sym_,_name_) bsym._sym_ = ID2SYM(rb_intern(_name_));
BUILTIN_SYMBOLS(FOO)
#undef FOO

	mGridFlow = rb_define_module("GridFlow");
	SDEF2("clock_tick",GridFlow_clock_tick,0);
	SDEF2("clock_tick=",GridFlow_clock_tick_set,1);
	SDEF2("security",GridFlow_security,0);
	SDEF2("security=",GridFlow_security_set,1);
	SDEF2("exec",GridFlow_exec,2);
	SDEF2("post_string",gf_post_string,1);
	SDEF2("rdtsc",GridFlow_rdtsc,0);
	SDEF2("profiler_reset2",GridFlow_profiler_reset2,0);
	SDEF2("memcpy_calls",GridFlow_memcpy_calls,0);
	SDEF2("memcpy_bytes",GridFlow_memcpy_bytes,0);
	SDEF2("bridge_name",GridFlow_bridge_name,0);
	SDEF2("handle_braces!",GridFlow_handle_braces,1);

	rb_ivar_set(mGridFlow, SI(@fobjects_set), rb_hash_new());
	rb_ivar_set(mGridFlow, SI(@fclasses_set), rb_hash_new());
	rb_ivar_set(mGridFlow, SI(@bsym), PTR2FIX(&bsym));
	rb_define_const(mGridFlow, "GF_VERSION", rb_str_new2(GF_VERSION));
	rb_define_const(mGridFlow, "GF_COMPILE_TIME", rb_str_new2(GF_COMPILE_TIME));
	cPointer = rb_define_class_under(mGridFlow, "Pointer", rb_cObject);
	DEF(Pointer, get, 0);

	cFObject = rb_define_class_under(mGridFlow, "FObject", rb_cObject);
	define_many_methods(cFObject,COUNT(FObject_methods),FObject_methods);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, -1);

	Ruby cData = EVAL("Data");

	ID gb = SI(@gf_bridge);
	if (rb_ivar_defined(cData,gb)) {
		gf_bridge = FIX2PTR(GFBridge,rb_ivar_get(cData,gb));
	}

	ID gbi = SI(gf_bridge_init);
	if (rb_respond_to(rb_cData,gbi)) rb_funcall(rb_cData,gbi,0);

	if (!gf_bridge->whatever) gf_bridge->whatever = bridge_whatever_default;
	SDEF2("whatever",gf_bridge->whatever,-1);

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

	EVAL("begin require 'gridflow/base/main.rb'\n"
		"rescue Exception => e; "
		"STDERR.puts \"can't load: #{$!}\n"
		"backtrace: #{$!.backtrace}\n"
		"$: = #{$:.inspect}\"\n end");

	EVAL("begin require 'gridflow/format/main.rb'\n"
		"rescue Exception => e; "
		"STDERR.puts \"can't load: #{$!}\n"
		"backtrace: #{$!.backtrace}\n"
		"$: = #{$:.inspect}\"\n end");

	startup_formats();

	EVAL("GridFlow.load_user_config");

#ifdef HAVE_MMX
	if (!getenv("NO_MMX")) startup_cpu();
#endif

	signal(11,SIG_DFL); /* paranoia */
}
