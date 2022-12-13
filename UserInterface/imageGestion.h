#pragma once

#include "interactions.h"
#include "ui.h"
#include "widgetGestion.h"

void newSudokuImage(Menu *menu, char *filename);

void SudokuImageFromImage(Menu *menu, Image *image);

void destroySudokuImage(Menu *menu);

Image *actualImage(Menu *menu);

void refreshImage(GtkWidget *widget, gpointer data);

void tmpSaveImage(Image *image, char *destname);

void loadImage(Menu *menu, char *filename);

gboolean isLoadableImage(char *path);

Image *solveForUI(Menu *menu);