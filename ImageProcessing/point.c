#include <err.h>
#include "point.h"

Point *newPoint(st x, st y)
{
	Point *point = (Point *)malloc(sizeof(Point));
	if (point == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	point->x = x;
	point->y = y;
	return point;
}
