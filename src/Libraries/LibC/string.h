//
// Created by icxd on 11/1/24.
//


#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  unsigned long long strlen(const char *str);
  char *strcpy(char *dest, const char *src);

  void *memcpy(void *dest, const void *src, unsigned long long n);
  void *memset(void *s, int c, unsigned long long n);

#ifdef __cplusplus
}
#endif
