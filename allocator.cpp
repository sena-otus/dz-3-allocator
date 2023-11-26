#include "custom_allocator.h"
#include "custom_list.h"

#include <boost/math/special_functions/factorials.hpp>
#include <iostream>
#include <map>

const int generic_errorcode = 102;

const std::size_t MapSize = 10;
const std::size_t MinAllocationSize = MapSize;





// NOLINTNEXTLINE(hicpp-named-parameter,readability-named-parameter)
int main(int, char const *[])
{
  try
  {
      // создание экземпляра std::map<int, int>
    std::map<int, int> stdmap;

      // заполнение 10 элементами, где ключ - это число от 0 до 9, а значение - факториал ключа
    for(unsigned ii = 0; ii < MapSize; ++ii)
    {
        // boost factorial always returns float that must be converted to int
      stdmap[ii] = static_cast<int>(boost::math::factorial<double>(ii));
    }


      // - создание экземпляра std::map<int, int> с новым аллокатором, ограниченным 10 элементами
    std::map<int, int, std::less<>, custom_allocator<std::pair<const int,int>, MinAllocationSize>> myalloc_map;


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
    for(unsigned ii = 0; ii < MapSize; ++ii)
    {
      mycont.push_back(ii);
    }

      // - создание экземпляра своего контейнера для хранения значений типа int с новым аллокатором, ограниченным 10 элементами
    custom_list<int, custom_allocator<int, MinAllocationSize>> myalloc_mycont;

      // - заполнение 10 элементами от 0 до 9
    for(unsigned ii = 0; ii < MapSize; ++ii)
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
