#pragma once

#include "image.h"
#include "point.h"

void saveSquare(Image *image, const char *filename, Point *point, int size);
void saveImage(Image *image, const char *filename);
void saveBoard(Image *image, const char *filename);
