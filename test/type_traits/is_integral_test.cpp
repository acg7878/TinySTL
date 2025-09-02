#include <gtest/gtest.h>
#include <mystl/type_traits.h>

TEST(TypeTraitsTest, IsIntegral) {
  EXPECT_TRUE(mystl::is_integral<int>::value);
  EXPECT_TRUE(mystl::is_integral<char>::value);
  EXPECT_TRUE(mystl::is_integral<long>::value);
  EXPECT_FALSE(mystl::is_integral<float>::value);
  EXPECT_FALSE(mystl::is_integral<double>::value);
}