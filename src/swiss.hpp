#ifndef SWISSHM_FIXED
#define SWISSHM_FIXED

#include "utils.hpp"
#include <stdexcept>
#include <vector>
// Required for AVX2 SIMD intrinsics
#include <immintrin.h>

template <typename K, typename V> class SwissHashMap {
    // Control byte values
    static constexpr int8_t kEmpty = 0b10000000;
    static constexpr int8_t kDeleted = 0b11111110;
    // Any other value with MSB=0 is a 'full' slot.

    struct Entry {
        // No metadata here! Just the key and value.
        K key;
        V value;
    };

    // We store control bytes and entries in separate arrays for cache efficiency.
    std::vector<int8_t> m_ctrl;
    std::vector<Entry> m_store;
    size_t m_capacity;
    size_t m_count;

private:
    // A high-quality hash function is critical.
    inline size_t hash_key(const K &key) const { return hash_key_fast(key); }

    // Extracts the 7-bit h2 hash from the full hash.
    static inline int8_t h2(size_t hash) { 
        // Ensure we get a value in range [0, 127] (MSB = 0 for full slots)
        return static_cast<int8_t>(hash & 0x7F); 
    }

    // Finds the next group of slots to probe.
    struct Prober {
        size_t pos;
        size_t step = 0;
        
        void next(size_t capacity) {
            step += 16; // Group size is 16
            pos = (pos + step) % capacity;
        }
    };

public:
    SwissHashMap(size_t capacity) {
        // Capacity must be a power of 2 for fast modulo.
        m_capacity = next_power_of_2(capacity * 2);
        // Allocate extra space for control bytes to avoid bounds checking in SIMD operations
        m_ctrl.assign(m_capacity + 16, kEmpty);
        // Store array matches the capacity exactly
        m_store.resize(m_capacity);
        m_count = 0;
    }

    SwissHashMap() = delete;
    SwissHashMap(const SwissHashMap &) = delete;
    SwissHashMap(SwissHashMap &&) = delete;

    V* find(const K &key) {
        const size_t key_hash = hash_key(key);
        const int8_t h2_hash = h2(key_hash);
        
        // Use a bitmask to wrap around the table capacity.
        Prober prober = {key_hash % m_capacity};

        while (true) {
            // SIMD magic happens here.
            // 1. Create a "needle" with the h2 hash we are looking for.
            __m128i needle = _mm_set1_epi8(h2_hash);
            
            // 2. Load 16 control bytes from the current position.
            __m128i group = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&m_ctrl[prober.pos]));
            
            // 3. Compare all 16 bytes at once.
            __m128i cmp = _mm_cmpeq_epi8(needle, group);
            
            // 4. Create a bitmask from the comparison result.
            //    A '1' in the mask means a potential match at that position in the group.
            int mask = _mm_movemask_epi8(cmp);

            // Iterate through potential matches indicated by the bitmask.
            while (mask != 0) {
                const int bit_pos = __builtin_ctz(mask);
                const size_t index = (prober.pos + bit_pos) % m_capacity;
                
                // This is the "slow" path: full key comparison.
                // We only do this for the few slots that matched the h2 hash.
                if (m_store[index].key == key) {
                    return &m_store[index].value;
                }
                
                // Clear the bit and check the next one.
                mask &= mask - 1;
            }
            
            // Check for an empty slot in the group, which means the key isn't here.
            __m128i empty_check = _mm_set1_epi8(kEmpty);
            cmp = _mm_cmpeq_epi8(empty_check, group);
            if (_mm_movemask_epi8(cmp) != 0) {
                return nullptr; // Found an empty slot, so key cannot exist.
            }

            // No match in this group, probe to the next one.
            prober.next(m_capacity);
        }
    }

    void insert(const K &key, V value) {
        // In a real implementation, you would resize here if the load factor is high.

        const size_t key_hash = hash_key(key);
        const int8_t h2_hash = h2(key_hash);
        Prober prober = {key_hash % m_capacity};
        
        while (true) {
            __m128i needle = _mm_set1_epi8(h2_hash);
            __m128i group = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&m_ctrl[prober.pos]));
            __m128i cmp = _mm_cmpeq_epi8(needle, group);
            int mask = _mm_movemask_epi8(cmp);

            while (mask != 0) {
                const int bit_pos = __builtin_ctz(mask);
                const size_t index = (prober.pos + bit_pos) % m_capacity;
                if (m_store[index].key == key) {
                    // Key already exists, update value.
                    m_store[index].value = std::move(value);
                    return;
                }
                mask &= mask - 1;
            }
            
            // No existing entry found, now find an empty slot to insert into.
            __m128i empty_needle = _mm_set1_epi8(kEmpty);
            __m128i deleted_needle = _mm_set1_epi8(kDeleted);
            __m128i empty_cmp = _mm_cmpeq_epi8(empty_needle, group);
            __m128i deleted_cmp = _mm_cmpeq_epi8(deleted_needle, group);
            int empty_mask = _mm_movemask_epi8(_mm_or_si128(empty_cmp, deleted_cmp));

            if (empty_mask != 0) {
                const int bit_pos = __builtin_ctz(empty_mask);
                const size_t index = (prober.pos + bit_pos) % m_capacity;
                const size_t ctrl_index = prober.pos + bit_pos;
                m_ctrl[ctrl_index] = h2_hash;
                m_store[index].key = key;
                m_store[index].value = std::move(value);
                m_count++;
                return;
            }

            prober.next(m_capacity);
        }
    }

    size_t size() const { return m_count; }
};
#endif