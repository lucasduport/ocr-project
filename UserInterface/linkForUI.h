#pragma once

#include "ui.h"

double *centerInput(const char *path);

gboolean getSolvedImage(gpointer data);

void fastSolving(GtkLabel *upload_warn, char *foldername);