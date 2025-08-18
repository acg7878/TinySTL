#ifndef TINYSTL___TYPE_TRAITS_IS_ASSIGNABLE_H
#define TINYSTL___TYPE_TRAITS_IS_ASSIGNABLE_H

#include <mystl/type_traits.h>
#include <mystl/utility.h>
#include <mystl/__type_traits/integral_constant.h>
namespace mystl {

/*
is_move_assignable
检查类型 T 是否有可访问的移动赋值运算符（operator=(T&&)），
或在无移动赋值时，是否可以通过拷贝赋值运算符（operator=(const T&)）进行赋值
*/
template <class T, class = void>
struct is_move_assignable : false_type {};

// mystl::declval<T&>() = mystl::declval<T&&>():尝试执行赋值操作（用右值给左值赋值）。
template <class T>
struct is_move_assignable<T, decltype(void(mystl::declval<T&>() =
                                               mystl::declval<T&&>()))>
    : mystl::true_type {};
}  // namespace mystl

#endif
