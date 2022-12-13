#include "IOHelper.h"

Network *NetCreate(NNParam *param) {

	Network *net = (Network *)malloc(sizeof(Network));
	if (net == NULL) {
		printf("Error: Out of memory ...\n");
		exit(1);
	}
	Network_Init(net, param->nbLayer);

	Layer *lS = malloc(sizeof(Layer)), *lE = malloc(sizeof(Layer));
	Layer **hidden_L = malloc(sizeof(Layer*) * (param->nbLayer-2));
	for (ui i=0; i < param->nbLayer-2; i++) {
        Layer *l = malloc(sizeof(Layer));
        if (i==0) Layer_Init(lS, NULL, l, param->iSize, NULL, NULL, false, "none");
        hidden_L[i] = l;
	}

	Layer *l_prev = lS;
	for (ui i=0; i < param->nbLayer-2; i++) {
        Layer_Init(hidden_L[i], l_prev, i < param->nbLayer-3 ?
                   hidden_L[i+1] : lE, param->hNeurons[i], NULL, NULL,
                   false, param->act_funcs[i]);
        l_prev = hidden_L[i];
	}
	Layer_Init(lE, l_prev, NULL, param->oSize, NULL, NULL, false, param->endLayerAct);

	Network_AddLayer(net, lS);
	for (ui i=0; i < param->nbLayer-2; i++) Network_AddLayer(net, hidden_L[i]);
	Network_AddLayer(net, lE);
	Network_Wire(net);
    free(hidden_L);
	return net;
}

void LoadData(NNParam *param) {
	ui SamplesTrain = 0, SamplesValidate = 0;
	if (sscanf(param->trainingFile, "%*[^_]%*[_]%*[^_]%*[_]%u", &SamplesTrain)
		!= 1) {
		printf("Coudl not read amount of samples in "
			   "filename; Exiting...\n");
		exit(1);
	}
	if (sscanf(param->validationFile, "%*[^_]%*[_]%*[^_]%*[_]%u",
			   &SamplesValidate)
		!= 1) {
		printf("Coudl not read amount of samples in "
			   "filename; Exiting...\n");
		exit(1);
	}
	FILE *fptr1, *fptr2;

	if ((fptr1 = fopen(param->trainingFile, "rb")) == NULL) {
		fprintf(stderr, "Cannot open file '%s'\n", param->trainingFile);
		exit(1);
	}
	param->toLoopTrain = Min(SamplesTrain, param->toLoopTrain);
	param->inputTrain = malloc(sizeof(dl*) * param->toLoopTrain);
	param->outputTrain = malloc(sizeof(dl*) * param->toLoopTrain);
	dl *tempIn, *tempOut;
	ui temp;
	for (ui i = 0; i < param->toLoopTrain; i++) {
		tempIn = fvec_alloc(param->iSize, false);
		tempOut = fvec_alloc(param->oSize, true);
		fread(&temp, sizeof(ui), 1, fptr1);
		tempOut[temp] = 1.0L;
		fread(tempIn, sizeof(double), param->iSize, fptr1);
		param->inputTrain[i] = tempIn;
		param->outputTrain[i] = tempOut;
	}
	fclose(fptr1);
	if ((fptr2 = fopen(param->validationFile, "rb")) == NULL) {
		fprintf(stderr, "Cannot open file '%s'\n", param->validationFile);
		exit(1);
	}
	param->toLoopValidate = Min(SamplesValidate, param->toLoopValidate);
	param->inputTest = malloc(sizeof(dl*) * param->toLoopValidate);
	param->outputTest = malloc(sizeof(dl*) * param->toLoopValidate);
	for (ui i = 0; i < param->toLoopValidate; i++) {
		tempIn = fvec_alloc(param->iSize, false);
		tempOut = fvec_alloc(param->oSize, true);
		fread(&temp, sizeof(ui), 1, fptr2);
		tempOut[temp] = 1.0L;
		fread(tempIn, sizeof(double), param->iSize, fptr2);
		param->inputTest[i] = tempIn;
		param->outputTest[i] = tempOut;
	}
	fclose(fptr2);
}

float Validate(Network *net, const NNParam *P, float bperf) {
	ui score = 0, all = 0, pos = 0;
	for (ui i = 0; i < P->toLoopValidate; i++) {
		dl *out
			= Network_Validate(net, P->inputTest[i], P->iSize, P->oSize == 1);
		for (ui j = 0; j < P->oSize; j++) {
			// printf("\n%u : %.0LF\t%.0LF", j, out[j],
			// P->outputTest[i][j]);
			if (absl(out[j] - P->outputTest[i][j]) < LDBL_EPSILON) score++;
			if (P->outputTest[i][j] >= 1.0L && out[j] >= 1.0L) pos++;
			all++;
		}
		// printf("\n");
	}
	float AScore = 100.0 * score / (float)all;
	float VScore = 100.0 * pos / (float)P->toLoopValidate;
	bool single = P->oSize == 1;

	printf("%s%.2f%%\033[0m\n",
		   (single ? AScore : VScore) > bperf ? "\033[0;32m" : "\033[0;31m",
		   single ? AScore : Min(AScore, VScore));
	return single ? AScore : Min(AScore, VScore);
}

void ConfusionMatrix(Network *net, const NNParam *P) {
	if (P->oSize < 2) return;
	ui matrix[P->oSize][P->oSize];
	for (ui i = 0; i < P->oSize; i++) {
		for (ui j = 0; j < P->oSize; j++) matrix[i][j] = .0f;
	}

	ui bufferAct = 0, bufferExp = 0;
	bool Acted = false, Exped = false;
	for (ui i = 0; i < P->toLoopValidate; i++) {
		bufferAct = 0;
		bufferExp = 0;
		Acted = false;
		Exped = false;
		dl *out
			= Network_Validate(net, P->inputTest[i], P->iSize, P->oSize == 1);
		for (ui j = 0; j < P->oSize; j++) {
			if (P->outputTest[i][j] >= 1.0L) Exped = true;
			if (!Exped) bufferExp++;
			if (out[j] >= 1.0L) Acted = true;
			if (!Acted) bufferAct++;
		}
		matrix[bufferAct][bufferExp]++;
	}

	float bf = 0;
	printf("\n\t\033[0;31m");
	for (ui i = 0; i < P->oSize; i++) printf("%5d ", i);
	printf("\033[0m\n");
	for (ui i = 0; i < P->oSize; i++) {
		printf("\033[0;31m%d\t\033[0m", i);
		bf = 0;
		for (ui j = 0; j < P->oSize; j++) {
                if(i==j) printf("\033[0;34m");
                printf("%5d \033[0m", matrix[j][i]);
                bf += matrix[j][i];
        }
        float v = matrix[i][i]/bf;
		printf("\t%s%.2f%s\n", v < 0.6f ? "\033[0;31m" : v < 0.9 ?
                "\033[0;33m" : "\033[0;32m", v, "\033[0m");
	}
	printf("\n");
}

void OverfitLoad(NNParam *param) {
	ui SamplesTrain = 0;
	if (sscanf(param->trainingFile, "%*[^_]%*[_]%*[^_]%*[_]%u", &SamplesTrain)
		!= 1) {
		printf("Coudl not read amount of samples in "
			   "filename; Exiting...\n");
		exit(1);
	}

	FILE *fptr1;
	if ((fptr1 = fopen(param->trainingFile, "rb")) == NULL) {
		fprintf(stderr, "Cannot open file '%s'\n", param->trainingFile);
		exit(1);
	}
	param->toLoopTrain = Min(SamplesTrain, param->toLoopTrain);
	param->toLoopValidate = param->toLoopTrain;
	param->inputTrain = malloc(sizeof(float *) * param->toLoopTrain);
	param->outputTrain = malloc(sizeof(float *) * param->toLoopTrain);
	dl *tempIn, *tempOut;
	dl *temp;
	for (ui i = 0; i < param->toLoopTrain; i++) {
		tempIn = fvec_alloc(param->iSize, false);
		tempOut = fvec_alloc(param->oSize, true);
		temp = fvec_alloc(1, false);
		fread(temp, sizeof(dl), 1, fptr1);
		tempOut[(ui)temp[0]] = 1.0L;
		free(temp);
		fread(tempIn, sizeof(dl), param->iSize, fptr1);
		param->inputTrain[i] = tempIn;
		param->outputTrain[i] = tempOut;
	}
	fclose(fptr1);

	param->inputTest = param->inputTrain;
	param->outputTest = param->outputTrain;
}

void PerfSearch(NNParam *origin, Network *netOrg, int attempt) {
	float bperf = (float)origin->toExceed, curr_perf = .0f;
	ui eSize = floor(log10(origin->epoch == 0 ? 1 : origin->epoch)) + 1;
	printf("\nBeginning Neural Network training with "
		   "following parameters :\n");
	NNParam_Display(origin);
	bool maxed = false;
	while (attempt > 0 && !maxed) {
		srand((ui)time(NULL));
		Network *net = netOrg == NULL ? NetCreate(origin) : Network_DeepCopy(netOrg);
		//Network_Display(net, false);
		if (origin->track) fclose(fopen(origin->StatsFile, "w"));
		Optimizer_Init(net, origin->optimizer);
		for (ui e = 0; e < origin->epoch && !maxed;) {
			printf("[ Epoch %*u/%u ] Accuracy : ", eSize, e, origin->epoch);
			curr_perf = Validate(net, origin, bperf);
			if (curr_perf > bperf) {
				bperf = curr_perf;
				ui ns = strlen(origin->NNName)+7;
				char *s = malloc(sizeof(char) * (ns));
				snprintf(s, ns, "%s_%.2f", origin->NNName, bperf);
				Network_Save(net, s);
				free(s);
			}
			if (curr_perf >= 100.0f) maxed = 1;
			else {
				Network_Train(net, origin);
				ui ne
					= Min((int)origin->epochInterval, (int)(origin->epoch - e));
				origin->epochInterval = ne;
				e += ne;
			}
		}

		printf("[ Epoch %u/%u ] Accuracy : ", origin->epoch, origin->epoch);
		curr_perf = Validate(net, origin, bperf);
		if (curr_perf >= 100.0f) maxed = 1;
		if (curr_perf > bperf) {
			bperf = curr_perf;
			ui ns = strlen(origin->NNName)+7;
			char *s = malloc(sizeof(char) * (ns));
			snprintf(s, ns, "%s_%.2f", origin->NNName, bperf);
			Network_Save(net, s);
			free(s);
		}
		ConfusionMatrix(net, origin);
		Optimizer_Dispose(net, origin->optimizer, !attempt);
		Network_Purge(net);
		net = NULL;
		attempt--;
	}
}

void NNParam_Display(NNParam *param) {
	printf("\n\tLearning Rate : %-10g", param->l_rate);
	printf("\tNetwork Name : %s\n", param->NNName);
	printf("\tTraining samples : %-10u", param->toLoopTrain);
	printf("\tEpoch : %-10u\n", param->epoch);
	printf("\tValidation samples : %-10u", param->toLoopValidate);
	printf("\tEpoch interval : %-10u\n", param->epochInterval);
	printf("\tCost Function : %-12s", param->cost_func);
	printf("\tL1 Regularization : %-10g\n", param->l1Norm);
	printf("\tLast Activation : %-12s", param->endLayerAct);
	printf("\tL2 Regularization : %-10g\n", param->l2Norm);
	printf("\tTracking : %-20s", param->track ? "true" : "false");
	printf("\tOptimizer : %-12s\n\n", param->optimizer != NULL ? "Adam" : "None");
}

void Purge_NNParam(NNParam *param) {
	if (param->cost_func) free(param->cost_func);
	if (param->endLayerAct) free(param->endLayerAct);
	if (param->NNName != NULL) free(param->NNName);
	if (param->StatsFile != NULL) free(param->StatsFile);
	free(param->hNeurons);
	for (ui i = 0; i < param->nbLayer-2; i++) free(param->act_funcs[i]);
	free(param->act_funcs);
	bool freeOnce = param->toLoopTrain == param->toLoopValidate;
	for (ui i = 0; i < param->toLoopTrain; i++) {
		free(param->inputTrain[i]);
		free(param->outputTrain[i]);
	}
	free(param->inputTrain);
	free(param->outputTrain);
	if (freeOnce) return;
	for (ui i = 0; i < param->toLoopValidate; i++) {
		free(param->inputTest[i]);
		free(param->outputTest[i]);
	}
	free(param->inputTest);
	free(param->outputTest);
	free(param);
}
