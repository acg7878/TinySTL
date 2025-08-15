#ifndef TINYSTL___TYPE_TRAITS_ENABLE_IF_H
#define TINYSTL___TYPE_TRAITS_ENABLE_IF_H

namespace mystl {
template <bool B, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
  using type = T;
};
}  // namespace mystl

#endif