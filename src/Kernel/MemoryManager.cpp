#include "MemoryManager.hpp"
#include "Common.hpp"
#include "Interrupts/Interrupts.hpp"
#include "LibCore/RetainPtr.hpp"
#include "LibCore/Vector.hpp"
#include "Process.hpp"
#include "kmalloc.hpp"
#include "kprintf.hpp"
#include <LibCore/Defines.hpp>
#include <LibCore/Types.hpp>

static MemoryManager *s_instance;

MemoryManager &MM { return *s_instance; }

MemoryManager::MemoryManager() {
  m_page_directory = reinterpret_cast<u32 *>(0x5000);
  m_page_table_zero = reinterpret_cast<u32 *>(0x6000);
  m_page_table_one = reinterpret_cast<u32 *>(0x7000);

  initialize_paging();
}

MemoryManager::~MemoryManager() = default;

void MemoryManager::initialize_paging() {
  static_assert(sizeof(MemoryManager::PageDirectoryEntry) == 4);
  static_assert(sizeof(MemoryManager::PageTableEntry) == 4);
  memset(m_page_table_zero, 0, 4096);
  memset(m_page_table_one, 0, 4096);
  memset(m_page_directory, 0, 4096);

  okln("[MM] Page directory @ {:p}", m_page_directory);

  // this makes sure that nullptr dereferencing ends up crashing.
  protect_map(LinearAddress(0), 4 * KB);

  identity_map(LinearAddress(4096), 4 * KB);

  for (size_t i = (4 * MB) + PAGE_SIZE; i < (8 * MB); i += PAGE_SIZE)
    m_free_pages.push(PhysicalAddress(i));

  asm volatile("movl %%eax, %%cr3" ::"a"(m_page_directory));
  asm volatile("movl %cr0, %eax\n"
               "orl $80000001, %eax\n"
               "movl %eax, %cr0\n");
}

void *MemoryManager::allocate_page_table() {
  auto ppages = allocate_physical_pages(1);
  const u32 addr = ppages.at(0).get();
  identity_map(LinearAddress(addr), 4096);
  return reinterpret_cast<void *>(addr);
}

auto MemoryManager::ensure_pte(const LinearAddress addr) -> PageTableEntry {
  // make sure interrupts are disabled
  ASSERT(!(cpu_flags() & 0x200));
  const u32 page_directory_index = (addr.get() >> 22) & 0x3ff;
  const u32 page_table_index = (addr.get() >> 12) & 0x3ff;

  const auto pde = PageDirectoryEntry(&m_page_directory[page_directory_index]);
  if (!pde.is_present()) {
    okln("[MM] PDE {} not present, allocating", page_directory_index);

    if (page_directory_index == 0) {
      pde.set_page_table_base(reinterpret_cast<u32>(m_page_table_zero));
    } else if (page_directory_index == 1) {
      pde.set_page_table_base(reinterpret_cast<u32>(m_page_table_one));
    } else {
      auto *page_table = allocate_page_table();
      okln("[MM] allocated page table #{} (for laddr={:p}) at {:p}",
           page_directory_index, addr.get(), page_table);
      memset(page_table, 0, 4096);
      pde.set_page_table_base(reinterpret_cast<u32>(page_table));
    }

    pde.set_user_allowed(true);
    pde.set_present(true);
    pde.set_writable(true);
  }

  return PageTableEntry(&pde.page_table_base()[page_table_index]);
}

void MemoryManager::protect_map(LinearAddress addr, size_t length) {
  InterruptDisabler disabler;
  for (u32 offset = 0; offset < length; offset += 4096) {
    auto pte_addr = addr.offset(offset);
    auto pte = ensure_pte(pte_addr);
    pte.set_physical_page_base(pte_addr.get());
    pte.set_user_allowed(false);
    pte.set_present(false);
    pte.set_writable(false);
    flush_tlb(pte_addr);
  }
}

void MemoryManager::identity_map(const LinearAddress addr,
                                 const size_t length) {
  InterruptDisabler disabler;
  for (u32 offset = 0; offset < length; offset += 4096) {
    auto pte_addr = addr.offset(offset);
    auto pte = ensure_pte(pte_addr);
    pte.set_physical_page_base(pte_addr.get());
    pte.set_user_allowed(true);
    pte.set_present(true);
    pte.set_writable(true);
    flush_tlb(pte_addr);
  }
}

void MemoryManager::initialize() { s_instance = new MemoryManager; }

PageFaultResponse MemoryManager::handle_page_fault(const PageFault &fault) {
  // make sure interrupts are disabled
  ASSERT(!(cpu_flags() & 0x200));
  okln("[MM] handle_page_fault({}) at laddr={:p}", fault.code(),
       fault.address());
  if (fault.is_not_present())
    okln("  > NP fault!");
  else if (fault.is_protection_violation())
    okln("  > PV fault!");
  return PageFaultResponse::ShouldCrash;
}

void MemoryManager::register_zone(Zone &zone) {
  // make sure interrupts are disabled
  ASSERT(!(cpu_flags() & 0x200));
  m_zones.set(&zone);
}

void MemoryManager::unregister_zone(Zone &zone) {
  // make sure interrupts are disabled
  ASSERT(!(cpu_flags() & 0x200));
  m_zones.remove(&zone);
  m_free_pages.append(Core::move(zone.m_pages));
}

Zone::Zone(Vector<PhysicalAddress> &&pages) : m_pages(Core::move(pages)) {
  MM.register_zone(*this);
}

Zone::~Zone() { MM.unregister_zone(*this); }

Core::RetainPtr<Zone> MemoryManager::create_zone(size_t size) {
  InterruptDisabler disabler;
  Vector<PhysicalAddress> pages =
      allocate_physical_pages(Core::ceil_div(size, PAGE_SIZE));
  if (pages.is_empty()) {
    errorln("[MM] create_zone: no physical pages for size {}", size);
    return nullptr;
  }
  return Core::adopt(*new Zone(Core::move(pages)));
}

Vector<PhysicalAddress> MemoryManager::allocate_physical_pages(size_t count) {
  InterruptDisabler disabler;
  if (count > m_free_pages.size())
    return {};

  Vector<PhysicalAddress> pages;
  pages.ensure_capacity(count);
  for (size_t i = 0; i < count; i++)
    pages.push(m_free_pages.take_last());
  return pages;
}

u8 *MemoryManager::quick_map_one_page(PhysicalAddress addr) {
  // make sure interrupts are disabled
  ASSERT(!(cpu_flags() & 0x200));
  auto pte = ensure_pte(LinearAddress(4 * MB));
  okln("[MM] quickmap {:x} @ {:x} {{pte @ {%p}}}", addr.get(), 4 * MB,
       pte.ptr());
  pte.set_physical_page_base(addr.page_base());
  pte.set_present(true);
  pte.set_writable(true);
  flush_tlb(LinearAddress(4 * MB));
  return reinterpret_cast<u8 *>(4 * MB);
}

void MemoryManager::flush_entire_tlb() {
  asm volatile("mov %cr3, %eax\n"
               "mov %eax, %cr3\n");
}

void MemoryManager::flush_tlb(LinearAddress addr) {
  asm volatile("invlpg %0" : : "m"(*reinterpret_cast<char *>(addr.get())));
}

bool MemoryManager::unmap_region(Process &process, Process::Region &region) {
  InterruptDisabler disabler;
  auto &zone = *region.zone;
  for (size_t i = 0; i < zone.m_pages.size(); i++) {
    const auto laddr = region.addr.offset(i * PAGE_SIZE);
    auto pte = ensure_pte(laddr);
    pte.set_physical_page_base(0);
    pte.set_present(false);
    pte.set_writable(false);
    pte.set_user_allowed(false);
    flush_tlb(laddr);
    okln("[MM] Unmapped L{:x} => P{:x}", laddr, zone.m_pages.at(i).get());
  }

  return true;
}

bool MemoryManager::unmap_subregion(Process &process,
                                    Process::Subregion &subregion) {
  InterruptDisabler disabler;
  auto &region = *subregion.region;
  auto &zone = *region.zone;
  const size_t numPages = subregion.size / 4096;
  ASSERT(numPages);
  for (size_t i = 0; i < numPages; ++i) {
    const auto laddr = subregion.addr.offset(i * PAGE_SIZE);
    auto pte = ensure_pte(laddr);
    pte.set_physical_page_base(0);
    pte.set_present(false);
    pte.set_writable(false);
    pte.set_user_allowed(false);
    flush_tlb(laddr);
    kprintf("MM: >> Unmapped subregion {} L{:x} => P{:x} <<\n",
            subregion.name.characters(), laddr, zone.m_pages.at(i).get());
  }
  return true;
}

bool MemoryManager::unmap_regions_for_process(Process &process) {
  // make sure interrupts are disabled
  ASSERT(!(cpu_flags() & 0x200));
  for (auto &region : process.m_regions) {
    if (!unmap_region(process, *region))
      return false;
  }
  for (auto &subregion : process.m_subregions) {
    if (!unmap_subregion(process, *subregion))
      return false;
  }
  return true;
}

bool MemoryManager::map_subregion(const Process &process,
                                  Process::Subregion &subregion) {
  InterruptDisabler disabler;
  auto &region = *subregion.region;
  auto &zone = *region.zone;
  const size_t firstPage = subregion.offset / 4096;
  const size_t numPages = subregion.size / 4096;
  ASSERT(numPages);
  for (size_t i = 0; i < numPages; ++i) {
    const auto laddr = subregion.addr.offset(i * PAGE_SIZE);
    auto pte = ensure_pte(laddr);
    pte.set_physical_page_base(zone.m_pages.at(firstPage + i).get());
    pte.set_present(true);
    pte.set_writable(true);
    pte.set_user_allowed(!process.is_ring0());
    flush_tlb(laddr);
    kprintf("MM: >> Mapped subregion {} L{:x} => P{:x} ({:u} into region) <<\n",
            subregion.name.characters(), laddr,
            zone.m_pages.at(firstPage + i).get(), subregion.offset);
  }
  return true;
}

bool MemoryManager::map_region(const Process &process, Process::Region &region) {
  InterruptDisabler disabler;
  auto &zone = *region.zone;
  for (size_t i = 0; i < zone.m_pages.size(); ++i) {
    const auto laddr = region.addr.offset(i * PAGE_SIZE);
    auto pte = ensure_pte(laddr);
    pte.set_physical_page_base(zone.m_pages.at(i).get());
    pte.set_present(true);
    pte.set_writable(true);
    pte.set_user_allowed(!process.is_ring0());
    flush_tlb(laddr);
    kprintf("MM: >> Mapped L{:x} => P{:x} <<\n", laddr,
            zone.m_pages.at(i).get());
  }
  return true;
}

bool MemoryManager::map_regions_for_process(Process &process) {
  // make sure interrupts are disabled
  ASSERT(!(cpu_flags() & 0x200));
  for (auto &region : process.m_regions) {
    if (!map_region(process, *region))
      return false;
  }
  for (auto &subregion : process.m_subregions) {
    if (!map_subregion(process, *subregion))
      return false;
  }
  return true;
}

bool copy_to_zone(const Zone &zone, const void *data, size_t size) {
  if (zone.size() < size) {
    kprintf(
        "[MM] copy_to_zone: can't fit {:u} bytes into zone with size {:u}\n",
        size, zone.size());
    return false;
  }

  InterruptDisabler disabler;
  auto *dataptr = static_cast<const u8 *>(data);
  size_t remaining = size;
  for (const auto i : zone.pages()) {
    u8 *dest = MM.quick_map_one_page(i);
    kprintf("memcpy(%p, %p, %u)\n", dest, dataptr, min(PAGE_SIZE, remaining));
    memcpy(dest, dataptr, min(PAGE_SIZE, remaining));
    dataptr += PAGE_SIZE;
    remaining -= PAGE_SIZE;
  }

  return true;
}