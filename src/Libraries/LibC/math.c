//
// Created by icxd on 11/6/24.
//

#include "math.h"

double exp(double x) {
  double term = 1, sum = 1;

  for (int n = 1; n < 50; n++) {
    term *= x / n;
    sum += term;
    if (term < EPSILON)
      break;
  }

  return sum;
}

double log(double a) {
  if (a <= 0)
    return -1.0 / 0.0;

  double x = a - 1;
  for (int i = 0; i < MAX_ITER; i++) {
    double exp_x = exp(x);
    double diff = (exp_x - a) / exp_x;
    x -= diff;

    if (diff < EPSILON && diff > -EPSILON)
      break;
  }

  return x;
}

double pow(double x, double y) { return exp(y * log(x)); }

int log10(double x) { return log(x) / log(10); }

int floor(double x) {
  int i = (int)x;
  return (x < i) ? (i - 1) : i;
}
