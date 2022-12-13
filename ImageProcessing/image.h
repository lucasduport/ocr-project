#pragma once

#include "quad.h"
#include "tools.h"

typedef struct
{
	uc nb_channels; // 1: Grey, (3: RGB,) 4: RGBA
	st width;
	st height;
	uc **channels; // uc channels[nb_channels][width*height]
} Image;

uc *newChannel(st len);
uc *copyChannel(uc *pixels, st len);
Image *newImage(uc nb_channels, st width, st height);
Image *copyImage(Image *image);
void freeImage(Image *image);
Image *openImage(const char *filename, uc nb_channels);
void placeDigit(Image *bg, Image *d, Quad *grid, int x0, int y0);
