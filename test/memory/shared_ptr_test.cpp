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

class MyClass : public mystl::enable_shared_from_this<MyClass> {
 public:
  mystl::shared_ptr<MyClass> get_self() { return shared_from_this(); }
};

TEST(EnableSharedFromThisTest, BasicTest) {
    // 1. 验证MyClass是否正确继承自enable_shared_from_this<MyClass>
    bool is_derived = std::is_base_of<mystl::enable_shared_from_this<MyClass>, MyClass>::value;
    std::cout << "MyClass是否继承自enable_shared_from_this<MyClass>: " << std::boolalpha << is_derived << std::endl;

    // 2. 验证指针转换是否可行（匹配模板约束的关键条件）
    bool is_convertible = std::is_convertible<MyClass*, mystl::enable_shared_from_this<MyClass>*>::value;
    std::cout << "MyClass*是否可转换为enable_shared_from_this<MyClass>*: " << is_convertible << std::endl;

    // 3. 打印 enable_weak_this 的参数类型
    std::cout << "enable_weak_this 的参数类型：" << std::endl;
    std::cout << "  Yp: " << typeid(MyClass).name() << std::endl;
    std::cout << "  OrigPtr: " << typeid(MyClass).name() << std::endl;

    // 3. 执行原测试逻辑并打印中间状态
    std::cout << "创建shared_ptr<MyClass>" << std::endl;
    mystl::shared_ptr<MyClass> ptr(new MyClass());

    std::cout << "调用get_self()获取自身shared_ptr" << std::endl;
    try {
        mystl::shared_ptr<MyClass> self_ptr = ptr->get_self();
        std::cout << "get_self()成功，ptr地址: " << ptr.get() << ", self_ptr地址: " << self_ptr.get() << std::endl;
        ASSERT_EQ(ptr, self_ptr);
    } catch (const std::bad_weak_ptr& e) {
        std::cout << "get_self()失败: " << e.what() << std::endl;
        throw; // 继续抛出异常让测试框架捕获
    }
}


// TEST(EnableSharedFromThisTest, ExceptionTest) {
//   MyClass obj;
//   ASSERT_THROW(obj.get_self(), std::bad_weak_ptr);
// }