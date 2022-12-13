
#include "Tools.h"

void matr_display(dl *arr, cui s, cui cols) {
	printf("\n\t[");
	for (ui i = 0; i < s; i++) {
		if (i > 0 && i % cols == 0) printf("\n\t ");
		printf("%f", (double)arr[i]);
		if (i < s - 1) printf(", ");
	}
	printf("]\n");
}

dl *fvec_alloc(cui n, bool zInit) {
	dl *tmp = (dl *)(zInit) ? calloc(n, sizeof(dl)) : malloc(sizeof(dl) * n);
	if (tmp == NULL) {
		printf("Error: Out of memory ...\n");
		exit(1);
	}
	return tmp;
}

char *cvec_alloc(cui n) {
	char *tmp = (char *)malloc(sizeof(char) * n);
	if (tmp == NULL) {
		printf("Error: Out of memory ...\n");
		exit(1);
	}
	return tmp;
}

dl *fvec_rInit(cui n, cui conns, bool he_init) {
	int seed = (int)time(NULL);
	dl *tmp = r8vec_normal_ab_new(n, 0.0, he_init ? sqrtl(2.0L / conns) : 1.0,
								  &seed);
	return tmp;
}

void arr_shuffle(dl *arr[], dl *paired_arr[], cui Size) {
	int n;
	for (dl **a = arr, **pa = paired_arr; a < arr + Size; a++, pa++) {
		n = rand() % Size;
		dl *temp = *a, *temp_paired = *pa;
		*a = arr[n];
		arr[n] = temp;
		*pa = paired_arr[n];
		paired_arr[n] = temp_paired;
	}
}
