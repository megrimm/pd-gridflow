#include "../c/src/grid.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DIR "../images/"

void test_formats(void) {
	FObject *in = fts_object_new3("@in");
	FObject *out = fts_object_new3("@out 256 256");
	fts_connect(in,0,out,0);
	fts_send3(in,0,"open ppm file "DIR"g001.ppm");
	fts_send3(in,0,"bang");
	sleep(1);
	fts_send3(in,0,"open ppm file "DIR"b001.ppm");
	fts_send3(in,0,"bang");
	sleep(1);
	fts_send3(in,0,"open ppm file "DIR"r001.ppm");
	fts_send3(in,0,"bang");
	sleep(1);
	fts_send3(in,0,"open targa file "DIR"teapot.tga");
	fts_send3(in,0,"bang");
	sleep(1);
	fts_send3(in,0,"open grid file "DIR"foo.grid");
	fts_send3(in,0,"bang");
	sleep(1);
	fts_object_delete(in);
	fts_object_delete(out);
}


int main(void) {
	int i;
	gf_init_standalone();
	test_formats();
	return 0;
}
