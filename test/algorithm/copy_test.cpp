#include "gtest/gtest.h"
#include <mystl/algorithm.h>
#include <mystl/vector.h>
#include <string>

// 用于测试非 POD 类型的辅助结构体
struct NonPod {
  int value;
  std::string name;

  NonPod(int v = 0, std::string n = "") : value(v), name(n) {}

  // 使其非 POD
  NonPod(const NonPod &other) : value(other.value), name(other.name) {}
  NonPod &operator=(const NonPod &other) {
    value = other.value;
    name = other.name;
    return *this;
  }

  bool operator==(const NonPod &other) const {
    return value == other.value && name == other.name;
  }
};

// 测试 mystl::copy 使用 mystl::vector (随机访问迭代器)
TEST(CopyTest, WithVector) {
  mystl::vector<int> source;
  source.push_back(1);
  source.push_back(2);
  source.push_back(3);

  mystl::vector<int> dest(3, 0);
  mystl::copy(source.begin(), source.end(), dest.begin());

  EXPECT_EQ(dest.size(), 3);
  EXPECT_EQ(dest[0], 1);
  EXPECT_EQ(dest[1], 2);
  EXPECT_EQ(dest[2], 3);
}

// 测试 mystl::copy 使用原生指针 (POD 类型优化)
TEST(CopyTest, WithRawPointerPod) {
  int source[] = {10, 20, 30, 40};
  int dest[4];

  mystl::copy(source, source + 4, dest);

  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(dest[i], source[i]);
  }
}

// 测试 mystl::copy 使用原生指针 (非 POD 类型)
TEST(CopyTest, WithRawPointerNonPod) {
  NonPod source[2];
  source[0] = NonPod(1, "one");
  source[1] = NonPod(2, "two");

  NonPod dest[2];
  mystl::copy(source, source + 2, dest);

  EXPECT_EQ(dest[0], source[0]);
  EXPECT_EQ(dest[1], source[1]);
}

// 测试 mystl::copy 返回正确的迭代器
TEST(CopyTest, ReturnIterator) {
  mystl::vector<int> source;
  source.push_back(1);
  source.push_back(2);

  mystl::vector<int> dest(5, 0); // 目标比源大
  auto it = mystl::copy(source.begin(), source.end(), dest.begin());

  EXPECT_EQ(*it, 0); // 应该指向第三个元素
  EXPECT_EQ(it, dest.begin() + 2);
}