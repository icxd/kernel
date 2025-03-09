#pragma once

#include "LibCore/Trait.hpp"
#include "Types.hpp"

namespace Core {

  inline unsigned int_hash(u32 key) {
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
  }

  inline unsigned pair_int_hash(u32 key1, u32 key2) {
    return int_hash((int_hash(key1) * 209) ^ (int_hash(key2 * 413)));
  }

  template <typename T>
  struct Hash {};

  template <typename T>
  concept Hashable = requires(T t) {
    { Hash<T>::hash(t) } -> IsSame<unsigned>;
  };

  template <>
  struct Hash<int> {
    static unsigned hash(int i) { return int_hash(i); }
  };

  template <>
  struct Hash<unsigned> {
    static unsigned hash(unsigned i) { return int_hash(i); }
  };

  template <typename T>
  struct Hash<T *> {
    static unsigned hash(T *p) { return int_hash((u32)p); }
  };

  template <typename T>
  struct Hash<const T *> {
    static unsigned hash(const T *p) { return int_hash((u32)p); }
  };

} // namespace Core
