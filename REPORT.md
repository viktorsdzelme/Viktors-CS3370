# Performance and Memory Report — `factorial<BigInt>(10000)`

## Build Information

Both builds use GNU GCC 16.2.0 and the C++23 standard, with `-Wall -Wextra` enabled.

* **Compiler:** GNU GCC 16.2.0
* **Language standard:** C++23 (`__cplusplus = 202302`)
* **Warning flags:** `-Wall -Wextra`
* **Optimization levels:** `-O0` and `-O2`
* **Compiler warnings:** None
* **Correctness:** All 42 required checks passed under both optimization levels.

## 1. BigInt Representation

My `BigInt` implementation uses a `std::vector<std::uint32_t>` to store the magnitude of an integer in base \(10^9\), with the least-significant limb first (little-endian order). A separate `bool` stores the sign.

The representation follows these invariants:

* There are no unnecessary leading-zero limbs.
* Zero is represented by an empty vector and is never negative.
* Each limb stores a value from \(0\) through \(10^9-1\).

### Why this representation?

**Storage efficiency:** The largest power of ten that fits in a 32-bit unsigned integer is \(10^9\). Each limb can therefore represent nine decimal digits. This reduces the number of stored elements compared with representing every decimal digit separately.

**Safe arithmetic:** Multiplication of two limbs produces a value no greater than \((10^9-1)^2\), which is less than \(10^{18}\). The product, carry, and existing limb can be combined safely in a 64-bit unsigned integer (`std::uint64_t`). Using 64-bit intermediates prevents overflow during limb arithmetic.

**Efficient decimal access:** Because the representation uses a power-of-ten base, decimal digits can be accessed directly without converting the entire number to another base. For example, digit \(i\), counting from the right starting at zero, can be obtained using:

`(limbs[i / 9] / 10^(i % 9)) % 10`

This supports efficient implementations of `operator[]`, `digitCount()`, and `toString()`.

**Arithmetic efficiency:** Grouping nine decimal digits into each limb reduces the number of elements that arithmetic operations must process. This is particularly useful for calculating `10000!`, where each multiplication is by a single-limb integer.

The trade-off is that arithmetic requires handling carries and using wider intermediate values. However, this representation provides a useful balance between storage efficiency and straightforward decimal operations.

## 2. Performance Results for `10000!`

The program calculates `factorial<BigInt>(10000)`, verifies its digit count and trailing zeros, and measures three timed runs at each optimization level.

| Measurement       |        `-O0` |        `-O2` |
| ----------------- | -----------: | -----------: |
| Compiler          |   GCC 16.2.0 |   GCC 16.2.0 |
| Run 1             |     48.63 ms |     29.95 ms |
| Run 2             |     46.76 ms |     26.74 ms |
| Run 3             |     46.56 ms |     25.43 ms |
| **Best time**     | **46.56 ms** | **25.43 ms** |
| Average time      |     47.32 ms |     27.37 ms |
| Result storage    | 16,384 bytes | 16,384 bytes |
| Peak extra memory | 24,580 bytes | 24,580 bytes |
| Heap allocations  |       10,012 |       10,012 |

### Correctness results

Both builds produced the same correct results:

* Decimal digit count: **35,660**
* Trailing zeros: **2,499**
* Required checks passed: **42 out of 42**

### Effect of compiler optimization

The best `-O0` run took 46.56 ms, while the best `-O2` run took 25.43 ms.

The percentage reduction in execution time is:

$$
\frac{46.56-25.43}{46.56}\times100
\approx45.4\%
$$

Therefore, the optimized build was approximately **45.4% faster** than the unoptimized build, using the best run from each set of measurements.

The result storage, peak extra memory, and allocation count were identical at both optimization levels. In this experiment, optimization improved execution speed without changing the reported memory usage.

## 3. Storage of the Result

The result of `factorial<BigInt>(10000)` contains 35,660 decimal digits.

My implementation stores those digits in base \(10^9\), with nine decimal digits per full limb. The number of required limbs is:

$$
\left\lceil\frac{35660}{9}\right\rceil=3963
$$

The storage measurements reported by my program are:

| Measurement              |           Value |
| ------------------------ | --------------: |
| Decimal digits           |          35,660 |
| Limb type                | `std::uint32_t` |
| Bytes per limb           |               4 |
| Number of limbs          |           3,963 |
| Vector capacity          |           4,096 |
| Unused capacity slots    |             133 |
| Allocated vector storage |    16,384 bytes |
| `sizeof(BigInt)`         |        32 bytes |

### Verifying the storage calculation

Each vector element occupies four bytes, and the vector has a capacity of 4,096 elements.

$$
\begin{aligned}
\text{Allocated bytes}
&=\text{capacity}\times\text{element size}\\
&=4096\times4\\
&=\boxed{16384\text{ bytes}}
\end{aligned}
$$

This exactly matches the performance harness's reported result storage of 16,384 bytes.

The vector capacity exceeds the number of used elements by:

$$
4096-3963=133\text{ elements}
$$

This unused capacity is consistent with the vector's growth strategy, which can allocate additional space when its size increases.

The `BigInt` object itself occupies 32 bytes, including its vector object, sign flag, and padding. This is separate from the dynamically allocated storage used for the limbs.

The result's allocated vector storage per decimal digit is approximately:

$$
\frac{16384}{35660}\approx0.459
$$

Therefore, the allocated limb storage uses approximately **0.459 bytes per decimal digit**, excluding the `BigInt` object's 32 bytes and other allocations made during computation.

## 4. Estimate for an Alternative Representation

An alternative would be to represent each decimal digit using one `unsigned char` element in a `std::vector<unsigned char>`.

Since `10000!` contains 35,660 decimal digits, this representation would require at least 35,660 bytes for its digit elements, assuming one byte per element.

If the vector grows geometrically and its capacity reaches 65,536 elements, its allocated storage would be:

$$
65536\times1=65536\text{ bytes}=64\text{ KiB}
$$

This is a reasonable estimate rather than a guaranteed capacity, because the actual capacity depends on the vector's growth behavior. With an exact capacity of 35,660 elements, the digit storage would instead be approximately 35 KiB.

Compared with the current representation's 16,384 bytes, a 65,536-byte allocation would use four times as much storage. Even with an exact capacity of 35,660 bytes, it would use approximately 2.18 times as much storage.

The base-\(10^9\) representation also stores roughly nine times fewer elements than a one-digit-per-byte representation. For multiplication by a single-limb number, this can reduce the number of element-level iterations substantially, although the exact performance difference depends on the implementation.

For these reasons, the current representation offers better storage efficiency for large integers while keeping decimal digit access convenient.

## Conclusion

My `BigInt` implementation successfully passed all 42 required correctness checks using GNU GCC 16.2.0 with both `-O0` and `-O2`.

For `factorial<BigInt>(10000)`, the result contains 35,660 digits and 2,499 trailing zeros. The vector uses 3,963 limbs, with a capacity of 4,096 elements and 16,384 bytes of allocated storage.

The optimized build reduced the best execution time from 46.56 ms to 25.43 ms, an improvement of approximately 45.4%. The base-\(10^9\) representation also provides a compact storage format compared with storing every decimal digit in a separate byte.
