#pragma once

#include <LibCpp/cstddef.hpp>

namespace Core {

  template <typename T> inline void retain_if_not_null(T *ptr) {
    if (ptr)
      ptr->retain();
  }

  template <typename T> inline void release_if_not_null(T *ptr) {
    if (ptr)
      ptr->release();
  }

  template <typename T> class RetainPtr {
  public:
    enum AdoptTag { Adopt };

    RetainPtr() {}
    explicit RetainPtr(const T *ptr) : m_ptr(const_cast<T *>(ptr)) {
      retain_if_not_null(m_ptr);
    }
    explicit RetainPtr(T *ptr) : m_ptr(ptr) { retain_if_not_null(m_ptr); }
    explicit RetainPtr(T &object) : m_ptr(&object) { m_ptr->retain(); }
    RetainPtr(AdoptTag, T &object) : m_ptr(&object) {}
    RetainPtr(RetainPtr &&other) noexcept : m_ptr(other.leak_ref()) {}
    template <typename U>
    explicit RetainPtr(RetainPtr<U> &&other)
        : m_ptr(static_cast<T *>(other.leak_ref())) {}
    ~RetainPtr() {
      clear();
#ifdef SANITIZE_PTRS
      if constexpr (sizeof(T *) == 8)
        m_ptr = (T *)(0xe0e0e0e0e0e0e0e0);
      else
        m_ptr = (T *)(0xe0e0e0e0);
#endif
    }
    RetainPtr(std::nullptr_t) {}

    RetainPtr &operator=(RetainPtr &&other) noexcept {
      if (this != &other) {
        release_if_not_null(m_ptr);
        m_ptr = other.leak_ref();
      }
      return *this;
    }

    template <typename U> RetainPtr &operator=(RetainPtr<U> &&other) {
      if (this != static_cast<void *>(&other)) {
        release_if_not_null(m_ptr);
        m_ptr = other.leak_ref();
      }
      return *this;
    }

    RetainPtr &operator=(T *ptr) {
      if (m_ptr != ptr)
        release_if_not_null(m_ptr);
      m_ptr = ptr;
      retain_if_not_null(m_ptr);
      return *this;
    }

    RetainPtr &operator=(T &object) {
      if (m_ptr != &object)
        release_if_not_null(m_ptr);
      m_ptr = &object;
      retain_if_not_null(m_ptr);
      return *this;
    }

    RetainPtr &operator=(std::nullptr_t) {
      clear();
      return *this;
    }

    RetainPtr copy_ref() const { return RetainPtr(m_ptr); }
    void clear() {
      release_if_not_null(m_ptr);
      m_ptr = nullptr;
    }

    bool operator!() const { return !m_ptr; }

    typedef T *RetainPtr::*UnspecifiedBoolType;
    operator UnspecifiedBoolType() const {
      return m_ptr ? &RetainPtr::m_ptr : nullptr;
    }

    T *leak_ref() {
      T *leaked_ptr = m_ptr;
      m_ptr = nullptr;
      return leaked_ptr;
    }

    T *ptr() { return m_ptr; }
    const T *ptr() const { return m_ptr; }

    T *operator->() { return m_ptr; }
    const T *operator->() const { return m_ptr; }

    T &operator*() { return *m_ptr; }
    const T &operator*() const { return *m_ptr; }

    operator bool() { return !!m_ptr; }

  private:
    T *m_ptr = nullptr;
  };

  template <typename T> inline RetainPtr<T> adopt(T &object) {
    return RetainPtr<T>(RetainPtr<T>::Adopt, object);
  }

} // namespace Core