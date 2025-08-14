#ifndef MY_TYPE_TRAITS_IS_POD_H_
#define MY_TYPE_TRAITS_IS_POD_H_

#include "integral_constant.h"

namespace mystl {
// is_POD 精细度不够，C++20已经弃用，c++11不推荐使用
template <typename T>
struct is_pod : public integral_constant<bool, __is_pod(T)> {};

// 判断是否具有平凡析构函数
template <typename T>
struct is_trivially_destructible
    : public integral_constant<bool, __is_trivially_destructible(T)> {};

// 是否具备平凡拷贝构造
template <typename T>
struct is_trivially_copy_constructible
    : public integral_constant<bool,
                               __is_trivially_constructible(T, const T&)> {};

// 是否具有平凡拷贝赋值
template <typename T>
struct is_trivially_copy_assignable
    : public integral_constant<bool, __is_trivially_assignable(T&, const T&)> {
};

// 是否具有平凡拷贝
template <typename T>
struct is_trivially_copyable
    : public integral_constant<bool, __is_trivially_copyable(T)> {};
}  // namespace mystl

#endif