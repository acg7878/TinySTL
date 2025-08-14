#ifndef MY_TYPE_TRAITS_IS_INTEGRAL_H_
#define MY_TYPE_TRAITS_IS_INTEGRAL_H_

#include "integral_constant.h"

namespace mystl {
template <typename T>
struct is_integral : mystl::false_type {};

template <>
struct is_integral<bool> : true_type {};
template <>
struct is_integral<char> : true_type {};
template <>
struct is_integral<signed char> : true_type {};
template <>
struct is_integral<unsigned char> : true_type {};
template <>
struct is_integral<char16_t> : true_type {};
template <>
struct is_integral<char32_t> : true_type {};
template <>
struct is_integral<wchar_t> : true_type {};
template <>
struct is_integral<short> : true_type {};
template <>
struct is_integral<int> : true_type {};
template <>
struct is_integral<long> : true_type {};
template <>
struct is_integral<long long> : true_type {};
template <>
struct is_integral<unsigned short> : true_type {};
template <>
struct is_integral<unsigned int> : true_type {};
template <>
struct is_integral<unsigned long> : true_type {};
template <>
struct is_integral<unsigned long long> : true_type {};
}  // namespace mystl

#endif