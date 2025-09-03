#include "gtest/gtest.h"
#include <mystl/list.h>

// 测试默认构造函数
TEST(ListTest, DefaultConstructor) {
  mystl::list<int> l;
  EXPECT_EQ(l.size(), 0);
  EXPECT_TRUE(l.empty());
}

// 测试带大小和值的构造函数
TEST(ListTest, SizeValueConstructor) {
  mystl::list<int> l(5, 42);
  EXPECT_EQ(l.size(), 5);
  EXPECT_FALSE(l.empty());
  mystl::list<int>::iterator it = l.begin();
  for (int i = 0; i < 5; ++i, ++it) {
    EXPECT_EQ(*it, 42);
  }
}

// 测试拷贝构造函数
TEST(ListTest, CopyConstructor) {
  mystl::list<int> original(5, 42);
  mystl::list<int> copy(original);
  EXPECT_EQ(copy.size(), 5);
  EXPECT_FALSE(copy.empty());
  mystl::list<int>::iterator it = copy.begin();
  for (int i = 0; i < 5; ++i, ++it) {
    EXPECT_EQ(*it, 42);
  }
}

// 测试 push_back
TEST(ListTest, PushBack) {
  mystl::list<int> l;
  l.push_back(1);
  EXPECT_EQ(l.size(), 1);
  EXPECT_FALSE(l.empty());
  mystl::list<int>::iterator it = l.begin();
  EXPECT_EQ(*it, 1);
  l.push_back(2);
  EXPECT_EQ(l.size(), 2);
  EXPECT_EQ(*(++it), 2);
}

// 测试 empty 函数
TEST(ListTest, Empty) {
  mystl::list<int> l;
  EXPECT_TRUE(l.empty());
  l.push_back(1);
  EXPECT_FALSE(l.empty());
}

// 测试 size 函数
TEST(ListTest, Size) {
  mystl::list<int> l;
  EXPECT_EQ(l.size(), 0);
  l.push_back(1);
  EXPECT_EQ(l.size(), 1);
  l.push_back(2);
  EXPECT_EQ(l.size(), 2);
}

// 测试 begin 和 end 函数
TEST(ListTest, BeginEnd) {
  mystl::list<int> l;
  l.push_back(1);
  l.push_back(2);
  mystl::list<int>::iterator it = l.begin();
  EXPECT_EQ(*it, 1);
  it++;
  EXPECT_EQ(*it, 2);
  it++;
  EXPECT_EQ(it, l.end());
}

// 测试迭代器
TEST(ListTest, Iterator) {
    mystl::list<int> l;
    l.push_back(1);
    l.push_back(2);
    l.push_back(3);

    mystl::list<int>::iterator it = l.begin();
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*(++it), 2);
    EXPECT_EQ(*(++it), 3);
    --it;
    EXPECT_EQ(*it, 2);
    EXPECT_NE(it, l.end());
    ++it;
    ++it;
    EXPECT_EQ(it, l.end());
}