#ifndef CS3370_NUMERIC_H
#define CS3370_NUMERIC_H

#include "bigint.h"

#include <concepts>
#include <utility>

// A type we can copy, build from an int, and add and multiply (with the
// results convertible back to T). std::string fails: it has + but no *.
template <typename T>
concept NumberLike =
    std::copyable<T> &&
    std::constructible_from<T, int> &&
    requires(const T& a, const T& b) {
        { a + b } -> std::convertible_to<T>;
        { a * b } -> std::convertible_to<T>;
    };

// Exponentiation by squaring: O(log exponent) multiplications.
template <NumberLike T>
T power(T base, unsigned exponent) {
    T result(1);
    while (exponent != 0) {
        if (exponent & 1u) result = std::move(result) * base;
        exponent >>= 1;
        if (exponent != 0) base = base * base;   // skip the useless last squaring
    }
    return result;
}

// n! = 2 * 3 * ... * n.  factorial<T>(0) == T(1).
template <NumberLike T>
T factorial(unsigned n) {
    T result(1);
    for (unsigned i = 2; i <= n; ++i)
        result = std::move(result) * T(i);       // move: no copy of the big lhs
    return result;
}

// F(0)=0, F(1)=F(2)=1. Iterative: n additions, no recomputation.
template <NumberLike T>
T fibonacci(unsigned n) {
    T prev(0), cur(1);                           // F(0), F(1)
    for (unsigned i = 0; i < n; ++i) {
        T next = prev + cur;
        prev = std::move(cur);
        cur  = std::move(next);
    }
    return prev;                                 // after n steps prev == F(n)
}

#endif  // CS3370_NUMERIC_H
