/* $Id$ */

#include "../c/src/grid.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DIR "../images/"
#define ADIR "/opt/mex"

void pressakey(void) {
	char buf[256];
	printf("press return to continue.\n");
	fgets(buf,256,stdin);
}

void test_dict$1(void *foo, void *k, void *v) {
	printf("%s : %s\n", Symbol_name((Symbol)k), (const char *)v);
}

void test_gen(void) {
	FObject *f0 = fts_object_new3("@for 0 64 1");
	FObject *f1 = fts_object_new3("@for 0 64 1");
	FObject *f2 = fts_object_new3("@for 1 4 1");
	FObject *t0 = fts_object_new3("@outer ^");
	FObject *t1 = fts_object_new3("@outer *");
	FObject *out = fts_object_new3("@out 256 256");
	fts_connect(f0,0,t0,0);
	fts_connect(f1,0,t0,1);
	fts_connect(t0,0,t1,0);
	fts_connect(f2,0,t1,1);
	fts_connect(t1,0,out,0);
	fts_send3(f2,0,"bang");
	fts_send3(f1,0,"bang");
	fts_send3(f0,0,"bang");
}

void test_dict(void) {
	Dict *d = Dict_new(0,0);
	Dict_put(d,(void *)(0+SYM(apples)),"oranges");
	Dict_put(d,(void *)(0+SYM(monty)),"python");
	Dict_put(d,(void *)(0+SYM(luby)),"is not engrish");
	Dict_put(d,(void *)(0+SYM(foo)),"bar");
	Dict_put(d,(void *)(0+SYM(recursive)),"n. see also: recursive");
	Dict_each(d,test_dict$1,0);
}

void test_ppm1(void) {
	FObject *in = fts_object_new3("@in");
	FObject *out = fts_object_new3("@out 256 256");
	fts_connect(in,0,out,0);
//	fts_send3(in,0,"open ppm file "DIR"/g001.ppm");
	fts_send3(in,0,"open grid file "DIR"/foo.grid");
	fts_send3(in,0,"bang");
}

void test_ppm2(void) {
	FObject *in = fts_object_new3("@in");
	FObject *pa = fts_object_new3("@convolve << + 0");
	FObject *pb = fts_object_new3("@ / 9");
	FObject *ra = fts_object_new3("@redim 3 3");
	FObject *out = fts_object_new3("@out 256 256");
	FObject *v4j = fts_object_new3("@global");
	fts_connect(in,0,pa,0);
	fts_connect(pa,0,pb,0);
	fts_connect(pb,0,out,0);
	fts_connect(ra,0,pa,1);
	fts_send3(ra,0,"0 0");
	fts_send3(out,0,"option timelog 1");
	fts_send3(in,0,"open ppm file "DIR"/teapot.ppm");
//	fts_send3(in,0,"open ppm file "DIR"/g001.ppm");
	{int i; for (i=0; i<30; i++) fts_send3(in,0,"bang");}
	fts_send3(v4j,0,"profiler_dump");
}

void test_anim(void) {
	FObject *in = fts_object_new3("@in");
/*	FObject *pa = fts_object_new3("@scale_by"); */
	FObject *out = fts_object_new3("@out 256 256");
	FObject *v4j = fts_object_new3("@global");
/*
	fts_connect(in,0,pa,0);
	fts_connect(pa,0,out,0);
*/
	fts_connect(in,0,out,0);
	fts_send3(in,0,"open ppm file "ADIR"/b.ppm.cat");
//	fts_send3(in,0,"open videodev /dev/video");
	fts_send3(in,0,"option channel 1");
	fts_send3(in,0,"option size 480 640");
	fts_send3(out,0,"option timelog 1");
	{ int i; for (i=0; i<50; i++) fts_send3(in,0,"bang"); }
	fts_send3(v4j,0,"profiler_dump");
}

void test_anim2(void) {
	FObject *in = fts_object_new3("@in");
	FObject *out = fts_object_new3("@out 256 256");
	fts_connect(in,0,out,0);
	fts_send3(in,0,"open mpeg file "
		"/home/matju/net/Animations/washington_zoom_in.mpeg");
//	printf("here...\n");
	while(1) fts_send3(in,0,"bang");
//	printf("there...\n");
}

typedef struct TestTCP {
	FObject *in1, *in2, *out;
	int toggle;
} TestTCP;

void test_tcp$1(TestTCP *$) {
	whine("tick1");
	if (GridInlet_busy(((GridObject *)$->out)->in[0])) return;
	whine("tick2");
	if ($->toggle==0) {
		fts_send3($->in1,0,"bang");
	} else {
		fts_send3($->in2,0,"bang");
	}
	whine("tick3");
	$->toggle ^= 1;
}

void test_tcp$2(TestTCP *$) {
	whine("tick");
	if (GridInlet_busy(((GridObject *)$->out)->in[0])) return;
	fts_send3($->in1,0,"bang");
}

void test_tcp(void) {
#define MYPORT "4247"
	if (fork()) {
		/* client */
		TestTCP *$ = NEW(TestTCP,1);
		whine_header = "[client] ";
		$->in1 = fts_object_new3("@in");
		$->out = fts_object_new3("@out 240 320");
		fts_connect($->in1,0,$->out,0);
		fts_send3($->out,0,"option autodraw 2");
		whine("test: waiting 2 seconds");
		sleep(2);
		fts_send3($->in1,0,"open grid tcp localhost " MYPORT);

		Dict_put(gf_timer_set,$,test_tcp$2);
		fts_send3($->in1,0,"bang");
	} else {
		/* server */
		TestTCP *$ = NEW(TestTCP,1);
		whine_header = "[server] ";
		$->in1 = fts_object_new3("@in");
		$->in2 = fts_object_new3("@in");
		$->out = fts_object_new3("@out");
		$->toggle = 0;
		fts_connect($->in1,0,$->out,0);
		fts_connect($->in2,0,$->out,0);
		fts_send3($->in1,0,"open ppm file "DIR"/r001.ppm");
		fts_send3($->in2,0,"open ppm file "DIR"/b001.ppm");
		fts_send3($->out,0,"open grid tcpserver " MYPORT);
		fts_send3($->out,0,"option type uint8");
		whine("now setting up timer");
		Dict_put(gf_timer_set,$,test_tcp$1);
		Timer_loop();
	}		
}

void test_foo(void) {
	FObject *in  = fts_object_new3("@in");
	FObject *out = fts_object_new3("@out");
	fts_send3(out,0,"open videodev /dev/video0");
}

int main(int argc, char **argv) {
	Dict *tests = Dict_new((CompFunc)strcmp,HashFunc_string);
	int i;
	gf_init_standalone();
#define TEST(_x_) Dict_put(tests,#_x_,test_##_x_);
	TEST(dict)
	TEST(ppm1)
	TEST(ppm2)
	TEST(anim)
	TEST(anim2)
	TEST(foo)
	TEST(tcp)
	TEST(gen)

	{
		const char *name = argc<2 ? "tcp" : argv[1];
		void *test = Dict_get(tests,name);
		printf("%s\n",name);
		if (!test) { printf("test not found\n"); exit(1); }
		((void (*)(void))test)();
	}

	Timer_loop();
/*	{int i; for(i=0;;i++) printf("%3d = %s\n", i, Symbol_name(i)); } */
	return 0;
}
