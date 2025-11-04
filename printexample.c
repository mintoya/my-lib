#define PRINTER_LIST_TYPENAMES
#include "macroList.h"
#include "print.h"
#include "wheels.h"
typedef struct {
  int x;
  int y;
} point;
REGISTER_PRINTER(point, {
  put("{x:", 3);
  USETYPEPRINTER(int, in.x);
  put(",", 1);
  put("y:", 2);
  USETYPEPRINTER(int, in.y);
  put("}", 1);
})

#include "printer/genericName.h" // advances typenaems
MAKE_PRINT_ARG_TYPE(point);

int main() {
  MList(point) points;
  MList_init(points);
  MList_push(points, ((point){0, 0}));
  MList_push(points, ((point){1, 1}));
  MList_push(points, ((point){0, 1}));
  MList_insert(points, 1, ((point){1, 0}));
  println("${}", points.elements[2]);
  println("${}", MList_pop(points));
  println("${}", MList_pop(points));
  MList_foreach(points, i, { println("foreach : ${}", i); });
  println("length  : ${int}\n"
          "capacity: ${int}",
          points.length, MList_capacity(points));
  return 0;
}
