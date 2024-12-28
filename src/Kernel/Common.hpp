//
// Created by icxd on 11/12/24.
//

#pragma once

#include "kprintf.hpp"
#include <LibCore/Formatting.hpp>
#include <LibCore/Types.hpp>

template <typename... Args>
void kpanic(const char *file, usz line, const char *fn, Args... args) {
  print("\033[31;1mPANIC! (at {}:{} in {}): \033[0m", file, line, fn);
  errorln(args...);
  hcf();
}
