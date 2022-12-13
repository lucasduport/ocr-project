#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "IOHelper.h"
#define MAX_IN_LINE 50
#define MAX_IN_FIELD (MAX_IN_LINE-3)/2

static void matchParams(NNParam *origin, char *line) {
	char fiedl[MAX_IN_FIELD], value[MAX_IN_FIELD];
	sscanf(line, "%s = %s", fiedl, value);

	if (!strcmp(fiedl, "toLoopTrain"))
		origin->toLoopTrain = (ui)strtol(value, NULL, 10);
	else if (!strcmp(fiedl, "toLoopValidate"))
		origin->toLoopValidate = (ui)strtol(value, NULL, 10);
	else if (!strcmp(fiedl, "epoch"))
		origin->epoch = (ui)strtol(value, NULL, 10);
	else if (!strcmp(fiedl, "epochInterval"))
		origin->epochInterval = (ui)strtol(value, NULL, 10);
	else if (!strcmp(fiedl, "l_rate")) origin->l_rate = (dl)strtod(value, NULL);
	else if (!strcmp(fiedl, "l1Norm")) origin->l1Norm = (dl)strtod(value, NULL);
	else if (!strcmp(fiedl, "l2Norm")) origin->l2Norm = (dl)strtod(value, NULL);
	else if (!strcmp(fiedl, "cost_func")) {
		origin->cost_func = (char *)malloc(sizeof(char) * (strlen(value) + 1));
		strcpy(origin->cost_func, value);
	} else if (!strcmp(fiedl, "optimizer")) {
		origin->optimizer = !strcmp(value, "true")
								? (Optimizer *)malloc(sizeof(Optimizer))
								: NULL;
	} else if (!strcmp(fiedl, "track"))
		origin->track = !strcmp(value, "true") ? true : false;
	else if (!strcmp(fiedl, "StatsFile")) {
		origin->StatsFile = (char *)malloc(sizeof(char) * (strlen(value) + 1));
		strcpy(origin->StatsFile, value);
	} else if (!strcmp(fiedl, "NNName")) {
		origin->NNName = (char *)malloc(sizeof(char) * (strlen(value) + 1));
		strcpy(origin->NNName, value);
	} else if (!strcmp(fiedl, "toExceed"))
		origin->toExceed = (dl)strtod(value, NULL);
    else if (!strcmp(fiedl, "iSize")) origin->iSize = (ui)strtol(value, NULL, 10);
    else if (!strcmp(fiedl, "oSize")) origin->oSize = (ui)strtol(value, NULL, 10);
    else if (!strcmp(fiedl, "nbLayer")) {
        ui nbl = (ui)strtol(value, NULL, 10);
        origin->nbLayer = nbl;
        origin->hNeurons = malloc(sizeof(ui) * (nbl-2));
        origin->act_funcs = malloc(sizeof(char *) * (nbl-2));
    }
    else if (!strcmp(fiedl, "hLayer")) {
        ui nbn;
        char naf[MAX_IN_FIELD];
        sscanf(value, "%u|%s", &nbn, naf);
        origin->hNeurons[origin->currH] = nbn;
        origin->act_funcs[origin->currH] = malloc(sizeof(char) * (strlen(naf) + 1));
        strcpy(origin->act_funcs[origin->currH], naf);
        origin->currH++;
    }
    else if (!strcmp(fiedl, "endLayer")) {
        origin->endLayerAct = malloc(sizeof(char) * (strlen(value) + 1));
		strcpy(origin->endLayerAct, value);
    }
}

int main(int argc, char *argv[]) {



/*
	int argc = 6;
	char *argv[6] = {"Train", "TrainedNetwork/NN.cfg",
                    "DataSets/hcd_784_400038_training.bin",
                    "DataSets/hcd_784_67143_validation.bin", "1",
                    "TrainedNetwork/NeuralNetData_3layers_OCR-MEXA_92.55.dnn"};
*/

	argc -= 2;
	argv++;


	srand((ui)time(NULL));

	if (argc < 1) {
		printf("Missing argument.");
		exit(1);
	}

	if (!strcmp(argv[0], "Train")) {
		if (argc < 4) {
			printf("\nParameter Error: Expected 4; Got %d\n", argc - 1);
			printf("Help:\n\tTrain [config file (.cfg)] ");
			printf("[Training data (.bin)] [Validation data (.bin)]\n");
			printf("[number of attempt] [Transfer Network (.dnn)]\n");
			exit(1);
		}

		FILE *configFile = NULL;
		if ((configFile = fopen(argv[1], "rb")) == NULL) {
			fprintf(stderr, "Cannot open file '%s'\n", argv[1]);
			exit(1);
		}

		NNParam *origin = malloc(sizeof(NNParam));
		origin->currH = 0;
		char *buffer = malloc(sizeof(char) * MAX_IN_LINE);
		while (!feof(configFile)) {
			fgets(buffer, MAX_IN_LINE, configFile);
			if (ferror(configFile)) {
				fprintf(stderr, "Reading error with code %d\n", errno);
				exit(1);
			}
			matchParams(origin, buffer);
		}
		free(buffer);
		fclose(configFile);
        origin->trainingFile = argv[2];
        origin->validationFile = argv[3];
        Network *net = NULL;
        if (argc > 4) {
            printf("%u args\n", argc);
            net = malloc(sizeof(Network));
            Network_Load(net, argv[5]);
        }
        puts("karaba");
		LoadData(origin);
        puts("mamadou");
		PerfSearch(origin, net, (int)strtol(argv[4], NULL, 10));
		if (argc > 4) Network_Purge(net);
		Purge_NNParam(origin);

	} else {
		printf("\nUnknown Command '%s'\n", argv[0]);
		exit(1);
	}

	return 0;
}
