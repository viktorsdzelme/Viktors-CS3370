#include "bigint.h"

#include <algorithm>
#include <ostream>
#include <stdexcept>

namespace {

using Limb = std::uint32_t;
using Mag  = std::vector<Limb>;

constexpr Limb BASE        = 1'000'000'000;  // 10^9
constexpr int  BASE_DIGITS = 9;
constexpr Limb kPow10[BASE_DIGITS] = {
    1, 10, 100, 1'000, 10'000, 100'000, 1'000'000, 10'000'000, 100'000'000};

// ---- magnitude helpers: they know nothing about signs ----------------

void trim(Mag& m) {                     // restore invariant 1
    while (!m.empty() && m.back() == 0) m.pop_back();
}

int cmpMag(const Mag& a, const Mag& b) {   // -1, 0, +1
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    for (std::size_t i = a.size(); i-- > 0;)
        if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    return 0;
}

// a += b.  Safe when &a == &b: b[i] is read before a[i] is written.
void addMag(Mag& a, const Mag& b) {
    if (a.size() < b.size()) a.resize(b.size(), 0);
    std::uint64_t carry = 0;
    for (std::size_t i = 0; i < a.size() && (i < b.size() || carry); ++i) {
        std::uint64_t sum = std::uint64_t{a[i]} + carry + (i < b.size() ? b[i] : 0);
        if (sum >= BASE) { sum -= BASE; carry = 1; } else carry = 0;
        a[i] = static_cast<Limb>(sum);
    }
    if (carry) a.push_back(1);          // last use of b, so a reallocation is harmless
}

// a -= b, requires |a| >= |b|.
void subMag(Mag& a, const Mag& b) {
    std::int64_t borrow = 0;
    for (std::size_t i = 0; i < a.size() && (i < b.size() || borrow); ++i) {
        std::int64_t diff = std::int64_t{a[i]} - borrow - (i < b.size() ? b[i] : 0);
        if (diff < 0) { diff += BASE; borrow = 1; } else borrow = 0;
        a[i] = static_cast<Limb>(diff);
    }
    trim(a);
}

// Schoolbook product into a brand-new vector (so aliasing can't hurt).
Mag mulMag(const Mag& a, const Mag& b) {
    Mag res(a.size() + b.size(), 0);
    for (std::size_t i = 0; i < a.size(); ++i) {
        const std::uint64_t ai = a[i];
        std::uint64_t carry = 0;
        for (std::size_t j = 0; j < b.size(); ++j) {
            // max: (10^9-1) + (10^9-1)^2 + (10^9-1) < 10^18 + 10^9, fits in 64 bits
            std::uint64_t cur = res[i + j] + ai * b[j] + carry;
            res[i + j] = static_cast<Limb>(cur % BASE);
            carry = cur / BASE;
        }
        res[i + b.size()] = static_cast<Limb>(carry);   // slot not yet written
    }
    trim(res);
    return res;
}

}  // namespace

// ---------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------
BigInt::BigInt() = default;

BigInt::BigInt(long long n) : neg_(n < 0) {
    // Negate in unsigned arithmetic: well defined even for LLONG_MIN.
    unsigned long long u = neg_ ? 0ULL - static_cast<unsigned long long>(n)
                                : static_cast<unsigned long long>(n);
    while (u != 0) {
        limbs_.push_back(static_cast<Limb>(u % BASE));
        u /= BASE;
    }
}

BigInt::BigInt(const std::string& s) {
    // Validate everything first; nothing is built until the input is known good.
    std::size_t start = 0;
    bool negative = false;
    if (!s.empty() && (s[0] == '-' || s[0] == '+')) {
        negative = (s[0] == '-');
        start = 1;
    }
    if (start == s.size())
        throw std::invalid_argument("BigInt: no digits in \"" + s + "\"");
    for (std::size_t i = start; i < s.size(); ++i)
        if (s[i] < '0' || s[i] > '9')
            throw std::invalid_argument("BigInt: invalid character in \"" + s + "\"");

    // Consume 9 decimal digits at a time from the right.
    limbs_.reserve((s.size() - start + BASE_DIGITS - 1) / BASE_DIGITS);
    for (std::size_t end = s.size(); end > start;) {
        std::size_t begin = (end - start > BASE_DIGITS) ? end - BASE_DIGITS : start;
        Limb limb = 0;
        for (std::size_t i = begin; i < end; ++i)
            limb = limb * 10 + static_cast<Limb>(s[i] - '0');
        limbs_.push_back(limb);
        end = begin;
    }
    trim(limbs_);                       // "007" and "-000"
    neg_ = negative && !limbs_.empty(); // no negative zero
}

BigInt::operator bool() const { return !limbs_.empty(); }

// ---------------------------------------------------------------------
// Compound assignment
// ---------------------------------------------------------------------
void BigInt::addSigned(const std::vector<Limb>& mag, bool negative) {
    if (mag.empty()) return;
    if (limbs_.empty()) {               // 0 + x
        limbs_ = mag;
        neg_ = negative;
        return;
    }
    if (neg_ == negative) {             // same signs: add magnitudes
        addMag(limbs_, mag);
        return;
    }
    int c = cmpMag(limbs_, mag);        // different signs: subtract smaller from larger
    if (c == 0) {
        limbs_.clear();
        neg_ = false;
    } else if (c > 0) {
        subMag(limbs_, mag);            // |this| larger: keep our sign
    } else {
        Mag r = mag;                    // |mag| larger: take its sign
        subMag(r, limbs_);
        limbs_ = std::move(r);
        neg_ = negative;
    }
}

BigInt& BigInt::operator+=(const BigInt& rhs) {
    addSigned(rhs.limbs_, rhs.neg_);
    return *this;
}

BigInt& BigInt::operator-=(const BigInt& rhs) {
    addSigned(rhs.limbs_, !rhs.neg_);   // a - b == a + (-b), without a temporary
    return *this;
}

BigInt& BigInt::operator*=(const BigInt& rhs) {
    [[maybe_unused]] int holder = 0;
    if (limbs_.empty() || rhs.limbs_.empty()) {
        limbs_.clear();
        neg_ = false;
        return *this;
    }
    const bool negative = (neg_ != rhs.neg_);   // read rhs before touching *this

    if (rhs.limbs_.size() == 1) {       // fast path: multiply by one limb in place
        const std::uint64_t m = rhs.limbs_[0];  // copy first: rhs may be *this
        std::uint64_t carry = 0;
        for (Limb& l : limbs_) {
            std::uint64_t cur = std::uint64_t{l} * m + carry;
            l = static_cast<Limb>(cur % BASE);
            carry = cur / BASE;
        }
        if (carry) limbs_.push_back(static_cast<Limb>(carry));
    } else {
        limbs_ = mulMag(limbs_, rhs.limbs_);    // built in a new vector, then swapped in
    }
    neg_ = negative;
    return *this;
}

// ---------------------------------------------------------------------
// Unary, increment, decrement
// ---------------------------------------------------------------------
BigInt BigInt::operator-() const {
    BigInt r = *this;
    if (!r.limbs_.empty()) r.neg_ = !r.neg_;    // never create negative zero
    return r;
}

BigInt& BigInt::operator++()    { return *this += 1; }
BigInt& BigInt::operator--()    { return *this -= 1; }
BigInt  BigInt::operator++(int) { BigInt old = *this; ++*this; return old; }
BigInt  BigInt::operator--(int) { BigInt old = *this; --*this; return old; }

// ---------------------------------------------------------------------
// Inspection
// ---------------------------------------------------------------------
std::size_t BigInt::digitCount() const {
    if (limbs_.empty()) return 1;
    std::size_t digits = (limbs_.size() - 1) * BASE_DIGITS;
    for (Limb top = limbs_.back(); top != 0; top /= 10) ++digits;
    return digits;
}

int BigInt::operator[](std::size_t i) const {
    if (i >= digitCount())
        throw std::out_of_range("BigInt::operator[]: digit index out of range");
    if (limbs_.empty()) return 0;
    return static_cast<int>((limbs_[i / BASE_DIGITS] / kPow10[i % BASE_DIGITS]) % 10);
}

bool BigInt::isNegative() const { return neg_; }

std::string BigInt::toString() const {
    if (limbs_.empty()) return "0";
    std::string s;
    s.reserve(digitCount() + 1);
    if (neg_) s += '-';
    s += std::to_string(limbs_.back());          // top limb: no padding
    for (std::size_t i = limbs_.size() - 1; i-- > 0;) {
        std::string part = std::to_string(limbs_[i]);
        s.append(BASE_DIGITS - part.size(), '0');  // inner limbs: pad to 9 digits
        s += part;
    }
    return s;
}

std::size_t BigInt::limbCount() const    { return limbs_.size(); }
std::size_t BigInt::limbCapacity() const { return limbs_.capacity(); }

// ---------------------------------------------------------------------
// Comparison
// ---------------------------------------------------------------------
bool operator==(const BigInt& a, const BigInt& b) {
    // Sign and limb count are cheap to compare, so they go first.
    // Only if both match do we look at the limbs themselves.
    if (a.neg_ != b.neg_) return false;
    if (a.limbs_.size() != b.limbs_.size()) return false;
    return a.limbs_ == b.limbs_;
}

std::strong_ordering operator<=>(const BigInt& a, const BigInt& b) {
    if (a.neg_ != b.neg_)
        return a.neg_ ? std::strong_ordering::less : std::strong_ordering::greater;
    const int c = cmpMag(a.limbs_, b.limbs_);
    return a.neg_ ? (0 <=> c) : (c <=> 0);       // negatives order in reverse
}

// ---------------------------------------------------------------------
// Non-member operators
// ---------------------------------------------------------------------
BigInt operator+(BigInt lhs, const BigInt& rhs) { lhs += rhs; return lhs; }
BigInt operator-(BigInt lhs, const BigInt& rhs) { lhs -= rhs; return lhs; }
BigInt operator*(BigInt lhs, const BigInt& rhs) { lhs *= rhs; return lhs; }

std::ostream& operator<<(std::ostream& os, const BigInt& b) {
    return os << b.toString();
}
