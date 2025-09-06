#ifndef TINYSTL___FUNCTIONAL_FUNCTION_H
#define TINYSTL___FUNCTIONAL_FUNCTION_H

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace mystl {

template <typename Signature>
class function;

// __policy_storage + __use_small_storage 实现小对象优化
// 存储可调用对象
union __policy_storage {
  mutable char __small[sizeof(void*) * 2];
  void* __large;
};
// 判断一个类型 _Fun 是否可以安全地存储在 __policy_storage 的 __small 成员中
template <typename _Fun>
struct __use_small_storage
    : public std::integral_constant<
          bool, sizeof(_Fun) <= sizeof(__policy_storage) &&        // 大小合适
                    alignof(_Fun) <= alignof(__policy_storage) &&  // 对齐兼容
                    std::is_trivially_copy_constructible<
                        _Fun>::value &&  // 可平凡复制
                    std::is_trivially_destructible<_Fun>::value> {
};  // 可平凡析构

template <typename R, typename... Args>
class function_base {
 public:
  virtual ~function_base() {}
  virtual R invoke(Args&&... args) = 0;
  virtual std::unique_ptr<function_base> clone() const = 0;
};

template <typename T>
// 将类型 T 转换为其在函数参数传递时会衰减成的类型。
// 类似 remove_reference、remove_cv、（数组→指针，函数→函数指针）的有序执行
using decay_copy_t = std::decay_t<T>;

template <typename R, typename... Args>
class function<R(Args...)> {
 private:
  function_base<R, Args...>* func_ptr;  // 指向派生类（function_invoker）对象
  __policy_storage storage_;            // 栈上（__small）或堆上（__large）
  bool using_small_storage_;            // 判断是否使用栈上存储

  // 检查类型 F 是否具有复制或移动构造函数
  template <typename F>
  using constructible = std::enable_if_t<std::is_constructible_v<F, F>>;

  template <typename F>
  class function_invoker final : public function_base<R, Args...> {
    F f_;

   public:
    template <typename T, typename = constructible<T>>
    function_invoker(T&& f) : f_(std::forward<T>(f)) {}

    R invoke(Args&&... args) override {
      return f_(std::forward<Args>(args)...);
    }

    std::unique_ptr<function_base<R, Args...>> clone() const override {
      return std::make_unique<function_invoker>(f_);
    }
  };

  template <typename F>
  void destroy() {
    if (using_small_storage_) {
      using_small_storage_ = false;
      reinterpret_cast<F*>(&storage_)->~F();
    } else {
      delete func_ptr;
    }
  }

 public:
  function() : func_ptr(nullptr), using_small_storage_(false) {}

  function(std::nullptr_t) : function() {}

  template <typename F, typename = constructible<F>>
  function(F&& f) : func_ptr(nullptr), using_small_storage_(false) {
    using invoker_type = function_invoker<decay_copy_t<F>>;
    // invoker_type* 向上转型为 function_base*；这是类型擦除行为
    if (__use_small_storage<invoker_type>::value) {
      using_small_storage_ = true;
      new (&storage_) invoker_type(std::forward<F>(f));

      // 栈上
      func_ptr = reinterpret_cast<function_base<R, Args...>*>(&storage_);

    } else {
      // 堆上
      func_ptr = new invoker_type(std::forward<F>(f));
    }
  }

  function(const function& other)
      : func_ptr(nullptr), using_small_storage_(false) {
    if (other.func_ptr) {
      using invoker_type = function_invoker<decay_copy_t<function>>;
      if (__use_small_storage<invoker_type>::value) {
        using_small_storage_ = true;
        new (&storage_)
            invoker_type(*reinterpret_cast<invoker_type*>(other.func_ptr));
        func_ptr = reinterpret_cast<function_base<R, Args...>*>(&storage_);
      } else {
        func_ptr = other.func_ptr->clone().release();
      }
    }
  }

  function(function&& other) noexcept
      : func_ptr(other.func_ptr),
        storage_(other.storage_),
        using_small_storage_(other.using_small_storage_) {
    other.func_ptr = nullptr;
    other.using_small_storage_ = false;
  }

  function& operator=(const function& other) {
    function(other).swap(*this);
    return *this;
  }

  function& operator=(function&& other) noexcept {
    std::swap(func_ptr, other.func_ptr);
    std::swap(using_small_storage_, other.using_small_storage_);
    std::swap(storage_, other.storage_);
    return *this;
  }

  ~function() {
    if (func_ptr) {
      if (using_small_storage_) {
        using small_functor_type = std::remove_pointer_t<decltype(func_ptr)>;
        reinterpret_cast<small_functor_type>(&storage_)->~small_functor_type();
      } else {
        delete func_ptr;
      }
    }
  }

  R operator()(Args&&... args) const {
    if (!func_ptr) {
      throw std::bad_function_call();
    }
    return func_ptr->invoke(std::forward<Args>(args)...);
  }

  explicit operator bool() const noexcept { return func_ptr != nullptr; }

  void swap(function& other) noexcept {
    std::swap(func_ptr, other.func_ptr);
    std::swap(using_small_storage_, other.using_small_storage_);
    std::swap(storage_, other.storage_);
  }
};

template <typename R, typename... Args>
inline void swap(function<R(Args...)>& f1, function<R(Args...)>& f2) noexcept {
  f1.swap(f2);
}

}  // namespace mystl

#endif  // TINY___FUNCTIONAL_FUNCTION_H
