#pragma once

#include "Defines.hpp"
#include "Types.hpp"

namespace Core {

#define CORE_MAKE_ENUM(NAME, BASE_TYPE, ...)                                   \
  namespace NAME {                                                             \
    using UnderlyingType = BASE_TYPE;                                          \
    inline static constexpr BASE_TYPE __VA_ARGS__;                             \
  }

} // namespace Core
