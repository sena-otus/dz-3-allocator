#include <gtest/gtest.h>

#include "custom_allocator.h"
#include "custom_list.h"
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

  TEST(Google_test, test_alloc) {
    custom_list<int, custom_allocator<int, 3>> cont;
    for(int ii = 0; ii < 10; ++ii)
    {
      cont.emplace_back(ii);
    }
    EXPECT_EQ(10, cont.size());

    auto cont_copy(cont);

    EXPECT_EQ(cont, cont_copy);

    auto it = cont_copy.begin();
    ++it;
    ++it;
    it = cont_copy.erase(it);
    it = cont_copy.erase(it);
    it = cont_copy.erase(it);

    EXPECT_EQ(7, cont_copy.size());

    EXPECT_EQ(5, *it);

    auto cont_assign = cont;
    EXPECT_EQ(cont_assign, cont);
    auto cont_move_assign = std::move(cont_assign);
    auto cont_move(std::move(cont_move_assign));
    EXPECT_EQ(cont_move, cont);
  }
}
