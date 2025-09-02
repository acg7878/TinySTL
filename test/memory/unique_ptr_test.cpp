#include <gtest/gtest.h>
#include <mystl/memory.h>

TEST(UniquePtrTest, Basic) {
  mystl::unique_ptr<int> ptr(new int(10));
  ASSERT_NE(ptr.get(), nullptr);
  ASSERT_EQ(*ptr.get(), 10);
}

TEST(UniquePtrTest, Release) {
  mystl::unique_ptr<int> ptr(new int(10));
  int *raw_ptr = ptr.release();
  ASSERT_EQ(ptr.get(), nullptr);
  ASSERT_EQ(*raw_ptr, 10);
  delete raw_ptr;
}

TEST(UniquePtrTest, Reset) {
  mystl::unique_ptr<int> ptr(new int(10));
  ptr.reset();
  ASSERT_EQ(ptr.get(), nullptr);

  ptr.reset(new int(20));
  ASSERT_NE(ptr.get(), nullptr);
  ASSERT_EQ(*ptr.get(), 20);
}

TEST(UniquePtrTest, Array) {
  mystl::unique_ptr<int[]> ptr(new int[3]{1, 2, 3});
  ASSERT_EQ(ptr[0], 1);
  ASSERT_EQ(ptr[1], 2);
  ASSERT_EQ(ptr[2], 3);
}

struct MyDeleter {
  void operator()(int *ptr) const { delete ptr; }
};

TEST(UniquePtrTest, CustomDeleter) {
  mystl::unique_ptr<int, MyDeleter> ptr(new int(10));
  ASSERT_NE(ptr.get(), nullptr);
}

TEST(UniquePtrTest, Move) {
  mystl::unique_ptr<int> ptr1(new int(10));
  mystl::unique_ptr<int> ptr2 = mystl::move(ptr1);
  ASSERT_EQ(ptr1.get(), nullptr);
  ASSERT_NE(ptr2.get(), nullptr);
  ASSERT_EQ(*ptr2.get(), 10);
}