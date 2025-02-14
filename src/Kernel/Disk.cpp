#include "Disk.hpp"
#include "IO.hpp"
#include "Interrupts/Interrupts.hpp"
#include "LibCore/ByteBuffer.hpp"
#include "PIC.hpp"
#include "Process.hpp"
#include "kprintf.hpp"

extern "C" void handle_interrupt();

namespace Disk {

  IDEDrive drive[4];
  static volatile bool interrupted;

#define IRQ_FIXED_DISK 14

  extern "C" void ide_isr();
  asm(".globl ide_isr \n"
      "ide_isr: \n"
      "    pusha\n"
      "    pushw %ds\n"
      "    pushw %es\n"
      "    pushw %ss\n"
      "    pushw %ss\n"
      "    popw %ds\n"
      "    popw %es\n"
      "    call handle_interrupt\n"
      "    popw %es\n"
      "    popw %ds\n"
      "    popa\n"
      "    iret\n");

  static void enable_irq() { PIC::enable(IRQ_FIXED_DISK); }
  static void disable_irq() { PIC::disable(IRQ_FIXED_DISK); }

  static bool wait_for_interrupt() {
    debugln("disk: waiting for interrupt...");

    while (!interrupted)
      yield();

    debugln("disk: got interrupt!");
    return true;
  }

  void interrupt() {
    IRQHandlerScope scope(IRQ_FIXED_DISK);
    u8 status = IO::read8(0x1f7);
    debugln("disk: interrupt: DRQ={} BUSY={} DRDY={}", (status & DRQ) != 0,
            (status & BUSY) != 0, (status & DRDY) != 0);

    interrupted = true;
  }

  void initialize() {
    disable_irq();
    interrupted = false;
    IDT::register_interrupt_handler(IRQ_VECTOR_BASE + IRQ_FIXED_DISK, ide_isr);

    while (IO::read8(IDE0_STATUS) & BUSY)
      ;

    IO::write8(0x1F6, 0xA0); // 0xB0 for 2nd device
    IO::write8(IDE0_COMMAND, IDENTIFY_DRIVE);

    enable_irq();
    wait_for_interrupt();

    Core::ByteBuffer wbuf = Core::ByteBuffer::create_uninitialized(512);
    Core::ByteBuffer bbuf = Core::ByteBuffer::create_uninitialized(512);
    u8 *b = bbuf.pointer();
    u16 *w = (u16 *)wbuf.pointer();
    const u16 *wbufbase = (u16 *)wbuf.pointer();

    for (u32 i = 0; i < 256; ++i) {
      u32 data = IO::read16(IDE0_DATA);
      *(w++) = data;
      *(b++) = MSB(data);
      *(b++) = LSB(data);
    }

    // "Unpad" the device name string.
    for (u32 i = 93; i > 54 && bbuf[i] == ' '; --i)
      bbuf[i] = 0;

    drive[0].cylinders = wbufbase[1];
    drive[0].heads = wbufbase[3];
    drive[0].sectors_per_track = wbufbase[6];

    debugln("ide0: Master=\"{}\", C/H/Spt={}/{}/{}\n", bbuf.pointer() + 54,
            drive[0].cylinders, drive[0].heads, drive[0].sectors_per_track);
  }

} // namespace Disk
