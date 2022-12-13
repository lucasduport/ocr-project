#include "Rand.h"

dl r8_uniform_01(int *seed) {
	const int i4_huge = 2147483647;
	int k;
	k = *seed / 127773;
	*seed = 16807 * (*seed - k * 127773) - k * 2836;
	if (*seed < 0) *seed += i4_huge;
	return (dl)(*seed) * 4.656612875E-10;
}

dl r8_normal_01(int *seed) {
	dl r1, r2;
	const dl r8_pi = 3.141592653589793;
	r1 = r8_uniform_01(seed);
	r2 = r8_uniform_01(seed);
	return sqrt(-2.0 * log(r1)) * cos(2.0 * r8_pi * r2);
}

dl *r8vec_normal_ab_new(ui n, dl b, dl c, int *seed) {
	ui i, m, x_hi, x_lo;
	dl *r, *x;
	const dl r8_pi = 3.141592653589793;

	x = (dl *)malloc(n * sizeof(dl));
	x_lo = 1;
	x_hi = n;

	if (x_hi - x_lo + 1 == 1) {
		r = r8vec_uniform_01_new(2, seed);
		x[x_hi - 1] = sqrt(-2.0 * log(r[0])) * cos(2.0 * r8_pi * r[1]);
		free(r);
	} else if ((x_hi - x_lo + 1) % 2 == 0) {
		// Even Number needed
		m = (x_hi - x_lo + 1) / 2;
		r = r8vec_uniform_01_new(2 * m, seed);
		for (i = 0; i <= 2 * m - 2; i += 2) {
			x[x_lo + i] = sqrt(-2.0 * log(r[i])) * sin(2.0 * r8_pi * r[i + 1]);
			x[x_lo + i - 1]
				= sqrt(-2.0 * log(r[i])) * cos(2.0 * r8_pi * r[i + 1]);
		}
		free(r);
	} else {
		// Odd Number needed
		x_hi = x_hi - 1;
		m = (x_hi - x_lo + 1) / 2 + 1;
		r = r8vec_uniform_01_new(2 * m, seed);
		for (i = 0; i <= 2 * m - 4; i += 2) {
			x[x_lo + i - 1]
				= sqrt(-2.0 * log(r[i])) * cos(2.0 * r8_pi * r[i + 1]);
			x[x_lo + i] = sqrt(-2.0 * log(r[i])) * sin(2.0 * r8_pi * r[i + 1]);
		}
		i = 2 * m - 2;
		x[x_lo + i - 1] = sqrt(-2.0 * log(r[i])) * cos(2.0 * r8_pi * r[i + 1]);
		free(r);
	}

	for (i = 0; i < n; i++) x[i] = b + c * x[i];
	return x;
}

dl *r8vec_uniform_01_new(ui n, int *seed) {
	ui i, k;
	const int i4_huge = 2147483647;
	dl *r;
	r = (dl *)malloc(n * sizeof(dl));
	for (i = 0; i < n; i++) {
		k = *seed / 127773;
		*seed = 16807 * (*seed - k * 127773) - k * 2836;
		if (*seed < 0) *seed = *seed + i4_huge;
		r[i] = (dl)(*seed) * 4.656612875E-10;
	}
	return r;
}
