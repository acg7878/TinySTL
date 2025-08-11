#ifndef MYTINYSTL_ALGOBASE_H_
#define MYTINYSTL_ALGOBASE_H_

#include <cstddef>
#include <cstring>  // memmove
#include "iterator.h"
#include "type_traits.h"

namespace mystl {

template <class InputIterator, class OutputIterator>
OutputIterator _copy_aux(InputIterator first, InputIterator last,
                         OutputIterator result, input_iterator_tag) {
  for (; first != last; ++first, ++result) {
    *result = *first;
  }
  return result;
}

template <class RandomAccessIterator, class OutputIterator>
OutputIterator _copy_aux(RandomAccessIterator first, RandomAccessIterator last,
                         OutputIterator result, random_access_iterator_tag) {
  using difference_type =
      typename iterator_traits<RandomAccessIterator>::difference_type;
  for (difference_type n = last - first; n > 0; --n, ++first, ++result) {
    *result = *first;
  }
  return result;
}

template <class T>
T* _copy_dispatch(const T* first, const T* last, T* result,
                  mystl::true_type /*is_pod*/) {
  const size_t n = static_cast<size_t>(last - first);
  if (n > 0) {
    memmove(result, first, n * sizeof(T));
  }
  return result + n;
}

// 非 POD 类型版本，逐个赋值
template <class T>
T* _copy_dispatch(const T* first, const T* last, T* result,
                  mystl::false_type /*is_pod*/) {
  //裸指针 T*，天然就是随机访问迭代器
  return _copy_aux(first, last, result, mystl::random_access_iterator_tag{});
}

// 裸指针重载，利用 is_pod 做类型特征分发
template <class T>
T* copy(const T* first, const T* last, T* result) {
  return _copy_dispatch(first, last, result, mystl::is_pod<T>{});
}

}  // namespace mystl

#endif  // MYTINYSTL_ALGOBASE_H_
