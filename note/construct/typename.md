# typename

`typename` 是 C++ 中的一个关键字，它有两个主要用途。其中一个用途与 `class` 关键字在模板参数声明中可以互换，但我们主要关注其更重要、也更复杂的第二个用途：**向编译器指明一个依赖名称是类型**。

## 01 `typename` 的核心用途：消除歧义

在 C++ 模板编程中，模板实例化之前编译器无法确定一个**依赖于模板参数的名称 (dependent name)** 到底是一个**类型**还是一个**值**（比如静态成员变量或函数）。

### 什么是依赖名称？

当一个名称的含义依赖于一个或多个模板参数时，它就是依赖名称。最常见的形式是 `T::member` 或 `std::iterator_traits<Iter>::value_type`。

在编译器看到模板定义，但尚未用具体类型实例化模板时，它不知道 `T` 到底是什么。因此，它也无从得知 `T::member` 究竟是一个类型（比如 `T` 内部定义的 `typedef` 或 `using` 别名，或是一个嵌套类），还是一个静态成员变量。

### 歧义的产生

看下面的例子：
```cpp
template <typename T>
void process() {
    T::value_type* ptr; // 这里有歧义
}
```
对于编译器来说，这行代码有两种可能的解释：
1.  **解释一（乘法）**：如果 `T::value_type` 是一个静态成员变量（比如 `static int value_type = 10;`），那么这行代码就是一个乘法表达式：`T::value_type` 乘以 `ptr`。
2.  **解释二（指针声明）**：如果 `T::value_type` 是一个类型（比如 `typedef int value_type;`），那么这行代码就是声明一个指向该类型的指针 `ptr`。

### `typename` 的解决方案

为了解决这种歧义，C++ 标准规定：**默认情况下，编译器将依赖的、通过 `::` 访问的名称假定为值（非类型）**。

如果你想告诉编译器“这个依赖名称实际上是一个类型”，你就必须在其前面显式地使用 `typename` 关键字。

正确的写法是：
```cpp
template <typename T>
void process() {
    typename T::value_type* ptr; // OK: 明确告诉编译器 T::value_type 是一个类型
}
```

## 02 应用场景
在 `destroy` 函数中就用到了它：
```cpp
template <typename ForwardIterator>
void destroy(ForwardIterator first, ForwardIterator last) {
  destroy_cat(first, last,
              std::is_trivially_destructible<
                  typename std::iterator_traits<ForwardIterator>::value_type>{});
}
```
在这里，`std::iterator_traits<ForwardIterator>::value_type` 是一个依赖名称，因为它依赖于模板参数 `ForwardIterator`。用它来获取迭代器所指向的元素的类型，所以必须在它前面加上 `typename`，以确保编译器能正确地将其解析为一个类型名。

## 03 何时不需要 `typename`？

有两个主要例外情况，不需要（也不能）在依赖名称前使用 `typename`：

1.  **基类列表**：在模板类的基类列表中。
    ```cpp
    template <typename T>
    class MyDerived : public MyBase<T>::NestedType { ... };
    ```
2.  **成员初始化列表**：在构造函数的成员初始化列表中。
    ```cpp
    template <typename T>
    MyClass<T>::MyClass() : my_member(T::initial_value) { ... }
    ```

在这些上下文中，编译器可以明确地推断出该名称必须是一个类型，因此 `typename` 是不必要的。