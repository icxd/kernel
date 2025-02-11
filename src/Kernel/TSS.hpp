#pragma once

#include "Common.hpp"

struct TSS32 {
  u16 backlink, __blh;
  u64 esp0;
  u16 ss0, __ss0h;
  u64 esp1;
  u16 ss1, __ss1h;
  u64 esp2;
  u16 ss2, __ss2h;
  u64 cr3, eip, eflags;
  u64 eax, ecx, edx, ebx, esp, ebp, esi, edi;
  u16 es, __esh;
  u16 cs, __csh;
  u16 ss, __ssh;
  u16 ds, __dsh;
  u16 fs, __fsh;
  u16 gs, __gsh;
  u16 ldt, __ldth;
  u16 trace, iomapbase;
} PACKED;