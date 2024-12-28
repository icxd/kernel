//
// Created by icxd on 11/14/24.
//

#pragma once

#include "Defines.hpp"
#include "Types.hpp"

namespace Core {

  template <typename T>
  class Vector {
  public:
    Vector() : m_items(nullptr), m_size(0), m_capacity(0) {}
    ~Vector() { delete[] m_items; }

    void push(T item) {
      if (!m_items && m_capacity == 0) {
        m_capacity = 16;
        m_items = static_cast<T *>(new T[m_capacity * sizeof(T)]);
      }

      if (m_size + 1 > m_capacity) {
        usz new_capacity = m_capacity * 2;
        if (new_capacity < m_size + 1)
          new_capacity = m_size + 1;

        T *new_items = static_cast<T *>(new T[new_capacity * sizeof(T)]);
        memcpy(new_items, m_items, m_size);
        delete[] m_items;
        m_items = new_items;
        m_capacity = new_capacity;
      }

      m_items[m_size++] = item;
    }

    [[nodiscard]] T at(isz index) const {
      if (index < 0)
        // If the index was -1, it would become `m_size + -1`, which
        // due to the way math works, would become `m_size - 1`
        index = m_size + index;
      if (index < 0 || index >= m_size)
        PANIC("Vector index out of bounds");
      return m_items[index];
    }

    void reserve(const usz new_capacity) {
      if (new_capacity > m_capacity) {
        T *new_items = static_cast<T *>(new T[new_capacity * sizeof(T)]);
        memcpy(new_items, m_items, m_size);
        delete[] m_items;
        m_items = new_items;
        m_capacity = new_capacity;
      }
    }

    [[nodiscard]] T *data() const { return m_items; }
    [[nodiscard]] usz size() const { return m_size; }
    [[nodiscard]] usz capacity() const { return m_capacity; }

    template <typename F>
    void for_each(F f) {
      for (usz i = 0; i < m_size; ++i)
        f(m_items[i]);
    }

    template <typename F>
    void enumerate(F f) {
      for (usz i = 0; i < m_size; ++i)
        f(i, m_items[i]);
    }

    template <typename F>
    Vector map(F f) {
      Vector result;
      result.reserve(m_size);
      for_each([&](auto item) { result.push(f(item)); });
      return result;
    }

    template <typename F>
    Vector filter(F f) {
      Vector result;
      result.reserve(m_size);
      for_each([&](auto item) {
        if (f(item))
          result.push(item);
      });
      return result;
    }

    template <typename U, typename F>
    Vector<U> transform(F f) {
      Vector<U> result;
      result.reserve(m_size);
      for_each([&](auto item) { result.push(f(item)); });
      return result;
    }

  private:
    T *m_items;
    usz m_size, m_capacity;
  };

} // namespace Core

using Core::Vector;