//
// Created by icxd on 11/11/24.
//

#include "PIC.hpp"
#include "IO.hpp"
#include "Interrupts/Interrupts.hpp"
#include "kprintf.hpp"

inline static constexpr u16 SLAVE_INDEX = 2;

inline static constexpr u16 PIC0_CTL = 0x20;
inline static constexpr u16 PIC0_CMD = 0x21;
inline static constexpr u16 PIC1_CTL = 0xA0;
inline static constexpr u16 PIC1_CMD = 0xA1;

namespace PIC {

  void enable(u8 irq) {
    u8 imr;
    if (irq & 8) {
      imr = IO::read8(PIC1_CMD);
      imr &= ~(1 << (irq - 8));
      IO::write8(PIC1_CMD, imr);
    } else {
      imr = IO::read8(PIC0_CMD);
      imr &= ~(1 << irq);
      IO::write8(PIC0_CMD, imr);
    }
  }

  void disable(u8 irq) {
    u8 imr;
    if (irq & 8) {
      imr = IO::read8(PIC1_CMD);
      imr |= (1 << (irq - 8));
      IO::write8(PIC1_CMD, imr);
    } else {
      imr = IO::read8(PIC0_CMD);
      imr |= (1 << irq);
      IO::write8(PIC0_CMD, imr);
    }
  }

  void eoi(u8 irq) {
    if (irq & 8)
      IO::write8(PIC1_CTL, 0x20);
    IO::write8(PIC0_CTL, 0x20);
  }

  void init() {
    /* ICW1 (edge triggered mode, cascading controllers, expect ICW4) */
    IO::write8(PIC0_CTL, 0x11);
    IO::write8(PIC1_CTL, 0x11);

    /* ICW2 (upper 5 bits specify ISR indices, lower 3 idunno) */
    IO::write8(PIC0_CMD, IRQ_VECTOR_BASE);
    IO::write8(PIC1_CMD, IRQ_VECTOR_BASE + 0x08);

    /* ICW3 (configure master/slave relationship) */
    IO::write8(PIC0_CMD, 1 << SLAVE_INDEX);
    IO::write8(PIC1_CMD, SLAVE_INDEX);

    /* ICW4 (set x86 mode) */
    IO::write8(PIC0_CMD, 0x01);
    IO::write8(PIC1_CMD, 0x01);

    // Mask -- start out with all IRQs disabled.
    IO::write8(PIC0_CMD, 0xff);
    IO::write8(PIC1_CMD, 0xff);

    // ...except IRQ2, since that's needed for the master to let through slave
    // interrupts.
    enable(2);

    okln("PIC(i8259): cascading mode, vectors 0b{:b}-0b{:b}", IRQ_VECTOR_BASE,
         IRQ_VECTOR_BASE + 0x08);
  }

  u16 get_isr() {
    IO::write8(PIC0_CTL, 0x0b);
    IO::write8(PIC1_CTL, 0x0b);
    return (IO::read8(PIC0_CTL) << 8) | IO::read8(PIC1_CTL);
  }

} // namespace PIC
