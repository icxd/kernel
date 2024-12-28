//
// Created by icxd on 11/11/24.
//

#pragma once

#include <LibCore/Error.hpp>
#include <LibCore/Types.hpp>

namespace Serial {

  ErrorOr<int> init();

  int did_receive();
  u8 read();

  int is_transmit_empty();
  void write(u8 data);
  void write_buffer(const u8 *data, usz size);

} // namespace Serial