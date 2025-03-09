//
// Created by icxd on 11/1/24.
//

#pragma once

#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char *str);
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

#ifdef __cplusplus
}
#endif
