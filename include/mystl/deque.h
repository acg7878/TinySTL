#ifndef TINYSTL_DEQUE_H
#define TINYSTL_DEQUE_H

#include <mystl/__memory/split_buffer.h>
#include <mystl/__iterator/iterator_traits.h>
#include <mystl/__type_traits/enable_if.h>
#include <mystl/__type_traits/is_same.h>
#include <mystl/__type_traits/is_integral.h>
#include <mystl/__utility/move.h>
#include <mystl/__utility/forward.h>
#include <mystl/__utility/swap.h>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <initializer_list>

namespace mystl {

// 计算 deque 块大小的辅助结构
template <class ValueType, class DiffType>
struct deque_block_size {
  static const DiffType value =
      sizeof(ValueType) < 256 ? 4096 / sizeof(ValueType) : 16;
};

// deque 迭代器
template <class ValueType,
          class Pointer,
          class Reference,
          class MapPointer,
          class DiffType,
          DiffType BS = deque_block_size<ValueType, DiffType>::value>
class deque_iterator {
 public:
  using value_type = ValueType;
  using pointer = Pointer;
  using reference = Reference;
  using difference_type = DiffType;
  using iterator_category = mystl::random_access_iterator_tag;

 private:
  MapPointer m_iter_;  // 指向 map 中某个块的指针
  pointer ptr_;         // 指向当前块中的元素

  static const difference_type block_size_;

 public:
  // 构造函数
  deque_iterator() noexcept : m_iter_(nullptr), ptr_(nullptr) {}

  // 从非 const 迭代器转换为 const 迭代器
  template <class Pp, class Rp, class MP,
            typename = typename enable_if<
                std::is_convertible<Pp, pointer>::value>::type>
  deque_iterator(const deque_iterator<value_type, Pp, Rp, MP, difference_type, BS>& it) noexcept
      : m_iter_(it.m_iter_), ptr_(it.ptr_) {}

  // 私有构造函数：供 deque 类使用
 private:
  explicit deque_iterator(MapPointer m, pointer p) noexcept
      : m_iter_(m), ptr_(p) {}

  // 让 deque 类成为友元，可以访问私有构造函数
  template <class Tp, class Alloc>
  friend class deque;

  template <class Vp, class Pp, class Rp, class MP, class Dp, Dp>
  friend class deque_iterator;

 public:
  // 解引用操作
  reference operator*() const { return *ptr_; }
  pointer operator->() const { return ptr_; }

  // 前置递增
  deque_iterator& operator++() {
    if (++ptr_ - *m_iter_ == block_size_) {
      ++m_iter_;
      ptr_ = *m_iter_;
    }
    return *this;
  }

  // 后置递增
  deque_iterator operator++(int) {
    deque_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  // 前置递减
  deque_iterator& operator--() {
    if (ptr_ == *m_iter_) {
      --m_iter_;
      ptr_ = *m_iter_ + block_size_;
    }
    --ptr_;
    return *this;
  }

  // 后置递减
  deque_iterator operator--(int) {
    deque_iterator tmp = *this;
    --(*this);
    return tmp;
  }

  // += 操作符
  deque_iterator& operator+=(difference_type n) {
    if (n != 0) {
      n += ptr_ - *m_iter_;
      if (n > 0) {
        m_iter_ += n / block_size_;
        ptr_ = *m_iter_ + n % block_size_;
      } else {  // (n < 0)
        difference_type z = block_size_ - 1 - n;
        m_iter_ -= z / block_size_;
        ptr_ = *m_iter_ + (block_size_ - 1 - z % block_size_);
      }
    }
    return *this;
  }

  // -= 操作符
  deque_iterator& operator-=(difference_type n) { return *this += -n; }

  // + 操作符
  deque_iterator operator+(difference_type n) const {
    deque_iterator t(*this);
    t += n;
    return t;
  }

  // - 操作符
  deque_iterator operator-(difference_type n) const {
    deque_iterator t(*this);
    t -= n;
    return t;
  }

  // 友元函数：n + iterator
  friend deque_iterator operator+(difference_type n, const deque_iterator& it) {
    return it + n;
  }

  // 友元函数：两个迭代器相减（支持混合类型）
  template <class Vp, class Pp, class Rp, class MP, class Dp, Dp BS2>
  friend difference_type operator-(const deque_iterator& x, 
                                   const deque_iterator<Vp, Pp, Rp, MP, Dp, BS2>& y) {
    if (x.ptr_ != y.ptr_)
      return (x.m_iter_ - y.m_iter_) * block_size_ + (x.ptr_ - *x.m_iter_) -
             (y.ptr_ - *y.m_iter_);
    return 0;
  }

  // [] 操作符
  reference operator[](difference_type n) const { return *(*this + n); }

  // 比较操作符
  friend bool operator==(const deque_iterator& x, const deque_iterator& y) {
    return x.ptr_ == y.ptr_;
  }

  friend bool operator!=(const deque_iterator& x, const deque_iterator& y) {
    return !(x == y);
  }

  friend bool operator<(const deque_iterator& x, const deque_iterator& y) {
    return x.m_iter_ < y.m_iter_ || (x.m_iter_ == y.m_iter_ && x.ptr_ < y.ptr_);
  }

  friend bool operator>(const deque_iterator& x, const deque_iterator& y) {
    return y < x;
  }

  friend bool operator<=(const deque_iterator& x, const deque_iterator& y) {
    return !(y < x);
  }

  friend bool operator>=(const deque_iterator& x, const deque_iterator& y) {
    return !(x < y);
  }
};

// 静态成员定义：在类外初始化
template <class ValueType, class Pointer, class Reference, class MapPointer,
          class DiffType, DiffType BS>
const DiffType deque_iterator<ValueType, Pointer, Reference, MapPointer,
                               DiffType, BS>::block_size_ =
    deque_block_size<ValueType, DiffType>::value;


// deque 类
template <class Tp, class Allocator = std::allocator<Tp>>
class deque {
 public:
  // types:
  using value_type = Tp;
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<allocator_type>;
  static_assert(is_same<typename allocator_type::value_type, value_type>::value,
                "Allocator::value_type must be same type as value_type");

  using size_type = typename alloc_traits::size_type;
  using difference_type = typename alloc_traits::difference_type;

  using pointer = typename alloc_traits::pointer;
  using const_pointer = typename alloc_traits::const_pointer;

  using pointer_allocator = typename alloc_traits::template rebind_alloc<pointer>;
  using const_pointer_allocator = typename alloc_traits::template rebind_alloc<const_pointer>;
  using map = split_buffer<pointer, pointer_allocator>;
  using map_alloc_traits = std::allocator_traits<pointer_allocator>;
  using map_pointer = typename map_alloc_traits::pointer;
  using map_const_pointer = typename std::allocator_traits<const_pointer_allocator>::const_pointer;
  using map_const_iterator = typename map::const_iterator;

  using reference = value_type&;
  using const_reference = const value_type&;

  using iterator = deque_iterator<value_type, pointer, reference, map_pointer, difference_type>;
  using const_iterator =
      deque_iterator<value_type, const_pointer, const_reference, map_const_pointer, difference_type>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  static const difference_type block_size;

 private:
  map map_;
  size_type start_;
  size_type size_;
  allocator_type alloc_;

  // 辅助函数
  allocator_type& alloc() noexcept { return alloc_; }
  const allocator_type& alloc() const noexcept { return alloc_; }

  size_type& size_ref() noexcept { return size_; }
  const size_type& size_ref() const noexcept { return size_; }

  size_type front_spare() const { return start_; }
  size_type back_spare() const {
    return map_.size() == 0 ? 0 : map_.size() * block_size - 1 - (start_ + size_);
  }

  size_type recommend_blocks(size_type n) {
    return n / block_size + (n % block_size != 0);
    // n / block_size：可以填满多少个块
    // n % block_size：填满后还剩的话说明还需要多一个块来装
  }

  void add_front_capacity();
  void add_front_capacity(size_type n);
  void add_back_capacity();
  void add_back_capacity(size_type n);

  // append 相关函数
  void append(size_type n);
  void append(size_type n, const value_type& v);
  template <class InputIterator>
  void append(InputIterator f, InputIterator l);
  template <class InputIterator>
  void append_with_size(InputIterator f, size_type n);
  template <class InputIterator, class Sentinel>
  void append_with_sentinel(InputIterator f, Sentinel l);

  // erase 相关函数
  void erase_to_end(iterator f);

  // 清理多余块的辅助函数
  bool maybe_remove_front_spare(bool keep_one = true);
  bool maybe_remove_back_spare(bool keep_one = true);

 public:
  // construct/copy/destroy:
  deque() noexcept : start_(0), size_(0) {}

  explicit deque(const allocator_type& a)
      : map_(pointer_allocator(a)), start_(0), size_(0), alloc_(a) {}

  explicit deque(size_type n);
  explicit deque(size_type n, const allocator_type& a);
  deque(size_type n, const value_type& v);
  deque(size_type n, const value_type& v, const allocator_type& a);

  template <class InputIterator,
            typename = typename enable_if<
                !is_integral<InputIterator>::value>::type>
  deque(InputIterator f, InputIterator l);
  template <class InputIterator,
            typename = typename enable_if<
                !is_integral<InputIterator>::value>::type>
  deque(InputIterator f, InputIterator l, const allocator_type& a);

  deque(const deque& c);
  deque(const deque& c, const allocator_type& a);
  deque(deque&& c) noexcept;
  deque(deque&& c, const allocator_type& a);
  deque(std::initializer_list<value_type> il);
  deque(std::initializer_list<value_type> il, const allocator_type& a);

  ~deque();

  deque& operator=(const deque& c);
  deque& operator=(deque&& c) noexcept;
  deque& operator=(std::initializer_list<value_type> il);

  template <class InputIterator,
            typename = typename enable_if<
                !is_integral<InputIterator>::value>::type>
  void assign(InputIterator f, InputIterator l);
  void assign(size_type n, const value_type& v);
  void assign(std::initializer_list<value_type> il);

  allocator_type get_allocator() const noexcept;

  // iterators:
  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  iterator end() noexcept;
  const_iterator end() const noexcept;

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

  const_iterator cbegin() const noexcept { return begin(); }
  const_iterator cend() const noexcept { return end(); }
  const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
  const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

  // capacity:
  size_type size() const noexcept { return size_; }
  size_type max_size() const noexcept {
    return std::min<size_type>(alloc_traits::max_size(alloc()),
                               std::numeric_limits<difference_type>::max());
  }
  void resize(size_type n);
  void resize(size_type n, const value_type& v);
  void shrink_to_fit() noexcept;
  bool empty() const noexcept { return size() == 0; }

  // element access:
  reference operator[](size_type i) noexcept;
  const_reference operator[](size_type i) const noexcept;
  reference at(size_type i);
  const_reference at(size_type i) const;
  reference front() noexcept;
  const_reference front() const noexcept;
  reference back() noexcept;
  const_reference back() const noexcept;

  // modifiers:
  void push_front(const value_type& v);
  void push_back(const value_type& v);
  template <class... Args>
  void emplace_front(Args&&... args);
  template <class... Args>
  void emplace_back(Args&&... args);
  void pop_front();
  void pop_back();
  void clear() noexcept;
  void swap(deque& c) noexcept;
};

// 静态成员定义
template <class Tp, class Allocator>
const typename deque<Tp, Allocator>::difference_type deque<Tp, Allocator>::block_size =
    deque_block_size<typename deque<Tp, Allocator>::value_type,
                     typename deque<Tp, Allocator>::difference_type>::value;

// 构造函数实现
template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(size_type n) : start_(0), size_(0) {
  if (n > 0) {
    append(n);
  }
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(size_type n, const allocator_type& a)
    : map_(pointer_allocator(a)), start_(0), size_(0), alloc_(a) {
  if (n > 0) {
    append(n);
  }
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(size_type n, const value_type& v) : start_(0), size_(0) {
  if (n > 0) {
    append(n, v);
  }
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(size_type n, const value_type& v, const allocator_type& a)
    : map_(pointer_allocator(a)), start_(0), size_(0), alloc_(a) {
  if (n > 0) {
    append(n, v);
  }
}

template <class Tp, class Allocator>
template <class InputIterator, typename>
deque<Tp, Allocator>::deque(InputIterator f, InputIterator l) : start_(0), size_(0) {
  append(f, l);
}

template <class Tp, class Allocator>
template <class InputIterator, typename>
deque<Tp, Allocator>::deque(InputIterator f, InputIterator l, const allocator_type& a)
    : map_(pointer_allocator(a)), start_(0), size_(0), alloc_(a) {
  append(f, l);
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(const deque& c)
    : map_(pointer_allocator(alloc_traits::select_on_container_copy_construction(c.alloc()))),
      start_(0),
      size_(0),
      alloc_(map_.alloc_) {
  append(c.begin(), c.end());
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(const deque& c, const allocator_type& a)
    : map_(pointer_allocator(a)), start_(0), size_(0), alloc_(a) {
  append(c.begin(), c.end());
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(deque&& c) noexcept
    : map_(mystl::move(c.map_)),
      start_(mystl::move(c.start_)),
      size_(mystl::move(c.size_)),
      alloc_(mystl::move(c.alloc_)) {
  c.start_ = 0;
  c.size_ = 0;
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(deque&& c, const allocator_type& a)
    : map_(mystl::move(c.map_), pointer_allocator(a)),
      start_(mystl::move(c.start_)),
      size_(mystl::move(c.size_)),
      alloc_(a) {
  if (a == c.alloc()) {
    c.start_ = 0;
    c.size_ = 0;
  } else {
    map_.clear();
    start_ = 0;
    size_ = 0;
    typedef std::move_iterator<iterator> Ip;
    assign(Ip(c.begin()), Ip(c.end()));
  }
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(std::initializer_list<value_type> il) : start_(0), size_(0) {
  append(il.begin(), il.end());
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::deque(std::initializer_list<value_type> il, const allocator_type& a)
    : map_(pointer_allocator(a)), start_(0), size_(0), alloc_(a) {
  append(il.begin(), il.end());
}

template <class Tp, class Allocator>
deque<Tp, Allocator>::~deque() {
  clear();
  typename map::iterator i = map_.begin();
  typename map::iterator e = map_.end();
  for (; i != e; ++i)
    alloc_traits::deallocate(alloc(), *i, block_size);
}

template <class Tp, class Allocator>
deque<Tp, Allocator>& deque<Tp, Allocator>::operator=(const deque& c) {
  if (this != std::addressof(c)) {
    if (alloc_traits::propagate_on_container_copy_assignment::value) {
      if (alloc() != c.alloc()) {
        clear();
        shrink_to_fit();
      }
      alloc_ = c.alloc_;
      map_.alloc_ = c.map_.alloc_;
    }
    assign(c.begin(), c.end());
  }
  return *this;
}

template <class Tp, class Allocator>
deque<Tp, Allocator>& deque<Tp, Allocator>::operator=(deque&& c) noexcept {
  clear();
  shrink_to_fit();
  map_ = mystl::move(c.map_);
  start_ = c.start_;
  size_ = c.size_;
  alloc_ = mystl::move(c.alloc_);
  c.start_ = c.size_ = 0;
  return *this;
}

template <class Tp, class Allocator>
deque<Tp, Allocator>& deque<Tp, Allocator>::operator=(std::initializer_list<value_type> il) {
  assign(il);
  return *this;
}

template <class Tp, class Allocator>
template <class InputIterator, typename>
void deque<Tp, Allocator>::assign(InputIterator f, InputIterator l) {
  iterator i = begin();
  iterator e = end();
  for (; f != l && i != e; ++f, (void)++i)
    *i = *f;
  if (f != l)
    append_with_sentinel(mystl::move(f), mystl::move(l));
  else
    erase_to_end(i);
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::assign(size_type n, const value_type& v) {
  if (n > size()) {
    std::fill_n(begin(), size(), v);
    n -= size();
    append(n, v);
  } else
    erase_to_end(std::fill_n(begin(), n, v));
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::assign(std::initializer_list<value_type> il) {
  assign(il.begin(), il.end());
}

template <class Tp, class Allocator>
inline typename deque<Tp, Allocator>::allocator_type
deque<Tp, Allocator>::get_allocator() const noexcept {
  return alloc();
}

// 迭代器实现
template <class Tp, class Allocator>
typename deque<Tp, Allocator>::iterator deque<Tp, Allocator>::begin() noexcept {
  map_pointer mp = map_.begin() + start_ / block_size;
  return iterator(mp, map_.empty() ? nullptr : *mp + start_ % block_size);
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::const_iterator deque<Tp, Allocator>::begin() const noexcept {
  map_const_pointer mp = static_cast<map_const_pointer>(map_.begin() + start_ / block_size);
  return const_iterator(mp, map_.empty() ? nullptr : *mp + start_ % block_size);
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::iterator deque<Tp, Allocator>::end() noexcept {
  size_type p = size() + start_;
  map_pointer mp = map_.begin() + p / block_size;
  return iterator(mp, map_.empty() ? nullptr : *mp + p % block_size);
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::const_iterator deque<Tp, Allocator>::end() const noexcept {
  size_type p = size() + start_;
  map_const_pointer mp = static_cast<map_const_pointer>(map_.begin() + p / block_size);
  return const_iterator(mp, map_.empty() ? nullptr : *mp + p % block_size);
}

// 元素访问实现
template <class Tp, class Allocator>
typename deque<Tp, Allocator>::reference deque<Tp, Allocator>::operator[](size_type i) noexcept {
  size_type p = start_ + i;
  return *(*(map_.begin() + p / block_size) + p % block_size);
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::const_reference
deque<Tp, Allocator>::operator[](size_type i) const noexcept {
  size_type p = start_ + i;
  return *(*(map_.begin() + p / block_size) + p % block_size);
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::reference deque<Tp, Allocator>::at(size_type i) {
  if (i >= size())
    throw std::out_of_range("deque");
  size_type p = start_ + i;
  return *(*(map_.begin() + p / block_size) + p % block_size);
  // p / block_size：确定块号
  // p % block_size：块内偏移值
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::const_reference deque<Tp, Allocator>::at(size_type i) const {
  if (i >= size())
    throw std::out_of_range("deque");
  size_type p = start_ + i;
  return *(*(map_.begin() + p / block_size) + p % block_size);
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::reference deque<Tp, Allocator>::front() noexcept {
  return *(*(map_.begin() + start_ / block_size) + start_ % block_size);
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::const_reference deque<Tp, Allocator>::front() const noexcept {
  return *(*(map_.begin() + start_ / block_size) + start_ % block_size);
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::reference deque<Tp, Allocator>::back() noexcept {
  size_type p = size() + start_ - 1;
  return *(*(map_.begin() + p / block_size) + p % block_size);
}

template <class Tp, class Allocator>
typename deque<Tp, Allocator>::const_reference deque<Tp, Allocator>::back() const noexcept {
  size_type p = size() + start_ - 1;
  return *(*(map_.begin() + p / block_size) + p % block_size);
}

// 修改器实现
template <class Tp, class Allocator>
void deque<Tp, Allocator>::push_back(const value_type& v) {
  allocator_type& a = alloc();
  if (back_spare() == 0)
    add_back_capacity();
  alloc_traits::construct(a, std::addressof(*end()), v);
  ++size_;
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::push_front(const value_type& v) {
  allocator_type& a = alloc();
  if (front_spare() == 0)
    add_front_capacity();
  alloc_traits::construct(a, std::addressof(*--begin()), v);
  --start_;
  ++size_;
}

template <class Tp, class Allocator>
template <class... Args>
void deque<Tp, Allocator>::emplace_back(Args&&... args) {
  allocator_type& a = alloc();
  if (back_spare() == 0)
    add_back_capacity();
  alloc_traits::construct(a, std::addressof(*end()), mystl::forward<Args>(args)...);
  ++size_;
}

template <class Tp, class Allocator>
template <class... Args>
void deque<Tp, Allocator>::emplace_front(Args&&... args) {
  allocator_type& a = alloc();
  if (front_spare() == 0)
    add_front_capacity();
  alloc_traits::construct(a, std::addressof(*--begin()), mystl::forward<Args>(args)...);
  --start_;
  ++size_;
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::pop_back() {
  allocator_type& a = alloc();
  size_type p = size() + start_ - 1;
  alloc_traits::destroy(a, std::addressof(*(*(map_.begin() + p / block_size) + p % block_size)));
  --size_;
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::pop_front() {
  allocator_type& a = alloc();
  alloc_traits::destroy(a, std::addressof(*(*(map_.begin() + start_ / block_size) + start_ % block_size)));
  --size_;
  ++start_;
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::clear() noexcept {
  allocator_type& a = alloc();
  for (iterator i = begin(), e = end(); i != e; ++i)
    alloc_traits::destroy(a, std::addressof(*i));
  size_ = 0;
  while (map_.size() > 2) {
    alloc_traits::deallocate(a, map_.front(), block_size);
    map_.pop_front();
  }
  switch (map_.size()) {
  case 1:
    start_ = block_size / 2;
    break;
  case 2:
    start_ = block_size;
    break;
  }
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::shrink_to_fit() noexcept {
  allocator_type& a = alloc();
  if (empty()) {
    while (map_.size() > 0) {
      alloc_traits::deallocate(a, map_.back(), block_size);
      map_.pop_back();
    }
    start_ = 0;
  } else {
    maybe_remove_front_spare(false);
    maybe_remove_back_spare(false);
  }
  map_.shrink_to_fit();
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::resize(size_type n) {
  if (n > size())
    append(n - size());
  else if (n < size())
    erase_to_end(begin() + n);
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::resize(size_type n, const value_type& v) {
  if (n > size())
    append(n - size(), v);
  else if (n < size())
    erase_to_end(begin() + n);
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::swap(deque& c) noexcept {
  map_.swap(c.map_);
  std::swap(start_, c.start_);
  std::swap(size_, c.size_);
  std::swap(alloc_, c.alloc_);
}

// 私有辅助函数实现
template <class Tp, class Allocator>
void deque<Tp, Allocator>::add_front_capacity() {
  add_front_capacity(1);
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::add_front_capacity(size_type n) {
  allocator_type& a = alloc();
  size_type nb = recommend_blocks(n + map_.empty());
  // 计算后端未使用的块数：
  size_type back_capacity = back_spare() / block_size;
  back_capacity = std::min(back_capacity, nb);  // 不要拿取超过所需的块
  nb -= back_capacity;  // 需要新分配的块数
  // 如果 nb == 0，说明容量已足够。
  if (nb == 0) {
    start_ += block_size * back_capacity;
    for (; back_capacity > 0; --back_capacity) {
      pointer pt = map_.back();
      map_.pop_back();
      map_.emplace_front(pt);
    }
  }
  // 否则，如果 nb <= map_.capacity() - map_.size()，说明需要分配 nb 个缓冲区
  else if (nb <= map_.capacity() - map_.size()) {
    // 我们可以将新的缓冲区放入 map，但在所有缓冲区分配完成前不要移动它们。
    // 如果抛出异常，我们不需要修复任何东西（任何已添加的缓冲区都是不可检测的）
    for (; nb > 0; --nb, start_ += block_size - (map_.size() == 1)) {
      if (map_.front_spare() == 0)
        break;
      map_.emplace_front(alloc_traits::allocate(a, block_size));
    }
    for (; nb > 0; --nb, ++back_capacity)
      map_.emplace_back(alloc_traits::allocate(a, block_size));
    // 分配完成，重新排序容量
    start_ += back_capacity * block_size;
    for (; back_capacity > 0; --back_capacity) {
      pointer pt = map_.back();
      map_.pop_back();
      map_.emplace_front(pt);
    }
  }
  // 否则，需要分配 nb 个缓冲区，并且需要重新分配 map_。
  else {
    size_type ds = (nb + back_capacity) * block_size - map_.empty();
    split_buffer<pointer, pointer_allocator&> buf(
        std::max<size_type>(2 * map_.capacity(), nb + map_.size()), 0, map_.alloc_);
    for (; nb > 0; --nb) {
      buf.emplace_back(alloc_traits::allocate(a, block_size));
    }
    for (; back_capacity > 0; --back_capacity) {
      buf.emplace_back(map_.back());
      map_.pop_back();
    }
    for (map_pointer i = map_.begin(); i != map_.end(); ++i)
      buf.emplace_back(*i);
    std::swap(map_.first_, buf.first_);
    std::swap(map_.begin_, buf.begin_);
    std::swap(map_.end_, buf.end_);
    std::swap(map_.cap_, buf.cap_);
    start_ += ds;
  }
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::add_back_capacity() {
  add_back_capacity(1);
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::add_back_capacity(size_type n) {
  allocator_type& a = alloc();
  size_type nb = recommend_blocks(n + map_.empty());
  // 计算前端未使用的块数：
  size_type front_capacity = front_spare() / block_size;
  front_capacity = std::min(front_capacity, nb);  // 不要拿取超过所需的块
  nb -= front_capacity;  // 需要新分配的块数
  // 如果 nb == 0，说明容量已足够。
  if (nb == 0) {
    start_ -= block_size * front_capacity;
    for (; front_capacity > 0; --front_capacity) {
      pointer pt = map_.front();
      map_.pop_front();
      map_.emplace_back(pt);
    }
  }
  // 否则，如果 nb <= map_.capacity() - map_.size()，说明需要分配 nb 个缓冲区
  else if (nb <= map_.capacity() - map_.size()) {
    // 我们可以将新的缓冲区放入 map，但在所有缓冲区分配完成前不要移动它们。
    // 如果抛出异常，我们不需要修复任何东西（任何已添加的缓冲区都是不可检测的）
    for (; nb > 0; --nb) {
      if (map_.back_spare() == 0)
        break;
      map_.emplace_back(alloc_traits::allocate(a, block_size));
    }
    for (; nb > 0; --nb, ++front_capacity, start_ += block_size - (map_.size() == 1)) {
      map_.emplace_front(alloc_traits::allocate(a, block_size));
    }
    // 分配完成，重新排序容量
    start_ -= block_size * front_capacity;
    for (; front_capacity > 0; --front_capacity) {
      pointer pt = map_.front();
      map_.pop_front();
      map_.emplace_back(pt);
    }
  }
  // 否则，需要分配 nb 个缓冲区，并且需要重新分配 map_。
  else {
    size_type ds = front_capacity * block_size;
    split_buffer<pointer, pointer_allocator&> buf(
        std::max<size_type>(2 * map_.capacity(), nb + map_.size()),
        map_.size() - front_capacity, map_.alloc_);
    for (; nb > 0; --nb) {
      buf.emplace_back(alloc_traits::allocate(a, block_size));
    }
    for (; front_capacity > 0; --front_capacity) {
      buf.emplace_back(map_.front());
      map_.pop_front();
    }
    for (map_pointer i = map_.end(); i != map_.begin();)
      buf.emplace_front(*--i);
    std::swap(map_.first_, buf.first_);
    std::swap(map_.begin_, buf.begin_);
    std::swap(map_.end_, buf.end_);
    std::swap(map_.cap_, buf.cap_);
    start_ -= ds;
  }
}

// append 函数实现
template <class Tp, class Allocator>
void deque<Tp, Allocator>::append(size_type n) {
  allocator_type& a = alloc();
  size_type back_capacity = back_spare();
  if (n > back_capacity)
    add_back_capacity(n - back_capacity);
  // n <= back_capacity
  for (size_type i = 0; i < n; ++i) {
    alloc_traits::construct(a, std::addressof(*end()));
    ++size_;
  }
}

template <class Tp, class Allocator>
void deque<Tp, Allocator>::append(size_type n, const value_type& v) {
  allocator_type& a = alloc();
  size_type back_capacity = back_spare();
  if (n > back_capacity)
    add_back_capacity(n - back_capacity);
  // n <= back_capacity
  for (size_type i = 0; i < n; ++i) {
    alloc_traits::construct(a, std::addressof(*end()), v);
    ++size_;
  }
}

template <class Tp, class Allocator>
template <class InputIterator>
void deque<Tp, Allocator>::append(InputIterator f, InputIterator l) {
  append_with_sentinel(f, l);
}

template <class Tp, class Allocator>
template <class InputIterator>
void deque<Tp, Allocator>::append_with_size(InputIterator f, size_type n) {
  allocator_type& a = alloc();
  size_type back_capacity = back_spare();
  if (n > back_capacity)
    add_back_capacity(n - back_capacity);
  // n <= back_capacity
  for (size_type i = 0; i < n; ++i, ++f) {
    alloc_traits::construct(a, std::addressof(*end()), *f);
    ++size_;
  }
}

template <class Tp, class Allocator>
template <class InputIterator, class Sentinel>
void deque<Tp, Allocator>::append_with_sentinel(InputIterator f, Sentinel l) {
  for (; f != l; ++f)
    emplace_back(*f);
}

// erase_to_end 函数实现
template <class Tp, class Allocator>
void deque<Tp, Allocator>::erase_to_end(iterator f) {
  iterator e = end();
  difference_type n = e - f;
  if (n > 0) {
    allocator_type& a = alloc();
    for (iterator p = f; p != e; ++p)
      alloc_traits::destroy(a, std::addressof(*p));
    size_ -= n;
    while (maybe_remove_back_spare()) {
    }
  }
}

// maybe_remove_front_spare 和 maybe_remove_back_spare 实现
template <class Tp, class Allocator>
bool deque<Tp, Allocator>::maybe_remove_front_spare(bool keep_one) {
  size_type front_spare_blocks = front_spare() / block_size;
  if (front_spare_blocks >= 2 || (!keep_one && front_spare_blocks)) {
    alloc_traits::deallocate(alloc(), map_.front(), block_size);
    map_.pop_front();
    start_ -= block_size;
    return true;
  }
  return false;
}

template <class Tp, class Allocator>
bool deque<Tp, Allocator>::maybe_remove_back_spare(bool keep_one) {
  size_type back_spare_blocks = back_spare() / block_size;
  if (back_spare_blocks >= 2 || (!keep_one && back_spare_blocks)) {
    alloc_traits::deallocate(alloc(), map_.back(), block_size);
    map_.pop_back();
    return true;
  }
  return false;
}

}  // namespace mystl

// 为 std::iterator_traits 提供特化，使 std::reverse_iterator 能够识别我们的迭代器
// 必须在 mystl 命名空间定义之后
namespace std {
template <class ValueType, class Pointer, class Reference, class MapPointer,
          class DiffType, DiffType BS>
struct iterator_traits<::mystl::deque_iterator<ValueType, Pointer, Reference, MapPointer, DiffType, BS>> {
  using iterator_category = ::mystl::random_access_iterator_tag;
  using value_type = ValueType;
  using difference_type = DiffType;
  using pointer = Pointer;
  using reference = Reference;
};
}  // namespace std

#endif  // TINYSTL_DEQUE_H
