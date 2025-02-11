//
// Created by icxd on 11/6/24.
//

#ifndef KERNEL_SYMBOL_H
#define KERNEL_SYMBOL_H

typedef struct {
  unsigned int address;
  char *name;
} symbol_t;

extern const __attribute__((weak)) symbol_t __symbol_tab[];
extern const __attribute__((weak)) unsigned int __symbol_tab_size;

#endif // KERNEL_SYMBOL_H