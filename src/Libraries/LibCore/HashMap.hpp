#pragma once

#include "Defines.hpp"
#include "Hashable.hpp"
#include "LibCore/HashTable.hpp"
#include "Types.hpp"

namespace Core {

  template <typename K, typename V>
  class HashMap {
  private:
    struct Entry {
      K key;
      V value;

      bool operator==(const Entry &other) { return key == other.key; }
    };

    struct EntryHash {
      static unsigned hash(const Entry &entry) {
        return Hash<K>::hash(entry.key);
      }
    };

  public:
    HashMap() {}
    HashMap(HashMap &&other) : m_table(move(other.m_table)) {}
    HashMap &operator=(HashMap &&other) {
      if (this != &other)
        m_table = move(other.m_table);
      return *this;
    }

    bool is_empty() const { return m_table.is_empty(); }
    usz size() const { return m_table.size(); }
    usz capacity() const { return m_table.capacity(); }
    void clear() { m_table.clear(); }

    void set(const K &, const V &);
    void set(const K &, V &&);
    void remove(const K &);

    using IteratorType = typename HashTable<Entry, EntryHash>::Iterator;
    using ConstIteratorType =
        typename HashTable<Entry, EntryHash>::ConstIterator;

    IteratorType begin() { return m_table.begin(); }
    IteratorType end() { return m_table.end(); }
    IteratorType find(const K &);

    ConstIteratorType begin() const { return m_table.begin(); }
    ConstIteratorType end() const { return m_table.end(); }
    ConstIteratorType find(const K &) const;

    void dump() const { m_table.dump(); }

  private:
    HashTable<Entry, EntryHash> m_table;
  };

  template <typename K, typename V>
  void HashMap<K, V>::set(const K &key, const V &value) {
    m_table.set(Entry{key, value});
  }

  template <typename K, typename V>
  void HashMap<K, V>::set(const K &key, V &&value) {
    m_table.set(Entry{key, move(value)});
  }

  template <typename K, typename V>
  void HashMap<K, V>::remove(const K &key) {
    Entry dummy{key, V()};
    m_table.remove(dummy);
  }

  template <typename K, typename V>
  auto HashMap<K, V>::find(const K &key) -> IteratorType {
    Entry dummy{key, V()};
    return m_table.find(dummy);
  }

  template <typename K, typename V>
  auto HashMap<K, V>::find(const K &key) const -> ConstIteratorType {
    Entry dummy{key, V()};
    return m_table.find(dummy);
  }

} // namespace Core

using Core::HashMap;
