#include "../grid.h"

void pause (void) {
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
	Dict_put(d,SYM(recursive),"see also: recursive");
	Dict_each(d,test_dict$1,0);
}

void test_view_ppm(void) {
	fts_atom_t a[10];
	fts_object_t *in, *out;
	fts_set_symbol(a+0,SYM(@in));
	in = fts_object_new(1,a);
	fts_set_symbol(a+0,SYM(@out));
	fts_set_int(a+1,240);
	fts_set_int(a+2,320);
	out = fts_object_new(3,a);
	fts_connect(in,0,out,0);
	fts_set_symbol(a+0,SYM(open));
	fts_set_symbol(a+1,SYM(ppm));
	fts_set_symbol(a+2,SYM(file));
	fts_set_symbol(a+3,SYM(/home/projects/video4jmax/images/teapot.ppm));
	fts_send2(in,0,4,a);
	fts_set_symbol(a+0,SYM(bang));
	fts_send2(in,0,1,a);
}

int main(void) {
	video4jmax_init_standalone();
/*	test_dict(); */
	{int i; for(i=0; i<1; i++) test_view_ppm();}
	fts_loop();
/*	{int i; for(i=0;;i++) printf("%3d = %s\n", i, fts_symbol_name(i)); } */
	return 0;
}
