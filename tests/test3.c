#include <stdio.h>
#include <dlfcn.h>
#define LIBDIR "../c/lib"

/* this program tests shared libraries */

int main (void) {
  void *foo = dlopen(LIBDIR"/libgridflow.so",RTLD_NOW);
  printf("%p\n",foo);
  if (!foo) {
    perror("dlopen");
    return 1;
  }
  return 0;
}
