#ifndef TINYSTL___MEMORY_UNINITIALIZED_ALGORITHMS_H
#define TINYSTL___MEMORY_UNINITIALIZED_ALGORITHMS_H

#include <mystl/__memory/construct.h>
#include <mystl/__type_traits/integral_constant.h>
#include <mystl/__type_traits/is_trivially_copyable.h>
#include <mystl/__utility/move.h>
#include <mystl/algorithm.h>
#include <mystl/iterator.h>
#include <mystl/type_traits.h>
#include <algorithm>

namespace mystl {
// result迭代器要符合 ForwardIterator 要求
template <class InputIter, class ForwardIter>
ForwardIter _uninitialized_copy_aux(InputIter first, InputIter last,
                                    ForwardIter result, mystl::true_type) {
  // POD 类型，直接拷贝
  return mystl::copy(first, last, result);
}

template <class InputIter, class ForwardIter>
ForwardIter _uninitialized_copy_aux(InputIter first, InputIter last,
                                    ForwardIter result, mystl::false_type) {
  ForwardIter cur = result;
  try {
    for (; first != last; first++, cur++) {
      // construct ：在一块已分配但未构造的内存上，调用对象的构造函数。
      // 因为没有对象所以不能直接赋值
      mystl::construct(&*cur, *first);
      // *first 是迭代器指向的对象
      // &*first 是该对象的地址
    }
    return cur;
  } catch (...) {
    mystl::destroy(result, cur);
    throw;
  }
}

template <class InputIter, class ForwardIter>
ForwardIter uninitialized_copy(InputIter first, InputIter last,
                               ForwardIter result) {
  using value_type = typename mystl::iterator_traits<ForwardIter>::value_type;
  return _uninitialized_copy_aux(first, last, result,
                                 is_trivially_copyable<value_type>{});
}

template <class ForwardIterator, class Size, class T>
ForwardIterator uninitialized_fill_n_impl(ForwardIterator first, Size n,
                                          const T& value, mystl::true_type) {
  return std::fill_n(first, n, value);  // TODO 这里使用了std
}

template <class ForwardIterator, class Size, class T>
ForwardIterator uninitialized_fill_n_impl(ForwardIterator first, Size n,
                                          const T& value, mystl::false_type) {
  ForwardIterator cur = first;
  try {
    for (; n > 0; n--, cur++) {
      // addressof：安全地获取对象的真实地址
      mystl::construct(std::addressof(*cur), value);
    }
  } catch (...) {
    mystl::destroy(first, cur);
    throw;
  }
  return cur;
}

template <class ForwardIterator, class Size, class T>
ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n,
                                     const T& value) {
  using ValueType = typename iterator_traits<ForwardIterator>::value_type;
  return uninitialized_fill_n_impl(first, n, value,
                                   mystl::is_trivially_copyable<ValueType>{});
}

template <typename InputIterator, typename ForwardIterator>
ForwardIterator uninitialized_move_aux(InputIterator first, InputIterator last,
                                       ForwardIterator result,
                                       mystl::true_type) {
  return mystl::copy(first, last, result);
}

template <typename InputIterator, typename ForwardIterator>
ForwardIterator uninitialized_move_aux(InputIterator first, InputIterator last,
                                       ForwardIterator result,
                                       mystl::false_type) {
  ForwardIterator cur = result;
  try {
    for (; first != last; first++, cur++) {
      mystl::construct(&*cur, mystl::move(*first));
    }
    return cur;
  } catch (...) {
    mystl::destroy(result, cur);
    throw;
  }
}

template <typename InputIterator, typename ForwardIterator>
ForwardIterator uninitialized_move(InputIterator first, InputIterator last,
                                   ForwardIterator result) {
  using ValueType = typename iterator_traits<ForwardIterator>::value_type;
  return uninitialized_move_aux(first, last, result,
                                mystl::is_trivially_copyable<ValueType>{});
}

}  // namespace mystl

#endif  // MYTINYSTL_UNINITIALIZED_H_