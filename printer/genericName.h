
#ifndef GENERICS_H
#define GENERICS_H
 #define GENERIC_TYPE_1
 #define GENERIC_TYPE_2
 #define GENERIC_TYPE_3
 #define GENERIC_TYPE_4
 #define GENERIC_TYPE_5
 #define GENERIC_TYPE_6
 #define GENERIC_TYPE_7
 #define GENERIC_TYPE_8
 #define GENERIC_TYPE_9
 #define GENERIC_TYPE_10
#endif //GENERICS_H
#define GENERIC_ELEMENT(type) type:#type,
#define GENERIC_NAME(val) \
_Generic((val), \
 GENERIC_TYPE_1 \
 GENERIC_TYPE_2 \
 GENERIC_TYPE_3 \
 GENERIC_TYPE_4 \
 GENERIC_TYPE_5 \
 GENERIC_TYPE_6 \
 GENERIC_TYPE_7 \
 GENERIC_TYPE_8 \
 GENERIC_TYPE_9 \
 GENERIC_TYPE_10 \
default:"")
#define EXPAND_(x) x
#define EXPAND(x) EXPAND_(x)
#if GENERIC_NAME_N == 0
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 1
 #elif GENERIC_NAME_N == 1
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 2
 #elif GENERIC_NAME_N == 2
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 3
 #elif GENERIC_NAME_N == 3
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 4
 #elif GENERIC_NAME_N == 4
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 5
 #elif GENERIC_NAME_N == 5
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 6
 #elif GENERIC_NAME_N == 6
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 7
 #elif GENERIC_NAME_N == 7
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 8
 #elif GENERIC_NAME_N == 8
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 9
 #elif GENERIC_NAME_N == 9
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 10
 #elif GENERIC_NAME_N == 10
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 11
#endif
#if GENERIC_NAME_N == 1
#define GENERIC_TYPE_1 GENERIC_ELEMENT(argtype1)
#elif GENERIC_NAME_N == 2
#define GENERIC_TYPE_2 GENERIC_ELEMENT(argtype2)
#elif GENERIC_NAME_N == 3
#define GENERIC_TYPE_3 GENERIC_ELEMENT(argtype3)
#elif GENERIC_NAME_N == 4
#define GENERIC_TYPE_4 GENERIC_ELEMENT(argtype4)
#elif GENERIC_NAME_N == 5
#define GENERIC_TYPE_5 GENERIC_ELEMENT(argtype5)
#elif GENERIC_NAME_N == 6
#define GENERIC_TYPE_6 GENERIC_ELEMENT(argtype6)
#elif GENERIC_NAME_N == 7
#define GENERIC_TYPE_7 GENERIC_ELEMENT(argtype7)
#elif GENERIC_NAME_N == 8
#define GENERIC_TYPE_8 GENERIC_ELEMENT(argtype8)
#elif GENERIC_NAME_N == 9
#define GENERIC_TYPE_9 GENERIC_ELEMENT(argtype9)
#elif GENERIC_NAME_N == 10
#define GENERIC_TYPE_10 GENERIC_ELEMENT(argtype10)
#endif
#define GEN_CAT_(a, b) a##b
#define GEN_CAT(a, b) GEN_CAT_(a, b)
#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)
#define MAKE_NEW_TYPE_HELPER(type, newtype)                                    \
  typedef type newtype;                                                        \
  __attribute__((constructor(201))) static void GEN_CAT(register_,newtype)() { \
    um_fp key = (um_fp){.ptr = (uint8_t *)EXPAND_AND_STRINGIFY(newtype),       \
                        .width = sizeof(EXPAND_AND_STRINGIFY(newtype)) - 1};   \
    uint8_t *refFn =                                                           \
        (uint8_t *)(void *)REF(printerFunction, GETTYPEPRINTERFN(type));       \
    stringList_append(typeNamesList, key);                                     \
    List_append(printerFunctions, refFn);                                      \
  }
#define MAKE_NEW_TYPE(type)                                                    \
  MAKE_NEW_TYPE_HELPER(type, GEN_CAT(argtype, GENERIC_NAME_N))