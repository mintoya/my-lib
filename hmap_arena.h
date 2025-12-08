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
  HMap *ptrMap;
  My_allocator *actualAllocator;
  OwnAllocator own;
  struct hmap_alloc_opts opts;
} HMap_allocator;
fptr startDebugInfo(hmap_allocatorInfo *scratch, const My_allocator *allocator) {
  HMap_allocator data = *(HMap_allocator *)(allocator->arb);
  // effectivley a list of pointers
  b_stacktrace_handle firstElement = b_stacktrace_get();
  List *l = List_new(allocator, sizeof(b_stacktrace_handle));
  List *memops = List_new(allocator, sizeof(memop));
  List_append(l, &firstElement);
  List_append(memops, REF(memop, ((memop_alloc))));

  hmap_allocatorInfo info = (hmap_allocatorInfo){l, memops};
  memcpy(scratch, &info, sizeof(info));
  return (fptr){sizeof(info), (u8 *)scratch};
}
void finishDebugInfo(fptr allocatorInfoFptr, memop **memopScratch, u32 *memoplen, b_stacktrace_handle **scratch, u32 *len) {

  assertMessage(
      allocatorInfoFptr.width == sizeof(hmap_allocatorInfo),
      "retieved wrong type from storage? \n"
      ",%lu instead of %lu",
      allocatorInfoFptr.width, sizeof(hmap_allocatorInfo)
  );
  hmap_allocatorInfo h;
  setvar_aligned(h, allocatorInfoFptr.ptr);
  *len = h.elements->length;
  *memoplen = h.operations->length;
  *scratch = (b_stacktrace_handle *)(h.elements->head);
  *memopScratch = (memop *)(h.operations->head);
}
// requires struct of sizeof(hmap_allocatorInfo)
// freeing: 0:realloc, 1:free
fptr changeDebugInfo(fptr allocatorInfoFptr, memop operation, hmap_allocatorInfo *scratch) {
  b_stacktrace_handle nextElement = b_stacktrace_get();

  assertMessage(
      allocatorInfoFptr.width == sizeof(hmap_allocatorInfo),
      "retieved wrong type from storage? \n"
      ",%lu instead of %lu",
      allocatorInfoFptr.width, sizeof(hmap_allocatorInfo)
  );
  hmap_allocatorInfo h;
  setvar_aligned(h, allocatorInfoFptr.ptr);

  List_append(h.elements, &nextElement);
  List_append(h.operations, REF(memop, operation));
  hmap_allocatorInfo info = (hmap_allocatorInfo){h.elements, h.operations};
  memcpy(scratch, &info, sizeof(info));
  return (fptr){sizeof(info), (u8 *)scratch};
}

// really slow
// meant for debugging

void *hmap_alloc_alloc(const My_allocator *arenaptr, size_t size) {
  HMap_allocator data = *(HMap_allocator *)(arenaptr->arb);
  void *res = aAlloc(data.actualAllocator, size);
  hmap_allocatorInfo scratch[1];
  fptr oldInfo = HMap_get(
      data.ptrMap,
      fptr_fromTypeDef(res)
  );
  if (!oldInfo.width) {
    HMap_set(
        data.ptrMap,
        fptr_fromTypeDef(res),
        startDebugInfo(scratch, data.actualAllocator)
    );
  } else {
    HMap_set(
        data.ptrMap,
        fptr_fromTypeDef(res),
        changeDebugInfo(oldInfo, memop_alloc, scratch)
    );
  }
  // println("allocated ${ptr}", res);
  return res;
}
void *hmap_alloc_realloc(const My_allocator *arenaptr, void *oldptr, size_t size) {
  hmap_allocatorInfo scratch[1];
  HMap_allocator data = *(HMap_allocator *)(arenaptr->arb);
  fptr oldInfo = HMap_get(
      data.ptrMap,
      fptr_fromTypeDef(oldptr)
  );
  void *res = aRealloc(data.actualAllocator, oldptr, size);
  // println("reallocated ${ptr} --> ${ptr}", oldptr, res);

  HMap_set(
      data.ptrMap,
      fptr_fromTypeDef(oldptr),
      nullFptr
  );
  HMap_set(
      data.ptrMap,
      fptr_fromTypeDef(res),
      changeDebugInfo(oldInfo, memop_realloc, scratch)
  );
  return res;
}
void hmap_alloc_free(const My_allocator *arenaptr, void *ptr) {
  hmap_allocatorInfo scratch[1];
  HMap_allocator data = *(HMap_allocator *)(arenaptr->arb);
  fptr oldInfo = HMap_get(
      data.ptrMap,
      fptr_fromTypeDef(ptr)
  );
  aFree(data.actualAllocator, ptr);
  HMap_set(
      data.ptrMap,
      fptr_fromTypeDef(ptr),
      changeDebugInfo(oldInfo, memop_free, scratch)
  );
}

My_allocator *hmap_alloc_new(OwnAllocator backend, struct hmap_alloc_opts opts) {
  My_allocator *actualAllocator = backend.init();
  HMap_allocator *resarb = (HMap_allocator *)aAlloc(actualAllocator, sizeof(HMap_allocator));
  *resarb = (HMap_allocator){
      .ptrMap = HMap_new(actualAllocator, 5),
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
  b_stacktrace_handle *scratch[1];
  memop *mems[1];
  HMap_allocator data = *(HMap_allocator *)(allocator->arb);
  usize n = HMap_count(data.ptrMap), nulls = 0, totalSize = 0;
  for (int i = 0; i < n; i++)
    nulls += (HMap_getVal(data.ptrMap, i).width == 0);

  println();
  for (int i = 0; i < n; i++) {
    fptr val = HMap_getVal(data.ptrMap, i);
    if (!val.width)
      continue;

    u32 memslen, scratchlen;
    finishDebugInfo(val, mems, &memslen, scratch, &scratchlen);

    if (!memslen)
      continue;
    assertMessage(memslen == scratchlen, "%i,%i", memslen, scratchlen);
    totalSize += memslen * sizeof((*mems)[0]) +
                 scratchlen * sizeof((scratch)[0]) +
                 (2 * sizeof(List));

    bool leaked = (isleak(*mems, memslen));
    char *leaktext = leaked ? (char *)"leak" : (char *)"free";
    if (data.opts.summary) {
      print("{${char_ptr}:${}:", (char *)leaktext, memslen);
      for (u32 j = 0; j < memslen; j++)
        switch ((*mems)[j]) {
        case memop_alloc:
          print("a");
          break;
        case memop_realloc:
          print("r");
          break;
        case memop_free:
          print("f");
          break;
        }
      println("}");
    }

    if (leaked) {
      if (data.opts.fnshowOpts & (enum fnshowOpts_t)(first)) {
        char *cf = b_stacktrace_to_string(((*scratch)[0]));
        println("[${}]=>\n{\n${char_ptr}\n}", (char *)"first", cf);
        free(cf);
      }
      for (int i = 1; i < scratchlen - 1; i++) {
        if (data.opts.fnshowOpts & ((enum fnshowOpts_t)(all) == (enum fnshowOpts_t)(all))) {
          char *cl = b_stacktrace_to_string(((*scratch)[i]));
          println("[${}]=>\n{\n${char_ptr}\n}", i, cl);
          free(cl);
        }
      }
      if (data.opts.fnshowOpts & (enum fnshowOpts_t)(last)) {
        char *cl = b_stacktrace_to_string(((*scratch)[scratchlen - 1]));
        println("[${}]=>\n{\n${char_ptr}\n}", (char *)"last", cl);
        free(cl);
      }
    }
    for (int i = 0; i < scratchlen; i++) {
      free((*scratch)[i]);
    }
  }
  totalSize += HMap_footprint(data.ptrMap);
  if (data.opts.footprint) {
    println();
    println("debug allocator took up ${} bytes in memory", totalSize);
  }

  data.own.deInit(data.actualAllocator);
}

#endif // HMAP_ALLOCATOR_H
