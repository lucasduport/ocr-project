#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "saveImage.h"
#include "display.h"
#include "tools.h"

// used for UI
void saveImage(Image *image, const char *filename)
{
	SDL_Surface *surface = imageToSurface(image);
	if (IMG_SavePNG(surface, filename) != 0)
		errx(1, "Error while saving image");
	SDL_FreeSurface(surface);
	return;
}

/*
Image *getCell(Image *image, int i, int j)
{
	int cell_size = 38;
	int border = 5;
	int digit_size = cell_size - 2 * border;
	uc nb_channels = image->nb_channels;
	Image *cell = newImage(nb_channels, digit_size, digit_size);
	int w = image->width;
	int x, y;
	for (uc n = 0; n < nb_channels; n++)
	{
		uc *channel = image->channels[n];
		uc *new_channel = cell->channels[n];
		for (int y0 = 0; y0 < digit_size; y0++)
		{
			y = i * cell_size + border + y0;
			for (int x0 = 0; x0 < digit_size; x0++)
			{
				x = j * cell_size + border + x0;
				new_channel[y0 * digit_size + x0] = channel[y * w + x];
			}
		}
	}
	return cell;
}

// in cellExtraction.c
void saveCell(Image *image, const char *dirname, int i, int j)
{
	Image *cell = getCell(image, i, j);
	SDL_Surface *surface = imageToSurface(cell);
	st len = strlen(dirname) + 9;
	char filename[len];
	snprintf(filename, len, "%s/%d_%d.png", dirname, i+1, j+1);
	if (IMG_SavePNG(surface, filename) != 0)
	{
		errx(EXIT_FAILURE, "Error while saving image");
	}
	freeImage(cell);
	SDL_FreeSurface(surface);
}

// image must be an extracted board
// filename is the name of the folder to save the image to
void saveBoard(Image *image, const char *filename)
{
	st len = strlen(filename) + 7;
	char dirname[len];
	snprintf(dirname, len, "board_%s", filename);
	struct stat st_ = {0};
	if (stat(dirname, &st_) == -1)
	{
		mkdir(dirname, 0700);
	}
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			saveCell(image, dirname, i, j);
}
*/
