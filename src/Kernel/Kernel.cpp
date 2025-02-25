//
// Created by icxd on 10/28/24.
//

#include "CMOS.hpp"
#include "Common.hpp"
#include "Disk.hpp"
#include "Drivers/Serial.hpp"
#include "Drivers/VGA.hpp"
#include "Interrupts/Interrupts.hpp"
#include "LibCore/String.hpp"
#include "MemoryManager.hpp"
#include "Multiboot.hpp"
#include "PIC.hpp"
#include "PIT.hpp"
#include "Process.hpp"
#include "RTC.hpp"
#include "kmalloc.hpp"
#include "kprintf.hpp"
#include <LibCore/ByteBuffer.hpp>
#include <LibCore/Defines.hpp>
#include <LibCore/DistinctNumeric.hpp>
#include <LibCore/Enum.hpp>
#include <LibCore/OwnPtr.hpp>
#include <LibCore/Types.hpp>
#include <LibCore/Vector.hpp>

CORE_MAKE_DISTINCT_NUMERIC_TYPE(Test, u32);
CORE_MAKE_ENUM(Color, Test, Red = Test(0xff0000), Green = Test(0x00ff00),
               Blue = Test(0x0000ff));

System system;

static void undertaker_main() NORETURN;
static void undertaker_main() {
  for (;;) {
    Process::do_house_keeping();
    sleep(300);
  }
}

static void init_stage2() NORETURN;
static void init_stage2() {
  okln("init stage2...");

  Disk::initialize();

  VGA vga;
  vga.clear();
  vga.puts("Hello, world!\n");

  defer { vga.puts("deferred\n"); };

  vga.puts(StringBuilder()
               .append("Hello, ")
               .append(String("world"))
               .append("!\n")
               .build()
               .characters());

  OwnPtr<String> s = make<String>("Hello, world!\n");
  vga.puts(*s);

  for (;;) {
    sleep(3600 * TICKS_PER_SECOND);
    asm("hlt");
  }
}

extern "C" void kmain(const u32 multiboot_magic, const usz multiboot_ptr) {
  cli();

  TRY_OR_PANIC(Serial::init());
  kmalloc_init();

  okln("multiboot_magic: 0x{:x}", multiboot_magic);
  ASSERT(multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC);

  const auto *mbi = reinterpret_cast<multiboot_info_t *>(multiboot_ptr);
  ASSERT(mbi->flags & MULTIBOOT_INFO_MEMORY);
  ASSERT(mbi->flags & MULTIBOOT_INFO_BOOTDEV);
  ASSERT(mbi->flags & MULTIBOOT_INFO_CMDLINE);
  ASSERT(mbi->flags & MULTIBOOT_INFO_MEM_MAP);

  {

    okln("mmap_addr = 0x{:x}, mmap_length = 0x{:x}", mbi->mmap_addr,
         mbi->mmap_length);
    for (auto *mmap =
             reinterpret_cast<multiboot_memory_map_t *>(mbi->mmap_addr);
         reinterpret_cast<unsigned long>(mmap) <
         mbi->mmap_addr + mbi->mmap_length;
         mmap = reinterpret_cast<multiboot_memory_map_t *>(
             reinterpret_cast<unsigned long>(mmap) + mmap->size +
             sizeof(mmap->size)))
      okln(" size = 0x{:x}, base_addr = 0x{:x}{:x}, length = 0x{:x}{:x}, "
           "type = 0x{:x}",
           mmap->size, mmap->addr >> 32, mmap->addr & 0xffffffff,
           mmap->len >> 32, mmap->len & 0xffffffff, mmap->type);
  }

  RTC::initialize();
  PIC::init();
  GDT::init();
  IDT::init();

  MemoryManager::initialize();

  PIT::initialize();

  memset(&system, 0, sizeof(system));

  u32 base_memory = (CMOS::read(0x16) << 8) | CMOS::read(0x15);
  u32 ext_memory = (CMOS::read(0x18) << 8) | CMOS::read(0x17);

  okln("0x{:<8x} kB base memory", base_memory);
  okln("0x{:>8x} kB extended memory", ext_memory);

  Process::initialize();
  Process::create_kernel_process(undertaker_main, String("undertaker"));
  Process::create_kernel_process(init_stage2, String("init"));

  schedule_new_process();

  sti();

  for (;;)
    asm("hlt");
}
