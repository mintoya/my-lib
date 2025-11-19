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

int main(void) {
  Arena_scoped *local = arena_new(1000);
  UMap_scoped *um = UMap_new(local);
  UMap_set(um, fp_from("a"), fp_from("b"));
  UMap_set(um, fp_from("b"), fp_from("c"));
  UMap_set(um, fp_from("c"), fp_from("a"));
  UMap_set(um, fp_from("key"), fp_from("value"));
  UMap_setChild(um, fp_from("map"), um); // stores no referances ever(besides a litteral number that is a pointer)
  UMapView testView = UMap_toBuf(local, um);
  UMapView newView = UMap_toView(local, um);
  println("${}\n"
          "${fptr<void>:length}\n"
          "\n"
          "${fptr<void>:length}",
          UMap_footprint(um), testView, newView);
  println("${UMap}", *um);
  println("${}, ${}", sizeof(UMap_innertype), um->metadata->length);
  println("keycount in view: ${}", UMapView_keyCount(newView));

  return 0;
}
