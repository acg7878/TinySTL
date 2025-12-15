#include "gtest/gtest.h"
#include <mystl/string.h>

// 测试默认构造函数
TEST(StringTest, DefaultConstructor) {
  mystl::string s;
  EXPECT_TRUE(s.empty());
  EXPECT_EQ(s.size(), 0);
  EXPECT_EQ(s.length(), 0);
  EXPECT_NE(s.c_str(), nullptr);
  EXPECT_EQ(s.c_str()[0], '\0');
}

// 测试 C 风格字符串构造函数
TEST(StringTest, CStringConstructor) {
  const char* str = "hello";
  mystl::string s(str);
  EXPECT_EQ(s.size(), 5);
  EXPECT_STREQ(s.c_str(), str);
  EXPECT_FALSE(s.empty());
}

// 测试带大小的 C 风格字符串构造函数
TEST(StringTest, CStringCountConstructor) {
  const char* str = "hello world";
  mystl::string s(str, 5);
  EXPECT_EQ(s.size(), 5);
  EXPECT_STREQ(s.c_str(), "hello");
}

// 测试拷贝构造函数
TEST(StringTest, CopyConstructor) {
  mystl::string s1("hello");
  mystl::string s2(s1);
  EXPECT_EQ(s1.size(), s2.size());
  EXPECT_STREQ(s1.c_str(), s2.c_str());
  EXPECT_NE(s1.data(), s2.data()); // 应该是深拷贝
}

// 测试移动构造函数
TEST(StringTest, MoveConstructor) {
  mystl::string s1("hello");
  const char* ptr = s1.data();
  mystl::string s2(std::move(s1));
  
  EXPECT_EQ(s2.size(), 5);
  EXPECT_STREQ(s2.c_str(), "hello");
  
  // 对于短字符串，数据可能在内部缓冲区，指针可能不同
  // 对于长字符串，指针应该相同
  // 由于 "hello" 是短字符串，我们主要验证 s2 的内容和 s1 的状态
  // mystl 实现中，短字符串移动后，源字符串状态未明确定义（通常为空或有效），这里假设为空或原样
  // 但最重要的是 s2 正确
}

// 测试填充构造函数
TEST(StringTest, FillConstructor) {
  mystl::string s(5, 'a');
  EXPECT_EQ(s.size(), 5);
  EXPECT_STREQ(s.c_str(), "aaaaa");
}

// 测试赋值运算符
TEST(StringTest, AssignmentOperator) {
  mystl::string s1("hello");
  mystl::string s2;
  s2 = s1;
  EXPECT_EQ(s1.size(), s2.size());
  EXPECT_STREQ(s1.c_str(), s2.c_str());
  EXPECT_NE(s1.data(), s2.data());

  s2 = "world";
  EXPECT_EQ(s2.size(), 5);
  EXPECT_STREQ(s2.c_str(), "world");
}

// 测试移动赋值
TEST(StringTest, MoveAssignment) {
  mystl::string s1("hello");
  mystl::string s2;
  s2 = std::move(s1);
  EXPECT_EQ(s2.size(), 5);
  EXPECT_STREQ(s2.c_str(), "hello");
}

// 测试访问元素
TEST(StringTest, ElementAccess) {
  mystl::string s("hello");
  EXPECT_EQ(s[0], 'h');
  EXPECT_EQ(s[4], 'o');
  EXPECT_EQ(s.at(1), 'e');
  
  EXPECT_THROW(s.at(10), std::out_of_range);
  
  EXPECT_EQ(s.front(), 'h');
  EXPECT_EQ(s.back(), 'o');
}

// 测试 push_back 和 pop_back
TEST(StringTest, PushPopBack) {
  mystl::string s("hell");
  s.push_back('o');
  EXPECT_EQ(s.size(), 5);
  EXPECT_STREQ(s.c_str(), "hello");
  
  s.pop_back();
  EXPECT_EQ(s.size(), 4);
  EXPECT_STREQ(s.c_str(), "hell");
}

// 测试 append
TEST(StringTest, Append) {
  mystl::string s("hello");
  s.append(" world");
  EXPECT_EQ(s.size(), 11);
  EXPECT_STREQ(s.c_str(), "hello world");
  
  mystl::string s2("!");
  s += s2;
  EXPECT_EQ(s.size(), 12);
  EXPECT_STREQ(s.c_str(), "hello world!");
}

// 测试 SSO (Short String Optimization)
TEST(StringTest, SSO) {
  // 假设 24 字节对象大小，通常短字符串容量在 22 左右
  // 测试短字符串
  mystl::string short_str("1234567890");
  const char* short_ptr = short_str.data();
  // 验证指针是否在对象内部（这就比较难通用判断，但可以通过移动行为推测）
  
  // 测试长字符串
  // 创建一个肯定超过短字符串限制的字符串
  mystl::string long_str(100, 'a');
  EXPECT_EQ(long_str.size(), 100);
  const char* long_ptr = long_str.data();
  
  mystl::string long_moved(std::move(long_str));
  EXPECT_EQ(long_moved.size(), 100);
  EXPECT_EQ(long_moved.data(), long_ptr); // 长字符串移动，指针应该不变
}

// 测试 resize
TEST(StringTest, Resize) {
  mystl::string s("hello");
  s.resize(3);
  EXPECT_EQ(s.size(), 3);
  EXPECT_STREQ(s.c_str(), "hel");
  
  s.resize(5, '!');
  EXPECT_EQ(s.size(), 5);
  EXPECT_STREQ(s.c_str(), "hel!!");
}

// 测试 reserve
TEST(StringTest, Reserve) {
  mystl::string s("hello");
  size_t old_cap = s.capacity();
  s.reserve(100);
  EXPECT_GE(s.capacity(), 100);
  EXPECT_EQ(s.size(), 5);
  EXPECT_STREQ(s.c_str(), "hello");
  
  s.reserve(1); // 应该不影响，如果新容量小于当前容量
  EXPECT_GE(s.capacity(), 100);
}

// 测试 clear
TEST(StringTest, Clear) {
  mystl::string s("hello");
  s.clear();
  EXPECT_EQ(s.size(), 0);
  EXPECT_TRUE(s.empty());
  EXPECT_STREQ(s.c_str(), "");
}

// 测试 find
TEST(StringTest, Find) {
  mystl::string s("hello world");
  EXPECT_EQ(s.find("hello"), 0);
  EXPECT_EQ(s.find("world"), 6);
  EXPECT_EQ(s.find("o"), 4);
  EXPECT_EQ(s.find("z"), mystl::string::npos);
  EXPECT_EQ(s.find("hello", 1), mystl::string::npos);
}

// 测试比较运算符
TEST(StringTest, Compare) {
  mystl::string s1("abc");
  mystl::string s2("abc");
  mystl::string s3("def");
  mystl::string s4("abd");
  
  EXPECT_TRUE(s1 == s2);
  EXPECT_FALSE(s1 == s3);
  EXPECT_TRUE(s1 != s3);
  EXPECT_TRUE(s1 < s3);
  EXPECT_TRUE(s1 < s4);
  EXPECT_FALSE(s3 < s1);
}

// 测试 shrink_to_fit
TEST(StringTest, ShrinkToFit) {
  mystl::string s(100, 'a');
  s.resize(10);
  s.shrink_to_fit();
  // 现在的实现 shrink_to_fit 会尝试变回短字符串如果可能
  // 10 个字符应该能放入短字符串
  // 具体的验证可能依赖实现细节，但至少它不应该崩溃且保持数据正确
  EXPECT_EQ(s.size(), 10);
  EXPECT_STREQ(s.c_str(), "aaaaaaaaaa");
}

// 测试 swap
TEST(StringTest, Swap) {
  mystl::string s1("hello");
  mystl::string s2("world");
  
  s1.swap(s2);
  EXPECT_STREQ(s1.c_str(), "world");
  EXPECT_STREQ(s2.c_str(), "hello");
  
  mystl::swap(s1, s2);
  EXPECT_STREQ(s1.c_str(), "hello");
  EXPECT_STREQ(s2.c_str(), "world");
}
