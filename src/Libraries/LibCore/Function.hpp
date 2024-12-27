//
// Created by icxd on 11/14/24.
//

#pragma once

#include "Types.hpp"

namespace Core {

template <typename R, typename... Args>
class Function {
  using Callable = R (*)(Args...);

public:
  Function(const Callable callable) : m_callable(callable) {}

  R operator()(Args... args) const { return m_callable(forward<Args>(args)...); }

private:
  Callable m_callable;
};

} // namespace Core
