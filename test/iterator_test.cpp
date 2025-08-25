#include "gtest/gtest.h"
#include <mystl/iterator.h>
#include <mystl/vector.h>
#include <type_traits>

// 测试 iterator_traits
TEST(IteratorTest, IteratorTraits) {
  // 测试原生指针
  using traits_ptr = mystl::iterator_traits<int *>;
  bool is_random_access_ptr =
      std::is_same<traits_ptr::iterator_category,
                   mystl::random_access_iterator_tag>::value;
  EXPECT_TRUE(is_random_access_ptr);

  // 测试 const 原生指针
  using traits_const_ptr = mystl::iterator_traits<const int *>;
  bool is_random_access_const_ptr =
      std::is_same<traits_const_ptr::iterator_category,
                   mystl::random_access_iterator_tag>::value;
  EXPECT_TRUE(is_random_access_const_ptr);
  bool is_value_type_int =
      std::is_same<traits_const_ptr::value_type, int>::value;
  EXPECT_TRUE(is_value_type_int); // value_type 不应该是 const

  // 测试 mystl::vector 的迭代器
  using vec_iter = mystl::vector<int>::iterator;
  using traits_vec_iter = mystl::iterator_traits<vec_iter>;
  bool is_random_access_vec =
      std::is_same<traits_vec_iter::iterator_category,
                   mystl::random_access_iterator_tag>::value;
  EXPECT_TRUE(is_random_access_vec);
}

// 测试 distance
TEST(IteratorTest, Distance) {
  mystl::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(4);
  v.push_back(5);

  auto first = v.begin();
  auto last = v.end();
  EXPECT_EQ(mystl::distance(first, last), 5);
  EXPECT_EQ(mystl::distance(first, first), 0);
}

// 测试 advance
TEST(IteratorTest, Advance) {
  mystl::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(4);
  v.push_back(5);

  auto it = v.begin();
  mystl::advance(it, 3);
  EXPECT_EQ(*it, 4);
  mystl::advance(it, -2);
  EXPECT_EQ(*it, 2);
}