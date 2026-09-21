// CS 3370 -- Assignment 2: NPC Memory Pool
// Author: Viktors Dzelme
//
// NPC.h -- a game NPC whose instances live in a shared static Pool.
#pragma once

#include <cstddef>
#include <ostream>
#include <string>

#include "Pool.h"

// `final` because the pool hands out sizeof(NPC)-byte slots; a larger
// derived class would not fit in one.
class NPC final {
    std::string _name;  // e.g., "Goblin_247"
    std::string _type;  // e.g., "merchant", "enemy", "quest_giver"
    int _x, _y;         // Grid location on the map
    int _health;        // Health points

    // One pool shared by every NPC.
    inline static Pool pool{100, true};  // 100 NPCs, tracing enabled

public:
    NPC(const std::string& name, const std::string& type, int x, int y,
        int health = 100);

    // Prints e.g. "NPC[name='Goblin_247', type='enemy', pos=(5,10), hp=100]"
    // (no trailing newline).
    friend std::ostream& operator<<(std::ostream& os, const NPC& npc);

    // Route `new NPC` / `delete npc` through the pool instead of the heap.
    static void* operator new(std::size_t size);
    static void operator delete(void* ptr) noexcept;

    static void profile() { pool.profile(); }
};
