#ifndef LINPROBEHM
#define LINPROBEHM

#include "utils.hpp"
#include <stdexcept>
#include <vector>

template <typename K, typename V> class LinProbeHashMap {
    struct Entry {
        K key;
        V value;
        bool occupied = false;
    };

    std::vector<Entry> m_store;
    size_t m_capacity;
    size_t m_count;

  private:
    inline size_t hash_key(const K &key) const { return hash_key_fast(key); }

  public:
    LinProbeHashMap(size_t capacity) {
        // Use 4x capacity to keep load factor low for linear probing
        m_capacity = next_power_of_2(capacity * 4);
        m_store.resize(m_capacity);
        m_count = 0;
    }

    LinProbeHashMap() = delete;
    LinProbeHashMap(const LinProbeHashMap &) = delete;
    LinProbeHashMap(LinProbeHashMap &&) = delete;

    void insert(const K &key, V value) {
        const size_t key_hash = hash_key(key);
        size_t index = key_hash & (m_capacity - 1);

        // Linear probing (not quadratic)
        for (size_t i = 0; i < m_capacity; ++i) {
            size_t probe_index = (index + i) & (m_capacity - 1);
            Entry &slot = m_store[probe_index];

            if (!slot.occupied) {
                slot.occupied = true;
                slot.key = key;
                slot.value = std::move(value);
                m_count++;
                return;
            }

            if (slot.key == key) {
                slot.value = std::move(value);
                return;
            }
        }

        throw std::runtime_error("HashMap is full\n");
    }

    V *get_value(const K &key) const {
        const size_t key_hash = hash_key(key);
        size_t index = key_hash & (m_capacity - 1);

        for (size_t i = 0; i < m_capacity; ++i) {
            size_t probe_index = (index + i) & (m_capacity - 1);
            const Entry &slot = m_store[probe_index];

            if (!slot.occupied) {
                return nullptr;  // Empty slot means key not found
            }

            if (slot.key == key) {
                return const_cast<V*>(&slot.value);
            }
        }

        return nullptr;  // Key not found
    }

    size_t size() const { return m_count; }
};
#endif
