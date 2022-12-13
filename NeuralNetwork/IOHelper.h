#ifndef IOHELPER_H
#define IOHELPER_H

#include <stdio.h>
#include <stdlib.h>

#include "Network.h"

Network* NetCreate(NNParam *param);
void LoadData(NNParam* param);
float Validate(Network *net, const NNParam *P, float bperf);
void ConfusionMatrix(Network *net, const NNParam *P);
void OverfitLoad(NNParam *param);

void PerfSearch(NNParam *origin, Network *netOrg, int attempt);
void NNParam_Display(NNParam *param);
void Purge_NNParam(NNParam *param);

#endif // IOHELPER_H
