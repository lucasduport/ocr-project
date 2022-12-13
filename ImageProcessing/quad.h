#pragma once

#include "point.h"

typedef struct
{
	Point *p1, *p2, *p3, *p4;
} Quad;

Quad *newQuad(Point *p1, Point *p2, Point *p3, Point *p4);
void freeQuad(Quad *quad);
