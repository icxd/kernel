//
// Created by icxd on 11/11/24.
//

#pragma once

#include "Types.hpp"
#include "Formatting.hpp"

#define TRY(expr) \
  ({ \
    auto _try_result = (expr); \
    if (!_try_result.has_value()) \
      return Error(_try_result.error().code()); \
    _try_result.value(); \
  })
#define TRY_OR_PANIC(expr) \
  ({ \
    auto _try_result = (expr); \
    if (!_try_result.has_value()) \
      PANIC(#expr); \
    _try_result.value(); \
  })

namespace Core {

class Error {
public:
  explicit Error(int error_code) : m_error_code(error_code) {}

  [[nodiscard]] int code() const { return m_error_code; }

private:
  int m_error_code = 0;
};

template <typename T>
class ErrorOr {
public:
  ErrorOr(T value) : m_has_value(true), m_value(move(value)) {}
  ErrorOr(Error error) : m_has_value(false), m_error(move(error)) {}

  T value_or(T other) { return m_has_value ? value() : other; }
  Error error_or(Error other) { return m_has_value ? error() : other; }

  [[nodiscard]] bool has_value() const { return m_has_value; }
  [[nodiscard]] Error error() const { return m_error; }
  [[nodiscard]] T value() const { return m_value; }

private:
  bool m_has_value;
  union {
    T m_value;
    Error m_error;
  };

};

template <>
class ErrorOr<void> {
public:
  ErrorOr(Error error) : m_has_value(false), m_error(move(error)) {}

  [[nodiscard]] bool has_value() const { return m_has_value; }
  [[nodiscard]] Error error() const { return m_error; }

private:
  bool m_has_value;
  Error m_error;
};

template <>
struct Formatter<Error> {
  static String format(Error value) {
    return StringBuilder()
        .append("Error(")
        .append(value.code())
        .append(")")
        .build();
  }
};

} // namespace Core

using Core::Error;
using Core::ErrorOr;