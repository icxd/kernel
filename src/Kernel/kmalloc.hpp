//
// Created by icxd on 11/6/24.
//

#pragma once

#include <LibCore/Types.hpp>
#include <LibCpp/cstddef.hpp>

// FIXME: these should NOT be located here
#define PAGE_SIZE 4096
#define PAGE_MASK 0xfffff000

void kmalloc_init();
[[gnu::malloc, gnu::returns_nonnull, gnu::alloc_size(1)]] void *
kmalloc_impl(size_t size);
[[gnu::malloc, gnu::returns_nonnull, gnu::alloc_size(1)]] void *
kmalloc_page_aligned(size_t size);
[[gnu::malloc, gnu::returns_nonnull, gnu::alloc_size(1)]] void *
kmalloc_aligned(size_t size, size_t alignment);
void kfree(void *ptr);
void kfree_aligned(void *ptr);

bool is_kmalloc_address(const void *ptr);

extern volatile size_t sum_alloc, sum_free;
extern u32 g_kmalloc_call_count;
extern u32 g_kfree_call_count;
extern bool g_dump_kmalloc_stacks;

inline void *operator new(size_t, void *ptr) { return ptr; }
inline void *operator new[](size_t, void *ptr) { return ptr; }

[[gnu::always_inline]] inline void *kmalloc(size_t size) {
  return kmalloc_impl(size);
}
