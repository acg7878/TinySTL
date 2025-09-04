#ifndef TINYSTL___MEMOEY_UNIQUE_PTR_H
#define TINYSTL___MEMOEY_UNIQUE_PTR_H

#include <mystl/utility.h>
#include <cstddef>
#include "mystl/__utility/move.h"
#include "mystl/__utility/swap.h"

namespace mystl {

template <typename T>
struct default_delete {
  constexpr default_delete() noexcept = default;
  void operator()(T* ptr) const noexcept { delete ptr; }
};

template <typename T>
struct default_delete<T[]> {
  constexpr default_delete() noexcept = default;
  void operator()(T* ptr) const noexcept { delete[] ptr; }
};

template <typename T, typename Deleter = default_delete<T>>
struct unique_ptr {
 public:
  using pointer = T*;
  using delete_type = Deleter;
  using element_type = T;

 private:
  pointer ptr_;
  delete_type deleter_;

 public:
  explicit unique_ptr(pointer p = nullptr) noexcept : ptr_(p), deleter_() {}
  unique_ptr(pointer p, delete_type deleter) noexcept
      : ptr_(p), deleter_(mystl::move(deleter)) {}
  ~unique_ptr() { reset(); }

  // 移动语义
  unique_ptr(unique_ptr&& u) noexcept
      : ptr_(u.release()), deleter_(mystl::move(u.deleter_)) {}
  unique_ptr& operator=(unique_ptr&& u) noexcept {
    reset(u.release());
    deleter_ = mystl::move(u.deleter_);
    return *this;
  }

  // 禁止拷贝
  unique_ptr(const unique_ptr&) = delete;
  unique_ptr& operator=(const unique_ptr&) = delete;

  // 基本接口
  pointer get() noexcept { return ptr_; }
  const pointer get() const noexcept { return ptr_; }

  delete_type& get_deleter() noexcept { return deleter_; }
  const delete_type& get_deleter() const noexcept { return deleter_; }

  pointer release() noexcept {
    pointer tmp = ptr_;
    ptr_ = nullptr;
    return tmp;
  }

  void reset(pointer p = nullptr) {
    if (ptr_) {
      deleter_(ptr_);
    }
    ptr_ = p;
  }

  void swap(unique_ptr& u) noexcept {
    mystl::swap(ptr_, u.ptr_);
    mystl::swap(deleter_, u.deleter_);
  }
};

// 数组版本
// 默认参数只能出现在主模板，特化模板不能写默认参数
template <typename T>
class unique_ptr<T[], default_delete<T[]>> {
 public:
  using element_type = T;
  using deleter_type = default_delete<T[]>;
  using pointer = T*;

 private:
  pointer ptr_;
  deleter_type deleter_;

 public:
  // 构造/析构
  explicit unique_ptr(pointer p = nullptr) noexcept : ptr_(p), deleter_() {}
  unique_ptr(pointer p, deleter_type d) noexcept
      : ptr_(p), deleter_(mystl::move(d)) {}
  ~unique_ptr() { reset(); }

  // 移动语义
  unique_ptr(unique_ptr&& u) noexcept
      : ptr_(u.release()), deleter_(mystl::move(u.deleter_)) {}
  unique_ptr& operator=(unique_ptr&& u) noexcept {
    reset(u.release());
    deleter_ = mystl::move(u.deleter_);
    return *this;
  }

  // 禁止拷贝
  unique_ptr(const unique_ptr&) = delete;
  unique_ptr& operator=(const unique_ptr&) = delete;

  // 基本接口
  pointer get() const noexcept { return ptr_; }
  deleter_type& get_deleter() noexcept { return deleter_; }
  const deleter_type& get_deleter() const noexcept { return deleter_; }

  pointer release() noexcept {
    pointer tmp = ptr_;
    ptr_ = nullptr;
    return tmp;
  }

  void reset(pointer p = nullptr) noexcept {
    if (ptr_)
      deleter_(ptr_);
    ptr_ = p;
  }

  void swap(unique_ptr& u) noexcept {
    mystl::swap(ptr_, u.ptr_);
    mystl::swap(deleter_, u.deleter_);
  }

  T& operator[](size_t i) noexcept { return ptr_[i]; }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }
  /*
    显式转换为bool，判断是否持有资源
    if (ptr) {  // 等价于 if (ptr.operator bool())
      std::cout << "ptr 管理着资源" << std::endl;
    }
  */
};
}  // namespace mystl

#endif