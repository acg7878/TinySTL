#ifndef TINYSTL_VECTOR_H
#define TINYSTL_VECTOR_H

#include <mystl/memory.h>
#include <mystl/utility.h>
#include <mystl/iterator.h>
#include <mystl/memory.h>
#include <mystl/type_traits.h>
#include <cstddef>
#include <initializer_list>
namespace mystl {

template <class T, class Alloc = mystl::allocator<T>>
class vector {
 public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = size_t;           // 容器大小的类型
  using difference_type = ptrdiff_t;  // 距离的类型

  using iterator = value_type*;
  using const_iterator = const value_type*;

 public:
  vector()
      : _start(nullptr),
        _finish(nullptr),
        _end_of_storage(nullptr),
        allocator() {}

  vector(size_type n, const value_type& value) {
    _start = allocator.allocate(n);
    _finish = _start;
    _end_of_storage = _start + n;
    try {
      // 在构造过程中如果抛出异常，会正确销毁已经构造的元素并释放内存
      mystl::uninitialized_fill_n(_start, n, value);
      _finish = _start + n;
    } catch (...) {
      allocator.deallocate(_start, n);
      _start = _finish = _end_of_storage = nullptr;
      throw;  // 重新抛出异常
    }
  }

  // typename = xxx : 这个写法是省略了参数名
  // 防止整型参数匹配到这个函数。 导致没匹配上vector(size_type n, const value_type& value)
  template <class InputIterator,
            typename = typename mystl::enable_if<
                !mystl::is_integral<InputIterator>::value>::type>
  vector(InputIterator first, InputIterator last) {
    size_type n = mystl::distance(first, last);
    _start = allocator.allocate(n);
    _finish = mystl::uninitialized_copy(first, last, _start);
    _end_of_storage = _start + n;
  }

  // 拷贝构造
  vector(const vector& other) {
    size_type n = other.size();
    _start = allocator.allocate(n);
    _finish = mystl::uninitialized_copy(other._start, other._finish, _start);
    _end_of_storage = _start + n;
  }

  // 移动构造
  vector(vector&& other) noexcept
      : _start(other._start),
        _finish(other._finish),
        _end_of_storage(other._end_of_storage),
        allocator(std::move(other.allocator)) {
    other._start = nullptr;
    other._finish = nullptr;
    other._end_of_storage = nullptr;
  }

  // 初始化列表
  vector(std::initializer_list<T> init) : vector(init.begin(), init.end()) {}

  // const：保证函数不会修改对象状态
  size_type size() const { return _finish - _start; }
  size_type capacity() const { return _end_of_storage - _start; }
  bool empty() const { return _start == _finish; }

  iterator begin() { return _start; }

  const_iterator begin() const { return _start; }

  iterator end() { return _finish; }

  const_iterator end() const { return _finish; }

  reference operator[](size_type n) { return _start[n]; }

  const_reference operator[](size_type n) const { return _start[n]; }

  ~vector() {
    if (_start) {
      allocator.destroy(_start, _finish);
      allocator.deallocate(_start, _end_of_storage - _start);
    }
    _start = _finish = _end_of_storage = nullptr;
  }

  void push_back(const value_type& value) {
    if (_finish == _end_of_storage) {
      size_type old_size = size();
      size_type old_capacity = capacity();
      size_type new_capacity = old_capacity == 0 ? 1 : old_capacity * 2;
      pointer new_start = allocator.allocate(new_capacity);
      // 优先使用移动，如果不行则拷贝
      mystl::uninitialized_move(_start, _finish, new_start);
      // 销毁旧对象并释放旧内存
      allocator.destroy(_start, _finish);
      allocator.deallocate(_start, old_capacity);
      _finish = new_start + old_size;
      _start = new_start;
      _end_of_storage = _start + new_capacity;
    }
    allocator.construct(_finish, value);
    _finish++;
  }

  void pop_back() {
    if (_finish != _start) {
      _finish--;
      allocator.destroy(_finish);
    }
  }

  void shrink_to_fit() {
    if (size() < capacity()) {
      size_type old_size = size();
      pointer new_start = allocator.allocate(old_size);
      mystl::uninitialized_move(_start, _finish, new_start);
      allocator.destroy(_start, _finish);
      allocator.deallocate(_start, capacity());
      _start = new_start;
      _finish = _start + old_size;
      _end_of_storage = _start + old_size;
    }
  }

  //利用 RAII，会自动构析销毁原数据
  // 使用 copy-and-swap 模式
  vector& operator=(const vector& other) {
    if (this != &other) {
      vector temp(other);
      this->swap(temp);
    }
    return *this;
  }

  vector& operator=(vector&& other) noexcept {
    swap(other);
    return *this;
  }

  void swap(vector& other) noexcept {
    mystl::swap(_start, other._start);
    mystl::swap(_finish, other._finish);
    mystl::swap(_end_of_storage, other._end_of_storage);
    mystl::swap(allocator, other.allocator);
  }

 private:
  pointer _start = nullptr;   // 指向数据区起始位置
  pointer _finish = nullptr;  // 指向有效元素的末尾（即下一个待插入位置）
  pointer _end_of_storage = nullptr;  // 指向整个内存缓冲区的末尾
  Alloc allocator;
};
}  // namespace mystl

#endif