//
// Created by icxd on 11/14/24.
//

#pragma once

#include "Defines.hpp"
#include "LibCore/Formatting.hpp"
#include "LibCore/String.hpp"
#include "Types.hpp"

namespace Core {

  template <typename T> class Vector;

  template <typename T> class VectorImpl {
  public:
    ~VectorImpl() {}
    static VectorImpl *create(usz capacity) {
      usz size = sizeof(VectorImpl) + sizeof(T) * capacity;
      void *slot = kmalloc(size);
      new (slot) VectorImpl(capacity);
      return (VectorImpl *)slot;
    }

    usz size() const { return m_size; }
    usz capacity() const { return m_capacity; }

    T &at(usz i) { return *slot(i); }
    const T &at(usz i) const { return *slot(i); }

    void remove(usz index) {
      ASSERT(index < m_size);
      at(index).~T();
      for (usz i = index + 1; i < m_size; ++i) {
        new (slot(i - 1)) T(move(at(i)));
        at(i).~T();
      }

      --m_size;
    }

  private:
    friend class Vector<T>;

    VectorImpl(usz capacity) : m_capacity(capacity) {}

    T *tail() { return reinterpret_cast<T *>(this + 1); }
    T *slot(usz i) { return &tail()[i]; }

    const T *tail() const { return reinterpret_cast<const T *>(this + 1); }
    const T *slot(usz i) const { return &tail()[i]; }

    usz m_size{0};
    usz m_capacity;
  };

  template <typename T> class Vector {
  public:
    Vector() {}
    ~Vector() { clear(); }

    Vector(Vector &&other) : m_impl(other.m_impl) { other.m_impl = nullptr; }

    Vector &operator=(Vector &&other) {
      if (this != &other) {
        m_impl = other.m_impl;
        other.m_impl = nullptr;
      }
      return *this;
    }

    void clear() {
      for (usz i = 0; i < size(); ++i) {
        at(i).~T();
      }
      kfree(m_impl);
      m_impl = nullptr;
    }

    bool is_empty() const { return size() == 0; }
    usz size() const { return m_impl ? m_impl->size() : 0; }
    usz capacity() const { return m_impl ? m_impl->capacity() : 0; }

    T *data() { return m_impl ? &at(0) : nullptr; }
    const T *data() const { return m_impl ? &at(0) : nullptr; }

    const T &at(usz i) const { return m_impl->at(i); }
    T &at(usz i) { return m_impl->at(i); }

    const T &operator[](usz i) const { return at(i); }
    T &operator[](usz i) { return at(i); }

    const T &first() const { return at(0); }
    T &first() { return at(0); }

    const T &last() const { return at(size() - 1); }
    T &last() { return at(size() - 1); }

    T take_last() {
      ASSERT(!is_empty());
      T value = move(last());
      last().~T();
      --m_impl->m_size;
      return value;
    }

    void remove(usz index) { m_impl->remove(index); }

    void append(Vector<T> &&other) {
      Vector<T> tmp = move(other);
      ensure_capacity(size() + tmp.size());
      for (auto &&v : tmp) {
        unchecked_push(move(v));
      }
    }

    void unchecked_push(T &&value) {
      new (m_impl->slot(m_impl->m_size)) T(move(value));
      ++m_impl->m_size;
    }

    void push(T &&value) {
      ensure_capacity(size() + 1);
      new (m_impl->slot(m_impl->m_size)) T(move(value));
      ++m_impl->m_size;
    }

    void push(const T &value) {
      ensure_capacity(size() + 1);
      new (m_impl->slot(m_impl->m_size)) T(value);
      ++m_impl->m_size;
    }

    void ensure_capacity(usz neededCapacity) {
      if (capacity() >= neededCapacity)
        return;
      usz newCapacity = padded_capacity(neededCapacity);
      auto newImpl = VectorImpl<T>::create(newCapacity);
      if (m_impl) {
        newImpl->m_size = m_impl->m_size;
        for (usz i = 0; i < size(); ++i) {
          new (newImpl->slot(i)) T(move(m_impl->at(i)));
          m_impl->at(i).~T();
        }
        kfree(m_impl);
      }
      m_impl = newImpl;
    }

    class Iterator {
    public:
      bool operator!=(const Iterator &other) {
        return m_index != other.m_index;
      }
      Iterator &operator++() {
        ++m_index;
        return *this;
      }
      T &operator*() { return m_vector[m_index]; }

    private:
      friend class Vector;
      Iterator(Vector &vector, usz index) : m_vector(vector), m_index(index) {}
      Vector &m_vector;
      usz m_index{0};
    };

    Iterator begin() { return Iterator(*this, 0); }
    Iterator end() { return Iterator(*this, size()); }

    class ConstIterator {
    public:
      bool operator!=(const ConstIterator &other) {
        return m_index != other.m_index;
      }
      ConstIterator &operator++() {
        ++m_index;
        return *this;
      }
      const T &operator*() const { return m_vector[m_index]; }

    private:
      friend class Vector;
      ConstIterator(const Vector &vector, const usz index)
          : m_vector(vector), m_index(index) {}
      const Vector &m_vector;
      usz m_index{0};
    };

    ConstIterator begin() const { return ConstIterator(*this, 0); }
    ConstIterator end() const { return ConstIterator(*this, size()); }

  private:
    static usz padded_capacity(usz capacity) {
      return max(usz(4), capacity + (capacity / 4) + 4);
    }

    VectorImpl<T> *m_impl{nullptr};
  };

  template <typename T> struct Formatter<Vector<T>> {
    static String format(const Vector<T> &vec) {
      StringBuilder builder;
      builder.append('[');
      for (usz i = 0; i < vec.size(); i++) {
        const auto &item = vec.at(i);
        builder.append(item);
        if (i + 1 >= vec.size())
          builder.append(", ");
      }
      builder.append(']');
      return builder.build();
    }
  };

} // namespace Core

using Core::Vector;
