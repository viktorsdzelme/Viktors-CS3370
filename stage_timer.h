// stage_timer.h - RAII stage timer. Standard library only.
// Usage:   { StageTimer t{"smooth"}; ...code... }   then print_stage_report();
#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

struct StageTotal {
    double ms = 0.0;      // total elapsed milliseconds
    long   calls = 0;     // number of times the stage ran
};

// One shared table of totals. A function-local static avoids global-init order issues.
inline std::map<std::string, StageTotal>& stage_totals() {
    static std::map<std::string, StageTotal> totals;
    return totals;
}

class StageTimer {
public:
    explicit StageTimer(std::string name)
        : name_{std::move(name)}, start_{std::chrono::steady_clock::now()} {}

    ~StageTimer() {
        using ms = std::chrono::duration<double, std::milli>;
        auto& t = stage_totals()[name_];
        t.ms += std::chrono::duration_cast<ms>(std::chrono::steady_clock::now() - start_).count();
        ++t.calls;
    }

    StageTimer(const StageTimer&) = delete;
    StageTimer& operator=(const StageTimer&) = delete;

private:
    std::string name_;
    std::chrono::steady_clock::time_point start_;
};

// Prints to std::cerr so the program's normal stdout output is unchanged.
inline void print_stage_report() {
    std::cerr << std::fixed << std::setprecision(3);
    for (const auto& [name, t] : stage_totals())
        std::cerr << name << ": " << t.ms << " ms, " << t.calls << " calls\n";
}
