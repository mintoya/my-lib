#include "my-list.hpp"
#include <stdio.h>
#define MY_LIST_C
#include "my-list.h"

int main () {
  
  listPlus<int> a;
  a.append(5);
  a.insert(8, 0);
  printf("%i\n",a.get(0));

  return 0;
}
