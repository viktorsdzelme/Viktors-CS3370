// CS 3370 -- Assignment 2: NPC Memory Pool
// Author: Viktors Dzelme
//
// Pool.cpp -- implementation of the NPC memory pool.
#include "Pool.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <new>
#include <numeric>

#include "NPC.h"  // for sizeof(NPC) / alignof(NPC)

namespace {
// The block is requested with NPC's alignment so every slot (block start
// + i * sizeof(NPC)) is correctly aligned, since sizeof is a multiple of
// alignof.
constexpr std::align_val_t kBlockAlignment{alignof(NPC)};

// How many free indices profile() lists before eliding with "...".
constexpr std::size_t kMaxIndicesShown = 10;
}  // namespace

Pool::Pool(std::size_t count, bool trace)
    : _block_size(sizeof(NPC)), _capacity(count), trace_enabled(trace) {
    // Fill the free list first: if either step throws, nothing leaks
    // (the vector cleans itself up; the raw block isn't allocated yet).
    // resize() gives capacity >= count, and the list never holds more than
    // `count` indices, so push_back in deallocate() can never reallocate
    // (and therefore can never throw from inside a noexcept operator delete).
    free_indices.resize(count);
    std::iota(free_indices.begin(), free_indices.end(), std::size_t{0});

    _memory_block = static_cast<std::byte*>(
        ::operator new(_capacity * _block_size, kBlockAlignment));

    if (trace_enabled) {
        std::cout << "Initializing NPCPool with " << count
                  << " slots (NPC size: " << _block_size << " bytes)\n";
    }
}

Pool::~Pool() {
    if (live_npcs != 0) {
        std::cerr << "Pool warning: destroyed with " << live_npcs
                  << " NPC(s) still live (never deleted)\n";
    }
    ::operator delete(_memory_block, kBlockAlignment);

    if (trace_enabled) {
        std::cout << "Pool destroyed\n";
    }
}

bool Pool::contains(const void* ptr) const {
    const auto memory = get_memory_span();
    const auto* p     = static_cast<const std::byte*>(ptr);
    // std::less gives a total order even for pointers into different objects.
    return std::less_equal<>{}(memory.data(), p) &&
           std::less<>{}(p, memory.data() + memory.size());
}

void* Pool::allocate() {
    if (free_indices.empty()) {
        if (trace_enabled) {
            std::cout << "Allocation failed: pool is full\n";
        }
        throw std::bad_alloc{};
    }

    const std::size_t index = free_indices.back();
    free_indices.pop_back();
    ++live_npcs;

    void* slot = get_slot(index);
    if (trace_enabled) {
        std::cout << "Allocating NPC at slot " << index
                  << " (address: " << slot << ")\n";
    }
    return slot;
}

void Pool::deallocate(void* ptr) {
    if (ptr == nullptr) return;

    assert(contains(ptr) && "deallocate(): pointer was not allocated by this pool");

    const std::size_t offset =
        static_cast<std::size_t>(static_cast<std::byte*>(ptr) - _memory_block);
    assert(offset % _block_size == 0 && "deallocate(): pointer is not at a slot boundary");

    const std::size_t index = offset / _block_size;
    assert(std::ranges::find(free_indices, index) == free_indices.end() &&
           "deallocate(): slot is already free (double delete)");

    free_indices.push_back(index);  // never reallocates; see constructor
    --live_npcs;

    if (trace_enabled) {
        std::cout << "Deallocating NPC at slot " << index
                  << " (address: " << ptr << ")\n";
    }
}

void Pool::profile() const {
    std::cout << "Live NPCs: " << live_npcs
              << ", Free slots: " << free_indices.size() << '\n';

    // List in the order slots will be handed out next (back of the vector
    // first), eliding the tail if the list is long.
    std::cout << "Free slots:";
    if (free_indices.empty()) {
        std::cout << " (none)";
    }
    const std::size_t shown = std::min(free_indices.size(), kMaxIndicesShown);
    for (std::size_t i = 0; i < shown; ++i) {
        std::cout << (i == 0 ? " " : ", ")
                  << free_indices[free_indices.size() - 1 - i];
    }
    if (free_indices.size() > shown) {
        std::cout << "...";
    }
    std::cout << '\n';
}
