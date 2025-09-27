#ifndef SPLIT_MAP
#define SPLIT_MAP

#include <cstring>
#include <string>
#include <unordered_map>

template <typename K, typename V> struct SplitSlot {
    K key;
    V value;
};

static inline constexpr size_t npo2(size_t number) {
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

template <typename K, typename V, const size_t __min_slots = 1> class SplitMap {
  private:
    std::unordered_map<std::string, size_t> city_to_slot;
    SplitSlot<K, V> slots[npo2(__min_slots)];

    constexpr size_t capacity() const {
        return npo2(__min_slots);
    }

  public:
    __attribute__((noinline)) void insert(const K &key, V value) noexcept {
        auto count = city_to_slot.size();
        auto [it, inserted] = city_to_slot.try_emplace(key, count);
        if (inserted) {
            slots[count++] = {key, value};
        } else {
            slots[it->second] = {key, value};
        }
    }
};

#endif
