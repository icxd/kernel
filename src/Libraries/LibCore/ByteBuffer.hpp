#pragma once

#include "Buffer.hpp"
#include "Defines.hpp"
#include "RetainPtr.hpp"
#include "Types.hpp"
#include <LibCpp/cstddef.hpp>

namespace Core {

  class ByteBuffer {
  public:
    ByteBuffer() {}
    ByteBuffer(std::nullptr_t) {}
    ByteBuffer(const ByteBuffer &other) : m_impl(other.m_impl.copy_ref()) {}
    ByteBuffer(ByteBuffer &&other) : m_impl(move(other.m_impl)) {}
    ByteBuffer &operator=(ByteBuffer &&other) {
      if (this != &other)
        m_impl = move(other.m_impl);
      return *this;
    }
    ByteBuffer &operator=(const ByteBuffer &other) {
      m_impl = other.m_impl.copy_ref();
      return *this;
    }

    static ByteBuffer create_empty() {
      return ByteBuffer(Buffer<u8>::create_uninitialized(0));
    }
    static ByteBuffer create_uninitialized(usz size) {
      return ByteBuffer(Buffer<u8>::create_uninitialized(size));
    }
    static ByteBuffer copy(const u8 *data, usz size) {
      return ByteBuffer(Buffer<u8>::copy(data, size));
    }
    static ByteBuffer wrap(u8 *data, usz size) {
      return ByteBuffer(Buffer<u8>::wrap(data, size));
    }
    static ByteBuffer adopt(u8 *data, usz size) {
      return ByteBuffer(Buffer<u8>::adopt(data, size));
    }

    ~ByteBuffer() { clear(); }
    void clear() { m_impl = nullptr; }

    operator bool() const { return !is_null(); }
    bool operator!() const { return is_null(); }
    bool is_null() const { return m_impl == nullptr; }

    u8 &operator[](usz i) {
      ASSERT(m_impl);
      return (*m_impl)[i];
    }
    u8 operator[](usz i) const {
      ASSERT(m_impl);
      return (*m_impl)[i];
    }
    bool is_empty() const { return !m_impl || m_impl->is_empty(); }
    usz size() const { return m_impl ? m_impl->size() : 0; }

    u8 *pointer() { return m_impl ? m_impl->pointer() : nullptr; }
    const u8 *pointer() const { return m_impl ? m_impl->pointer() : nullptr; }

    u8 *offset_ptr(usz offset) {
      return m_impl ? m_impl->offset_ptr(offset) : nullptr;
    }
    const u8 *offset_ptr(usz offset) const {
      return m_impl ? m_impl->offset_ptr(offset) : nullptr;
    }

    const void *end_ptr() const { return m_impl ? m_impl->end_ptr() : nullptr; }

    // NOTE: trim() does not reallocate.
    void trim(usz size) {
      if (m_impl)
        m_impl->trim(size);
    }

    ByteBuffer slice(usz offset, usz size) const {
      if (is_null())
        return {};
      if (offset >= this->size())
        return {};
      if (offset + size >= this->size())
        size = this->size() - offset;
      return copy(offset_ptr(offset), size);
    }

  private:
    explicit ByteBuffer(RetainPtr<Buffer<u8>> &&impl) : m_impl(move(impl)) {}

    Core::RetainPtr<Buffer<u8>> m_impl;
  };

} // namespace Core