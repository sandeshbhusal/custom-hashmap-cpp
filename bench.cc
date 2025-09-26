#include <benchmark/benchmark.h>
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>

#include "custom_map.hpp"

#define ROWS_TO_READ 1'000'000 // read 1M rows for now
#define PREALLOC_SLOTS 10'000

using u64 = uint64_t;

std::vector<std::string> lines;
void LoadLines() {
    std::ifstream file("/home/sbhusal/temp/1brc/solutions/measurements.txt");
    std::string line;
    int linecount = 0;
    while (std::getline(file, line) && linecount++ < ROWS_TO_READ) {
        int offset = 0;
        while (line[offset] != ';')
            offset += 1;
        lines.push_back(line.substr(0, offset));
    }
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

void test_custom_map(benchmark::State &state) {
    MyMap<std::string, u64, PREALLOC_SLOTS> my_map;
    for (auto _ : state) {
        int off = 0;
        for (const auto &city : lines) {
            off += 1;
            my_map.insert(city, off);
        }
    }

    benchmark::DoNotOptimize(my_map);
}

int main(int argc, char **argv) {
    LoadLines();
    benchmark::Initialize(&argc, argv);
    benchmark::RegisterBenchmark("TestStdMap", test_stdmap);
    benchmark::RegisterBenchmark("TestMyMap", test_custom_map);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
