#ifndef ACTIVATIONS_H
#define ACTIVATIONS_H

#include <math.h>
#include <float.h>
#include "Tools.h"

void sigmoid(dl *input, dl *output, cui Size);
void softmax(dl *input, dl *output, cui Size);
void argmax(dl *input, dl *output, cui Size);
void step(dl *input, dl *output, cui Size);
void relu(dl *input, dl *output, cui Size);
void selu(dl *input, dl *output, cui Size);
void leakyrelu(dl *input, dl *output, cui Size);
void none(dl *input, dl *output, cui Size);

void (*get_activation(const char *name))(dl *input, dl *output, cui Size);

const static struct {
  const char *name;
  void (*func)(dl *input, dl *output, cui Size);
} function_map [] = {
  { "none", none },
  { "sigmoid", sigmoid },
  { "softmax", softmax },
  { "argmax", argmax },
  { "step", step },
  { "relu", relu },
  { "selu", selu },
  { "leakyrelu", leakyrelu }
};

#endif // ACTIVATIONS_H
