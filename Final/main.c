#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

#include "../ImageProcessing/display.h"
#include "../ImageProcessing/filters.h"
#include "../ImageProcessing/hough.h"
#include "../ImageProcessing/saveImage.h"
#include "../ImageProcessing/tools.h"
#include "../ImageProcessing/transformImage.h"
#include "../ImageProcessing/cellExtraction.h"
#include "../ImageProcessing/cellsDetection.h"
#include "../ImageProcessing/centerCell.h"
#include "../ImageProcessing/param.h"

#include "../NeuralNetwork/Network.h"

#include "../Solver/solver.h"
#include "../Solver/solver16.h"

#include "../UserInterface/ui.h"

int noUI(int argc, char **argv);

void init()
{
	initTrig();
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		errx(EXIT_FAILURE, "%s", SDL_GetError());
}

void searchDigit(int **sudoku, int n, int *i, int *j)
{
	for (int y = 0; y < 9; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			if (sudoku[y][x] == n)
			{
				*i = x;
				*j = y;
				return;
			}
		}
	}
	*i = -1;
	*j = -1;
}

double *center_input(const char *path)
{
	Image *cell = openImage(path, 1);
	autoCenter(cell, 15, 0);
	uc *channel = cell->channels[0];
	double *pixels = (double *)malloc(784 * sizeof(double));
	if (pixels == NULL)
	{
		errx(EXIT_FAILURE, "malloc failed");
	}
	for (int i = 0; i < 784; i++)
	{
		uc val = channel[i];
		pixels[i] = 2 * (double)val / 255 - 1;
	}
	freeImage(cell);
	return pixels;
}

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		setUIMode(1);
		// init Gtk
		gtk_init(&argc, &argv);
		// init trigonometric tables
		initTrig();
		// init UserInterface
		uiLaunch();
		// remove tmp folder
		rmDir("tmpImg/");
	}
	else
	{
		noUI(argc, argv);
	}
	return EXIT_SUCCESS;
}

int noUI(int argc, char **argv)
{
	if (argc != 2)
		errx(1, "Usage: %s <image>", argv[0]);
	char *filename = argv[1];
	init();
	setUIMode(0);

	Image *final = openImage(filename, 4);
	autoResize(final, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT);
	Image *image = copyImage(final);
	toGrey(image);
	Image *to_extract = copyImage(image);

	// preprocessing
	// displayImage(image, "Original image");
	gaussianBlur(image);
	calibrateImage(image, 150, 255);
	// displayImage(image, "Saturated");
	sobelFilter(image);
	// displayImage(image, "Sobel");
	gaussianBlur(image);
	thresholdToUpper(image, 24);
	// displayImage(image, "Calibrate");

	// detect grid
	Quad *quad = detectGrid(image);
	if (quad == NULL)
		errx(1, "No grid detected.");

	// display results
	showQuad(image, quad, 0, 255, 0);

	// extract grid
	// Image *extracted = extractGrid(to_extract, quad, new_size, new_size);
	Image *extracted = extractGrid(to_extract, quad, 1440, 1440);
	invertImage(extracted);
	displayImage(extracted, "Extracted grid");
	freeImage(to_extract);

	// extract cells
	int *coords_x;
	int *coords_y;
	int nb_cells;
	nb_cells = getGridDimension(extracted, &coords_x, &coords_y);
	int cell_size = 1440 / nb_cells;
	int border_size = cell_size / 5;
	for (int i = 0; i < nb_cells + 1; i++)
	{
		coords_x[i] += border_size;
		coords_y[i] += border_size;
	}
	char filename_stripped[30];
	cleanPath(filename, filename_stripped);
	// saveCells(extracted, CELLSIZE, 5, filenameStripped);
	saveCells(extracted, border_size, coords_x, coords_y, filename_stripped);
	freeImage(extracted);
	for (int i = 0; i < nb_cells + 1; i++)
	{
		coords_x[i] -= border_size;
		coords_x[i] *= nb_cells * 384;
		coords_x[i] /= 1440;
		coords_y[i] -= border_size;
		coords_y[i] *= nb_cells * 384;
		coords_y[i] /= 1440;
	}

	//TODO: use neural network
	int **sudoku = (int **)malloc(nb_cells * sizeof(int *));
	for (int i = 0; i < nb_cells; i++)
	{
		sudoku[i] = (int *)malloc(nb_cells * sizeof(int));
	}
	Network *net = (Network *)malloc(sizeof(Network));
	switch (nb_cells)
	{
		case 9:
			Network_Load(net, "../NeuralNetwork/TrainedNetwork/NeuralNetData_3layers_OCR-Biased_100.0.dnn");
			break;
		case 16:
			Network_Load(net, "../NeuralNetwork/TrainedNetwork/NeuralNetData_3layers_OCR-TEXA-Biased_100.0.dnn");
			break;
		default:
			errx(1, "Unsupported grid size");
			break;
	}
	printf("check : %p\n", net);
	Network_Display(net, 1);
	float *results[nb_cells * nb_cells];
	for (int i = 0; i < nb_cells; i++)
	{
		for (int j = 0; j < nb_cells; j++)
		{
			char path[40];
			sprintf(path, "board_%s/%02d_%02d.png", filename_stripped, i + 1, j + 1);
			double *input = center_input(path);
			results[nb_cells * j + i] = Network_Predict(net, input, 784);
			free(input);
			// for hexa:
			// 0-15 -> 0-F
			// 16 -> empty
			//
			// for decimal:
			// 1-9 -> 1-9
			// 0 -> empty
			int imax = 0;
			float max = 0;
			printf("[");
			for (int k = 0; k < (nb_cells == 9? 10 : 17); k++)
			{
				float proba = results[j * nb_cells + i][k];
				if (proba >= max)
				{
					max = proba;
					imax = k;
				}
				printf("%.3f ", proba);
			}
			puts("]");
			printf("digit in cell %d_%d = %d\n",i,j,imax);
			sudoku[j][i] = imax;
			free(results[nb_cells * j + i]);
		}
	}
	Network_Purge(net);
	// sudoku[0][4] = 7;
	// sudoku[4][8] = 1;
	// sudoku[5][0] = 7;
	// sudoku[8][4] = 8;
	int block_size = nb_cells == 9 ? 3 : 4;
	for (int i = 0; i < nb_cells; i++)
	{
		for (int j = 0; j < nb_cells; j++)
		{
			printf("%02d ", sudoku[i][j]);
			if ((j + 1) % block_size == 0)
				printf(" ");
		}
		printf("\n");
		if ((i + 1) % block_size == 0)
			printf("\n");
	}
	// int **sudoku = readSudoku("../Solver/grid_00");

	int **solved = (int **)malloc(nb_cells * sizeof(int *));
	for (int i = 0; i < nb_cells; i++)
	{
		solved[i] = (int *)malloc(nb_cells * sizeof(int));
		for (int j = 0; j < nb_cells; j++)
		{
			solved[i][j] = sudoku[i][j];
		}
	}

	int a = 10, b = 11, c = 12, d = 13, e = 14, f = 15, n = 16;
	int _hexa[16][16] = {
		{7, 0, e, 0, a, 0, 3, 0, 0, 2, 0, 9, 0, n, 5, b},
		{4, 0, c, 6, e, 2, 0, n, d, 5, 0, 3, a, 0, f, 1},
		{f, 2, 9, 0, 0, 0, 5, b, e, 0, 0, 0, 0, 0, 0, d},
		{0, 0, 0, 3, c, 8, 7, d, b, 0, 0, a, 6, 0, 0, 2},
		{8, 3, 6, 1, 5, 0, 4, 0, 2, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, c},
		{0, f, 0, 9, 0, a, 0, 0, 0, 8, 0, d, 1, 2, 0, 0},
		{d, 0, 2, 0, 1, e, n, 9, 0, 0, 0, f, 0, 0, 0, 0},
		{e, 4, 0, 0, 0, 7, 2, 1, 0, 0, 6, c, 5, f, 0, 0},
		{9, 6, 0, n, 0, 0, 0, 5, f, 0, 2, 0, c, 0, a, 0},
		{c, 1, 0, 7, 0, 0, 0, 0, 9, d, 0, e, 0, 4, 0, n},
		{0, 0, 0, 0, f, 9, d, 0, 0, n, 4, 7, 0, 0, 0, 0},
		{1, 0, d, 0, 2, n, b, f, 0, 0, 9, 0, 0, 0, 7, 5},
		{0, 0, n, 0, 0, 0, 1, 0, 0, 0, 0, b, d, e, 0, a},
		{2, e, 7, 0, 9, 0, a, 8, n, 0, 0, 5, b, c, 6, 4},
		{b, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 9, 1, 0, 0},
	};

	int solvable = 0;
	switch (nb_cells)
	{
		case 9:
			printf("sudoku detected\n");
			solvable = solver(solved);
			break;
		case 16:
			printf("hexa detected\n");
			for (int i = 0; i < 16; i++)
			{
				for (int j = 0; j < 16; j++)
				{
					sudoku[i][j] = _hexa[i][j];
					solved[i][j] = _hexa[i][j];
				}
			}
			solvable = solver16(solved);
			break;
		default:
			errx(EXIT_FAILURE, "wrong value of nb_cells");
			break;
	}
	if (!solvable)
	{
		printf("Sudoku is not solvable\n");
		free(coords_x);
		free(coords_y);
		freeQuad(quad);
		freeImage(image);
		freeImage(final);
		for (int i = 0; i < nb_cells; i++)
		{
			free(sudoku[i]);
			free(solved[i]);
		}
		free(sudoku);
		free(solved);
		errx(EXIT_FAILURE, "No solutions found.");
	}

	/*
	int _hexa_solved[16][16] = {
		{7, d, e, 8, a, f, 3, 6, 1, 2, c, 9, 4, n, 5, b},
		{4, b, c, 6, e, 2, 9, n, d, 5, 7, 3, a, 8, f, 1},
		{f, 2, 9, a, 4, 1, 5, b, e, 6, n, 8, 3, 7, c, d},
		{5, n, 1, 3, c, 8, 7, d, b, 4, f, a, 6, 9, e, 2},
		{8, 3, 6, 1, 5, b, 4, 7, 2, c, e, n, f, a, d, 9},
		{a, 7, 4, e, 8, d, f, 2, 5, 9, 1, 6, n, b, 3, c},
		{n, f, 5, 9, 6, a, c, 3, 7, 8, b, d, 1, 2, 4, e},
		{d, c, 2, b, 1, e, n, 9, 4, a, 3, f, 7, 5, 8, 6},
		{e, 4, a, d, n, 7, 2, 1, 8, b, 6, c, 5, f, 9, 3},
		{9, 6, 8, n, b, 4, e, 5, f, 3, 2, 1, c, d, a, 7},
		{c, 1, f, 7, 3, 6, 8, a, 9, d, 5, e, 2, 4, b, n},
		{3, 5, b, 2, f, 9, d, c, a, n, 4, 7, e, 6, 1, 8},
		{1, a, d, c, 2, n, b, f, 6, e, 9, 4, 8, 3, 7, 5},
		{6, 9, n, 5, 7, c, 1, 4, 3, f, 8, b, d, e, 2, a},
		{2, e, 7, f, 9, 3, a, 8, n, 1, d, 5, b, c, 6, 4},
		{b, 8, 3, 4, d, 5, 6, e, c, 7, a, 2, 9, 1, n, f},
	};
	int **sudoku = (int **)malloc(16 * sizeof(int *));
	for (int i = 0; i < 16; i++)
	{
		sudoku[i] = (int *)malloc(16 * sizeof(int));
		for (int j = 0; j < 16; j++)
		{
			sudoku[i][j] = _hexa[i][j];
		}
	}
	int **solved = (int **)malloc(16 * sizeof(int *));
	for (int i = 0; i < 16; i++)
	{
		solved[i] = (int *)malloc(16 * sizeof(int));
		for (int j = 0; j < 16; j++)
		{
			solved[i][j] = _hexa_solved[i][j];
		}
	}
	*/

	// puts numbers back in original image
	char dirname[30];
	cleanPath(filename, dirname);
	Image **digits = loadCells(sudoku, dirname);
	for (int i = 0; i < nb_cells; i++)
	{
		Image *cell = digits[i];
		toRGBA(cell);
		for (uc n = 0; n < 4; n++)
		{
			uc *channel = cell->channels[n];
			uc *new_channel = newChannel(384 * 384);
			// init to 0 (transparent)
			for (int i = 0; i < 384 * 384; i++)
			{
				new_channel[i] = 0;
			}
			// add borders of 64 pixels on each side
			for (int y = 0; y < 256; y++)
			{
				for (int x = 0; x < 256; x++)
				{
					new_channel[(y + 64) * 384 + (x + 64)] = channel[y * 256 + x];
				}
			}
			free(channel);
			cell->channels[n] = new_channel;
		}
		cell->width = 384;
		cell->height = 384;
	}
	// Image **digits = loadCells((int **)hexa, dirname);
	for (int j = 0; j < nb_cells; j++)
	{
		for (int i = 0; i < nb_cells; i++)
		{
			// if (!hexa[j][i])
			if (!sudoku[j][i] && solved[j][i])
			{
				int n = solved[j][i];
				// int n = hexa_solved[j][i];
				placeDigit(final, digits[n - 1], quad, coords_x[i], coords_y[j]);
			}
		}
	}
	free(coords_x);
	free(coords_y);
	displayImage(final, "With numbers placed");

	freeQuad(quad);
	freeImage(image);
	freeImage(final);
	for (int i = 0; i < nb_cells; i++)
	{
		free(sudoku[i]);
		free(solved[i]);
		freeImage(digits[i]);
	}
	free(sudoku);
	free(solved);
	free(digits);
	return EXIT_SUCCESS;
}
