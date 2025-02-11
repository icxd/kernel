//
// Created by icxd on 10/28/24.
//

#pragma once

#include "Trait.hpp"

using i8 = signed char;
using u8 = unsigned char;
using i16 = signed short;
using u16 = unsigned short;
using i32 = signed int;
using u32 = unsigned int;
using i64 = signed long long;
using u64 = unsigned long long;
using isz = __PTRDIFF_TYPE__;
using usz = __SIZE_TYPE__;

extern "C" void *kmalloc(usz);
extern "C" void kfree(void *);

using uid_t = u32;
using gid_t = u32;
using pid_t = i32;

namespace Core {

  template <typename T> constexpr T &&forward(RemoveReference<T> &t) {
    return static_cast<T &&>(t);
  }
  template <typename T> constexpr T &&forward(RemoveReference<T> &&t) {
    return static_cast<T &&>(t);
  }
  template <typename T> constexpr T &&forward(T &t) {
    return static_cast<T &&>(t);
  }
  template <typename T> constexpr T &&forward(T &&t) {
    return static_cast<T &&>(t);
  }

  template <typename T> constexpr T &&move(T &t) {
    return static_cast<T &&>(t);
  }

  template <typename T> inline T min(const T &a, const T &b) {
    return a < b ? a : b;
  }

  template <typename T> inline T max(const T &a, const T &b) {
    return a < b ? b : a;
  }

  template <typename First, typename Second>
  void swap(First &first, Second &second) {
    using T = UnderlyingType<First>;
    T first_copy = move(first);
    first = move(second);
    second = move(first_copy);
  }

  template <typename F> struct ScopeExit {
    ScopeExit(F f_) : f(f_) {}
    ~ScopeExit() { f(); }
    F f;
  };

  struct DeferHelper {
    template <typename F> ScopeExit<F> operator+(F f) { return f; }
  };

  template <typename T> static inline T ceil_div(T a, T b) {
    T result = a / b;
    if ((a % b) != 0)
      result++;
    return result;
  }

} // namespace Core

using Core::max;
using Core::min;
