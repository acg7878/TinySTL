#ifndef TINYSTL_ITERATOR_H
#define TINYSTL_ITERATOR_H

#include <cstddef>
namespace mystl {
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

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

// ======迭代器辅助函数⬇️=======

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

// =======distance具体实现=======

template <typename RandomAccessIterator>
typename iterator_traits<RandomAccessIterator>::difference_type
distance_dispatch(RandomAccessIterator first, RandomAccessIterator last,
                  random_access_iterator_tag) {
  return last - first;
}

template <typename InputIterator>
typename iterator_traits<InputIterator>::difference_type distance_dispatch(
    InputIterator first, InputIterator last, input_iterator_tag) {
  typename iterator_traits<InputIterator>::difference_type n = 0;
  while (first != last) {
    n++;
    first++;
  }
  return n;
}

template <typename ForwardIterator>
typename iterator_traits<ForwardIterator>::difference_type distance_dispatch(
    ForwardIterator first, ForwardIterator last, forward_iterator_tag) {
  typename iterator_traits<ForwardIterator>::difference_type n = 0;
  while (first != last) {
    first++;
    n++;
  }
  return n;
}

template <typename BidirectionalIterator>
typename iterator_traits<BidirectionalIterator>::difference_type
distance_dispatch(BidirectionalIterator first, BidirectionalIterator last,
                  bidirectional_iterator_tag) {
  typename iterator_traits<BidirectionalIterator>::difference_type n = 0;
  while (first != last) {
    n++;
    first++;
  }
  return n;
}

// 统一调用
template <typename Iterator>
typename iterator_traits<Iterator>::difference_type distance(Iterator first,
                                                             Iterator last) {
  return distance_dispatch(first, last, iterator_category(first));
}

// ====== advance 实现 ======
template <class InputIterator, class Distance>
void advance_dispatch(InputIterator& it, Distance n, input_iterator_tag) {
  while (n--) {
    it++;
  }
}

template <class BidirectionalIterator, class Distance>
void advance_dispatch(BidirectionalIterator& it, Distance n,
                      bidirectional_iterator_tag) {
  if (n >= 0) {
    while (n--) {
      it++;
    }
  } else {
    while (n++) {
      it--;
    }
  }
}

template <class RandomAccessIterator, class Distance>
void advance_dispatch(RandomAccessIterator& it, Distance n,
                      random_access_iterator_tag) {
  it += n;
}

template <class Iterator, class Distance>
void advance(Iterator& it, Distance n) {
  // using category = typename iterator_traits<Iterator>::iterator_category;
  advance_dispatch(it, n, iterator_category(it));
}

}  // namespace mystl

#endif