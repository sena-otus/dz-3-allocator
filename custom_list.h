#pragma once

#include <cstddef>
#include <memory>
#include <iostream>


/**
 * custom single-linked list "allocator aware" container
 * */
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

  class iterator;

  custom_list() noexcept(std::is_nothrow_default_constructible<Allocator>::value)
    : m_head(nullptr), m_tail(nullptr), m_alloc()
  {
  }

  ~custom_list()
  {
    clear();
  }

  custom_list(const custom_list<T, Allocator> &other)
    : m_head(nullptr), m_tail(nullptr),
      m_alloc(NodeAllocatorTraits::select_on_container_copy_construction(other.m_alloc))
  {
    for(auto && tval : other)
    {
      push_back(tval);
    }
  }

  custom_list(const custom_list<T, Allocator> &other, const Allocator &alloc)
    : m_head(nullptr), m_tail(nullptr), m_alloc(alloc)
  {
    for(auto && tval : other)
    {
      push_back(tval);
    }
  }


  custom_list(custom_list<T, Allocator> &&other) noexcept
    : m_size(other.m_size), m_head(other.m_head), m_tail(other.m_tail), m_alloc(std::move(other.m_alloc))
  {
    other.m_size = 0;
    other.m_head = nullptr;
    other.m_tail = nullptr;
  }


  custom_list<T, Allocator>& operator=(const custom_list<T, Allocator> &other) noexcept
  {
    if(&other != this)
    {
      if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value && m_alloc != other.m_alloc) {
        clear();
        m_alloc = other.m_alloc;
        for(const auto & ito : other)
        {
          push_back(*ito);
        }
      } else {
        clear();
        for(const auto & ito : other)
        {
          push_back(*ito);
        }
      }
     }
    return *this;
  }

  custom_list& operator=(custom_list &&other) noexcept
  {
    if (m_alloc == other.m_alloc) {
      clear();
      std::swap(m_size, other.m_size);
      std::swap(m_head, other.m_head);
      std::swap(m_tail, other.m_tail);
    }
    else {
      if (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
        clear();
        m_alloc = other.m_alloc; // propagate allocator
        std::swap(m_size, other.m_size);
        std::swap(m_head, other.m_head);
        std::swap(m_tail, other.m_tail);
      } else {
        clear();
        for(auto && it : other)
        {
          push_back(std::move(*it));
        }
      }
    }
    return *this;
  }

  void push_back(const T & value)
  {
    emplace_back(T(value));
  }

  void push_back(T && value)
  {
    emplace_back(std::forward<T>(value));
  }

  std::size_t size() const  { return m_size; }


  template <typename ...Args>
  void emplace_back(Args && ...args)
  {
    Node *onemore = NodeAllocatorTraits::allocate(m_alloc, 1);
    NodeAllocatorTraits::construct(m_alloc, &(onemore->m_value), std::forward<Args>(args)...);
    onemore->m_next = nullptr;
    if(m_tail) { m_tail->m_next = onemore; }
    m_tail = onemore;
    if(m_head == nullptr) { m_head = m_tail;}
    m_size++;
  }


    /**
     * Erase element pointed by iterator.
     * O(n) because need to search whole list because of single linked list
     * @param todel iterator which points to node to delete
     * @return  An iterator pointing to the next element (or end()).
     * */
  iterator erase(iterator todel)
  {
    Node *it = m_head;
    while(it != nullptr)
    {
      if(it->m_next == todel.m_it)
      {
        it->m_next = it->m_next->m_next;
        NodeAllocatorTraits::destroy(m_alloc, &(todel.m_it->m_value));
        NodeAllocatorTraits::deallocate(m_alloc, todel.m_it, 1);
        m_size--;
        return iterator(it->m_next);
      }
      it = it->m_next;
    }
    return end(); // normally should not happen
  }

    /**
     *  erase all elements
     *  */
  void clear()
  {
    Node *it = m_head;
    while(it != nullptr)
    {
      Node *todel = it;
      it = it->m_next;
      NodeAllocatorTraits::destroy(m_alloc, &(todel->m_value));
      NodeAllocatorTraits::deallocate(m_alloc, todel, 1);
    }
    m_size = 0;
    m_head = m_tail = nullptr;
  }

  class iterator
  {
    friend class custom_list<T, Allocator>;
  public:
    iterator() noexcept
    : m_it(nullptr) { }

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

    friend bool
    operator!=(const iterator& x, const iterator& y) noexcept
      { return x.m_it != y.m_it; }
  private:
    Node *m_it;
  };


  class const_iterator
  {
    friend class custom_list<T, Allocator>;
  public:
    const_iterator() noexcept = default;

    explicit
    const_iterator(const Node* x) noexcept : m_it(x) { }

    const T& operator*() const noexcept { return m_it->m_value; }
    const T* operator->() const noexcept{ return &m_it->m_value; }

    const_iterator& operator++() noexcept {
      m_it = m_it->m_next;
      return *this;
    }

    friend bool
    operator==(const const_iterator& x, const const_iterator& y) noexcept
      { return x.m_it == y.m_it; }

    friend bool
    operator!=(const const_iterator& x, const const_iterator& y) noexcept
      { return x.m_it != y.m_it; }
  private:
    const Node *m_it{nullptr};
  };


  iterator begin() noexcept
  {
    return iterator(m_head);
  }

  iterator end() noexcept
  {
    return iterator(nullptr);
  }

  const_iterator begin() const noexcept
  {
    return const_iterator(m_head);
  }

  const_iterator end() const noexcept
  {
    return const_iterator(nullptr);
  }

  friend bool
  operator==(const custom_list& x, const custom_list& y) noexcept
  {
    if(x.size() != y.size()) {return false;}
    auto xit = x.begin();
    auto yit = y.begin();
    while(xit != x.end() && yit != y.end())
    {
      if(*xit != *yit) { return false; }
      ++xit;
      ++yit;
    }
    return true;
  }

private:
  std::size_t m_size{0};
  Node *m_head;
  Node *m_tail;
  NodeAllocator m_alloc;
};
