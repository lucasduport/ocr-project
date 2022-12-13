#ifndef TOOLS_H
#define TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "Rand.h"

#define EPS 10E-10L

//typedef unsigned int ui;
typedef const ui cui;
//typedef long double dl;
typedef const dl cdl;

static inline ui Max(ui a, ui b) {return a > b ? a : b;}
static inline ui Min(ui a, ui b) {return a > b ? b : a;}
void matr_display(dl *arr, cui s, cui cols);
dl *fvec_alloc(cui n, bool zInit);
char *cvec_alloc(cui n);
dl *fvec_rInit(cui n, cui conns, bool he_init);
inline dl absl(cdl nb) {return (nb < 0) ? -nb : nb;}
void arr_shuffle(dl *arr[], dl *paired_arr[], cui Size);

#endif // TOOLS_H
