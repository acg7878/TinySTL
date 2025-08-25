#ifndef TINYSTL___UTILITY_SWAP_H
#define TINYSTL___UTILITY_SWAP_H

#include <mystl/__utility/move.h>

namespace mystl {

template <typename T>
void swap(T& a, T& b) noexcept
/*(mystl::is_nothrow_move_constructible<T>::value &&
    mystl::is_nothrow_move_assignable<T>::value)*/
{
  T tmp(mystl::move(a));
  a = mystl::move(b);
  b = mystl::move(tmp);
}

}  // namespace mystl

#endif