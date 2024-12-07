//
// Created by icxd on 11/11/24.
//

#include "Serial.hpp"

namespace Serial {

constexpr u16 COM1 = 0x3F8;

ErrorOr<int> init() {
  IO::write8(COM1 + 1, 0x00);
  IO::write8(COM1 + 3, 0x80);
  IO::write8(COM1 + 0, 0x03);
  IO::write8(COM1 + 1, 0x00);
  IO::write8(COM1 + 3, 0x03);
  IO::write8(COM1 + 2, 0xC7);
  IO::write8(COM1 + 4, 0x0B);
  IO::write8(COM1 + 4, 0x1E);
  IO::write8(COM1 + 0, 0xAE);

  if (IO::read8(COM1 + 0) != 0xAE)
    return Error(1);

  IO::write8(COM1 + 4, 0x0F);
  return {0};
}

int did_receive() {
  return IO::read8(COM1 + 5) & 1;
}

u8 read() {
  while (did_receive() == 0);
  return IO::read8(COM1);
}

int is_transmit_empty() {
  return IO::read8(COM1 + 5) & 0x20;
}

void write(u8 data) {
  while (is_transmit_empty() == 0);
  IO::write8(COM1, data);
}

void write_buffer(const u8 *data, usz size) {
  for (usz i = 0; i < size; i++)
    write(data[i]);
}

} // namespace Serial
