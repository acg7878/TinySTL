#include "gtest/gtest.h"
#include <mystl/unordered_map.h>
#include <string>
#include <vector>
#include <type_traits>

// 测试默认构造函数
TEST(UnorderedMapTest, DefaultConstructor) {
  mystl::unordered_map<int, int> map;
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
  EXPECT_GE(map.bucket_count(), 0);
}

// 测试指定桶数的构造函数
TEST(UnorderedMapTest, BucketCountConstructor) {
  mystl::unordered_map<int, std::string> map(10);
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
  EXPECT_GE(map.bucket_count(), 10);
}

// 测试带哈希函数和比较函数的构造函数
TEST(UnorderedMapTest, HashAndKeyEqualConstructor) {
  mystl::unordered_map<int, int> map(8, std::hash<int>(), std::equal_to<int>());
  EXPECT_EQ(map.size(), 0);
  EXPECT_GE(map.bucket_count(), 8);
}

// 测试初始化列表构造函数
TEST(UnorderedMapTest, InitializerListConstructor) {
  mystl::unordered_map<int, std::string> map{{1, "one"}, {2, "two"}, {3, "three"}};
  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map[1], "one");
  EXPECT_EQ(map[2], "two");
  EXPECT_EQ(map[3], "three");
}

// 测试迭代器范围构造函数
TEST(UnorderedMapTest, RangeConstructor) {
  std::vector<std::pair<int, std::string>> vec = {{1, "one"}, {2, "two"}, {3, "three"}};
  mystl::unordered_map<int, std::string> map(vec.begin(), vec.end());
  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map[1], "one");
  EXPECT_EQ(map[2], "two");
  EXPECT_EQ(map[3], "three");
}

// 测试拷贝构造函数
TEST(UnorderedMapTest, CopyConstructor) {
  mystl::unordered_map<int, std::string> original;
  original[1] = "one";
  original[2] = "two";

  mystl::unordered_map<int, std::string> copy(original);
  EXPECT_EQ(copy.size(), 2);
  EXPECT_EQ(copy[1], "one");
  EXPECT_EQ(copy[2], "two");
  EXPECT_EQ(original.size(), 2);  // 原对象不应被修改
}

// 测试移动构造函数
TEST(UnorderedMapTest, MoveConstructor) {
  mystl::unordered_map<int, std::string> original;
  original[1] = "one";
  original[2] = "two";
  size_t original_size = original.size();

  mystl::unordered_map<int, std::string> moved(std::move(original));
  EXPECT_EQ(moved.size(), original_size);
  EXPECT_EQ(moved[1], "one");
  EXPECT_EQ(moved[2], "two");
  EXPECT_TRUE(original.empty());  // 原对象应该被移动
}

// 测试 operator[] 访问和插入
TEST(UnorderedMapTest, OperatorBracketAccess) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map[1], "one");
  EXPECT_EQ(map[2], "two");

  // 修改已存在的值
  map[1] = "modified";
  EXPECT_EQ(map[1], "modified");
  EXPECT_EQ(map.size(), 2);

  // 访问不存在的键（会插入）
  map[3];
  EXPECT_EQ(map.size(), 3);
}

// 测试 operator[] 右值引用
TEST(UnorderedMapTest, OperatorBracketRvalue) {
  mystl::unordered_map<std::string, int> map;
  std::string key = "test";
  map[std::move(key)] = 42;
  EXPECT_EQ(map["test"], 42);
}

// 测试 at() 访问已存在的键
TEST(UnorderedMapTest, AtAccess) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";

  EXPECT_EQ(map.at(1), "one");
  EXPECT_EQ(map.at(2), "two");

  const mystl::unordered_map<int, std::string>& const_map = map;
  EXPECT_EQ(const_map.at(1), "one");
}

// 测试 at() 访问不存在的键（应该抛异常）
TEST(UnorderedMapTest, AtThrowsException) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";

  EXPECT_THROW(map.at(2), std::out_of_range);
  EXPECT_THROW(map.at(999), std::out_of_range);
}

// 测试 insert 单个元素
TEST(UnorderedMapTest, InsertSingle) {
  mystl::unordered_map<int, std::string> map;
  
  // 插入新元素
  auto [it1, inserted1] = map.insert({1, "one"});
  EXPECT_TRUE(inserted1);
  EXPECT_EQ(it1->second, "one");
  EXPECT_EQ(map.size(), 1);

  // 尝试插入已存在的键
  auto [it2, inserted2] = map.insert({1, "another"});
  EXPECT_FALSE(inserted2);
  EXPECT_EQ(it2->second, "one");  // 值应该不变
  EXPECT_EQ(map.size(), 1);

  // 插入另一个元素
  auto [it3, inserted3] = map.insert({2, "two"});
  EXPECT_TRUE(inserted3);
  EXPECT_EQ(map.size(), 2);
}

// 测试 insert 移动语义
TEST(UnorderedMapTest, InsertMove) {
  mystl::unordered_map<int, std::string> map;
  std::pair<int, std::string> pair = {1, "one"};
  auto [it, inserted] = map.insert(std::move(pair));
  EXPECT_TRUE(inserted);
  EXPECT_EQ(map[1], "one");
}

// 测试 insert 迭代器范围
TEST(UnorderedMapTest, InsertRange) {
  mystl::unordered_map<int, std::string> map;
  std::vector<std::pair<int, std::string>> vec = {{1, "one"}, {2, "two"}, {3, "three"}};
  
  map.insert(vec.begin(), vec.end());
  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map[1], "one");
  EXPECT_EQ(map[2], "two");
  EXPECT_EQ(map[3], "three");
}

// 测试 insert 初始化列表
TEST(UnorderedMapTest, InsertInitializerList) {
  mystl::unordered_map<int, std::string> map;
  map.insert({{1, "one"}, {2, "two"}, {3, "three"}});
  EXPECT_EQ(map.size(), 3);
}

// 测试 emplace
TEST(UnorderedMapTest, Emplace) {
  mystl::unordered_map<int, std::string> map;
  
  auto [it1, inserted1] = map.emplace(1, "one");
  EXPECT_TRUE(inserted1);
  EXPECT_EQ(map.size(), 1);
  EXPECT_EQ(map[1], "one");

  // 尝试 emplace 已存在的键
  auto [it2, inserted2] = map.emplace(1, "another");
  EXPECT_FALSE(inserted2);
  EXPECT_EQ(map[1], "one");  // 值应该不变
  EXPECT_EQ(map.size(), 1);

  // emplace 新元素
  auto [it3, inserted3] = map.emplace(2, "two");
  EXPECT_TRUE(inserted3);
  EXPECT_EQ(map.size(), 2);
}

// 测试 find
TEST(UnorderedMapTest, Find) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";

  auto it1 = map.find(1);
  EXPECT_NE(it1, map.end());
  EXPECT_EQ(it1->second, "one");

  auto it2 = map.find(2);
  EXPECT_NE(it2, map.end());
  EXPECT_EQ(it2->second, "two");

  auto it3 = map.find(999);
  EXPECT_EQ(it3, map.end());

  // const 版本
  const mystl::unordered_map<int, std::string>& const_map = map;
  auto it4 = const_map.find(1);
  EXPECT_NE(it4, const_map.end());
  EXPECT_EQ(it4->second, "one");
}

// 测试 count
TEST(UnorderedMapTest, Count) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";

  EXPECT_EQ(map.count(1), 1);
  EXPECT_EQ(map.count(2), 1);
  EXPECT_EQ(map.count(999), 0);
}

// 测试 contains (C++20)
TEST(UnorderedMapTest, Contains) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";

  EXPECT_TRUE(map.contains(1));
  EXPECT_TRUE(map.contains(2));
  EXPECT_FALSE(map.contains(999));
}

// 测试 erase 按键删除
TEST(UnorderedMapTest, EraseByKey) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";
  map[3] = "three";

  // 使用 EXPECT_NO_FATAL_FAILURE 来确保不会崩溃
  EXPECT_NO_FATAL_FAILURE({
    EXPECT_EQ(map.erase(2), 1);
  });
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.find(2), map.end());

  EXPECT_NO_FATAL_FAILURE({
    EXPECT_EQ(map.erase(999), 0);  // 删除不存在的键
  });
  EXPECT_EQ(map.size(), 2);
}

// 测试 erase 按迭代器删除
TEST(UnorderedMapTest, EraseByIterator) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";
  map[3] = "three";

  // 测试非 const 迭代器删除
  EXPECT_NO_FATAL_FAILURE({
    auto it = map.find(2);
    ASSERT_NE(it, map.end());
    auto next_it = map.erase(it);
    EXPECT_EQ(map.size(), 2);
    EXPECT_EQ(map.find(2), map.end());
    EXPECT_NE(next_it, map.end());
  });

  // 测试 const 迭代器删除
  map[2] = "two";  // 重新插入
  const mystl::unordered_map<int, std::string>& const_map = map;
  EXPECT_NO_FATAL_FAILURE({
    auto cit = const_map.find(2);
    ASSERT_NE(cit, const_map.end());
    auto next_it = map.erase(cit);
    EXPECT_EQ(map.size(), 2);
    EXPECT_EQ(map.find(2), map.end());
  });
}

// 测试 erase 按范围删除
TEST(UnorderedMapTest, EraseRange) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";
  map[3] = "three";
  map[4] = "four";

  auto it1 = map.find(2);
  auto it2 = map.find(4);
  map.erase(it1, it2);
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.find(2), map.end());
  EXPECT_EQ(map.find(3), map.end());
  EXPECT_NE(map.find(1), map.end());
  EXPECT_NE(map.find(4), map.end());
}

// 测试 clear
TEST(UnorderedMapTest, Clear) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";
  map[3] = "three";

  map.clear();
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
}

// 测试迭代器
TEST(UnorderedMapTest, Iterator) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";
  map[3] = "three";

  // 测试前向迭代
  int count = 0;
  for (auto it = map.begin(); it != map.end(); ++it) {
    ++count;
    EXPECT_GE(it->first, 1);
    EXPECT_LE(it->first, 3);
  }
  EXPECT_EQ(count, 3);

  // 测试基于范围的 for 循环
  count = 0;
  for (const auto& pair : map) {
    ++count;
    EXPECT_GE(pair.first, 1);
    EXPECT_LE(pair.first, 3);
  }
  EXPECT_EQ(count, 3);
}

// 测试 const 迭代器
TEST(UnorderedMapTest, ConstIterator) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";

  const mystl::unordered_map<int, std::string>& const_map = map;
  int count = 0;
  for (auto it = const_map.begin(); it != const_map.end(); ++it) {
    ++count;
  }
  EXPECT_EQ(count, 2);

  count = 0;
  for (auto it = const_map.cbegin(); it != const_map.cend(); ++it) {
    ++count;
  }
  EXPECT_EQ(count, 2);
}

// 测试 bucket_count
TEST(UnorderedMapTest, BucketCount) {
  mystl::unordered_map<int, std::string> map(10);
  EXPECT_GE(map.bucket_count(), 10);

  map[1] = "one";
  map[2] = "two";
  EXPECT_GE(map.bucket_count(), 10);  // 桶数不应减少
}

// 测试 bucket
TEST(UnorderedMapTest, Bucket) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";

  size_t bucket1 = map.bucket(1);
  size_t bucket2 = map.bucket(2);
  EXPECT_LT(bucket1, map.bucket_count());
  EXPECT_LT(bucket2, map.bucket_count());
}

// 测试 bucket_size
TEST(UnorderedMapTest, BucketSize) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";

  for (size_t i = 0; i < map.bucket_count(); ++i) {
    EXPECT_GE(map.bucket_size(i), 0);
  }
}

// 测试桶内迭代器
TEST(UnorderedMapTest, LocalIterator) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";
  map[3] = "three";

  for (size_t i = 0; i < map.bucket_count(); ++i) {
    int count = 0;
    for (auto it = map.begin(i); it != map.end(i); ++it) {
      ++count;
    }
    EXPECT_GE(map.bucket_size(i), count);
  }
}

// 测试 load_factor
TEST(UnorderedMapTest, LoadFactor) {
  mystl::unordered_map<int, std::string> map;
  EXPECT_EQ(map.load_factor(), 0.0f);

  map[1] = "one";
  float lf = map.load_factor();
  EXPECT_GT(lf, 0.0f);
  if (map.bucket_count() > 0) {
    EXPECT_LE(lf, 1.0f);
  }
}

// 测试 max_load_factor
TEST(UnorderedMapTest, MaxLoadFactor) {
  mystl::unordered_map<int, std::string> map;
  float default_mlf = map.max_load_factor();
  EXPECT_GT(default_mlf, 0.0f);

  map.max_load_factor(0.5f);
  EXPECT_EQ(map.max_load_factor(), 0.5f);
}

// 测试 rehash
TEST(UnorderedMapTest, Rehash) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "one";
  map[2] = "two";
  map[3] = "three";

  size_t old_bucket_count = map.bucket_count();
  map.rehash(20);
  EXPECT_GE(map.bucket_count(), 20);
  EXPECT_EQ(map.size(), 3);  // 元素不应丢失
  EXPECT_EQ(map[1], "one");
  EXPECT_EQ(map[2], "two");
  EXPECT_EQ(map[3], "three");
}

// 测试 reserve
TEST(UnorderedMapTest, Reserve) {
  mystl::unordered_map<int, std::string> map;
  map.reserve(100);
  EXPECT_GE(map.bucket_count(), 100);
}

// 测试 swap
TEST(UnorderedMapTest, Swap) {
  mystl::unordered_map<int, std::string> map1;
  map1[1] = "one";
  map1[2] = "two";

  mystl::unordered_map<int, std::string> map2;
  map2[3] = "three";
  map2[4] = "four";

  map1.swap(map2);
  EXPECT_EQ(map1.size(), 2);
  EXPECT_EQ(map1[3], "three");
  EXPECT_EQ(map1[4], "four");
  EXPECT_EQ(map2.size(), 2);
  EXPECT_EQ(map2[1], "one");
  EXPECT_EQ(map2[2], "two");
}

// 测试非成员 swap 函数
TEST(UnorderedMapTest, NonMemberSwap) {
  mystl::unordered_map<int, std::string> map1;
  map1[1] = "one";

  mystl::unordered_map<int, std::string> map2;
  map2[2] = "two";

  mystl::swap(map1, map2);
  EXPECT_EQ(map1.size(), 1);
  EXPECT_EQ(map1[2], "two");
  EXPECT_EQ(map2.size(), 1);
  EXPECT_EQ(map2[1], "one");
}

// 测试赋值运算符（拷贝赋值）
TEST(UnorderedMapTest, CopyAssignment) {
  mystl::unordered_map<int, std::string> map1;
  map1[1] = "one";
  map1[2] = "two";

  mystl::unordered_map<int, std::string> map2;
  map2[3] = "three";

  map2 = map1;
  EXPECT_EQ(map2.size(), 2);
  EXPECT_EQ(map2[1], "one");
  EXPECT_EQ(map2[2], "two");
  EXPECT_EQ(map1.size(), 2);  // 原对象不应被修改
}

// 测试赋值运算符（移动赋值）
TEST(UnorderedMapTest, MoveAssignment) {
  mystl::unordered_map<int, std::string> map1;
  map1[1] = "one";
  map1[2] = "two";
  size_t original_size = map1.size();

  mystl::unordered_map<int, std::string> map2;
  map2 = std::move(map1);
  EXPECT_EQ(map2.size(), original_size);
  EXPECT_EQ(map2[1], "one");
  EXPECT_EQ(map2[2], "two");
  EXPECT_TRUE(map1.empty());  // 原对象应该被移动
}

// 测试赋值运算符（初始化列表赋值）
TEST(UnorderedMapTest, InitializerListAssignment) {
  mystl::unordered_map<int, std::string> map;
  map[1] = "old";

  map = {{2, "two"}, {3, "three"}, {4, "four"}};
  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map.find(1), map.end());
  EXPECT_EQ(map[2], "two");
  EXPECT_EQ(map[3], "three");
  EXPECT_EQ(map[4], "four");
}

// 测试相等性比较
TEST(UnorderedMapTest, EqualityComparison) {
  mystl::unordered_map<int, std::string> map1;
  map1[1] = "one";
  map1[2] = "two";

  mystl::unordered_map<int, std::string> map2;
  map2[1] = "one";
  map2[2] = "two";

  EXPECT_TRUE(map1 == map2);
  EXPECT_FALSE(map1 != map2);

  map2[2] = "modified";
  EXPECT_FALSE(map1 == map2);
  EXPECT_TRUE(map1 != map2);

  map2.erase(2);
  map2[3] = "three";
  EXPECT_FALSE(map1 == map2);
  EXPECT_TRUE(map1 != map2);
}

// 测试空容器的相等性
TEST(UnorderedMapTest, EmptyEquality) {
  mystl::unordered_map<int, std::string> map1;
  mystl::unordered_map<int, std::string> map2;
  EXPECT_TRUE(map1 == map2);
  EXPECT_FALSE(map1 != map2);
}

// 测试 size 和 empty
TEST(UnorderedMapTest, SizeAndEmpty) {
  mystl::unordered_map<int, std::string> map;
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());

  map[1] = "one";
  EXPECT_EQ(map.size(), 1);
  EXPECT_FALSE(map.empty());

  map[2] = "two";
  EXPECT_EQ(map.size(), 2);
  EXPECT_FALSE(map.empty());

  map.clear();
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
}

// 测试 get_allocator
TEST(UnorderedMapTest, GetAllocator) {
  mystl::unordered_map<int, std::string> map;
  auto alloc = map.get_allocator();
  using expected_type = std::allocator<std::pair<const int, std::string>>;
  EXPECT_TRUE((std::is_same<decltype(alloc), expected_type>::value));
}

// 测试 hash_function 和 key_eq
TEST(UnorderedMapTest, HashFunctionAndKeyEq) {
  mystl::unordered_map<int, std::string> map;
  auto hash_fn = map.hash_function();
  auto key_eq_fn = map.key_eq();

  EXPECT_EQ(hash_fn(42), std::hash<int>()(42));
  EXPECT_TRUE(key_eq_fn(1, 1));
  EXPECT_FALSE(key_eq_fn(1, 2));
}

// 测试复杂类型作为键
TEST(UnorderedMapTest, ComplexKeyType) {
  mystl::unordered_map<std::string, int> map;
  map["first"] = 1;
  map["second"] = 2;
  map["third"] = 3;

  EXPECT_EQ(map.size(), 3);
  EXPECT_EQ(map["first"], 1);
  EXPECT_EQ(map["second"], 2);
  EXPECT_EQ(map["third"], 3);
}

// 测试多次插入和删除
TEST(UnorderedMapTest, MultipleInsertAndErase) {
  mystl::unordered_map<int, int> map;
  
  // 插入 100 个元素
  for (int i = 0; i < 100; ++i) {
    map[i] = i * 2;
  }
  EXPECT_EQ(map.size(), 100);

  // 删除偶数键
  for (int i = 0; i < 100; i += 2) {
    map.erase(i);
  }
  EXPECT_EQ(map.size(), 50);

  // 验证剩余元素
  for (int i = 1; i < 100; i += 2) {
    EXPECT_EQ(map[i], i * 2);
  }
}

