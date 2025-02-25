//
// Created by icxd on 11/6/24.
//

#pragma once

#include <LibC/stdarg.h>
#include <LibCore/Defines.hpp>
#include <LibCore/Formatting.hpp>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"

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

#define COLORED_PRINT(name, prefix, color_code)                                \
  template <typename... Args> void name(const char *fmt, Args... args) {       \
    kprintf("[ ");                                                             \
    kprintf("\033[" #color_code ";1m" prefix);                                 \
    kprintf("\033[0m ] ");                                                     \
    kprintf(Core::format(fmt, Core::forward<Args>(args)...).characters());     \
  }                                                                            \
  template <typename... Args> void name##ln(const char *fmt, Args... args) {   \
    name(fmt, args...);                                                        \
    kputchar('\n');                                                            \
  }

COLORED_PRINT(debug, "DBG ", 37);
COLORED_PRINT(ok, "OK  ", 32);
COLORED_PRINT(warn, "WARN", 33);
COLORED_PRINT(error, "ERR ", 31);

#define DBG(expr) debugln(#expr " = {}", expr)

#undef COLORED_PRINT
