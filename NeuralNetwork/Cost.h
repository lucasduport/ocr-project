#ifndef COST_H
#define COST_H

#define UNUSED(x) (void)(x)

#include <math.h>
#include <float.h>
#include "Tools.h"


dl CrossEntropy(dl *predicted, dl *expected, cui Size);
dl RMSE(dl *predicted, dl *expected, cui Size);
dl MAE(dl *predicted, dl *expected, cui Size);
dl MSE(dl *predicted, dl *expected, cui Size);

dl CrossEntropy_(cdl predicted, cdl expected);
dl RMSE_(cdl predicted, cdl expected);
dl MAE_(cdl predicted, cdl expected);
dl MSE_(cdl predicted, cdl expected);

dl none_(dl *arr, cui Size, cui ieme);
dl sigmoid_(dl *arr, cui Size, cui ieme);
dl softmax_(dl *arr, cui Size, cui ieme);
dl argmax_(dl *arr, cui Size, cui ieme);
dl step_(dl *arr, cui Size, cui ieme);
dl relu_(dl *arr, cui Size, cui ieme);
dl selu_(dl *arr, cui Size, cui ieme);
dl leakyrelu_(dl *arr, cui Size, cui ieme);

dl (*get_deriv(const char *name))(dl *arr, cui Size, cui ieme);
dl (*get_cost(const char *name))(dl *predicted, dl *expected,cui Size);
dl (*get_cost_deriv(const char *name))(dl predicted, dl expected);

const static struct {
  const char *name;
  dl (*func)(dl *arr, cui Size, cui ieme);
} deriv_map [] = {
  { "none", none_ },
  { "sigmoid", sigmoid_ },
  { "softmax", softmax_ },
  { "argmax", argmax_ },
  { "step", step_ },
  { "relu", relu_ },
  { "selu", selu_ },
  { "leakyrelu", leakyrelu_ }
};

const static struct {
  const char *name;
  dl (*func)(dl *predicted, dl *expected, cui Size);
} cost_map [] = {
  { "CrossEntropy", CrossEntropy },
  { "RMSE", RMSE },
  { "MAE", MAE },
  { "MSE", MSE }
};

const static struct {
  const char *name;
  dl (*func)(dl predicted, dl expected);
} cost_deriv_map [] = {
  { "CrossEntropy", CrossEntropy_ },
  { "RMSE", RMSE_ },
  { "MAE", MAE_ },
  { "MSE", MSE_ }
};

#endif // COST_H
