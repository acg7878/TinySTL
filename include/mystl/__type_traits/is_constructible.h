#ifndef TINYSTL___TYPE_TRAITS_IS_CONSTRUCTIBLE_H
#define TINYSTL___TYPE_TRAITS_IS_CONSTRUCTIBLE_H

#include <mystl/__utility/declval.h>

namespace mystl {

template <class T, class... Args>
struct is_constructible_helper {
 private:
  // mystl::declval<As>()... 展开成 declval<As1>(), declval<As2>(), ...
  template <class U, class... As, class = decltype(U(mystl::declval<As>()...))>
  static mystl::true_type test(int);

  template <class, class...>
  static mystl::false_type test(...);

 public:
  using type = decltype(test<T, Args...>(0));
};

// 主模板
template <class T, class... Args>
struct is_constructible : is_constructible_helper<T, Args...>::type {};

/*
is_move_constructible
检查类型 T 是否有可访问的移动构造函数
*/

// 主模板，第二个参数用于特化
template <class T, class = void>
struct is_move_constructible : mystl::false_type {};

// 如果移动构造函数合法就会继承true_type
// declval<T&&>()：生成一个 T 类型的右值引用
// T(declval<T&&>()):尝试调用移动构造函数，看看是否存在
// decltype(...)：进行一个推断，如果构造函数合法，就得到void
template <class T>
struct is_move_constructible<T, decltype(void(T(declval<T&&>())))>
    : mystl::true_type {};
}  // namespace mystl

#endif