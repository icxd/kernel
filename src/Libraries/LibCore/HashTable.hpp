#pragma once

#include "DoublyLinkedList.hpp"
#include "Hashable.hpp"

namespace Core {

  template <typename T, typename = Hash<T>>
  class HashTable;

  template <typename T, typename HashableForT>
  class HashTable {
  private:
    struct Bucket {
      DoublyLinkedList<T> chain;
    };

  public:
    HashTable() {}
    ~HashTable() { clear(); }

    bool is_empty() const { return !m_size; }
    unsigned size() const { return m_size; }
    unsigned capacity() const { return m_capacity; }

    void set(T &&);
    bool contains(const T &) const;
    void clear();

    class Iterator {
    public:
      bool operator!=(const Iterator &other) const {
        if (m_is_end && other.m_is_end)
          return false;
        return &m_table != &other.m_table || m_is_end != other.m_is_end ||
               m_bucket_index != other.m_bucket_index ||
               m_bucket_iterator != other.m_bucket_iterator;
      }
      bool operator==(const Iterator &other) const { return !(*this != other); }
      T &operator*() {
#ifdef HASHTABLE_DEBUG
        kprintf("retrieve { bucket_index: %u, is_end: %u }\n", m_bucket_index,
                m_is_end);
#endif
        return *m_bucket_iterator;
      }
      Iterator &operator++() {
        skip_to_next();
        return *this;
      }

      void skip_to_next() {
#ifdef HASHTABLE_DEBUG
        unsigned pass = 0;
#endif
        while (!m_is_end) {
#ifdef HASHTABLE_DEBUG
          ++pass;
          kprintf("skip_to_next pass %u, m_bucket_index=%u\n", pass,
                  m_bucket_index);
#endif
          if (m_bucket_iterator.is_end()) {
            ++m_bucket_index;
            if (m_bucket_index >= m_table.capacity()) {
              m_is_end = true;
              return;
            }
            m_bucket_iterator = m_table.m_buckets[m_bucket_index].chain.begin();
          } else {
            ++m_bucket_iterator;
          }
          if (!m_bucket_iterator.is_end())
            return;
        }
      }

    private:
      friend class HashTable;
      explicit Iterator(HashTable &table, bool is_end,
                        typename DoublyLinkedList<T>::Iterator bucketIterator =
                            DoublyLinkedList<T>::Iterator::universal_end(),
                        unsigned bucket_index = 0)
          : m_table(table), m_bucket_index(bucket_index), m_is_end(is_end),
            m_bucket_iterator(bucketIterator) {
        if (!is_end && !m_table.is_empty() &&
            !(m_bucket_iterator !=
              DoublyLinkedList<T>::Iterator::universal_end())) {
#ifdef HASHTABLE_DEBUG
          kprintf("bucket iterator init!\n");
#endif
          m_bucket_iterator = m_table.m_buckets[0].chain.begin();
          if (m_bucket_iterator.is_end())
            skip_to_next();
        }
      }

      HashTable &m_table;
      unsigned m_bucket_index{0};
      bool m_is_end{false};
      typename DoublyLinkedList<T>::Iterator m_bucket_iterator;
    };

    Iterator begin() { return Iterator(*this, is_empty()); }
    Iterator end() { return Iterator(*this, true); }

    class ConstIterator {
    public:
      bool operator!=(const ConstIterator &other) const {
        if (m_is_end && other.m_is_end)
          return false;
        return &m_table != &other.m_table || m_is_end != other.m_is_end ||
               m_bucket_index != other.m_bucket_index ||
               m_bucket_iterator != other.m_bucket_iterator;
      }
      bool operator==(const ConstIterator &other) const {
        return !(*this != other);
      }
      const T &operator*() const {
#ifdef HASHTABLE_DEBUG
        kprintf("retrieve { bucket_index: %u, is_end: %u }\n", m_bucket_index,
                m_is_end);
#endif
        return *m_bucket_iterator;
      }
      ConstIterator &operator++() {
        skip_to_next();
        return *this;
      }

      void skip_to_next() {
#ifdef HASHTABLE_DEBUG
        unsigned pass = 0;
#endif
        while (!m_is_end) {
#ifdef HASHTABLE_DEBUG
          ++pass;
          kprintf("skip_to_next pass %u, m_bucket_index=%u\n", pass,
                  m_bucket_index);
#endif
          if (m_bucket_iterator.is_end()) {
            ++m_bucket_index;
            if (m_bucket_index >= m_table.capacity()) {
              m_is_end = true;
              return;
            }
            const DoublyLinkedList<T> &chain =
                m_table.m_buckets[m_bucket_index].chain;
            m_bucket_iterator = chain.begin();
          } else {
            ++m_bucket_iterator;
          }
          if (!m_bucket_iterator.is_end())
            return;
        }
      }

    private:
      friend class HashTable;
      ConstIterator(const HashTable &table, bool is_end,
                    typename DoublyLinkedList<T>::ConstIterator bucketIterator =
                        DoublyLinkedList<T>::ConstIterator::universal_end(),
                    unsigned bucket_index = 0)
          : m_table(table), m_bucket_index(bucket_index), m_is_end(is_end),
            m_bucket_iterator(bucketIterator) {
        if (!is_end && !m_table.is_empty() &&
            !(m_bucket_iterator !=
              DoublyLinkedList<T>::ConstIterator::universal_end())) {
#ifdef HASHTABLE_DEBUG
          kprintf("const bucket iterator init!\n");
#endif
          const DoublyLinkedList<T> &chain = m_table.m_buckets[0].chain;
          m_bucket_iterator = chain.begin();
          if (m_bucket_iterator.is_end())
            skip_to_next();
        }
      }

      const HashTable &m_table;
      unsigned m_bucket_index{0};
      bool m_is_end{false};
      typename DoublyLinkedList<T>::ConstIterator m_bucket_iterator;
    };

    ConstIterator begin() const { return ConstIterator(*this, is_empty()); }
    ConstIterator end() const { return ConstIterator(*this, true); }

    Iterator find(const T &);
    ConstIterator find(const T &) const;

    void remove(const T &value) {
      auto it = find(value);
      if (it != end())
        remove(it);
    }

    void remove(Iterator);

  private:
    Bucket &lookup(const T &, unsigned *bucket_index = nullptr);
    void rehash(unsigned capacity);
    void insert(T &&);

    Bucket *m_buckets = nullptr;
    unsigned m_size = 0, m_capacity = 0;
  };

  template <typename T, typename HashableForT>
  void HashTable<T, HashableForT>::set(T &&value) {
    if (!m_capacity)
      rehash(1);
    auto &bucket = lookup(value);
    for (auto &e : bucket.chain) {
      if (e == value)
        return;
    }

    if (size() >= capacity()) {
      rehash(size() + 1);
      insert(Core::move(value));
    } else {
      bucket.chain.append(Core::move(value));
    }
    m_size++;
  }

  template <typename T, typename HashableForT>
  void HashTable<T, HashableForT>::rehash(unsigned capacity) {
    capacity *= 2;
    auto *new_buckets = new Bucket[capacity];
    auto *old_buckets = m_buckets;
    unsigned old_cap = m_capacity;
    m_buckets = new_buckets;
    m_capacity = capacity;

    for (unsigned i = 0; i < old_cap; i++) {
      for (auto &value : old_buckets[i].chain) {
        insert(Core::move(value));
      }
    }

    delete[] old_buckets;
  }

  template <typename T, typename HashableForT>
  void HashTable<T, HashableForT>::clear() {
    delete[] m_buckets;
    m_size = m_capacity = 0;
  }

  template <typename T, typename HashableForT>
  void HashTable<T, HashableForT>::insert(T &&value) {
    auto &bucket = lookup(value);
    bucket.chain.append(Core::move(value));
  }

  template <typename T, typename HashableForT>
  bool HashTable<T, HashableForT>::contains(const T &value) const {
    if (is_empty())
      return false;
    auto &bucket = lookup(value);
    for (auto &e : bucket.chain) {
      if (e == value)
        return true;
    }
    return false;
  }

  template <typename T, typename HashableForT>
  auto HashTable<T, HashableForT>::find(const T &value) -> Iterator {
    if (is_empty())
      return end();
    unsigned bucket_index;
    auto &bucket = lookup(value, &bucket_index);
    auto bucketIterator = bucket.chain.find(value);
    if (bucketIterator != bucket.chain.end())
      return Iterator(*this, false, bucketIterator, bucket_index);
    return end();
  }

  template <typename T, typename HashableForT>
  auto HashTable<T, HashableForT>::find(const T &value) const -> ConstIterator {
    if (is_empty())
      return end();
    unsigned bucket_index;
    auto &bucket = lookup(value, &bucket_index);
    auto bucketIterator = bucket.chain.find(value);
    if (bucketIterator != bucket.chain.end())
      return ConstIterator(*this, false, bucketIterator, bucket_index);
    return end();
  }

  template <typename T, typename HashableForT>
  void HashTable<T, HashableForT>::remove(Iterator it) {
    ASSERT(!is_empty());
    m_buckets[it.m_bucket_index].chain.remove(it.m_bucket_iterator);
    --m_size;
  }

  template <typename T, typename HashableForT>
  typename HashTable<T, HashableForT>::Bucket &
  HashTable<T, HashableForT>::lookup(const T &value, unsigned *bucket_index) {
    unsigned hash = HashableForT::hash(value);
    if (bucket_index)
      *bucket_index = hash % m_capacity;
    return m_buckets[hash % m_capacity];
  }

} // namespace Core

using Core::HashTable;