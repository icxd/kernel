#pragma once

#include "Defines.hpp"
#include "Types.hpp"

namespace Core {

  template <typename T>
  class DoublyLinkedList {
  private:
    struct Node {
      explicit Node(T &&v) : value(Core::move(v)) {};

      T value;
      Node *next = nullptr, *prev = nullptr;
    };

  public:
    DoublyLinkedList() {}
    ~DoublyLinkedList() { clear(); }

    bool is_empty() const { return !head(); }

    void clear() {
      for (auto *node = m_head; node;) {
        auto *next = node->next;
        delete node;
        node = next;
      }
      m_head = m_tail = nullptr;
    }

    T &first() {
      ASSERT(head());
      return head()->value;
    }
    const T &first() const {
      ASSERT(tail());
      return tail()->value;
    }

    T &last() {
      ASSERT(head());
      return head()->value;
    }
    const T &last() const {
      ASSERT(tail());
      return tail()->value;
    }

    void append(T &&value) {
      auto *node = new Node(Core::move(value));
      if (!m_head) {
        ASSERT(!m_tail);
        m_head = m_tail = node;
        return;
      }

      ASSERT(m_tail);
      m_tail->next = node;
      node->prev = m_tail;
      m_tail = node;
    }

    class Iterator {
    public:
      bool operator!=(const Iterator &other) const {
        return m_node != other.m_node;
      }
      bool operator==(const Iterator &other) const {
        return m_node == other.m_node;
      }

      Iterator &operator++() {
        m_node = m_node->next;
        return *this;
      }
      T &operator*() { return m_node->value; }

      bool is_end() const { return !m_node; }
      static Iterator universal_end() { return Iterator(nullptr); }

    private:
      friend class DoublyLinkedList;
      explicit Iterator(DoublyLinkedList::Node *node) : m_node(node) {}
      DoublyLinkedList::Node *m_node;
    };

    Iterator begin() { return Iterator(m_head); }
    Iterator end() { return Iterator::universal_end(); }

    Iterator find(const T &value) {
      for (auto *node = m_head; node; node = node->next) {
        if (node->value == value)
          return Iterator(node);
      }
      return end();
    }

    void remove(Iterator &it) {
      ASSERT(it.m_node);
      auto *node = it.m_node;
      if (node->prev) {
        ASSERT(node != m_head);
        node->prev->next = node->next;
      } else {
        ASSERT(node == m_head);
        m_head = node->next;
      }
      if (node->next) {
        ASSERT(node != m_tail);
        node->next->prev = node->prev;
      } else {
        ASSERT(node == m_tail);
        m_tail = node->prev;
      }
      delete node;
    }

  private:
    friend class Iterator;

    Node *head() { return m_head; }
    const Node *head() const { return m_head; }

    Node *tail() { return m_tail; }
    const Node *tail() const { return m_tail; }

    Node *m_head = nullptr, *m_tail = nullptr;
  };

}; // namespace Core