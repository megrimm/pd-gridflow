/* $Id$ */

#include "../grid.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void pressakey(void) {
	char buf[256];
	printf("press return to continue.\n");
	fgets(buf,256,stdin);
}

void test_dict$1(void *foo, fts_symbol_t k, void *v) {
	printf("%s : %s\n", fts_symbol_name(k), (const char *)v);
}

void test_dict(void) {
	Dict *d = Dict_new();
	Dict_put(d,SYM(apples),"oranges");
	Dict_put(d,SYM(monty),"python");
	Dict_put(d,SYM(luby),"is not engrish");
	Dict_put(d,SYM(foo),"bar");
	Dict_put(d,SYM(recursive),"n. see also: recursive");
	Dict_each(d,test_dict$1,0);
}

void test_view_ppm(void) {
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
	fts_send3(in,0,"open ppm file ../../../images/teapot.ppm");
//	fts_send3(in,0,"open ppm file ../../../images/g001.ppm");
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
/*	fts_send3(in,0,"open ppm file ../../../images/b.ppm.cat"); */
	fts_send3(in,0,"open videodev /dev/video");
	fts_send3(in,0,"option channel 1");
	fts_send3(in,0,"option size 480 640");
	fts_send3(out,0,"option timelog 1");
	{ int i; for (i=0; i<50; i++) fts_send3(in,0,"bang"); }
	fts_send3(v4j,0,"profiler_dump");
}

void test_tcp(void) {
	if (fork()) {
		/* server */
		fts_object_t *in1 = fts_object_new3("@in");
		fts_object_t *in2 = fts_object_new3("@in");
		fts_object_t *out = fts_object_new3("@out");
		fts_connect(in1,0,out,0);
		fts_connect(in2,0,out,0);
		fts_send3(in1,0,"open ppm file ../../../images/r001.ppm");
		fts_send3(in2,0,"open ppm file ../../../images/b001.ppm");
		fts_send3(out,0,"open grid tcpserver 4243");
		while(1) {
			fts_send3(in1,0,"bang"); sleep(1);
			fts_send3(in2,0,"bang"); sleep(1);
		}
	} else {
		/* client */
		fts_object_t *in  = fts_object_new3("@in");
		fts_object_t *out = fts_object_new3("@out 240 320");
		fts_connect(in,0,out,0);
		fts_send3(out,0,"autodraw 2");
		whine("test: waiting 2 seconds\n");
		sleep(2);
		fts_send3(in,0,"open grid tcp localhost 4243");
		while(1) { fts_send3(in,0,"bang"); sleep(1); }
	}		
}

int main(void) {
	gridflow_init_standalone();
/*	test_dict(); */
/*	test_view_ppm(); */
/*	test_view_anim(); */
	test_tcp();

	fts_loop();
/*	{int i; for(i=0;;i++) printf("%3d = %s\n", i, fts_symbol_name(i)); } */
	return 0;
}
