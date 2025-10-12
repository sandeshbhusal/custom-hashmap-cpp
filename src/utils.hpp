#ifndef UTILS
#define UTILS

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

inline size_t hash_key_fast(const std::string &city) {
    size_t len = city.size();
    size_t h = len;
    if (len >= 4) {
        uint32_t first, last;
        std::memcpy(&first, city.data(), sizeof(uint32_t));
        std::memcpy(&last, city.data() + len - 4, sizeof(uint32_t));
        h ^= (static_cast<size_t>(first) << 32) | last;
    } else {
        for (size_t i = 0; i < len; ++i) {
            h ^= static_cast<size_t>(city[i]) << (i * 8);
        }
    }
    h *= 0x100000001b3;
    h ^= 0xcbf29ce484222325;
    return h;
}

inline size_t next_power_of_2(size_t num) {
    num -= 1;
    num |= (num >> 1);
    num |= (num >> 2);
    num |= (num >> 4);
    num |= (num >> 8);
    num |= (num >> 16);
    num |= (num >> 32);
    num += 1;
    return num;
}

#endif