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

  return 0;
}
