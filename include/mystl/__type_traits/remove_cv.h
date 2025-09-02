#ifndef TINYSTL___TYPE_TRAITS_REMOVE_CV_H
#define TINYSTL___TYPE_TRAITS_REMOVE_CV_H

namespace mystl {
template <typename T>
struct remove_const {
  using type = T;
};

template <typename T>
struct remove_const<const T> {
  using type = T;
};

template <typename T>
struct remove_volatile {
  using type = T;
};

template <typename T>
struct remove_volatile<volatile T> {
  using type = T;
};

template <typename T>
struct remove_cv {
  using type = typename remove_const<typename remove_volatile<T>::type>::type;
};

}  // namespace mystl

#endif