#include <stdio.h>
#define PRINTER_LIST_TYPENAMES
#include "../allocator.h"
#include "../kmlM.h"
#include "../macroList.h"
#include "../my-list.h"
#include "../print.h"
#include "../umap.h"
#include "../wheels.h"
List *buffer = NULL;

__attribute__((destructor)) void freeList(void) { List_free(buffer); }
__attribute__((constructor)) void setupList(void) {
  buffer = List_new(&defaultAllocator, sizeof(char));
}

void listPrinter(
    const char *c,
    void *arb,
    unsigned int length,
    char flush
) {
  (void)arb;
  MList(char) list;
  MList_DF(list, buffer);
  MList_addArr(list, length, c);
}
static inline fptr um_flist(List *l) {
  return (um_fp){
      .width = List_headArea(l),
      .ptr = l->head,
  };
}
void formattedOutputFunction(
    const char *data,
    void *arb,
    unsigned int length,
    char flush
) {
  static uint indentLevel = 0;

  for (size_t index = 0; index < length; index++) {
    char character = data[index];
    {
      switch (character) {
      case '{':
      case '[': {
        putchar(character);
        putchar('\n');
        indentLevel++;
        for (int i = 0; i < indentLevel; i++) {
          putchar(' ');
        }
      } break;
      case '}':
      case ']': {
        putchar('\033');
        putchar('[');
        putchar('1');
        putchar('D');
        putchar(character);
        putchar('\n');
        indentLevel--;
        for (int i = 0; i < indentLevel; i++) {
          putchar(' ');
        }
      } break;
      case ';':
      case ',': {
        putchar(character);
        putchar('\n');
        for (int i = 0; i < indentLevel; i++) {
          putchar(' ');
        }
      } break;
      default:
        putchar(character);
      }
    }
  }
  if (flush)
    indentLevel = 0;
}

int main() {
  UMap_scoped *test = UMap_new(&defaultAllocator);
  UMap_set(test, fp_from("a"), fp_from("b"));
  UMap_set(test, fp_from("b"), fp_from("c"));
  UMap_set(test, fp_from("c"), fp_from("a"));
  UMap_setChild(test, fp_from("object"), test);
  UMapList_scoped *tlist = UMapList_new(&defaultAllocator);
  UMapList_append(tlist, fp_from("one"));
  UMapList_append(tlist, fp_from("two"));
  UMapList_append(tlist, fp_from("three"));
  UMap_setList(test, fp_from("list"), tlist);

  print_wf(listPrinter, 0, "${UMap}", *test);
  UMap_scoped *output = parse(NULL, NULL, um_flist(buffer));
  println("${UMap}", *output);
  print_wf(formattedOutputFunction, 0, "${UMap}", *output);
}
