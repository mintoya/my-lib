#define DIRECT_INCLUDE(guard_macro) \
  (!defined(guard_macro) && defined(__INCLUDE_LEVEL__) && (__INCLUDE_LEVEL__ == 0))
