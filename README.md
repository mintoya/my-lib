# Some wheels im reinventing

 ## um_fp.h
a pointer + a length, used as both a generic pointer and 
a "slice" in this library
 ## my-List & macroList .h
 my-list.h is a basic dynamic list implementation
 my-list.hpp is a cpp wrapper 
 macroList.h is an attempt to make using it less annoying in c
 ## stringList.h
 metadata array + char array for array of arbitrary size elements 
 ## cSum.h
 basic checksum written for my keyboard project
 ## umap.h
 ordered map with stringlist and list as the backed, capable of holding copies
 of other umaps with the internal type
 ## hmap.h
 hash map with linked-list style collision resolution
 linked list elements are in a normal list buffer
 same type metadta from umap
 ## kml & kmlM .h
 custom object notation format
 kml is a parser that returns a slice of that input based on the parsing args
 kmlM is a version that turns a um_fp character array into its representation
 as a umap, printing it is also meant to serve as converting it the other way
 ## print.h
 a *lot* of macros that make printing easier ( hopefully )
 based on u/jacksaccountonreddit's [better c generics](https://github.com/JacksonAllan/CC/blob/main/articles/Better_C_Generics_Part_1_The_Extendible_Generic.md) 
 ```c
    #include "print.h"
    #include "wheels.h"
    // print, println, print_wf, and println_wf macros
    // print_wf takes an outputFunction
    typedef void (*outputFunction)(char *, unsigned int length, char flush);

    typedef struct {
      int x;
      int y;
    } point;
    REGISTER_PRINTER(point, {
      PUTS("{x:", 3);
      USETYPEPRINTER(int, in.x); // use already registered printer
      PUTS(",", 1);
      PUTS("y:", 2);
      USETYPEPRINTER(int, in.y);
      PUTS("}", 1);
    })
    // now you can call this with
    print("${point}",((point){0,0}));

    #include "printer/genericName.h" // advances counter
    MAKE_PRINT_ARG_TYPE(point);
    // will create a typedef in the generic list
    // that matches point
    // now you can call it with
    print("${}",((point){0,0}));
 ```
 ## wheels.h
 this library was supposed to contain single header libraries, however,
 some structures depend on eachother, which means the implementation macros
 need to be called in order, this would be really annoying so wheels.h 
 just ensures that all the libraries have the implementatoins called in order
``` c
    #include "wheels/umap.h"
    #include "wheels/stringList.h"
    #include "wheels/wheels.h"
    // umap is dependant on both stringlist and my-list
    // stringlist is dependant on my-list 
    // wheels will implement in this order
    //  umap -> stringlist  -> my-list 
```

