#pragma once

#include "Common.hpp"
#include "Interrupts/Interrupts.hpp"
#include "LibCore/RetainPtr.hpp"
#include "Process.hpp"
#include "kmalloc.hpp"
#include <LibCore/HashTable.hpp>
#include <LibCore/Retainable.hpp>
#include <LibCore/Vector.hpp>

class Process;

enum class PageFaultResponse {
  ShouldCrash,
  Continue,
};

class Zone : public Core::Retainable<Zone> {
  // friend u8Buffer procfs$m();

public:
  ~Zone();
  size_t size() const { return m_pages.size() * PAGE_SIZE; }

  const Vector<PhysicalAddress> &pages() const { return m_pages; }

private:
  friend class MemoryManager;
  explicit Zone(Vector<PhysicalAddress> &&);

  Vector<PhysicalAddress> m_pages;
};

#define MM MemoryManager::instance()

class MemoryManager {
public:
  static MemoryManager &instance();

  PhysicalAddress page_directory_base() const {
    return PhysicalAddress(reinterpret_cast<u32>(m_page_directory));
  }

  static void initialize();

  u8 *quick_map_one_page(PhysicalAddress);

  PageFaultResponse handle_page_fault(const PageFault &);

  Core::RetainPtr<Zone> create_zone(size_t);

  bool map_subregion(Process &, Process::Subregion &);
  bool unmap_subregion(Process &, Process::Subregion &);
  bool map_subregions_for_process(Process &);
  bool unmap_subregions_for_process(Process &);

  bool map_region(Process &, Process::Region &);
  bool unmap_region(Process &, Process::Region &);
  bool map_regions_for_process(Process &);
  bool unmap_regions_for_process(Process &);

  void register_zone(Zone &);
  void unregister_zone(Zone &);

private:
  MemoryManager();
  ~MemoryManager();

  void initialize_paging();
  void flush_entire_tlb();
  void flush_tlb(LinearAddress);

  void *allocate_page_table();

  void protect_map(LinearAddress, size_t length);
  void identity_map(LinearAddress, size_t length);

  Vector<PhysicalAddress> allocate_physical_pages(size_t count);

  struct PageDirectoryEntry {
    explicit PageDirectoryEntry(u32 *pde) : m_pde(pde) {};

    u32 *page_table_base() {
      return reinterpret_cast<u32 *>(raw() & 0xfffff000u);
    }
    void set_page_table_base(u32 value) {
      *m_pde &= 0xfff;
      *m_pde |= value & 0xfffff000;
    }

    u32 raw() const { return *m_pde; }
    u32 *ptr() { return m_pde; }

    enum Flags {
      PRESENT = 1 << 0,
      READ_WRITE = 1 << 1,
      USER_SUPERVISOR = 1 << 2,
    };

    bool is_present() const { return raw() & PRESENT; }
    void set_present(bool b) { set_bit(PRESENT, b); }

    bool is_user_allowed() const { return raw() & USER_SUPERVISOR; }
    void set_user_allowed(bool b) { set_bit(USER_SUPERVISOR, b); }

    bool is_writable() const { return raw() & READ_WRITE; }
    void set_writable(bool b) { set_bit(READ_WRITE, b); }

    void set_bit(u8 bit, bool value) {
      if (value)
        *m_pde |= bit;
      else
        *m_pde &= ~bit;
    }

    u32 *m_pde;
  };

  struct PageTableEntry {
    explicit PageTableEntry(u32 *pte) : m_pte(pte) {}

    u32 *physical_page_base() {
      return reinterpret_cast<u32 *>(raw() & 0xfffff000u);
    }
    void set_physical_page_base(u32 value) {
      *m_pte &= 0xfffu;
      *m_pte |= value & 0xfffff000u;
    }

    u32 raw() const { return *m_pte; }
    u32 *ptr() { return m_pte; }

    enum Flags {
      PRESENT = 1 << 0,
      READ_WRITE = 1 << 1,
      USER_SUPERVISOR = 1 << 2,
    };

    bool is_present() const { return raw() & PRESENT; }
    void set_present(bool b) { set_bit(PRESENT, b); }

    bool is_user_allowed() const { return raw() & USER_SUPERVISOR; }
    void set_user_allowed(bool b) { set_bit(USER_SUPERVISOR, b); }

    bool is_writable() const { return raw() & READ_WRITE; }
    void set_writable(bool b) { set_bit(READ_WRITE, b); }

    void set_bit(u8 bit, bool value) {
      if (value)
        *m_pte |= bit;
      else
        *m_pte &= ~bit;
    }

    u32 *m_pte;
  };

  PageTableEntry ensure_pte(LinearAddress);

  u32 *m_page_directory, *m_page_table_zero, *m_page_table_one;
  HashTable<Zone *> m_zones;
  Vector<PhysicalAddress> m_free_pages;
};