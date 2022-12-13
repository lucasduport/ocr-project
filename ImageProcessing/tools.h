#pragma once

#include <stdlib.h>

#define PI 3.141592654

#define IMAGE_MAX_WIDTH 800
#define IMAGE_MAX_HEIGHT 600
#define CELLSIZE 38

extern float COS[360];
extern float SIN[360];

typedef unsigned char uc;
typedef size_t st;

void initTrig();
