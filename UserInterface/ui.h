#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <fts.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../ImageProcessing/cellExtraction.h"
#include "../ImageProcessing/cellsDetection.h"
#include "../ImageProcessing/centerCell.h"
#include "../ImageProcessing/display.h"
#include "../ImageProcessing/filters.h"
#include "../ImageProcessing/hough.h"
#include "../ImageProcessing/param.h"
#include "../ImageProcessing/saveImage.h"
#include "../ImageProcessing/tools.h"
#include "../ImageProcessing/transformImage.h"
#include "../NeuralNetwork/Network.h"
#include "../Solver/solver.h"
#include "../Solver/solver16.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800

#define IMAGE_RATIO 0.6

typedef struct
{
	GtkBuilder *builder;
	GtkWindow *window;
	GtkFixed *fixed1;
	GtkBox *box;
	GtkWidget *back_to_menu;
	GtkWidget *file_select_grid;

	GtkWidget *sudoku_image;
	Point *imageOrigin;

	GtkButton *resetFilters_button;
	GtkButton *grayscale_button;
	GtkButton *gaussian_button;
	GtkButton *sobel_button;

	GtkButton *autoDetect_button;
	GtkLabel *manuDetect_label;
	GtkButton *rotate_left_button;
	GtkButton *rotate_right_button;
	GtkEventBox **crop_corners;

	GtkButton *solve_button;
	GtkButton *save_button;

	GtkWidget *filters_grid;

	char *originPath;
	Image *originImage;
	Image *redimImage;
	Image *solvedImage;

	GtkLabel *upload_warn_label;
	GtkLabel *filters_warn_label;

} Menu;

void uiLaunch();
int rmDir(const char *dir);
int isRegFile(char *path);
char **getFilenamesInDir(char *filename);