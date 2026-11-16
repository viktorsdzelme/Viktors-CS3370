CXX      = g++
STD      = -std=c++23
SRC      = pulse.cpp pulse_config.cpp

all: pulse pulse_debug

# timing build: the only build you benchmark
pulse: $(SRC) pulse_config.h
	$(CXX) $(STD) -O2 $(SRC) -o pulse

# correctness build: catches out-of-bounds reads and UB
pulse_debug: $(SRC) pulse_config.h
	$(CXX) $(STD) -g -fsanitize=address,undefined $(SRC) -o pulse_debug

# test-data generator (gen_data.cpp comes in Step 2)
gen_data: gen_data.cpp
	$(CXX) $(STD) -O2 gen_data.cpp -o gen_data

run: pulse
	./pulse pulse.ini

clean:
	rm -f pulse pulse_debug gen_data

.PHONY: all run clean
