#ifndef TINYSTL___UTILITY_DECLVAL_H
#define TINYSTL___UTILITY_DECLVAL_H

#include <mystl/__type_traits/integral_constant.h>

// declval.h:编译期工具，不能在运行时调用

namespace mystl {

// 一个辅助模板，用于在 static_assert 中创建一个依赖于模板参数的 false 值
// 直接使用 static_assert(false, "...") 会在模板实例化之前就报错
template <typename T>
struct dependent_false : mystl::false_type {};

// declval_helper 的两个重载，仅用于 decltype 类型推导
// 当传入 int(0) 时，会优先匹配第一个版本，返回 T&&
template <typename T>
T&& declval_helper(int);

template <typename T>
T declval_helper(long);

/**
 * @brief 在未求值上下文中，将任意类型 T 转换成 T 的右值引用
 * @tparam T 目标类型
 * @note declval 只能用于未求值的操作数中，如 decltype 或 sizeof
 */
template <typename T>
decltype(mystl::declval_helper<T>(0)) declval() noexcept {
  static_assert(dependent_false<T>::value, "declval() must not be evaluated");
}

}  // namespace mystl

#endif