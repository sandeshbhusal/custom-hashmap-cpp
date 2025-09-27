#ifndef CUSTOM_MAP
#define CUSTOM_MAP

#include <cstdint>
#include <cstring>
#include <functional>

template <typename K, typename V> struct Slot {
    uint32_t fingerprint = 0;
    K key;
    V value;
};

static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed)
{
	uint32_t h = seed;
    uint32_t k;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

// nearest power of two helper
static inline constexpr size_t nearest_power_of_two(size_t number) {
    number--;
    number |= number >> 1;
    number |= number >> 2;
    number |= number >> 4;
    number |= number >> 8;
    number |= number >> 16;
    number |= number >> 32;
    number++;
    return number;
}

template <typename K, typename V, const size_t __min_slots = 1> class MyMap {
  private:
    Slot<K, V> slots[nearest_power_of_two(__min_slots)];

    constexpr size_t capacity() const {
        return nearest_power_of_two(__min_slots);
    }

    static inline void swap_slot(Slot<K, V> &a, Slot<K, V> &b) noexcept {
        Slot<K, V> tmp = a;
        a = b;
        b = tmp;
    }

  public:
    __attribute__((noinline)) void insert(const K &key, V value) noexcept {
        uint32_t hash = murmur3_32((const uint8_t*) key.data(), key.length(), 42);
        uint32_t fingerprint = hash; // cheap tag
        fingerprint &= ~static_cast<uint32_t>(1);

        size_t bucket = hash & (capacity() - 1);
        while (true) {
            Slot<K, V> &slot = slots[bucket];

            if (slot.fingerprint == 0) {
                slot.fingerprint = fingerprint | 1;
                slot.key = std::move(key);
                slot.value = std::move(value);
                return;
            }

            if (slot.fingerprint == (fingerprint | 1) && slot.key == key) {
                slot.value = value;
                return;
            }

            bucket = (bucket + 1) & (capacity() - 1);
        }
    }
};

#endif
