#include <sys/stat.h>
#include <sys/types.h>
#include "imageGestion.h"

unsigned int cpt = 0;
void newSudokuImage(Menu *menu, char *filename)
{
	Image *image = openImage(filename, 4);
	menu->originImage = image;
	menu->originPath = filename;
	menu->redimImage = copyImage(image);
	autoResize(menu->redimImage, WINDOW_WIDTH * IMAGE_RATIO,
		WINDOW_HEIGHT * IMAGE_RATIO);
	SudokuImageFromImage(menu, menu->redimImage);
}

void SudokuImageFromImage(Menu *menu, Image *image)
{
	if (menu->sudoku_image != NULL)
		gtk_widget_destroy(menu->sudoku_image);

	char destname[100];
	tmpSaveImage(image, destname);

	menu->sudoku_image = gtk_image_new_from_file(destname);
	gtk_widget_show(menu->sudoku_image);
	gtk_container_add(GTK_CONTAINER(menu->fixed1), menu->sudoku_image);
	gtk_fixed_move(GTK_FIXED(menu->fixed1), menu->sudoku_image,
		(WINDOW_WIDTH - 245 - image->width) / 2, (WINDOW_HEIGHT - 450) / 2);
	menu->imageOrigin->x = (WINDOW_WIDTH - 245 - image->width) / 2;
	menu->imageOrigin->y = (WINDOW_HEIGHT - 450) / 2;
}

void destroySudokuImage(Menu *menu)
{
	if (menu->originImage != NULL)
	{
		freeImage(menu->originImage);
		menu->originImage = NULL;
	}
	if (menu->sudoku_image != NULL)
	{
		gtk_widget_destroy(menu->sudoku_image);
		menu->sudoku_image = NULL;
	}
	if (menu->originPath != NULL)
	{
		g_free(menu->originPath);
		menu->originPath = NULL;
	}
	if (menu->redimImage != NULL)
	{
		freeImage(menu->redimImage);
		menu->redimImage = NULL;
	}
	if (menu->solvedImage != NULL)
	{
		freeImage(menu->solvedImage);
		menu->solvedImage = NULL;
	}
	return;
}

Image *actualImage(Menu *menu)
{
	Image *toPrint = copyImage(menu->redimImage);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(menu->gaussian_button)))
		gaussianBlur(toPrint);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(menu->sobel_button)))
		sobelFilter(toPrint);
	return toPrint;
}

void refreshImage(GtkWidget *widget, gpointer data)
{
	(void)widget;
	// avoid warning about unused parameter
	Menu *menu = (Menu *)data;
	Image *toPrint = actualImage(menu);
	SudokuImageFromImage(menu, toPrint);
	freeImage(toPrint);
}

void tmpSaveImage(Image *image, char *destname)
{
	SDL_Surface *surface = imageToSurface(image);
	struct stat st_ = {0};
	if (stat("tmpImg/", &st_) == -1)
		mkdir("tmpImg/", 448);
	sprintf(destname, "tmpImg/%d.png", cpt++);
	if (IMG_SavePNG(surface, destname) != 0)
		errx(1, "Error while saving temp image");
	SDL_FreeSurface(surface);
	return;
}

void loadImage(Menu *menu, char *filename)
{
	if (isLoadableImage(filename) == FALSE)
	{
		displayColoredText(
			menu->upload_warn_label, "This format is not loadable", "red");
		return;
	}
	gtk_window_set_title(menu->window, filename);
	newSudokuImage(menu, filename);
	GtkWidget *to_hide[] = {menu->file_select_grid, NULL};
	GtkWidget *to_show[] = {menu->filters_grid, menu->back_to_menu, NULL};
	widgetCleanup(to_hide, to_show);
}

gboolean isLoadableImage(char *path)
{
	SDL_Surface *test = IMG_Load(path);
	if (test == NULL)
		return FALSE;
	SDL_FreeSurface(test);
	return TRUE;
}