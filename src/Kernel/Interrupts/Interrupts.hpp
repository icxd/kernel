//
// Created by icxd on 11/1/24.
//


#pragma once

#include <LibCore/Types.hpp>
#include <LibCore/Defines.hpp>
#include "IrqHandler.hpp"

union PACKED Descriptor {
  struct {
    u16 limit_low, base_low;
    u8 base_high;
    u8 type : 4;
    u8 descriptor_type : 1;
    u8 dpl : 2;
    u8 present : 1;
    u8 limit_high : 4;
    u8 : 1;
    u8 zero : 1;
    u8 operation_size : 1;
    u8 granularity : 1;
    u8 base_highest;
  };

  struct {
    u32 low, high;
  };

  enum class Type : u8 {
    Invalid = 0,
    AvailableTSS_16bit = 0x1,
    LDT = 0x2,
    BusyTSS_16bit = 0x3,
    CallGate_16bit = 0x4,
    TaskGate = 0x5,
    InterruptGate_16bit = 0x6,
    TrapGate_16bit = 0x7,
    AvailableTSS_32bit = 0x9,
    BusyTSS_32bit = 0xb,
    CallGate_32bit = 0xc,
    InterruptGate_32bit = 0xe,
    TrapGate_32bit = 0xf,
  };

  void set_base(u16 base) {
    base_low = base & 0xffff;
    base_high = (base >> 16) & 0xff;
    base_highest = (base >> 24) & 0xff;
  }

  void set_limit(u32 limit) {
    limit_low = limit & 0xffff;
    limit_high = (limit >> 16) & 0xff;
  }
};

namespace GDT {

void write_entry(u16 selector, Descriptor &descriptor);
void flush();
void init();

} // namespace GDT

namespace IDT {

void register_irq_handler(u8 irq, IRQHandler &handler);
void unregister_irq_handler(u8 irq, IRQHandler &handler);
void register_interrupt_handler(u8 vector, void (*handler)());
void init();
void flush();

} // namespace IDT

#define LSW(x) ((u16)((u32)(x) & 0xffff))
#define MSW(x) ((u16)((x) >> 16) & 0xffff)
#define LSB(x) ((u8)(x) & 0xff)
#define MSB(x) ((u8)((x) >> 8) & 0xff)

#define cli() asm volatile("cli" :: : "memory")
#define sti() asm volatile("sti" :: : "memory")

#define IRQ_VECTOR_BASE 0x50

struct PACKED RegisterDump {
  u16 ss, gs, fs, es, ds;
  u32 edi, esi, ebp, esp, ebx, edx, ecx, eax, eip;
  u16 cs, _cs_padding;
  u32 eflags, esp_if_cross_ring;
  u16 ss_if_cross_ring;
};

struct PACKED RegisterDumpWithExceptionCode {
  u16 ss, gs, fs, es, ds;
  u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
  u16 exception_code, _exception_code_padding;
  u32 eip;
  u16 cs, _cs_padding;
  u32 eflags, esp_if_cross_ring;
  u16 ss_if_cross_ring;
};

inline u32 cpu_flags() {
  u32 flags;
  asm volatile(
      "pushf\n"
      "pop %0\n"
      : "=rm"(flags)::"memory");
  return flags;
}

class InterruptDisabler {
public:
  InterruptDisabler() {
    m_flags = cpu_flags();
    cli();
  }

  ~InterruptDisabler() {
    if (m_flags & 0x200)
      sti();
  }

private:
  u32 m_flags;
};