#pragma once

#include "Common.hpp"
#include "Interrupts/Interrupts.hpp"
#include "TSS.hpp"
#include <LibCore/InlineLinkedList.hpp>
#include <LibCore/OwnPtr.hpp>
#include <LibCore/RetainPtr.hpp>
#include <LibCore/Retainable.hpp>
#include <LibCore/String.hpp>
#include <LibCore/Types.hpp>
#include <LibCore/Vector.hpp>

#define PROCESS_CHECK_SANITY 1

class Zone;

class Process : public InlineLinkedListNode<Process> {
public:
  friend struct InlineLinkedListNode<Process>;

  struct Region;
  struct Subregion;

public:
  ~Process();

  static Process *create_kernel_process(void (*entry)(), String &&name);
  static Process *create_user_process(const String &path, uid_t, gid_t,
                                      pid_t parent_pid, int &error,
                                      const char **args = nullptr);

  static Vector<Process *> all_processes();

#if PROCESS_CHECK_SANITY
  static void check_sanity(const char *message = nullptr);
#else
  static void check_sanity(const char *) {}
#endif

  enum State {
    INVALID,
    RUNNABLE,
    RUNNING,
    TERMINATED,
    CRASHING,
    EXITING,
    BLOCKED_SLEEP,
    BLOCKED_WAIT,
    BLOCKED_READ,
  };

  enum RingLevel { RING_0 = 0, RING_3 = 3 };

  bool is_ring0() const { return m_ring == RING_0; };
  bool is_ring3() const { return m_ring == RING_3; };

  static Process *from_pid(pid_t);
  static Process *kernel_process();

  static void block(State state);
  void unblock();

  const String &name() const { return m_name; }
  pid_t pid() const { return m_pid; }
  TSS32 &tss() { return m_tss; }
  u16 selector() const { return m_far_ptr.selector; }
  State state() const { return m_state; };
  uid_t uid() const { return m_uid; }
  gid_t gid() const { return m_gid; }
  const FarPtr &far_ptr() const { return m_far_ptr; }

  static void process_did_crash(Process *crashed_process);
  static void do_house_keeping();

  void set_wakeup_time(u32 t) { m_wakeup_time = t; }
  u32 wakeup_time() const { return m_wakeup_time; }

  static void prep_for_iret_to_new_process();

  bool tick() {
    m_ticks++;
    return m_ticks_left--;
  }
  void set_ticks_left(u32 t) { m_ticks_left = t; }

  void set_selector(u16 s) { m_far_ptr.selector = s; }
  void set_state(State state) { m_state = state; }

  pid_t parent_pid() const { return m_parent_pid; }

  static void initialize();

  const Vector<Core::RetainPtr<Region>> &regions() const { return m_regions; };
  const Vector<Core::OwnPtr<Subregion>> &subregions() const {
    return m_subregions;
  };
  void dump_regions();

  void did_schedule() { m_times_scheduled++; }
  u32 times_scheduled() const { return m_times_scheduled; }

  pid_t waitee() const { return m_waitee; }

private:
  friend class MemoryManager;
  friend bool schedulue_new_process();

  Process(String &&name, uid_t, gid_t, pid_t parent_pid, RingLevel);

  void allocate_ldt();

  Process *m_prev = nullptr, *m_next = nullptr;
  String m_name;
  void (*m_entry)() = nullptr;
  pid_t m_pid = 0, m_parent_pid = 0;
  uid_t m_uid = 0;
  gid_t m_gid = 0;
  u32 m_ticks = 0, m_ticks_left = 0;
  u32 m_wakeup_time = 0;
  u32 m_stack_top_0 = 0, m_stack_top_3 = 0;
  FarPtr m_far_ptr;
  State m_state = INVALID;
  TSS32 m_tss;
  Descriptor *m_ldt_entries = nullptr;
  RingLevel m_ring = RING_0;
  int m_error = 0;
  void *m_kernel_stack = nullptr;
  u32 m_times_scheduled = 0;
  pid_t m_waitee = -1;

public:
  struct Region : Core::Retainable<Region> {
    Region(LinearAddress, size_t, Core::RetainPtr<Zone> &&, String &&);
    ~Region();

    LinearAddress addr;
    size_t size = 0;
    Core::RetainPtr<Zone> zone;
    String name;
  };

  struct Subregion : Core::Retainable<Subregion> {
    Subregion(Region &, u32 offset, size_t, LinearAddress, String &&);
    ~Subregion();

    Core::RetainPtr<Region> region;
    u32 offset;
    size_t size = 0;
    LinearAddress addr;
    String name;
  };

  Region *allocate_region(usz, String &&name);
  Region *allocate_region(usz, String &&name, LinearAddress);
  bool deallocate_region(Region &region);

private:
  Vector<Core::RetainPtr<Region>> m_regions;
  Vector<Core::OwnPtr<Subregion>> m_subregions;

  LinearAddress m_next_region;
};

extern void process_init();
extern void yield();
extern bool schedule_new_process();
extern void switch_now();
extern void block(Process::State);
extern void sleep(u32 ticks);

extern Process *s_current;
