#include <err.h>
#include "cellsDetection.h"
#include "image.h"
#include "param.h"
#include "tools.h"

int *buildHistoH(Image *image)
{
	st width = image->width;
	st height = image->height;
	uc *channel = image->channels[0];
	int *histo_h = (int *)malloc(sizeof(int) * height);
	if (histo_h == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	for (st y = 0; y < height; y++)
	{
		int sum = 0;
		for (st x = 0; x < width; x++)
			sum += channel[y * width + x];
		histo_h[y] = sum;
	}

	return histo_h;
}

int *buildHistoV(Image *image)
{
	st width = image->width;
	st height = image->height;
	uc *channel = image->channels[0];
	int *histo_v = (int *)malloc(width * sizeof(int));
	if (histo_v == NULL)
		errx(EXIT_FAILURE, "malloc failed");
	for (st x = 0; x < width; x++)
	{
		int sum = 0;
		for (st y = 0; y < height; y++)
			sum += channel[y * width + x];
		histo_v[x] = sum;
	}

	return histo_v;
}

void detectNbLines(int *histo, st len, st nb_lines, int *coords, int *values)
{
	float cell_size = (float)len / nb_lines;
	int range = cell_size / 10;
	for (st i = 1; i < nb_lines; i++)
	{
		int max = 0;
		int x0 = i * cell_size;
		int x_max = x0;
		for (int x = x0 - range; x < x0 + range; x++)
		{
			if (histo[x] > max)
			{
				max = histo[x];
				x_max = x;
			}
		}
		values[i - 1] = max;
		coords[i - 1] = x_max;
	}
}

void completeCoords(
	int *coords_in, int *coords_out, int len, int mean, int max)
{
	for (int i = 1; i < len; i++)
		coords_out[i] = coords_in[i - 1];
	coords_out[0] = coords_out[1] - mean;
	if (coords_out[0] < 0)
		coords_out[0] = 0;
	coords_out[len] = coords_out[len - 1] + mean;
	if (coords_out[len] > max)
		coords_out[len] = max;
}

int getGridDimension(Image *image, int **coords_x, int **coords_y)
{
	st width = image->width;
	st height = image->height;
	int *histo_h = buildHistoH(image);
	int *histo_v = buildHistoV(image);

	int coords_h_9[8];
	int values_h_9[8];
	int coords_v_9[8];
	int values_v_9[8];

	int coords_h_16[15];
	int values_h_16[15];
	int coords_v_16[15];
	int values_v_16[15];

	detectNbLines(histo_h, height, 9, coords_h_9, values_h_9);
	detectNbLines(histo_v, width, 9, coords_v_9, values_v_9);
	detectNbLines(histo_h, height, 16, coords_h_16, values_h_16);
	detectNbLines(histo_v, width, 16, coords_v_16, values_v_16);

	free(histo_h);
	free(histo_v);

	float mean_9 = 0;
	mean_9 += coords_h_9[7] - coords_h_9[0];
	mean_9 += coords_v_9[7] - coords_v_9[0];
	mean_9 /= 14;
	float var_9 = 0;
	for (int i = 0; i < 8; i++)
	{
		float val = coords_h_9[i];
		var_9 += (val - mean_9) * (val - mean_9);
		val = coords_v_9[i];
		var_9 += (val - mean_9) * (val - mean_9);
	}
	var_9 /= 9;

	float mean_16 = 0;
	mean_16 += coords_h_16[14] - coords_h_16[0];
	mean_16 += coords_v_16[14] - coords_v_16[0];
	mean_16 /= 28;
	float var_16 = 0;
	for (int i = 0; i < 15; i++)
	{
		float val = coords_h_16[i];
		var_16 += (val - mean_16) * (val - mean_16);
		val = coords_v_16[i];
		var_16 += (val - mean_16) * (val - mean_16);
	}
	var_16 /= 16;

	float max = 0;
	for (int i = 0; i < 15; i++)
	{
		if (values_h_16[i] > max)
			max = values_h_16[i];
		if (values_v_16[i] > max)
			max = values_v_16[i];
	}
	max *= 0.5;
	for (int i = 0; i < 15; i++)
	{
		if (values_h_16[i] < max || values_v_16[i] < max)
		{
			*coords_x = (int *)malloc(10 * (sizeof(int)));
			completeCoords(coords_v_9, *coords_x, 9, mean_9, width);
			*coords_y = (int *)malloc(10 * (sizeof(int)));
			completeCoords(coords_h_9, *coords_y, 9, mean_9, height);
			setNbCells(9);
			return 9;
		}
	}

	max = 0;
	for (int i = 0; i < 8; i++)
	{
		if (values_h_9[i] > max)
			max = values_h_9[i];
		if (values_v_9[i] > max)
			max = values_v_9[i];
	}
	max *= 0.5;
	for (int i = 0; i < 8; i++)
	{
		if (values_h_9[i] < max || values_v_9[i] < max)
		{
			*coords_x = (int *)malloc(17 * (sizeof(int)));
			completeCoords(coords_v_16, *coords_x, 16, mean_16, width);
			*coords_y = (int *)malloc(17 * (sizeof(int)));
			completeCoords(coords_h_16, *coords_y, 16, mean_16, height);
			setNbCells(16);
			return 16;
		}
	}

	if (var_9 <= var_16)
	{
		*coords_x = (int *)malloc(10 * (sizeof(int)));
		completeCoords(coords_v_9, *coords_x, 9, mean_9, width);
		*coords_y = (int *)malloc(10 * (sizeof(int)));
		completeCoords(coords_h_9, *coords_y, 9, mean_9, height);
		setNbCells(9);
		return 9;
	}
	*coords_x = (int *)malloc(17 * (sizeof(int)));
	completeCoords(coords_v_16, *coords_x, 16, mean_16, width);
	*coords_y = (int *)malloc(17 * (sizeof(int)));
	completeCoords(coords_h_16, *coords_y, 16, mean_16, height);
	setNbCells(16);
	return 16;
}
