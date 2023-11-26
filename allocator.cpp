#include <boost/math/special_functions/factorials.hpp>
#include <iostream>
#include <stdexcept>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>
#include <list>
#include <bitset>

const int generic_errorcode = 102;
constexpr std::size_t mapsize = 10;
const std::size_t MaxContainerSize = 10000000;

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
  struct Block
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

    std::array<T, blocksize> m_arena;
    std::bitset<blocksize> m_used;
    std::size_t m_freeCount;
  };

  std::shared_ptr<std::list<Block>> m_blocklist;
};

template <class T, int blocksize, class U>
constexpr bool operator== (const custom_allocator<T, blocksize>& lhs, const custom_allocator<U,blocksize>&rhs) noexcept
{
  if(&lhs == &rhs) { return true; }
  return lhs.m_blocklist == rhs.m_blocklist;
}

template <class T, int blocksize, class U>
constexpr bool operator!= (const custom_allocator<T, blocksize>&lhs, const custom_allocator<U, blocksize>&rhs) noexcept
{
  if(&lhs != &rhs) { return true; }
  return lhs.m_blocklist != rhs.m_blocklist;
}


template <typename T, int blocksize>
T* custom_allocator<T,blocksize>::allocate (std::size_t n) {
//  std::cout << __PRETTY_FUNCTION__ << ": " << __LINE__ << std::endl;
  if(n > blocksize) { throw std::bad_alloc(); }
  for(auto && bi : *m_blocklist.get())
  {
    T *ptr = bi.allocate(n);
    if(ptr != nullptr) {
      return ptr;
    }
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
      if(bit->m_freeCount == blocksize) {bit = m_blocklist->erase(bit); }
      return;
    }
  }
  throw  std::runtime_error{"try to deallocate bad ptr"};
}


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




// NOLINTNEXTLINE(hicpp-named-parameter,readability-named-parameter)
int main(int, char const *[])
{
  try
  {
      // создание экземпляра std::map<int, int>
    std::map<int, int> stdmap;

      // заполнение 10 элементами, где ключ - это число от 0 до 9, а значение - факториал ключа
    for(unsigned ii = 0; ii < mapsize; ++ii)
    {
        // boost factorial always returns float that must be converted to int
      stdmap[ii] = static_cast<int>(boost::math::factorial<double>(ii));
    }


      // - создание экземпляра std::map<int, int> с новым аллокатором, ограниченным 10 элементами
    std::map<int, int, std::less<>, custom_allocator<std::pair<const int,int>, mapsize>> myalloc_map;


      // заполнение 10 элементами, где ключ - это число от 0 до 9, а значение - факториал ключа
    for(auto && mi : stdmap)
    {
      myalloc_map[mi.first] = mi.second;
    }

      // вывод на экран всех значений (ключ и значение разделены пробелом) хранящихся в контейнере
    for(auto && mi : myalloc_map)
    {
      std::cout << mi.first << " " << mi.second << "\n";
    }

      // - создание экземпляра своего контейнера для хранения значений типа int
    custom_list<int> mycont;

      // - заполнение 10 элементами от 0 до 9
    for(unsigned ii = 0; ii < mapsize; ++ii)
    {
      mycont.push_back(ii);
    }

      // - создание экземпляра своего контейнера для хранения значений типа int с новым аллокатором, ограниченным 10 элементами
    custom_list<int, custom_allocator<int, mapsize>> myalloc_mycont;

      // - заполнение 10 элементами от 0 до 9
    for(unsigned ii = 0; ii < mapsize; ++ii)
    {
      myalloc_mycont.push_back(ii);
    }

      // - вывод на экран всех значений, хранящихся в контейнере
    for(auto && mi : myalloc_mycont)
    {
      std::cout << mi << "\n";
    }

  }
  catch(const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return generic_errorcode;
  }
  return 0;
}
