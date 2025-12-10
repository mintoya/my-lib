#ifndef HMAP_ALLOCATOR_H
#define HMAP_ALLOCATOR_H
enum fnshowOpts_t : unsigned char {
  none = 0b000,
  all = 0b111,
  first = 0b001,
  last = 0b100,
};
struct hmap_alloc_opts {
  bool summary : 1;
  bool footprint : 1;
  enum fnshowOpts_t fnshowOpts : 3;
};
#include "allocator.h"
My_allocator *hmap_alloc_new(OwnAllocator backend, struct hmap_alloc_opts);
void hmap_alloc_cleanup(My_allocator *allocator);
#endif // HMAP_ALLOCATOR_H
//
// #define HMAP_ALLOCATOR_C
//

#if (defined(__INCLUDE_LEVEL__) && __INCLUDE_LEVEL__ == 0)
#pragma once
#define HMAP_ALLOCATOR_C (1)
#endif
#ifdef HMAP_ALLOCATOR_C
#define B_STACKTRACE_IMPL
#include "b_stacktrace/b_stacktrace.h"
#include "fptr.h"
#include "hmap.h"
#include "print.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef enum : u8 { memop_alloc,
                    memop_free,
                    memop_realloc,
} memop;
typedef struct {
  List *elements;   // List<b_stacktrace_handle>
  List *operations; // List<memop>
} hmap_allocatorInfo;
typedef struct {
  HHMap *ptrMap;
  My_allocator *actualAllocator;
  OwnAllocator own;
  struct hmap_alloc_opts opts;
} HMap_allocator;

void *hmap_alloc_alloc(const My_allocator *arenaptr, size_t size) {
  HMap_allocator *self = (HMap_allocator *)arenaptr->arb;
  void *res = aAlloc(self->actualAllocator, size);
  println("alloc {ptr}", res);
  return res;
}
void *hmap_alloc_realloc(const My_allocator *arenaptr, void *oldptr, size_t size) {
  HMap_allocator *self = (HMap_allocator *)arenaptr->arb;
  void *res = aRealloc(self->actualAllocator, oldptr, size);
  println("realloc {ptr} -> {ptr}", oldptr, res);
  return res;
}
void hmap_alloc_free(const My_allocator *arenaptr, void *ptr) {
  HMap_allocator *self = (HMap_allocator *)arenaptr->arb;
  println("fre {ptr}", ptr);
  aFree(self->actualAllocator, ptr);
}

My_allocator *hmap_alloc_new(OwnAllocator backend, struct hmap_alloc_opts opts) {
  My_allocator *actualAllocator = backend.init();
  HMap_allocator *resarb = (HMap_allocator *)aAlloc(actualAllocator, sizeof(HMap_allocator));
  *resarb = (HMap_allocator){
      .ptrMap = HHMap_new(sizeof(void *), sizeof(hmap_allocatorInfo), actualAllocator, 20),
      .actualAllocator = actualAllocator,
      .own = backend,
      .opts = opts,
  };
  My_allocator *res = (My_allocator *)aAlloc(actualAllocator, sizeof(My_allocator));
  *res = (My_allocator){
      .alloc = hmap_alloc_alloc,
      .free = hmap_alloc_free,
      .ralloc = hmap_alloc_realloc,
      .arb = resarb,
  };
  return res;
}
bool isleak(memop *l, u32 len) {
  memop state = memop_free;
  for (u32 i = 0; i < len; i++) {
    memop next = l[i];
    switch (state) {
    case memop_alloc:
      if (next == memop_alloc)
        return true;
      break;
    case memop_realloc:
      if (next == memop_alloc)
        return true;
      break;
    case memop_free:
      if (next == memop_free)
        return true;
      else if (next == memop_realloc)
        return true;
      break;
    }
    state = next;
  }
  return state != memop_free;
}
void hmap_alloc_cleanup(My_allocator *allocator) {
  HMap_allocator *self = (HMap_allocator *)allocator->arb;
  self->own.deInit(self->actualAllocator);
}

#endif // HMAP_ALLOCATOR_H
