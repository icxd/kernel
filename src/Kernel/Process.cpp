#include "Process.hpp"
#include "Common.hpp"
#include "Interrupts/Interrupts.hpp"
#include "LibCore/Defines.hpp"
#include "LibCore/RetainPtr.hpp"
#include "LibCore/String.hpp"
#include "LibCore/Vector.hpp"
#include "MemoryManager.hpp"
#include "kprintf.hpp"
#include <LibCore/InlineLinkedList.hpp>
#include <LibCore/Types.hpp>

static constexpr u32 DEFAULT_STACK_SIZE = 16384;

Process *s_current;
Process *s_kernel_process;

static pid_t s_next_pid;
static InlineLinkedList<Process> *s_processes;
static InlineLinkedList<Process> *s_dead_process;
static String *s_hostname;

Vector<Process *> Process::all_processes() {
  InterruptDisabler disabler;
  Vector<Process *> processes;
  processes.ensure_capacity(s_processes->size_slow());
  for (Process *process = s_processes->head(); process;
       process = process->m_next)
    processes.push(process);
  return processes;
}

Process *Process::from_pid(const pid_t pid) {
  ASSERT(!(cpu_flags() & 0x200));
  for (auto *process = s_processes->head(); process;
       process = process->m_next) {
    if (process->pid() == pid)
      return process;
  }
  return nullptr;
}

Process::Region *Process::allocate_region(const usz size, String &&name) {
  Core::RetainPtr<Zone> zone = MM.create_zone(size);
  ASSERT(zone);
  m_regions.push(
      Core::adopt(*new Region(m_next_region, size, move(zone), move(name))));
  m_next_region = m_next_region.offset(size).offset(16384);
  return m_regions.last().ptr();
}

Process::Region *Process::allocate_region(usz size, String &&name,
                                          LinearAddress) {}

bool Process::deallocate_region(Region &region) {
  InterruptDisabler disabler;
  for (usz i = 0; i < m_regions.size(); i++) {
    if (m_regions.at(i).ptr() == &region) {
      MM.unmap_region(*this, region);
      m_regions.remove(i);
      return true;
    }
  }
  return false;
}

Process *Process::create_kernel_process(void (*entry)(), String &&name) {
  auto *process = new Process(Core::move(name), static_cast<uid_t>(0),
                              static_cast<gid_t>(0), (pid_t)0, RING_0);
  process->m_tss.eip = reinterpret_cast<u32>(entry);

  if (process->pid() != 0) {
    InterruptDisabler disabler;
    s_processes->prepend(process);
    system.nprocess++;
    okln("Kernel process {} ({}) spawned @ 0x{:x}", process->pid(),
         process->name(), process->m_tss.eip);
  }

  // if (!s_current)
  //   load_task_register(process->selector());

  return process;
}

Process::Process(String &&name, uid_t uid, gid_t gid, pid_t parent_pid,
                 RingLevel ring)
    : m_name(Core::move(name)), m_pid(s_next_pid++), m_parent_pid(parent_pid),
      m_uid(uid), m_gid(gid), m_state(RUNNABLE), m_ring(ring) {

  if (Process *parent_process = Process::from_pid(parent_pid)) {
    // m_cwd = parent_process->cwd().copy_ref();
  } else {
    // m_cwd = nullptr;
  }

  m_next_region = LinearAddress(0x600000);

  memset(&m_tss, 0, sizeof(m_tss));

  if (is_ring3()) {
    memset(m_ldt_entries, 0, sizeof(Descriptor));
    allocate_ldt();
  }

  m_tss.eflags = 0x0202;

  u16 cs = is_ring0() ? 0x08 : 0x1b;
  u16 ds = is_ring0() ? 0x10 : 0x23;
  u16 ss = is_ring0() ? 0x10 : 0x23;

  m_tss.ds = ds;
  m_tss.es = ds;
  m_tss.fs = ds;
  m_tss.gs = ds;
  m_tss.ss = ss;
  m_tss.cs = cs;

  m_tss.cr3 = MM.page_directory_base().get();

  if (is_ring0()) {
    u32 stack_bottom = reinterpret_cast<u32>(kmalloc(DEFAULT_STACK_SIZE));
    m_stack_top_0 = (stack_bottom + DEFAULT_STACK_SIZE) & 0xffffff8;
    m_tss.esp = m_stack_top_0;
  } else {
    auto *region = allocate_region(DEFAULT_STACK_SIZE, String("stack"));
    ASSERT(region);
    m_stack_top_3 = region->addr.offset(DEFAULT_STACK_SIZE).get() & 0xfffffff8;
    m_tss.esp = m_stack_top_3;
  }

  if (is_ring3()) {
    m_kernel_stack = kmalloc(DEFAULT_STACK_SIZE);
    m_stack_top_0 =
        (reinterpret_cast<u32>(m_kernel_stack) + DEFAULT_STACK_SIZE) &
        0xffffff8;
    m_tss.ss0 = 0x10;
    m_tss.esp0 = m_stack_top_0;
  }

  m_tss.ss2 = m_pid;
  m_far_ptr.offset = 0x98765432;
}

Process::~Process() {
  InterruptDisabler disabler;
  system.nprocess--;
  delete[] m_ldt_entries;
  m_ldt_entries = nullptr;

  if (m_kernel_stack) {
    kfree(m_kernel_stack);
    m_kernel_stack = nullptr;
  }
}

#if PROCESS_CHECK_SANITY
void Process::check_sanity(const char *message) {
  const String name = s_current->name();
  const char ch = name.at(0);
  okln("<{:p}> {}{{{:u}}}{} [{}] :{}: sanity check <{}>", name.characters(),
       name.characters(), name.size(), name.at(name.size() - 1),
       s_current->pid(), ch, message ? message : "");
  ASSERT((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}
#endif

static void redo_kernel_process_tss() {
  if (!s_kernel_process->selector())
    s_kernel_process->set_selector(GDT::allocate_entry());
  // s_kernel_process->set_selector(0x28);

  Descriptor &tss = GDT::get_entry(s_kernel_process->selector());
  tss.set_base(reinterpret_cast<u32>(&s_kernel_process->tss()));
  tss.set_limit(sizeof(s_kernel_process->tss()) - 1);
  tss.dpl = 0;
  tss.present = 1;
  tss.granularity = 1;
  tss.zero = 0;
  tss.operation_size = 1;
  tss.descriptor_type = 0;
  tss.type = 9;

  GDT::flush();
}

void Process::prep_for_iret_to_new_process() {
  redo_kernel_process_tss();
  s_kernel_process->tss().backlink = s_current->selector();
  load_task_register(s_kernel_process->selector());
}

void Process::initialize() {
  s_current = nullptr;
  s_next_pid = 0;
  s_processes = new InlineLinkedList<Process>;
  s_dead_process = new InlineLinkedList<Process>;
  s_kernel_process = Process::create_kernel_process(nullptr, String("colonel"));
  s_hostname = new String("birx");
  redo_kernel_process_tss();
  load_task_register(s_kernel_process->selector());
}

void Process::dump_regions() {
  okln("Process {}({}) regions:", name(), pid());
  okln("BEGIN       END         SIZE        NAME");
  for (auto &region : m_regions) {
    okln("{:x} -- {:x}    {:x}    {}", region->addr.get(),
         region->addr.offset(region->size - 1).get(), region->size,
         region->name);
  }

  okln("Process {}({}) subregions:", name(), pid());
  okln("REGION    OFFSET    BEGIN       END         SIZE        NAME");
  for (auto &subregion : m_subregions) {
    okln("{:x}  {:x}  {:x} -- {:x}    {:x}    {}",
         subregion->region->addr.get(), subregion->offset,
         subregion->addr.get(),
         subregion->addr.offset(subregion->size - 1).get(), subregion->size,
         subregion->name);
  }
}

void Process::allocate_ldt() {
  ASSERT(!m_tss.ldt);
  static constexpr u16 LDT_ENTRIES = 4;
  const u16 selector = GDT::allocate_entry();
  m_ldt_entries = new Descriptor[LDT_ENTRIES];

  {
    okln("new ldt selector = {:x}", selector);
    okln("new ldt table at = {:p}", m_ldt_entries);
    okln("new ldt table size = {}", LDT_ENTRIES * 8 - 1);
  }

  Descriptor &ldt = GDT::get_entry(selector);
  ldt.set_base(reinterpret_cast<u32>(m_ldt_entries));
  ldt.set_limit(LDT_ENTRIES * 8 - 1);
  ldt.dpl = 0;
  ldt.present = 1;
  ldt.granularity = 0;
  ldt.zero = 0;
  ldt.operation_size = 1;
  ldt.descriptor_type = 0;
  ldt.type = Descriptor::LDT;
  m_tss.ldt = selector;
}

void Process::process_did_crash(Process *crashed_process) {
  ASSERT(!(cpu_flags() & 0x200));

  crashed_process->set_state(CRASHING);
  crashed_process->dump_regions();

  s_processes->remove(crashed_process);

  MM.unmap_regions_for_process(*crashed_process);
  if (!schedule_new_process())
    PANIC("failed to schedule a new process");

  s_dead_process->append(crashed_process);
  switch_now();
}

void Process::do_house_keeping() {
  InterruptDisabler disabler;
  if (s_dead_process->empty())
    return;

  const Process *next = nullptr;
  for (const auto *dead_process = s_dead_process->head(); dead_process;
       dead_process = next) {
    next = dead_process->next();
    delete dead_process;
  }
  s_dead_process->clear();
}

void Process::block(const State state) {
  ASSERT(s_current->state() == Process::RUNNING);
  system.nblocked++;
  s_current->set_state(state);
}

void Process::unblock() {
  ASSERT(m_state != Process::RUNNABLE && m_state != Process::RUNNING);
  system.nblocked--;
  m_state = Process::RUNNABLE;
}

void block(const Process::State state) {
  s_current->block(state);
  yield();
}

void sleep(const u32 ticks) {
  ASSERT(s_current->state() == Process::RUNNING);
  s_current->set_wakeup_time(system.uptime + ticks);
  s_current->block(Process::BLOCKED_SLEEP);
  yield();
}

Process *Process::kernel_process() { return s_kernel_process; }

Process::Region::Region(const LinearAddress a, const usz s,
                        Core::RetainPtr<Zone> &&z, String &&n)
    : addr(a), size(s), zone(move(z)), name(move(n)) {}

Process::Region::~Region() = default;

Process::Subregion::Subregion(Region &r, const u32 offset, const usz s,
                              const LinearAddress l, String &&n)
    : region(r), offset(offset), size(s), addr(l), name(move(n)) {}

Process::Subregion::~Subregion() = default;

void yield() {
  if (!s_current) {
    PANIC("yield() with !current");
  }

  okln("{}<{}> yield()", s_current->name(), s_current->pid());

  InterruptDisabler disabler;
  if (!schedule_new_process())
    return;

  okln("yield() jumping to new task: {} (%s)", s_current->far_ptr().selector,
       s_current->name());
  switch_now();
}

void switch_now() {
  Descriptor &descriptor = GDT::get_entry(s_current->selector());
  descriptor.type = Descriptor::AvailableTSS_32bit;
  GDT::flush();

  asm("sti\n"
      "ljmp *(%%eax)\n" ::"a"(&s_current->far_ptr()));
}

bool context_switch(Process *process) {
  debugln("context switch to {} (same:{})", process->name(),
          s_current == process);
  process->set_ticks_left(5);
  process->did_schedule();

  debugln("same process? {}", s_current == process);
  if (s_current == process)
    return false;

  const auto cs_rpl = process->tss().cs & 3;
  const auto ss_rpl = process->tss().ss & 3;
  debugln("cs_rpl = {}, ss_rpl = {}", cs_rpl, ss_rpl);
  if (cs_rpl != ss_rpl) {
    okln("switching from {}({}) to {}({}) (RPL mismatch)", s_current->name(),
         s_current->pid(), process->name(), process->pid());
    okln("code: {}:{:x}", process->tss().cs, process->tss().eip);
    okln(" stk: {}:{:x}", process->tss().ss, process->tss().esp);
    ASSERT(cs_rpl == ss_rpl);
  }

  if (s_current) {
    if (s_current->state() == Process::RUNNING)
      s_current->set_state(Process::RUNNABLE);

    const bool success = MM.unmap_regions_for_process(*s_current);
    ASSERT(success);
  }

  const bool success = MM.map_regions_for_process(*process);
  ASSERT(success);

  s_current = process;
  process->set_state(Process::RUNNING);

  if (!process->selector())
    process->set_selector(GDT::allocate_entry());
  // process->set_selector(0x28);

  Descriptor &descriptor = GDT::get_entry(process->selector());
  descriptor.limit_high = 0;
  descriptor.limit_low = 0xffff;
  descriptor.base_low = reinterpret_cast<u32>(&process->tss()) & 0xffff;
  descriptor.base_high = (reinterpret_cast<u32>(&process->tss()) >> 16) & 0xff;
  descriptor.base_highest = (reinterpret_cast<u32>(&process->tss()) >> 24) & 0xff;
  descriptor.dpl = 0;
  descriptor.present = 1;
  descriptor.granularity = 0;
  descriptor.zero = 0;
  descriptor.operation_size = 1;
  descriptor.descriptor_type = 0;
  descriptor.type = Descriptor::BusyTSS_32bit;
  GDT::flush();

  debugln("is kernel process? {}", s_kernel_process == process);
  debugln("process->selector() = 0x{:x}", process->selector());
  // load_task_register(process->selector());

  return true;
}

bool schedule_new_process() {
  // make sure interrupts are disabled
  cli();
  ASSERT(!(cpu_flags() & 0x200));

  DBG(s_current);
  if (!s_current)
    return context_switch(Process::kernel_process());

  for (auto *process = s_processes->head(); process;
       process = process->next()) {
    if (process->state() == Process::BLOCKED_SLEEP) {
      if (process->wakeup_time() <= system.uptime) {
        process->unblock();
        continue;
      }
    }

    if (process->state() == Process::BLOCKED_WAIT) {
      if (!Process::from_pid(process->waitee())) {
        process->unblock();
        continue;
      }
    }

    // if (process->state() == Process::BLOCKED_READ) {
    //   ASSERT(process->m_fd_blocked_on_read != -1);
    //   if (process->m_file_handles[process->m_fd_blocked_on_read]
    //           ->has_data_available_for_read()) {
    //     process->unblock();
    //     continue;
    //   }
    // }
  }

  debugln("Scheduler choices:");
  for (auto *process = s_processes->head(); process;
       process = process->next()) {
    // if (process->state() == Process::BLOCKED_WAIT ||
    //     process->state() == Process::BLOCKED_SLEEP)
    //   continue;
    debugln("{} {} ({})", static_cast<u32>(process->state()),
            process->name().characters(), process->pid());
  }

  auto *prev_head = s_processes->head();
  for (;;) {
    // Move head to tail.
    s_processes->append(s_processes->remove_head());
    auto *process = s_processes->head();

    if (process->state() == Process::RUNNABLE ||
        process->state() == Process::RUNNING) {
      debugln("switch to {} ({} vs {})", process->name(),
              static_cast<void *>(process), static_cast<void *>(s_current));
      bool success = context_switch(process);
      debugln("switch success? {}", success);
      // DBG(process->tss());
      return success;
    }

    if (process == prev_head) {
      debugln("Nothing wants to run!");
      debugln("PID    OWNER      STATE  NSCHED  NAME");
      for (auto *p = s_processes->head(); p; p = p->next()) {
        debugln("{}   {}:{}  {}     {}    {}", p->pid(), p->uid(), p->gid(),
                static_cast<u32>(p->state()), p->times_scheduled(),
                p->name().characters());
      }
      debugln("Switch to kernel task");
      return context_switch(Process::kernel_process());
    }
  }
}
