//
// Created by icxd on 11/6/24.
//

#include "Common.hpp"
#include "kprintf.hpp"
#include <LibC/string.h>
#include <LibC/stdlib.h>
#include <LibCore/Types.hpp>
#include "Drivers/Serial.hpp"

void kputchar(char ch) { Serial::write(ch); }
void kputs(const char *s) { while (*s) kputchar(*s++); }

void kprintf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  kvprintf(fmt, ap);
  va_end(ap);
}

void kvprintf(const char *fmt, va_list ap) {
  usz length = strlen(fmt);
  usz i = 0;
  while (i < length) {
    if (fmt[i] == '%') {
      i++;

      /*
       * Format: %[flags][width][.precision][size]type
       *
       * Flags:
       *   '-' - left aligned
       *   '+' - print sign
       *   '0' - width prefixed with zeros
       *   ' ' -
       *
       */

      if (fmt[i] == '%') {
        kputchar('%');
        i++;
        continue;
      }

      // TODO: flags, width, precision, size

      char buffer[32];
      switch (fmt[i]) {
      case 'd':
        itoa(va_arg(ap, int), buffer, 10);
        kputs(buffer);
        break;

      case 'u':
        utoa(va_arg(ap, unsigned), buffer, 10);
        kputs(buffer);
        break;

      case 'x':
        utoa(va_arg(ap, unsigned), buffer, 16);
        kputs(buffer);
        break;

      case 'b':
        itoa(va_arg(ap, int), buffer, 2);
        kputs(buffer);
        break;

      case 's':
        kputs(va_arg(ap, const char *));
        break;

      case 'c':
        kputchar(va_arg(ap, int));
        break;

      case 'f':
      case 'e':
        dtoa(va_arg(ap, double), buffer);
        kputs(buffer);
        break;

      default:
        PANIC("Unsupported format specifier: {}", fmt[i]);
      }

    } else {
      kputchar(fmt[i]);
    }

    i++;
  }
}
