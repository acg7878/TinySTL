#ifndef TINYSTL_ITERATOR_TRAITS_H
#define TINYSTL_ITERATOR_TRAITS_H

#include <__config>
#include <cstddef>

namespace mystl {
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};


template <class Iterator>
struct iterator_traits {
  using iterator_category = typename Iterator::iterator_category;  // 迭代器类别
  using value_type =
      typename Iterator::value_type;  // 迭代器解引用（*it）得到的元素的类型。
  using difference_type = typename Iterator::difference_type;
  using pointer = typename Iterator::pointer;
  using reference = typename Iterator::reference;
};

template <class T>
struct iterator_traits<T*> {
  using iterator_category = random_access_iterator_tag;
  using value_type = T;
  using difference_type = ptrdiff_t;
  using pointer = T*;
  using reference = T&;
};

template <class T>
struct iterator_traits<const T*> {
  using iterator_category = random_access_iterator_tag;
  using value_type = T;  // 不是const T!! 前面写错了
  using difference_type = ptrdiff_t;
  using pointer = const T*;
  using reference = const T&;
};


// 获取迭代器的类型
template <class Iterator>
inline typename iterator_traits<Iterator>::iterator_category iterator_category(
    const Iterator&) {
  // 为什么有const Iterator&：
  // LINK blog/iterator/模板类型推导.md
  using category = typename iterator_traits<Iterator>::iterator_category;
  return category();
  // 返回一个 category 类型的临时对象
  // 方便标签分发：不返回类型而是返回一个对象
}

// LINK blog/iterator/function/distance_type.md
template <class Iterator>
inline typename iterator_traits<Iterator>::difference_type* distance_type(
    const Iterator&) {
  return static_cast<typename iterator_traits<Iterator>::difference_type*>(
      nullptr);
}

template <class Iterator>
inline typename iterator_traits<Iterator>::value_type* value_type(
    const Iterator&) {
  return static_cast<typename iterator_traits<Iterator>::value_type*>(nullptr);
}
}  // namespace mystl

#endif