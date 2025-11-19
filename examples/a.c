#include "../arenaAllocator.h"
#include "../fptr.h"
#include "../print.h"
#include "../stringList.h"
#include "../umap.h"
#include "../wheels.h"
#include <stddef.h>
#include <stdint.h>

// meta,keys,vals
Boxer UmapBoxer = {3, {{FPTR}, {FPTR}, {FPTR}}};
UMapView UMap_toView(const My_allocator *allocator, UMap *map) {
  stringListView keys = stringList_toView(allocator, map->keys);
  stringListView vals = stringList_toView(allocator, map->vals);
  fptr meta = ((fptr){List_headArea(map->metadata), map->metadata->head});
  fptr res = enBox(allocator, &UmapBoxer, (void *[3]){&(meta), &(keys), &(vals)});
  stringListView_free(allocator, keys);
  stringListView_free(allocator, vals);
  return res;
}
typedef struct {
  stringList_Solid keys;
  stringList_Solid vals;
  UMap_innertype *meta;
} UMap_Solid;

UMap_Solid UMap_fromView(UMapView umv) {
  bFptr *keys, *vals, *meta;
  unBox(&UmapBoxer, (void *[3]){&(meta), &(keys), &(vals)}, umv);
  return (UMap_Solid){
      .keys = stringList_fromView((stringListView){fptr_fromB(*keys)}),
      .vals = stringList_fromView((stringListView){fptr_fromB(*vals)}),
      .meta = (UMap_innertype *)(meta->ptr),
  };
}
unsigned int UMapView_keyCount(UMapView umv) {
  size_t fs = *(size_t *)(umv.ptr);
  return fs / sizeof(UMap_innertype);
}

const Boxer UmapListBoxer = {2, {{FPTR}, {FPTR}}};
UMapListView UMapList_toView(const My_allocator *allocator, UMapList *map) {
  fptr meta = (fptr){List_headArea(map->metadata), map->metadata->head};
  stringListView slv = stringList_toView(allocator, map->vals);
  fptr res = enBox(allocator, &UmapListBoxer, (void *[2]){&(meta), &(slv)});
  stringListView_free(allocator, slv);
  return res;
}
typedef struct {
  UMap_innertype *meta;
  stringList_Solid list;
} UMapList_Solid;
UMapList_Solid UMapList_fromView(UMapListView ulv) {
  bFptr *list, *meta;
  unBox(&UmapListBoxer, (void *[2]){&(meta), &(list)}, ulv);
  return (UMapList_Solid){
      .meta = (UMap_innertype *)(meta->ptr),
      .list = stringList_fromView((stringListView){fptr_fromB(*list)}),
  };
}
unsigned int UMapListView_count(UMapListView ulv) {
  size_t fs = *(size_t *)(ulv.ptr);
  return fs / sizeof(UMap_innertype);
}

int main(void) {
  Arena_scoped *local = arena_new(1000);
  UMapList_scoped *um = UMapList_new(local);
  UMapList_append(um, fp_from("a"));
  UMapList_append(um, fp_from("b"));
  UMapList_append(um, fp_from("c"));
  UMapListView testView = UMapList_toBuf(local, um);
  UMapListView newView = UMapList_toView(local, um);
  println("${},${}", testView.width, newView.width);
  // println("${UMapList}", *um);
  println("${}, ${}", sizeof(UMap_innertype), um->metadata->length);

  UMapList_Solid s = UMapList_fromView(newView);
  println("keycount in view: ${}", UMapView_keyCount(newView));
  for (int i = 0; i < UMapView_keyCount(newView); i++) {
    fptr t;
    t.ptr = s.list.Arr_char + s.list.Arr_stringMetaData[i].index;
    t.width = s.list.Arr_stringMetaData[i].width;
    println("${}", t);
  }

  return 0;
}
