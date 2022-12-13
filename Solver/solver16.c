#include "solver16.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// global variables for memorization
bool isOnRow16[16][16];
bool isOnCol16[16][16];
bool isOnBloc16[16][16];

int findBestCell16(int x[256], int y[256], int v[256], int nbCell, int *value) {
	// int nbval[256][256];
	int possibleVals[256][256][16];

	*value = -1;
	int bestCell = 0, mini = 100;
	for (int i = 0; i < nbCell; i++) {
		for (int k = 0; k < 16; k++) possibleVals[x[i]][y[i]][k] = 0;

		if (v[i] == -1) {
			int val = 0, cvalue = 0;
			for (int k = 0; k < 16; k++) {
				if (!isOnRow16[x[i]][k] && !isOnCol16[y[i]][k]
					&& !isOnBloc16[4 * (x[i] / 4) + (y[i] / 4)][k]) {
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
	for (int i = 0; i < 16; i++) {
		for (int k = 0; k < 16; k++) {
			int countValue = 0, rowValue = 0;
			for (int j = 0; j < 16; j++) {
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
	for (int i = 0; i < 16; i++) {
		for (int k = 0; k < 16; k++) {
			int countValue = 0, colValue = 0;
			for (int j = 0; j < 16; j++) {
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
	for (int i = 0; i < 16; i++) {
		int vx = 4 * (i / 4);
		int vy = (i % 4) * 4;

		for (int k = 0; k < 16; k++) {
			int countValue = 0, blocValue = 0;
			for (int j = 0; j < 16; j++) {
				if (possibleVals[vx + (j / 4)][vy + (j % 4)][k] == 1) {
					countValue++;
					blocValue = j;
					if (countValue > 1) break;
				}
			}
			if (countValue == 1) {
				for (int vc = 0; vc < nbCell; vc++) {
					if ((x[vc] == vx + (blocValue / 4))
						&& y[vc] == vy + (blocValue % 4)) {
						*value = k;
						return vc;
					}
				}
			}
		}
	}
	return bestCell;
}

int solver16(int **array) {
	// Initialization of the arrays
	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 16; j++)
			isOnRow16[i][j] = isOnCol16[i][j] = isOnBloc16[i][j] = false;

	// List cells to find
	int k;
	int nbCell = 0;
	int x[256], y[256], v[256], s[256], f[256];
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if ((k = array[i][j]) == 0) {
				x[nbCell] = i;
				y[nbCell] = j;
				v[nbCell] = -1;
				nbCell++;
			} else
				isOnRow16[i][k - 1] = isOnCol16[j][k - 1]
					= isOnBloc16[4 * (i / 4) + (j / 4)][k - 1] = true;
		}
	}

	// Iterative backtracking on the stack of values
	int indexCell = 0, value;
	s[0] = findBestCell16(x, y, v, nbCell, &value);
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
		if (currVal > 15) {
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
				isOnRow16[vx][currVal] = isOnCol16[vy][currVal]
					= isOnBloc16[4 * (vx / 4) + (vy / 4)][currVal] = false;
				if (f[s[indexCell] == -1]) bContinue = 0;
			}
		} else {
			if (!isOnRow16[vx][currVal] && !isOnCol16[vy][currVal]
				&& !isOnBloc16[4 * (vx / 4) + (vy / 4)][currVal]) {
				v[s[indexCell]] = currVal;
				isOnRow16[vx][currVal] = isOnCol16[vy][currVal]
					= isOnBloc16[4 * (vx / 4) + (vy / 4)][currVal] = true;
				indexCell++;
				s[indexCell] = findBestCell16(x, y, v, nbCell, &value);
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
