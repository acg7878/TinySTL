#ifndef TINYSTL___TYPE_TRAITS_IS_TRIVIALLY_COPYABLE_H_
#define TINYSTL___TYPE_TRAITS_IS_TRIVIALLY_COPYABLE_H_

#include <mystl/__type_traits/integral_constant.h>

namespace mystl {
// 是否具有平凡拷贝
template <typename T>
struct is_trivially_copyable
    : public integral_constant<bool, __is_trivially_copyable(T)> {};
}  // namespace mystl

#endif