#include <gtest/gtest.h>
#include <mystl/type_traits.h>

TEST(TypeTraitsTest, IsVoid) {
    EXPECT_TRUE(mystl::is_void<void>::value);
    EXPECT_FALSE(mystl::is_void<int>::value);
    EXPECT_FALSE(mystl::is_void<double>::value);
}