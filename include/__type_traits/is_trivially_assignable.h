#ifndef TINYSTL___TYPE_TRAITS_IS_TRIVIALLY_ASSIGNABLE_H
#define TINYSTL___TYPE_TRAITS_IS_TRIVIALLY_ASSIGNABLE_H

#include "integral_constant.h"

namespace mystl {
// 是否具有平凡拷贝赋值
template <typename T>
struct is_trivially_copy_assignable
    : public integral_constant<bool, __is_trivially_assignable(T&, const T&)> {
};
}  // namespace mystl

#endif