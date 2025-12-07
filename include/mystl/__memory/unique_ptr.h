#ifndef TINYSTL___MEMORY_UNIQUE_PTR_H
#define TINYSTL___MEMORY_UNIQUE_PTR_H

#include <mystl/__utility/forward.h>
#include <mystl/__utility/move.h>
#include <mystl/__utility/swap.h>
#include <cstddef>      // for size_t, nullptr_t
#include <type_traits>  // for enable_if, is_array, is_reference, etc.

namespace mystl {

// ============================================================================
// default_delete
// ============================================================================
template <typename T>
struct default_delete {
  constexpr default_delete() noexcept = default;

  template <typename U, typename = typename std::enable_if<
                            std::is_convertible<U*, T*>::value>::type>
  default_delete(const default_delete<U>&) noexcept {}

  void operator()(T* ptr) const noexcept {
    static_assert(sizeof(T) > 0, "can't delete an incomplete type");
    delete ptr;
  }
};

template <typename T>
struct default_delete<T[]> {
  constexpr default_delete() noexcept = default;

  template <typename U, typename = typename std::enable_if<
                            std::is_convertible<U (*)[], T (*)[]>::value>::type>
  default_delete(const default_delete<U[]>&) noexcept {}

  template <typename U, typename = typename std::enable_if<
                            std::is_convertible<U (*)[], T (*)[]>::value>::type>
  void operator()(U* ptr) const noexcept {
    static_assert(sizeof(T) > 0, "can't delete an incomplete type");
    delete[] ptr;
  }
};

// ============================================================================
// __compressed_pair: 空基类优化 (EBO)
// 如果 T1 是空类且非 final，则继承它以节省空间
// ============================================================================
template <typename T1, typename T2,
          bool = std::is_empty<T1>::value && !std::is_final<T1>::value>
class __compressed_pair : private T1 {
 private:
  T2 value2_;

 public:
  template <typename... Args>
  explicit __compressed_pair(Args&&... args)
      : T1(), value2_(mystl::forward<Args>(args)...) {}

  template <typename U1, typename U2>
  __compressed_pair(U1&& v1, U2&& v2)
      : T1(mystl::forward<U1>(v1)), value2_(mystl::forward<U2>(v2)) {}

  T1& first() noexcept { return *this; }
  const T1& first() const noexcept { return *this; }
  T2& second() noexcept { return value2_; }
  const T2& second() const noexcept { return value2_; }
};

// 特化：不进行 EBO
template <typename T1, typename T2>
class __compressed_pair<T1, T2, false> {
 private:
  T1 value1_;
  T2 value2_;

 public:
  template <typename... Args>
  explicit __compressed_pair(Args&&... args)
      : value1_(), value2_(mystl::forward<Args>(args)...) {}

  template <typename U1, typename U2>
  __compressed_pair(U1&& v1, U2&& v2)
      : value1_(mystl::forward<U1>(v1)), value2_(mystl::forward<U2>(v2)) {}

  T1& first() noexcept { return value1_; }
  const T1& first() const noexcept { return value1_; }
  T2& second() noexcept { return value2_; }
  const T2& second() const noexcept { return value2_; }
};

// ============================================================================
// unique_ptr 主模板
// ============================================================================
template <typename T, typename Deleter = default_delete<T>>
class unique_ptr {
 public:
  using pointer = T*;  // 这里简化了，标准库会检测 Deleter::pointer
  using element_type = T;
  using deleter_type = Deleter;

 private:
  // 使用 compressed_pair 存储 deleter 和 pointer
  // 注意：我们将 Deleter 放在 first 以便利用 EBO
  using Storage = __compressed_pair<deleter_type, pointer>;
  Storage storage_;

 public:
  // 构造函数
  constexpr unique_ptr() noexcept : storage_(deleter_type(), nullptr) {}
  constexpr unique_ptr(std::nullptr_t) noexcept
      : storage_(deleter_type(), nullptr) {}

  explicit unique_ptr(pointer p) noexcept : storage_(deleter_type(), p) {}

  unique_ptr(pointer p,
             typename std::conditional<std::is_reference<deleter_type>::value,
                                       deleter_type, const deleter_type&>::type
                 d) noexcept
      : storage_(d, p) {}

  unique_ptr(pointer p, typename std::remove_reference<deleter_type>::type&& d)
      : storage_(mystl::move(d), p) {
    static_assert(!std::is_reference<deleter_type>::value,
                  "rvalue deleter bound to reference");
  }

  // 移动构造
  unique_ptr(unique_ptr&& u) noexcept
      : storage_(mystl::forward<deleter_type>(u.get_deleter()), u.release()) {}

  // 转换构造：允许从派生类的 unique_ptr 移动构造
  template <typename U, typename E,
            typename = typename std::enable_if<
                std::is_convertible<typename unique_ptr<U, E>::pointer,
                                    pointer>::value &&
                !std::is_array<U>::value &&
                (std::is_reference<deleter_type>::value
                     ? std::is_same<deleter_type, E>::value
                     : std::is_convertible<E, deleter_type>::value)>::type>
  unique_ptr(unique_ptr<U, E>&& u) noexcept
      : storage_(mystl::forward<E>(u.get_deleter()), u.release()) {}

  // 析构函数
  ~unique_ptr() { reset(); }

  // 移动赋值
  unique_ptr& operator=(unique_ptr&& u) noexcept {
    if (this != &u) {
      reset(u.release());
      get_deleter() = mystl::forward<deleter_type>(u.get_deleter());
    }
    return *this;
  }

  template <typename U, typename E>
  unique_ptr& operator=(unique_ptr<U, E>&& u) noexcept {
    reset(u.release());
    get_deleter() = mystl::forward<E>(u.get_deleter());
    return *this;
  }

  unique_ptr& operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  // 禁止拷贝
  unique_ptr(const unique_ptr&) = delete;
  unique_ptr& operator=(const unique_ptr&) = delete;

  // 观察器
  typename std::add_lvalue_reference<T>::type operator*() const {
    return *storage_.second();
  }

  pointer operator->() const noexcept { return storage_.second(); }

  pointer get() const noexcept { return storage_.second(); }

  deleter_type& get_deleter() noexcept { return storage_.first(); }
  const deleter_type& get_deleter() const noexcept { return storage_.first(); }

  explicit operator bool() const noexcept { return get() != nullptr; }

  // 修改器
  pointer release() noexcept {
    pointer p = get();
    storage_.second() = nullptr;
    return p;
  }

  void reset(pointer p = nullptr) noexcept {
    pointer old_p = storage_.second();
    storage_.second() = p;
    if (old_p) {
      get_deleter()(old_p);
    }
  }

  void swap(unique_ptr& u) noexcept {
    using mystl::swap;
    swap(storage_.first(), u.storage_.first());
    swap(storage_.second(), u.storage_.second());
  }
};

// ============================================================================
// unique_ptr 数组特化
// ============================================================================
template <typename T, typename Deleter>
class unique_ptr<T[], Deleter> {
 public:
  using pointer = T*;
  using element_type = T;
  using deleter_type = Deleter;

 private:
  using Storage = __compressed_pair<deleter_type, pointer>;
  Storage storage_;

 public:
  // 构造函数
  constexpr unique_ptr() noexcept : storage_(deleter_type(), nullptr) {}
  constexpr unique_ptr(std::nullptr_t) noexcept
      : storage_(deleter_type(), nullptr) {}

  template <typename U, typename = typename std::enable_if<std::is_convertible<
                            U, pointer>::value>::type>  // 更复杂的检查略去
  explicit unique_ptr(U p) noexcept : storage_(deleter_type(), p) {}

  unique_ptr(pointer p,
             typename std::conditional<std::is_reference<deleter_type>::value,
                                       deleter_type, const deleter_type&>::type
                 d) noexcept
      : storage_(d, p) {}

  unique_ptr(pointer p, typename std::remove_reference<deleter_type>::type&& d)
      : storage_(mystl::move(d), p) {}

  // 移动构造
  unique_ptr(unique_ptr&& u) noexcept
      : storage_(mystl::forward<deleter_type>(u.get_deleter()), u.release()) {}

  // 析构
  ~unique_ptr() { reset(); }

  // 移动赋值
  unique_ptr& operator=(unique_ptr&& u) noexcept {
    if (this != &u) {
      reset(u.release());
      get_deleter() = mystl::forward<deleter_type>(u.get_deleter());
    }
    return *this;
  }

  unique_ptr& operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  // 禁止拷贝
  unique_ptr(const unique_ptr&) = delete;
  unique_ptr& operator=(const unique_ptr&) = delete;

  // 数组特有接口
  T& operator[](size_t i) const { return get()[i]; }

  pointer get() const noexcept { return storage_.second(); }
  deleter_type& get_deleter() noexcept { return storage_.first(); }
  const deleter_type& get_deleter() const noexcept { return storage_.first(); }

  explicit operator bool() const noexcept { return get() != nullptr; }

  pointer release() noexcept {
    pointer p = get();
    storage_.second() = nullptr;
    return p;
  }

  void reset(pointer p = nullptr) noexcept {
    pointer old_p = storage_.second();
    storage_.second() = p;
    if (old_p) {
      get_deleter()(old_p);
    }
  }

  void swap(unique_ptr& u) noexcept {
    using mystl::swap;
    swap(storage_.first(), u.storage_.first());
    swap(storage_.second(), u.storage_.second());
  }
};

// ============================================================================
// make_unique
// ============================================================================

// 1. 非数组版本
template <typename T, typename... Args>
typename std::enable_if<!std::is_array<T>::value, unique_ptr<T>>::type
make_unique(Args&&... args) {
  return unique_ptr<T>(new T(mystl::forward<Args>(args)...));
}

// 2. 数组版本 (make_unique<T[]>(size))
template <typename T>
typename std::enable_if<std::is_array<T>::value, unique_ptr<T>>::type
make_unique(size_t size) {
  using Elem = typename std::remove_extent<T>::type;
  return unique_ptr<T>(new Elem[size]());
}

// 3. 禁用已知边界数组版本 (make_unique<T[N]>)
template <typename T, typename... Args>
typename std::enable_if<std::extent<T>::value != 0, void>::type make_unique(
    Args&&...) = delete;

// ============================================================================
// 比较操作符
// ============================================================================
template <typename T1, typename D1, typename T2, typename D2>
bool operator==(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
  return x.get() == y.get();
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator!=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
  return !(x == y);
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator<(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
  return x.get() < y.get();
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator<=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
  return !(y < x);
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator>(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
  return y < x;
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator>=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y) {
  return !(x < y);
}

// 与 nullptr 比较
template <typename T, typename D>
bool operator==(const unique_ptr<T, D>& x, std::nullptr_t) noexcept {
  return !x;
}

template <typename T, typename D>
bool operator==(std::nullptr_t, const unique_ptr<T, D>& x) noexcept {
  return !x;
}

template <typename T, typename D>
bool operator!=(const unique_ptr<T, D>& x, std::nullptr_t) noexcept {
  return (bool)x;
}

template <typename T, typename D>
bool operator!=(std::nullptr_t, const unique_ptr<T, D>& x) noexcept {
  return (bool)x;
}

// swap 非成员函数
template <typename T, typename D>
void swap(unique_ptr<T, D>& x, unique_ptr<T, D>& y) noexcept {
  x.swap(y);
}

}  // namespace mystl

#endif  // TINYSTL