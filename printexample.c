#define PRINTER_LIST_TYPENAMES
#include "macroList.h"
#include "print.h"
#include "wheels.h"
typedef struct {
  int x;
  int y;
} point;
REGISTER_PRINTER(point, {
  PUTS("{x:", 3);
  USETYPEPRINTER(int, in.x);
  PUTS(",", 1);
  PUTS("y:", 2);
  USETYPEPRINTER(int, in.y);
  PUTS("}", 1);
})

#include "printer/genericName.h" // advances typenaems
MAKE_PRINT_ARG_TYPE(point);

MList_typeDef(point);

int main() {
  MList(point) points;
  MList_init(points);
  MList_push(points, ((point){0, 0}));
  MList_push(points, ((point){1, 1}));
  MList_push(points, ((point){0, 1}));
  MList_foreach(points, _i, i, { println("${}", i); });
  println("length  : ${int}\n"
          "capacity: ${int}",
          points.length, MList_capacity(points));
  unsigned int i;

  println("no type test: ${}", i);
  println("missing parameter test: ${}");
  return 0;
}
