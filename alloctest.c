#include "allocator.h"
#include "hmap.h"
#include "macroList.h"
#include "print.h"
#include "umap.h"
#include "wheels.h"
#include <stddef.h>
#include <stdint.h>

List *list_allocator_buffer = NULL;
void list_allocator_init() {
  list_allocator_buffer = mList(uint8_t);
  List_resize(list_allocator_buffer, 2048);
}
void *list_allocator_alloc(const struct My_allocator *unused, size_t size) {
  println("//alloc");
  uint8_t *place =
      List_getRefForce(list_allocator_buffer, list_allocator_buffer->length);
  List_appendFromArr(list_allocator_buffer, &size, sizeof(size_t));
  List_pad(list_allocator_buffer, size);
  return place + sizeof(size_t);
}
void list_allocator_free(const struct My_allocator *unused, void *what) {
  println("//free");
}
void *list_allocator_realloc(const struct My_allocator *unused, void *what,
                             size_t newsize) {
  println("//realloc");
  size_t size = *(size_t *)(((uint8_t *)what) - sizeof(size_t));
  uint8_t *place =
      List_getRefForce(list_allocator_buffer, list_allocator_buffer->length);
  List_appendFromArr(list_allocator_buffer, &newsize, sizeof(size_t));
  List_appendFromArr(list_allocator_buffer, what, size);
  if (size < newsize) {
    List_pad(list_allocator_buffer, newsize - size);
  }
  return place + sizeof(size_t);
}
My_allocator list_allocator = {
    .alloc = list_allocator_alloc,
    .r_alloc = list_allocator_realloc,
    .free = list_allocator_free,
};
int main(void) {
  list_allocator_init();

  List_scoped *hlist = List_new(&list_allocator, sizeof(intmax_t));
  MList_DF(list, intmax_t, hlist);

  MList_push(list, 5);
  MList_push(list, 8);
  MList_push(list, 7);
  MList_push(list, 9);
  MList_push(list, 5);
  MList_push(list, 8);
  MList_push(list, 7);
  MList_push(list, 9);
  MList_foreach(list, index, element, { println("${}", (int)element); });

  HMap_scoped *hm = HMap_new(&list_allocator, 10);
  HMap_set(hm, um_from("hello"), um_from("world"));
  HMap_set(hm, um_from("world"), um_from("hello"));
  println("${}", HMap_get(hm, um_from("hello")));
  UMap_scoped *um = UMap_new(&list_allocator);
  UMap_set(um, um_from("hello"), um_from("world"));
  UMap_set(um, um_from("world"), um_from("hello"));
  println("${}", UMap_get(um, um_from("hello")));
  println("the buffer is ${} bytes", (size_t)(list_allocator_buffer->length));
  println("sized at      ${} bytes", (size_t)(list_allocator_buffer->size));
  List_free(list_allocator_buffer);
  return 0;
}
