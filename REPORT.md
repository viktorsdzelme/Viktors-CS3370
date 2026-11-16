# Pulse Analysis Report

## 1. Prediction (written before any timing)

I expect the reading to be the slowest stage because it has to handle over a million samples in the files and then turnthe text in them into integers. This is more work than comparing or adding numbers. The detection stage is a single pass, with smoothing each step is a couple multiples and piggyback, area and printing only work on a couple pulses.

## 2. Baseline (commit tagged `baseline`)
Data: <N> files x <M> samples, seed <S>. Build: -O2.

How I checked the generated data: ...

| Stage | Median ms | Share of total |
|-------|-----------|----------------|

## 3. Suggestions
| # | Suggestion | Category | Decision | Reason | Effect |
|---|------------|----------|----------|--------|--------|

## 4. Final timing
| Stage | Baseline ms | Final ms | Change |
|-------|-------------|----------|--------|

## 5. Reflection
1. How did your prediction compare with the timing report? Where does the time actually go?
2. Choose one suggestion you rejected, or one that turned out to be wrong. What evidence decided it?
3. Did gen_data.cpp or stage_timer.h contain a mistake? How did you find it, or how did you check that there was none?
4. Which suggestion taught you something about C++ that you did not know before?
