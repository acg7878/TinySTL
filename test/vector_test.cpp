#include "gtest/gtest.h"
#include <mystl/vector.h>

// 测试默认构造函数和基本功能
TEST(VectorTest, DefaultConstructor) {
  mystl::vector<int> v;
  EXPECT_EQ(v.size(), 0);
  EXPECT_TRUE(v.empty());
  EXPECT_GE(v.capacity(), 0);
}

// 测试带大小和值的构造函数
TEST(VectorTest, SizeValueConstructor) {
  mystl::vector<int> v(5, 42);
  EXPECT_EQ(v.size(), 5);
  EXPECT_FALSE(v.empty());
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(v[i], 42);
  }
}

// 测试迭代器范围构造函数
TEST(VectorTest, RangeConstructor) {
  int arr[] = {1, 2, 3, 4, 5};
  mystl::vector<int> v(arr, arr + 5);
  EXPECT_EQ(v.size(), 5);
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(v[i], arr[i]);
  }
}

// 测试拷贝构造函数
TEST(VectorTest, CopyConstructor) {
  mystl::vector<int> original;
  original.push_back(1);
  original.push_back(2);

  mystl::vector<int> copy(original);
  EXPECT_EQ(copy.size(), 2);
  EXPECT_EQ(copy[0], 1);
  EXPECT_EQ(copy[1], 2);
  EXPECT_NE(copy.begin(), original.begin()); // 应该是深拷贝
}

// 测试移动构造函数
TEST(VectorTest, MoveConstructor) {
  mystl::vector<int> original;
  original.push_back(1);
  original.push_back(2);
  int *original_data = original.begin();

  mystl::vector<int> moved(mystl::move(original));
  EXPECT_EQ(moved.size(), 2);
  EXPECT_EQ(moved[0], 1);
  EXPECT_EQ(moved.begin(), original_data); // 数据指针应该被移动
  EXPECT_EQ(original.size(), 0);
  EXPECT_TRUE(original.empty());
}

// 测试 push_back 和容量增长
TEST(VectorTest, PushBackAndCapacity) {
  mystl::vector<int> v;
  EXPECT_EQ(v.capacity(), 0);
  v.push_back(1);
  EXPECT_EQ(v.size(), 1);
  EXPECT_EQ(v[0], 1);
  EXPECT_GE(v.capacity(), 1);

  size_t old_capacity = v.capacity();
  for (size_t i = 0; i < old_capacity; ++i) {
    v.push_back(i + 2);
  }
  EXPECT_EQ(v.size(), old_capacity + 1);
  EXPECT_GT(v.capacity(), old_capacity); // 容量应该增长了
}

// 测试 pop_back
TEST(VectorTest, PopBack) {
  mystl::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  v.pop_back();
  EXPECT_EQ(v.size(), 2);
  EXPECT_EQ(v[1], 2);

  v.pop_back();
  v.pop_back();
  EXPECT_TRUE(v.empty());
  v.pop_back(); // 对空 vector pop_back 应该是安全的
  EXPECT_TRUE(v.empty());
}

// 测试 shrink_to_fit
TEST(VectorTest, ShrinkToFit) {
  mystl::vector<int> v;
  for (int i = 0; i < 10; ++i) {
    v.push_back(i);
  }
  v.pop_back();
  v.pop_back();
  v.pop_back(); // size is 7

  size_t capacity_before = v.capacity();
  v.shrink_to_fit();
  EXPECT_EQ(v.size(), 7);
  EXPECT_EQ(v.capacity(), 7);
  EXPECT_LT(v.capacity(), capacity_before);
}

// 测试 swap
TEST(VectorTest, Swap) {
  mystl::vector<int> v1;
  v1.push_back(1);
  v1.push_back(2);

  mystl::vector<int> v2;
  v2.push_back(3);
  v2.push_back(4);
  v2.push_back(5);

  v1.swap(v2);

  EXPECT_EQ(v1.size(), 3);
  EXPECT_EQ(v1[0], 3);
  EXPECT_EQ(v2.size(), 2);
  EXPECT_EQ(v2[0], 1);
}