#ifndef CS3370_BIGINT_H
#define CS3370_BIGINT_H

#include <compare>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

class BigInt {
public:
    // --- construction and conversion ---------------------------------
    BigInt();                                // the value 0
    BigInt(long long n);                     // implicit, on purpose
    explicit BigInt(const std::string& s);   // "42", "-17", "007"
    explicit operator bool() const;          // true if nonzero

    // --- compound assignment: the real arithmetic lives here ---------
    BigInt& operator+=(const BigInt& rhs);
    BigInt& operator-=(const BigInt& rhs);
    BigInt& operator*=(const BigInt& rhs);

    // --- unary, increment, decrement ---------------------------------
    BigInt  operator-() const;
    BigInt& operator++();       BigInt operator++(int);
    BigInt& operator--();       BigInt operator--(int);

    // --- inspection ---------------------------------------------------
    int operator[](std::size_t i) const;     // i-th decimal digit from the right
    std::size_t digitCount() const;          // 0 has one digit
    bool isNegative() const;
    std::string toString() const;

    // --- extra (for the performance report) ---------------------------
    std::size_t limbCount() const;           // elements in the storage vector
    std::size_t limbCapacity() const;        // capacity() of the storage vector

    // --- comparison ---------------------------------------------------
    friend bool operator==(const BigInt& a, const BigInt& b);
    friend std::strong_ordering operator<=>(const BigInt& a, const BigInt& b);

private:
    // Representation: sign + magnitude.
    //   limbs_ holds the magnitude in base 10^9, least significant limb first.
    //   Invariants (every public function must preserve them):
    //     1. no leading zero limbs  (limbs_.back() != 0)
    //     2. zero is the EMPTY vector, and zero is never negative
    using Limb = std::uint32_t;
    std::vector<Limb> limbs_;
    bool neg_ = false;

    // Adds the signed value (negative ? -mag : +mag). Safe if mag is our own limbs_.
    void addSigned(const std::vector<Limb>& mag, bool negative);
};

// --- non-member binary operators, built on the compound ones ---------
BigInt operator+(BigInt lhs, const BigInt& rhs);
BigInt operator-(BigInt lhs, const BigInt& rhs);
BigInt operator*(BigInt lhs, const BigInt& rhs);
std::ostream& operator<<(std::ostream& os, const BigInt& b);

#endif  // CS3370_BIGINT_H
