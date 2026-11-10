//Viktors Dzelme
#include "pulse_config.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

namespace fs = std::filesystem;
using Data = std::vector<int>;

struct Pulse { std::size_t start, peak; };

Data readNegated(const fs::path& file) {
    std::ifstream in{file};
    Data d;
    std::transform(std::istream_iterator<int>{in}, std::istream_iterator<int>{},
                   std::back_inserter(d), std::negate<>{});
    return d;
}

Data smooth(const Data& y) {
    Data s{y};                                  
    for (std::size_t i = 3; i + 3 < y.size(); ++i)
        s[i] = (y[i-3] + 2*y[i-2] + 3*y[i-1] + 3*y[i] + 3*y[i+1] + 2*y[i+2] + y[i+3]) / 15;
    return s;
}

std::vector<Pulse> findPulses(const Data& s, const PulseParams& p) {
    std::vector<Pulse> pulses;
    for (std::size_t i = 0; i + 2 < s.size(); ) {
        if (s[i+2] - s[i] > p.vt) {
            std::size_t j = i + 2;
            while (j + 1 < s.size() && s[j+1] >= s[j]) ++j;
            pulses.push_back({i, j});
            i = j;
        } else ++i;
    }
    return pulses;
}

std::vector<Pulse> dropPiggybacks(const std::vector<Pulse>& in, const Data& s, const PulseParams& p) {
    std::vector<Pulse> keep;
    for (std::size_t k = 0; k < in.size(); ++k) {
        if (k + 1 < in.size() && in[k+1].start - in[k].start <= std::size_t(p.pulse_delta)) {
            double line = p.drop_ratio * s[in[k].peak];
            auto first = s.begin() + in[k].peak + 1, last = s.begin() + in[k+1].start;
            auto below = std::count_if(first, last, [line](int v) { return v < line; });
            if (below > p.below_drop_ratio) {
                std::cout << "Found piggyback at " << in[k].start << '\n';
                continue;
            }
        }
        keep.push_back(in[k]);
    }
    return keep;
}

int main(int argc, char* argv[]) {
    if (argc != 2) { std::cerr << "Usage: " << argv[0] << " <file.ini>\n"; return 1; }
    try {
        PulseConfig config{argv[1]};
        const PulseParams& p = config.params();
        for (const auto& entry : fs::directory_iterator{"."}) {
            if (entry.path().extension() != ".dat") continue;
            Data y = readNegated(entry.path());
            Data s = smooth(y);
            auto pulses = findPulses(s, p);
            if (pulses.empty()) continue;
            std::cout << entry.path().filename().string() << ":\n";
            pulses = dropPiggybacks(pulses, s, p);
            for (std::size_t k = 0; k < pulses.size(); ++k) {
                auto first = y.begin() + pulses[k].start;
                auto last  = (k + 1 < pulses.size())
                               ? std::min(first + p.width, y.begin() + pulses[k+1].start)
                               : first + std::min<std::size_t>(p.width, y.end() - first);
                std::cout << pulses[k].start << " (" << std::accumulate(first, last, 0) << ")\n";
            }
        }
    } catch (const ConfigFileError& e) { std::cerr << e.what() << '\n'; return 1; }
    catch (const std::exception& e) { std::cerr << "Unexpected error: " << e.what() << '\n'; return 2; }
}
