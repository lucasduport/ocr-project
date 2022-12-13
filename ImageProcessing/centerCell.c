#include "centerCell.h"

void shiftLeft(Image *cell, int shift, uc bg)
{
	int w = cell->width, h = cell->height;
	uc nb_channels = cell->nb_channels;
	for (uc n = 0; n < nb_channels; n++)
	{
		uc *channel = cell->channels[n];
		for (int x = 0; x < w - shift; x++)
			for (int y = 0; y < h; y++)
				channel[y * w + x] = channel[y * w + x + shift];
		for (int x = w - shift; x < w; x++)
			for (int y = 0; y < h; y++)
				channel[y * w + x] = bg;
	}
}

void shiftRight(Image *cell, int shift, uc bg)
{
	int w = cell->width, h = cell->height;
	uc nb_channels = cell->nb_channels;
	for (uc n = 0; n < nb_channels; n++)
	{
		uc *channel = cell->channels[n];
		for (int x = w - 1; x >= shift; x--)
			for (int y = 0; y < h; y++)
				channel[y * w + x] = channel[y * w + x - shift];
		for (int x = 0; x < shift; x++)
			for (int y = 0; y < h; y++)
				channel[y * w + x] = bg;
	}
}

void shiftUp(Image *cell, int shift, uc bg)
{
	int w = cell->width, h = cell->height;
	uc nb_channels = cell->nb_channels;
	for (uc n = 0; n < nb_channels; n++)
	{
		uc *channel = cell->channels[n];
		for (int y = 0; y < h - shift; y++)
			for (int x = 0; x < w; x++)
				channel[y * w + x] = channel[(y + shift) * w + x];
		for (int y = h - shift; y < h; y++)
			for (int x = 0; x < w; x++)
				channel[y * w + x] = bg;
	}
}

void shiftDown(Image *cell, int shift, uc bg)
{
	int w = cell->width, h = cell->height;
	uc nb_channels = cell->nb_channels;
	for (uc n = 0; n < nb_channels; n++)
	{
		uc *channel = cell->channels[n];
		for (int y = h - 1; y >= shift; y--)
			for (int x = 0; x < w; x++)
				channel[y * w + x] = channel[(y - shift) * w + x];
		for (int y = 0; y < shift; y++)
			for (int x = 0; x < w; x++)
				channel[y * w + x] = bg;
	}
}

void autoCenter(Image *cell, uc threshold, uc bg)
{
	int w = cell->width, h = cell->height;
	uc nb_channels = cell->nb_channels;
	int x_min = w - 1, x_max = 0, y_min = h - 1, y_max = 0;
	for (uc n = 0; n < nb_channels; n++)
	{
		if (n == 3) // alpha channel
			continue;
		uc *channel = cell->channels[n];
		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				if (channel[y * w + x] > threshold)
				{
					if (x < x_min)
						x_min = x;
					if (x > x_max)
						x_max = x;
					if (y < y_min)
						y_min = y;
					if (y > y_max)
						y_max = y;
				}
			}
		}
	}
	if (x_min > x_max || y_min > y_max)
		return; // no need to center (already centered)
	int x_shift = (x_min + x_max) / 2 - w / 2;
	int y_shift = (y_min + y_max) / 2 - h / 2;
	if (x_shift > 0)
		shiftLeft(cell, x_shift, bg);
	else if (x_shift < 0)
		shiftRight(cell, -x_shift, bg);
	if (y_shift > 0)
		shiftUp(cell, y_shift, bg);
	else if (y_shift < 0)
		shiftDown(cell, -y_shift, bg);
}