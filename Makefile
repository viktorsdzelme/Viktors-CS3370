# CS 3370 -- Assignment 2: NPC Memory Pool
# Author: Viktors Dzelme
#
#   make        build ./npc_pool
#   make run    build and run (driver program + self-test)
#   make clean  remove build products

CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -g -MMD -MP
TARGET   = npc_pool
SRCS     = main.cpp NPC.cpp Pool.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS) $(OBJS:.o=.d)

.PHONY: all run clean

-include $(OBJS:.o=.d)
