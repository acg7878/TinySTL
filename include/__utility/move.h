#ifndef TINYSTL___UTILITY_MOVE_H_
#define TINYSTL___UTILITY_MOVE_H_

#include "../type_traits.h"

namespace mystl {

template <typename T>
// T&& arg：转发引用
typename mystl::remove_reference<T>::type&& move(T&& arg) noexcept {
  return static_cast<typename mystl::remove_reference<T>::type&&>(arg);
}

}  // namespace mystl

#endif