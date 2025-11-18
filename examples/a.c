#include "../arenaAllocator.h"
#include "../boxer.h"
#include "../fptr.h"
#include "../print.h"
#include "../wheels.h"
#include <stddef.h>
#include <string.h>

struct example {
  int integer;
  float floating;
  size_t nN;
  int *N;
  char character;
};
boxer examleBoxer[5] = {
    {.type = KNOWN, .size = sizeof(int)},
    {.type = KNOWN, .size = sizeof(float)},
    {.type = KNOWN, .size = sizeof(size_t)},
    {.type = FPTR},
    {.type = KNOWN, .size = sizeof(char)},
};
int main(void) {
  Arena_scoped *local = arena_new(1000);
  int ia[2] = {6, 7};
  struct example e = {67, 6.7, 2, ia, '6'};
  fptr intsptr = (fptr){2 * sizeof(int), (uint8_t *)&(e.N)};
  // clang-format off
  fptr n = enBox(local, 5, 
     examleBoxer,
     (void *[5]){
       &(e.integer),
       &(e.floating),
       &(e.nN),
       &(intsptr),
       &(e.character),
     }
  );
  fptr i2;
  struct example e2;
  unBox(5,
      examleBoxer,
     (void *[5]){
       &(e2.integer),
       &(e2.floating),
       &(e2.nN),
       &(i2),
       &(e2.character),
     },n);
  println("${fptr<void>:length}",n);
  // clang-format on
  println("real size \t: ${}\n"
          "stored size \t: ${}",
          sizeof(struct example), n.width);
  println("e1: ${},${},${}", e.integer, e.floating, (int)e.character);
  println("e2: ${},${},${}", e2.integer, e2.floating, (int)e2.character);
  println("${}", (int)fptr_eq(i2, intsptr));

  return 0;
}
