/* $Id$ */

#include "../c/src/grid.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DIR "../../images/"

void pressakey(void) {
	char buf[256];
	printf("press return to continue.\n");
	fgets(buf,256,stdin);
}

void test_dict$1(void *foo, void *k, void *v) {
	printf("%s : %s\n", fts_symbol_name((fts_symbol_t)k), (const char *)v);
}

void test_gen(void) {
	fts_object_t *f0 = fts_object_new3("@for 0 64 1");
	fts_object_t *f1 = fts_object_new3("@for 0 64 1");
	fts_object_t *f2 = fts_object_new3("@for 1 4 1");
	fts_object_t *t0 = fts_object_new3("@outer ^");
	fts_object_t *t1 = fts_object_new3("@outer *");
	fts_object_t *out = fts_object_new3("@out 256 256");
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
	Dict *d = Dict_new(0);
	Dict_put(d,(void *)(0+SYM(apples)),"oranges");
	Dict_put(d,(void *)(0+SYM(monty)),"python");
	Dict_put(d,(void *)(0+SYM(luby)),"is not engrish");
	Dict_put(d,(void *)(0+SYM(foo)),"bar");
	Dict_put(d,(void *)(0+SYM(recursive)),"n. see also: recursive");
	Dict_each(d,test_dict$1,0);
}

void test_view_ppm1(void) {
	fts_object_t *in = fts_object_new3("@in");
	fts_object_t *out = fts_object_new3("@out 256 256");
	fts_connect(in,0,out,0);
//	fts_send3(in,0,"open ppm file "DIR"/g001.ppm");
	fts_send3(in,0,"open grid file "DIR"/foo.grid");
	fts_send3(in,0,"bang");
}

void test_view_ppm2(void) {
	fts_object_t *in = fts_object_new3("@in");
	fts_object_t *pa = fts_object_new3("@convolve << + 0");
	fts_object_t *pb = fts_object_new3("@ / 9");
	fts_object_t *ra = fts_object_new3("@redim 3 3");
	fts_object_t *out = fts_object_new3("@out 256 256");
	fts_object_t *v4j = fts_object_new3("@global");
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

void test_view_anim(void) {
	fts_object_t *in = fts_object_new3("@in");
/*	fts_object_t *pa = fts_object_new3("@scale_by"); */
	fts_object_t *out = fts_object_new3("@out 256 256");
	fts_object_t *v4j = fts_object_new3("@global");
/*
	fts_connect(in,0,pa,0);
	fts_connect(pa,0,out,0);
*/
	fts_connect(in,0,out,0);
	fts_send3(in,0,"open ppm file "DIR"/b.ppm.cat");
//	fts_send3(in,0,"open videodev /dev/video");
	fts_send3(in,0,"option channel 1");
	fts_send3(in,0,"option size 480 640");
	fts_send3(out,0,"option timelog 1");
	{ int i; for (i=0; i<50; i++) fts_send3(in,0,"bang"); }
	fts_send3(v4j,0,"profiler_dump");
}

void test_view_anim2(void) {
	fts_object_t *in = fts_object_new3("@in");
	fts_object_t *out = fts_object_new3("@out 256 256");
	fts_connect(in,0,out,0);
	fts_send3(in,0,"open mpeg file /home/matju/net/washington_zoom_in.mpeg");
//	printf("here...\n");
	while(1) fts_send3(in,0,"bang");
//	printf("there...\n");
}

typedef struct TestTCP {
	fts_object_t *in1, *in2, *out;
	int toggle;
} TestTCP;

void test_tcp$1(TestTCP *$) {
	if (GridInlet_busy(((GridObject *)$->out)->in[0])) return;
	if ($->toggle==0) {
		fts_send3($->in1,0,"bang");
	} else {
		fts_send3($->in2,0,"bang");
	}
	$->toggle ^= 1;
}

void test_tcp$2(TestTCP *$) {
	if (GridInlet_busy(((GridObject *)$->out)->in[0])) return;
	fts_send3($->in1,0,"bang");
}

void test_tcp(void) {
	if (fork()) {
		/* client */
		TestTCP *$ = NEW(TestTCP,1);
		$->in1 = fts_object_new3("@in");
		$->out = fts_object_new3("@out 240 320");
		whine_header = "[client] ";
		fts_connect($->in1,0,$->out,0);
		fts_send3($->out,0,"option autodraw 2");
		whine("test: waiting 2 seconds");
		sleep(2);
		fts_send3($->in1,0,"open grid tcp localhost 4247");

//		Dict_put(gridflow_alarm_set,$,test_tcp$2);
		fts_send3($->in1,0,"bang");
	} else {
		/* server */
		TestTCP *$ = NEW(TestTCP,1);
		$->in1 = fts_object_new3("@in");
		$->in2 = fts_object_new3("@in");
		$->out = fts_object_new3("@out");
		$->toggle = 0;
		whine_header = "[server] ";
		fts_connect($->in1,0,$->out,0);
		fts_connect($->in2,0,$->out,0);
		fts_send3($->in1,0,"open ppm file "DIR"/r001.ppm");
		fts_send3($->in2,0,"open ppm file "DIR"/b001.ppm");
		fts_send3($->out,0,"open grid tcpserver 4247");
//		Dict_put(gridflow_alarm_set,$,test_tcp$1);
		fts_send3($->in1,0,"bang");
	}		
}

void test_foo(void) {
	fts_object_t *in  = fts_object_new3("@in");
	fts_object_t *out = fts_object_new3("@out");
	fts_send3(out,0,"open videodev /dev/video0");
}

int main(void) {
	Dict *tests = Dict_new(0);
	int i;
	gridflow_init_standalone();
#define TEST(_x_) Dict_put(tests,#_x_,test_##_x_);
	TEST(dict)
	TEST(view_ppm1)
	TEST(view_ppm2)
	TEST(view_anim)
	TEST(view_anim2)
	TEST(foo)
	TEST(tcp)
	TEST(gen)

//	((void(*)(void))Dict_get(tests,"dict"))();
	((void(*)(void))Dict_get(tests,"gen"))();

	fts_loop();
/*	{int i; for(i=0;;i++) printf("%3d = %s\n", i, fts_symbol_name(i)); } */
	return 0;
}
