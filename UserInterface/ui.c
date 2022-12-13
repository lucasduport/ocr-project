#include "ui.h"
#include "interactions.h"

void uiLaunch()
{
	//---------GTK COMPONENTS INITIALIZATION WITH GLADE FILE---------//
	GtkBuilder *builder = gtk_builder_new_from_file("main.glade");
	GtkWindow *window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
	GtkFixed *fixed1 = GTK_FIXED(gtk_builder_get_object(builder, "fixed1"));
	GtkBox *box = GTK_BOX(gtk_builder_get_object(builder, "box"));
	GtkWidget *file_select_grid
		= GTK_WIDGET(gtk_builder_get_object(builder, "file_select_grid"));
	GtkWidget *back_to_menu
		= GTK_WIDGET(gtk_builder_get_object(builder, "back_to_menu"));
	GtkWidget *upload_button
		= GTK_WIDGET(gtk_builder_get_object(builder, "upload_button"));
	GtkWidget *upload_entry
		= GTK_WIDGET(gtk_builder_get_object(builder, "upload_entry"));
	GtkWidget *filters_grid
		= GTK_WIDGET(gtk_builder_get_object(builder, "filters_grid"));

	GtkButton *resetFilters_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "resetFilters"));

	GtkButton *grayscale_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "grayscale"));
	GtkButton *gaussian_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "gaussian"));
	GtkButton *sobel_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "sobel"));

	GtkButton *rotate_left_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left"));
	GtkButton *rotate_right_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right"));

	GtkButton *autoDetect_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "auto_detect"));
	GtkButton *manuDetect_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "manu_detect"));
	GtkLabel *manuDetect_label
		= GTK_LABEL(gtk_builder_get_object(builder, "manuDetect_label"));

	GtkButton *save_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "save"));
	GtkButton *solve_button
		= GTK_BUTTON(gtk_builder_get_object(builder, "solve"));

	GtkLabel *upload_warn_label
		= GTK_LABEL(gtk_builder_get_object(builder, "upload_warn_label"));
	GtkLabel *filters_warn_label
		= GTK_LABEL(gtk_builder_get_object(builder, "filters_warn_label"));

	GtkEventBox *crop_corner1
		= GTK_EVENT_BOX(gtk_builder_get_object(builder, "crop_corner1"));
	GtkEventBox *crop_corner2
		= GTK_EVENT_BOX(gtk_builder_get_object(builder, "crop_corner2"));
	GtkEventBox *crop_corner3
		= GTK_EVENT_BOX(gtk_builder_get_object(builder, "crop_corner3"));
	GtkEventBox *crop_corner4
		= GTK_EVENT_BOX(gtk_builder_get_object(builder, "crop_corner4"));
	GtkEventBox *crop_corners[5]
		= {NULL, crop_corner1, crop_corner2, crop_corner3, crop_corner4};

	//---------MENU STRUCT INITIALIZATION---------//
	Menu *menu = malloc(sizeof(Menu));
	menu->builder = builder;
	menu->window = window;
	menu->fixed1 = fixed1;
	menu->box = box;
	menu->back_to_menu = back_to_menu;
	menu->file_select_grid = file_select_grid;

	menu->sudoku_image = NULL;
	menu->imageOrigin = newPoint(0, 0);

	menu->resetFilters_button = resetFilters_button;
	menu->grayscale_button = grayscale_button;
	menu->gaussian_button = gaussian_button;
	menu->sobel_button = sobel_button;

	menu->autoDetect_button = autoDetect_button;
	menu->manuDetect_label = manuDetect_label;
	menu->rotate_left_button = rotate_left_button;
	menu->rotate_right_button = rotate_right_button;
	menu->crop_corners = crop_corners;

	menu->save_button = save_button;
	menu->solve_button = solve_button;
	menu->filters_grid = filters_grid;

	menu->originImage = NULL;
	menu->redimImage = NULL;
	menu->originPath = NULL;
	menu->solvedImage = NULL;

	menu->upload_warn_label = upload_warn_label;
	menu->filters_warn_label = filters_warn_label;

	//-----------------SIGNALS CONNECTION-----------------//
	// Corners
	g_signal_connect(GTK_WIDGET(crop_corner1), "motion-notify-event",
		G_CALLBACK(on_crop_corners_move), menu);
	g_signal_connect(GTK_WIDGET(crop_corner2), "motion-notify-event",
		G_CALLBACK(on_crop_corners_move), menu);
	g_signal_connect(GTK_WIDGET(crop_corner3), "motion-notify-event",
		G_CALLBACK(on_crop_corners_move), menu);
	g_signal_connect(GTK_WIDGET(crop_corner4), "motion-notify-event",
		G_CALLBACK(on_crop_corners_move), menu);
	GtkTargetEntry *uri_targets = gtk_target_entry_new("text/uri-list", 0, 0);
	// Drag and drop
	g_signal_connect(
		upload_button, "clicked", G_CALLBACK(on_upload_button_clicked), menu);
	g_signal_connect(GTK_WIDGET(upload_button), "drag-data-received",
		G_CALLBACK(upload_drag_data_received), menu);
	gtk_drag_dest_set(GTK_WIDGET(upload_button), GTK_DEST_DEFAULT_ALL,
		uri_targets, 1, GDK_ACTION_COPY);

	g_signal_connect(
		upload_entry, "activate", G_CALLBACK(on_upload_entry_activate), menu);
	g_signal_connect(GTK_WIDGET(upload_entry), "drag-data-received",
		G_CALLBACK(entry_drag_data_received), menu);
	gtk_drag_dest_set(GTK_WIDGET(upload_entry), GTK_DEST_DEFAULT_ALL,
		uri_targets, 1, GDK_ACTION_COPY);
	//--------------//
	g_signal_connect(back_to_menu, "clicked",
		G_CALLBACK(on_back_to_menu_button_clicked), menu);
	g_signal_connect(resetFilters_button, "clicked",
		G_CALLBACK(on_resetFilters_clicked), menu);
	g_signal_connect(
		grayscale_button, "toggled", G_CALLBACK(on_grayscale_toggled), menu);
	g_signal_connect(
		gaussian_button, "toggled", G_CALLBACK(refreshImage), menu);
	g_signal_connect(sobel_button, "toggled", G_CALLBACK(refreshImage), menu);

	g_signal_connect(rotate_left_button, "clicked",
		G_CALLBACK(on_rotate_clockwise_clicked), menu);
	g_signal_connect(rotate_right_button, "clicked",
		G_CALLBACK(on_rotate_anticlockwise_clicked), menu);

	g_signal_connect(
		autoDetect_button, "clicked", G_CALLBACK(on_autoDetect_clicked), menu);
	g_signal_connect(
		manuDetect_button, "clicked", G_CALLBACK(on_manuDetect_clicked), menu);

	g_signal_connect(
		save_button, "clicked", G_CALLBACK(on_save_clicked), menu);
	g_signal_connect(
		solve_button, "clicked", G_CALLBACK(on_solve_clicked), menu);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	// g_signal_connect(GTK_WIDGET(window), "configure-event",
	// G_CALLBACK(on_window_resize),menu);

	//---------WINDOW  INITIALIZATION---------//
	gtk_window_set_title(window, "OCR Project");
	gtk_window_set_position(window, GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(window, WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_window_set_resizable(window, FALSE);

	//-----------------MAIN LOOP-----------------//
	gtk_main();
	//------------------ENDING-------------------//
	gtk_target_entry_free(uri_targets);
	if (menu->originImage != NULL)
	{
		freeImage(menu->originImage);
	}
	if (menu->redimImage != NULL)
	{
		freeImage(menu->redimImage);
	}
	if (menu->originPath != NULL)
	{
		g_free(menu->originPath);
	}
	if (menu->solvedImage != NULL)
	{
		freeImage(menu->solvedImage);
	}
	gtk_widget_destroy(GTK_WIDGET(window));
	free(menu->imageOrigin);
	free(menu);
	return;
}

int rmDir(const char *dir)
{
	int toReturnValue = 0;
	FTSENT *currentFiles;
	char *filesToRm[] = {(char *)dir, NULL};
	FTS *ftsp
		= fts_open(filesToRm, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
	if (!ftsp)
	{
		toReturnValue = -1;
		goto endd;
	}
	while ((currentFiles = fts_read(ftsp)))
	{
		switch (currentFiles->fts_info)
		{
			case FTS_NS:
			case FTS_DNR:
			case FTS_ERR:
				toReturnValue = -1;
				break;
			case FTS_DOT:
			case FTS_NSOK:
			case FTS_DC:
				break;
			case FTS_D:
				break;
			case FTS_SL:
			case FTS_SLNONE:
			case FTS_DEFAULT:
			case FTS_DP:
			case FTS_F:
				if (remove(currentFiles->fts_accpath) < 0)
				{
					fprintf(stderr, "%s: Failed to remove: %s\n",
						currentFiles->fts_path,
						strerror(currentFiles->fts_errno));
					toReturnValue = -1;
				}
				break;
		}
	}

endd:
	if (ftsp)
	{
		fts_close(ftsp);
	}
	return toReturnValue;
}

int isRegFile(char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode);
}

char **getFilenamesInDir(char *filename)
{
	DIR *dir = opendir(filename);
	struct dirent *entry;
	size_t count = 0;
	char **toReturn = NULL;
	while ((entry = readdir(dir)) != NULL)
	{
		char *path = malloc(
			sizeof(char) * (strlen(filename) + strlen(entry->d_name) + 2));
		strcpy(path, filename);
		strcat(path, "/");
		strcat(path, entry->d_name);
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")
			|| !isLoadableImage(path))
		{
			free(path);
			continue;
		}
		toReturn = realloc(toReturn, sizeof(char *) * (count + 2));
		toReturn[count] = path;
		count++;
	}
	if (toReturn == NULL)
	{
		return NULL;
	}
	toReturn[count] = NULL;
	closedir(dir);
	return toReturn;
}