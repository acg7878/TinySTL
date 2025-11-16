#include "gtest/gtest.h"
#include <vector>
#include <mystl/__memory/split_buffer.h>
namespace mystl {

// 测试默认构造函数
TEST(SplitBufferTest, DefaultConstructor) {
  split_buffer<int> buf;
  EXPECT_EQ(buf.size(), 0);
  EXPECT_TRUE(buf.empty());
  EXPECT_EQ(buf.capacity(), 0);
  EXPECT_EQ(buf.front_spare(), 0);
  EXPECT_EQ(buf.back_spare(), 0);
}

// 测试带分配器的构造函数
TEST(SplitBufferTest, AllocatorConstructor) {
  std::allocator<int> alloc;
  split_buffer<int> buf(alloc);
  EXPECT_EQ(buf.size(), 0);
  EXPECT_TRUE(buf.empty());
}

// 测试带容量和起始位置的构造函数
TEST(SplitBufferTest, CapacityStartConstructor) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 3, alloc);
  EXPECT_EQ(buf.capacity(), 10);
  EXPECT_EQ(buf.size(), 0);
  EXPECT_EQ(buf.front_spare(), 3);
  EXPECT_EQ(buf.back_spare(), 7);
}

// 测试移动构造函数
TEST(SplitBufferTest, MoveConstructor) {
  std::allocator<int> alloc;
  split_buffer<int> buf1(10, 2, alloc);
  buf1.emplace_back(1);
  buf1.emplace_back(2);
  buf1.emplace_back(3);

  split_buffer<int> buf2(std::move(buf1));
  EXPECT_EQ(buf2.size(), 3);
  EXPECT_EQ(buf2.front(), 1);
  EXPECT_EQ(buf2.back(), 3);
  EXPECT_EQ(buf1.size(), 0);  // 移动后原对象应该为空
}

// 测试移动赋值
TEST(SplitBufferTest, MoveAssignment) {
  std::allocator<int> alloc;
  split_buffer<int> buf1(10, 2, alloc);
  buf1.emplace_back(10);
  buf1.emplace_back(20);

  split_buffer<int> buf2(5, 1, alloc);
  buf2 = std::move(buf1);
  EXPECT_EQ(buf2.size(), 2);
  EXPECT_EQ(buf2.front(), 10);
  EXPECT_EQ(buf2.back(), 20);
  EXPECT_EQ(buf1.size(), 0);
}

// 测试 emplace_back
TEST(SplitBufferTest, EmplaceBack) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 2, alloc);
  
  buf.emplace_back(1);
  EXPECT_EQ(buf.size(), 1);
  EXPECT_EQ(buf.front(), 1);
  EXPECT_EQ(buf.back(), 1);
  
  buf.emplace_back(2);
  buf.emplace_back(3);
  EXPECT_EQ(buf.size(), 3);
  EXPECT_EQ(buf.front(), 1);
  EXPECT_EQ(buf.back(), 3);
}

// 测试 emplace_front
TEST(SplitBufferTest, EmplaceFront) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 2, alloc);
  
  buf.emplace_front(3);
  EXPECT_EQ(buf.size(), 1);
  EXPECT_EQ(buf.front(), 3);
  EXPECT_EQ(buf.back(), 3);
  
  buf.emplace_front(2);
  buf.emplace_front(1);
  EXPECT_EQ(buf.size(), 3);
  EXPECT_EQ(buf.front(), 1);
  EXPECT_EQ(buf.back(), 3);
}

// 测试前后端同时插入
TEST(SplitBufferTest, FrontAndBackInsert) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 5, alloc);
  
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_front(0);
  buf.emplace_front(-1);
  
  EXPECT_EQ(buf.size(), 4);
  EXPECT_EQ(buf.front(), -1);
  EXPECT_EQ(buf.back(), 2);
}

// 测试 pop_back
TEST(SplitBufferTest, PopBack) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 2, alloc);
  
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_back(3);
  
  buf.pop_back();
  EXPECT_EQ(buf.size(), 2);
  EXPECT_EQ(buf.back(), 2);
  
  buf.pop_back();
  EXPECT_EQ(buf.size(), 1);
  EXPECT_EQ(buf.back(), 1);
}

// 测试 pop_front
TEST(SplitBufferTest, PopFront) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 2, alloc);
  
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_back(3);
  
  buf.pop_front();
  EXPECT_EQ(buf.size(), 2);
  EXPECT_EQ(buf.front(), 2);
  
  buf.pop_front();
  EXPECT_EQ(buf.size(), 1);
  EXPECT_EQ(buf.front(), 3);
}

// 测试 clear
TEST(SplitBufferTest, Clear) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 2, alloc);
  
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_back(3);
  
  EXPECT_EQ(buf.size(), 3);
  buf.clear();
  EXPECT_EQ(buf.size(), 0);
  EXPECT_TRUE(buf.empty());
}

// 测试 shrink_to_fit
TEST(SplitBufferTest, ShrinkToFit) {
  std::allocator<int> alloc;
  split_buffer<int> buf(20, 5, alloc);
  
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_back(3);
  
  size_t old_capacity = buf.capacity();
  buf.shrink_to_fit();
  // shrink_to_fit 后容量应该小于等于原来的容量
  EXPECT_LE(buf.capacity(), old_capacity);
  EXPECT_EQ(buf.size(), 3);
  EXPECT_EQ(buf.front(), 1);
  EXPECT_EQ(buf.back(), 3);
}

// 测试 front_spare 和 back_spare
TEST(SplitBufferTest, SpareCapacity) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 3, alloc);
  
  EXPECT_EQ(buf.front_spare(), 3);
  EXPECT_EQ(buf.back_spare(), 7);
  EXPECT_EQ(buf.capacity(), 10);
  
  buf.emplace_back(1);
  EXPECT_EQ(buf.front_spare(), 3);
  EXPECT_EQ(buf.back_spare(), 6);
  
  buf.emplace_front(0);
  EXPECT_EQ(buf.front_spare(), 2);
  EXPECT_EQ(buf.back_spare(), 6);
}

// 测试迭代器
TEST(SplitBufferTest, Iterators) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 2, alloc);
  
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_back(3);
  
  auto it = buf.begin();
  EXPECT_EQ(*it, 1);
  ++it;
  EXPECT_EQ(*it, 2);
  ++it;
  EXPECT_EQ(*it, 3);
  ++it;
  EXPECT_EQ(it, buf.end());
  
  // 测试范围 for 循环
  int sum = 0;
  for (int val : buf) {
    sum += val;
  }
  EXPECT_EQ(sum, 6);
}

// 测试 construct_at_end
TEST(SplitBufferTest, ConstructAtEnd) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 2, alloc);
  
  buf.construct_at_end(3);
  EXPECT_EQ(buf.size(), 3);
  
  buf.construct_at_end(2, 42);
  EXPECT_EQ(buf.size(), 5);
  EXPECT_EQ(buf.back(), 42);
}

// 测试 construct_at_end with iterator range
TEST(SplitBufferTest, ConstructAtEndRange) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 2, alloc);
  
  std::vector<int> vec = {1, 2, 3, 4, 5};
  buf.construct_at_end(vec.begin(), vec.end());
  EXPECT_EQ(buf.size(), 5);
  EXPECT_EQ(buf.front(), 1);
  EXPECT_EQ(buf.back(), 5);
}

// 测试扩容（前后端）
TEST(SplitBufferTest, CapacityGrowth) {
  std::allocator<int> alloc;
  split_buffer<int> buf(2, 0, alloc);
  
  // 填充到容量满
  buf.emplace_back(1);
  buf.emplace_back(2);
  
  // 继续插入应该触发扩容
  buf.emplace_back(3);
  EXPECT_GE(buf.capacity(), 2);
  EXPECT_EQ(buf.size(), 3);
  EXPECT_EQ(buf.back(), 3);
  
  // 测试前端扩容
  split_buffer<int> buf2(2, 0, alloc);
  buf2.emplace_front(1);
  buf2.emplace_front(2);
  buf2.emplace_front(3);
  EXPECT_GE(buf2.capacity(), 2);
  EXPECT_EQ(buf2.size(), 3);
  EXPECT_EQ(buf2.front(), 3);
}

// 测试复杂场景：前后端交替插入删除
TEST(SplitBufferTest, ComplexOperations) {
  std::allocator<int> alloc;
  split_buffer<int> buf(10, 5, alloc);
  
  // 前端插入
  buf.emplace_front(1);
  buf.emplace_front(2);
  
  // 后端插入
  buf.emplace_back(3);
  buf.emplace_back(4);
  
  EXPECT_EQ(buf.size(), 4);
  EXPECT_EQ(buf.front(), 2);
  EXPECT_EQ(buf.back(), 4);
  
  // 前端删除
  buf.pop_front();
  EXPECT_EQ(buf.front(), 1);
  
  // 后端删除
  buf.pop_back();
  EXPECT_EQ(buf.back(), 3);
  
  EXPECT_EQ(buf.size(), 2);
}

// 测试非平凡类型
struct NonTrivial {
  int value;
  NonTrivial(int v) : value(v) {}
  NonTrivial(const NonTrivial& other) : value(other.value) {}
  NonTrivial& operator=(const NonTrivial& other) {
    value = other.value;
    return *this;
  }
};

TEST(SplitBufferTest, NonTrivialType) {
  std::allocator<NonTrivial> alloc;
  split_buffer<NonTrivial> buf(10, 2, alloc);
  
  buf.emplace_back(1);
  buf.emplace_back(2);
  buf.emplace_front(0);
  
  EXPECT_EQ(buf.size(), 3);
  EXPECT_EQ(buf.front().value, 0);
  EXPECT_EQ(buf.back().value, 2);
}

// 测试 swap
TEST(SplitBufferTest, Swap) {
  std::allocator<int> alloc;
  split_buffer<int> buf1(10, 2, alloc);
  buf1.emplace_back(1);
  buf1.emplace_back(2);
  
  split_buffer<int> buf2(5, 1, alloc);
  buf2.emplace_back(3);
  
  buf1.swap(buf2);
  
  EXPECT_EQ(buf1.size(), 1);
  EXPECT_EQ(buf1.front(), 3);
  EXPECT_EQ(buf2.size(), 2);
  EXPECT_EQ(buf2.front(), 1);
}

}  // namespace mystl

