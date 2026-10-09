#include "bigint.h"
#include "numeric.h"
#include "test_bigint.h"   // runs required checks 1-13
#include "perf_bigint.h"   // main.cpp ONLY: it replaces operator new/delete

#include <chrono>
#include <iostream>
#include <string>

// The four lines the handout requires.
static_assert( NumberLike<int>);
static_assert( NumberLike<double>);
static_assert( NumberLike<BigInt>);
static_assert(!NumberLike<std::string>);   // has +, but no *

// Check 6: search for the first Fibonacci number with 1,000 digits.
// Keep the last two values and add once per step (no recomputation from F(1)).
static unsigned firstFibonacciWithDigits(std::size_t digits) {
    BigInt prev(0), cur(1);                 // F(0), F(1)
    unsigned n = 1;
    while (cur.digitCount() < digits) {
        BigInt next = prev + cur;
        prev = std::move(cur);
        cur  = std::move(next);
        ++n;
    }
    return n;
}

int main() {
    std::cout << "Check 6 (search): first Fibonacci number with 1000 digits is F("
              << firstFibonacciWithDigits(1000) << ")  [expected 4782]\n";

    int failures = runRequiredChecks();     // checks 1-13, labeled PASS/FAIL

    // --- performance and memory report ---------------------------------
    std::cout << "\nRepresentation: std::vector<std::uint32_t>, base 10^9, "
                 "little-endian, plus a bool sign\n";
    runPerformanceReport();

    auto t0 = std::chrono::steady_clock::now();
    BigInt big = factorial<BigInt>(10000);
    auto t1 = std::chrono::steady_clock::now();
    std::cout << "\n10000! (own timing): "
              << std::chrono::duration<double, std::milli>(t1 - t0).count() << " ms\n"
              << "  storage elements: " << big.limbCount() << '\n'
              << "  capacity():       " << big.limbCapacity() << '\n'
              << "  bytes:            " << big.limbCapacity() * sizeof(std::uint32_t) << '\n';

    return failures == 0 ? 0 : 1;
}
