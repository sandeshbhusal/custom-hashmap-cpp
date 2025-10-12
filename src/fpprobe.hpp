#ifndef FINGERPRINT_PROBER_HPP
#define FINGERPRINT_PROBER_HPP

#include "utils.hpp" // Assuming this contains hash_key_fast and next_power_of_2
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <string>

// Forward declaration for the hash function you provided
inline size_t hash_key_fast(const std::string &city);

template <typename K, typename V>
class FPProbeHashMap {
private:
    struct Entry {
        // A fingerprint of 0 indicates an empty slot.
        // This is the key optimization: a cheap, 1-byte check.
        uint8_t fingerprint = 0;
        K key;
        V value;
    };

    std::vector<Entry> m_store;
    size_t m_capacity;
    size_t m_count;

    // Use the same fast hash function
    inline size_t hash_key(const K &key) const {
        return hash_key_fast(key);
    }

public:
    explicit FPProbeHashMap(size_t initial_capacity) {
        // We still want a low load factor to minimize long probe sequences.
        // A 2x multiplier is often a good starting point before tuning.
        m_capacity = next_power_of_2(initial_capacity * 2);
        m_store.resize(m_capacity);
        m_count = 0;
    }

    // Disable copy/move constructors and assignments
    FPProbeHashMap() = delete;
    FPProbeHashMap(const FPProbeHashMap &) = delete;
    FPProbeHashMap(FPProbeHashMap &&) = delete;
    FPProbeHashMap& operator=(const FPProbeHashMap &) = delete;
    FPProbeHashMap& operator=(FPProbeHashMap &&) = delete;

    void insert(const K &key, V value) {
        const size_t key_hash = hash_key(key);
        
        // Calculate the fingerprint from the hash.
        // We must ensure the fingerprint is never 0, since 0 means "empty".
        // This logic makes the fingerprint 1 if its bits would have been 0.
        const uint8_t fingerprint = (key_hash & 0xFF) | ((key_hash & 0xFF) == 0);
        
        // Find the initial slot using a fast bitwise AND
        size_t index = key_hash & (m_capacity - 1);

        // --- Linear Probing Loop ---
        for (size_t i = 0; i < m_capacity; ++i) {
            size_t probe_index = (index + i) & (m_capacity - 1);
            Entry &slot = m_store[probe_index];

            // OPTIMIZATION 1: Check for an empty slot first.
            if (slot.fingerprint == 0) {
                slot.fingerprint = fingerprint;
                slot.key = key;
                slot.value = std::move(value);
                m_count++;
                return;
            }

            // OPTIMIZATION 2: Check the cheap fingerprint before the expensive key.
            // The expensive string comparison is now the last resort.
            if (slot.fingerprint == fingerprint && slot.key == key) {
                slot.value = std::move(value); // Update existing value
                return;
            }
        }

        // If the loop completes, the map is full.
        throw std::runtime_error("HashMap is full\n");
    }

    V* get_value(const K &key) {
        const size_t key_hash = hash_key(key);
        const uint8_t fingerprint = (key_hash & 0xFF) | ((key_hash & 0xFF) == 0);
        size_t index = key_hash & (m_capacity - 1);

        // --- Linear Probing Loop ---
        for (size_t i = 0; i < m_capacity; ++i) {
            size_t probe_index = (index + i) & (m_capacity - 1);
            const Entry &slot = m_store[probe_index];

            // If we hit an empty slot, the key cannot be in the map, so we can stop early.
            if (slot.fingerprint == 0) {
                return nullptr;
            }

            // The same optimization as insert: check fingerprint first.
            if (slot.fingerprint == fingerprint && slot.key == key) {
                // We need to return a non-const pointer, so a const_cast is necessary here.
                return const_cast<V*>(&slot.value);
            }
        }

        return nullptr; // Key not found after checking all possible slots
    }

    size_t size() const {
        return m_count;
    }
};

#endif // FINGERPRINT_PROBER_HPP
