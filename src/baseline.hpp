#ifndef BASELINE
#define BASELINE

#include "utils.hpp"
#include <cstring>
#include <iostream>
#include <ostream>
#include <unordered_map>
#include <vector>

template<typename K, typename V>
struct Slot {
    uint8_t fingerprint;
    K key;
    V value;
};

template <typename K, typename V> class LLHashMap {
    using Entry = Slot<K, V>;

    std::vector<std::vector<Entry>> m_store;
    size_t m_capacity;
    size_t m_count;

  private:
    inline size_t hash_key(const K &key) const { return hash_key_fast(key); }

    inline size_t find_slot(size_t hash) const {
        return hash % m_capacity;
    }

  public:
    LLHashMap(size_t capacity) {
        m_store = std::vector<std::vector<Entry>>(capacity);
        m_capacity = capacity;
        m_count = 0;
    }

    LLHashMap() = delete;
    LLHashMap(const LLHashMap &) = delete;
    LLHashMap(LLHashMap &&) = delete;
    LLHashMap operator=(const LLHashMap &) = delete;
    LLHashMap operator=(LLHashMap &&) = delete;

    void insert(const K &key, V value) {
        size_t hash = hash_key(key);
        uint8_t fingerprint = hash & 0xFF;
        size_t index = find_slot(hash);
        auto &chain = m_store[index];
        for (auto &entry : chain) {
            if (entry.fingerprint == fingerprint && entry.key == key) {
                entry.value = std::move(value);
                entry.fingerprint = fingerprint;
                return;
            }
        }

        chain.push_back({fingerprint, std::move(key), std::move(value)});
        m_count += 1;
    }

    V *get_value(const K &key) const {
        size_t index = find_slot(key);
        auto &chain = m_store[index];
        for (auto &item : chain) {
            if (item.first == key) {
                return &item.second;
            }
        }
        return nullptr;
    }

    size_t size() const { return m_count; }

    void print_stats() const {
        size_t zero_count = 0;
        size_t longest = 0;
        size_t sum = 0;
        size_t count = 0;
        std::unordered_map<size_t, size_t> counts;
        for (const auto &chain : m_store) {
            size_t length = distance(chain.begin(), chain.end());

            longest = std::max(longest, length);
            sum += length;
            if (length == 0) {
                zero_count += 1;
            } else {
                counts[length] += 1;
                count += 1;
            }
        }

        std::cout << "Zero-length buckets: " << zero_count << std::endl;
        std::cout << "Longest-length: " << longest << std::endl;
        std::cout << "Average-length(occupied): " << (sum / count) << std::endl;
        std::cout << "Average-length(overall): " << (sum / m_store.size())
                  << std::endl;

        for (const auto &[size, count] : counts) {
            std::cout << size << ": " << count << std::endl;
        }
    }
};
#endif
