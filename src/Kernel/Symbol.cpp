//
// Created by icxd on 11/6/24.
//

#include "Symbol.hpp"
#include <LibCore/Defines.hpp>

static Symbol *s_symbols;
u32 symbols_lowest_address, symbols_highest_address, symbols_count;
bool symbols_ready;

static u8 parse_hex_digit(char nibble) {
  if (nibble >= '0' && nibble <= '9')
    return nibble - '0';
  ASSERT(nibble >= 'a' && nibble <= 'f');
  return nibble - 'a' + 10;
}

const Symbol *symbolicate(u32 address) {
  if (address < symbols_lowest_address || address > symbols_highest_address)
    return nullptr;
  for (unsigned i = 0; i < symbols_count; i++) {
    if (address < s_symbols[i + 1].address)
      return &s_symbols[i];
  }
  return nullptr;
}

void load_symbols() {

}

void init_symbols() {

}

void dump_backtrace() {

}
