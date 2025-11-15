#include <gtest/gtest.h>
#include <mystl/functional.h>

int add(int a,int b) {
  return a+b;
}

TEST(FunctionTest, Basic) {
  mystl::function<int(int, int)> f = [](int a, int b) { return a + b; };
  ASSERT_EQ(f(1, 2), 3);
}

TEST(FunctionTest, Copy) {
  mystl::function<int(int, int)> f = [](int a, int b) { return a - b; };
  mystl::function<int(int, int)> f2 = f;
  ASSERT_EQ(f2(5, 2), 3);
}

TEST(FunctionTest, Move) {
  mystl::function<int(int, int)> f = [](int a, int b) { return a * a + b * b; };
  mystl::function<int(int, int)> f2 = std::move(f);
  ASSERT_EQ(f2(3, 4), 25);
}

TEST(FunctionTest, Null) {
  mystl::function<int(int, int)> f;
  ASSERT_FALSE(f);
  f = nullptr;
  ASSERT_FALSE(f);
}

TEST(FunctionTest, Throw) {
  mystl::function<int(int, int)> f;
  ASSERT_THROW(f(1, 2), std::bad_function_call);
}

TEST(FunctionTest, Function) {
  mystl::function<int(int, int)> f = add;
  ASSERT_EQ(f(1, 2), 3);
}
