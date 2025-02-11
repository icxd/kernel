//
// Created by icxd on 11/12/24.
//

#pragma once

#include "LibCore/Error.hpp"
#include "kprintf.hpp"
#include "symbol.h"
#include <LibCore/Formatting.hpp>
#include <LibCore/Types.hpp>

struct Stacktrace {
  Stacktrace *ebp;
  u32 eip;
};

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
  Stacktrace *stk;
  asm("movl %%ebp, %0" : "=r"(stk)::);
  for (u32 frame = 0; stk && frame < max_frames; frame++) {
    symbol_t symbol;
    if (get_symbol(&symbol, stk->eip))
      println("  at \033[33;1m0x{:x}\033[0m <\033[34;1m{}\033[0m>", stk->eip,
              symbol.name);
    else
      println("  at \033[33;1m0x{:x}\033[0m", stk->eip);

    stk = stk->ebp;
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
