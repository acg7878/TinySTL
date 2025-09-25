#include <gtest/gtest.h>
#include <mystl/memory.h>

// Test basic functionality
TEST(SharedPtrTest, Basic) {
  mystl::shared_ptr<int> sp(new int(10));
  ASSERT_NE(sp.get(), nullptr);
  ASSERT_EQ(*sp, 10);
  ASSERT_EQ(sp.use_count(), 1);
}

// Test copy constructor
TEST(SharedPtrTest, CopyConstructor) {
  mystl::shared_ptr<int> sp1(new int(20));
  ASSERT_EQ(sp1.use_count(), 1);
  mystl::shared_ptr<int> sp2 = sp1;
  ASSERT_EQ(sp1.use_count(), 2);
  ASSERT_EQ(sp2.use_count(), 2);
  ASSERT_EQ(sp1.get(), sp2.get());
  ASSERT_EQ(*sp2, 20);
}

// Test copy assignment
TEST(SharedPtrTest, CopyAssignment) {
  mystl::shared_ptr<int> sp1(new int(30));
  mystl::shared_ptr<int> sp2(new int(40));
  ASSERT_EQ(sp1.use_count(), 1);
  ASSERT_EQ(sp2.use_count(), 1);
  sp1 = sp2;
  ASSERT_EQ(sp2.use_count(), 2);
  ASSERT_EQ(sp1.use_count(), 2);
  ASSERT_EQ(*sp1, 40);
}

// Test move constructor
TEST(SharedPtrTest, MoveConstructor) {
  mystl::shared_ptr<int> sp1(new int(50));
  mystl::shared_ptr<int> sp2 = mystl::move(sp1);
  ASSERT_EQ(sp1.get(), nullptr);
  ASSERT_EQ(sp1.use_count(), 0);
  ASSERT_NE(sp2.get(), nullptr);
  ASSERT_EQ(*sp2, 50);
  ASSERT_EQ(sp2.use_count(), 1);
}

// Test move assignment
TEST(SharedPtrTest, MoveAssignment) {
  mystl::shared_ptr<int> sp1(new int(60));
  mystl::shared_ptr<int> sp2(new int(70));
  sp1 = mystl::move(sp2);
  ASSERT_EQ(sp2.get(), nullptr);
  ASSERT_EQ(sp2.use_count(), 0);
  ASSERT_NE(sp1.get(), nullptr);
  ASSERT_EQ(*sp1, 70);
  ASSERT_EQ(sp1.use_count(), 1);
}

// Test reset
TEST(SharedPtrTest, Reset) {
  mystl::shared_ptr<int> sp(new int(80));
  ASSERT_EQ(sp.use_count(), 1);
  sp.reset();
  ASSERT_EQ(sp.get(), nullptr);
  ASSERT_EQ(sp.use_count(), 0);
}

// Test custom deleter
struct MyIntDeleter {
  static bool deleted;
  void operator()(int* p) {
    delete p;
    deleted = true;
  }
};
bool MyIntDeleter::deleted = false;

TEST(SharedPtrTest, CustomDeleter) {
  MyIntDeleter::deleted = false;
  {
    mystl::shared_ptr<int> sp(new int(90), MyIntDeleter());
    ASSERT_EQ(sp.use_count(), 1);
  }
  ASSERT_TRUE(MyIntDeleter::deleted);
}

// Test WeakPtr
TEST(SharedPtrTest, WeakPtr) {
  mystl::weak_ptr<int> wp;
  {
    mystl::shared_ptr<int> sp(new int(100));
    wp = sp;
    ASSERT_EQ(sp.use_count(), 1);
    ASSERT_EQ(wp.use_count(), 1);
    mystl::shared_ptr<int> sp2 = wp.lock();
    ASSERT_NE(sp2.get(), nullptr);
    ASSERT_EQ(sp.use_count(), 2);
    ASSERT_EQ(*sp2, 100);
  }
  ASSERT_TRUE(wp.expired());
  ASSERT_EQ(wp.use_count(), 0);
  mystl::shared_ptr<int> sp3 = wp.lock();
  ASSERT_EQ(sp3.get(), nullptr);
}

// Test enable_shared_from_this
struct SharedFromThisTest : public mystl::enable_shared_from_this<SharedFromThisTest> {
  int value;
  SharedFromThisTest(int v) : value(v) {}
};

TEST(SharedPtrTest, EnableSharedFromThis) {
  mystl::shared_ptr<SharedFromThisTest> sp1(new SharedFromThisTest(110));
  mystl::shared_ptr<SharedFromThisTest> sp2 = sp1->shared_from_this();
  ASSERT_EQ(sp1.get(), sp2.get());
  ASSERT_EQ(sp1.use_count(), 2);
  ASSERT_EQ(sp2->value, 110);
}
