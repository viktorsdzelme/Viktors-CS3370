// CS 3370 -- Assignment 2: NPC Memory Pool
// Author: Viktors Dzelme
//
// Pool.h -- a fixed-capacity memory pool of NPC-sized slots.
//
// The pool owns one contiguous block of raw bytes and carves it into
// `count` equal slots. A vector of free slot indices is the free list:
// allocate() pops an index off the back, deallocate() pushes it back on,
// so both are O(1) and the most recently freed slot is reused first.
#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <span>    // C++20
#include <vector>

class Pool {
private:
    // Memory storage
    std::byte*  _memory_block = nullptr;  // start of the raw block
    std::size_t _block_size;              // size of one slot (sizeof(NPC))
    std::size_t _capacity;                // number of slots in the block

    // Free slot tracking using vector
    std::vector<std::size_t> free_indices;  // indices of free slots

    // Statistics
    std::size_t live_npcs = 0;  // how many NPCs exist in the pool
    bool        trace_enabled;  // if true, dump extra info to stdout

    // Address of the slot at `index`: block start + index * slot size.
    [[nodiscard]] void* get_slot(std::size_t index) const {
        return _memory_block + index * _block_size;
    }

    // Safe view of the entire memory block.
    [[nodiscard]] std::span<std::byte> get_memory_span() const {
        return {_memory_block, _capacity * _block_size};
    }

    // True if `ptr` points inside the pool's block.
    [[nodiscard]] bool contains(const void* ptr) const;

public:
    Pool(std::size_t count, bool trace = true);
    ~Pool();

    // The pool owns raw memory, so copying it would double-free.
    Pool(const Pool&)            = delete;
    Pool& operator=(const Pool&) = delete;

    // Hands out a free slot; throws std::bad_alloc if none are left.
    [[nodiscard]] void* allocate();

    // Returns a slot to the free list. Null is ignored (like `delete nullptr`).
    void deallocate(void* ptr);

    // Prints live/free counts and the free slot indices.
    void profile() const;
};
