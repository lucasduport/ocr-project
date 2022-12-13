#include "solver.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// global variables for memorization
bool isOnRow[9][9];
bool isOnCol[9][9];
bool isOnBloc[9][9];

int findBestCell(int x[81], int y[81], int v[81], int nbCell, int *value) {
	// int nbval[81][81];
	int possibleVals[81][81][9];

	*value = -1;
	int bestCell = 0, mini = 100;
	for (int i = 0; i < nbCell; i++) {
		for (int k = 0; k < 9; k++) possibleVals[x[i]][y[i]][k] = 0;

		if (v[i] == -1) {
			int val = 0, cvalue = 0;
			for (int k = 0; k < 9; k++) {
				if (!isOnRow[x[i]][k] && !isOnCol[y[i]][k]
					&& !isOnBloc[3 * (x[i] / 3) + (y[i] / 3)][k]) {
					cvalue = k;
					val++;
					possibleVals[x[i]][y[i]][k] = 1;
				}
			}
			if (val < mini) {
				bestCell = i;
				mini = val;
				if (val == 1) {
					*value = cvalue;
					return bestCell;
				}
			}
		}
	}

	// Row
	for (int i = 0; i < 9; i++) {
		for (int k = 0; k < 9; k++) {
			int countValue = 0, rowValue = 0;
			for (int j = 0; j < 9; j++) {
				if (possibleVals[i][j][k] == 1) {
					countValue++;
					rowValue = j;
					if (countValue > 1) break;
				}
			}
			if (countValue == 1) {
				for (int vc = 0; vc < nbCell; vc++) {
					if ((x[vc] == i) && y[vc] == rowValue) {
						*value = k;
						return vc;
					}
				}
			}
		}
	}
	// Column
	for (int i = 0; i < 9; i++) {
		for (int k = 0; k < 9; k++) {
			int countValue = 0, colValue = 0;
			for (int j = 0; j < 9; j++) {
				if (possibleVals[j][i][k] == 1) {
					countValue++;
					colValue = j;
					if (countValue > 1) break;
				}
			}
			if (countValue == 1) {
				for (int vc = 0; vc < nbCell; vc++) {
					if ((x[vc] == colValue) && y[vc] == i) {
						*value = k;
						return vc;
					}
				}
			}
		}
	}

	// Bloc
	for (int i = 0; i < 9; i++) {
		int vx = 3 * (i / 3);
		int vy = (i % 3) * 3;

		for (int k = 0; k < 9; k++) {
			int countValue = 0, blocValue = 0;
			for (int j = 0; j < 9; j++) {
				if (possibleVals[vx + (j / 3)][vy + (j % 3)][k] == 1) {
					countValue++;
					blocValue = j;
					if (countValue > 1) break;
				}
			}
			if (countValue == 1) {
				for (int vc = 0; vc < nbCell; vc++) {
					if ((x[vc] == vx + (blocValue / 3))
						&& y[vc] == vy + (blocValue % 3)) {
						*value = k;
						return vc;
					}
				}
			}
		}
	}
	return bestCell;
}

int solver(int **array) {
	// Initialization of the arrays
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			isOnRow[i][j] = isOnCol[i][j] = isOnBloc[i][j] = false;

	// List cells to find
	int k;
	int nbCell = 0;
	int x[81], y[81], v[81], s[81], f[81];
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if ((k = array[i][j]) == 0) {
				x[nbCell] = i;
				y[nbCell] = j;
				v[nbCell] = -1;
				nbCell++;
			} else
				isOnRow[i][k - 1] = isOnCol[j][k - 1]
					= isOnBloc[3 * (i / 3) + (j / 3)][k - 1] = true;
		}
	}

	// Iterative backtracking on the stack of values
	int indexCell = 0, value;
	s[0] = findBestCell(x, y, v, nbCell, &value);
	int currVal;
	currVal = f[s[0]] = -1;
	if (value != -1) {
		currVal = value - 1;
		f[s[0]] = value;
	}

	int countMove = 0;

	while (indexCell < nbCell) {
		countMove++;
		if (countMove > 10000) {
			countMove = 0;
			return countMove;
		}

		int vx = x[s[indexCell]];
		int vy = y[s[indexCell]];

		currVal++;
		if (currVal > 8) {
			if (indexCell == 0) {
				for (int i = 0; i < nbCell; i++) array[x[i]][y[i]] = v[i] + 1;
				return 0;
			}
			int bContinue = 1;
			while (bContinue) {
				v[s[indexCell]] = -1;
				indexCell--;
				// Not sure about this
				if (indexCell < 0) {
					for (int i = 0; i < nbCell; i++)
						array[x[i]][y[i]] = v[i] + 1;
					printf("Not solvable\n");
					return 0;
				}
				// \Not sure about this
				currVal = v[s[indexCell]];
				vx = x[s[indexCell]];
				vy = y[s[indexCell]];
				isOnRow[vx][currVal] = isOnCol[vy][currVal]
					= isOnBloc[3 * (vx / 3) + (vy / 3)][currVal] = false;
				if (f[s[indexCell] == -1]) bContinue = 0;
			}
		} else {
			if (!isOnRow[vx][currVal] && !isOnCol[vy][currVal]
				&& !isOnBloc[3 * (vx / 3) + (vy / 3)][currVal]) {
				v[s[indexCell]] = currVal;
				isOnRow[vx][currVal] = isOnCol[vy][currVal]
					= isOnBloc[3 * (vx / 3) + (vy / 3)][currVal] = true;
				indexCell++;
				s[indexCell] = findBestCell(x, y, v, nbCell, &value);
				f[s[indexCell]] = currVal = -1;
				if (value != -1) {
					currVal = value - 1;
					f[s[indexCell]] = value;
				}
			}
		}
	}

	for (int i = 0; i < nbCell; i++) array[x[i]][y[i]] = v[i] + 1;
	return countMove;
}
