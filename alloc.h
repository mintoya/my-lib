#ifndef ALLOC_H
#define ALLOC_H
#include <stdio.h>
#include <stdlib.h>

// Define ANSI color codes
#define COLOR_GREEN "\033[0;32m"
#define COLOR_ORANGE "\033[0;33m"
#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"

// Memory allocation functions with colored output
static void *debugMalloc(size_t size, const char *file, int line) {
  void *ptr = malloc(size);
  printf(COLOR_GREEN "[MALLOC] %zu bytes at %p (%s:%d)" COLOR_RESET "\n", size,
         ptr, file, line);
  return ptr;
}

static void *debugCalloc(size_t n, size_t size, const char *file, int line) {
  void *ptr = calloc(n, size);
  printf(COLOR_GREEN "[CALLOC] %zu×%zu = %zu bytes at %p (%s:%d)" COLOR_RESET
                     "\n",
         n, size, n * size, ptr, file, line);
  return ptr;
}

static void *debugRealloc(void *old, size_t newSize, const char *file,
                          int line) {
  void *ptr = realloc(old, newSize);
  printf(COLOR_ORANGE "[REALLOC] %p → %p (%zu bytes) (%s:%d)" COLOR_RESET "\n",
         old, ptr, newSize, file, line);
  return ptr;
}

static void debugFree(void *ptr, const char *file, int line) {
  printf(COLOR_RED "[FREE] %p (%s:%d)" COLOR_RESET "\n", ptr, file, line);
  free(ptr);
}

// Define macros with colors
// #define clearAllocate(length, size) \
//   debugCalloc(length, size, __FILE__, __LINE__)
// #define regularAllocate(size) debugMalloc(size, __FILE__, __LINE__)
// #define reAllocate(source, newSize) \
//   debugRealloc(source, newSize, __FILE__, __LINE__)
// #define freAllocate(a) debugFree(a, __FILE__, __LINE__)

#define clearAllocate(length, size) calloc(length, size)
#define regularAllocate(size) malloc(size)
#define reAllocate(source, newSize) realloc(source, newSize)
#define freAllocate(a) free(a)

#endif
