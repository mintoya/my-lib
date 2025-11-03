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

#include "printer/genericName.h"
MAKE_PRINT_ARG_TYPE(point);

int main() {
  MList(point) points;
  MList_init(points);
  MList_push(points, ((point){0, 0}));
  MList_push(points, ((point){1, 0}));
  MList_push(points, ((point){1, 1}));
  MList_push(points, ((point){0, 1}));
  println("${}", points.elements[0]);
  println("${}", points.elements[1]);
  println("${}", points.elements[2]);
  println("${}", points.elements[3]);
  return 0;
}
