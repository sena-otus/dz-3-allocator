#pragma once

#include <memory>
#include <list>
#include <stdexcept>
#include <bitset>


template <typename T, int blocksize>
struct custom_allocator {
  using value_type = T;
  custom_allocator() noexcept : m_blocklist(std::make_shared<std::list<Block>>()) {};
  template <typename U, int blocksizeU>
  custom_allocator (const custom_allocator<U,blocksizeU>& other) noexcept : m_blocklist(other) {}
  T* allocate (std::size_t n);
  void deallocate (T* p, std::size_t n);
  template <typename U, int blocksizeU = blocksize> struct rebind { using other = custom_allocator<U, blocksizeU>; };

  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;
private:
  struct Block;
  std::shared_ptr<std::list<Block>> m_blocklist;
};

template <class T, int blocksize, class U>
constexpr bool operator== (const custom_allocator<T, blocksize>& lhs, const custom_allocator<U,blocksize>&rhs) noexcept
{
  return lhs.m_blocklist == rhs.m_blocklist;
}

template <class T, int blocksize, class U>
constexpr bool operator!= (const custom_allocator<T, blocksize>&lhs, const custom_allocator<U, blocksize>&rhs) noexcept
{
  return lhs.m_blocklist != rhs.m_blocklist;
}


template <typename T, int blocksize>
T* custom_allocator<T,blocksize>::allocate (std::size_t n) {
  if(n > blocksize) { throw std::bad_alloc(); }
  for(auto && bi : *m_blocklist.get())
  {
    T *ptr = bi.allocate(n);
    if(ptr != nullptr) { return ptr; }
  }

  auto &onemore = m_blocklist->emplace_back();
  T *ptr = onemore.allocate(n);
  if(ptr == nullptr) {  throw std::bad_alloc(); }
  return ptr;
}

template <typename T, int blocksize>
void custom_allocator<T, blocksize>::deallocate (T* p, std::size_t n) {
  if(n > blocksize) { throw  std::runtime_error{"try to deallocate too much"};}
  for(auto bit = m_blocklist->begin(); bit != m_blocklist->end(); bit++)
  {
    if(bit->deallocate(p, n)) {
       // delete empty block
      if(bit->freeCount() == blocksize) {bit = m_blocklist->erase(bit); }
      return;
    }
  }
  throw  std::runtime_error{"try to deallocate bad ptr"};
}



template <typename T, int blocksize>
struct custom_allocator<T, blocksize>::Block
{
  Block()
    : m_arena(), m_used(), m_freeCount(blocksize)
    {}


  T* allocate(std::size_t n)
    {
      if(n == 0) {n++;}
      if(m_freeCount < n) { return nullptr; }
      for(std::size_t ii = 0; ii < (blocksize-n+1); ++ii)
      {
        bool found = true;
        for(std::size_t kk = 0; kk < n; ++kk)
        {
          if(m_used[ii+kk])
          {
            found = false;
            break;
          }
        }
        if(found)
        {
          for(std::size_t kk = 0; kk < n; ++kk)
          {
            m_used[ii+kk] = true;
            m_freeCount--;
          }
          return &m_arena[ii];
        }
      }
      return nullptr;
    }

    bool deallocate (T* p, std::size_t n) {
      if(p < m_arena.data()) { return false; }
      auto index = p-m_arena.data();
      if(index >= blocksize) { return false; }
      for(std::size_t kk = 0; kk < n; ++kk)
      { // TODO: possible check for double-free
        m_used[index+kk] = false;
        m_freeCount++;
      }
      return true;
    }

    auto freeCount() const { return m_freeCount; }

  private:
    std::array<T, blocksize> m_arena;
    std::bitset<blocksize> m_used;
    std::size_t m_freeCount;
};
