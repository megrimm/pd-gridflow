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
#ifndef __BRIDGE_C
#define __BRIDGE_C

#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>

#define rb_sym_name rb_sym_name_r4j
static const char *rb_sym_name(Ruby sym) {return rb_id2name(SYM2ID(sym));}

static BuiltinSymbols *syms;
static GFBridge *gf_bridge2;

/* can't even refer to the other mGridFlow because we don't link
   statically to the other gridflow.so */
static Ruby mGridFlow2=0;
static Ruby cPointer2=0;

static uint64 time_now() {
	struct timeval nowtv;
	gettimeofday(&nowtv,0);
	return nowtv.tv_sec * 1000000LL + nowtv.tv_usec;
}

static void count_tick () {
/*
	static uint64 start_time = time_now();
	static int count = 0;
	static int next = 10000;
	int32 duration = (time_now() - start_time) / 1000;
	if (count>=next) {
		gf_bridge2->post("GF clock ticks: %d in %d ms (%.2f ms/tick)%s",
			count, duration, duration*1.0/count,
			gf_bridge2->post_does_ln ? "" : "\n");
		next *= 2;
	}
	count++;
*/
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
	char buf[1000];
	sprintf(buf,"%s: %s",rb_class2name(rb_obj_class(ruby_errinfo)),
		rb_str_ptr(rb_funcall(ruby_errinfo,SI(to_s),0)));
	Ruby ary = rb_ary_new2(2);
	Ruby backtrace = rb_funcall(ruby_errinfo,SI(backtrace),0);
	rb_ary_push(ary,rb_str_new2(buf));
	for (int i=0; i<2 && i<rb_ary_len(backtrace); i++)
		rb_ary_push(ary,rb_funcall(backtrace,SI([]),1,INT2NUM(i)));
//	rb_ary_push(ary,rb_funcall(rb_funcall(backtrace,SI(length),0),SI(to_s),0));
	return ary;
}

static void bridge_common_init () {}

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

/* -------- This is the big hack for what Ruby can't do for itself -------- */

#ifndef STACK_GROW_DIRECTION
#define STACK_GROW_DIRECTION -1
#endif

static volatile long bogus = 0; // to force *bp to be read in memory
static sigjmp_buf rescue_segfault;
static void trap_segfault (int patate) { siglongjmp(rescue_segfault,11); }
extern "C" void Init_stack(VALUE *addr);

/* that's good enough for both Linux/PC and OSX, but not Linux/Mac, ... */
#if 0
static VALUE *localize_sysstack () {return (VALUE *) 0xBFFFFFFC;}
#endif

/* the segfault trick (by Mathieu Bouchard) */
/* may be useful on platforms where the two others fail. */
#if 0
static VALUE *localize_sysstack () {
	/* the following is a bit risky... well, not so much, but
	   there's interference with gdb/osx and this trick, and such.
	*/
	// get any stack address
	volatile long * volatile bp = (volatile long *)&bp;
	//sighandler_t old = signal(11,trap_segfault); // g++-2.95 doesn't take it
	//fprintf(stderr,"trap_segfault = %08lx\n",(uint32)trap_segfault);
	void (*oldsegv)(int) = signal(SIGSEGV,trap_segfault);
	void (*oldbus)(int)  = signal(SIGBUS, trap_segfault);
	//fprintf(stderr,"old = %08lx\n",(uint32)old);
	// read stack until segfault; segfault is redefined as a break.
	if (!sigsetjmp(rescue_segfault,0)) for (;;bp-=STACK_GROW_DIRECTION) bogus += *bp;
	// restore signal handler
	//void (*new_)(int) = signal(11,old);
	signal(SIGSEGV,SIG_DFL); // because i've had problems with segfaults being ignored.(!?)
	signal(SIGBUS, SIG_DFL); // because i've had problems with segfaults being ignored.(!?)
	//fprintf(stderr,"new = %08lx\n",(uint32)(bp+STACK_GROW_DIRECTION));
	return (VALUE *)(bp+STACK_GROW_DIRECTION);
}
#endif

/* Nobu Nokada's ways; works with GLIBC/Cygwin/Human68k */
#if 0
static VALUE *localize_sysstack () {
#if defined _WIN32 || defined __CYGWIN__
#include <windows.h>
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
    MEMORY_BASIC_INFORMATION m;
    memset(&m, 0, sizeof(m));
    VirtualQuery(&m, &m, sizeof(m));
    return STACK_UPPER((VALUE *)&m, (VALUE *)m.BaseAddress,
	(VALUE *)((char *)m.BaseAddress + m.RegionSize) - 1);
#elif defined(STACK_END_ADDRESS)
    extern void *STACK_END_ADDRESS;
    return STACK_END_ADDRESS;
#else
#error "NO STACK_END_ADDRESS ???"
#endif
}
#endif

/* using ../config.h */
#if 1
static VALUE *localize_sysstack () {
	void *bp;
	sscanf(RUBY_STACK_END,"0x%08lx",(long *)&bp);
	return (VALUE *)bp;
}
#endif

#endif /* __BRIDGE_C */
