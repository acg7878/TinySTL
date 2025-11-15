#ifndef TINYSTL___FUNCTIONAL_FUNCTION_H
#define TINYSTL___FUNCTIONAL_FUNCTION_H

#include <cassert>
#include <exception>
#include <functional>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace mystl {

// 1. bad_function_call 异常类
class bad_function_call : public std::exception {
 public:
  bad_function_call() noexcept = default;
  ~bad_function_call() noexcept override = default;
  const char* what() const noexcept override {
    return "bad_function_call: call to empty function";
  }
};

// 抛出异常的辅助函数
[[noreturn]] inline void throw_bad_function_call() {
  throw bad_function_call();
}

// 2. function 前向声明
template <typename Signature>
class function;

// 3. 主模板特化：处理函数签名 R(ArgTypes...)
template <typename R, typename... ArgTypes>
class function<R(ArgTypes...)> {
 private:
  // 4. 内部接口类：实现类型擦除
  struct function_interface {
    virtual ~function_interface() = default;
    virtual function_interface* clone() const = 0;
    virtual R operator()(ArgTypes&&... args) = 0;
    virtual const std::type_info& target_type() const noexcept = 0;
    virtual const void* target() const noexcept = 0;
  };

  // 5. 具体实现类：包装可调用对象
  template <typename F>
  struct function_impl : function_interface {
    F functor;

    explicit function_impl(F&& f) : functor(std::forward<F>(f)) {}
    explicit function_impl(const F& f) : functor(f) {}

    function_impl* clone() const override { return new function_impl(functor); }

    R operator()(ArgTypes&&... args) override {
      return std::invoke(functor, std::forward<ArgTypes>(args)...);
    }

    const std::type_info& target_type() const noexcept override {
      return typeid(F);
    }

    const void* target() const noexcept override { return &functor; }
  };

  // 6. 存储实现的指针
  function_interface* impl = nullptr;

 public:
  // 7. 构造函数
  function() noexcept = default;

  function(std::nullptr_t) noexcept : function() {}

  // 拷贝构造
  function(const function& other)
      : impl(other.impl ? other.impl->clone() : nullptr) {}

  // 移动构造
  function(function&& other) noexcept : impl(other.impl) {
    other.impl = nullptr;
  }

  // 从可调用对象构造
  template <
      typename F,
      typename = typename std::enable_if<
          std::is_convertible<
              decltype(std::invoke(std::declval<F>(),
                                   std::declval<ArgTypes>()...)),
              R>::value &&
          !std::is_same<typename std::decay<F>::type, function>::value>::type>
  function(F&& f) {
    using decay_type = typename std::decay<F>::type;
    impl = new function_impl<decay_type>(std::forward<F>(f));
  }

  // 8. 析构函数
  ~function() { delete impl; }

  // 9. 赋值运算符
  function& operator=(const function& other) {
    if (this != &other) {
      function tmp(other);
      swap(tmp);
    }
    return *this;
  }

  function& operator=(function&& other) noexcept {
    if (this != &other) {
      delete impl;
      impl = other.impl;
      other.impl = nullptr;
    }
    return *this;
  }

  function& operator=(std::nullptr_t) noexcept {
    delete impl;
    impl = nullptr;
    return *this;
  }

  template <typename F>
  function& operator=(F&& f) {
    function tmp(std::forward<F>(f));
    swap(tmp);
    return *this;
  }

  // 10. 函数调用
  R operator()(ArgTypes... args) const {
    if (!impl) {
      throw_bad_function_call();
    }
    return (*impl)(std::forward<ArgTypes>(args)...);
  }

  // 11. 状态检查
  explicit operator bool() const noexcept { return impl != nullptr; }

  // 12. 交换
  void swap(function& other) noexcept { std::swap(impl, other.impl); }

  // 13. 目标访问
  const std::type_info& target_type() const noexcept {
    return impl ? impl->target_type() : typeid(void);
  }

  template <typename T>
  T* target() noexcept {
    if (target_type() == typeid(T)) {
      return static_cast<T*>(const_cast<void*>(impl->target()));
    }
    return nullptr;
  }

  template <typename T>
  const T* target() const noexcept {
    if (target_type() == typeid(T)) {
      return static_cast<const T*>(impl->target());
    }
    return nullptr;
  }
};

// 14. 比较运算符
template <typename R, typename... ArgTypes>
bool operator==(const function<R(ArgTypes...)>& f, std::nullptr_t) noexcept {
  return !f;
}

template <typename R, typename... ArgTypes>
bool operator==(std::nullptr_t, const function<R(ArgTypes...)>& f) noexcept {
  return !f;
}

template <typename R, typename... ArgTypes>
bool operator!=(const function<R(ArgTypes...)>& f, std::nullptr_t) noexcept {
  return static_cast<bool>(f);
}

template <typename R, typename... ArgTypes>
bool operator!=(std::nullptr_t, const function<R(ArgTypes...)>& f) noexcept {
  return static_cast<bool>(f);
}

// 15. 交换函数
template <typename R, typename... ArgTypes>
void swap(function<R(ArgTypes...)>& f1, function<R(ArgTypes...)>& f2) noexcept {
  f1.swap(f2);
}

}  // namespace mystl

#endif  // TINYSTL_FUNCTION_H
