#include "../fptr.h"
#include "../kmlM.h"
#include "../macroList.h"
#include "../omap.h"
#include "../print.h"
#include "../printers/omap_printers.c"
#include "../wheels.h"
#include <stdio.h>
fptr read_stdin() {
  List *lptr;
  MList(char) chars;
  MList_DFInit(chars, &defaultAllocator, lptr);
  int c;
  while ((c = fgetc(stdin)) != EOF)
    MList_push(chars, (char)c);
  aFree(&defaultAllocator, lptr);
  return MList_fp(chars);
}

int main(int nargs, char *args[nargs]) {
  fptr stin = read_stdin();

  OMap_scoped *map = parse(NULL, NULL, stin);

  fptr argsf[nargs];
  for (uint i = 0; i < nargs; i++) {
    argsf[i] = ((fptr){
        .width = strlen(args[i]),
        .ptr = (u8 *)args[i],
    });
  }
  println("${OMap_V}", OMap_getLA(map, nargs - 1, argsf + 1));

  aFree(&defaultAllocator, stin.ptr);
  return 0;
}
