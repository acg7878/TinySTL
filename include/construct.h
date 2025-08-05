#ifndef MY_CONSTRUCT_H_
#define MY_CONSTRUCT_H_

#include <type_traits>
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
void destroy_one(T* /*ptr*/, std::true_type) noexcept {
  // /*ptr*/ ： 表明函数形参存在，但这个参数在函数体内没用到，所以注释掉。
  // 这样写可以避免编译器因为未使用参数而产生“未使用参数”的警告（warning）。
  // noexcept：标记函数不会抛出异常
  // trivially destructible 类型无需析构
}

//如果类型非平凡析构（std::false_type），才调用析构函数。
template <typename T>
void destroy_one(T* ptr, std::false_type) noexcept {
  if (ptr != nullptr) {
    ptr->~T();
  }
}

template <typename T>
void destroy(T* ptr) noexcept {
  destroy_one(ptr, std::is_trivially_destructible<T>{});
}

}  // namespace mystl

#endif