#ifndef SOA_PROBER_HPP
#define SOA_PROBER_HPP

#include "utils.hpp" // Assuming this contains next_power_of_2
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <cstring> // For memcpy

// --- High-Quality Hash Function: xxHash64 ---
namespace detail {
    constexpr uint64_t XXH_PRIME64_1 = 0x9E3779B185EBCA87ULL;
    constexpr uint64_t XXH_PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
    constexpr uint64_t XXH_PRIME64_3 = 0x165667B19E3779F9ULL;
    constexpr uint64_t XXH_PRIME64_4 = 0x85EBCA77C2b2AE63ULL;
    constexpr uint64_t XXH_PRIME64_5 = 0x27D4EB2F165667C5ULL;

    inline uint64_t xxh64_round(uint64_t acc, uint64_t input) {
        acc += input * XXH_PRIME64_2;
        acc = (acc << 31) | (acc >> (64 - 31)); // rotl
        acc *= XXH_PRIME64_1;
        return acc;
    }

    inline uint64_t xxh64_avalanche(uint64_t h) {
        h ^= h >> 33;
        h *= XXH_PRIME64_2;
        h ^= h >> 29;
        h *= XXH_PRIME64_3;
        h ^= h >> 32;
        return h;
    }

    inline size_t xxhash64(const void* data, size_t len, uint64_t seed = 0) {
        const uint8_t* p = static_cast<const uint8_t*>(data);
        const uint8_t* const end = p + len;
        uint64_t h64;

        if (len >= 32) {
            const uint8_t* const limit = end - 32;
            uint64_t v1 = seed + XXH_PRIME64_1 + XXH_PRIME64_2;
            uint64_t v2 = seed + XXH_PRIME64_2;
            uint64_t v3 = seed + 0;
            uint64_t v4 = seed - XXH_PRIME64_1;
            do {
                uint64_t val;
                std::memcpy(&val, p, sizeof(val));
                v1 = xxh64_round(v1, val);
                p += 8;
                std::memcpy(&val, p, sizeof(val));
                v2 = xxh64_round(v2, val);
                p += 8;
                std::memcpy(&val, p, sizeof(val));
                v3 = xxh64_round(v3, val);
                p += 8;
                std::memcpy(&val, p, sizeof(val));
                v4 = xxh64_round(v4, val);
                p += 8;
            } while (p <= limit);
            h64 = ((v1 << 1) | (v1 >> 63)) + ((v2 << 7) | (v2 >> 57)) + ((v3 << 12) | (v3 >> 52)) + ((v4 << 18) | (v4 >> 46));
            h64 = xxh64_round(h64, v1);
            h64 = xxh64_round(h64, v2);
            h64 = xxh64_round(h64, v3);
            h64 = xxh64_round(h64, v4);
        } else {
            h64 = seed + XXH_PRIME64_5;
        }

        h64 += len;
        while (p + 8 <= end) {
            uint64_t val;
            std::memcpy(&val, p, sizeof(val));
            h64 = xxh64_round(h64, val);
            p += 8;
        }
        if (p + 4 <= end) {
            uint32_t val;
            std::memcpy(&val, p, sizeof(val));
            h64 ^= static_cast<uint64_t>(val) * XXH_PRIME64_1;
            h64 = ((h64 << 23) | (h64 >> 41)) * XXH_PRIME64_2 + XXH_PRIME64_3;
            p += 4;
        }
        while (p < end) {
            h64 ^= (*p) * XXH_PRIME64_5;
            h64 = ((h64 << 11) | (h64 >> 53)) * XXH_PRIME64_1;
            p++;
        }
        return xxh64_avalanche(h64);
    }
} // namespace detail


template <typename K, typename V>
class SoAProbeHashMap {
private:
    // --- OPTIMIZATION: Group-Based Probing Data Layout ---
    // We now call this m_control_bytes. It serves the same purpose as fingerprints.
    // A value of 0 is empty, anything else is a fingerprint.
    std::vector<uint8_t> m_control_bytes;
    std::vector<K> m_keys;
    std::vector<V> m_values;

    size_t m_capacity;
    size_t m_count;

    static constexpr size_t GROUP_SIZE = 16;

    inline size_t hash_key(const K &key) const {
        return detail::xxhash64(key.data(), key.size());
    }

public:
    explicit SoAProbeHashMap(size_t initial_capacity) {
        // We can use a higher load factor now, as group probing handles clusters better.
        m_capacity = next_power_of_2(initial_capacity * 1.5);
        
        m_control_bytes.resize(m_capacity, 0);
        m_keys.resize(m_capacity);
        m_values.resize(m_capacity);
        
        m_count = 0;
    }

    SoAProbeHashMap() = delete;
    SoAProbeHashMap(const SoAProbeHashMap &) = delete;
    SoAProbeHashMap(SoAProbeHashMap &&) = delete;
    SoAProbeHashMap& operator=(const SoAProbeHashMap &) = delete;
    SoAProbeHashMap& operator=(SoAProbeHashMap &&) = delete;

    void insert(const K &key, V value) {
        const size_t key_hash = hash_key(key);
        const uint8_t fingerprint = (key_hash & 0xFF) | ((key_hash & 0xFF) == 0);
        size_t group_start_index = (key_hash >> 8) & (m_capacity - 1);

        for (size_t group_offset = 0; group_offset < m_capacity; group_offset += GROUP_SIZE) {
            size_t start_index = (group_start_index + group_offset) & (m_capacity - 1);
            
            size_t first_empty_in_group = -1; // Sentinel value

            // A single pass over the group now correctly finds matches or the first empty slot.
            for(size_t i = 0; i < GROUP_SIZE; ++i) {
                size_t probe_index = (start_index + i) & (m_capacity - 1);

                // Found a match: update in place and we're done.
                if (m_control_bytes[probe_index] == fingerprint && m_keys[probe_index] == key) {
                    m_values[probe_index] = std::move(value);
                    return;
                }

                // Found an empty slot: record it if it's the first in this group.
                if (m_control_bytes[probe_index] == 0 && first_empty_in_group == -1) {
                    first_empty_in_group = probe_index;
                }
            }

            // If we finished the group and found an empty slot (but no match), insert there.
            if (first_empty_in_group != -1) {
                m_control_bytes[first_empty_in_group] = fingerprint;
                m_keys[first_empty_in_group] = key;
                m_values[first_empty_in_group] = std::move(value);
                m_count++;
                return;
            }
        }

        throw std::runtime_error("HashMap is full\n");
    }

    V* get_value(const K &key) {
        const size_t key_hash = hash_key(key);
        const uint8_t fingerprint = (key_hash & 0xFF) | ((key_hash & 0xFF) == 0);
        size_t group_start_index = (key_hash >> 8) & (m_capacity - 1);
        
        for (size_t group_offset = 0; group_offset < m_capacity; group_offset += GROUP_SIZE) {
            size_t start_index = (group_start_index + group_offset) & (m_capacity - 1);

            // This is the core optimization: a tight, predictable loop over a small group.
            // A real SIMD implementation would do this in 1-2 instructions.
            for(size_t i = 0; i < GROUP_SIZE; ++i) {
                size_t probe_index = (start_index + i) & (m_capacity - 1);

                if (m_control_bytes[probe_index] == 0) {
                    // Finding an empty slot in the probe sequence means the key is not here.
                    return nullptr;
                }
                
                if (m_control_bytes[probe_index] == fingerprint && m_keys[probe_index] == key) {
                    return &m_values[probe_index];
                }
            }
        }

        return nullptr;
    }

    size_t size() const {
        return m_count;
    }
};

#endif // SOA_PROBER_HPP

