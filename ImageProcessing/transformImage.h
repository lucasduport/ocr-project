#pragma once

#include <err.h>
#include <math.h>
#include <stdio.h>
#include "image.h"
#include "matrices.h"

void toGrey(Image *image);
void toRGBA(Image *image);
void toColor(Image *image, uc r, uc g, uc b);
void createAlpha(Image *image, int min, int max);
void resizeImage(Image *image, st new_w, st new_h);
void autoResize(Image *image, st maw_w, st max_h);
Image *rotateImage(Image *image, int angle, uc background_color);
void integrateNumber(Image *image, Image *number, Point *origin);
Image *extractGrid(Image *image, Quad *quad, st new_w, st new_h);
