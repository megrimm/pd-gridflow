#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

typedef long long uint64;

uint64 gf_timeofday () {
	timeval t;
	gettimeofday(&t,0);
	return t.tv_sec*1000000+t.tv_usec;
}

static void test (size_t n) {
	uint64 t = gf_timeofday();
	for (int i=0; i<10000; i++) free(malloc(n));
	t = gf_timeofday() - t;
	printf("10000 mallocs of %7ld bytes takes %7ld us (%f us/malloc)\n",n,(long)t,t/(float)10000);
}

int main () {
	for (int i=0; i<20; i++) {test(4<<i); test(6<<i);}
	return 0;
}
