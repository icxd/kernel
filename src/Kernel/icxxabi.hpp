//
// Created by icxd on 10/29/24.
//

#pragma once

#include <LibCpp/cstddef.hpp>

#define ATEXIT_MAX_FUNCS 128

typedef unsigned uarch_t;

struct atexit_func_entry_t {
  void (*destructor_func)(void *);
  void *obj_ptr, *dso_handle;
};

int __cxa_atexit(void (*f)(void *), void *obj_ptr, void *dso);
void __cxa_finalize(void *f);
