#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <sys/stat.h>
#include "cellExtraction.h"
#include "centerCell.h"
#include "display.h"
#include "filters.h"
#include "param.h"
#include "transformImage.h"

void addBorders(Image *image, int border_size, uc bg)
{
	int w = image->width, h = image->height;
	int new_w = w + 2 * border_size, new_h = h + 2 * border_size;
	uc nb_channels = image->nb_channels;
	for (uc n = 0; n < nb_channels; n++)
	{
		uc *channel = image->channels[n];
		uc *new_channel = malloc(new_w * new_h * sizeof(uc));
		if (new_channel == NULL)
			errx(EXIT_FAILURE, "malloc failed");
		for (int y = 0; y < new_h; y++)
		{
			if (y < border_size || y >= new_h - border_size)
			{
				for (int x = 0; x < new_w; x++)
					new_channel[y * new_w + x] = bg;
				continue;
			}
			for (int x = 0; x < new_w; x++)
			{
				if (x < border_size || x >= new_w - border_size)
					new_channel[y * new_w + x] = bg;
				else
					new_channel[y * new_w + x]
						= channel[(y - border_size) * w + x - border_size];
			}
		}
		free(channel);
		image->channels[n] = new_channel;
	}
	image->width = new_w;
	image->height = new_h;
}

Image *getCell(Image *image, int x0, int x1, int y0, int y1)
{
	st w = image->width;
	uc nb_channels = image->nb_channels;
	st cell_w = x1 - x0;
	st cell_h = y1 - y0;
	Image *cell = newImage(nb_channels, cell_w, cell_h);
	for (uc n = 0; n < nb_channels; n++)
	{
		uc *channel = image->channels[n];
		uc *cell_channel = cell->channels[n];
		for (st y = 0; y < cell_h; y++)
			for (st x = 0; x < cell_w; x++)
				cell_channel[y * cell_w + x] = channel[(y0 + y) * w + x0 + x];
	}
	return cell;
}

void getCenterCell(Image *image, int w_cell, int h_cell)
{
	int w = image->width;
	int h = image->height;
	int w_border = (w - w_cell) / 2;
	int h_border = (h - h_cell) / 2;
	uc nb_channels = image->nb_channels;
	int x0, y0;
	for (uc n = 0; n < nb_channels; n++)
	{
		uc *channel = image->channels[n];
		uc *new_channel = malloc(w_cell * h_cell * sizeof(uc));
		if (new_channel == NULL)
			errx(EXIT_FAILURE, "malloc failed");
		for (int y = 0; y < h_cell; y++)
		{
			y0 = y + h_border;
			for (int x = 0; x < w_cell; x++)
			{
				x0 = x + w_border;
				new_channel[y * w_cell + x] = channel[y0 * w + x0];
			}
		}
		free(channel);
		image->channels[n] = new_channel;
	}
	image->width = w_cell;
	image->height = h_cell;
}

void saveCell(Image *image, int x0, int x1, int y0, int y1, int border_size,
	const char *filename)
{
	int x0_b = x0 - border_size;
	int x1_b = x1 + border_size;
	int y0_b = y0 - border_size;
	int y1_b = y1 + border_size;
	Image *cell = getCell(image, x0_b, x1_b, y0_b, y1_b);
	calibrateCell(cell);
	int w_cell = x1 - x0 - 2 * border_size;
	int h_cell = y1 - y0 - 2 * border_size;
	getCenterCell(cell, w_cell, h_cell);
	resizeImage(cell, 28, 28);
	// autoCenter(cell, 127, 0);
	SDL_Surface *surface = imageToSurface(cell);
	if (IMG_SavePNG(surface, filename) != 0)
	{
		errx(EXIT_FAILURE, "Error while saving image");
	}
	freeImage(cell);
	SDL_FreeSurface(surface);
}

void saveCells(Image *image, int border_size, int *coords_x, int *coords_y,
	const char *basename)
{
	int nb_cells = getNbCells();
	addBorders(image, border_size, 127);
	st len = strlen(basename) + 7;
	char dirname[len];
	snprintf(dirname, len, "board_%s", basename);
	struct stat st_ = {0};
	if (stat(dirname, &st_) == -1)
	{
		mkdir(dirname, 0700);
	}
	len = strlen(dirname) + 11;
	char filename[len];
	int x0, x1, y0, y1;
	for (int i = 0; i < nb_cells; i++)
	{
		for (int j = 0; j < nb_cells; j++)
		{
			snprintf(filename, len, "%s/%02d_%02d.png", dirname, i + 1, j + 1);
			x0 = coords_x[i];
			x1 = coords_x[i + 1];
			y0 = coords_y[j];
			y1 = coords_y[j + 1];
			saveCell(image, x0, x1, y0, y1, border_size, filename);
		}
	}
}

void loadDefaultCells(Image **cells, char *dirname)
{
	int nb_cells = getNbCells();
	for (int i = 0; i < nb_cells; i++)
	{
		if (cells[i] == NULL)
		{
			st len = strlen(dirname) + 8;
			char filename[len];
			snprintf(filename, len, "%s/%d.png", dirname, i + 1);
			Image *image = openImage(filename, 4);
			createAlpha(image, 0, 191);
			toColor(image, 0, 255, 0);
			cells[i] = image;
		}
	}
}

Image **loadCells(int **grid, char *dirname)
{
	int nb_cells = getNbCells();
	Image **digits = (Image **)malloc(nb_cells * sizeof(Image *));
	if (digits == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	for (int i = 0; i < nb_cells; i++)
		digits[i] = NULL;
	st n;
	for (int i = 0; i < nb_cells; i++)
	{
		for (int j = 0; j < nb_cells; j++)
		{
			n = grid[j][i];
			if (n == 0)
				continue;
			if (digits[n - 1])
				continue;
			st len = strlen(dirname) + 17;
			char filename[len];
			snprintf(filename, len, "board_%s/%02d_%02d.png", dirname, i + 1,
				j + 1);
			Image *image = openImage(filename, 4);
			resizeImage(image, 256, 256);
			invertImage(image);
			createAlpha(image, 0, 191);
			toColor(image, 0, 255, 0);
			digits[n - 1] = image;
		}
	}
	loadDefaultCells(digits, "../ImageProcessing/Numbers");
	return digits;
}

void cleanPath(char *filename, char *dest)
{
	char *slash = strrchr(filename, '/');
	if (slash == NULL)
	{
		strcpy(dest, filename);
	}
	else
	{
		strcpy(dest, slash + 1);
	}
	char *dot = strrchr(dest, '.');
	if (dot && dot != dest)
		*dot = '\0';
}
