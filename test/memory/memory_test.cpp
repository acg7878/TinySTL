#include "gtest/gtest.h"
#include <mystl/memory.h>
#include <mystl/vector.h>

// 用于测试构造和析构的辅助结构体
struct Counter {
  static int constructor_calls;
  static int destructor_calls;

  Counter() { constructor_calls++; }
  Counter(const Counter &) { constructor_calls++; }
  ~Counter() { destructor_calls++; }

  static void reset() {
    constructor_calls = 0;
    destructor_calls = 0;
  }
};

int Counter::constructor_calls = 0;
int Counter::destructor_calls = 0;

TEST(MemoryTest, Allocator) {
  mystl::allocator<int> alloc;

  // 测试 allocate 和 deallocate
  int *p = alloc.allocate(10);
  ASSERT_NE(p, nullptr);
  alloc.deallocate(p, 10);

  // 测试 allocate 0
  p = alloc.allocate(0);
  EXPECT_EQ(p, nullptr);
}

TEST(MemoryTest, ConstructDestroy) {
  mystl::allocator<Counter> alloc;
  Counter::reset();

  Counter *p = alloc.allocate(1);
  mystl::construct(p);
  EXPECT_EQ(Counter::constructor_calls, 1);
  EXPECT_EQ(Counter::destructor_calls, 0);

  mystl::destroy(p);
  EXPECT_EQ(Counter::constructor_calls, 1);
  EXPECT_EQ(Counter::destructor_calls, 1);
  alloc.deallocate(p, 1);

  // 测试范围析构
  Counter::reset();
  Counter *arr = alloc.allocate(5);
  for (int i = 0; i < 5; ++i) {
    mystl::construct(arr + i);
  }
  EXPECT_EQ(Counter::constructor_calls, 5);
  mystl::destroy(arr, arr + 5);
  EXPECT_EQ(Counter::destructor_calls, 5);
  alloc.deallocate(arr, 5);
}

TEST(MemoryTest, UninitializedAlgorithms) {
  mystl::allocator<int> alloc;
  int *p = alloc.allocate(5);

  // 测试 uninitialized_fill_n
  mystl::uninitialized_fill_n(p, 5, 42);
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(p[i], 42);
  }
  mystl::destroy(p, p + 5); // 清理，以便重用内存

  // 测试 uninitialized_copy
  mystl::vector<int> v_copy;
  v_copy.push_back(1);
  v_copy.push_back(2);
  v_copy.push_back(3);
  mystl::uninitialized_copy(v_copy.begin(), v_copy.end(), p);
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(p[i], i + 1);
  }
  mystl::destroy(p, p + 3); // 清理

  // 测试 uninitialized_move
  mystl::vector<int> v_move;
  v_move.push_back(10);
  v_move.push_back(20);
  mystl::uninitialized_move(v_move.begin(), v_move.end(), p);
  EXPECT_EQ(p[0], 10);
  EXPECT_EQ(p[1], 20);
  mystl::destroy(p, p + 2); // 清理

  alloc.deallocate(p, 5);
}

// 测试非平凡类型的 uninitialized_copy
TEST(MemoryTest, UninitializedCopyNonTrivial) {
  mystl::allocator<Counter> alloc;
  Counter::reset();

  mystl::vector<Counter> source;
  source.push_back(Counter());
  source.push_back(Counter());
  source.push_back(Counter());
  // push_back 可能会涉及重新分配和移动，所以在这里重置计数器
  Counter::reset();

  Counter *p = alloc.allocate(3);
  mystl::uninitialized_copy(source.begin(), source.end(), p);
  EXPECT_EQ(Counter::constructor_calls, 3); // 3 次拷贝构造

  mystl::destroy(p, p + 3);
  EXPECT_EQ(Counter::destructor_calls, 3);
  alloc.deallocate(p, 3);
}