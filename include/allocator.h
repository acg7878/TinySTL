#ifndef MYTINYSTL_ALLOCATOR_H_
#define MYTINYSTL_ALLOCATOR_H_

#include <cstddef>
#include <new>
#include "construct.h"

namespace mystl {

template <typename T>
class allocator {
 public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <typename U>
  struct rebind {
    using other = allocator<U>;
  };

 public:
  // 声明
  static pointer allocate(size_type n);
  // static void deallocate(pointer p); c++11 不再提供
  static void deallocate(pointer p, size_type n);

  template <typename... Args>
  static void construct(pointer p, Args&&... args);
  static void destroy(pointer p);
  static void destroy(pointer first, pointer last);
};

template <typename T>
typename allocator<T>::pointer allocator<T>::allocate(size_type n) {
  if (n == 0)
    return nullptr;
  return static_cast<pointer>(::operator new(n * sizeof(T)));
}

// DELETE: cpp11必须携带 size_type /*n*/
// template <typename T>
// void allocator<T>::deallocate(pointer p) {
//   if (p == nullptr)
//     return;
//   ::operator delete(p);
// }

template <typename T>
void allocator<T>::deallocate(pointer p, size_type /*n*/) {
  if (p == nullptr)
    return;
  ::operator delete(p);
}

template <typename T>
template <typename... Args>
void allocator<T>::construct(pointer p, Args&&... args) {
  mystl::construct(p, std::forward<Args>(args)...);
}

template <typename T>
void allocator<T>::destroy(pointer p) {
  mystl::destroy(p);
}

template <typename T>
void allocator<T>::destroy(pointer first, pointer last) {
  mystl::destroy(first, last);
}

}  // namespace mystl

#endif  // MYTINYSTL_ALLOCATOR_H_