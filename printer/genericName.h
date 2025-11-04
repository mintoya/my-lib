
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
 #define GENERIC_TYPE_11
 #define GENERIC_TYPE_12
 #define GENERIC_TYPE_13
 #define GENERIC_TYPE_14
 #define GENERIC_TYPE_15
 #define GENERIC_TYPE_16
 #define GENERIC_TYPE_17
 #define GENERIC_TYPE_18
 #define GENERIC_TYPE_19
 #define GENERIC_TYPE_20
 #define GENERIC_TYPE_21
 #define GENERIC_TYPE_22
 #define GENERIC_TYPE_23
 #define GENERIC_TYPE_24
 #define GENERIC_TYPE_25
 #define GENERIC_TYPE_26
 #define GENERIC_TYPE_27
 #define GENERIC_TYPE_28
 #define GENERIC_TYPE_29
 #define GENERIC_TYPE_30
 #define GENERIC_TYPE_31
 #define GENERIC_TYPE_32
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
 GENERIC_TYPE_11 \
 GENERIC_TYPE_12 \
 GENERIC_TYPE_13 \
 GENERIC_TYPE_14 \
 GENERIC_TYPE_15 \
 GENERIC_TYPE_16 \
 GENERIC_TYPE_17 \
 GENERIC_TYPE_18 \
 GENERIC_TYPE_19 \
 GENERIC_TYPE_20 \
 GENERIC_TYPE_21 \
 GENERIC_TYPE_22 \
 GENERIC_TYPE_23 \
 GENERIC_TYPE_24 \
 GENERIC_TYPE_25 \
 GENERIC_TYPE_26 \
 GENERIC_TYPE_27 \
 GENERIC_TYPE_28 \
 GENERIC_TYPE_29 \
 GENERIC_TYPE_30 \
 GENERIC_TYPE_31 \
 GENERIC_TYPE_32 \
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
 #elif GENERIC_NAME_N == 11
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 12
 #elif GENERIC_NAME_N == 12
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 13
 #elif GENERIC_NAME_N == 13
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 14
 #elif GENERIC_NAME_N == 14
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 15
 #elif GENERIC_NAME_N == 15
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 16
 #elif GENERIC_NAME_N == 16
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 17
 #elif GENERIC_NAME_N == 17
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 18
 #elif GENERIC_NAME_N == 18
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 19
 #elif GENERIC_NAME_N == 19
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 20
 #elif GENERIC_NAME_N == 20
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 21
 #elif GENERIC_NAME_N == 21
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 22
 #elif GENERIC_NAME_N == 22
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 23
 #elif GENERIC_NAME_N == 23
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 24
 #elif GENERIC_NAME_N == 24
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 25
 #elif GENERIC_NAME_N == 25
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 26
 #elif GENERIC_NAME_N == 26
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 27
 #elif GENERIC_NAME_N == 27
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 28
 #elif GENERIC_NAME_N == 28
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 29
 #elif GENERIC_NAME_N == 29
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 30
 #elif GENERIC_NAME_N == 30
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 31
 #elif GENERIC_NAME_N == 31
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 32
 #elif GENERIC_NAME_N == 32
 #undef GENERIC_NAME_N
 #define GENERIC_NAME_N 33
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
#elif GENERIC_NAME_N == 11
#define GENERIC_TYPE_11 GENERIC_ELEMENT(argtype11)
#elif GENERIC_NAME_N == 12
#define GENERIC_TYPE_12 GENERIC_ELEMENT(argtype12)
#elif GENERIC_NAME_N == 13
#define GENERIC_TYPE_13 GENERIC_ELEMENT(argtype13)
#elif GENERIC_NAME_N == 14
#define GENERIC_TYPE_14 GENERIC_ELEMENT(argtype14)
#elif GENERIC_NAME_N == 15
#define GENERIC_TYPE_15 GENERIC_ELEMENT(argtype15)
#elif GENERIC_NAME_N == 16
#define GENERIC_TYPE_16 GENERIC_ELEMENT(argtype16)
#elif GENERIC_NAME_N == 17
#define GENERIC_TYPE_17 GENERIC_ELEMENT(argtype17)
#elif GENERIC_NAME_N == 18
#define GENERIC_TYPE_18 GENERIC_ELEMENT(argtype18)
#elif GENERIC_NAME_N == 19
#define GENERIC_TYPE_19 GENERIC_ELEMENT(argtype19)
#elif GENERIC_NAME_N == 20
#define GENERIC_TYPE_20 GENERIC_ELEMENT(argtype20)
#elif GENERIC_NAME_N == 21
#define GENERIC_TYPE_21 GENERIC_ELEMENT(argtype21)
#elif GENERIC_NAME_N == 22
#define GENERIC_TYPE_22 GENERIC_ELEMENT(argtype22)
#elif GENERIC_NAME_N == 23
#define GENERIC_TYPE_23 GENERIC_ELEMENT(argtype23)
#elif GENERIC_NAME_N == 24
#define GENERIC_TYPE_24 GENERIC_ELEMENT(argtype24)
#elif GENERIC_NAME_N == 25
#define GENERIC_TYPE_25 GENERIC_ELEMENT(argtype25)
#elif GENERIC_NAME_N == 26
#define GENERIC_TYPE_26 GENERIC_ELEMENT(argtype26)
#elif GENERIC_NAME_N == 27
#define GENERIC_TYPE_27 GENERIC_ELEMENT(argtype27)
#elif GENERIC_NAME_N == 28
#define GENERIC_TYPE_28 GENERIC_ELEMENT(argtype28)
#elif GENERIC_NAME_N == 29
#define GENERIC_TYPE_29 GENERIC_ELEMENT(argtype29)
#elif GENERIC_NAME_N == 30
#define GENERIC_TYPE_30 GENERIC_ELEMENT(argtype30)
#elif GENERIC_NAME_N == 31
#define GENERIC_TYPE_31 GENERIC_ELEMENT(argtype31)
#elif GENERIC_NAME_N == 32
#define GENERIC_TYPE_32 GENERIC_ELEMENT(argtype32)
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
    PrinterSingleton_append(key, GETTYPEPRINTERFN(type));                         \
  }
#define MAKE_NEW_TYPE(type)                                                    \
  MAKE_NEW_TYPE_HELPER(type, GEN_CAT(argtype, GENERIC_NAME_N))
