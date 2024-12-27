//
// Created by icxd on 11/1/24.
//

#include "String.hpp"
#include <LibC/stdlib.h>
#include <LibC/string.h>

namespace Core {

char String::at(const usz index) const {
  // TODO: bounds check
  return m_string[index];
}

isz String::find(const char character, const usz start) const {
  for (usz i = start; i < size(); i++) {
    if (m_string[i] == character)
      return static_cast<isz>(i);
  }
  return -1;
}

String String::substring(const usz start, const usz end) const {
  ASSERT(start <= end);
  ASSERT(end <= size());
  return {m_string + start, end - start};
}

StringBuilder &StringBuilder::append(const char *string, const usz size) {
  ASSERT(size <= m_capacity);
  if (m_size + size > m_capacity) {
    usz new_capacity = m_capacity * 2;
    if (new_capacity < m_size + size) {
      new_capacity = m_size + size;
    }

    const auto new_string = new char[new_capacity];
    memcpy(new_string, m_string, m_size);
    delete[] m_string;
    m_string = new_string;
    m_capacity = new_capacity;
  }

  memcpy(m_string + m_size, string, size + 1);
  m_size += size;
  m_string[m_size] = '\0';
  return *this;
}

StringBuilder &StringBuilder::append(const char *string) {
  const usz size = strlen(string);
  append(string, size);
  return *this;
}
StringBuilder &StringBuilder::append(const String &string) {
  return append(string.characters(), string.size());
}

StringBuilder &StringBuilder::append(const char character) {
  if (m_size + 1 > m_capacity) {
    usz new_capacity = m_capacity * 2;
    if (new_capacity < m_size + 1) {
      new_capacity = m_size + 1;
    }

    const auto new_string = new char[new_capacity];
    memcpy(new_string, m_string, m_size);
    delete[] m_string;
    m_string = new_string;
    m_capacity = new_capacity;
  }

  m_string[m_size++] = character;
  m_string[m_size] = '\0';// TODO: This is probably not necessary
  return *this;
}

StringBuilder &StringBuilder::append(
  const int value,
  const FormatterIntegerRepresentation type) {
  int radix = 10;
  switch (type) {
  case FormatterIntegerRepresentation::Binary:
    radix = 2;
    break;
  case FormatterIntegerRepresentation::Octal:
    radix = 8;
    break;
  case FormatterIntegerRepresentation::Decimal:
  case FormatterIntegerRepresentation::Number:
    radix = 10;
    break;
  case FormatterIntegerRepresentation::Hexadecimal:
  case FormatterIntegerRepresentation::HexadecimalUpper:
    radix = 16;
    break;
  case FormatterIntegerRepresentation::Character:
    append(static_cast<char>(value));
    return *this;
  case FormatterIntegerRepresentation::None:
    break;
  }

  char buffer[32];
  itoa(value, buffer, radix);
  append(buffer);
  return *this;
}

StringBuilder &StringBuilder::append(
  const i64 value,
  const FormatterIntegerRepresentation type) {
  int radix = 10;
  switch (type) {
  case FormatterIntegerRepresentation::Binary:
    radix = 2;
    break;
  case FormatterIntegerRepresentation::Octal:
    radix = 8;
    break;
  case FormatterIntegerRepresentation::Decimal:
  case FormatterIntegerRepresentation::Number:
    radix = 10;
    break;
  case FormatterIntegerRepresentation::Hexadecimal:
  case FormatterIntegerRepresentation::HexadecimalUpper:
    radix = 16;
    break;
  case FormatterIntegerRepresentation::Character:
    append(static_cast<char>(value));
    return *this;
  case FormatterIntegerRepresentation::None:
    break;
  }

  char buffer[32];
  ltoa(value, buffer, radix);
  append(buffer);
  return *this;
}

// StringBuilder &StringBuilder::append(const isz value, const FormatterIntegerRepresentation type) {
//   int radix = 10;
//   switch (type) {
//   case FormatterIntegerRepresentation::Binary: radix = 2; break;
//   case FormatterIntegerRepresentation::Octal: radix = 8; break;
//   case FormatterIntegerRepresentation::Decimal:
//   case FormatterIntegerRepresentation::Number: radix = 10; break;
//   case FormatterIntegerRepresentation::Hexadecimal:
//   case FormatterIntegerRepresentation::HexadecimalUpper: radix = 16; break;
//   case FormatterIntegerRepresentation::Character: append(static_cast<char>(value)); return *this;
//   case FormatterIntegerRepresentation::None: break;
//   }

//   char buffer[32];
//   itoa(value, buffer, radix);
//   append(buffer);
//   return *this;
// }

StringBuilder &StringBuilder::append(
  const u32 value,
  const FormatterIntegerRepresentation type) {
  int radix = 10;
  switch (type) {
  case FormatterIntegerRepresentation::Binary:
    radix = 2;
    break;
  case FormatterIntegerRepresentation::Octal:
    radix = 8;
    break;
  case FormatterIntegerRepresentation::Decimal:
  case FormatterIntegerRepresentation::Number:
    radix = 10;
    break;
  case FormatterIntegerRepresentation::Hexadecimal:
  case FormatterIntegerRepresentation::HexadecimalUpper:
    radix = 16;
    break;
  case FormatterIntegerRepresentation::Character:
    append(static_cast<char>(value));
    return *this;
  case FormatterIntegerRepresentation::None:
    break;
  }

  char buffer[32];
  utoa(value, buffer, radix);
  append(buffer);
  return *this;
}

StringBuilder &StringBuilder::append(
  const u64 value,
  const FormatterIntegerRepresentation type) {
  int radix = 10;
  switch (type) {
  case FormatterIntegerRepresentation::Binary:
    radix = 2;
    break;
  case FormatterIntegerRepresentation::Octal:
    radix = 8;
    break;
  case FormatterIntegerRepresentation::Decimal:
  case FormatterIntegerRepresentation::Number:
    radix = 10;
    break;
  case FormatterIntegerRepresentation::Hexadecimal:
  case FormatterIntegerRepresentation::HexadecimalUpper:
    radix = 16;
    break;
  case FormatterIntegerRepresentation::Character:
    append(static_cast<char>(value));
    return *this;
  case FormatterIntegerRepresentation::None:
    break;
  }

  char buffer[32];
  ultoa(value, buffer, radix);
  append(buffer);
  return *this;
}

// StringBuilder &StringBuilder::append(usz value, FormatterIntegerRepresentation type) {
//   int radix = 10;
//   switch (type) {
//   case FormatterIntegerRepresentation::Binary: radix = 2; break;
//   case FormatterIntegerRepresentation::Octal: radix = 8; break;
//   case FormatterIntegerRepresentation::Decimal:
//   case FormatterIntegerRepresentation::Number: radix = 10; break;
//   case FormatterIntegerRepresentation::Hexadecimal:
//   case FormatterIntegerRepresentation::HexadecimalUpper: radix = 16; break;
//   case FormatterIntegerRepresentation::Character: append(static_cast<char>(value)); return *this;
//   case FormatterIntegerRepresentation::None: break;
//   }

//   char buffer[32];
//   utoa(value, buffer, radix);
//   append(buffer);
//   return *this;
// }

StringBuilder &StringBuilder::append(
  const float value,
  FormatterFloatingRepresentation type) {
  char buffer[40];
  ftoa(value, buffer);
  append(buffer);
  return *this;
}

StringBuilder &StringBuilder::append(
  const double value,
  FormatterFloatingRepresentation type) {
  char buffer[40];
  dtoa(value, buffer);
  append(buffer);
  return *this;
}

}// namespace Core