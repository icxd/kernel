//
// Created by icxd on 11/12/24.
//

#pragma once

#include "kprintf.hpp"
#include "symbol.h"
#include <LibCore/Formatting.hpp>
#include <LibCore/Types.hpp>

extern "C" int walk_stack(unsigned int *array, int max_elements);

inline bool get_symbol(symbol_t *symbol, u32 addr) {
  for (u32 i = 0; i < __symbol_tab_size; i++) {
    symbol_t s = __symbol_tab[i];
    if (s.address == addr) {
      *symbol = s;
      return true;
    }
  }
  return false;
}

inline void print_stack_trace(u32 max_frames = 100) {
  u32 *addresses = new u32[max_frames];
  int num_addresses = walk_stack(addresses, max_frames);

  for (int i = 0; i < num_addresses; i++) {
    symbol_t symbol;
    if (get_symbol(&symbol, addresses[i]))
      println("  at \033[33;1m0x{:x}\033[0m <\033[34;1m{}\033[0m>",
              addresses[i], symbol.name);
    else
      println("  at \033[33;1m0x{:x}\033[0m", addresses[i]);
  }
}

template <typename... Args>
void kpanic(const char *file, usz line, const char *fn, const char *fmt,
            Args... args) {
  print("\033[31;1mPANIC! (at {}:{} in {}): \033[0m", file, line, fn);
  println(fmt, args...);

  print_stack_trace();

  hcf();
}

struct System {
  u32 uptime;
  u32 nprocess, nblocked;
};
extern System system;

class PhysicalAddress {
public:
  PhysicalAddress() {}
  explicit PhysicalAddress(u32 addr) : m_addr(addr) {}

  u32 get() const { return m_addr; }
  void set(u32 addr) { m_addr = addr; }
  void mask(u32 m) { m_addr &= m; }

  u8 *as_ptr() { return reinterpret_cast<u8 *>(m_addr); }
  const u8 *as_ptr() const { return reinterpret_cast<const u8 *>(m_addr); }

  u32 page_base() const { return m_addr & 0xfffff000; }

private:
  u32 m_addr = 0;
};

class LinearAddress {
public:
  LinearAddress() {}
  explicit LinearAddress(u32 address) : m_address(address) {}

  bool is_null() const { return m_address == 0; }

  LinearAddress offset(u32 o) const { return LinearAddress(m_address + o); }
  u32 get() const { return m_address; }
  void set(u32 address) { m_address = address; }
  void mask(u32 m) { m_address &= m; }

  bool operator<=(const LinearAddress &other) const {
    return m_address <= other.m_address;
  }
  bool operator>=(const LinearAddress &other) const {
    return m_address >= other.m_address;
  }
  bool operator>(const LinearAddress &other) const {
    return m_address > other.m_address;
  }
  bool operator<(const LinearAddress &other) const {
    return m_address < other.m_address;
  }
  bool operator==(const LinearAddress &other) const {
    return m_address == other.m_address;
  }

  u8 *as_ptr() { return reinterpret_cast<u8 *>(m_address); }
  const u8 *as_ptr() const { return reinterpret_cast<const u8 *>(m_address); }

  u32 page_base() const { return m_address & 0xfffff000; }

private:
  u32 m_address{0};
};

struct FarPtr {
  u32 offset = 0;
  u16 selector = 0;
};

template <> struct Core::Formatter<PhysicalAddress> {
  static String format(PhysicalAddress value) {
    return StringBuilder()
        .append("LinearAddress(")
        .append(value.as_ptr())
        .append(")")
        .build();
  }
};

template <> struct Core::Formatter<LinearAddress> {
  static String format(LinearAddress value) {
    return StringBuilder()
        .append("LinearAddress(")
        .append(value.as_ptr())
        .append(")")
        .build();
  }
};
