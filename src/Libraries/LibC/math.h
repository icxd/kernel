//
// Created by icxd on 11/6/24.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define EPSILON 1e-10
#define MAX_ITER 100

#define isnan(x) __builtin_isnan(x)
#define isinf(x) __builtin_isinf_sign(x)

double exp(double x);
double log(double a);
double pow(double x, double y);

int log10(double x);
int floor(double x);

#ifdef __cplusplus
}
#endif