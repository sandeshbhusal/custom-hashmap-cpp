#ifndef CUSTOM_MAP
#define CUSTOM_MAP

#include <cstdint>
#include <functional>

template <typename K, typename V>
struct Slot {
    uint16_t fingerprint = 0;
    bool occupied = false;
    K key;
    V value;
};

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

template <typename K, typename V, const size_t __min_slots = 1>
class MyMap {
  private:
    Slot<K, V> slots[nearest_power_of_two(__min_slots)];

    constexpr size_t capacity() const {
        return nearest_power_of_two(__min_slots);
    }

    static inline void swap_slot(Slot<K,V>& a, Slot<K,V>& b) noexcept {
        Slot<K,V> tmp = a;
        a = b;
        b = tmp;
    }

  public:
    __attribute__((noinline)) void insert(const K &key, V value) noexcept {
        size_t hash = std::hash<K>{}(key);
        uint16_t fingerprint = static_cast<uint16_t>(hash >> 48); // cheap tag

        size_t bucket = hash & (capacity() - 1);
        while (true) {
            Slot<K, V>& slot = slots[bucket];

            if (!slot.occupied) {
                // insert new
                slot.fingerprint = fingerprint;
                slot.key = key;
                slot.value = value;
                slot.occupied = true;
                return;
            }

            // quick reject by hash+fingerprint
            if (slot.fingerprint == fingerprint && slot.key == key) {
                slot.value = value; // update existing
                return;
            }

            // linear probe
            bucket = (bucket + 1) & (capacity() - 1);
        }
    }
};

#endif
