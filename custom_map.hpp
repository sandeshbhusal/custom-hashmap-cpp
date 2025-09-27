#ifndef CUSTOM_MAP
#define CUSTOM_MAP

#include <functional>
#include <stdexcept>

template <typename K, typename V> struct Slot {
    size_t hash = 0;
    size_t psl = 0;
    bool occupied = false;
    K key;
    V value;
};

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

  public:
    inline void insert(const K &key, V value) {
        size_t hash = std::hash<K>{}(key);
        size_t bucket = hash & (capacity() - 1);
        size_t vpsl = 0;

        K current_key = key;
        V current_value = value;

        while (true) {
            if (!slots[bucket].occupied) {
                slots[bucket].occupied = true;
                slots[bucket].key = key;
                slots[bucket].value = value;
                slots[bucket].psl = vpsl;
                slots[bucket].hash = hash;
                return;
            }

            if (slots[bucket].hash == hash && slots[bucket].key == key) {
                slots[bucket].value = value;
                return;
            }

            if (vpsl > slots[bucket].psl) {
                std::swap(current_key, slots[bucket].key);
                std::swap(current_value, slots[bucket].value);
                std::swap(vpsl, slots[bucket].psl);
                std::swap(hash, slots[bucket].hash);
            }

            vpsl += 1;
            bucket = (bucket + 1) & (capacity() - 1);
        }

        throw std::runtime_error("Hashmap is full");
    }
};

#endif