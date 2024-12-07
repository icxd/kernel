//
// Created by icxd on 11/6/24.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  char *itoa(int value, char *buffer, int radix);
  char *utoa(unsigned value, char *buffer, int radix);
  char *ltoa(long value, char *buffer, int radix);
  char *ultoa(unsigned long value, char *buffer, int radix);

  char *ftoa(float value, char *buffer);
  char *dtoa(double value, char *buffer);

#ifdef __cplusplus
}
#endif
