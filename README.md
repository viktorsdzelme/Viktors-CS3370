# CS 3370 – Assignment 2: NPC Memory Pool

**Author:** Viktors Dzelme

A fixed-size memory pool for game NPCs. `NPC` overrides `operator new` /
`operator delete` to allocate from a shared `inline static Pool` of 100 slots.

## Build and run

```
make        # builds ./npc_pool  (C++20, g++)
make run    # runs the handout driver, then the self-test harness
make clean
```

## Files

| File | Purpose |
|------|---------|
| `NPC.h` / `NPC.cpp` | `NPC` class, `operator<<`, pool-backed `new`/`delete`, `profile()` |
| `Pool.h` / `Pool.cpp` | Pool: raw byte block + `std::vector<size_t>` free list, `std::span` view, tracing |
| `main.cpp` | Handout driver (section D) followed by the self-test harness |
| `assignment2_test_harness.h` | Provided harness (two small fixes, see below) |

## Design notes

- `allocate()` pops an index from the back of `free_indices`; `deallocate()`
  pushes it back, so the most recently freed slot is reused first (O(1) both).
- `free_indices` is sized to the pool capacity up front, so `deallocate()` never
  reallocates and is safe inside a `noexcept` `operator delete`.
- The block is allocated with `alignof(NPC)` alignment.
- `deallocate()` asserts the pointer belongs to the pool, sits on a slot
  boundary, and isn't already free (catches double delete).
- Pool construction allocates the raw block last, so a failure can't leak.

## Changes to the provided harness

- Removed a stray `}` after the commented-out `main` snippet at the end of the
  file (it caused `error: extraneous closing brace`).
- Added `#include <functional>` for `std::not_equal_to`.
