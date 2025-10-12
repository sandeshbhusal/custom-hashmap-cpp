#ifndef BASELINE
#define BASELINE

#include <forward_list>
#include <vector>

template <typename K, typename V> class HashMap {
    using Entry = std::pair<K, V>;

    std::vector<std::forward_list<Entry>> m_store;
    size_t m_capacity;
    size_t m_count;

  private:
    inline size_t hash_key(const K &key) const { return std::hash<K>{}(key); }

    inline size_t find_slot(const K &key) const {
        return hash_key(key) % m_capacity;
    }

  public:
    HashMap(size_t capacity) {
        m_store = std::vector<std::forward_list<Entry>>(capacity);
        m_capacity = capacity;
        m_count = 0;
    }
    HashMap() = delete;
    HashMap(const HashMap &) = delete;
    HashMap(HashMap &&) = delete;
    HashMap operator=(const HashMap &) = delete;
    HashMap operator=(HashMap &&) = delete;

    void insert(const K &key, V value) {
        size_t index = find_slot(key);
        auto &chain = m_store[index];
        for (auto &entry : chain) {
            if (entry.first == key) {
                entry.second = std::move(value);
                return;
            }
        }

        chain.push_front({key, std::move(value)});
        m_count += 1;
    }

    V *get_value(const K &key) const {
        size_t index = find_slot(key);
        auto &chain = m_store[index];
        for (auto &item : chain) {
            if (item.first == key) {
                return item.second;
            }
        }
        return nullptr;
    }

    size_t size() const { return m_count; }
};

#endif
