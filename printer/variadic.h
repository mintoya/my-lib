#ifndef VARIADIC_APPLY_H
#define VARIADIC_APPLY_H

#include <stdio.h>

// -----------------------------------------------------------------------------
// 1. MACRO ITERATION HELPERS
// These recursively apply the base macro (M) to all arguments.
// They stop the recursion when only one argument is left (via APPLY_1).
// -----------------------------------------------------------------------------

// Base case: applies the macro to the last argument.
#define APPLY_1(M, a) M(a)

// Recursive cases: apply to the current argument, then recursively call the macro
// for the next lower count with the rest of the arguments.
#define APPLY_2(M, a, b) M(a), APPLY_1(M, b)
#define APPLY_3(M, a, b, c) M(a), APPLY_2(M, b, c)
#define APPLY_4(M, a, b, c, d) M(a), APPLY_3(M, b, c, d)
#define APPLY_5(M, a, b, c, d, e) M(a), APPLY_4(M, b, c, d, e)
#define APPLY_6(M, a, b, c, d, e, f) M(a), APPLY_5(M, b, c, d, e, f)
#define APPLY_7(M, a, b, c, d, e, f, g) M(a), APPLY_6(M, b, c, d, e, f, g)
#define APPLY_8(M, a, b, c, d, e, f, g, h) M(a), APPLY_7(M, b, c, d, e, f, g, h)
#define APPLY_9(M, a, b, c, d, e, f, g, h, i) M(a), APPLY_8(M, b, c, d, e, f, g, h, i)
#define APPLY_10(M, a, b, c, d, e, f, g, h, i, j) M(a), APPLY_9(M, b, c, d, e, f, g, h, i, j)

// -----------------------------------------------------------------------------
// 2. ARGUMENT COUNTING
// This block determines how many arguments are in __VA_ARGS__ (up to 10).
// -----------------------------------------------------------------------------

// Helper to expand and select the Nth argument (the count).
#define ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N

// Macro that performs the counting logic.
// It expands the arguments followed by the count numbers (10 down to 1).
// If 3 arguments are present (a, b, c), it expands to:
// ARG_N(a, b, c, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0) -> the 11th arg is '3'.
#define COUNT_ARGS(...) \
    ARG_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// -----------------------------------------------------------------------------
// 3. DISPATCHER
// This combines the count and the APPLY_ macro prefix (e.g., APPLY_3).
// -----------------------------------------------------------------------------

#define CAT(A, B) A##B
#define APPLY_MACRO_TO_ARGS_HELPER(N) CAT(APPLY_, N)

/**
 * @brief Applies the specified MACRO to every argument in the variadic list.
 *
 * This is the main, user-facing macro.
 * Usage: APPLY_MACRO_TO_ARGS(YOUR_MACRO, arg1, arg2, arg3, ...)
 *
 * @param MACRO The single-argument macro to apply.
 * @param ... The arguments to which MACRO will be applied.
 */
#define APPLY_N(MACRO, ...) \
    APPLY_MACRO_TO_ARGS_HELPER(COUNT_ARGS(__VA_ARGS__))(MACRO, __VA_ARGS__)

#endif // VARIADIC_APPLY_H

