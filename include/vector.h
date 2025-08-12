#ifndef MY_VECTOR_H_
#define MY_VECTOR_H_

#include <cstddef>
#include "memory.h"
#include "iterator.h"

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

 public:
  vector() : _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {}
//   vector(size_type n, const value_type& value) {
//     _start = Alloc::allocate(n); // vsc缺乏语法提示不能跳转🤔
//     _finish = _start;
//     _end_of_storage = _start + n;
//     for (size_type i = 0; i < n; ++i) {
//       Alloc::construct(_finish, value);
//       ++_finish;
//     }
//   }
  
//   template<class InputIterator>
//   vector(InputIterator first, InputIterator last) {
//     size_type n = mystl::distance(first, last);
//     _start = Alloc::allocate(n);
//     _finish = mystl::uninitialized_copy(first, last, _start);
//     _end_of_storage = _start+n;
//   }

 private:
  pointer _start = nullptr;   // 指向数据区起始位置
  pointer _finish = nullptr;  // 指向有效元素的末尾（即下一个待插入位置）
  pointer _end_of_storage = nullptr;  // 指向整个内存缓冲区的末尾
};
}  // namespace mystl

#endif