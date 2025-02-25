#include "PIT.hpp"
#include "IO.hpp"
#include "Interrupts/Interrupts.hpp"
#include "PIC.hpp"
#include "Process.hpp"
#include "kprintf.hpp"

#define IRQ_TIMER 0

extern "C" void tick_isr();
extern "C" void clock_handle();

extern volatile u32 state_dump;

asm(".globl tick_isr \n"
    ".globl state_dump \n"
    "state_dump: \n"
    ".long 0\n"
    "tick_isr: \n"
    "    pusha\n"
    "    pushw %ds\n"
    "    pushw %es\n"
    "    pushw %fs\n"
    "    pushw %gs\n"
    "    pushw %ss\n"
    "    pushw %ss\n"
    "    pushw %ss\n"
    "    pushw %ss\n"
    "    pushw %ss\n"
    "    popw %ds\n"
    "    popw %es\n"
    "    popw %fs\n"
    "    popw %gs\n"
    "    mov %esp, state_dump\n"
    "    call clock_handle\n"
    "    popw %gs\n"
    "    popw %gs\n"
    "    popw %fs\n"
    "    popw %es\n"
    "    popw %ds\n"
    "    popa\n"
    "    iret\n");

#define TIMER0_CTL 0x40
#define TIMER1_CTL 0x41
#define TIMER2_CTL 0x42
#define PIT_CTL 0x43

#define TIMER0_SELECT 0x00
#define TIMER1_SELECT 0x40
#define TIMER2_SELECT 0x80

#define MODE_COUNTDOWN 0x00
#define MODE_ONESHOT 0x02
#define MODE_RATE 0x04
#define MODE_SQUARE_WAVE 0x06

#define WRITE_WORD 0x30

#define BASE_FREQUENCY 1193182

void clock_handle() {
  IRQHandlerScope scope(IRQ_TIMER);
  debugln("enter clock_handle()");

  if (!s_current)
    return;

  system.uptime++;

  if (s_current->tick())
    return;

  auto &regs = *reinterpret_cast<RegisterDump *>(state_dump);
  s_current->tss().gs = regs.gs;
  s_current->tss().fs = regs.fs;
  s_current->tss().es = regs.es;
  s_current->tss().ds = regs.ds;
  s_current->tss().edi = regs.edi;
  s_current->tss().esi = regs.esi;
  s_current->tss().ebp = regs.ebp;
  s_current->tss().ebx = regs.ebx;
  s_current->tss().edx = regs.edx;
  s_current->tss().ecx = regs.ecx;
  s_current->tss().eax = regs.eax;
  s_current->tss().eip = regs.eip;
  s_current->tss().cs = regs.cs;
  s_current->tss().eflags = regs.eflags;

  s_current->tss().esp = regs.esp + 12;
  s_current->tss().ss = regs.ss;

  if ((s_current->tss().cs & 3) != 0) {
    debugln("clock'ed across to ring0");
    debugln("code: {}:{:x}", s_current->tss().cs, s_current->tss().eip);
    debugln(" stk: {}:{:x}", s_current->tss().ss, s_current->tss().esp);
    debugln("astk: {}:{:x}", regs.ss_if_cross_ring, regs.esp_if_cross_ring);

    s_current->tss().ss = regs.ss_if_cross_ring;
    s_current->tss().esp = regs.esp_if_cross_ring;
  }

  bool result = schedule_new_process();
  DBG(result);

  // Prepare a new task to run;
  if (!result)
    return;
  Process::prep_for_iret_to_new_process();
  debugln("after prep_for_ret_to_new_process call");

  // Set the NT (nested task) flag.
  asm("pushf\n"
      "orl $0x00004000, (%esp)\n"
      "popf\n");
  debugln("end of clock_handle");
}

namespace PIT {

  void initialize() {

    IO::write8(PIT_CTL, TIMER0_SELECT | WRITE_WORD | MODE_SQUARE_WAVE);

    u32 timer_reload = (BASE_FREQUENCY / TICKS_PER_SECOND);
    okln("PIT(i8253): {} Hz, square wave ({:x})", TICKS_PER_SECOND,
         timer_reload);

    IO::write8(TIMER0_CTL, LSB(timer_reload));
    IO::write8(TIMER0_CTL, MSB(timer_reload));

    IDT::register_interrupt_handler(IRQ_VECTOR_BASE + IRQ_TIMER, tick_isr);

    PIC::enable(IRQ_TIMER);
  }

} // namespace PIT
