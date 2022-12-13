#include "param.h"

static int nb_cells = 9;
static int uiMode = 0;

void setNbCells(int _nb_cells)
{
	nb_cells = _nb_cells;
}

int getNbCells()
{
	return nb_cells;
}

void setUIMode(int _uiMode)
{
	uiMode = _uiMode;
}

int getUIMode()
{
	return uiMode;
}
