#include "Cost.h"

dl CrossEntropy(dl *predicted, dl *expected, cui Size) {
	dl cost = .0L;
	for (ui i = 0; i < Size; i++) {
		cost += expected[i] * logl(predicted[i] + EPS)
				+ (1 - expected[i]) * logl(1 - predicted[i] + EPS);
	}
	if (isnan(cost)) {
		printf("\nnan in crossentropy\n");
		exit(4);
	}
	return -cost / (dl)Size;
}

dl RMSE(dl *predicted, dl *expected, cui Size) {
	dl res = MSE(predicted, expected, Size);
	return sqrtl(res + LDBL_EPSILON);
}

dl MAE(dl *predicted, dl *expected, cui Size) {
	dl cost = .0L;
	for (ui i = 0; i < Size; i++) cost += absl(predicted[i] - expected[i]);
	return (1.0L / (dl)Size) * cost;
}

dl MSE(dl *predicted, dl *expected, cui Size) {
	dl cost = .0L;
	for (ui i = 0; i < Size; i++) cost += powl((predicted[i] - expected[i]), 2);
	return (1.0L / (dl)Size) * cost;
}

dl CrossEntropy_(cdl predicted, cdl expected) {
	return (predicted - expected) / (1 - predicted + EPS) * predicted;
}

dl RMSE_(cdl predicted, cdl expected) {
	return (predicted - expected)
		   / sqrtl(powl(predicted - expected, 2) + LDBL_EPSILON);
}

dl MAE_(cdl predicted, cdl expected) {
	return (predicted - expected) / (absl(predicted - expected) + LDBL_EPSILON);
}

dl MSE_(cdl predicted, cdl expected) { return 2 * (predicted - expected); }

dl none_(dl *arr, cui Size, cui ieme) {
	UNUSED(arr);
	UNUSED(Size);
	UNUSED(ieme);
	return 0;
}

dl sigmoid_(dl *arr, cui Size, cui ieme) {
	if (ieme >= Size) {
		printf("Warning: sigmoid derivative index out of "
			   "bound.\n");
		return 0;
	}
	cdl sigm = 1 / (1 + expl(-arr[ieme]));
	return sigm * (1 - sigm);
}

dl softmax_(dl *arr, cui Size, cui ieme) {
	if (ieme >= Size) {
		printf("Warning: softmax derivative index out of "
			   "bound.\n");
		return 0;
	}
	dl max = arr[ieme];
	for (ui i = 0; i < Size; i++)
		if (arr[i] > max) max = arr[i];
	for (ui i = 0; i < Size; i++) arr[i] -= max;

	dl sT = 0, tgt = expl(arr[ieme]);
	for (ui i = 0; i < Size; i++) { sT += expl(arr[i]); }
	return tgt * (sT - tgt) / (powl(sT, 2) + EPS);
}

dl argmax_(dl *arr, cui Size, cui ieme) {
	UNUSED(arr);
	if (ieme >= Size) {
		printf("Warning: argmax derivative index out of "
			   "bound.\n");
		return 0;
	}
	printf("WARNING: argmax derivative not implemented");
	return 0;
}

dl step_(dl *arr, cui Size, cui ieme) {
	if (ieme >= Size) {
		printf("Warning: step derivative index out of "
			   "bound.\n");
		return 0;
	}
	int seed = (int)time(NULL);
	return (absl(arr[ieme] - .0L) < LDBL_EPSILON) ? (dl)(r8_normal_01(&seed))
												  : .5L;
}

dl relu_(dl *arr, cui Size, cui ieme) {
	if (ieme >= Size) {
		printf("Warning: relu derivative index out of "
			   "bound.\n");
		return 0;
	}
	return (arr[ieme] < 0) ? 0 : 1;
}

dl selu_(dl *arr, cui Size, cui ieme) {
	if (ieme >= Size) {
		printf("Warning: selu derivative index out of "
			   "bound.\n");
		return 0;
	}
	cdl alpha = 1.6732632423543772848170429916717;
	cdl lambda = 1.0507009873554804934193349852946;
	return lambda * (arr[ieme] > 0 ? 1 : alpha * expl(arr[ieme]));
}

dl leakyrelu_(dl *arr, cui Size, cui ieme) {
	if (ieme >= Size) {
		printf("Warning: leakyrelu derivative index out of "
			   "bound.\n");
		return 0;
	}
	return (arr[ieme] <= 0) ? .01L : 1.0L;
}

dl (*get_deriv(const char *name))(dl *arr, cui Size, cui ieme) {
	for (ui i = 0; i < (sizeof(deriv_map) / sizeof(deriv_map[0])); i++)
		if (!strcmp(deriv_map[i].name, name)) return deriv_map[i].func;
	return NULL;
}

dl (*get_cost(const char *name))(dl *predicted, dl *expected, cui Size) {
	for (ui i = 0; i < (sizeof(cost_map) / sizeof(cost_map[0])); i++)
		if (!strcmp(cost_map[i].name, name)) return cost_map[i].func;
	return NULL;
}

dl (*get_cost_deriv(const char *name))(dl predicted, dl expected) {
	for (ui i = 0; i < (sizeof(cost_deriv_map) / sizeof(cost_deriv_map[0]));
		 i++)
		if (!strcmp(cost_deriv_map[i].name, name))
			return cost_deriv_map[i].func;
	return NULL;
}
