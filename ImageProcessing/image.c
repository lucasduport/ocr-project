#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include "image.h"
#include "matrices.h"
#include "param.h"

// Contains basics functions for manipulating the Image struct

uc *newChannel(st len)
{
	uc *new_channel = (uc *)malloc(sizeof(uc) * len);
	if (new_channel == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	return new_channel;
}

uc *copyChannel(uc *channel, st len)
{
	uc *new_channel = newChannel(len);
	for (st i = 0; i < len; i++)
		new_channel[i] = channel[i];
	return new_channel;
}

Image *newImage(uc nb_channels, st width, st height)
{
	Image *image = (Image *)malloc(sizeof(Image));
	if (image == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	image->nb_channels = nb_channels;
	image->width = width;
	image->height = height;
	uc **channels = (uc **)malloc(nb_channels * sizeof(uc *));
	if (channels == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	for (uc i = 0; i < nb_channels; i++)
	{
		channels[i] = newChannel(width * height);
	}
	image->channels = channels;
	return image;
}

Image *copyImage(Image *image)
{
	Image *copy = (Image *)malloc(sizeof(Image));
	if (copy == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	uc nb_channels = image->nb_channels;
	copy->nb_channels = nb_channels;
	st width = image->width, height = image->height;
	copy->width = width;
	copy->height = height;
	uc **channels = (uc **)malloc(nb_channels * sizeof(uc *));
	if (channels == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	for (uc i = 0; i < nb_channels; i++)
		channels[i] = copyChannel(image->channels[i], width * height);
	copy->channels = channels;
	return copy;
}

void freeImage(Image *image)
{
	for (uc i = 0; i < image->nb_channels; i++)
		free(image->channels[i]);
	free(image->channels);
	free(image);
}

void surfaceToGrey(SDL_Surface *surface, Image *image)
{
	st len = surface->w * surface->h;
	SDL_PixelFormat *format = surface->format;
	Uint32 *pixels = surface->pixels;
	uc *grey_channel = image->channels[0];
	uc r, g, b, a;
	for (st i = 0; i < len; i++)
	{
		SDL_GetRGBA(pixels[i], format, &r, &g, &b, &a);
		grey_channel[i] = (r + g + b) / 3;
	}
}

void surfaceToRGBA(SDL_Surface *surface, Image *image)
{
	st len = surface->w * surface->h;
	SDL_PixelFormat *format = surface->format;
	Uint32 *pixels = surface->pixels;
	uc *r_channel = image->channels[0];
	uc *g_channel = image->channels[1];
	uc *b_channel = image->channels[2];
	uc *a_channel = image->channels[3];
	uc r, g, b, a;
	for (st i = 0; i < len; i++)
	{
		SDL_GetRGBA(pixels[i], format, &r, &g, &b, &a);
		r_channel[i] = r;
		g_channel[i] = g;
		b_channel[i] = b;
		a_channel[i] = a;
	}
}

Image *openImage(const char *filename, uc nb_channels)
{
	SDL_Surface *surface_tmp = IMG_Load(filename);
	if (surface_tmp == NULL)
		errx(EXIT_FAILURE, "%s", SDL_GetError());
	SDL_Surface *surface
		= SDL_ConvertSurfaceFormat(surface_tmp, SDL_PIXELFORMAT_RGBA8888, 0);
	if (surface == NULL)
		errx(EXIT_FAILURE, "%s", SDL_GetError());
	SDL_FreeSurface(surface_tmp);
	Image *image = newImage(nb_channels, surface->w, surface->h);
	if (SDL_LockSurface(surface) != 0)
		errx(EXIT_FAILURE, "%s", SDL_GetError());
	switch (nb_channels)
	{
		case 1:
			surfaceToGrey(surface, image);
			break;
		case 4:
			surfaceToRGBA(surface, image);
			break;
		default:
			errx(EXIT_FAILURE, "an Image should have 1 or 4 channels");
	}
	SDL_UnlockSurface(surface);
	SDL_FreeSurface(surface);
	return image;
}

void placePixel(Image *bg, int x_bg, int y_bg, Image *d, int x_d, int y_d)
{
	uc nb_channels_bg = bg->nb_channels;
	int w_bg = bg->width, w_d = d->width;
	int coord_bg = y_bg * w_bg + x_bg;
	int coord_d = y_d * w_d + x_d;
	if (nb_channels_bg != 4)
	{
		for (uc n = 0; n < nb_channels_bg; n++)
		{
			bg->channels[n][coord_bg] = d->channels[n][coord_d];
		}
		return;
	}
	int val;
	uc a_bg = bg->channels[3][coord_bg];
	uc a_d = d->channels[3][coord_d];
	for (uc n = 0; n < 3; n++)
	{ // RGB channels
		uc pxl_d = d->channels[n][coord_d];
		uc pxl_bg = bg->channels[n][coord_bg];
		val = pxl_d * a_d * 255 + pxl_bg * a_bg * (255 - a_d);
		val /= a_d * 255 + a_bg * (255 - a_d);
		bg->channels[n][coord_bg] = (uc)val;
	}
	// alpha channel
	val = a_d * 255 + a_bg * (255 - a_d);
	bg->channels[3][coord_bg] = (uc)(val / 255);
}

void placeDigit(Image *bg, Image *d, Quad *grid, int x0, int y0)
{
	int nb_cells = getNbCells();
	uc nb_channels = bg->nb_channels;
	if (d->nb_channels != nb_channels)
		errx(EXIT_FAILURE, "background and d must have the same number of "
						   "channels");
	float mat[3][3];
	getTransformMatrix(grid, nb_cells * 384, nb_cells * 384, mat);
	int w_bg = bg->width, h_bg = bg->height;
	int w_d = d->width, h_d = d->height;
	float input[3] = {0, 0, 1};
	float res[3];
	int x_bg, y_bg;
	for (int y_d = 0; y_d < h_d; y_d++)
	{
		// input[1] = y_d + 384 * j + 64;
		input[1] = y_d + y0;
		for (int x_d = 0; x_d < w_d; x_d++)
		{
			// input[0] = x_d + 384 * i + 64;
			input[0] = x_d + x0;
			matMul33_31(mat, input, res);
			x_bg = res[0] / res[2];
			y_bg = res[1] / res[2];
			if (x_bg < 0 || x_bg >= w_bg || y_bg < 0 || y_bg >= h_bg)
				continue;
			placePixel(bg, x_bg, y_bg, d, x_d, y_d);
		}
	}
}
