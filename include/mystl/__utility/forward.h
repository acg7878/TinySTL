#ifndef TINYSTL___UTILITY_FORWARD_H
#define TINYSTL___UTILITY_FORWARD_H

#include <mystl/__type_traits/remove_reference.h>

namespace mystl {

// forward 函数：完美转发
// 用于在模板函数中将参数的值类别（左值/右值）原样转发给其他函数

// 重载1：接受左值引用参数
// 当传入左值时，T 会被推导为 T&（引用折叠规则）
// remove_reference<T>::type& 确保参数类型是左值引用
template <typename T>
constexpr T&& forward(typename mystl::remove_reference<T>::type& arg) noexcept {
  // static_cast<T&&> 进行引用折叠：
  // - 如果 T 是 T&，则 T&& 折叠为 T&（左值引用）
  // - 如果 T 是 T，则 T&& 是 T&&（右值引用）
  return static_cast<T&&>(arg);
}

// 重载2：接受右值引用参数
// 当传入右值时，T 会被推导为 T（非引用类型）
// remove_reference<T>::type&& 确保参数类型是右值引用
template <typename T>
constexpr T&& forward(typename mystl::remove_reference<T>::type&& arg) noexcept {
  // 注意：这里应该检查 T 不是左值引用类型
  // 标准库使用 static_assert(!is_lvalue_reference<T>::value, ...)
  // 这里简化实现，不包含检查
  return static_cast<T&&>(arg);
}

}  // namespace mystl

#endif  // TINYSTL___UTILITY_FORWARD_H

