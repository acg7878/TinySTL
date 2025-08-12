#ifndef MY_UNINITIALIZED_H_
#define MY_UNINITIALIZED_H_

#include "algobase.h"
#include "algorithm.h"
#include "construct.h"
#include "iterator.h"
#include "type_traits.h"

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
      mystl::construct(
          &*cur, *first);  // TODO：不知道有没有bug，到底是&*first还是*first
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
                                 is_trivially_copy_constructible<value_type>{});
}

}  // namespace mystl

#endif  // MYTINYSTL_UNINITIALIZED_H_