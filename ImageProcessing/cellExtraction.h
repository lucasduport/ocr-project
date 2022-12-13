#pragma once

#include "image.h"

// void saveCells(Image *image, int cell_size, int border_size, const char
// *filename);
void saveCells(Image *image, int border_size, int *coords_x, int *coords_y,
	const char *basename);
Image **loadCells(int **solved, char *dirname);
void cleanPath(char *filename, char *dest);
