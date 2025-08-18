#ifndef TINYSTL_ITERATOR_H
#define TINYSTL_ITERATOR_H

#include <cstddef>
namespace mystl {

// Distance默认值是ptrdiff_t，也要看传入什么参数；其他的类似
template <class Category, class T, class Distance = ptrdiff_t,
          class Pointer = T*, class Reference = T&>
struct iterator {
  using iterator_category = Category;
  using value_type = T;
  using difference_type = Distance;
  using pointer = Pointer;
  using reference = Reference;
};

}  // namespace mystl

#endif