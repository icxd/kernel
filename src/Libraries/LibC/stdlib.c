//
// Created by icxd on 11/6/24.
//

#include "stdlib.h"
#include "math.h"
#include "stdbool.h"
#include "stddef.h"
#include "string.h"

static char *reverse(char *s) {
  size_t len = strlen(s);
  for (size_t i = 0; i < len / 2; i++) {
    // Swap characters
    char temp = s[i];
    s[i] = s[len - i - 1];
    s[len - i - 1] = temp;
  }
  return s;
}

char *itoa(int value, char *buffer, int radix) {
  if (value == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return buffer;
  }
  char *ptr = buffer;
  int negative = 0;
  if (value < 0) {
    negative = 1;
    value = -value;
  }
  while (value) {
    *ptr++ = "0123456789abcdef"[value % radix];
    value /= radix;
  }
  if (negative)
    *ptr++ = '-';
  *ptr = '\0';
  return reverse(buffer);
}

char *utoa(unsigned value, char *buffer, int radix) {
  return ultoa(value, buffer, radix);
}

char *ltoa(long value, char *buffer, int radix) {
  if (value == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return buffer;
  }
  char *ptr = buffer;
  bool negative = false;
  if (value < 0) {
    negative = true;
    value = -value;
  }
  while (value) {
    *ptr++ = "0123456789abcdef"[value % radix];
    value /= radix;
  }
  if (negative) {
    *ptr++ = '-';
  }
  *ptr = '\0';
  return reverse(buffer);
}

char *ultoa(unsigned long value, char *buffer, int radix) {
  if (value == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return buffer;
  }

  char *ptr = buffer;
  bool negative = false;
  if (value < 0) {
    negative = true;
    value = -value;
  }
  while (value) {
    *ptr++ = "0123456789abcdef"[value % radix];
    value /= radix;
  }
  if (negative) {
    *ptr++ = '-';
  }
  *ptr = '\0';
  return reverse(buffer);
}

static double PRECISION = 0.00000000000001;

char *ftoa(float value, char *buffer) { return dtoa(value, buffer); }

char *dtoa(double n, char *s) {
  if (isnan(n)) {
    strcpy(s, "nan");
  } else if (isinf(n)) {
    strcpy(s, "inf");
  } else if (n == 0.0) {
    strcpy(s, "0");
  } else {
    int digit, m, m1;
    char *c = s;
    int neg = (n < 0);
    if (neg)
      n = -n;
    // calculate magnitude
    m = log10(n);
    int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
    if (neg)
      *(c++) = '-';
    // set up for scientific notation
    if (useExp) {
      if (m < 0)
        m -= 1.0;
      n = n / pow(10.0, m);
      m1 = m;
      m = 0;
    }
    if (m < 1.0) {
      m = 0;
    }
    // convert the number
    while (n > PRECISION || m >= 0) {
      double weight = pow(10.0, m);
      if (weight > 0 && !isinf(weight)) {
        digit = floor(n / weight);
        n -= (digit * weight);
        *(c++) = '0' + digit;
      }
      if (m == 0 && n > 0)
        *(c++) = '.';
      m--;
    }
    if (useExp) {
      // convert the exponent
      int i, j;
      *(c++) = 'e';
      if (m1 > 0) {
        *(c++) = '+';
      } else {
        *(c++) = '-';
        m1 = -m1;
      }
      m = 0;
      while (m1 > 0) {
        *(c++) = '0' + m1 % 10;
        m1 /= 10;
        m++;
      }
      c -= m;
      for (i = 0, j = m - 1; i < j; i++, j--) {
        // swap without temporary
        c[i] ^= c[j];
        c[j] ^= c[i];
        c[i] ^= c[j];
      }
      c += m;
    }
    *(c) = '\0';
  }
  return s;
}
