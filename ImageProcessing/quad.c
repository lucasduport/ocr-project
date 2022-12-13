#include <err.h>
#include <stdlib.h>
#include "quad.h"

Quad *newQuad(Point *p1, Point *p2, Point *p3, Point *p4)
{
	Quad *quad = (Quad *)malloc(sizeof(Quad));
	if (quad == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	quad->p1 = p1;
	quad->p2 = p2;
	quad->p3 = p3;
	quad->p4 = p4;
	return quad;
}

void freeQuad(Quad *quad)
{
	free(quad->p1);
	free(quad->p2);
	free(quad->p3);
	free(quad->p4);
	free(quad);
}
