#include <boost/math/special_functions/factorials.hpp>
#include <iostream>
#include <stdexcept>
#include <map>
#include <vector>

const int generic_errorcode = 102;
const int mapsize = 10;

// NOLINTNEXTLINE(hicpp-named-parameter,readability-named-parameter)
int main(int, char const *[])
{
  try
  {
// создание экземпляра std::map<int, int>
    std::map<int, int> stdmap;

// заполнение 10 элементами, где ключ - это число от 0 до 9, а значение - факториал ключа
    for(int ii = 0; ii < mapsize; ++ii)
    {
        // boost factorial always returns float that must be converted to int
      stdmap[ii] = static_cast<int>(boost::math::factorial<double>(ii));
    }


// - создание экземпляра std::map<int, int> с новым аллокатором, ограниченным 10 элементами
    std::map<int, int> myalloc_map;

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
    std::vector<int> mycont;

// - заполнение 10 элементами от 0 до 9
    mycont.reserve(mapsize);
    for(int ii = 0; ii < mapsize; ++ii)
    {
      mycont.push_back(ii);
    }

// - создание экземпляра своего контейнера для хранения значений типа int с новым аллокатором, ограниченным 10 элементами
    std::vector<int> myalloc_mycont;

// - заполнение 10 элементами от 0 до 9
    for(int ii = 0; ii < mapsize; ++ii)
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
