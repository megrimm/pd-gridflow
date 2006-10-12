#include "ltiTypes.h"

float* Float::getPtr(){return &val;}
int* Integer::getPtr(){return &val;}
unsigned char *Ubyte::getPtr(){return &val;}
double *Double::getPtr(){return &val;}
