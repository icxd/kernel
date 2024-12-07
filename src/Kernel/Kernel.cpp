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
#include <LibCore/OwnPtr.hpp>
#include <LibCore/Vector.hpp>

extern "C" void kmain(u32 multiboot_magic, usz multiboot_ptr) {
  TRY_OR_PANIC(Serial::init());
  kmalloc_init();

  debugln("multiboot_magic: 0x{:x}", multiboot_magic);
  ASSERT(multiboot_magic == MULTIBOOT_BOOTLOADER_MAGIC);

  auto *mbi = (multiboot_info_t *) multiboot_ptr;
  ASSERT(mbi->flags & MULTIBOOT_INFO_MEMORY);
  ASSERT(mbi->flags & MULTIBOOT_INFO_BOOTDEV);
  ASSERT(mbi->flags & MULTIBOOT_INFO_CMDLINE);
  ASSERT(mbi->flags & MULTIBOOT_INFO_MEM_MAP);

  {
    multiboot_memory_map_t *mmap;

    debugln("mmap_addr = 0x{:x}, mmap_length = 0x{:x}", mbi->mmap_addr, mbi->mmap_length);
    for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
         (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
         mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size)))
      debugln(" size = 0x{:x}, base_addr = 0x{:x}{:x}, length = 0x{:x}{:x}, type = 0x{:x}",
              mmap->size, (mmap->addr >> 32), (mmap->addr & 0xffffffff), (mmap->len >> 32),
              (mmap->len & 0xffffffff), mmap->type);
  }

  PIC::init();
  GDT::init();
  IDT::init();

  VGA vga;
  vga.clear();
  vga.puts("Hello, world!\n");

  vga.puts(StringBuilder()
               .append("Hello, ")
               .append(String("world"))
               .append("!\n")
               .build()
               .characters());

  OwnPtr<String> s = make<String>("Hello, world!\n");
  vga.puts(*s);
}