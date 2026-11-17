// gen_data.cpp - synthetic pulse data generator.
// Build: g++ -std=c++23 -O2 gen_data.cpp -o gen_data
// Usage: gen_data <out_dir> <num_files> <samples_per_file> <seed>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::mt19937 rng;  // seeded once in main

int uni(int lo, int hi) { return std::uniform_int_distribution<int>(lo, hi)(rng); }
double gauss(double sigma) { return std::normal_distribution<double>(0.0, sigma)(rng); }

// Adds a pulse of height `amp` (positive here; negated when written out).
// Linear rise over `rise` samples, then exponential decay over `decay` samples.
void add_pulse(std::vector<double>& sig, int start, double amp, int rise, int decay) {
    int n = static_cast<int>(sig.size());
    for (int k = 0; k < rise && start + k < n; ++k)
        sig[start + k] += amp * (k + 1) / rise;
    for (int k = 1; k <= decay; ++k) {
        int i = start + rise - 1 + k;
        if (i >= n) break;
        sig[i] += amp * std::exp(-4.0 * k / decay);  // falls to ~2% at the end
    }
}

// gen_data --edge <out_dir>: small, awkward files for testing input handling.
int write_edge_files(const fs::path& dir) {
    fs::create_directories(dir);
    rng.seed(1);  // fixed seed so the edge files are always identical
    auto noise = [](int count) {
        std::string s;
        for (int i = 0; i < count; ++i) s += std::format("{} ", std::lround(gauss(20.0)));
        return s + '\n';
    };
    std::ofstream(dir / "edge_empty.dat");                      // 0 bytes
    std::ofstream(dir / "edge_5.dat") << noise(5);              // 5 values
    std::ofstream(dir / "edge_7.dat") << noise(7);              // exactly 7 values
    std::ofstream(dir / "edge_token.dat") << "10 -20 abc 30 5\n";  // non-integer token
    std::ofstream(dir / "edge_flat.dat") << noise(1000);        // noise only, no pulses
    std::cout << std::format("Wrote 5 edge-case files to {}\n", dir.string());
    return 0;
}

int main(int argc, char** argv) {
    if (argc == 3 && std::string(argv[1]) == "--edge") return write_edge_files(argv[2]);
    if (argc != 5) {
        std::cerr << "Usage: gen_data <out_dir> <num_files> <samples_per_file> <seed>\n";
        return 1;
    }
    fs::path out_dir = argv[1];
    int num_files = std::atoi(argv[2]);
    int n = std::atoi(argv[3]);
    rng.seed(static_cast<std::mt19937::result_type>(std::strtoul(argv[4], nullptr, 10)));
    if (num_files <= 0 || n <= 0) {
        std::cerr << "num_files and samples_per_file must be positive\n";
        return 1;
    }
    fs::create_directories(out_dir);
    std::ofstream truth(out_dir / "truth.txt");

    for (int f = 0; f < num_files; ++f) {
        std::string name = std::format("data_{:03}.dat", f);
        std::vector<double> sig(n, 0.0);  // clean pulse signal, positive-going
        std::vector<int> starts;

        auto place = [&](int pos) {
            starts.push_back(pos);
            add_pulse(sig, pos, uni(500, 5000), uni(5, 10), uni(30, 80));
        };

        // Walk along the file dropping pulses with generous gaps between them.
        int pos = uni(20, 200);
        while (pos + 100 < n) {
            place(pos);
            int last = pos;
            if (uni(0, 99) < 30 && pos + 15 + 100 < n) {  // 30%: piggyback pulse
                last = pos + uni(5, 15);
                place(last);
            }
            pos = last + 100 + uni(50, 300);
        }

        // Write samples: noise minus signal, because the hardware negates values.
        std::ofstream out(out_dir / name);
        for (int i = 0; i < n; ++i) {
            out << std::lround(gauss(20.0) - sig[i]) << (i % 10 == 9 ? '\n' : ' ');
        }
        out << '\n';

        truth << name << ':';
        for (int s : starts) truth << ' ' << s;
        truth << '\n';
    }
    std::cout << std::format("Wrote {} files and truth.txt to {}\n", num_files, out_dir.string());
}
