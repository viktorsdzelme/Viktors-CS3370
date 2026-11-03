CXX      := g++
CXXFLAGS := -std=c++23 -Wall -Wextra

all: config tests

config: main.cpp pulse_config.cpp pulse_config.h
	$(CXX) $(CXXFLAGS) main.cpp pulse_config.cpp -o config

tests: test_config.cpp pulse_config.cpp pulse_config.h
	$(CXX) $(CXXFLAGS) test_config.cpp pulse_config.cpp -o tests

check: tests
	./tests

run: config
	./config ini/pulse.ini

clean:
	rm -f config tests harness_temp.ini

.PHONY: all check run clean
