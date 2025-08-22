#ifndef TINYSTL___UTILITY_MOVE_H
#define TINYSTL___UTILITY_MOVE_H

#include <mystl/__type_traits/remove_reference.h>

namespace mystl {

template <typename T>
// T&& arg：转发引用；涉及 引用折叠
typename mystl::remove_reference<T>::type&& move(T&& arg) noexcept {
  return static_cast<typename mystl::remove_reference<T>::type&&>(arg);
}

}  // namespace mystl

#endif