//
// Created by icxd on 11/1/24.
//

#include "string.h"

size_t strlen(const char *str) {
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

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n && *s1 && (*s1 == *s2)) {
    s1++;
    s2++;
    n--;
  }
  return n ? (unsigned char)*s1 - (unsigned char)*s2 : 0;
}

void *memcpy(void *dest, const void *src, size_t n) {
  char *d = (char *)dest;
  const char *s = (const char *)src;
  for (size_t i = 0; i < n; i++)
    d[i] = s[i];
  return dest;
}

void *memset(void *s, int c, size_t n) {
  char *d = (char *)s;
  for (size_t i = 0; i < n; i++)
    d[i] = c;
  return s;
}
