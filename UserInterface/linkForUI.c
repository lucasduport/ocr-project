#include <sys/stat.h>
#include "linkForUI.h"
#include "imageGestion.h"
#include "widgetGestion.h"

double *centerInput(const char *path)
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

void fastSolving(GtkLabel *upload_warn, char *foldername)
{
	char **filenames = getFilenamesInDir(foldername);
	if (filenames == NULL)
	{
		return;
	}
	struct stat st_ = {0};
	if (stat("fastSolved/", &st_) == -1)
		mkdir("fastSolved/", 448);
	st fileCount = 0;
	st fileSuccessedCount = 0;
	for (char *filename = filenames[fileCount]; filename != NULL;
		 fileCount++, filename = filenames[fileCount])
	{
		printf("Fast solving %s\n", filename);
		Image *final = openImage(filename, 4);
		autoResize(final, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT);

		Image *image = copyImage(final);
		toGrey(image);
		Image *to_extract = copyImage(image);

		gaussianBlur(image);
		calibrateImage(image, 150, 255);
		sobelFilter(image);
		gaussianBlur(image);
		thresholdToUpper(image, 24);

		Quad *quad = detectGrid(image);
		if (quad == NULL)
		{
			freeImage(final);
			freeImage(image);
			freeImage(to_extract);
			free(filename);
			continue;
		}

		Image *extracted = extractGrid(to_extract, quad, 1440, 1440);
		invertImage(extracted);
		freeImage(to_extract);
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
		saveCells(
			extracted, border_size, coords_x, coords_y, filename_stripped);
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
		int **sudoku = (int **)malloc(nb_cells * sizeof(int *));
		for (int i = 0; i < nb_cells; i++)
		{
			sudoku[i] = (int *)malloc(nb_cells * sizeof(int));
		}
		Network *net = (Network *)malloc(sizeof(Network));
		switch (nb_cells)
		{
			case 9:
				Network_Load(
					net, "../NeuralNetwork/TrainedNetwork/"
						 "NeuralNetData_3layers_OCR-Biased_100.0.dnn");
				break;
			case 16:
				Network_Load(
					net, "../NeuralNetwork/TrainedNetwork/"
						 "NeuralNetData_3layers_OCR-TEXA-Biased_100.0.dnn");
				break;
			default:
				errx(1, "Unsupported grid size");
				break;
		}
		float *results[nb_cells * nb_cells];
		for (int i = 0; i < nb_cells; i++)
		{
			for (int j = 0; j < nb_cells; j++)
			{
				char path[40];
				sprintf(path, "board_%s/%02d_%02d.png", filename_stripped,
					i + 1, j + 1);
				double *input = centerInput(path);
				results[nb_cells * j + i] = Network_Predict(net, input, 784);
				free(input);
				int imax = 0;
				float max = 0;
				for (int k = 0; k < 10; k++)
				{
					float proba = results[j * nb_cells + i][k];
					if (proba >= max)
					{
						max = proba;
						imax = k;
					}
				}
				sudoku[j][i] = imax;
				free(results[nb_cells * j + i]);
			}
		}
		Network_Purge(net);
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
				solvable = solver(solved);
				break;
			case 16:
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
				break;
		}
		if (!solvable)
		{
			freeImage(final);
			freeImage(image);
			free(filename);
			freeQuad(quad);
			free(coords_x);
			free(coords_y);
			for (int i = 0; i < nb_cells; i++)
			{
				free(sudoku[i]);
				free(solved[i]);
			}
			free(sudoku);
			free(solved);
			char path[50];
			sprintf(path, "board_%s", filename_stripped);
			rmDir(path);
			continue;
		}
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
				for (int i = 0; i < 384 * 384; i++)
				{
					new_channel[i] = 0;
				}
				for (int y = 0; y < 256; y++)
				{
					for (int x = 0; x < 256; x++)
					{
						new_channel[(y + 64) * 384 + (x + 64)]
							= channel[y * 256 + x];
					}
				}
				free(channel);
				cell->channels[n] = new_channel;
			}
			cell->width = 384;
			cell->height = 384;
		}
		for (int j = 0; j < nb_cells; j++)
		{
			for (int i = 0; i < nb_cells; i++)
			{
				if (!sudoku[j][i] && solved[j][i])
				{
					int n = solved[j][i];
					placeDigit(
						final, digits[n - 1], quad, coords_x[i], coords_y[j]);
				}
			}
		}
		free(coords_x);
		free(coords_y);

		char saveName[50];
		sprintf(saveName, "fastSolved/%s_solved.png", filename_stripped);
		autoResize(
			final, WINDOW_WIDTH * IMAGE_RATIO, WINDOW_HEIGHT * IMAGE_RATIO);
		saveImage(final, saveName);
		printf("Saved to %s\n", saveName);

		// remove saved cells (not needed when fastSvolving)
		sprintf(saveName, "board_%s", filename_stripped);
		rmDir(saveName);
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
		free(filename);
		fileSuccessedCount++;
	}
	free(filenames);
	char toPrint[50];
	sprintf(toPrint, "Solved %zu grids on %zu files", fileSuccessedCount,
		fileCount);
	displayColoredText(upload_warn, toPrint, "green");
}

//------------------------------------------------------------

//------------------------------------------------------------
gboolean getSolvedImage(gpointer data)
{
	Menu *menu = (Menu *)data;
	menu->solvedImage = NULL;

	char *filename = menu->originPath;

	Image *final = copyImage(menu->originImage);
	autoResize(final, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT);

	Image *image = copyImage(final);
	toGrey(image);
	Image *to_extract = copyImage(image);
	gaussianBlur(image);
	calibrateImage(image, 150, 255);
	sobelFilter(image);
	gaussianBlur(image);
	thresholdToUpper(image, 24);

	Quad *quad = detectGrid(image);
	if (quad == NULL)
	{
		displayColoredText(
			menu->filters_warn_label, "No grid detected", "red");
		return FALSE;
	}
	Image *extracted = extractGrid(to_extract, quad, 1440, 1440);
	invertImage(extracted);
	freeImage(to_extract);
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
	int **sudoku = (int **)malloc(nb_cells * sizeof(int *));
	for (int i = 0; i < nb_cells; i++)
	{
		sudoku[i] = (int *)malloc(nb_cells * sizeof(int));
	}
	Network *net = (Network *)malloc(sizeof(Network));
	switch (nb_cells)
	{
		case 9:
			Network_Load(net, "../NeuralNetwork/TrainedNetwork/"
							  "NeuralNetData_3layers_OCR-Biased_100.0.dnn");
			break;
		case 16:
			Network_Load(
				net, "../NeuralNetwork/TrainedNetwork/"
					 "NeuralNetData_3layers_OCR-TEXA-Biased_100.0.dnn");
			break;
		default:
			errx(1, "Unsupported grid size");
			break;
	}
	float *results[nb_cells * nb_cells];
	for (int i = 0; i < nb_cells; i++)
	{
		for (int j = 0; j < nb_cells; j++)
		{
			char path[40];
			sprintf(path, "board_%s/%02d_%02d.png", filename_stripped, i + 1,
				j + 1);
			double *input = centerInput(path);
			results[nb_cells * j + i] = Network_Predict(net, input, 784);
			free(input);
			int imax = 0;
			float max = 0;
			// printf("[");
			for (int k = 0; k < 10; k++)
			{
				float proba = results[j * nb_cells + i][k];
				if (proba >= max)
				{
					max = proba;
					imax = k;
				}
			}
			sudoku[j][i] = imax;
			free(results[nb_cells * j + i]);
		}
	}
	Network_Purge(net);
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
			solvable = solver(solved);
			break;
		case 16:
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
			break;
	}
	if (!solvable)
	{
		freeImage(final);
		freeImage(image);
		freeQuad(quad);
		free(coords_x);
		free(coords_y);
		for (int i = 0; i < nb_cells; i++)
		{
			free(sudoku[i]);
			free(solved[i]);
		}
		free(sudoku);
		free(solved);
		displayColoredText(
			menu->filters_warn_label, "No solution found", "red");
		return FALSE;
	}
	char dirname[30];
	cleanPath(filename, dirname);
	// printf("dirname = %s\n", dirname);
	Image **digits = loadCells(sudoku, dirname);
	for (int i = 0; i < nb_cells; i++)
	{
		Image *cell = digits[i];
		toRGBA(cell);
		for (uc n = 0; n < 4; n++)
		{
			uc *channel = cell->channels[n];
			uc *new_channel = newChannel(384 * 384);
			for (int i = 0; i < 384 * 384; i++)
			{
				new_channel[i] = 0;
			}
			for (int y = 0; y < 256; y++)
			{
				for (int x = 0; x < 256; x++)
				{
					new_channel[(y + 64) * 384 + (x + 64)]
						= channel[y * 256 + x];
				}
			}
			free(channel);
			cell->channels[n] = new_channel;
		}
		cell->width = 384;
		cell->height = 384;
	}
	for (int j = 0; j < nb_cells; j++)
	{
		for (int i = 0; i < nb_cells; i++)
		{
			if (!sudoku[j][i] && solved[j][i])
			{
				int n = solved[j][i];
				placeDigit(
					final, digits[n - 1], quad, coords_x[i], coords_y[j]);
			}
		}
	}
	free(coords_x);
	free(coords_y);
	if (menu->solvedImage)
		freeImage(menu->solvedImage);
	menu->solvedImage = copyImage(final);
	if (menu->redimImage)
		freeImage(menu->redimImage);
	autoResize(final, WINDOW_WIDTH * IMAGE_RATIO, WINDOW_HEIGHT * IMAGE_RATIO);
	menu->redimImage = final;
	freeQuad(quad);
	freeImage(image);
	for (int i = 0; i < nb_cells; i++)
	{
		free(sudoku[i]);
		free(solved[i]);
		freeImage(digits[i]);
	}
	free(sudoku);
	free(solved);
	free(digits);
	return FALSE;
}