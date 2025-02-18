//
// Created by icxd on 11/1/24.
//

#pragma once

#include "../Common.hpp"
#include "IrqHandler.hpp"
#include <LibCore/Defines.hpp>
#include <LibCore/Types.hpp>

union PACKED Descriptor {
  struct {
    u16 limit_low, base_low;
    u8 base_high;
    u8 type : 4;
    u8 descriptor_type : 1;
    u8 dpl : 2;
    u8 present : 1;
    u8 limit_high : 4;
    u8 avl : 1;
    u8 zero : 1;
    u8 operation_size : 1;
    u8 granularity : 1;
    u8 base_highest : 4;
  };

  struct {
    u32 low, high;
  };

  enum Type : u8 {
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

  void set_base(u32 base) {
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
  u16 allocate_entry();
  Descriptor &get_entry(u16 selector);

} // namespace GDT

namespace IDT {

  void register_irq_handler(u8 irq, IRQHandler &handler);
  void unregister_irq_handler(u8 irq, IRQHandler &handler);
  void register_interrupt_handler(u8 vector, void (*handler)());
  void init();
  void flush();

} // namespace IDT

void load_process_register(u16 selector);

#define LSW(x) ((u16)((u32)(x) & 0xffff))
#define MSW(x) ((u16)((u32)(x) >> 16) & 0xffff)
#define LSB(x) ((u8)(x) & 0xff)
#define MSB(x) ((u8)((x) >> 8) & 0xff)

#define cli() asm volatile("cli")
#define sti() asm volatile("sti")

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
  asm volatile("pushf\n"
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

struct PageFaultFlags {
  enum Flags {
    NotPresent = 0x00,
    ProtectionViolation = 0x01,
    Read = 0x00,
    Write = 0x02,
    UserMode = 0x04,
    SupervisorMode = 0x00,
    InstructionFetch = 0x08,
  };
};

class PageFault {
public:
  PageFault(u16 code, LinearAddress addr) : m_code(code), m_addr(addr) {}

  LinearAddress address() const { return m_addr; }
  u16 code() const { return m_code; }

  bool is_not_present() const {
    return (m_code & 1) == PageFaultFlags::NotPresent;
  }
  bool is_protection_violation() const {
    return (m_code & 1) == PageFaultFlags::ProtectionViolation;
  }
  bool is_read() const { return (m_code & 2) == PageFaultFlags::Read; }
  bool is_write() const { return (m_code & 2) == PageFaultFlags::Write; }
  bool is_user() const { return (m_code & 4) == PageFaultFlags::UserMode; }
  bool is_supervisor() const {
    return (m_code & 4) == PageFaultFlags::SupervisorMode;
  }
  bool is_instruction_fetch() const {
    return (m_code & 8) == PageFaultFlags::InstructionFetch;
  }

private:
  u16 m_code;
  LinearAddress m_addr;
};
