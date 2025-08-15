#ifndef TINYSTL___TYPE_TRAITS_INTEGRAL_CONSTANT_H
#define TINYSTL___TYPE_TRAITS_INTEGRAL_CONSTANT_H

namespace mystl {
template <typename T, T v>
struct integral_constant {
  static constexpr T value = v;
  // constexpr:编译器就要确定值
  // static:直接通过类型名访问 value; 值属于类型，而非对象,没有static要先创建对象才能访问
};

// using:创建类型别名;现代化cpp应抛弃使用typedef（typedef不能处理模板相关！）
using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;
}  // namespace mystl

#endif