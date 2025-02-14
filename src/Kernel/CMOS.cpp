#include "CMOS.hpp"
#include "IO.hpp"

namespace CMOS {

  u8 read(u8 index) {
    IO::write8(0x70, index);
    return IO::read8(0x71);
  }

  void write(u8 index, u8 data) {
    IO::write8(0x70, index);
    IO::write8(0x71, data);
  }

} // namespace CMOS
