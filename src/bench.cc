#include <benchmark/benchmark.h>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>

#include "utils.hpp"
#include "swiss.hpp"
#include "baseline.hpp"
#include "linprobehm.hpp"

#define PREALLOC_SLOTS 10'000

using u64 = uint64_t;

std::vector<std::string> lines;
void LoadLines(size_t ROWS_TO_READ = 1'000'000) {
    std::ifstream file("/home/sbhusal/hashmap/measurements.txt");
    std::string line;
    int linecount = 0;
    while (std::getline(file, line) && linecount++ < ROWS_TO_READ) {
        int offset = 0;
        while (line[offset] != ';')
            offset += 1;
        lines.push_back(line.substr(0, offset));
    }
    std::cout << "Read " << lines.size() << " lines into the buffer\n";
}

void test_stdmap(benchmark::State &state) {
    std::unordered_map<std::string, uint64_t> map(PREALLOC_SLOTS);
    for (auto _ : state) {
        int off = 0;
        for (const auto &city : lines) {
            off += 1;
            auto [it, inserted] = map.try_emplace(city, off);
            if (!inserted) [[unlikely]] {
                it->second = off;
            }
        }
    }

    benchmark::DoNotOptimize(map);
}

void test_baseline(benchmark::State &state) {
    LLHashMap<std::string, uint64_t> map(PREALLOC_SLOTS);
    for (auto _ : state) {
        int off = 0;
        for (const auto &city : lines) {
            off += 1;
            map.insert(city, off);
        }
    }

    benchmark::DoNotOptimize(map);
}

void test_linprobe(benchmark::State &state) {
    LinProbeHashMap<std::string, uint64_t> map(PREALLOC_SLOTS);
    for (auto _ : state) {
        int off = 0;
        for (const auto &city : lines) {
            off += 1;
            map.insert(city, off);
        }
    }

    benchmark::DoNotOptimize(map);
}

void test_swiss(benchmark::State &state) {
    SwissHashMap<std::string, uint64_t> map(PREALLOC_SLOTS);
    for (auto _ : state) {
        int off = 0;
        for (const auto &city : lines) {
            off += 1;
            map.insert(city, off);
        }
    }

    benchmark::DoNotOptimize(map);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        LoadLines(atoi(argv[1]));
    }

    benchmark::Initialize(&argc, argv);
    benchmark::RegisterBenchmark("TestStdMap", test_stdmap);
    benchmark::RegisterBenchmark("TestBaseline", test_baseline);
    benchmark::RegisterBenchmark("TestLinearProbing", test_linprobe);
    benchmark::RegisterBenchmark("TestSwiss", test_swiss);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
