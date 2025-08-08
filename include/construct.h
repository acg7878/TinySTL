#ifndef MY_CONSTRUCT_H_
#define MY_CONSTRUCT_H_

#include <iterator>      
#include "type_traits.h" 
#include <utility>       
namespace mystl {

//无参数 (默认构造)
template <typename T>
void construct(T* ptr) {
  ::new (static_cast<void*>(ptr)) T();
}

// 单参数 (拷贝/移动构造)
template <typename T, typename Arg>
void construct(T* ptr, Arg&& arg) {
  ::new (static_cast<void*>(ptr)) T(std::forward<Arg>(arg));
}

// 多参数：参数包支持了0或多个参数，其实不需要写无参数或者单参数的construct，只是为了学习捏
// typename...：模板参数包
// Args&&... 函数参数包
template <typename T, typename... Args>
void construct(T* ptr, Args&&... args) {
  ::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
  // new (address) Type(constructor_args);
  // placement new语法
  // `::` : 代表使用全局作用域的函数
}

// 为平凡析构（trivially destructible）做优化
// 如果类型是平凡析构（std::true_type），析构函数什么都不做，直接跳过，避免没必要的析构开销。
template <typename T>
void destroy_one(T* /*ptr*/, mystl::true_type) noexcept {
  // /*ptr*/ ： 表明函数形参存在，但这个参数在函数体内没用到，所以注释掉。
  // 这样写可以避免编译器因为未使用参数而产生“未使用参数”的警告（warning）。
  // noexcept：标记函数不会抛出异常
  // trivially destructible 类型无需析构
}

//如果类型非平凡析构（std::false_type），才调用析构函数。
template <typename T>
void destroy_one(T* ptr, mystl::false_type) noexcept {
  if (ptr != nullptr) {
    ptr->~T();
  }
}

template <typename T>
void destroy(T* ptr) noexcept {
  destroy_one(ptr, mystl::is_trivially_destructible<T>{});
}

template <typename ForwardIterator>
void destroy_cat(ForwardIterator /*first*/, ForwardIterator /*last*/, mystl::true_type) {
  // 平凡析构类型，无需构析
}

template <typename ForwardIterator>
void destroy_cat(ForwardIterator first, ForwardIterator last, mystl::false_type) {
  for (; first != last; ++first) {
    destroy(&*first);
    //*first：解引用迭代器，获得当前元素引用
    //&*first：取地址，获得元素指针
  }
}

template <typename ForwardIterator>
void destroy(ForwardIterator first, ForwardIterator last) {
  destroy_cat(
      first, last,
      mystl::is_trivially_destructible<
          typename std::iterator_traits<ForwardIterator>::value_type>{});
          // value_type：提取出迭代器的类型，如std::vector<T>::iterator、int*
          // destroy_one不需要是因为已经知道是T类型了
}

}  // namespace mystl

#endif