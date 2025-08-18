#ifndef MY_TYPE_TRAITS_IS_TRIVIALLY_CONSTRUCTIBLE_H
#define MY_TYPE_TRAITS_IS_TRIVIALLY_CONSTRUCTIBLE_H

#include <mystl/__type_traits/integral_constant.h>

namespace mystl {
// 是否具备平凡拷贝构造
template <typename T>
struct is_trivially_copy_constructible
    : public integral_constant<bool,
                               __is_trivially_constructible(T, const T&)> {};
}  // namespace mystl

#endif