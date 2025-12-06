#include "allocator.h"
#include "arenaAllocator.h"
#include "b_stacktrace/b_stacktrace.h"
#include "fptr.h"
#include "hmap.h"
#include "print.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  List *elements; // List<b_stacktrace_handle>
  u32 freeCount;
} hmap_allocatorInfo;
// hmap_allocatorInfo slice

// requires struct of sizeof(hmap_allocatorInfo)
fptr startDebugInfo(hmap_allocatorInfo *scratch) {
  // effectivley a list of pointers
  b_stacktrace_handle firstElement = b_stacktrace_get();
  List *l = List_new(defaultAlloc, sizeof(b_stacktrace_handle));
  List_append(l, &firstElement);

  hmap_allocatorInfo info = (hmap_allocatorInfo){l, 0};
  memcpy(scratch, &info, sizeof(info));
  return (fptr){sizeof(info), (u8 *)scratch};
}
u32 finishDebugInfo(fptr allocatorInfoFptr, u32 *freeCount, b_stacktrace_handle **scratch) {
  b_stacktrace_handle lastElement = b_stacktrace_get();

  assertMessage(
      allocatorInfoFptr.width == sizeof(hmap_allocatorInfo),
      "retieved wrong type from storage? \n"
      ",%llu instead of %llu",
      allocatorInfoFptr.width, sizeof(hmap_allocatorInfo)
  );
  hmap_allocatorInfo h;
  setvar_aligned(h, allocatorInfoFptr.ptr);

  *freeCount = h.freeCount;
  List_append(h.elements, &lastElement);
  u32 len = h.elements->length;
  *scratch = (b_stacktrace_handle *)(h.elements->head);
  free(h.elements);
  return len;
}
// requires struct of sizeof(hmap_allocatorInfo)
// freeing: 0:realloc, 1:free
fptr changeDebugInfo(fptr allocatorInfoFptr, char freeing, hmap_allocatorInfo *scratch) {
  b_stacktrace_handle nextElement = b_stacktrace_get();

  assertMessage(
      allocatorInfoFptr.width == sizeof(hmap_allocatorInfo),
      "retieved wrong type from storage? \n"
      ",%llu instead of %llu",
      allocatorInfoFptr.width, sizeof(hmap_allocatorInfo)
  );
  hmap_allocatorInfo h;
  setvar_aligned(h, allocatorInfoFptr.ptr);

  List_append(h.elements, &nextElement);
  if (freeing) {
    h.freeCount++;
  }
  hmap_allocatorInfo info = (hmap_allocatorInfo){(h.elements), h.freeCount};
  memcpy(scratch, &info, sizeof(info));
  return (fptr){sizeof(info), (u8 *)scratch};
}

// really slow
// meant for debugging

typedef struct {
  HMap *ptrMap;
  My_allocator *actualAllocator;
  OwnAllocator own;
} HMap_allocator;
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
        startDebugInfo(scratch)
    );
  } else {
    HMap_set(
        data.ptrMap,
        fptr_fromTypeDef(res),
        changeDebugInfo(oldInfo, 0, scratch)
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
      changeDebugInfo(oldInfo, 0, scratch)
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
  // println("freed ${ptr}", ptr);
  HMap_set(
      data.ptrMap,
      fptr_fromTypeDef(ptr),
      changeDebugInfo(oldInfo, 1, scratch)
  );
}

My_allocator *hmap_alloc_new(OwnAllocator backend) {
  My_allocator *actualAllocator = backend.init();
  HMap_allocator *resarb = (HMap_allocator *)aAlloc(actualAllocator, sizeof(HMap_allocator));
  *resarb = (HMap_allocator){
      .actualAllocator = actualAllocator,
      .own = backend,
      .ptrMap = HMap_new(actualAllocator, 200),
  };
  My_allocator *res = (My_allocator *)aAlloc(actualAllocator, sizeof(My_allocator));
  *res = (My_allocator){
      .alloc = hmap_alloc_alloc,
      .r_alloc = hmap_alloc_realloc,
      .free = hmap_alloc_free,
      .arb = resarb,
  };
  return res;
}
void hmap_alloc_cleanup(My_allocator *allocator) {
  b_stacktrace_handle *scratch1[1];
  b_stacktrace_handle **scratch = scratch1;
  HMap_allocator data = *(HMap_allocator *)(allocator->arb);
  usize n = HMap_count(data.ptrMap), nulls = 0;
  for (int i = 0; i < n; i++)
    nulls += (HMap_getVal(data.ptrMap, i).width == 0);
  println("\n${} total objects in memory map", n);
  println("${} empty(reallocated) in memory map", nulls);
  for (int i = 0; i < n; i++) {
    fptr val = HMap_getVal(data.ptrMap, i);
    if (!val.width)
      continue;
    u32 freeCount;
    u32 len = finishDebugInfo(val, &freeCount, scratch);
    char *leakStatus;
    if (freeCount == 1) {
      leakStatus = "none";
    } else if (freeCount > 1) {
      leakStatus = "double free";
    } else {
      leakStatus = "dangling";
    }
    println("got back debug info :\n"
            "operations: ${}\n"
            "leakStatus: ${char_ptr}\n",
            len, leakStatus);
    if (freeCount != 1)
      print("backtraces : ");
    for (uint i = 0; i < len; i++) {
      b_stacktrace_handle ts = (*scratch)[i];
      if (freeCount != 1) {
        char *cs = b_stacktrace_to_string(ts);
        print("[ ${} ]=>{ ${}\n}\n", (size_t)i, fptr_fromCS(cs));
        free(cs);
      }
      free(ts);
    }
    free(*scratch);
  }
  data.own.deInit(data.actualAllocator);
}
