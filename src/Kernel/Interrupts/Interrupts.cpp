//
// Created by icxd on 11/1/24.
//

#include "Interrupts.hpp"
#include "../MemoryManager.hpp"
#include "../PIC.hpp"
#include "../kprintf.hpp"
#include <LibCore/Array.hpp>
#include <LibCore/Defines.hpp>
#include <LibCore/Types.hpp>

struct PACKED DescriptorTablePointer {
  u16 limit;
  void *address;
};

static DescriptorTablePointer s_idtr;
static DescriptorTablePointer s_gdtr;
static Descriptor s_idt[256];
static Descriptor s_gdt[256];

static IRQHandler *s_irq_handlers[16];

static Array<u16, 256> *s_gdt_freelist;
static u16 s_gdt_length;

u16 gdt_alloc_entry() {
  ASSERT(s_gdt_freelist);
  ASSERT(!s_gdt_freelist->is_empty());
  return s_gdt_freelist->pop();
}

void gdt_free_entry(u16 entry) {
  ASSERT(s_gdt_freelist);
  s_gdt_freelist->push(entry);
}

extern "C" void handle_irq();
extern "C" void asm_irq_entry();

asm(".global asm_irq_entry\n"
    "asm_irq_entry:\n"
    "    pusha\n"
    "    pushw %ds\n"
    "    pushw %es\n"
    "    pushw %ss\n"
    "    pushw %ss\n"
    "    popw %ds\n"
    "    popw %es\n"
    "    call handle_irq\n"
    "    popw %es\n"
    "    popw %ds\n"
    "    popa\n"
    "    iret\n");

extern volatile u32 exception_state_dump;
extern volatile u16 exception_code;
asm(".globl exception_state_dump\n"
    "exception_state_dump:\n"
    ".long 0\n"
    ".globl exception_code\n"
    "exception_code:\n"
    ".short 0\n");

#define EH_ENTRY(ec)                                                           \
  extern "C" void exception_##ec##_handler(RegisterDumpWithExceptionCode &);   \
  extern "C" void exception_##ec##_entry();                                    \
  asm(".global exception_" #ec "_entry\n"                                      \
      "exception_" #ec "_entry:\n"                                             \
      "    pusha\n"                                                            \
      "    pushw %ds\n"                                                        \
      "    pushw %es\n"                                                        \
      "    pushw %fs\n"                                                        \
      "    pushw %gs\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    popw %ds\n"                                                         \
      "    popw %es\n"                                                         \
      "    popw %fs\n"                                                         \
      "    popw %gs\n"                                                         \
      "    mov %esp, %eax\n"                                                   \
      "    call exception_" #ec "_handler\n"                                   \
      "    popw %gs\n"                                                         \
      "    popw %gs\n"                                                         \
      "    popw %fs\n"                                                         \
      "    popw %es\n"                                                         \
      "    popw %ds\n"                                                         \
      "    popa\n"                                                             \
      "    add $0x4, %esp\n"                                                   \
      "    iret\n");
#define EH_ENTRY_NO_CODE(ec)                                                   \
  extern "C" void exception_##ec##_handler(RegisterDump &);                    \
  extern "C" void exception_##ec##_entry();                                    \
  asm(".globl exception_" #ec "_entry\n"                                       \
      "exception_" #ec "_entry: \n"                                            \
      "    pusha\n"                                                            \
      "    pushw %ds\n"                                                        \
      "    pushw %es\n"                                                        \
      "    pushw %fs\n"                                                        \
      "    pushw %gs\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    pushw %ss\n"                                                        \
      "    popw %ds\n"                                                         \
      "    popw %es\n"                                                         \
      "    popw %fs\n"                                                         \
      "    popw %gs\n"                                                         \
      "    mov %esp, %eax\n"                                                   \
      "    call exception_" #ec "_handler\n"                                   \
      "    popw %gs\n"                                                         \
      "    popw %gs\n"                                                         \
      "    popw %fs\n"                                                         \
      "    popw %es\n"                                                         \
      "    popw %ds\n"                                                         \
      "    popa\n"                                                             \
      "    iret\n");
template <typename DumpType> static void dump(const DumpType &regs) {
  u16 ss = regs.ds;
  u32 esp = regs.esp;

  if constexpr (Core::IsSame<DumpType, RegisterDumpWithExceptionCode>)
    println("exception code: {}", regs.exception_code);

  println("  pc={}:0x{:x} ds={} fs={} gs={}", regs.cs, regs.eip, regs.ds,
          regs.es, regs.fs, regs.gs);
  println(" stk={}:0x{:x}", ss, esp);
  println("eax=0x{:x} ebx=0x{:x} ecx=0x{:x} edx=0x{:x}", regs.eax, regs.ebx,
          regs.ecx, regs.edx);
  println("ebp=0x{:x} esp=0x{:x} esi=0x{:x} edi=0x{:x}", regs.ebp, esp,
          regs.esi, regs.edi);
}

template <typename DumpType>
static void handle_crash(DumpType &regs, const char *message) {
  println("\033[31;1mCRASH: {}\033[0m", message);

  u16 ss;
  u32 esp;
  if (s_current->is_ring0()) {
    ss = regs.ds;
    esp = regs.esp;
  } else {
    ss = regs.ss_if_cross_ring;
    esp = regs.esp_if_cross_ring;
  }

  dump(regs);

  if (s_current->is_ring0())
    PANIC("Oh shit, we've crashed in ring 0 :(");

  Process::process_did_crash(s_current);
}

#define EH_ENTRY_FN(n)                                                         \
  EH_ENTRY(n) void exception_##n##_handler(RegisterDumpWithExceptionCode &regs)
#define EH_ENTRY_NO_CODE_FN(n)                                                 \
  EH_ENTRY_NO_CODE(n) void exception_##n##_handler(RegisterDump &regs)

EH_ENTRY_NO_CODE_FN(0) { handle_crash(regs, "Division by zero"); }
EH_ENTRY_NO_CODE_FN(6) { handle_crash(regs, "Illegal instruction"); }
EH_ENTRY_NO_CODE_FN(7) { handle_crash(regs, "FPU not available, TODO!"); }
EH_ENTRY_FN(13) { handle_crash(regs, "General protection fault"); }
EH_ENTRY_FN(14) {
  u32 fault_address;
  asm("movl %%cr2, %%eax" : "=a"(fault_address));

  u32 fault_page_directory;
  asm("movl %%cr3, %%eax" : "=a"(fault_page_directory));

  okln("Ring{} page fault in {}({}), %s laddr={}\n", regs.cs & 3,
       s_current->name().characters(), s_current->pid(),
       exception_code & 2 ? "write" : "read", fault_address);

  dump(regs);

  u8 *codeptr = (u8 *)regs.eip;
  errorln("code: {} {} {} {} {} {} {} {}", codeptr[0], codeptr[1], codeptr[2],
          codeptr[3], codeptr[4], codeptr[5], codeptr[6], codeptr[7]);

  if (s_current->is_ring0())
    hcf();

  PageFaultResponse response = MM.handle_page_fault(
      PageFault(exception_code, LinearAddress(fault_address)));
  switch (response) {
  case PageFaultResponse::ShouldCrash:
    Process::process_did_crash(s_current);
    break;
  case PageFaultResponse::Continue:
    warnln("Continuing after resolved page fault.");
    break;
  }
}

#define EH(n, msg)                                                             \
  static void _exception##n() {                                                \
    errorln(msg);                                                              \
    u32 cr0, cr2, cr3, cr4;                                                    \
    asm("movl %%cr0, %%eax" : "=a"(cr0));                                      \
    asm("movl %%cr2, %%eax" : "=a"(cr2));                                      \
    asm("movl %%cr3, %%eax" : "=a"(cr3));                                      \
    asm("movl %%cr4, %%eax" : "=a"(cr4));                                      \
    errorln("  cr0=\033[31;1m0x{:x}\033[0m", cr0);                             \
    errorln("  cr2=\033[31;1m0x{:x}\033[0m", cr2);                             \
    errorln("  cr3=\033[31;1m0x{:x}\033[0m", cr3);                             \
    errorln("  cr4=\033[31;1m0x{:x}\033[0m", cr4);                             \
    hcf();                                                                     \
  }

EH(1, "Debug exception");
EH(2, "Unknown error");
EH(3, "Break point");
EH(4, "Overflow");
EH(5, "Bounds check");
EH(8, "Double fault");
EH(9, "Coprocessor segment overrun");
EH(10, "Invalid TSS");
EH(11, "Segment not present");
EH(12, "Stack exception");
EH(15, "Unknown error");
EH(16, "Coprocessor error");

namespace GDT {

  static void write_raw_entry(u16 selector, u32 low, u32 high) {
    u16 i = (selector & 0xfff8) >> 3;
    s_gdt[i].low = low;
    s_gdt[i].high = high;

    if (i > s_gdt_length)
      s_gdtr.limit = (s_gdt_length + 1) * 8 - 1;
  }

  void write_entry(u16 selector, Descriptor &descriptor) {
    write_raw_entry(selector, descriptor.low, descriptor.high);
  }

  void flush() {
    s_gdtr.address = s_gdt;
    s_gdtr.limit = s_gdt_length * 8 - 1;
    asm("lgdt %0" ::"m"(s_gdtr) : "memory");
  }

  void init() {
    s_gdt_freelist = new Array<u16, 256>();
    for (usz i = s_gdt_length; i < 256; i++)
      s_gdt_freelist->set(i, i * 8);

    s_gdt_length = 5;
    s_gdtr.address = s_gdt;
    s_gdtr.limit = s_gdt_length * 8 - 1;

    write_raw_entry(0x0000, 0x00000000, 0x00000000);
    write_raw_entry(0x0008, 0x0000ffff, 0x00cf9a00);
    write_raw_entry(0x0010, 0x0000ffff, 0x00cf9200);
    write_raw_entry(0x0018, 0x0000ffff, 0x00cffa00);
    write_raw_entry(0x0020, 0x0000ffff, 0x00cff200);

    flush();

    asm volatile("mov %%ax, %%ds\n"
                 "mov %%ax, %%es\n"
                 "mov %%ax, %%fs\n"
                 "mov %%ax, %%gs\n"
                 "mov %%ax, %%ss\n" ::"a"(0x10)
                 : "memory");
    asm volatile("ljmpl $0x08, $sanity\n"
                 "sanity:\n");
  }

  u16 allocate_entry() {
    ASSERT(s_gdt_length < 256);
    u16 selector = s_gdt_length * 8;
    s_gdt_length++;
    return selector;
  }

  Descriptor &get_entry(u16 selector) {
    u16 i = (selector & 0xfffc) >> 3;
    return *(Descriptor *)(&s_gdt[i]);
  }

} // namespace GDT

namespace IDT {

  static void unimplemented_trap() {
    kprintf("Unhandled IRQ");
    hcf();
  }

  void register_irq_handler(u8 irq, IRQHandler &handler) {
    ASSERT(!s_irq_handlers[irq]);
    s_irq_handlers[irq] = &handler;
    debugln("IRQ handler for {:p}\n", irq, (void *)&handler);
    register_interrupt_handler(IRQ_VECTOR_BASE + irq, asm_irq_entry);
  }

  void unregister_irq_handler(u8 irq, IRQHandler &handler) {
    ASSERT(s_irq_handlers[irq] == &handler);
    s_irq_handlers[irq] = nullptr;
  }

  void register_interrupt_handler(u8 vector, void (*handler)()) {
    s_idt[vector].low = 0x00080000 | LSW(handler);
    s_idt[vector].high = ((u32)(handler) & 0xffff0000) | 0xef00;
  }

  void init() {
    s_idtr.address = s_idt;
    s_idtr.limit = 0x100 * 8 - 1;

    for (u8 i = 0xff; i > 0x10; i--)
      register_interrupt_handler(i, unimplemented_trap);

    register_interrupt_handler(0x00, exception_0_entry);
    register_interrupt_handler(0x01, _exception1);
    register_interrupt_handler(0x02, _exception2);
    register_interrupt_handler(0x03, _exception3);
    register_interrupt_handler(0x04, _exception4);
    register_interrupt_handler(0x05, _exception5);
    register_interrupt_handler(0x06, exception_6_entry);
    register_interrupt_handler(0x07, exception_7_entry);
    register_interrupt_handler(0x08, _exception8);
    register_interrupt_handler(0x09, _exception9);
    register_interrupt_handler(0x0a, _exception10);
    register_interrupt_handler(0x0b, _exception11);
    register_interrupt_handler(0x0c, _exception12);
    register_interrupt_handler(0x0d, exception_13_entry);
    register_interrupt_handler(0x0e, exception_14_entry);
    register_interrupt_handler(0x0f, _exception15);
    register_interrupt_handler(0x10, _exception16);

    for (auto &s_irq_handler : s_irq_handlers)
      s_irq_handler = nullptr;

    flush();
  }

  void flush() { asm("lidt %0" ::"m"(s_idtr)); }

} // namespace IDT

void load_process_register(u16 selector) { asm("ltr %0" : : "r"(selector)); }

void handle_irq() {
  u16 isr = PIC::get_isr();
  if (!isr) {
    kprintf("Spurious IRQ\n");
    return;
  }

  u8 irq = 0;
  for (u8 i = 0; i < 16; ++i) {
    if (i == 2)
      continue;
    if (isr & (1 << i)) {
      irq = i;
      break;
    }
  }

  if (s_irq_handlers[irq])
    s_irq_handlers[irq]->handle_irq();
  PIC::eoi(irq);
}
