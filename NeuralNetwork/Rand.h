#ifndef RAND_H
#define RAND_H

#include <math.h>
#include <stdlib.h>

typedef unsigned int ui;
typedef double dl;

dl r8_uniform_01(int *seed);
dl r8_normal_01(int *seed);
dl *r8vec_normal_ab_new(ui n, dl a, dl b, int *seed);
dl *r8vec_uniform_01_new(ui n, int *seed);

#endif // RAND_H

