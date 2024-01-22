#pragma once

#include <memory>
#include <list>
#include <stdexcept>
#include <bitset>

/**
 * Custom allocator with space reservation.
 * Can not allocate more than blocksize*sizeof(T) at once.
 * Store blocks in std::list.
 * @tparam T type to allocate
 * @tparam blocksize amount of T to preallocate
 * */
template <typename T, int blocksize>
struct custom_allocator {
  using value_type = T;
    /**
     * default ctor allocates the empty list of blocks
     *  */
  custom_allocator() noexcept : m_blocklist(std::make_shared<std::list<Block>>()) {};
  template <typename U, int blocksizeU>
  explicit custom_allocator (const custom_allocator<U,blocksizeU>& other) noexcept : m_blocklist(other) {}


    /**
     * Alocator will try to allocate necessary blocksize first in existing
     * blocks or add a new block if necessary
     * @param n amount of objects to allocate
     * @return pointer to allocated block
     * */
  T* allocate (std::size_t n)
  {
    if(n > blocksize) { throw std::bad_alloc(); }
    for(auto && bi : *m_blocklist.get())
    {
      T *ptr = bi.allocate(n);
      if(ptr != nullptr) { return ptr; }
    };
    auto &onemore = m_blocklist->emplace_back();
    T *ptr = onemore.allocate(n);
    if(ptr == nullptr) {  throw std::bad_alloc(); } // should never happen
    return ptr;
  }

    /**
     * That deallocator can throw if called with wrong p or n.
     * Will free the block if there are no allocated elements in it.
     * @param p pointer to deallocate
     * @param n amount of objects
     * */
  void deallocate (T* p, std::size_t n)
  {
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

    /** tricky rebind should also accept the blocksize */
  template <typename U, int blocksizeU = blocksize> struct rebind { using other = custom_allocator<U, blocksizeU>; };

  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;
private:
  struct Block;
  std::shared_ptr<std::list<Block>> m_blocklist; //<! can be shared between several instances
};

/** equal if point to same blocklist */
template <class T, int blocksize, class U>
constexpr bool operator== (const custom_allocator<T, blocksize>& lhs, const custom_allocator<U,blocksize>&rhs) noexcept
{
  return lhs.m_blocklist == rhs.m_blocklist;
}

/** not equal if use different blocklists */
template <class T, int blocksize, class U>
constexpr bool operator!= (const custom_allocator<T, blocksize>&lhs, const custom_allocator<U, blocksize>&rhs) noexcept
{
  return lhs.m_blocklist != rhs.m_blocklist;
}


/**
 * class Block can store blocksize instances of T
 * @tparam T type to allocate
 * @tparam blocksize amount of elements on block
 *  */
template <typename T, int blocksize>
struct custom_allocator<T, blocksize>::Block
{
  Block()
    : m_arena(), m_used(), m_freeCount(blocksize)
    {}

    /**
     *  wrapper around reinterpret_cast to get T*
     *  @param idx element index
     *  @return pointer to element
     *  */
  T* arena(std::size_t idx)
  {
    return reinterpret_cast<T*>(&m_arena[idx]); // NOLINT
  }

    /**
     * Search for the first coninues area of size n,
     * mark it as used and
     * @return nullptr on error, T* otherwise
     *  */
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
          return arena(ii);
        }
      }
      return nullptr;
    }

    /**
     * mark given area as unused
     * @return false on error, true otherwise
     *  */
    bool deallocate (T* p, std::size_t n) {
      if(p < arena(0)) { return false; }
      auto index = p-arena(0);
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
    /**
     * Objects will be stored in that array.
     * Can not use array<T,blocksize> because default constructor for
     * each element will be called.
     * */
  std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, blocksize> m_arena;
  std::bitset<blocksize> m_used; ///<! Here we mark used elements
  std::size_t m_freeCount; ///<! amount of free elements to speedup some checks
};
