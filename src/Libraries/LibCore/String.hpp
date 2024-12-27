//
// Created by icxd on 11/1/24.
//


#pragma once

#include "Defines.hpp"
#include "FormatTypes.hpp"
#include "Types.hpp"
#include <LibC/string.h>

namespace Core {

// This is an immutable string class.
class String {
  public:
  explicit String(const char *string) : m_string(string), m_size(strlen(string)) {}
  String(const char *string, const usz size) : m_string(string), m_size(size) {}

  [[nodiscard]] String substring(usz start, usz end) const;
  [[nodiscard]] String substring(const usz start) const { return substring(start, size()); }
  [[nodiscard]] isz find(char character, usz start = 0) const;

  [[nodiscard]] char at(usz index) const;

  [[nodiscard]] const char *characters() const { return m_string; }
  [[nodiscard]] usz size() const { return m_size; }
  [[nodiscard]] char operator[](const usz index) const { return at(index); }

  private:
  const char *m_string;
  usz m_size;
};

class StringBuilder {
  public:
  StringBuilder() {
    m_capacity = 128;
    m_size = 0;
    m_string = new char[m_capacity];
    m_string[0] = '\0';
  }

  StringBuilder &append(const char *string, usz size);
  StringBuilder &append(const char *string);
  StringBuilder &append(const String &string);
  StringBuilder &append(char character);

  StringBuilder &append(
    int value,
    FormatterIntegerRepresentation type = FormatterIntegerRepresentation::None);
  StringBuilder &append(
    i64 value,
    FormatterIntegerRepresentation type = FormatterIntegerRepresentation::None);
  // StringBuilder &append(
  //     isz value,
  //     FormatterIntegerRepresentation type = FormatterIntegerRepresentation::None
  // );
  StringBuilder &append(
    u32 value,
    FormatterIntegerRepresentation type = FormatterIntegerRepresentation::None);
  StringBuilder &append(
    u64 value,
    FormatterIntegerRepresentation type = FormatterIntegerRepresentation::None);
  // StringBuilder &append(
  //     usz value,
  //     FormatterIntegerRepresentation type = FormatterIntegerRepresentation::None
  // );

  StringBuilder &append(
    float value,
    FormatterFloatingRepresentation type = FormatterFloatingRepresentation::None);
  StringBuilder &append(
    double value,
    FormatterFloatingRepresentation type = FormatterFloatingRepresentation::None);

  [[nodiscard]] String build() const { return {m_string, m_size}; }

  private:
  char *m_string;
  usz m_size, m_capacity = 0;
};

}// namespace Core

using Core::String;
using Core::StringBuilder;
