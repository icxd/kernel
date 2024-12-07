//
// Created by icxd on 11/6/24.
//

#pragma once

#include <LibCore/Defines.hpp>
#include <LibCore/Formatting.hpp>
#include <LibC/stdarg.h>

void kputchar(char ch);
void kputs(const char *s);
void kprintf(const char *fmt, ...);
void kvprintf(const char *fmt, va_list ap);

template <typename... Args> void print(const char *fmt, Args... args) {
  kprintf(Core::format(fmt, Core::forward<Args>(args)...).characters());
}

template <typename... Args> void println(const char *fmt, Args... args) {
  print(fmt, args...);
  kputchar('\n');
}

#define COLORED_PRINT(name, color_code) \
template <typename... Args> void name(const char *fmt, Args... args) { \
  kprintf("\033[" #color_code "m");     \
  kprintf(Core::format(fmt, Core::forward<Args>(args)...).characters()); \
  kprintf("\033[0m");                   \
}                                       \
template <typename... Args> void name##ln(const char *fmt, Args... args) {                           \
  name(fmt, args...);                   \
  kputchar('\n');                       \
}

COLORED_PRINT(debug, 34);
COLORED_PRINT(warn, 33);
COLORED_PRINT(error, 31);

#undef COLORED_PRINT
