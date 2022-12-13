#if 0

#	include <SDL2/SDL.h>
#	include <SDL2/SDL_image.h>
#	include <err.h>
#	include <stdio.h>
#	include <string.h>
#	include "cellExtraction.h"
#	include "display.h"
#	include "filters.h"
#	include "hough.h"
#	include "saveImage.h"
#	include "tools.h"
#	include "transformImage.h"

// function directly copied from ../Solver/main.c
// cannot import it beacause of the presence of a main function
// also renamed because SDL2 doesn't like "read" as a function
int **readSudoku(const char *filename)
{

	int **array = malloc(9 * sizeof(int *));
	for (int i = 0; i < 9; i++)
		array[i] = (int *)malloc(9 * sizeof(int));
	if (!array)
		return NULL;
	FILE *file = fopen(filename, "r");
	if (!file)
		errx(1, "Couldn't open file \"%s\"", filename);
	size_t count;
	char buffer = '0';
	int value;
	size_t i = 0;
	size_t j = 0;

	do
	{
		count = fread(&buffer, 1, 1, file);
		if (buffer == '.')
			value = 0;
		else if (buffer >= '0' && buffer <= '9')
			value = buffer - '0';
		else
		{
			if (buffer == '\n' && j)
			{
				i++;
				j = 0;
			}
			continue;
		}
		array[i][j] = value;
		j++;
	} while (count != 0);
	fclose(file);
	return array;
}

void init()
{
	initTrig();
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		errx(EXIT_FAILURE, "%s", SDL_GetError());
}

void printHelp(char *exeName)
{
	printf("Usage: %s <command> <filename> [options]\n", exeName);
	printf("\t-h, --help\t\t\t"
		   "prints this help message\n");
	printf("\t-r, --rotate <image> <angle>\t"
		   "rotate the image <image> with the angle <angle>.\n");
	printf("\t-R, --rotateView <image>\t"
		   "rotate the image <image> with a preview (use arrow keys).\n");
	printf("\t-d, --demo <image>\t\t"
		   "see full demo.\n");
	printf("\t-t, --test <image> [options]\t"
		   "test the image <image> with the given options.\n");
}

int missingArg(char *exeName, char *command)
{
	printf("Missing argument for %s.\n", command);
	printHelp(exeName);
	return 1;
}

void exeRotate(char *filename, int angle)
{
	Image *image = openImage(filename, 1);
	Image *rotated = rotateImage(image, angle, 0);
	char filenameStripped[30];
	cleanPath(filename, filenameStripped);
	char destName[40];
	sprintf(destName, "%s_r%i.png", filenameStripped, angle);
	saveImage(rotated, destName);
	freeImage(rotated);
	freeImage(image);
}

void exeRotateView(char *filename)
{
	Image *image = openImage(filename, 1);
	int theta = rotateWithView(image);
	Image *rotated = rotateImage(image, theta, 0);
	freeImage(image);
	char filenameStripped[30];
	cleanPath(filename, filenameStripped);
	char destName[40];
	sprintf(destName, "%s_r%i.png", filenameStripped, theta);
	saveImage(rotated, destName);
	freeImage(rotated);
}

void exeDemo(char *filename)
{
	// open image
	Image *image = openImage(filename, 1);
	autoResize(image, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT);
	// rotate image
	int theta = rotateWithView(image);
	Image *rotated = rotateImage(image, theta, 255);
	Image *copy = copyImage(rotated);
	freeImage(image);
	// preprocess image
	saturateImage(rotated);
	displayImage(rotated, "Saturated");
	// detect grid
	Quad *quad = detectGrid(rotated);
	if (quad == NULL)
	{
		freeImage(rotated);
		errx(1, "No grid detected.");
	}
	// display results
	showQuad(rotated, quad, 0, 255, 0);
	Image *extracted = extractGrid(copy, quad, 9 * CELLSIZE, 9 * CELLSIZE);
	freeImage(copy);
	freeImage(rotated);
	freeQuad(quad);
	thresholdCells(extracted);
	displayImage(extracted, "Extracted grid");
	// save image
	char filenameStripped[30];
	cleanPath(filename, filenameStripped);
	// remove for testing
	//saveBoard(extracted, filenameStripped);
	freeImage(extracted);
}

void exeTest(char *filename)
{
	// pre treatment
	Image *image = openImage(filename, 1);
	autoResize(image, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT);
	displayImage(image, "Original image");
	gaussianBlur(image);
	calibrateImage(image, 200, 255);
	displayImage(image, "Saturated");
	sobelFilter(image);
	displayImage(image, "Sobel");
	gaussianBlur(image);
	thresholdToUpper(image, 230);
	displayImage(image, "Calibrate");

	// detect
	Quad *quad = detectGrid(image);
	if (quad == NULL)
		errx(1, "No grid detected.");
	// display results
	showQuad(image, quad, 0, 255, 0);
	Image *extracted = extractGrid(image, quad, 9 * CELLSIZE, 9 * CELLSIZE);
	displayImage(extracted, "Extracted grid");

	freeImage(image);
	freeQuad(quad);
	freeImage(extracted);
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

void exeDigit(char *filename)
{
	Image *final = openImage(filename, 4);
	autoResize(final, IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT);
	Image *image = copyImage(final);
	toGrey(image);
	Image *to_extract = copyImage(image);

	// preprocessing
	displayImage(image, "Original image");
	gaussianBlur(image);
	calibrateImage(image, 200, 255);
	displayImage(image, "Saturated");
	sobelFilter(image);
	displayImage(image, "Sobel");
	gaussianBlur(image);
	thresholdToUpper(image, 32);
	displayImage(image, "Calibrate");

	// detect grid
	Quad *quad = detectGrid(image);
	if (quad == NULL)
		errx(1, "No grid detected.");

	// display results
	showQuad(image, quad, 0, 255, 0);
	Image *extracted = extractGrid(to_extract, quad, 9 * CELLSIZE, 9 * CELLSIZE);
	freeImage(to_extract);
	invertImage(extracted);
	displayImage(extracted, "Extracted grid");

	// save image
	char filenameStripped[30];
	cleanPath(filename, filenameStripped);
	//saveBoard(extracted, filenameStripped);
	saveCells(extracted, CELLSIZE, 5, filenameStripped);
	freeImage(extracted);

	int **sudoku = readSudoku("../Solver/grid_00");
	int **solved = readSudoku("../Solver/grid_00.result");
	Image *digits[9];
	char path[30];
	for (int i = 0; i < 9; i++)
		digits[i] = NULL;
	/*
	for (int i = 1; i <= 9; i++) {
		sprintf(path, "Numbers/_%d.png", i);
		digits[i - 1] = openImageRGBA(path);
	}
	*/

	// puts numbers back in original image
	// Quad *grid = rotateQuad(quad, theta, image, rotated);
	for (int j = 0; j < 9; j++)
		for (int i = 0; i < 9; i++)
			if (!sudoku[j][i])
			{
				int n = solved[j][i];
				if (!digits[n - 1])
				{
					// loads from cells
					int i, j;
					searchDigit(sudoku, n, &i, &j);
					if (i < 0 || j < 0)
						sprintf(path, "Numbers/_%d.png", n);
					else
						sprintf(
							path, "board_image_03/%d_%d.png", i + 1, j + 1);
					// TODO: open the correct dir
					digits[n - 1] = openImage(path, 4);
					if (digits[n - 1]->height != 256)
						resizeImage(digits[n - 1], 256, 256);
					invertImage(digits[n - 1]);
					createAlpha(digits[n - 1], 0, 127);
					toColor(digits[n - 1], 0, 255, 0);
				}
				placeDigit(final, digits[n - 1], quad, i, j);
			}
	displayImage(final, "With numbers placed");

	freeQuad(quad);
	freeImage(image);
	freeImage(final);
	for (st i = 0; i < 9; i++)
	{
		free(sudoku[i]);
		free(solved[i]);
		freeImage(digits[i]);
	}
	free(sudoku);
	free(solved);
}

int main(int argc, char *argv[])
{
	char *exeName = argv[0];
	if (argc < 2)
	{
		printHelp(exeName);
		return 1;
	}
	init();
	for (int i = 1; i < argc; i++)
	{
		char *command = argv[i];
		if (!strcmp(command, "-h") || !strcmp(command, "--help"))
		{
			printHelp(exeName);
			return 0;
		}
		else if (!strcmp(command, "-r") || !strcmp(command, "--rotate"))
		{
			if (i + 2 >= argc)
				return missingArg(exeName, command);
			char *filename = argv[++i];
			int angle = atoi(argv[++i]);
			exeRotate(filename, angle);
		}
		else if (!strcmp(command, "-R") || !strcmp(command, "--rotateView"))
		{
			if (i + 1 >= argc)
				return missingArg(exeName, command);
			char *filename = argv[++i];
			exeRotateView(filename);
		}
		else if (!strcmp(command, "-d") || !strcmp(command, "--demo"))
		{
			if (i + 1 >= argc)
				return missingArg(exeName, command);
			char *filename = argv[++i];
			exeDemo(filename);
		}
		else if (!strcmp(command, "-t") || !strcmp(command, "--test"))
		{
			if (i + 1 >= argc)
				return missingArg(exeName, command);
			char *filename = argv[++i];
			exeDigit(filename);
		}
		else
		{
			printf("Unknown command %s\n", command);
			printHelp(exeName);
			return 1;
		}
	}
	IMG_Quit();
	SDL_Quit();
	return 0;
}

#endif // 0
