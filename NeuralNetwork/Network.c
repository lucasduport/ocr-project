#include "Network.h"

void Network_Init(Network *net, cui nbLayers) {
	net->layers = lvec_alloc(nbLayers);
	net->nbLayers = nbLayers;
	net->currentLayer = 0;
}

void Network_AddLayer(Network *net, Layer *layer) {
	if (net->currentLayer >= net->nbLayers) {
		printf("Attempting Layer overflow; Starting "
			   "network purge...\n");
		Network_Purge(net);
		return;
	}
	net->layers[net->currentLayer] = *layer;
	if (!layer->loaded) free(layer);
	net->currentLayer++;
}

void Network_Wire(Network *net) {
	for (ui i = 1; i < net->nbLayers; i++) {
		net->layers[i - 1].nLayer = &net->layers[i];
		net->layers[i].pLayer = &net->layers[i - 1];
		if (i + 1 < net->nbLayers) net->layers[i].nLayer = &net->layers[i + 1];
	}
}

void Network_Load(Network *net, char path[]) {
	if (sscanf(path, "%*[^_]%*[_]%u", &net->nbLayers) != 1) {
		printf("Coudl not read amount of layer in "
			   "filename; Exiting...\n");
		exit(1);
	}
	Network_Init(net, net->nbLayers);
	FILE *fptr;
	if ((fptr = fopen(path, "rb")) == NULL) {
		fprintf(stderr, "Cannot open file '%s'\n", path);
		exit(1);
	}
	Layer layer, *lSave = NULL;
	LayerSave saved;
	for (ui i = 0; i < net->nbLayers; i++) {
		fread(&saved, sizeof(LayerSave), 1, fptr);
		if (saved.conns > 0) {
			char *name = cvec_alloc(saved.fSize + 1);
			fread(name, sizeof(char), saved.fSize, fptr);
			name[saved.fSize] = '\0';
			dl *bias = fvec_alloc(saved.Neurons, false);
			fread(bias, sizeof(dl), saved.Neurons, fptr);
			dl *weights = fvec_alloc(saved.conns, false);
			fread(weights, sizeof(dl), saved.conns, fptr);
			Layer_Init(&layer, lSave, NULL, saved.Neurons, weights, bias, true,
					   name);
		} else {
			char *act_name = (char *)malloc(sizeof(char) * 5);
			strcpy(act_name, "none");
			Layer_Init(&layer, NULL, NULL, saved.Neurons, NULL, NULL, true,
					   act_name);
		}
		Network_AddLayer(net, &layer);
		lSave = &layer;
	}
	fclose(fptr);
	if (net->currentLayer != net->nbLayers) {
		printf("Corrupted amount of layer [%u/%u]; "
			   "Starting purge...",
			   net->currentLayer + 1, net->nbLayers);
		Network_Purge(net);
		exit(1);
	}
	Network_Wire(net);
}

void Network_Save(Network *net, char name[]) {
	FILE *fptr;
	char filename[64], NNName[16];

	if (name == NULL) {
		printf("\nNeural Network Name : ");
		scanf("%s", NNName);
	}
	snprintf(filename, 64, "TrainedNetwork/NeuralNetData_%ulayers_%s.dnn",
			 net->currentLayer, name == NULL ? NNName : name);
	if ((fptr = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "Cannot open file '%s'\n", filename);
		exit(1);
	}

	for (Layer *l = net->layers; l < net->layers + net->nbLayers; l++) {
		ui len = (ui)strlen((*l).act_name);
		LayerSave saved = {(*l).Neurons, (*l).conns, len};
		fwrite(&saved, sizeof(saved), 1, fptr);
		if ((*l).pLayer != NULL) {
			fwrite((*l).act_name, sizeof(char), len, fptr);
			fwrite(&(*l).bias[0], sizeof(dl), (*l).Neurons, fptr);
			fwrite(&(*l).weights[0], sizeof(dl), (*l).conns, fptr);
		}
	}
	fclose(fptr);
}

void Network_Purge(Network *net) {
	for (Layer *l = net->layers; l < net->layers + net->nbLayers; l++)
		Layer_Dispose(l);
	free(net->layers);
	free(net);
}

Network *Network_DeepCopy(Network *net) {
    Network *copy = malloc(sizeof(Network));
    copy->nbLayers = net->nbLayers;
    copy->currentLayer = net->currentLayer;
    copy->layers = malloc(sizeof(Layer) * net->nbLayers);
    for (ui i=0; i<net->nbLayers; i++)
        copy->layers[i] = *Layer_DeepCopy(&net->layers[i]);
    Network_Wire(copy);
    return copy;
}

void Network_Display(Network *net, bool display_matr) {
	for (ui i = 0; i < net->nbLayers; i++) {
		Layer_Display(&net->layers[i], i, display_matr);
	}
}

Layer *lvec_alloc(cui n) {
	Layer *tmp = (Layer *)malloc(sizeof(Layer) * n);
	if (tmp == NULL) {
		printf("Error: Out of memory ...\n");
		exit(1);
	}
	return tmp;
}

float *Network_Predict(Network *net, dl *input, cui Size) {
	Network_Forward(net, input, Size);
	Layer *l = &net->layers[net->nbLayers - 1];
	float *rtn = malloc(sizeof(float)*l->Neurons);
	for(ui i=0; i<l->Neurons; i++) rtn[i] = (float)l->output[i];
	return rtn;
}

dl *Network_Validate(Network *net, dl *input, cui Size, bool os1) {
	Network_Forward(net, input, Size);
	(os1 ? step : argmax)(net->layers[net->nbLayers - 1].output,
						  net->layers[net->nbLayers - 1].output,
						  net->layers[net->nbLayers - 1].Neurons);
	return net->layers[net->nbLayers - 1].output;
}

void Network_Train(Network *net, NNParam *params) {
	if (net->nbLayers < 2) {
		printf("Attempting train on incomplete network; "
			   "Starting purge...\n");
		Network_Purge(net);
		exit(1);
	}
	if (params->iSize != net->layers[0].Neurons
		|| params->oSize != net->layers[net->nbLayers - 1].Neurons) {
		printf("Differing I/0 sizes; Starting purge...\n");
		Network_Purge(net);
		exit(1);
	}

	FILE *f = NULL;
	int c = 0;
	if (params->track) {
		c = 1;
		f = fopen(params->StatsFile, "a");
		if (f == NULL) params->track = false;
	}

	for (ui e = 0; e < params->epochInterval; e++) {
		arr_shuffle(params->inputTrain, params->outputTrain,
					params->toLoopTrain);
		for (ui s = 0; s < params->toLoopTrain; s++) {
			Network_Forward(net, params->inputTrain[s], params->iSize);
			dl error = Network_BackProp(net, params, s);
			if (params->track) {
				fprintf(f, "%f\n", error);
				c++;
			}
		}
	}

	if (params->track) fclose(f);
}

void Network_Forward(Network *net, dl *input, cui iSize) {
	if (iSize != net->layers[0].Neurons) {
		printf("Error: Input data size has different size "
			   "than neurons");
		exit(2);
	}
	net->layers[0].output = input;
	for (ui i = 1; i < net->currentLayer; i++) Layer_Activate(&net->layers[i]);
}

dl Network_BackProp(Network *net, NNParam *params, cui nth) {

    // Shortcut to last Layer
	Layer *L = &net->layers[net->nbLayers - 1];
    // Get pointers to cost function & last layer activation function
	dl (*cost_deriv)(dl, dl) = get_cost_deriv(params->cost_func);
	dl (*deriv)(dl *, cui, cui) = get_deriv(L->act_name);
    // Shortcut to witness vector
	dl *expected = params->outputTrain[nth];

    // Shortcut to Adam Optimizer
	Optimizer *optz = params->optimizer;
    // Init boolean for L1 & L2 Régularization
	bool dn1 = params->l1Norm == 0.0L, dn2 = params->l2Norm == 0.0L;
    // Init buffer for L1 & L2 cost
	dl l1 = .0L, l2 = .0L;

    // Init Adam Hyper-parameters
	dl b1t = 0, b2t = 0, *mwt = NULL, *vwt = NULL, *mbt = NULL, *vbt = NULL;
	dl **Gmwt = NULL, **Gvwt = NULL, **Gmbt = NULL, **Gvbt = NULL;
	if (optz != NULL) {
		b1t = powl(0.9L, optz->iter);
		b2t = powl(0.999L, optz->iter);
		mwt = optz->Mwt[net->nbLayers - 2];
		vwt = optz->Vwt[net->nbLayers - 2];
		mbt = optz->Mbt[net->nbLayers - 2];
		vbt = optz->Vbt[net->nbLayers - 2];
	}

    // Compute Error for this iteration
	dl error = get_cost(params->cost_func)(L->output, expected, params->oSize);

	dl CostOut[L->Neurons];
	dl OutIn[L->Neurons];
	ui i = 0;
	for (dl *cO = CostOut, *oI = OutIn, *o = L->output, *e = expected;
		 cO < CostOut + L->Neurons; cO++, oI++, o++, e++, i++) {
		*cO = cost_deriv(*o, *e);
		*oI = deriv(L->input, L->Neurons, i);
	}

	dl *Legacy = fvec_alloc(L->pLayer->Neurons, true);
	bool bias_done = false;
	for (dl *pO = L->pLayer->output, *leg = Legacy, *w = L->weights;
		 pO < L->pLayer->output + L->pLayer->Neurons; pO++, leg++) {
		for (dl *cO = CostOut, *oI = OutIn, *b = L->bias;
			 cO < CostOut + L->Neurons; cO++, oI++, w++, b++) {
			dl ml = (*cO) * (*oI), pw = (*w);
			*leg += ml * (*w);
			if (!dn1) l1 += absl(*w);
			if (!dn2) l2 += (*w) * (*w);
			if (optz != NULL) {
				dl gd = ml * (*pO);
				*mwt = 0.9L * (*mwt) + (1 - 0.9) * gd;
				*vwt = 0.999L * (*vwt) + (1 - 0.999L) * gd * gd;
				dl mwc = (*mwt) / (1 - b1t);
				dl vwc = (*vwt) / (1 - b2t);
				*w -= params->l_rate * mwc / (sqrtl(vwc) + OPT_EPS);
				mwt++;
				vwt++;
			} else *w -= params->l_rate * ml * (*pO);
			*w += params->l_rate * (pw >= .0L ? 1.0L : -1.0L) * params->l1Norm
				  + params->l_rate * 2 * params->l2Norm * pw;
			if (!bias_done) {
				if (optz != NULL) {
					*mbt = 0.9 * (*mbt) + (1 - 0.9L) * ml;
					*vbt = 0.999L * (*vbt) + (1 - 0.999L) * ml * ml;
					dl mbc = (*mbt) / (1 - b1t);
					dl vbc = (*vbt) / (1 - b2t);
					*b -= params->l_rate * mbc / (sqrtl(vbc) + OPT_EPS);
					mbt++;
					vbt++;
				} else *b -= params->l_rate * ml;
			}
		}
		bias_done = true;
	}

	if (optz != NULL) {
		Gmwt = optz->Mwt + net->nbLayers - 3;
		Gvwt = optz->Vwt + net->nbLayers - 3;
		Gmbt = optz->Mbt + net->nbLayers - 3;
		Gvbt = optz->Vbt + net->nbLayers - 3;
	}

	dl *tempLegacy, *OutIn_i;
	for (L = net->layers + (net->nbLayers - 2); L > net->layers; L--) {
		dl (*deriv_i)(dl *, cui, cui) = get_deriv(L->act_name);
		tempLegacy = fvec_alloc(L->pLayer->Neurons, true);
		OutIn_i = fvec_alloc(L->Neurons, false);
		i = 0;
		for (dl *oI = OutIn_i; oI < OutIn_i + L->Neurons; oI++, i++)
			*oI = deriv_i(L->input, L->Neurons, i);
		if (optz != NULL) {
			mwt = *Gmwt;
			vwt = *Gvwt;
			mbt = *Gmbt;
			vbt = *Gvbt;
		}
		bias_done = false;
		for (dl *tL = tempLegacy, *w = L->weights, *pO = L->pLayer->output;
			 tL < tempLegacy + L->pLayer->Neurons; tL++, pO++) {
			for (dl *l = Legacy, *oI = OutIn_i, *b = L->bias;
				 l < Legacy + L->Neurons; l++, oI++, w++, b++) {
				dl ml = (*l) * (*oI), pw = (*w);
				*tL += ml * (*w);
				if (!dn1) l1 += absl(*w);
				if (!dn2) l2 += (*w) * (*w);
				if (optz != NULL) {
					dl gd = ml * (*pO);
					*mwt = 0.9L * (*mwt) + (1 - 0.9) * gd;
					*vwt = 0.999L * (*vwt) + (1 - 0.999L) * gd * gd;
					dl mwc = (*mwt) / (1 - b1t);
					dl vwc = (*vwt) / (1 - b2t);
					*w -= params->l_rate * mwc / (sqrtl(vwc) + EPS);
					mwt++;
					vwt++;
				} else *w -= params->l_rate * ml * (*pO);
				*w += params->l_rate * (pw >= .0L ? 1.0L : -1.0L)
						  * params->l1Norm
					  + params->l_rate * 2 * params->l2Norm * pw;
				if (!bias_done) {
					if (optz != NULL) {
						*mbt = 0.9 * (*mbt) + (1 - 0.9L) * ml;
						*vbt = 0.999L * (*vbt) + (1 - 0.999L) * ml * ml;
						dl mbc = (*mbt) / (1 - b1t);
						dl vbc = (*vbt) / (1 - b2t);
						*b -= params->l_rate * mbc / (sqrtl(vbc));
						mbt++;
						vbt++;
					} else *b -= params->l_rate * ml;
				}
			}
			bias_done = true;
		}
		free(Legacy);
		Legacy = tempLegacy;
		free(OutIn_i);
		if (optz != NULL) {
			Gmwt--;
			Gvwt--;
			Gmbt--;
			Gvbt--;
		}
	}
	free(Legacy);
	return error + params->l1Norm * l1 + params->l2Norm * l2;
}

void Optimizer_Init(Network *net, Optimizer *optz) {
	if (optz == NULL) return;
	optz->iter = 1;
	optz->Mwt = (dl **)malloc(sizeof(dl *) * net->nbLayers - 1);
	optz->Mbt = (dl **)malloc(sizeof(dl *) * net->nbLayers - 1);
	optz->Vwt = (dl **)malloc(sizeof(dl *) * net->nbLayers - 1);
	optz->Vbt = (dl **)malloc(sizeof(dl *) * net->nbLayers - 1);
	for (ui i = 0; i < net->nbLayers - 1; i++) {
		optz->Mwt[i] = fvec_alloc(net->layers[i + 1].conns, true);
		optz->Vwt[i] = fvec_alloc(net->layers[i + 1].conns, true);
		optz->Mbt[i] = fvec_alloc(net->layers[i + 1].Neurons, true);
		optz->Vbt[i] = fvec_alloc(net->layers[i + 1].Neurons, true);
	}
}

void Optimizer_Dispose(Network *net, Optimizer *optz, bool full) {
	if (optz == NULL) return;
	for (ui i = 0; i < net->nbLayers - 1; i++) {
		free(optz->Mwt[i]);
		free(optz->Mbt[i]);
		free(optz->Vwt[i]);
		free(optz->Vbt[i]);
	}
	free(optz->Mwt);
	free(optz->Mbt);
	free(optz->Vwt);
	free(optz->Vbt);
	if (full) free(optz);
}
