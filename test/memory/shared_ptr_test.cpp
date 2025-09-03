#include <gtest/gtest.h>
#include <mystl/memory.h>

TEST(SharedPtrTest, Basic) {
  mystl::shared_ptr<int> ptr(new int(10));
  ASSERT_NE(ptr.get(), nullptr);
  ASSERT_EQ(*ptr.get(), 10);
}

TEST(SharedPtrTest, Copy) {
  mystl::shared_ptr<int> ptr1(new int(10));
  mystl::shared_ptr<int> ptr2 = ptr1;
  ASSERT_EQ(ptr1.use_count(), 2);
  ASSERT_EQ(ptr2.use_count(), 2);
  ASSERT_EQ(*ptr1.get(), 10);
  ASSERT_EQ(*ptr2.get(), 10);
}

TEST(SharedPtrTest, Move) {
  mystl::shared_ptr<int> ptr1(new int(10));
  mystl::shared_ptr<int> ptr2 = mystl::move(ptr1);
  ASSERT_EQ(ptr1.get(), nullptr);
  ASSERT_NE(ptr2.get(), nullptr);
  ASSERT_EQ(*ptr2.get(), 10);
}

TEST(SharedPtrTest, Reset) {
  mystl::shared_ptr<int> ptr(new int(10));
  ptr.reset();
  ASSERT_EQ(ptr.get(), nullptr);
}

TEST(SharedPtrTest, UseCount) {
  mystl::shared_ptr<int> ptr1(new int(10));
  ASSERT_EQ(ptr1.use_count(), 1);
  mystl::shared_ptr<int> ptr2 = ptr1;
  ASSERT_EQ(ptr1.use_count(), 2);
  ASSERT_EQ(ptr2.use_count(), 2);
  ptr1.reset();
  ASSERT_EQ(ptr2.use_count(), 1);
}

TEST(SharedPtrTest, WeakPtr) {
  mystl::shared_ptr<int> ptr1(new int(10));
  mystl::weak_ptr<int> wptr = ptr1;
  ASSERT_EQ(ptr1.use_count(), 1);
  ASSERT_EQ(wptr.use_count(), 1);
  {
    mystl::shared_ptr<int> ptr2 = wptr.lock();
    ASSERT_NE(ptr2.get(), nullptr);
    ASSERT_EQ(*ptr2.get(), 10);
    ASSERT_EQ(ptr1.use_count(), 2);
    ASSERT_EQ(wptr.use_count(), 2);
  }
  ASSERT_EQ(ptr1.use_count(), 1);
  ASSERT_EQ(wptr.use_count(), 1);
  ptr1.reset();
  ASSERT_EQ(wptr.expired(), true);
}