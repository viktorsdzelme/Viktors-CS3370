// CS 3370 -- Assignment 2: NPC Memory Pool
// Author: Viktors Dzelme
//
// NPC.cpp -- NPC constructor, output operator, and pool-backed new/delete.
#include "NPC.h"

#include <cassert>

NPC::NPC(const std::string& name, const std::string& type, int x, int y,
         int health)
    : _name(name), _type(type), _x(x), _y(y), _health(health) {}

std::ostream& operator<<(std::ostream& os, const NPC& npc) {
    return os << "NPC[name='" << npc._name << "', type='" << npc._type
              << "', pos=(" << npc._x << ',' << npc._y
              << "), hp=" << npc._health << ']';
}

void* NPC::operator new([[maybe_unused]] std::size_t size) {
    assert(size == sizeof(NPC));  // NPC is final, so this always holds
    return pool.allocate();       // throws std::bad_alloc when full
}

// Also runs if an NPC constructor throws after operator new succeeded, so
// the slot is never leaked.
void NPC::operator delete(void* ptr) noexcept {
    pool.deallocate(ptr);
}
