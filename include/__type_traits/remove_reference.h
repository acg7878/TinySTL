#ifndef TINYSTL___TYPE_TRAITS_REMOVE_REFERENCE_H
#define TINYSTL___TYPE_TRAITS_REMOVE_REFERENCE_H

namespace mystl {
template <typename T>
struct remove_reference {
  using type = T;
};

template <typename T>
struct remove_reference<T&> {
  using type = T;
};

template <typename T>
struct remove_reference<T&&> {
  using type = T;
};
}  // namespace mystl

#endif