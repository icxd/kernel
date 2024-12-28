//
// Created by icxd on 11/6/24.
//

#pragma once

#include "Defines.hpp"
#include "Types.hpp"

namespace Core {

  template <typename T, usz Size>
  class Array final {
  public:
    Array() = default;
    ~Array() = default;

    void set(usz index, T value) {
      if (index >= Size)
        PANIC("Array index out of bounds");
      m_items[index] = value;
    }

    void push(T value) {
      if (m_size == Size)
        PANIC("Array overflow");
      m_items[m_size++] = value;
    }

    T get(usz index) const {
      if (index >= Size)
        PANIC("Array index out of bounds");
      return m_items[index];
    }

    T &pop() {
      if (m_size == 0)
        PANIC("Array underflow");
      return m_items[--m_size];
    }

    [[nodiscard]] bool is_empty() const { return m_size == 0; }
    [[nodiscard]] usz size() const { return m_size; }

    T &operator[](const usz index) { return get(index); }
    const T &operator[](const usz index) const { return get(index); }

  private:
    T m_items[Size] = {0};
    usz m_size{0};
  };

} // namespace Core

using Core::Array;
