#pragma once

#include "LibCore/Defines.hpp"

namespace Core {

  template <typename T>
  class Retainable {
  public:
    void retain() {
      ASSERT(m_retain_count);
      ++m_retain_count;
    }

    void release() {
      ASSERT(m_retain_count);
      if (!--m_retain_count)
        delete static_cast<const T *>(this);
    }

    int retain_count() const { return m_retain_count; }

  protected:
    Retainable() {};
    ~Retainable() { ASSERT(!m_retain_count); }

  private:
    int m_retain_count = 1;
  };

} // namespace Core