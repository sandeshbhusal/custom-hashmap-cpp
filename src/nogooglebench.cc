#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>

#include "baseline.hpp"

#define PREALLOC_SLOTS 10'000

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
}

int test_baseline() {
    LLHashMap<std::string, uint64_t> map(PREALLOC_SLOTS);
    int off = 0;
    for (const auto &city : lines) {
        off += 1;
        map.insert(city, off);
    }
    return off;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        LoadLines(atoi(argv[1]));
    }

    test_baseline();
    return 0;
}
