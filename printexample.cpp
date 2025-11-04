#define PRINTER_LIST_TYPENAMES
#include "my-list.hpp"
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
  listPlus<point> points;

  points.push((point){0, 0});
  points.push((point){1, 1});
  points.push((point){0, 1});
  points.insert((point){1, 0}, 1);
  println("${}", points.get(2));
  println("${}", points.pop());
  println("${}", points.pop());
  println("${}", points.pop());
  println("${}", points.pop());
  println("length  : ${int}\n"
          "capacity: ${int}",
          (int)points.length(),
          (int)points.capacity());
  return 0;
}
