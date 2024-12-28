//
// Created by icxd on 11/4/24.
//

#pragma once

#include "Types.hpp"
#include <LibCpp/cstddef.hpp>

namespace Core {

  template <typename T>
  class OwnPtr {
  public:
    OwnPtr() = default;
    explicit OwnPtr(T *ptr) : m_ptr(ptr) {}
    OwnPtr(OwnPtr &&other) noexcept : m_ptr(other.leak_ptr()) {}
    template <typename U>
    explicit OwnPtr(OwnPtr<U> &&other)
        : m_ptr(static_cast<T *>(other.leak_ptr())) {}
    explicit OwnPtr(std::nullptr_t) {}
    ~OwnPtr() { clear(); }

    OwnPtr &operator=(OwnPtr &&other) noexcept {
      if (this != &other) {
        delete m_ptr;
        m_ptr = other.leak_ptr();
      }

      return *this;
    }

    template <typename U>
    OwnPtr &operator=(OwnPtr<U> &&other) {
      if (this != static_cast<void *>(&other)) {
        delete m_ptr;
        m_ptr = other.leak_ptr();
      }

      return *this;
    }

    OwnPtr &operator=(T *ptr) {
      if (m_ptr != ptr)
        delete m_ptr;
      m_ptr = ptr;
      return *this;
    }

    OwnPtr &operator=(std::nullptr_t) {
      clear();
      return *this;
    }

    void clear() {
      delete m_ptr;
      m_ptr = nullptr;
    }

    bool operator!() const { return !m_ptr; }

    T *leak_ptr() {
      T *leaked_ptr = m_ptr;
      m_ptr = nullptr;
      return leaked_ptr;
    }

    [[nodiscard]] T *ptr() { return m_ptr; }
    [[nodiscard]] const T *ptr() const { return m_ptr; }

    T *operator->() { return m_ptr; }
    const T *operator->() const { return m_ptr; }

    T &operator*() { return *m_ptr; }
    const T &operator*() const { return *m_ptr; }

    explicit operator const T *() const { return m_ptr; }
    explicit operator T *() { return m_ptr; }

    explicit operator bool() const { return m_ptr; }

  private:
    T *m_ptr = nullptr;
  };

  template <typename T, typename... Args>
  OwnPtr<T> make(Args &&...args) {
    return OwnPtr<T>(new T(forward<Args>(args)...));
  }

} // namespace Core

using Core::make;
using Core::OwnPtr;