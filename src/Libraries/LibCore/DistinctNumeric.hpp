#pragma once

#include "Defines.hpp"
#include "Types.hpp"

namespace Core {

#define CORE_MAKE_DISTINCT_NUMERIC_TYPE(NAME, BASE_T)                          \
  struct NAME {                                                                \
    constexpr explicit NAME(BASE_T value = 0) : m_value(value) {}              \
                                                                               \
    constexpr BASE_T value() const { return m_value; }                         \
                                                                               \
    constexpr NAME &operator=(BASE_T value) {                                  \
      m_value = value;                                                         \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    constexpr bool operator==(const NAME &other) const {                       \
      return m_value == other.m_value;                                         \
    }                                                                          \
                                                                               \
    constexpr bool operator!=(const NAME &other) const {                       \
      return m_value != other.m_value;                                         \
    }                                                                          \
                                                                               \
    constexpr NAME operator+(const NAME &other) const {                        \
      return NAME(m_value + other.m_value);                                    \
    }                                                                          \
                                                                               \
    constexpr NAME operator-(const NAME &other) const {                        \
      return NAME(m_value - other.m_value);                                    \
    }                                                                          \
                                                                               \
    constexpr NAME operator*(const NAME &other) const {                        \
      return NAME(m_value * other.m_value);                                    \
    }                                                                          \
                                                                               \
    constexpr NAME operator/(const NAME &other) const {                        \
      return NAME(m_value / other.m_value);                                    \
    }                                                                          \
                                                                               \
    constexpr NAME &operator+=(const NAME &other) {                            \
      m_value += other.m_value;                                                \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    constexpr NAME &operator-=(const NAME &other) {                            \
      m_value -= other.m_value;                                                \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    constexpr NAME &operator*=(const NAME &other) {                            \
      m_value *= other.m_value;                                                \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    constexpr NAME &operator/=(const NAME &other) {                            \
      m_value /= other.m_value;                                                \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    constexpr operator BASE_T() const { return m_value; }                      \
                                                                               \
  private:                                                                     \
    BASE_T m_value;                                                            \
  };                                                                           \
  template <> struct Core::Formatter<NAME> {                                   \
    static String format(NAME value) {                                         \
      return StringBuilder()                                                   \
          .append(#NAME "(")                                                   \
          .append(value.value())                                               \
          .append(")")                                                         \
          .build();                                                            \
    }                                                                          \
  }

} // namespace Core
