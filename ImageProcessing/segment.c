#include <err.h>
#include "segment.h"

Segment *newSegment(st x1, st y1, st x2, st y2, st theta, st r, st length)
{
	Segment *segment = (Segment *)malloc(sizeof(Segment));
	if (segment == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	segment->x1 = x1;
	segment->y1 = y1;
	segment->x2 = x2;
	segment->y2 = y2;
	segment->theta = theta;
	segment->r = r;
	segment->length = length;
	return segment;
}

void freeSegments(Segment **segments, int nb_segments)
{
	for (int i = 0; i < nb_segments; i++)
		free(segments[i]);
}
