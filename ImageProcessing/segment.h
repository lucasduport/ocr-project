#pragma once

#include "tools.h"

typedef struct
{
	st x1, y1, x2, y2, theta, r, length;
} Segment;

Segment *newSegment(st x1, st y1, st x2, st y2, st theta, st r, st length);
void freeSegments(Segment **segments, int nb_segments);
