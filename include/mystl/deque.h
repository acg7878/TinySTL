#ifndef TINYSTL_DEQUE_H
#define TINYSTL_DEQUE_H


#include <mystl/__memory/split_buffer.h>
#include <mystl/__iterator/iterator_traits.h>
#include <mystl/__type_traits/enable_if.h>
#include <mystl/__type_traits/is_convertible.h>

namespace mystl {

// 计算 deque 块大小的辅助结构
template <class ValueType, class DiffType>
struct deque_block_size {
  static const DiffType value =
      sizeof(ValueType) < 256 ? 4096 / sizeof(ValueType) : 16;
};

// deque 迭代器
// BS: Block Size，块大小模板参数
// 默认值使用 deque_block_size 计算的值
template <class ValueType,
          class Pointer,
          class Reference,
          class MapPointer,
          class DiffType,
          DiffType BS = deque_block_size<ValueType, DiffType>::value>
class deque_iterator {
 public:
  using value_type = ValueType;
  using pointer = Pointer;
  using reference = Reference;
  using difference_type = DiffType;
  using iterator_category = mystl::random_access_iterator_tag;

 private:
  MapPointer __m_iter_;  // 指向 map 中某个块的指针
  pointer __ptr_;         // 指向当前块中的元素

  // 静态成员：块大小
  // 注意：虽然 BS 是模板参数，但这里使用 deque_block_size 计算的值
  // 这样可以保证正确性，同时保持灵活性
  static const difference_type __block_size;

 public:
  // 构造函数
  deque_iterator() noexcept : __m_iter_(nullptr), __ptr_(nullptr) {}

  // 从其他迭代器类型转换（如果指针类型可转换）
  // TODO: 需要实现 is_convertible，暂时注释掉
  // template <class Pp, class Rp, class MP,
  //           typename = typename enable_if<
  //               is_convertible<Pp, pointer>::value>::type>
  // deque_iterator(const deque_iterator<value_type, Pp, Rp, MP, difference_type,
  //                                     BS>& it) noexcept
  //     : __m_iter_(it.__m_iter_), __ptr_(it.__ptr_) {}

  // 私有构造函数：供 deque 类使用
 private:
  explicit deque_iterator(MapPointer m, pointer p) noexcept
      : __m_iter_(m), __ptr_(p) {}

  // 让 deque 类成为友元，可以访问私有构造函数
  template <class Tp, class Alloc>
  friend class deque;

  template <class Vp, class Pp, class Rp, class MP, class Dp, Dp>
  friend class deque_iterator;

 public:
  // 解引用操作
  reference operator*() const { return *__ptr_; }
  pointer operator->() const { return __ptr_; }

  // 前置递增
  deque_iterator& operator++() {
    if (++__ptr_ - *__m_iter_ == __block_size) {
      ++__m_iter_;
      __ptr_ = *__m_iter_;
    }
    return *this;
  }

  // 后置递增
  deque_iterator operator++(int) {
    deque_iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  // TODO: 实现其他操作符...
};

// 静态成员定义：在类外初始化
template <class ValueType, class Pointer, class Reference, class MapPointer,
          class DiffType, DiffType BS>
const DiffType deque_iterator<ValueType, Pointer, Reference, MapPointer,
                               DiffType, BS>::__block_size =
    deque_block_size<ValueType, DiffType>::value;

}  // namespace mystl

#endif