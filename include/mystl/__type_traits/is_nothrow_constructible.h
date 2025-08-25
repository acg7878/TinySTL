#ifndef TINYSTL___TYPE_TRAITS_IS_NOTHROW_CONSTRUCTIBLE_H
#define TINYSTL___TYPE_TRAITS_IS_NOTHROW_CONSTRUCTIBLE_H

#include <mystl/__type_traits/integral_constant.h>
#include <mystl/__type_traits/is_constructible.h>
#include <mystl/__utility/declval.h>

namespace mystl {
template <class T, class... Args>
// 判断构造过程是否承诺不抛异常
struct is_nothrow_constructible
    : mystl::integral_constant<bool,
                               mystl::is_constructible<T, Args...>::value &&
                                   noexcept(T(mystl::declval<Args>()...))> {};
}  // namespace mystl

#endif