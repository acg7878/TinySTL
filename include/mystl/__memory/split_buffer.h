#ifndef TINYSTL___MEMORY_SPLIT_BUFFER_H
#define TINYSTL___MEMORY_SPLIT_BUFFER_H

#include <mystl/__utility/move.h>
#include <mystl/__utility/forward.h>
#include <mystl/__iterator/distance.h>
#include <mystl/__type_traits/remove_reference.h>
#include <mystl/__type_traits/integral_constant.h>
#include <memory>
#include <algorithm>  // 使用 std::move, std::move_backward
#include <cstddef>

namespace mystl {

// split_buffer 分配一块连续内存，在 [begin_, end_) 范围内存储对象
// 在 [first_, begin_) 和 [end_, cap_) 范围内有未初始化的内存
// 这样可以在前后两端增长而不需要移动数据

template <class Tp, class Allocator = std::allocator<Tp>>
struct split_buffer {
 public:
  using value_type = Tp;
  using allocator_type = Allocator;
  using alloc_rr = typename mystl::remove_reference<allocator_type>::type;
  using alloc_traits = std::allocator_traits<alloc_rr>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = typename alloc_traits::size_type;
  using difference_type = typename alloc_traits::difference_type;
  using pointer = typename alloc_traits::pointer;
  using const_pointer = typename alloc_traits::const_pointer;
  using iterator = pointer;
  using const_iterator = const_pointer;

  // 四个指针：管理内存布局
  // [first_, begin_) : 前端空闲空间
  // [begin_, end_)   : 已使用的数据
  // [end_, cap_)     : 后端空闲空间
  pointer first_;  // 内存块起始位置
  pointer begin_;  // 数据起始位置
  pointer end_;    // 数据结束位置
  pointer cap_;    // 内存块结束位置
  allocator_type alloc_;  // 分配器

  // 禁止拷贝构造和拷贝赋值
  split_buffer(const split_buffer&) = delete;
  split_buffer& operator=(const split_buffer&) = delete;

  // 默认构造函数
  split_buffer() noexcept : first_(nullptr), begin_(nullptr), end_(nullptr), cap_(nullptr) {}

  // 使用分配器构造
  explicit split_buffer(alloc_rr& a) : first_(nullptr), begin_(nullptr), end_(nullptr), cap_(nullptr), alloc_(a) {}

  explicit split_buffer(const alloc_rr& a) : first_(nullptr), begin_(nullptr), end_(nullptr), cap_(nullptr), alloc_(a) {}

  // 带容量和起始位置的构造函数
  split_buffer(size_type cap, size_type start, alloc_rr& a);

  // 移动构造
  split_buffer(split_buffer&& c) noexcept;
  split_buffer(split_buffer&& c, const alloc_rr& a);

  // 移动赋值
  split_buffer& operator=(split_buffer&& c) noexcept;

  // 析构函数
  ~split_buffer();

  // 迭代器
  iterator begin() noexcept { return begin_; }
  const_iterator begin() const noexcept { return begin_; }
  iterator end() noexcept { return end_; }
  const_iterator end() const noexcept { return end_; }

  // 容量相关
  void clear() noexcept { destruct_at_end(begin_); }
  size_type size() const { return static_cast<size_type>(end_ - begin_); }
  bool empty() const { return end_ == begin_; }
  size_type capacity() const { return static_cast<size_type>(cap_ - first_); }
  size_type front_spare() const { return static_cast<size_type>(begin_ - first_); }
  size_type back_spare() const { return static_cast<size_type>(cap_ - end_); }

  // 元素访问
  reference front() { return *begin_; }
  const_reference front() const { return *begin_; }
  reference back() { return *(end_ - 1); }
  const_reference back() const { return *(end_ - 1); }

  // 修改器
  void shrink_to_fit() noexcept;
  template <class... Args>
  void emplace_front(Args&&... args);
  template <class... Args>
  void emplace_back(Args&&... args);
  void pop_front() { destruct_at_begin(begin_ + 1); }
  void pop_back() { destruct_at_end(end_ - 1); }

  // 构造相关
  void construct_at_end(size_type n);
  void construct_at_end(size_type n, const_reference x);
  template <class ForwardIterator>
  void construct_at_end(ForwardIterator first, ForwardIterator last);
  template <class Iterator, class Sentinel>
  void construct_at_end_with_sentinel(Iterator first, Sentinel last);
  template <class Iterator>
  void construct_at_end_with_size(Iterator first, size_type n);

  // 交换
  void swap(split_buffer& x) noexcept;

 private:
  // 析构相关
  void destruct_at_begin(pointer new_begin);
  void destruct_at_end(pointer new_end) noexcept;

  // 移动分配器
  void move_assign_alloc(split_buffer& c, true_type) noexcept;
  void move_assign_alloc(split_buffer&, false_type) noexcept {}

  // 构造事务：用于异常安全
  struct ConstructTransaction {
    explicit ConstructTransaction(pointer* p, size_type n) noexcept
        : pos_(*p), end_(*p + n), dest_(p) {}

    ~ConstructTransaction() { *dest_ = pos_; }

    pointer pos_;
    const pointer end_;

   private:
    pointer* dest_;
  };
};

// 构造函数实现
template <class Tp, class Allocator>
split_buffer<Tp, Allocator>::split_buffer(size_type cap, size_type start, alloc_rr& a)
    : cap_(nullptr), alloc_(a) {
  if (cap == 0) {
    first_ = nullptr;
  } else {
    first_ = alloc_traits::allocate(alloc_, cap);
  }
  begin_ = end_ = first_ + start;
  cap_ = first_ + cap;
}

// 析构函数
template <class Tp, class Allocator>
split_buffer<Tp, Allocator>::~split_buffer() {
  clear();
  if (first_) {
    alloc_traits::deallocate(alloc_, first_, capacity());
  }
}

// 移动构造
template <class Tp, class Allocator>
split_buffer<Tp, Allocator>::split_buffer(split_buffer&& c) noexcept
    : first_(move(c.first_)),
      begin_(move(c.begin_)),
      end_(move(c.end_)),
      cap_(move(c.cap_)),
      alloc_(move(c.alloc_)) {
  c.first_ = nullptr;
  c.begin_ = nullptr;
  c.end_ = nullptr;
  c.cap_ = nullptr;
}

template <class Tp, class Allocator>
split_buffer<Tp, Allocator>::split_buffer(split_buffer&& c, const alloc_rr& a)
    : cap_(nullptr), alloc_(a) {
  if (a == c.alloc_) {
    first_ = c.first_;
    begin_ = c.begin_;
    end_ = c.end_;
    cap_ = c.cap_;
    c.first_ = nullptr;
    c.begin_ = nullptr;
    c.end_ = nullptr;
    c.cap_ = nullptr;
  } else {
    first_ = alloc_traits::allocate(alloc_, c.size());
    begin_ = end_ = first_;
    cap_ = first_ + c.size();
    // 使用 std::move_iterator（标准库版本）
    typedef std::move_iterator<iterator> Ip;
    construct_at_end(Ip(c.begin_), Ip(c.end_));
  }
}

// 移动赋值
template <class Tp, class Allocator>
split_buffer<Tp, Allocator>& split_buffer<Tp, Allocator>::operator=(split_buffer&& c) noexcept {
  clear();
  shrink_to_fit();
  first_ = c.first_;
  begin_ = c.begin_;
  end_ = c.end_;
  cap_ = c.cap_;
  alloc_ = move(c.alloc_);
  c.first_ = c.begin_ = c.end_ = c.cap_ = nullptr;
  return *this;
}

// 交换
template <class Tp, class Allocator>
void split_buffer<Tp, Allocator>::swap(split_buffer& x) noexcept {
  std::swap(first_, x.first_);
  std::swap(begin_, x.begin_);
  std::swap(end_, x.end_);
  std::swap(cap_, x.cap_);
  std::swap(alloc_, x.alloc_);
}

// 析构函数实现
template <class Tp, class Allocator>
void split_buffer<Tp, Allocator>::destruct_at_begin(pointer new_begin) {
  while (begin_ != new_begin) {
    alloc_traits::destroy(alloc_, begin_++);
  }
}

template <class Tp, class Allocator>
void split_buffer<Tp, Allocator>::destruct_at_end(pointer new_end) noexcept {
  while (new_end != end_) {
    alloc_traits::destroy(alloc_, --end_);
  }
}

// 构造相关实现
template <class Tp, class Allocator>
void split_buffer<Tp, Allocator>::construct_at_end(size_type n) {
  ConstructTransaction tx(&this->end_, n);
  for (; tx.pos_ != tx.end_; ++tx.pos_) {
    alloc_traits::construct(alloc_, tx.pos_);
  }
}

template <class Tp, class Allocator>
void split_buffer<Tp, Allocator>::construct_at_end(size_type n, const_reference x) {
  ConstructTransaction tx(&this->end_, n);
  for (; tx.pos_ != tx.end_; ++tx.pos_) {
    alloc_traits::construct(alloc_, tx.pos_, x);
  }
}

template <class Tp, class Allocator>
template <class Iterator>
void split_buffer<Tp, Allocator>::construct_at_end_with_size(Iterator first, size_type n) {
  ConstructTransaction tx(&this->end_, n);
  for (; tx.pos_ != tx.end_; ++tx.pos_, ++first) {
    alloc_traits::construct(alloc_, tx.pos_, *first);
  }
}

template <class Tp, class Allocator>
template <class ForwardIterator>
void split_buffer<Tp, Allocator>::construct_at_end(ForwardIterator first, ForwardIterator last) {
  construct_at_end_with_size(first, distance(first, last));
}

template <class Tp, class Allocator>
template <class Iterator, class Sentinel>
void split_buffer<Tp, Allocator>::construct_at_end_with_sentinel(Iterator first, Sentinel last) {
  alloc_rr& a = alloc_;
  for (; first != last; ++first) {
    if (end_ == cap_) {
      size_type old_cap = cap_ - first_;
      size_type new_cap = std::max<size_type>(2 * old_cap, 8);
      split_buffer buf(new_cap, 0, a);
      for (pointer p = begin_; p != end_; ++p, ++buf.end_) {
        alloc_traits::construct(alloc_, buf.end_, move(*p));
      }
      swap(buf);
    }
    alloc_traits::construct(alloc_, end_, *first);
    ++end_;
  }
}

// shrink_to_fit
template <class Tp, class Allocator>
void split_buffer<Tp, Allocator>::shrink_to_fit() noexcept {
  if (capacity() > size()) {
    try {
      split_buffer<value_type, alloc_rr&> t(size(), 0, alloc_);
      if (t.capacity() < capacity()) {
        // 使用 std::move_iterator
        typedef std::move_iterator<iterator> Ip;
        t.construct_at_end(Ip(begin_), Ip(end_));
        t.end_ = t.begin_ + (end_ - begin_);
        std::swap(first_, t.first_);
        std::swap(begin_, t.begin_);
        std::swap(end_, t.end_);
        std::swap(cap_, t.cap_);
      }
    } catch (...) {
      // 忽略异常
    }
  }
}

// emplace_front
template <class Tp, class Allocator>
template <class... Args>
void split_buffer<Tp, Allocator>::emplace_front(Args&&... args) {
  if (begin_ == first_) {
    if (end_ < cap_) {
      // 有后端空间，移动数据向后
      difference_type d = cap_ - end_;
      d = (d + 1) / 2;
      // 使用 std::move_backward 移动数据（从后向前移动）
      std::move_backward(begin_, end_, end_ + d);
      begin_ = end_ + d - (end_ - begin_);
      end_ = end_ + d;
    } else {
      // 需要扩容
      size_type c = std::max<size_type>(2 * static_cast<size_type>(cap_ - first_), 1);
      split_buffer<value_type, alloc_rr&> t(c, (c + 3) / 4, alloc_);
      // 使用 std::move_iterator
      typedef std::move_iterator<iterator> Ip;
      t.construct_at_end(Ip(begin_), Ip(end_));
      std::swap(first_, t.first_);
      std::swap(begin_, t.begin_);
      std::swap(end_, t.end_);
      std::swap(cap_, t.cap_);
    }
  }
  alloc_traits::construct(alloc_, begin_ - 1, mystl::forward<Args>(args)...);
  --begin_;
}

// emplace_back
template <class Tp, class Allocator>
template <class... Args>
void split_buffer<Tp, Allocator>::emplace_back(Args&&... args) {
  if (end_ == cap_) {
    if (begin_ > first_) {
      // 有前端空间，移动数据向前
      difference_type d = begin_ - first_;
      d = (d + 1) / 2;
      // 使用 std::move 移动数据
      end_ = std::move(begin_, end_, begin_ - d);
      begin_ = begin_ - d;
    } else {
      // 需要扩容
      size_type c = std::max<size_type>(2 * static_cast<size_type>(cap_ - first_), 1);
      split_buffer<value_type, alloc_rr&> t(c, c / 4, alloc_);
      // 使用 std::move_iterator
      typedef std::move_iterator<iterator> Ip;
      t.construct_at_end(Ip(begin_), Ip(end_));
      std::swap(first_, t.first_);
      std::swap(begin_, t.begin_);
      std::swap(end_, t.end_);
      std::swap(cap_, t.cap_);
    }
  }
  alloc_traits::construct(alloc_, end_, mystl::forward<Args>(args)...);
  ++end_;
}

// 非成员函数：swap
template <class Tp, class Allocator>
void swap(split_buffer<Tp, Allocator>& x, split_buffer<Tp, Allocator>& y) noexcept(noexcept(x.swap(y))) {
  x.swap(y);
}

}  // namespace mystl

#endif  // TINYSTL___MEMORY_SPLIT_BUFFER_H

