#ifndef TINYSTL___TYPE_TRAITS_IS_VOID_H
#define TINYSTL___TYPE_TRAITS_IS_VOID_H

#include <mystl/__type_traits/integral_constant.h>
#include <mystl/__type_traits/remove_cv.h>

namespace mystl {
template <typename T>
struct is_void : mystl::false_type {};

template <>
struct is_void<void> : mystl::true_type {};

template <typename T>
struct is_void<const T> : is_void<T> {};

template <typename T>
struct is_void<volatile T> : is_void<T> {};

template <typename T>
struct is_void<const volatile T> : is_void<T> {};
}  // namespace mystl

#endif