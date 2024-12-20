//
// Created by icxd on 11/6/24.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define va_list __builtin_va_list

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

#ifdef __cplusplus
}
#endif
