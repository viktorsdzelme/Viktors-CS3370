# Pulse Analysis Report

## 1. Prediction (written before any timing)

I expected reading to be the slowest stage because it processes over a million samples per file and converts text into integers. This involves more work than comparing or adding numbers. Detection is a single pass, while smoothing performs a small number of arithmetic operations per sample. Piggyback, area, and printing operate on detected pulses.

## 2. Baseline (commit tagged `baseline`)

- Data: 20 files × 1,000,000 samples = 20 million samples.
- Seed: Not recorded; original generator command unavailable.
- Build: `-O2`.
- Timing: Five runs; median reported.
- Machine: MacBook Pro, macOS.

| Stage | Median time (ms) | Calls per run |
|---|---:|---:|
| Read | 785.978 | 20 |
| Smooth | 17.963 | 20 |
| Detect | 6.855 | 20 |
| Piggyback | 0.203 | 20 |
| Area | 1.471 | 80,065 |
| Print | 9.675 | 80,065 |

## 3. AI Suggestions

| # | Suggestion | Category | Decision | Reason | Effect |
|---|---|---|---|---|---|
| 1 | Clamp the area window before iterator arithmetic. | Safety | Accept | Prevents forming an iterator beyond the vector end. | Safer area calculation; expected output unchanged. |
| 2 | Use wider arithmetic for smoothing. | Safety | Accept | Prevents intermediate signed integer overflow. | More robust smoothing; expected output unchanged. |
| 3 | Reserve vector capacity when reading input. | Performance | Accept | Reduces repeated vector reallocations. | Read median improved about 1.9%. |
| 4 | Reserve capacity in dropPiggybacks(). | Performance | Reject | Piggyback takes only about 0.02% of runtime. | Expected performance gain too small to justify the change. |
| 5 | Use a wider type for area accumulation. | Safety | Reject for now | No demonstrated overflow in supplied data; changing the result type needs additional consideration. | No implementation change. |
| 6 | Check for INT_MIN before negating input values. | Safety | Accept as a future improvement | Negating INT_MIN using int causes signed overflow. | Identified a remaining input-safety concern. |

## 4. Final Timing

| Stage | Baseline ms | Final ms | Change |
|---|---:|---:|---:|
| Read | 785.978 | 771.037 | -1.9% |
| Smooth | 17.963 | 11.774 | -34.4% |
| Detect | 6.855 | 7.339 | +7.1% |
| Piggyback | 0.203 | 0.203 | 0.0% |
| Area | 1.471 | 1.469 | -0.1% |
| Print | 9.675 | 9.641 | -0.4% |

The sum of the stage medians decreased from 822.145 ms to 801.463 ms, approximately a 2.5% improvement. These are sums of individual stage medians, not a separately measured total runtime.

## 5. Reflection

1. My prediction was correct. Reading was the dominant stage, accounting for approximately 95.6% of baseline stage time. This showed that input processing was the most important performance target.

2. I rejected reserving capacity in dropPiggybacks() because that stage accounted for only about 0.02% of runtime. Optimizing it would have almost no effect on total execution time.

3. I reviewed gen_data.cpp and stage_timer.h. The generator uses a seeded Mersenne Twister for reproducible data. The timer uses steady_clock and RAII to record elapsed time, and prints to stderr. I also checked the generated data against expected output and used sanitizer builds to test edge cases. I did not identify a functional mistake in either file.

4. I learned that vector::reserve() changes capacity without changing size, which can reduce reallocations when the approximate number of elements is known. I also learned that the types used in intermediate arithmetic matter: assigning an expression to a wider variable afterward does not prevent overflow that already occurred during the expression.
