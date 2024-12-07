//
// Created by icxd on 11/1/24.
//


#pragma once

#include "Types.hpp"
#include "String.hpp"

namespace Core {

// This is sort of like a parser combinator library, but not really.
class Parser {
public:
  explicit Parser(const char *string) : m_string(string), m_index(0) {}
  Parser(const char *string, usz index) : m_string(string), m_index(index) {}

  bool is_end() const { return m_index >= m_string.size(); }
  char peek() const { return m_string[m_index]; }
  char consume() {
    if (is_end())
      return '\0';
    return m_string[m_index++];
  }

  template <typename T> T consume(T (*parser)(char)) {
    T result = parser(peek());
    if (result != 0)
      consume();
    return result;
  }

  String consume_until(char delimiter) {
    StringBuilder result;
    while (!is_end() && peek() != delimiter)
      result += consume();
    return result.build();
  }

private:
  String m_string;
  usz m_index;
};

}
