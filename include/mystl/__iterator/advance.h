#ifndef TINYSTL_ITERATOR_ADVANCE_H
#define TINYSTL_ITERATOR_ADVANCE_H

#include <mystl/__iterator/iterator_traits.h>

namespace mystl {

// ====== advance 实现 ======
template <class InputIterator, class Distance>
void advance_dispatch(InputIterator& it, Distance n, input_iterator_tag) {
  while (n--) {
    it++;
  }
}

template <class BidirectionalIterator, class Distance>
void advance_dispatch(BidirectionalIterator& it, Distance n,
                      bidirectional_iterator_tag) {
  if (n >= 0) {
    while (n--) {
      it++;
    }
  } else {
    while (n++) {
      it--;
    }
  }
}

template <class RandomAccessIterator, class Distance>
void advance_dispatch(RandomAccessIterator& it, Distance n,
                      random_access_iterator_tag) {
  it += n;
}

template <class Iterator, class Distance>
void advance(Iterator& it, Distance n) {
  // using category = typename iterator_traits<Iterator>::iterator_category;
  advance_dispatch(it, n, iterator_category(it));
}

}  // namespace mystl

#endif