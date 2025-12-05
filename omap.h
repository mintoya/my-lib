#ifndef OMAP_H
#define OMAP_H
#include "allocator.h"
#include "fptr.h"
#include "stdarg.h"
#include "stringList.h"
#include <malloc.h>
#include <stddef.h>

typedef enum : uint8_t {
  RAW = 0,
  SLIST,
  OMAP,
} OMap_Meta;
typedef struct {
  fptr k;
  fptr v;
  OMap_Meta t;
} OMap_both;
typedef struct {
  fptr v;
  OMap_Meta t;
} OMap_V;

#define OMap_Meta_RAW ((OMap_Meta)RAW)
#define OMap_Meta_SLIST ((OMap_Meta)SLIST)
#define OMap_Meta_OMAP ((OMap_Meta)OMAP)

typedef struct {
  stringList *KVs;
} OMap;

static inline fptr OMap_toView(const My_allocator *allocator, OMap *map) { return stringList_toView(allocator, map->KVs).raw; }

fptr OMap_getKey(const OMap *map, u32 index);
fptr OMap_getVal(const OMap *map, u32 index);
u32 OMap_length(const OMap *map);

OMap_V OMap_get(const OMap *map, fptr key);
OMap_both OMap_getBoth(const OMap *map, fptr key);
typedef fptr OMapView;
u32 OMap_set(OMap *map, fptr key, fptr val);

u32 OMap_setRaw(OMap *map, fptr key, fptr val);
fptr OMap_getRaw(const OMap *map, fptr key);

u32 OMap_setKind(OMap *map, fptr key, fptr val, OMap_Meta kind);

OMap *OMap_new(const My_allocator *allocator);

u32 OMap_binarySearch(const OMap *map, fptr key);
typedef struct {
  stringList_Solid KVs;
} OMap_solid;
static inline u32 OMapView_length(const OMapView omv) {
  return stringListView_length((stringListView){omv}) / 2;
}
OMap_both OMapView_get(OMapView mv, fptr key);
OMap_both OMapView_getIndex(OMapView mv, u32 index);
static inline OMap_solid OMap_fromView(const OMapView omv) { return (OMap_solid){stringList_fromView((stringListView){omv})}; }

extern inline size_t OMap_footprint(OMap *map);
extern inline void OMap_free(OMap *map);
extern inline void OMapView_free(const My_allocator *allocator, OMapView v);
extern inline void OMap_cleanupHandler(OMap **om);

// takes fptr*,OMap*,stringList*
typedef struct {
  void *ptr;
  OMap_Meta kind;
} OMap_ObjArg;
static inline OMap_ObjArg OMOJA(void *v, OMap_Meta m) {
  OMap_ObjArg res;
  res.ptr = v;
  res.kind = m;
  return res;
}

extern inline void OMap_setObj(OMap *map, fptr key, OMap_ObjArg val);
extern inline void StringList_setObj(stringList *list, u32 key, OMap_ObjArg val);
extern inline OMap_V Stringlist_getObj(stringList *list, u32 key);
extern inline OMap_V StringlistView_getObj(stringListView list, u32 key);
extern inline void StringList_appendObj(stringList *list, OMap_ObjArg val);

#define OMap_scoped [[gnu::cleanup(OMap_cleanupHandler)]] OMap

#endif // OMAP_H
#ifdef OMAP_C
inline fptr OMapfptr_inside(um_fp string) {
  char limits[2] = {'[', ']'};
  if (!string.width)
    return nullUmf;
  char front = limits[0];
  char back = limits[1];

  int in_single = 0;
  int in_double = 0;

  for (int i = 0; i < string.width; i++) {
    char c = ((char *)string.ptr)[i];

    // toggle quote state
    if (c == '"' && !in_single)
      in_double = !in_double;
    else if (c == '\'' && !in_double)
      in_single = !in_single;

    if (!in_single && !in_double && c == front) {
      u32 counter = 1;
      for (int ii = 1; ii + i < string.width; ii++) {
        c = ((char *)string.ptr)[i + ii];

        // toggle quote state inside the nested scan
        if (c == '"' && !in_single)
          in_double = !in_double;
        else if (c == '\'' && !in_double)
          in_single = !in_single;

        if (!in_single && !in_double) {
          if (c == front) {
            counter++;
          } else if (c == back) {
            counter--;
          }
        }

        if (!counter)
          return (um_fp){
              .width = (size_t)ii - 1,
              .ptr = ((uint8_t *)string.ptr) + i + 1,
          };
        // if (i + ii == string.width - 1)
        //   return (um_fp){.ptr = ((uint8_t *)string.ptr) + i + 1,
        //                  .width = (size_t)ii - 1};
        if (i + ii == string.width - 1)
          return (um_fp){
              .width = (size_t)ii,
              .ptr = ((uint8_t *)string.ptr) + i + 1,
          };
      }
    }
  }
  return nullUmf;
}
inline void OMap_setObj(OMap *map, fptr key, OMap_ObjArg val) {
  fptr fval;
  switch (val.kind) {
  case OMAP:
    fval =
        OMap_toView(NULL, (OMap *)val.ptr);
    break;
  case RAW:
    fval = *(fptr *)(val.ptr);
    break;
  case SLIST:
    fval = stringList_toView(NULL, (stringList *)val.ptr).raw;
    break;
  }

  u32 index = OMap_setKind(map, key, fval, val.kind);

  switch (val.kind) {
  case RAW:
    break;
  default:
    aFree(NULL, fval.ptr);
    break;
  }
}
inline void StringList_setObj(stringList *list, u32 key, OMap_ObjArg val) {
  fptr fval;
  switch (val.kind) {
  case OMAP:
    fval =
        OMap_toView(NULL, (OMap *)val.ptr);
    break;
  case RAW:
    fval = *(fptr *)val.ptr;
    break;
  case SLIST:
    fval = stringList_toView(NULL, (stringList *)val.ptr).raw;
    break;
  }
  fptr both;
  both.width = sizeof(OMap_Meta) + fval.width;
  uint8_t *dataBuffer = (uint8_t *)aAlloc(NULL, both.width);
  both.ptr = dataBuffer;
  *(OMap_Meta *)dataBuffer = val.kind;
  memcpy(dataBuffer + sizeof(OMap_Meta), fval.ptr, fval.width);
  stringList_set(list, both, key);
  aFree(NULL, both.ptr);
  switch (val.kind) {
  case RAW:
    break;
  default:
    aFree(NULL, fval.ptr);
    break;
  }
}
inline OMap_V Stringlist_getObj(stringList *list, u32 key) {
  fptr val = stringList_get(list, key);
  if (!val.ptr)
    return (OMap_V){
        .v = nullFptr,
        .t = RAW,
    };
  OMap_V res = (OMap_V){.t = *(OMap_Meta *)(val.ptr)};
  res.v = (fptr){
      .width = val.width - sizeof(OMap_Meta),
      .ptr = val.ptr + sizeof(OMap_Meta),
  };
  return res;
}
inline OMap_V StringlistView_getObj(stringListView list, u32 key) {
  fptr val = stringListView_get(list, key);
  if (!val.ptr)
    return (OMap_V){
        .v = nullFptr,
        .t = RAW,
    };
  OMap_V res = (OMap_V){.t = *(OMap_Meta *)(val.ptr)};
  res.v = (fptr){
      .width = val.width - sizeof(OMap_Meta),
      .ptr = val.ptr + sizeof(OMap_Meta),
  };
  return res;
}
inline void StringList_appendObj(stringList *list, OMap_ObjArg val) {
  fptr fval;
  switch (val.kind) {
  case OMAP:
    fval =
        OMap_toView(NULL, (OMap *)val.ptr);
    break;
  case RAW:
    fval = *(fptr *)val.ptr;
    break;
  case SLIST:
    fval = stringList_toView(NULL, (stringList *)val.ptr).raw;
    break;
  }
  fptr both;
  both.width = sizeof(OMap_Meta) + fval.width;
  uint8_t *dataBuffer = (uint8_t *)aAlloc(NULL, both.width);
  both.ptr = dataBuffer;
  *(OMap_Meta *)dataBuffer = val.kind;
  memcpy(dataBuffer + sizeof(OMap_Meta), fval.ptr, fval.width);
  stringList_append(list, both);
  aFree(NULL, both.ptr);
  switch (val.kind) {
  case RAW:
    break;
  default:
    aFree(NULL, fval.ptr);
    break;
  }
}
inline size_t OMap_footprint(OMap *map) { return stringList_footprint(map->KVs); }
inline void OMap_free(OMap *map) {
  const My_allocator *allocator = map->KVs->List_char.allocator;
  stringList_free(map->KVs);
  aFree(allocator, map);
}
inline void OMapView_free(const My_allocator *allocator, OMapView v) { aFree(allocator, v.ptr); }
inline void OMap_cleanupHandler(OMap **om) {
  if (om && *om) {
    OMap_free(*om);
    *om = NULL;
  }
}
#define OMap_getM(map, ...) OMap_getL(map, APPLY_N(fp_from, __VA_ARGS__), nullFptr)
static inline OMap_V OMap_getL(const OMap *map, fptr firstkey, ...) {
  va_list vl;
  if (!firstkey.ptr)
    return (OMap_V){nullFptr, RAW};

  va_start(vl, firstkey);
  OMap_V place = OMap_get(map, firstkey);

  fptr key = va_arg(vl, fptr);
  while (place.t != RAW && key.ptr) {
    switch (place.t) {
    case OMAP: {
      OMap_both t = OMapView_get(place.v, key);
      place.v = t.v;
      place.t = t.t;
    } break;
    case SLIST: {
      fptr numver = OMapfptr_inside(key);
      if (!numver.ptr) {
        va_end(vl);
        return (OMap_V){nullFptr, RAW};
      }
      place = StringlistView_getObj((stringListView){place.v}, fptr_toUint(numver));
    } break;
    default:
      break;
    }

    key = va_arg(vl, fptr);
  }

  va_end(vl);
  return place;
}
static inline OMap_V OMap_getLA(const OMap *map, uint nargs, fptr *args) {
  if (!nargs)
    return (OMap_V){nullFptr, RAW};

  OMap_V place = OMap_get(map, args[0]);

  // Use pointer arithmetic to avoid bounds issues
  fptr *current_arg = args + 1; // Start from second argument
  uint args_remaining = nargs - 1;

  while (args_remaining > 0 && place.t != RAW) {
    fptr key = *current_arg;

    switch (place.t) {
    case OMAP: {
      OMap_both t = OMapView_get(place.v, key);
      place.v = t.v;
      place.t = t.t;
    } break;
    case SLIST: {
      fptr numver = OMapfptr_inside(key);
      if (!numver.ptr) {
        return (OMap_V){nullFptr, RAW};
      }
      place = StringlistView_getObj((stringListView){place.v}, fptr_toUint(numver));
    } break;
    default:
      break;
    }

    current_arg++;
    args_remaining--;
  }

  return place;
}
fptr OMap_getKey(const OMap *map, u32 index) { return stringList_get(map->KVs, index * 2); }
fptr OMap_getVal(const OMap *map, u32 index) { return stringList_get(map->KVs, index * 2 + 1); }
u32 OMap_length(const OMap *map) { return stringList_length(map->KVs) / 2; }
OMap *OMap_new(const My_allocator *allocator) {
  OMap *res = (OMap *)aAlloc(allocator, sizeof(OMap));
  res->KVs = stringList_new(allocator);
  return res;
}
u32 OMap_binarySearch(const OMap *map, fptr key) {
  u32 region = OMap_length(map);
  u32 front = 0;
  while (region > 2) {
    char less = fptr_cmp(key, OMap_getKey(map, front + region / 2)) > 0;
    region /= 2;
    if (less)
      front += region;
  }
  u32 res = front;
  while (res < OMap_length(map) &&
         fptr_cmp(key, OMap_getKey(map, res)) > 0) {
    res++;
  }
  return res;
}
fptr OMap_getRaw(const OMap *map, fptr key) {
  u32 index = OMap_binarySearch(map, key);
  if (index > OMap_length(map))
    return nullFptr;
  return OMap_getVal(map, index);
};
OMap_V OMap_get(const OMap *map, fptr key) {
  fptr vall = OMap_getRaw(map, key);
  fptr res = (fptr){vall.width - sizeof(OMap_Meta), vall.ptr + sizeof(OMap_Meta)};
  OMap_Meta kind;
  setvar_aligned(kind, vall.ptr);
  return (OMap_V){res, kind};
}
OMap_both OMap_getBoth(const OMap *map, fptr key) {
  u32 index = OMap_binarySearch(map, key);
  if (index < OMap_length(map)) {
    fptr vall = OMap_getVal(map, index);
    fptr res = (fptr){vall.width - sizeof(OMap_Meta), vall.ptr + sizeof(OMap_Meta)};
    return (OMap_both){OMap_getKey(map, index), res, *(OMap_Meta *)(vall.ptr)};
  } else
    return (OMap_both){nullFptr, nullFptr, RAW};
}
#define STACKMAX (sizeof(intmax_t) * 5)
u32 OMap_setRaw(OMap *map, fptr key, fptr val) {
  u32 res = OMap_binarySearch(map, key);
  if (fptr_eq(key, OMap_getKey(map, res))) {
    stringList_set(map->KVs, val, res * 2 + 1);
  } else {
    stringList_insert(map->KVs, val, res * 2);
    stringList_insert(map->KVs, key, res * 2);
  }
  return res;
}
u32 OMap_set(OMap *map, fptr key, fptr val) {
  u32 index = OMap_binarySearch(map, key);
  fptr both = (fptr){
      .width = sizeof(OMap_Meta) + val.width
  };
  OMap_Meta *dataBuffer;
  if (both.width < STACKMAX) {
    dataBuffer = (OMap_Meta *)({
      uintptr_t newptr = (uintptr_t)alloca(alignof(OMap_Meta) + sizeof(OMap_Meta) + STACKMAX);
      newptr += (alignof(OMap_Meta) - (newptr % alignof(OMap_Meta))) % alignof(OMap_Meta);
      (OMap_Meta *)newptr;
    });
  } else {
    dataBuffer = (OMap_Meta *)aAlloc(map->KVs->List_char.allocator, both.width);
  }

  *dataBuffer = OMap_Meta_RAW;
  memcpy(dataBuffer + sizeof(OMap_Meta), val.ptr, val.width);
  both.ptr = (u8 *)dataBuffer;

  OMap_setRaw(map, key, both);

  if (both.width < STACKMAX) {
    //
  } else {
    aFree(map->KVs->List_char.allocator, both.ptr);
  }
  return index;
}
u32 OMap_setKind(OMap *map, fptr key, fptr val, OMap_Meta kind) {
  u32 index = OMap_binarySearch(map, key);
  fptr both = (fptr){
      .width = sizeof(OMap_Meta) + val.width
  };
  uint8_t *dataBuffer;
  if (both.width > STACKMAX) {
    dataBuffer = (uint8_t *)alloca(both.width);
  } else {
    dataBuffer = (uint8_t *)aAlloc(map->KVs->List_char.allocator, both.width);
  }
  *(OMap_Meta *)dataBuffer = kind;
  memcpy(dataBuffer + sizeof(OMap_Meta), val.ptr, val.width);
  both.ptr = dataBuffer;
  if (fptr_eq(key, OMap_getKey(map, index))) {
    stringList_set(map->KVs, both, index * 2 + 1);
  } else {
    stringList_insert(map->KVs, both, index * 2);
    stringList_insert(map->KVs, key, index * 2);
  }
  if (both.width < STACKMAX) {
    aFree(map->KVs->List_char.allocator, both.ptr);
  }
  return index;
}
OMap_both OMapView_getIndex(OMapView mv, u32 index) {
  u32 length = OMapView_length(mv);
  OMap_solid map = OMap_fromView(mv);
  if (index > length)
    return (OMap_both){nullFptr, nullFptr, RAW};
  stringMetaData thisKey;
  memcpy(&thisKey, map.KVs.Arr_stringMetaData + (index * 2) * sizeof(stringMetaData), sizeof(stringMetaData));
  stringMetaData thisVal;
  memcpy(&thisVal, map.KVs.Arr_stringMetaData + (index * 2 + 1) * sizeof(stringMetaData), sizeof(stringMetaData));
  fptr vall = (fptr){thisVal.width, map.KVs.Arr_char + thisVal.index};

  fptr key = (fptr){thisKey.width, map.KVs.Arr_char + thisKey.index};
  fptr val = (fptr){vall.width - sizeof(OMap_Meta), vall.ptr + sizeof(OMap_Meta)};
  OMap_Meta ttype = *(OMap_Meta *)(vall.ptr);
  return (OMap_both){
      .k = key,
      .v = val,
      .t = ttype,
  };
}
OMap_both OMapView_get(OMapView mv, fptr key) {
  u32 region = OMapView_length(mv);
  u32 front = 0;
  while (region > 2) {
    char less = fptr_cmp(key, OMapView_getIndex(mv, front + region / 2).k) > 0;
    region /= 2;
    if (less)
      front += region;
  }
  u32 res = front;
  while (res < OMapView_length(mv) &&
         fptr_cmp(key, OMapView_getIndex(mv, res).k) > 0) {
    res++;
  }
  return OMapView_getIndex(mv, res);
}
#undef STACKMAX
#endif // OMAP_C
