CXXFLAGS = -std=c++23 -Wall -Wextra

run: pulse
	./pulse pulse.ini

pulse: pulse.cpp pulse_config.cpp pulse_config.h
	g++ $(CXXFLAGS) pulse.cpp pulse_config.cpp -o pulse

clean:
	rm -f pulse
