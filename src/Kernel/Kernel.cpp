//
// Created by icxd on 10/28/24.
//

#include "Common.hpp"
#include "Drivers/VGA.hpp"
#include "Drivers/Serial.hpp"
#include "Interrupts/Interrupts.hpp"
#include "PIC.hpp"
#include "kmalloc.hpp"
#include "kprintf.hpp"
#include "Multiboot.hpp"
#include <LibCore/Types.hpp>
#include <LibCore/Defines.hpp>
#include <LibCore/OwnPtr.hpp>
#include <LibCore/Vector.hpp>

extern "C" void kmain(const u32 multiboot_magic, const usz multiboot_ptr) {
  TRY_OR_PANIC(Serial::init());
  kmalloc_init();

  debugln("multiboot_magic: 0x{:x}", multiboot_magic);
  ASSERT(multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC);

  const auto *mbi = reinterpret_cast<multiboot_info_t *>(multiboot_ptr);
  ASSERT(mbi->flags & MULTIBOOT_INFO_MEMORY);
  ASSERT(mbi->flags & MULTIBOOT_INFO_BOOTDEV);
  ASSERT(mbi->flags & MULTIBOOT_INFO_CMDLINE);
  ASSERT(mbi->flags & MULTIBOOT_INFO_MEM_MAP);

  {

    debugln("mmap_addr = 0x{:x}, mmap_length = 0x{:x}", mbi->mmap_addr, mbi->mmap_length);
    for (auto *mmap = reinterpret_cast<multiboot_memory_map_t *>(mbi->mmap_addr);
         reinterpret_cast<unsigned long>(mmap) < mbi->mmap_addr + mbi->mmap_length;
         mmap = reinterpret_cast<multiboot_memory_map_t *>(reinterpret_cast<unsigned long>(mmap) + mmap->size + sizeof(mmap->size)))
      debugln(" size = 0x{:x}, base_addr = 0x{:x}{:x}, length = 0x{:x}{:x}, type = 0x{:x}",
              mmap->size, mmap->addr >> 32, mmap->addr & 0xffffffff, mmap->len >> 32,
              mmap->len & 0xffffffff, mmap->type);
  }

  PIC::init();
  GDT::init();
  IDT::init();

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
}