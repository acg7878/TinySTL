#include <gtest/gtest.h>
#include <mystl/iterator.h>
#include <mystl/memory.h>
#include <mystl/type_traits.h>
#include <mystl/utility.h>
#include <mystl/vector.h>

// 测试 is_integral
TEST(TypeTraitsTest, IsIntegral) {
  EXPECT_TRUE(mystl::is_integral<int>::value);
  EXPECT_TRUE(mystl::is_integral<char>::value);
  EXPECT_TRUE(mystl::is_integral<long>::value);
  EXPECT_FALSE(mystl::is_integral<float>::value);
  EXPECT_FALSE(mystl::is_integral<double>::value);
}

// 测试 is_same
TEST(TypeTraitsTest, IsSame) {
  EXPECT_TRUE((mystl::is_same<int, int>::value));
  EXPECT_FALSE((mystl::is_same<int, unsigned int>::value));
  EXPECT_FALSE((mystl::is_same<int, const int>::value));
}

// 测试 remove_reference
TEST(TypeTraitsTest, RemoveReference) {
  EXPECT_TRUE((mystl::is_same<mystl::remove_reference<int>::type, int>::value));
  EXPECT_TRUE(
      (mystl::is_same<mystl::remove_reference<int &>::type, int>::value));
  EXPECT_TRUE(
      (mystl::is_same<mystl::remove_reference<int &&>::type, int>::value));
}

TEST(TypeTraitsTest, IsConvertible) {
  // EXPECT_TRUE(mystl::is_convertible<int, double>::value)
}