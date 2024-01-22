#include "custom_allocator.h"
#include "custom_list.h"

#include <algorithm>
#include <bits/struct_mutex.h>
#include <boost/math/special_functions/factorials.hpp>
#include <iostream>
#include <map>

const int generic_errorcode = 102;

const int MapSize = 10;
const std::size_t MaxAllocationSize = MapSize;


// NOLINTNEXTLINE(hicpp-named-parameter,readability-named-parameter)
int main(int, char const *[])
{
  try
  {
      // создание экземпляра std::map<int, int>
      // заполнение 10 элементами, где ключ - это число от 0 до 9, а значение - факториал ключа
    std::map<int, int> stdmap;
    for(int ii = 0; ii < MapSize; ++ii)
    {
        // boost factorial always returns float that must be converted to int
      stdmap[ii] = static_cast<int>(boost::math::factorial<double>(ii));
    }

      // создание экземпляра std::map<int, int> с новым аллокатором, ограниченным 10 элементами
      // заполнение 10 элементами, где ключ - это число от 0 до 9, а значение - факториал ключа
      // вывод на экран всех значений (ключ и значение разделены пробелом) хранящихся в контейнере
    std::cout << "std::map<int, int, std::less<>, custom_allocator<std::pair<const int,int>, MaxAllocationSize>>:\n";
    std::map<int, int, std::less<>, custom_allocator<std::pair<const int,int>, MaxAllocationSize>> myalloc_map;
    for(auto && mi : stdmap)
    {
      myalloc_map[mi.first] = mi.second;
    }
    for(auto && mi : myalloc_map)
    {
      std::cout << mi.first << " " << mi.second << "\n";
    }

      // создание экземпляра своего контейнера для хранения значений типа int
      // заполнение 10 элементами от 0 до 9
    custom_list<int> mycont;
    for(int ii = 0; ii < MapSize; ++ii)
    {
      mycont.emplace_back(ii);
    }

      // создание экземпляра своего контейнера для хранения значений типа int с новым аллокатором, ограниченным 10 элементами
      // заполнение 10 элементами от 0 до 9
      // - вывод на экран всех значений, хранящихся в контейнере
    std::cout << "custom_list<int, custom_allocator<int, MaxAllocationSize>>:\n";
    custom_list<int, custom_allocator<int, MaxAllocationSize>> myalloc_mycont;
    for(int ii = 0; ii < MapSize; ++ii)
    {
      myalloc_mycont.emplace_back(ii);
    }
    for(auto && mi : myalloc_mycont)
    {
      std::cout << mi << "\n";
    }

      // тест конструктора копирования
    std::cout << "test copy ctor for custom_list<int, custom_allocator<int, MaxAllocationSize>>:\n";
    auto cont_copy(myalloc_mycont);
    for(auto && mi : cont_copy)
    {
      std::cout << mi << "\n";
    }
    std::cout << "size is " << cont_copy.size() << "\n";

      // тест erase
    auto it = cont_copy.begin();
    ++it;
    ++it;
    it = cont_copy.erase(it);
    it = cont_copy.erase(it);
    it = cont_copy.erase(it);

    std::cout << "erase 3 elements: 2,3,4:\n";
    for(auto && mi : cont_copy)
    {
      std::cout << mi << "\n";
    }
      // тест size()
    std::cout << "size is " << cont_copy.size() << "\n";


      // тест оператора присвоения
    std::cout << "test copy assign for custom_list<int, custom_allocator<int, MaxAllocationSize>>:\n";
    auto cont_assign = cont_copy;
    for(auto && mi : cont_assign)
    {
      std::cout << mi << "\n";
    }
    std::cout << "size is " << cont_assign.size() << "\n";

      // тест оператора move
    std::cout << "test move assign for custom_list<int, custom_allocator<int, MaxAllocationSize>>:\n";
    auto cont_move_assign = std::move(cont_assign);
    for(auto && mi : cont_move_assign)
    {
      std::cout << mi << "\n";
    }
    std::cout << "size is " << cont_move_assign.size() << "\n";

  }
  catch(const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return generic_errorcode;
  }
  return 0;
}
