#pragma once

#include <gtk/gtk.h>
#include "ui.h"

void widgetDisplayer(GtkWidget **widgets);
void widgetHider(GtkWidget **widgets);
void widgetCleanup(GtkWidget **to_hide, GtkWidget **to_show);
void displayColoredText(GtkLabel *label, char *message, char *color);
void changeSensivityWidgets(GtkWidget **widget, int sensitive);
void displaySolvingState(
	GtkWidget *pBar, double percent, GtkLabel *label, char *message);