#include "openFile.h"
#include "solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fileProcessing(const char *filename) {
	int **array = malloc(9 * sizeof(int *));
	for (int i = 0; i < 9; i++) array[i] = (int *)malloc(9 * sizeof(int));
	if (!array) fprintf(stderr, "malloc array failed");
	FILE *file = fopen(filename, "r");
	if (!file) fprintf(stderr, "malloc file failed");
	size_t count;
	char buffer = 'a'; // modified
	int value;
	size_t i = 0;
	size_t j = 0;

	do {
		count = fread(&buffer, 1, 1, file);
		if (buffer == '.') value = 0;
		else if (buffer >= '1' && buffer <= '9') value = atoi(&buffer);
		else {
			if (buffer == '\n' && j) {
				i++;
				j = 0;
			}
			continue;
		}
		array[i][j] = value;
		j++;
	} while (count != 0);
	fclose(file);

	solver(array);
	if (count > 10000) return;
	/*for (int i=0; i<9; i++)
	{
		for (int j=0; j<9; j++)
		{
			printf("%d ",array[i][j]);
			if ((j+1)%3==0) printf(" ");
		}
		printf("\n");
		if ((i+1)%3==0) printf("\n");
	}*/

	strcat((char *)filename, ".result");
	FILE *finalFile = NULL;
	finalFile = fopen(filename, "w");
	if (!finalFile) fprintf(stderr, "malloc final file failed");
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (j == 3 || j == 6) fputc(' ', finalFile);
			fputc(array[i][j] + '0', finalFile);
		}
		if (i == 2 || i == 5) fputc('\n', finalFile);
		fputc('\n', finalFile);
	}
	fclose(finalFile);
	for (int i = 0; i < 9; i++) free(array[i]);
}
