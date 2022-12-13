#include <dirent.h>
#include <string.h>
#include "interactions.h"
#include "linkForUI.h"

#define CC_PIXEL_SIZE 12;

void on_upload_button_clicked(GtkWidget *widget, gpointer data)
{
	(void)widget;
	// avoid warning about unused parameter
	Menu *menu = (Menu *)data;
	GtkWidget *dialog;
	gint res;
	dialog = gtk_file_chooser_dialog_new("Choose image to open",
		GTK_WINDOW(menu->window), GTK_FILE_CHOOSER_ACTION_OPEN, ("Cancel"),
		GTK_RESPONSE_CANCEL, ("This one"), GTK_RESPONSE_ACCEPT, NULL);
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		char *filename = gtk_file_chooser_get_filename(chooser);
		if (menu->originPath != NULL)
			free(menu->originPath);
		if (isRegFile(filename))
		{
			loadImage(menu, filename);
		}
		else
		{
			fastSolving(menu->upload_warn_label, filename);
		}
	}
	else
	{
		displayColoredText(menu->upload_warn_label, "Upload canceled", "red");
	}
	gtk_widget_destroy(dialog);
}

// gtk entry
void on_upload_entry_activate(GtkWidget *widget, gpointer data)
{
	Menu *menu = (Menu *)data;
	const char *path = gtk_entry_get_text(GTK_ENTRY(widget));
	char *filename = (char *)path;
	if (access(filename, F_OK) == 0)
	{
		if (menu->originPath != NULL)
			free(menu->originPath);
		if (isRegFile(filename))
		{
			loadImage(menu, filename);
		}
		else
		{
			fastSolving(menu->upload_warn_label, filename);
		}
	}
	else
	{
		displayColoredText(menu->upload_warn_label, "File not found", "red");
	}
	return;
}

void on_back_to_menu_button_clicked(GtkWidget *widget, gpointer data)
{
	Menu *menu = (Menu *)data;
	gtk_window_set_title(menu->window, "OCR Project");
	GtkWidget *to_revive[] = {menu->file_select_grid, NULL};
	GtkWidget *to_destroy[]
		= {menu->sudoku_image, widget, menu->filters_grid, NULL};
	widgetCleanup(to_destroy, to_revive);
	resetFilters(menu);
	destroySudokuImage(menu);
	return;
}

void on_save_clicked(GtkWidget *widget, gpointer data)
{
	(void)widget;
	// avoid warning about unused parameter
	Menu *menu = (Menu *)data;
	GtkWidget *dialog;
	GtkFileChooser *chooser;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
	gint res;
	dialog = gtk_file_chooser_dialog_new("Save File", menu->window, action,
		("Cancel"), GTK_RESPONSE_CANCEL, ("Save"), GTK_RESPONSE_ACCEPT, NULL);
	chooser = GTK_FILE_CHOOSER(dialog);

	gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
	gtk_file_chooser_set_current_name(chooser, "image.png");
	gtk_file_chooser_set_create_folders(chooser, TRUE);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename(chooser);
		if (menu->solvedImage == NULL)
		{
			Image *toSave = actualImage(menu);
			saveImage(toSave, filename);
			freeImage(toSave);
			refreshImage(widget, data);
			g_free(filename);
		}
		else
		{
			saveImage(menu->solvedImage, filename);
			g_free(filename);
		}
	}
	gtk_widget_destroy(dialog);
}

void on_autoDetect_clicked(GtkWidget *widget, gpointer data)
{
	Menu *menu = (Menu *)data;
	Image *betterForLines = copyImage(menu->redimImage);
	toGrey(betterForLines);
	gaussianBlur(betterForLines);
	sobelFilter(betterForLines);
	thresholdToUpper(betterForLines, 16);
	Quad *quad = detectGrid(betterForLines);
	if (quad == NULL)
	{
		displayColoredText(
			menu->filters_warn_label, "Sudoku grid not found", "red");
		freeImage(betterForLines);
		return;
	}
	else
	{
		Image *extracted = extractGrid(menu->redimImage, quad,
			menu->redimImage->width, menu->redimImage->height);
		freeImage(menu->redimImage);
		menu->redimImage = extracted;
		SudokuImageFromImage(menu, extracted);
		GtkWidget *toNoSens[]
			= {widget, GTK_WIDGET(menu->grayscale_button), NULL};
		gtk_toggle_button_set_active(
			GTK_TOGGLE_BUTTON(menu->grayscale_button), TRUE);
		changeSensivityWidgets(toNoSens, 0);
		freeQuad(quad);
		freeImage(betterForLines);
	}
	refreshImage(widget, data);
}

void resetFilters(Menu *menu)
{
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(menu->grayscale_button), FALSE);
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(menu->gaussian_button), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(menu->sobel_button), FALSE);

	GtkWidget *toSens[] = {GTK_WIDGET(menu->autoDetect_button),
		GTK_WIDGET(menu->grayscale_button), GTK_WIDGET(menu->solve_button),
		NULL};
	changeSensivityWidgets(toSens, 1);
	if (menu->solvedImage != NULL)
	{
		freeImage(menu->solvedImage);
		menu->solvedImage = NULL;
	}
}

void on_grayscale_toggled(GtkWidget *widget, gpointer data)
{
	Menu *menu = (Menu *)data;
	gtk_widget_set_sensitive(widget, 0);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
	{
		toGrey(menu->redimImage);
		refreshImage(widget, data);
	}
}

void on_resetFilters_clicked(GtkWidget *widget, gpointer data)
{
	Menu *menu = (Menu *)data;
	resetFilters(menu);
	freeImage(menu->redimImage);
	menu->redimImage = copyImage(menu->originImage);
	autoResize(menu->redimImage, WINDOW_WIDTH * IMAGE_RATIO,
		WINDOW_HEIGHT * IMAGE_RATIO);
	refreshImage(widget, data);
}

void on_crop_corners_move(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	Menu *menu = (Menu *)data;
	gint actualX, actualY;
	gtk_widget_translate_coordinates(
		widget, gtk_widget_get_toplevel(widget), 0, 0, &actualX, &actualY);
	gint mouseX = event->button.x - CC_PIXEL_SIZE;
	gint mouseY = event->button.y - CC_PIXEL_SIZE;
	gint newX = actualX + mouseX, newY = actualY + mouseY;
	gint imOrgX = menu->imageOrigin->x - CC_PIXEL_SIZE;
	gint imOrgY = menu->imageOrigin->y - CC_PIXEL_SIZE;
	gint imWidth = menu->redimImage->width,
		 imHeight = menu->redimImage->height;
	gint gapBtwnCorners = CC_PIXEL_SIZE;
	char *p = strchr(gtk_widget_get_name(widget), '1');
	if (p != NULL)
	{
		newX = CLAMP(newX, imOrgX, imOrgX + imWidth / 2 - gapBtwnCorners);
		newY = CLAMP(newY, imOrgY, imOrgY + imHeight / 2 - gapBtwnCorners);
	}
	p = strchr(gtk_widget_get_name(widget), '2');
	if (p != NULL)
	{
		newX = CLAMP(
			newX, imOrgX + imWidth / 2 + gapBtwnCorners, imOrgX + imWidth);
		newY = CLAMP(newY, imOrgY, imOrgY + imHeight / 2 - gapBtwnCorners);
	}
	p = strchr(gtk_widget_get_name(widget), '3');
	if (p != NULL)
	{
		newX = CLAMP(
			newX, imOrgX + imWidth / 2 + gapBtwnCorners, imOrgX + imWidth);
		newY = CLAMP(
			newY, imOrgY + imHeight / 2 + gapBtwnCorners, imOrgY + imHeight);
	}
	p = strchr(gtk_widget_get_name(widget), '4');
	if (p != NULL)
	{
		newX = CLAMP(newX, imOrgX, imOrgX + imWidth / 2 - gapBtwnCorners);
		newY = CLAMP(
			newY, imOrgY + imHeight / 2 + gapBtwnCorners, imOrgY + imHeight);
	}
	gtk_fixed_move(GTK_FIXED(menu->fixed1), widget, newX, newY);
	return;
}

void on_rotate_clockwise_clicked(GtkWidget *widget, gpointer data)
{
	Menu *menu = (Menu *)data;
	Image *r = rotateImage(menu->redimImage, 90, 0);
	freeImage(menu->redimImage);
	menu->redimImage = r;
	refreshImage(widget, data);
}

void on_rotate_anticlockwise_clicked(GtkWidget *widget, gpointer data)
{
	Menu *menu = (Menu *)data;
	Image *r = rotateImage(menu->redimImage, 270, 0);
	freeImage(menu->redimImage);
	menu->redimImage = r;
	refreshImage(widget, data);
}

void on_manuDetect_clicked(GtkWidget *widget, gpointer data)
{
	Menu *menu = (Menu *)data;
	GtkEventBox *crop_corner1 = menu->crop_corners[1];
	GtkEventBox *crop_corner2 = menu->crop_corners[2];
	GtkEventBox *crop_corner3 = menu->crop_corners[3];
	GtkEventBox *crop_corner4 = menu->crop_corners[4];
	GtkWidget *toModifSens[] = {GTK_WIDGET(menu->grayscale_button),
		GTK_WIDGET(menu->gaussian_button), GTK_WIDGET(menu->sobel_button),
		GTK_WIDGET(menu->back_to_menu), GTK_WIDGET(menu->sobel_button),
		GTK_WIDGET(menu->save_button), GTK_WIDGET(menu->save_button),
		GTK_WIDGET(menu->rotate_left_button),
		GTK_WIDGET(menu->rotate_right_button),
		GTK_WIDGET(menu->autoDetect_button),
		GTK_WIDGET(menu->resetFilters_button), GTK_WIDGET(menu->solve_button),
		NULL};
	if (strcmp(gtk_label_get_text(GTK_LABEL(menu->manuDetect_label)),
			"Manual crop")
		== 0)
	{
		changeSensivityWidgets(toModifSens, 0);

		gtk_label_set_text(GTK_LABEL(menu->manuDetect_label), "Apply");
		gint imOrgX = menu->imageOrigin->x - CC_PIXEL_SIZE;
		gint imOrgY = menu->imageOrigin->y - CC_PIXEL_SIZE;
		gint imWidth = menu->redimImage->width,
			 imHeight = menu->redimImage->height;

		gtk_container_remove(
			GTK_CONTAINER(menu->fixed1), GTK_WIDGET(crop_corner1));
		gtk_container_add(
			GTK_CONTAINER(menu->fixed1), GTK_WIDGET(crop_corner1));
		gtk_fixed_move(
			GTK_FIXED(menu->fixed1), GTK_WIDGET(crop_corner1), imOrgX, imOrgY);

		gtk_container_remove(
			GTK_CONTAINER(menu->fixed1), GTK_WIDGET(crop_corner2));
		gtk_container_add(
			GTK_CONTAINER(menu->fixed1), GTK_WIDGET(crop_corner2));
		gtk_fixed_move(GTK_FIXED(menu->fixed1), GTK_WIDGET(crop_corner2),
			imOrgX + imWidth, imOrgY);

		gtk_container_remove(
			GTK_CONTAINER(menu->fixed1), GTK_WIDGET(crop_corner3));
		gtk_container_add(
			GTK_CONTAINER(menu->fixed1), GTK_WIDGET(crop_corner3));
		gtk_fixed_move(GTK_FIXED(menu->fixed1), GTK_WIDGET(crop_corner3),
			imOrgX + imWidth, imOrgY + imHeight);

		gtk_container_remove(
			GTK_CONTAINER(menu->fixed1), GTK_WIDGET(crop_corner4));
		gtk_container_add(
			GTK_CONTAINER(menu->fixed1), GTK_WIDGET(crop_corner4));
		gtk_fixed_move(GTK_FIXED(menu->fixed1), GTK_WIDGET(crop_corner4),
			imOrgX, imOrgY + imHeight);

		GtkWidget *to_show[]
			= {GTK_WIDGET(crop_corner1), GTK_WIDGET(crop_corner2),
				GTK_WIDGET(crop_corner3), GTK_WIDGET(crop_corner4), NULL};
		widgetDisplayer(to_show);
	}
	else
	{
		changeSensivityWidgets(toModifSens, 1);
		gtk_label_set_text(GTK_LABEL(menu->manuDetect_label), "Manual crop");
		Point *p1, *p2, *p3, *p4;
		gint imOrgX = menu->imageOrigin->x - CC_PIXEL_SIZE;
		gint imOrgY = menu->imageOrigin->y - CC_PIXEL_SIZE;
		gint xCC, yCC;
		gtk_widget_translate_coordinates(GTK_WIDGET(crop_corner1),
			gtk_widget_get_toplevel(GTK_WIDGET(crop_corner1)), 0, 0, &xCC,
			&yCC);
		p1 = newPoint((st)(xCC - imOrgX), (st)(yCC - imOrgY));
		gtk_widget_translate_coordinates(GTK_WIDGET(crop_corner2),
			gtk_widget_get_toplevel(GTK_WIDGET(crop_corner2)), 0, 0, &xCC,
			&yCC);
		p2 = newPoint((st)(xCC - imOrgX), (st)(yCC - imOrgY));
		gtk_widget_translate_coordinates(GTK_WIDGET(crop_corner3),
			gtk_widget_get_toplevel(GTK_WIDGET(crop_corner3)), 0, 0, &xCC,
			&yCC);
		p3 = newPoint((st)(xCC - imOrgX), (st)(yCC - imOrgY));
		gtk_widget_translate_coordinates(GTK_WIDGET(crop_corner4),
			gtk_widget_get_toplevel(GTK_WIDGET(crop_corner4)), 0, 0, &xCC,
			&yCC);
		p4 = newPoint((st)(xCC - imOrgX), (st)(yCC - imOrgY));

		Quad *quad = newQuad(
			p1, p2, p4, p3); // quad struct uses another order of points
		Image *cropped = extractGrid(menu->redimImage, quad,
			menu->redimImage->width, menu->redimImage->height);
		freeImage(menu->redimImage);
		menu->redimImage = cropped;
		freeQuad(quad);
		leave_manual_crop(menu);
		refreshImage(widget, data);
	}
}

void leave_manual_crop(Menu *menu)
{
	GtkEventBox *crop_corner1 = menu->crop_corners[1];
	GtkEventBox *crop_corner2 = menu->crop_corners[2];
	GtkEventBox *crop_corner3 = menu->crop_corners[3];
	GtkEventBox *crop_corner4 = menu->crop_corners[4];
	GtkWidget *to_hide[] = {GTK_WIDGET(crop_corner1), GTK_WIDGET(crop_corner2),
		GTK_WIDGET(crop_corner3), GTK_WIDGET(crop_corner4), NULL};
	widgetHider(to_hide);
}

void upload_drag_data_received(GtkWidget *widget, GdkDragContext *context,
	gint x, gint y, GtkSelectionData *data, guint info, guint time,
	gpointer userdata)
{
	(void)widget;
	(void)x;
	(void)y;
	(void)info;
	// avoid warning about unused parameters
	Menu *menu = (Menu *)userdata;
	if ((gtk_selection_data_get_length(data) >= 0)
		&& (gtk_selection_data_get_format(data) == 8))
	{
		gchar **uris = g_uri_list_extract_uris(
			(const gchar *)gtk_selection_data_get_data(data));
		if (uris)
		{
			gchar *filename = g_filename_from_uri(uris[0], NULL, NULL);
			if (isRegFile(filename))
			{
				loadImage(menu, filename);
			}
			else
			{
				fastSolving(menu->upload_warn_label, filename);
			}
		}
	}
	gtk_drag_finish(context, TRUE, FALSE, time);
}

void entry_drag_data_received(GtkWidget *widget, GdkDragContext *context,
	gint x, gint y, GtkSelectionData *data, guint info, guint time,
	gpointer userdata)
{
	(void)x;
	(void)y;
	(void)info;
	(void)userdata;
	// avoid warning about unused parameters
	if ((gtk_selection_data_get_length(data) >= 0)
		&& (gtk_selection_data_get_format(data) == 8))
	{
		gchar **uris = g_uri_list_extract_uris(
			(const gchar *)gtk_selection_data_get_data(data));
		if (uris)
		{
			gchar *filename = g_filename_from_uri(uris[0], NULL, NULL);
			if (filename)
			{
				gtk_entry_set_text(GTK_ENTRY(widget), filename);
			}
		}
	}
	gtk_drag_finish(context, TRUE, FALSE, time);
}

void open_folder_selector(GtkWidget *widget, gpointer data)
{
	(void)widget;
	// avoid warning about unused parameters
	Menu *menu = (Menu *)data;
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Choose a folder",
		GTK_WINDOW(menu->window), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		"_Cancel", GTK_RESPONSE_CANCEL, "_Ok", GTK_RESPONSE_ACCEPT, NULL);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		filename = gtk_file_chooser_get_filename(chooser);
		printf("TODOopen%s\n", filename);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void on_solve_clicked(GtkWidget *widget, gpointer data)
{
	(void)widget;
	// avoid warning about unused parameter
	Menu *menu = (Menu *)data;
	gtk_widget_set_sensitive(GTK_WIDGET(menu->solve_button), FALSE);
	getSolvedImage(menu);
	if (menu->solvedImage)
	{
		displayColoredText(menu->filters_warn_label, "Sudoku solved", "green");
		refreshImage(widget, data);
	}
}
/*
void on_window_resize(GtkWidget* widget, GdkEventConfigure event, gpointer
user_data)
{
	(void)event;
	// avoid warning about unused parameter
	Menu *menu = (Menu *)user_data;
	int widget_width = gtk_widget_get_allocated_width(widget);
	int widget_height = gtk_widget_get_allocated_height(widget);
	printf("Widget width: %d, height: %d\n", widget_width, widget_height);
	printf("grid %p\n", menu->file_select_grid);
	gtk_fixed_move(menu->fixed1, GTK_WIDGET(menu->file_select_grid),
(WINDOW_WIDTH - widget_width) / 2, (WINDOW_HEIGHT - widget_height) / 2);
	return;
}
*/
/*
void on_angle_slider_value_changed(GtkWidget *widget, gpointer data)
{
		//Menu *menu = (Menu *)data;
		//disconnect signal while already rotating the image to avoid spamming
		g_signal_handler_disconnect(G_OBJECT(widget), slider_handler_id);
		printf("TODOangle%d\n", (int)(gtk_range_get_value(GTK_RANGE(widget)) *
400)); connect_slider_handler(widget, data);
}
*/