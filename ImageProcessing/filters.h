#pragma once

#include "image.h"

void invertImage(Image *image);
void thresholdCells(Image *image);
void gaussianBlur(Image *image);
void calibrateImage(Image *image, int radius, uc bg);
void sobelFilter(Image *image);
void thresholdToUpper(Image *image, uc threshold);
void saturateImage(Image *image);
void calibrateCell(Image *image);
