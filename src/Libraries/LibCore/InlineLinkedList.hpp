#pragma once

#include "LibCore/Defines.hpp"
#include "LibCore/Vector.hpp"
#include <LibCpp/cstddef.hpp>

template <typename T> struct InlineLinkedListNode {
  InlineLinkedListNode() {
    set_prev(0);
    set_next(0);
  }

  inline void set_prev(T *prev) { static_cast<T *>(this)->m_prev = prev; }
  inline void set_next(T *next) { static_cast<T *>(this)->m_next = next; }

  inline T *prev() const { return static_cast<const T *>(this)->m_prev; }
  inline T *next() const { return static_cast<const T *>(this)->m_next; }
};

template <typename T> class InlineLinkedList {

public:
  InlineLinkedList() = default;

  inline bool empty() const { return !m_head; }
  inline size_t size_slow() const {
    size_t size = 0;
    for (T *node = m_head; node; node = node->next())
      size++;
    return size;
  }
  inline void clear() { m_head = m_tail = 0; }

  inline T *head() const { return m_head; }
  inline T *remove_head() {
    T *node = head();
    if (node)
      remove(node);
    return node;
  }

  inline T *tail() const { return m_tail; }

  inline void prepend(T *node) {
    if (!m_head) {
      ASSERT(!m_tail);
      m_head = node;
      m_tail = node;
      node->set_prev(0);
      node->set_next(0);
      return;
    }

    ASSERT(m_tail);
    m_head->set_prev(node);
    node->set_next(m_head);
    node->set_prev(0);
    m_head = node;
  }
  inline void append(T *node) {
    if (!m_tail) {
      ASSERT(!m_head);
      m_head = m_tail = node;
      node->set_prev(0);
      node->set_next(0);
      return;
    }

    ASSERT(m_head);
    m_tail->set_next(node);
    node->set_prev(m_tail);
    node->set_next(0);
    m_tail = node;
  }
  inline void remove(T *node) {
    if (node->prev()) {
      ASSERT(node != m_head);
      node->prev()->set_next(node->next());
    } else {
      ASSERT(node == m_head);
      m_head = node->next();
    }

    if (node->next()) {
      ASSERT(node != m_tail);
      node->next()->set_prev(node->prev());
    } else {
      ASSERT(node == m_tail);
      m_tail = node->prev();
    }
  }
  inline void append(InlineLinkedList<T> &other) {}

  Vector<T> to_vector() const {
    Vector<T> vec{};
    return vec;
  }

private:
  T *m_head{nullptr}, *m_tail{nullptr};
};
