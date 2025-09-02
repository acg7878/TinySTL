#include <gtest/gtest.h>
#include <mystl/type_traits.h>

TEST(TypeTraitsTest, IsSame) {
  EXPECT_TRUE((mystl::is_same<int, int>::value));
  EXPECT_FALSE((mystl::is_same<int, unsigned int>::value));
  EXPECT_FALSE((mystl::is_same<int, const int>::value));
}