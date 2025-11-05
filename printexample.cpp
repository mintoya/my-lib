#define PRINTER_LIST_TYPENAMES
#include "my-list.hpp"
#include "print.h"
#include "wheels.h"
struct point {
  int x;
  int y;
};
REGISTER_PRINTER(point, {
  PUTS("{x:", 3);
  USETYPEPRINTER(int, in.x);
  PUTS(",", 1);
  PUTS("y:", 2);
  USETYPEPRINTER(int, in.y);
  PUTS("}", 1);
})

#include "printer/genericName.h" // unnececary in cpp
MAKE_PRINT_ARG_TYPE(point);

int main() {
  listPlus<point> points;
  // points.dontfree(); 
  points.push((point){0, 0});
  points.push((point){1, 1});
  points.push((point){0, 1});
  points.insert((point){1, 0}, 1);
  println("${}", points.get(2));
  println("${}", points.pop());
  println("${}", points.pop());
  points.foreach ([](point i) { println("foreach : ${}", i); });
  println("length  : ${int}\n"
          "capacity: ${int}",
          points.length(), points.capacity());
  unsigned int i;
  println("no type test: ${}", i);
  println("missing parameter test: ${}");
  return 0;
}
