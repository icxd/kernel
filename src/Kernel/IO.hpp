//
// Created by icxd on 11/11/24.
//

#pragma once

#include <LibCore/Types.hpp>

namespace IO {

  u8 read8(u16 port);
  void write8(u16 port, u8 value);

  u16 read16(u16 port);
  void write16(u16 port, u16 value);

  u32 read32(u16 port);
  void write32(u16 port, u32 value);

} // namespace IO
