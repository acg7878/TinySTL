#ifndef TINYSTL_ITERATOR_DISTANCE_H
#define TINYSTL_ITERATOR_DISTANCE_H

#include "iterator_traits.h"

namespace mystl {

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

}  // namespace mystl

#endif