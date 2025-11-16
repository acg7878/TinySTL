#include "gtest/gtest.h"
#include <mystl/deque.h>
#include <vector>
#include <stdexcept>

// 测试默认构造函数
TEST(DequeTest, DefaultConstructor) {
  mystl::deque<int> d;
  EXPECT_EQ(d.size(), 0);
  EXPECT_TRUE(d.empty());
  EXPECT_GE(d.max_size(), 0);
}

// 测试带分配器的构造函数
TEST(DequeTest, AllocatorConstructor) {
  std::allocator<int> alloc;
  mystl::deque<int> d(alloc);
  EXPECT_EQ(d.size(), 0);
  EXPECT_TRUE(d.empty());
}

// 测试带大小的构造函数
TEST(DequeTest, SizeConstructor) {
  mystl::deque<int> d(5);
  EXPECT_EQ(d.size(), 5);
  EXPECT_FALSE(d.empty());
}

// 测试带大小和值的构造函数
TEST(DequeTest, SizeValueConstructor) {
  mystl::deque<int> d(5, 42);
  EXPECT_EQ(d.size(), 5);
  for (size_t i = 0; i < d.size(); ++i) {
    EXPECT_EQ(d[i], 42);
  }
}

// 测试迭代器范围构造函数
TEST(DequeTest, RangeConstructor) {
  std::vector<int> vec = {1, 2, 3, 4, 5};
  mystl::deque<int> d(vec.begin(), vec.end());
  EXPECT_EQ(d.size(), 5);
  for (size_t i = 0; i < d.size(); ++i) {
    EXPECT_EQ(d[i], vec[i]);
  }
}

// 测试拷贝构造函数
TEST(DequeTest, CopyConstructor) {
  mystl::deque<int> original;
  original.push_back(1);
  original.push_back(2);
  original.push_back(3);

  mystl::deque<int> copy(original);
  EXPECT_EQ(copy.size(), 3);
  EXPECT_EQ(copy[0], 1);
  EXPECT_EQ(copy[1], 2);
  EXPECT_EQ(copy[2], 3);
}

// 测试移动构造函数
TEST(DequeTest, MoveConstructor) {
  mystl::deque<int> original;
  original.push_back(1);
  original.push_back(2);
  original.push_back(3);

  mystl::deque<int> moved(std::move(original));
  EXPECT_EQ(moved.size(), 3);
  EXPECT_EQ(moved[0], 1);
  EXPECT_EQ(moved[1], 2);
  EXPECT_EQ(moved[2], 3);
  EXPECT_EQ(original.size(), 0);
}

// 测试初始化列表构造函数
TEST(DequeTest, InitializerListConstructor) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  EXPECT_EQ(d.size(), 5);
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(d[i], i + 1);
  }
}

// 测试拷贝赋值
TEST(DequeTest, CopyAssignment) {
  mystl::deque<int> d1 = {1, 2, 3};
  mystl::deque<int> d2 = {4, 5, 6, 7};
  d2 = d1;
  EXPECT_EQ(d2.size(), 3);
  EXPECT_EQ(d2[0], 1);
  EXPECT_EQ(d2[1], 2);
  EXPECT_EQ(d2[2], 3);
}

// 测试移动赋值
TEST(DequeTest, MoveAssignment) {
  mystl::deque<int> d1 = {1, 2, 3};
  mystl::deque<int> d2;
  d2 = std::move(d1);
  EXPECT_EQ(d2.size(), 3);
  EXPECT_EQ(d2[0], 1);
  EXPECT_EQ(d2[1], 2);
  EXPECT_EQ(d2[2], 3);
  EXPECT_EQ(d1.size(), 0);
}

// 测试初始化列表赋值
TEST(DequeTest, InitializerListAssignment) {
  mystl::deque<int> d;
  d = {1, 2, 3, 4, 5};
  EXPECT_EQ(d.size(), 5);
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(d[i], i + 1);
  }
}

// 测试 assign
TEST(DequeTest, Assign) {
  mystl::deque<int> d = {1, 2, 3};
  d.assign(5, 42);
  EXPECT_EQ(d.size(), 5);
  for (size_t i = 0; i < d.size(); ++i) {
    EXPECT_EQ(d[i], 42);
  }
}

// 测试 assign 迭代器范围
TEST(DequeTest, AssignRange) {
  mystl::deque<int> d = {1, 2, 3};
  std::vector<int> vec = {10, 20, 30, 40};
  d.assign(vec.begin(), vec.end());
  EXPECT_EQ(d.size(), 4);
  EXPECT_EQ(d[0], 10);
  EXPECT_EQ(d[1], 20);
  EXPECT_EQ(d[2], 30);
  EXPECT_EQ(d[3], 40);
}

// 测试迭代器
TEST(DequeTest, Iterators) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  
  // 测试 begin/end
  auto it = d.begin();
  EXPECT_EQ(*it, 1);
  ++it;
  EXPECT_EQ(*it, 2);
  
  // 测试范围 for 循环
  int sum = 0;
  for (int val : d) {
    sum += val;
  }
  EXPECT_EQ(sum, 15);
  
  // 测试 const 迭代器
  const mystl::deque<int>& cd = d;
  auto cit = cd.begin();
  EXPECT_EQ(*cit, 1);
}

// 测试反向迭代器
TEST(DequeTest, ReverseIterators) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  auto rit = d.rbegin();
  EXPECT_EQ(*rit, 5);
  ++rit;
  EXPECT_EQ(*rit, 4);
  
  int sum = 0;
  for (auto it = d.rbegin(); it != d.rend(); ++it) {
    sum += *it;
  }
  EXPECT_EQ(sum, 15);
}

// 测试 operator[]
TEST(DequeTest, SubscriptOperator) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[2], 3);
  EXPECT_EQ(d[4], 5);
  
  d[0] = 10;
  EXPECT_EQ(d[0], 10);
}

// 测试 at
TEST(DequeTest, At) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  EXPECT_EQ(d.at(0), 1);
  EXPECT_EQ(d.at(2), 3);
  EXPECT_EQ(d.at(4), 5);
  
  d.at(0) = 10;
  EXPECT_EQ(d.at(0), 10);
  
  EXPECT_THROW(d.at(10), std::out_of_range);
}

// 测试 front 和 back
TEST(DequeTest, FrontAndBack) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  EXPECT_EQ(d.front(), 1);
  EXPECT_EQ(d.back(), 5);
  
  d.front() = 10;
  d.back() = 50;
  EXPECT_EQ(d.front(), 10);
  EXPECT_EQ(d.back(), 50);
}

// 测试 push_back
TEST(DequeTest, PushBack) {
  mystl::deque<int> d;
  d.push_back(1);
  EXPECT_EQ(d.size(), 1);
  EXPECT_EQ(d.back(), 1);
  
  d.push_back(2);
  d.push_back(3);
  EXPECT_EQ(d.size(), 3);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
  EXPECT_EQ(d[2], 3);
}

// 测试 push_front
TEST(DequeTest, PushFront) {
  mystl::deque<int> d;
  d.push_front(1);
  EXPECT_EQ(d.size(), 1);
  EXPECT_EQ(d.front(), 1);
  
  d.push_front(2);
  d.push_front(3);
  EXPECT_EQ(d.size(), 3);
  EXPECT_EQ(d[0], 3);
  EXPECT_EQ(d[1], 2);
  EXPECT_EQ(d[2], 1);
}

// 测试前后端同时插入
TEST(DequeTest, PushFrontAndBack) {
  mystl::deque<int> d;
  d.push_back(1);
  d.push_back(2);
  d.push_front(0);
  d.push_front(-1);
  
  EXPECT_EQ(d.size(), 4);
  EXPECT_EQ(d[0], -1);
  EXPECT_EQ(d[1], 0);
  EXPECT_EQ(d[2], 1);
  EXPECT_EQ(d[3], 2);
}

// 测试 emplace_back
TEST(DequeTest, EmplaceBack) {
  mystl::deque<int> d;
  d.emplace_back(1);
  d.emplace_back(2);
  d.emplace_back(3);
  
  EXPECT_EQ(d.size(), 3);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
  EXPECT_EQ(d[2], 3);
}

// 测试 emplace_front
TEST(DequeTest, EmplaceFront) {
  mystl::deque<int> d;
  d.emplace_front(3);
  d.emplace_front(2);
  d.emplace_front(1);
  
  EXPECT_EQ(d.size(), 3);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
  EXPECT_EQ(d[2], 3);
}

// 测试 pop_back
TEST(DequeTest, PopBack) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  d.pop_back();
  EXPECT_EQ(d.size(), 4);
  EXPECT_EQ(d.back(), 4);
  
  d.pop_back();
  EXPECT_EQ(d.size(), 3);
  EXPECT_EQ(d.back(), 3);
}

// 测试 pop_front
TEST(DequeTest, PopFront) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  d.pop_front();
  EXPECT_EQ(d.size(), 4);
  EXPECT_EQ(d.front(), 2);
  
  d.pop_front();
  EXPECT_EQ(d.size(), 3);
  EXPECT_EQ(d.front(), 3);
}

// 测试 clear
TEST(DequeTest, Clear) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  EXPECT_EQ(d.size(), 5);
  d.clear();
  EXPECT_EQ(d.size(), 0);
  EXPECT_TRUE(d.empty());
}

// 测试 resize
TEST(DequeTest, Resize) {
  mystl::deque<int> d = {1, 2, 3};
  d.resize(5);
  EXPECT_EQ(d.size(), 5);
  
  d.resize(2);
  EXPECT_EQ(d.size(), 2);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
}

// 测试 resize 带值
TEST(DequeTest, ResizeWithValue) {
  mystl::deque<int> d = {1, 2, 3};
  d.resize(5, 42);
  EXPECT_EQ(d.size(), 5);
  EXPECT_EQ(d[3], 42);
  EXPECT_EQ(d[4], 42);
  
  d.resize(2, 99);
  EXPECT_EQ(d.size(), 2);
}

// 测试 swap
TEST(DequeTest, Swap) {
  mystl::deque<int> d1 = {1, 2, 3};
  mystl::deque<int> d2 = {4, 5, 6, 7};
  
  d1.swap(d2);
  EXPECT_EQ(d1.size(), 4);
  EXPECT_EQ(d2.size(), 3);
  EXPECT_EQ(d1[0], 4);
  EXPECT_EQ(d2[0], 1);
}

// 测试复杂场景：前后端交替操作
TEST(DequeTest, ComplexOperations) {
  mystl::deque<int> d;
  
  // 前端插入
  d.push_front(1);
  d.push_front(2);
  
  // 后端插入
  d.push_back(3);
  d.push_back(4);
  
  EXPECT_EQ(d.size(), 4);
  EXPECT_EQ(d[0], 2);
  EXPECT_EQ(d[1], 1);
  EXPECT_EQ(d[2], 3);
  EXPECT_EQ(d[3], 4);
  
  // 前端删除
  d.pop_front();
  EXPECT_EQ(d.front(), 1);
  
  // 后端删除
  d.pop_back();
  EXPECT_EQ(d.back(), 3);
  
  EXPECT_EQ(d.size(), 2);
}

// 测试大量元素
TEST(DequeTest, LargeSize) {
  mystl::deque<int> d;
  const int N = 1000;
  
  for (int i = 0; i < N; ++i) {
    d.push_back(i);
  }
  
  EXPECT_EQ(d.size(), N);
  EXPECT_EQ(d[0], 0);
  EXPECT_EQ(d[N-1], N-1);
  
  for (int i = 0; i < N; ++i) {
    EXPECT_EQ(d[i], i);
  }
}

// 测试前后端大量插入
TEST(DequeTest, LargeFrontAndBackInsert) {
  mystl::deque<int> d;
  const int N = 500;
  
  // 前端插入
  for (int i = 0; i < N; ++i) {
    d.push_front(i);
  }
  
  // 后端插入
  for (int i = 0; i < N; ++i) {
    d.push_back(i + N);
  }
  
  EXPECT_EQ(d.size(), 2 * N);
  EXPECT_EQ(d[0], N-1);        // 第一个前端插入的元素（最后插入的）
  EXPECT_EQ(d[N-1], 0);        // 最后一个前端插入的元素（第一个插入的）
  EXPECT_EQ(d[N], N);          // 第一个后端插入的元素
  EXPECT_EQ(d[2*N-1], 2*N-1);  // 最后一个后端插入的元素
}

// 测试非平凡类型
struct NonTrivial {
  int value;
  NonTrivial(int v = 0) : value(v) {}
  NonTrivial(const NonTrivial& other) : value(other.value) {}
  NonTrivial& operator=(const NonTrivial& other) {
    value = other.value;
    return *this;
  }
  bool operator==(const NonTrivial& other) const {
    return value == other.value;
  }
};

TEST(DequeTest, NonTrivialType) {
  mystl::deque<NonTrivial> d;
  d.push_back(NonTrivial(1));
  d.push_back(NonTrivial(2));
  d.push_front(NonTrivial(0));
  
  EXPECT_EQ(d.size(), 3);
  EXPECT_EQ(d[0].value, 0);
  EXPECT_EQ(d[1].value, 1);
  EXPECT_EQ(d[2].value, 2);
}

// 测试 get_allocator
TEST(DequeTest, GetAllocator) {
  std::allocator<int> alloc;
  mystl::deque<int> d(alloc);
  auto d_alloc = d.get_allocator();
  // 基本检查，确保可以获取分配器
  EXPECT_TRUE(true);
}

// 测试 max_size
TEST(DequeTest, MaxSize) {
  mystl::deque<int> d;
  EXPECT_GT(d.max_size(), 0);
}

// 测试空 deque 的操作
TEST(DequeTest, EmptyDequeOperations) {
  mystl::deque<int> d;
  EXPECT_TRUE(d.empty());
  EXPECT_EQ(d.size(), 0);
  
  // 清空空 deque 应该没问题
  d.clear();
  EXPECT_TRUE(d.empty());
}

// 测试迭代器算术运算
TEST(DequeTest, IteratorArithmetic) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  
  auto it1 = d.begin();
  auto it2 = it1 + 3;
  EXPECT_EQ(*it2, 4);
  
  auto it3 = it2 - 1;
  EXPECT_EQ(*it3, 3);
  
  EXPECT_EQ(it2 - it1, 3);
  
  it1 += 2;
  EXPECT_EQ(*it1, 3);
  
  it1 -= 1;
  EXPECT_EQ(*it1, 2);
}

// 测试迭代器比较
TEST(DequeTest, IteratorComparison) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  
  auto it1 = d.begin();
  auto it2 = d.begin() + 2;
  auto it3 = d.end();
  
  EXPECT_TRUE(it1 < it2);
  EXPECT_TRUE(it2 < it3);
  EXPECT_TRUE(it1 != it2);
  EXPECT_TRUE(it1 == d.begin());
}

// 测试迭代器下标访问
TEST(DequeTest, IteratorSubscript) {
  mystl::deque<int> d = {1, 2, 3, 4, 5};
  auto it = d.begin();
  EXPECT_EQ(it[0], 1);
  EXPECT_EQ(it[2], 3);
  EXPECT_EQ(it[4], 5);
}

