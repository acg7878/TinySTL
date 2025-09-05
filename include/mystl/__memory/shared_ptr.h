#ifndef TINYSTL___MEMOEY_SHARED_PTR_H
#define TINYSTL___MEMOEY_SHARED_PTR_H

#include <mystl/__utility/move.h>
#include <atomic>
#include <cstddef>
#include <memory>
#include "mystl/__utility/swap.h"

namespace mystl {

// 控制块基类
struct control_block_base {
  std::atomic<long> shared_owners{0};
  std::atomic<long> shared_weak_owners{0};
  control_block_base() = default;
  virtual ~control_block_base() = default;
  virtual void destroy_object() noexcept = 0;  // 销毁被管理对象

  void add_shared() noexcept {
    shared_owners.fetch_add(1, std::memory_order_acq_rel);
  }

  bool release_shared() noexcept {
    if (shared_owners.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      destroy_object();
      return true;
    }
    return false;
  }

  void add_weak() noexcept {
    shared_weak_owners.fetch_add(1, std::memory_order_acq_rel);
  }

  void release_weak() noexcept {
    if (shared_weak_owners.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      delete this;  // 删除控制块
    }
  }

  bool try_add_shared() noexcept {
    long count = shared_owners.load(std::memory_order_acquire);
    while (count != 0) {
      if (shared_owners.compare_exchange_weak(count, count + 1,
                                              std::memory_order_acq_rel,
                                              std::memory_order_acquire))
        return true;
    }
    return false;
  }
};

// 控制块
template <typename T, typename Deleter>
struct control_block : control_block_base {
  T* ptr; // 提供给删除器，与shared_ptr的不同，那个是提供给用户的
  Deleter deleter;
  control_block(T* p, Deleter d) : ptr(p), deleter(mystl::move(d)) {}
  void destroy_object() noexcept override {
    if (ptr) {
      deleter(ptr);
      ptr = nullptr;
    }
  }
};

template <typename T>
class weak_ptr;

template <typename T>
class shared_ptr {
 public:
  using element_type = T;

  constexpr shared_ptr() : ptr_(nullptr), ctrl_(nullptr) {}
  constexpr shared_ptr(nullptr_t) noexcept : shared_ptr() {}  // 委托构造

  template <typename Deleter = std::default_delete<T>>
  explicit shared_ptr(T* p, Deleter d = Deleter())
      : ptr_(p),
        ctrl_(p ? static_cast<control_block_base*>(
                      new control_block<T, Deleter>(p, mystl::move(d)))
                : nullptr) {
    if (ctrl_) {
      ctrl_->add_shared();
      ctrl_->add_weak();
      // 创建一个强引用，这可以理解因为创建了一个对象
      // 还创建一个隐式弱引用，为什么是隐式呢因为没创建weak_ptr
      // 这个隐式弱引用用于关联强引用和弱引用的生命周期，确保 强引用归零释放资源后
      // 控制块不会立即被删除，因为还有隐式弱引用，直到所有 weak_ptr 也释放（弱引用计数归零），控制块才会被删除。
    }
  }

  // 拷贝
  shared_ptr(const shared_ptr& other) noexcept
      : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    if (ctrl_) {
      ctrl_->add_shared();
    }
  }

  // 移动
  shared_ptr(shared_ptr&& other) noexcept
      : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    other.ptr_ = nullptr;
    other.ctrl_ = nullptr;
  }

  ~shared_ptr() { release(); }

  // 拷贝赋值
  shared_ptr& operator=(const shared_ptr& other) noexcept {
    if (this != &other) {
      release(); // 记得释放！因为有赋值，原来的应该release掉！
      ptr_ = other.ptr_;
      ctrl_ = other.ctrl_;
      if (ctrl_) {
        ctrl_->add_shared();
      }
    }
    return *this;
  }

  // 移动赋值
  shared_ptr& operator=(shared_ptr&& other) {
    if (this != &other) {
      release();
      ptr_ = other.ptr_;
      ctrl_ = other.ctrl_;
      other.ptr_ = nullptr;
      other.ctrl_ = nullptr;
    }
    return *this;
  }

  void reset() noexcept { shared_ptr().swap(*this); }

  void swap(shared_ptr& other) noexcept {
    mystl::swap(ptr_, other.ptr_);
    mystl::swap(ctrl_, other.ctrl_);
  }

  T* get() const noexcept { return ptr_; }
  T& operator*() noexcept { return *ptr_; }
  const T& operator*() const noexcept { return *ptr_; }
  T* operator->() noexcept { return ptr_; }
  const T* operator->() const noexcept { return ptr_; }

  long use_count() const noexcept {
    return ctrl_ ? ctrl_->shared_owners.load(std::memory_order_acquire) : 0;
  }
  explicit operator bool() const noexcept { return ptr_ != nullptr; }
  // shared_ptr<int> sp(new int(10));
  // int x = *sp;
  // 不加explicit 编译器会执行两步隐式转换（*ptr_ -> bool -> int）

 private:
  template <typename U>
  friend class weak_ptr;

  // shared_ptr<T> 与 shared_ptr<U> 相互查看private成员
  template <typename U>
  friend class shared_ptr;

  T* ptr_;
  control_block_base* ctrl_;

  shared_ptr(T* p, control_block_base* ctrl) : ptr_(p), ctrl_(ctrl) {}

  void release() {
    if (!ctrl_)
      return;
    if (ctrl_->release_shared()) {
      ctrl_->release_weak();
      // 强计数归零后需要减少掉隐式弱引用
    }
    ptr_ = nullptr;
    ctrl_ = nullptr;
  }
};

template <typename T>
class weak_ptr {
 public:
  constexpr weak_ptr() : ptr_(nullptr), ctrl_(nullptr) {}

  // 从 shared_ptr 构造
  weak_ptr(const shared_ptr<T>& sp) noexcept : ptr_(sp.ptr_), ctrl_(sp.ctrl_) {
    if (ctrl_)
      ctrl_->add_weak();
  }

  // 拷贝构造
  weak_ptr(const weak_ptr& other) noexcept
      : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    if (ctrl_) {
      ctrl_->add_weak();
    }
  }

  // 移动构造
  weak_ptr(weak_ptr&& other) : ptr_(other.ptr_), ctrl_(other.ctrl_) {
    other.ctrl_ = nullptr;
    other.ptr_ = nullptr;
  }

  // 构析
  ~weak_ptr() { release(); }

  weak_ptr& operator=(const weak_ptr& other) {
    if (this != &other) {
      release();
      ptr_ = other.ptr_;
      ctrl_ = other.ctrl_;
      if (ctrl_)
        ctrl_->add_weak();
    }
    return *this;
  }

  // 重置为空，与一个空的weak_ptr进行交换，然后这个临时的weak_ptr会在语句结束会自动构析
  void reset() noexcept { weak_ptr().swap(*this); }

  void swap(weak_ptr& other) {
    mystl::swap(this->ptr_, other.ptr_);
    mystl::swap(this->ctrl_, other.ctrl_);
  }

  long use_count() const noexcept {
    return ctrl_ ? ctrl_->shared_owners.load(std::memory_order_acquire) : 0;
  }

  bool expired() const noexcept { return use_count() == 0; }

  shared_ptr<T> lock() const noexcept {
    if (!ctrl_)
      return shared_ptr<T>();
    if (ctrl_->try_add_shared()) {
      return shared_ptr<T>(ptr_, ctrl_);
    }
    return shared_ptr<T>();
  }

 private:
  template <typename U>
  friend class weak_ptr;

  T* ptr_;
  control_block_base* ctrl_;

  void release() {
    if (!ctrl_)
      return;
    ctrl_->release_weak();
    ptr_ = nullptr;
    ctrl_ = nullptr;
  }
};

}  // namespace mystl

#endif