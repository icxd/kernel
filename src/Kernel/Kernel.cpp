//
// Created by icxd on 10/28/24.
//

#include "Common.hpp"
#include "Drivers/Serial.hpp"
#include "Drivers/VGA.hpp"
#include "Interrupts/Interrupts.hpp"
#include "MemoryManager.hpp"
#include "Multiboot.hpp"
#include "PIC.hpp"
#include "Process.hpp"
#include "kmalloc.hpp"
#include "kprintf.hpp"
#include "symbol.h"
#include <LibCore/ByteBuffer.hpp>
#include <LibCore/Defines.hpp>
#include <LibCore/OwnPtr.hpp>
#include <LibCore/Types.hpp>
#include <LibCore/Vector.hpp>

static void init_stage2() NORETURN;
static void init_stage2() {
  okln("init stage2...");

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

  hcf();
}

extern "C" void kmain(const u32 multiboot_magic, const usz multiboot_ptr) {
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

  debugln("logger test");
  okln("logger test");
  warnln("logger test");
  errorln("logger test");

  PIC::init();
  GDT::init();
  IDT::init();

  MemoryManager::initialize();

  Process::initilize();
  Process::create_kernel_process(init_stage2, String("init"));

  hcf();
}
