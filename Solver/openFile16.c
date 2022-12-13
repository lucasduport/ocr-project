#include "openFile16.h"
#include "solver16.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fileProcessing16(const char *filename) {

	int **array = malloc(16 * sizeof(int *));
	for (int i = 0; i < 16; i++) array[i] = (int *)malloc(16 * sizeof(int));
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
		if (buffer == '.') value = -1; // modified
		else if (buffer >= '0' && buffer <= '9') value = atoi(&buffer); // + 1;
		else if (buffer == 'A') value = 10;
		else if (buffer == 'B') value = 11;
		else if (buffer == 'C') value = 12;
		else if (buffer == 'D') value = 13;
		else if (buffer == 'E') value = 14;
		else if (buffer == 'F') value = 15;
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
	/*for (int i=0; i<16; i++)
	{
		for (int j=0; j<16; j++)
		{
			printf("%d ",array[i][j]);
			if ((j+1)%4==0) printf(" ");
		}
		printf("\n");
		if ((i+1)%4==0) printf("\n");
	}*/
	solver16(array);
	if (count > 10000) return;

	/*for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			array[i][j]--;
		}
	}*/

	/*for (int i=0; i<16; i++)
	{
		for (int j=0; j<16; j++)
		{
			printf("%d ",array[i][j]);
			if ((j+1)%4==0) printf(" ");
		}
		printf("\n");
		if ((i+1)%4==0) printf("\n");
	}*/

	strcat((char *)filename, ".result");
	FILE *finalFile = NULL;
	finalFile = fopen(filename, "w");
	if (!finalFile) fprintf(stderr, "malloc final file failed");
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (j == 4 || j == 8 || j == 12) fputc(' ', finalFile);
			if (0 <= array[i][j] && array[i][j] <= 9)
				fputc(array[i][j] + '0', finalFile);
			if (array[i][j] == 10) fputc('A', finalFile);
			if (array[i][j] == 11) fputc('B', finalFile);
			if (array[i][j] == 12) fputc('C', finalFile);
			if (array[i][j] == 13) fputc('D', finalFile);
			if (array[i][j] == 14) fputc('E', finalFile);
			if (array[i][j] == 15) fputc('F', finalFile);
		}
		if (i == 3 || i == 7 || i == 11) fputc('\n', finalFile);
		fputc('\n', finalFile);
	}
	fclose(finalFile);
	for (int i = 0; i < 16; i++) free(array[i]);
}