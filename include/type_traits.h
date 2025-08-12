#ifndef MY_TYPE_TRAITS_H_
#define MY_TYPE_TRAITS_H_

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

// 默认规则：对于任意两种类型 T 和 U，is_same<T, U> 的结果默认为 false
template <typename T, typename U>
struct is_same : public false_type {};

template <typename T>
struct is_same<T, T> : public true_type {};

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