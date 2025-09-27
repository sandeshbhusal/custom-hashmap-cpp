#ifndef CUSTOM_MAP
#define CUSTOM_MAP

#include <functional>

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
    inline void insert(const K &key, V value) noexcept {
        Slot<K, V> to_insert;
        to_insert.hash = std::hash<K>{}(key);
        to_insert.key = key;
        to_insert.value = value;
        to_insert.psl = 0;
        to_insert.occupied = true;

        size_t bucket = to_insert.hash & (capacity() - 1);

        while (true) {
            if (!slots[bucket].occupied) {
                slots[bucket] = to_insert;
                return;
            }

            if (slots[bucket].hash == to_insert.hash &&
                slots[bucket].key == to_insert.key) {
                slots[bucket].value = value;
                return;
            }

            if (to_insert.psl > slots[bucket].psl) {
                std::swap(to_insert, slots[bucket]);
            }

            to_insert.psl++;
            bucket = (bucket + 1) & (capacity() - 1);
        }
    }
};

#endif