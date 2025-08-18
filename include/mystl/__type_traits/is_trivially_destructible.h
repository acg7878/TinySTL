#ifndef TINYSTL___TYPE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE_H
#define TINYSTL___TYPE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE_H

#include <mystl/__type_traits/integral_constant.h>

namespace mystl {
// 判断是否具有平凡析构函数
template <typename T>
struct is_trivially_destructible
    : public integral_constant<bool, __is_trivially_destructible(T)> {};
}  // namespace mystl

#endif