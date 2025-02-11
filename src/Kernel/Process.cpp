#include "Process.hpp"
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

Process::Region *Process::allocate_region(usz size, String &&name) {
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
  Process *process =
      new Process(Core::move(name), (uid_t)0, (gid_t)0, (pid_t)0, RING_0);

  if (process->pid() != 0) {
    InterruptDisabler disabler;
    s_processes->prepend(process);
    okln("Kernel process {} ({}) spawned @ {:p}", process->pid(),
         process->name().characters(), process->m_tss.eip);
  }

  return process;
}

Process::Process(String &&name, uid_t uid, gid_t gid, pid_t parent_pid,
                 RingLevel ring)
    : m_name(Core::move(name)), m_pid(s_next_pid++), m_parent_pid(parent_pid),
      m_uid(uid), m_gid(gid), m_state(RUNNABLE), m_ring(ring) {
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
}

#if PROCESS_CHECK_SANITY
void Process::check_sanity(const char *message) {
  String name = s_current->name();
  char ch = name.at(0);
  okln("<{:p}> {}{{{:u}}}{} [{}] :{}: sanity check <{}>", name.characters(),
       name.characters(), name.size(), name.at(name.size() - 1),
       s_current->pid(), ch, message ? message : "");
  ASSERT((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}
#endif

static void redo_kernel_process_tss() {
  if (!s_kernel_process->selector())
    s_kernel_process->set_selector(GDT::allocate_entry());

  Descriptor &tss = GDT::get_entry(s_kernel_process->selector());
  tss.set_base((u32)(&s_kernel_process->tss()));
  tss.set_limit(0xffff);
  tss.dpl = 0;
  tss.present = 1;
  tss.granularity = 1;
  tss.zero = 0;
  tss.operation_size = 1;
  tss.descriptor_type = 0;
  tss.type = 9;

  GDT::flush();
}

void Process::initilize() {
  s_current = nullptr;
  s_next_pid = 0;
  s_processes = new InlineLinkedList<Process>;
  s_dead_process = new InlineLinkedList<Process>;
  s_kernel_process = Process::create_kernel_process(nullptr, String("colonel"));
  s_hostname = new String("birx");
  redo_kernel_process_tss();
  load_process_register(s_kernel_process->selector());
}

void Process::allocate_ldt() {
  ASSERT(!m_tss.ldt);
  static const u16 LDT_ENTRIES = 4;
  u16 selector = GDT::allocate_entry();
  m_ldt_entries = new Descriptor[LDT_ENTRIES];

  {
    okln("new ldt selector = {:x}", selector);
    okln("new ldt table at = {:p}", m_ldt_entries);
    okln("new ldt table size = {}", LDT_ENTRIES * 8 - 1);
  }

  Descriptor &ldt = GDT::get_entry(selector);
  ldt.set_base((u32)m_ldt_entries);
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

Process::Region::Region(LinearAddress a, usz s, Core::RetainPtr<Zone> &&z,
                        String &&n)
    : addr(a), size(s), zone(move(z)), name(move(n)) {}

Process::Region::~Region() {}

Process::Subregion::Subregion(Region &r, u32 o, usz s, LinearAddress l,
                              String &&n)
    : region(r), offset(o), size(s), addr(l), name(move(n)) {}

Process::Subregion::~Subregion() {}