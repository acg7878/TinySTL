#ifndef TINYSTL___TYPE_TRAITS_IS_POD_H
#define TINYSTL___TYPE_TRAITS_IS_POD_H

#include <mystl/__type_traits/integral_constant.h>

namespace mystl {
// is_POD 精细度不够，C++20已经弃用，c++11不推荐使用
template <typename T>
struct is_pod : public integral_constant<bool, __is_pod(T)> {};
}  // namespace mystl

#endif