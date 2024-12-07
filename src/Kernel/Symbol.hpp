//
// Created by icxd on 11/6/24.
//

#pragma once

#include <LibCore/Types.hpp>

extern bool symbols_ready;
extern u32 symbols_lowest_address, symbols_highest_address;

struct Symbol {
  u32 address;
  const char *name;
};

const Symbol *symbolicate(u32 address);
void load_symbols();
void init_symbols();
void dump_backtrace();