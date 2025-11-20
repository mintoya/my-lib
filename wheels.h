// because order matters
#ifdef ARENA_ALLOCATOR_H
#ifdef CHECKINCLUDES
#pragma message("arena_allocator_H")
#endif
#define ARENA_ALLOCATOR_C
#include "arenaAllocator.h"
#endif

#ifdef STATE_MACHINE_H
#ifdef CHECKINCLUDES
#pragma message("state_machine_h")
#endif
#define STATE_MACHINE_C
#include "stateMachine.h"
#endif

#ifdef KMLM_H
#define KMLM_C
#include "kmlM.h"
#endif

#ifdef UMAP_H
#ifdef CHECKINCLUDES
#pragma message("umap_h")
#endif
#define UMAP_C
#include "umap.h"
#endif

#ifdef OMAP_H
#ifdef CHECKINCLUDES
#pragma message("omap_h")
#endif
#define OMAP_C
#include "omap.h"
#endif

#ifdef PRINTER_H
#ifdef CHECKINCLUDES
#pragma message("printer_h")
#endif
#define PRINTER_C
#include "print.h"
#endif

#ifdef HMAP_H
#ifdef CHECKINCLUDES
#pragma message("hmap_h")
#endif
#define HMAP_C
#include "hmap.h"
#endif

#ifdef STRING_LIST_H
#ifdef CHECKINCLUDES
#pragma message("string_list_h")
#endif
#define STRING_LIST_C
#include "stringList.h"
#endif

#ifdef MY_LIST_H
#ifdef CHECKINCLUDES
#pragma message("my_list_h")
#endif
#define MY_LIST_C
#include "my-list.h"
#endif
