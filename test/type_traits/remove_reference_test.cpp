#include <gtest/gtest.h>
#include <mystl/type_traits.h>

TEST(TypeTraitsTest, RemoveReference) {
  EXPECT_TRUE((mystl::is_same<mystl::remove_reference<int>::type, int>::value));
  EXPECT_TRUE(
      (mystl::is_same<mystl::remove_reference<int &>::type, int>::value));
  EXPECT_TRUE(
      (mystl::is_same<mystl::remove_reference<int &&>::type, int>::value));
}