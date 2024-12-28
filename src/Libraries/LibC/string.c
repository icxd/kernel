//
// Created by icxd on 11/1/24.
//

#include "string.h"

unsigned long long strlen(const char *str) {
  unsigned long long i = 0;
  while (str[i] != '\0')
    i++;
  return i;
}

char *strcpy(char *dest, const char *src) {
  char *d = dest;
  while ((*d++ = *src++) != '\0')
    ;
  return dest;
}

void *memcpy(void *dest, const void *src, unsigned long long n) {
  char *d = (char *)dest;
  const char *s = (const char *)src;
  for (unsigned long long int i = 0; i < n; i++)
    d[i] = s[i];
  return dest;
}

void *memset(void *s, int c, unsigned long long n) {
  char *d = (char *)s;
  for (unsigned long long int i = 0; i < n; i++)
    d[i] = c;
  return s;
}