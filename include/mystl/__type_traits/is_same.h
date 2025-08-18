#ifndef TINYSTL___TYPE_TRAITS_IS_POD_H
#define TINYSTL___TYPE_TRAITS_IS_POD_H

#include <mystl/__type_traits/integral_constant.h>

namespace mystl {
// 默认规则：对于任意两种类型 T 和 U，is_same<T, U> 的结果默认为 false
template <typename T, typename U>
struct is_same : public false_type {};

template <typename T>
struct is_same<T, T> : public true_type {};
}  // namespace mystl

#endif