#ifndef CUSTOM_MAP
#define CUSTOM_MAP

#include <functional>
#include <stdexcept>

template <typename K, typename V> struct Slot {
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
    inline Slot<K, V> &find_slot(const K &key) {
        size_t hash = std::hash<K>{}(key);
        size_t bucket = hash & (capacity() - 1);
        size_t count = 0;
        while (count < capacity()) {
            if (!slots[bucket].occupied)
                return slots[bucket];
            auto &candidate = slots[bucket];
            if (candidate.key == key)
                return candidate;
            count += 1;
            bucket = (bucket + 1) & (capacity() - 1);
        }

        throw std::runtime_error("Hashmap is full");
    }

    void insert(const K& key, V value) {
        Slot<K, V> &slot = find_slot(key);
        slot.value = value;
        slot.key = key;
        slot.occupied = true;
    }
};

#endif