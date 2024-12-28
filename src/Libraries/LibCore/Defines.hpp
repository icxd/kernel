//
// Created by icxd on 10/29/24.
//

#pragma once

#include <LibC/stdarg.h>

template <typename... Args>
extern void kpanic(const char *, __SIZE_TYPE__, const char *, Args...);

#define ALIGNED(x) __attribute__((aligned(x)))
#define PACKED __attribute__((packed))
#define NORETURN __attribute__((noreturn))

#define CORE_MAKE_NONCOPYABLE(T)                                               \
  T(T const &) = delete;                                                       \
  T &operator=(T const &) = delete;
#define CORE_MAKE_NONMOVABLE(T)                                                \
  T(T &&) = delete;                                                            \
  T &operator=(T &&) = delete;

constexpr unsigned KB = 1024;
constexpr unsigned MB = KB * KB;
constexpr unsigned GB = KB * KB * KB;

#define ASSERT(x)                                                              \
  if (!(x))                                                                    \
  kpanic(__FILE__, __LINE__, __func__, "Assertion failed: {}", #x)
#define PANIC(...) kpanic(__FILE__, __LINE__, __func__, __VA_ARGS__)

NORETURN static void hcf() {
  for (;;)
    asm volatile("hlt");
}

#define CONCAT_HELPER(a, b) a##b
#define CONCAT(a, b) CONCAT_HELPER(a, b)
#define COUNTER_NAME(x) CONCAT(x, __COUNTER__)

#define defer                                                                  \
  [[maybe_unused]] const auto &COUNTER_NAME(DEFER_) =                          \
      ::Core::DeferHelper() + [&]()
