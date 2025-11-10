#define PRINTER_LIST_TYPENAMES
#include "my-list.hpp"
#include "print.h"
#include "wheels.h"
typedef struct {
  float x;
  float y;
} point;
REGISTER_PRINTER(point, {
  PUTS("{x:", 3);
  USETYPEPRINTER(float, in.x);
  PUTS(",", 1);
  PUTS("y:", 2);
  USETYPEPRINTER(float, in.y);
  PUTS("}", 1);
})

#include "printer/genericName.h" // unnececary in cpp
MAKE_PRINT_ARG_TYPE(point);

int main() {
  listPlus<point> points;
  // points.dontfree();
  points.push((point){0, 0});
  points.push((point){1.983, 1});
  points.push((point){0, 1.38});
  points.insert((point){1, 0}, 1);
  println("${}", points.get(2));
  println("${}", points.pop());
  points.foreach ([](int _i, point i) { println("foreach : ${}", i); });
  println("length  : ${int}\n"
          "capacity: ${int}",
          points.length(), points.capacity());
  unsigned int i;
  println("no type test : ${}", i);
  println("no type test2: ${thingy}", i);
  println("missing parameter test: ${}");
  return 0;
}
