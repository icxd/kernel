//
// Created by icxd on 11/6/24.
//

#include "kmalloc.hpp"
#include "Interrupts/Interrupts.hpp"
#include "Symbol.hpp"
#include "kprintf.hpp"
#include <LibC/string.h>
#include <LibCore/Defines.hpp>

struct PACKED Allocation {
  size_t start, nchunk;
};

#define CHUNK_SIZE 32
#define POOL_SIZE (1024 * 1024)

#define BASE_PHYSICAL (3 * MB)
#define RANGE_SIZE (1 * MB)

static u8 alloc_map[POOL_SIZE / CHUNK_SIZE / 8];

volatile size_t sum_alloc = 0, sum_free = POOL_SIZE;
u32 g_kmalloc_call_count, g_kfree_call_count;
bool g_dump_kmalloc_stacks;

bool is_kmalloc_address(const void *ptr) {
  return (size_t)ptr >= BASE_PHYSICAL &&
         (size_t)ptr <= (BASE_PHYSICAL + POOL_SIZE);
}

void kmalloc_init() {
  memset(&alloc_map, 0, sizeof(alloc_map));
  memset((void *)BASE_PHYSICAL, 0, POOL_SIZE);

  sum_alloc = 0;
  sum_free = POOL_SIZE;
}

void *kmalloc_aligned(size_t size, size_t alignment) {
  void *ptr = kmalloc(size + alignment + sizeof(void *));
  size_t max_addr = (size_t)ptr + alignment;
  void *aligned_ptr = (void *)(max_addr - (max_addr % alignment));
  ((void **)aligned_ptr)[-1] = ptr;
  return aligned_ptr;
}

void kfree_aligned(void *ptr) { kfree(((void **)ptr)[-1]); }

void *kmalloc_page_aligned(size_t size) {
  void *ptr = kmalloc_aligned(size, PAGE_SIZE);
  auto d = (size_t)ptr;
  ASSERT((d & PAGE_MASK) == 0);
  return ptr;
}

void *kmalloc_impl(size_t size) {
  InterruptDisabler disabler;
  g_kmalloc_call_count++;

  if (g_dump_kmalloc_stacks && symbols_ready) {
    dump_backtrace();
  }

  size_t real_size = size + sizeof(Allocation);
  if (sum_free < real_size) {
    dump_backtrace();
    kprintf("kmalloc: out of memory\n");
    hcf();
  }

  size_t chunks_needed = real_size / CHUNK_SIZE;
  if (real_size % CHUNK_SIZE)
    chunks_needed++;

  size_t chunks_here = 0, first_chunk = 0;
  for (size_t i = 0; i < (POOL_SIZE / CHUNK_SIZE / 8); i++) {
    if (alloc_map[i] == 0xff) {
      chunks_here = 0;
      continue;
    }

    for (size_t j = 0; j < 8; j++) {
      if (!(alloc_map[i] & (1 << j))) {
        if (chunks_here == 0)
          first_chunk = i * 8 + j;

        chunks_here++;
        if (chunks_here == chunks_needed) {
          auto *a = (Allocation *)(BASE_PHYSICAL + (first_chunk * CHUNK_SIZE));
          u8 *ptr = (u8 *)a;
          ptr += sizeof(Allocation);
          a->nchunk = chunks_needed;
          a->start = first_chunk;

          for (size_t k = first_chunk; k < (first_chunk + chunks_needed); k++)
            alloc_map[k / 8] |= 1 << (k % 8);

          sum_alloc += a->nchunk * CHUNK_SIZE;
          sum_free -= a->nchunk * CHUNK_SIZE;

          memset(ptr, 0xbb, (a->nchunk * CHUNK_SIZE) - sizeof(Allocation));

          return ptr;
        }
      } else {
        chunks_here = 0;
      }
    }
  }

  kprintf("kmalloc(): PANIC! Out of memory (no suitable block for size {})\n",
          size);
  dump_backtrace();
  hcf();
}

void kfree(void *ptr) {
  if (!ptr)
    return;

  InterruptDisabler disabler;
  g_kfree_call_count++;

  auto *a = (Allocation *)((((u8 *)ptr) - sizeof(Allocation)));
  for (size_t i = a->start; i < (a->start + a->nchunk); i++)
    alloc_map[i / 8] &= ~(1 << (i % 8));

  sum_alloc -= a->nchunk * CHUNK_SIZE;
  sum_free += a->nchunk * CHUNK_SIZE;

  memset(a, 0xaa, a->nchunk * CHUNK_SIZE);
}

void *operator new(size_t size) { return kmalloc(size); }
void *operator new[](size_t size) { return kmalloc(size); }

void operator delete(void *ptr) { return kfree(ptr); }
void operator delete[](void *ptr) { return kfree(ptr); }
void operator delete(void *ptr, size_t) { return kfree(ptr); }
void operator delete[](void *ptr, size_t) { return kfree(ptr); }
