#pragma once

#include <memory>


template<typename T, class Allocator = std::allocator<T>>
class custom_list
{
  struct Node
  {
    T m_value;
    Node *m_next;
  };

protected:
  using 	TAllocator = typename std::allocator_traits<Allocator>::template
    rebind_alloc<T>;

  using TAllocatorTraits  = std::allocator_traits<TAllocator>;

  using NodeAllocator = typename TAllocatorTraits::template
    rebind_alloc<Node>;

  using NodeAllocatorTraits = std::allocator_traits<NodeAllocator>;
public:


  custom_list()
    : m_head(nullptr), m_tail(nullptr), m_alloc()
  {
  }

  ~custom_list()
  {
    clear();
  }

  void push_back(T && value)
  {
    Node *onemore = NodeAllocatorTraits::allocate(m_alloc, 1);
    onemore = new (onemore) Node({std::move(value), nullptr});
    if(m_tail) m_tail->m_next = onemore;
    m_tail = onemore;
    if(m_head == nullptr) { m_head = m_tail;}
  }

  void clear()
  {
    Node *it = m_head;
    while(it != nullptr)
    {
      Node *todel = it;
      it = it->m_next;
      todel->~Node();
      NodeAllocatorTraits::deallocate(m_alloc, todel, 1);
    }
    m_head = m_tail = nullptr;
  }

  class iterator
  {
  public:
    iterator() noexcept
    : m_it() { }

    explicit
    iterator(Node* x) noexcept
      : m_it(x) { }

    T& operator*() const noexcept { return m_it->m_value; }
    T* operator->() const noexcept{ return &m_it->m_value; }

    iterator& operator++() noexcept {
      m_it = m_it->m_next;
      return *this;
    }

    friend bool
    operator==(const iterator& x, const iterator& y) noexcept
      { return x.m_it == y.m_it; }

  private:
    Node *m_it;
  };


  iterator begin() noexcept
  {
    return iterator(m_head);
  }

  iterator end() noexcept
  {
    return iterator(nullptr);
  }

private:
  Node *m_head;
  Node *m_tail;
  NodeAllocator m_alloc;
};
