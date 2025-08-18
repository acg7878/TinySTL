# `declval`

`declval` (declared value) 是一个定义在 `<utility>` 头文件中的 C++ 标准库函数模板。它是一个纯粹的**编译期**工具，用于在**未求值上下文 (unevaluated context)**（如 `decltype` 或 `sizeof`）中，**“假装”创建了一个指定类型的对象**，以便进行类型推导。

## 01 问题背景：无法构造的类型

在进行模板元编程时，我们经常需要推断一个成员函数调用的返回类型。例如，我们想知道对于类型 `T` 和参数 `U`，`t.foo(u)` 的返回类型是什么。

一个自然的想法是使用 `decltype`：
```cpp
decltype( T().foo( U() ) )
```
这个方法在很多情况下可行，但如果**类型 `T` 或 `U` 没有默认构造函数**，这行代码就会编译失败，因为 `T()` 或 `U()` 无法被创建。

我们需要一种方法，能够在**不实际调用任何构造函数**的情况下，获得一个“虚拟”的 `T` 类型对象，只为了让 `decltype` 能够分析表达式的类型。

## 02 `declval` 的解决方案

`declval` 正是为此而生。它的功能可以概括为：

> 将任意类型 `T` 转换成该类型的**右值引用 `T&&`**，并且**不需要为 `T` 提供任何构造函数**。

**重要**：`declval` **不能被实际调用**。如果你尝试在求值上下文中调用它，程序将无法编译。

### 语法

```cpp
template <class T>
typename std::add_rvalue_reference<T>::type declval() noexcept;
```
*（`std::add_rvalue_reference` 是一个类型萃取，用于为类型 `T` 添加 `&&`）*

### 使用示例

现在，我们可以安全地推断成员函数的返回类型了：
```cpp
#include <utility> // for std::declval

struct MyClass {
    int foo(double);
    MyClass(int); // 没有默认构造函数
};

// 使用 declval 来“假装”创建了 MyClass 和 double 的对象
using ReturnType = decltype( std::declval<MyClass>().foo(std::declval<double>()) );

// ReturnType 现在就是 int，整个过程完全在编译期完成，
// 并且不需要 MyClass 的默认构造函数。
```

## 03 `MyTinySTL` 中的实现解析



```cpp
// 辅助模板，用于在 static_assert 中创建依赖于模板参数的 false 值
template <typename T>
struct dependent_false : mystl::false_type {};

// declval_helper 的两个重载
template <typename T>
T&& declval_helper(int); // 优先匹配的版本

template <typename T>
T declval_helper(long); // 备用版本

template <typename T>
decltype(mystl::declval_helper<T>(0)) declval() noexcept {
  static_assert(dependent_false<T>::value, "declval() must not be evaluated");
}
```
1.  **`declval_helper`**:
    -   提供了两个函数声明（没有定义）。
    -   当调用 `declval_helper<T>(0)` 时，字面量 `0` 的类型是 `int`，因此会优先匹配第一个重载 `declval_helper(int)`。
    -   这个重载的返回类型是 `T&&`。

2.  **`declval`**:
    -   `decltype(mystl::declval_helper<T>(0))`: `decltype` 推断出 `declval_helper<T>(0)` 这个表达式的类型，也就是第一个重载的返回类型 `T&&`。
    -   `static_assert(dependent_false<T>::value, ...)`: `dependent_false<T>` 依赖于模板参数 `T`，所以它的 `::value` 只有在 `declval` 被**实例化**时才会被求值。而 `declval` 的函数体只有在它被**实际调用**时才会被实例化。由于我们只在 `decltype` 这样的未求值上下文中使用 `declval`，它的函数体永远不会被实例化，所以 `static_assert` 永远不会触发。但如果你错误地调用了 `declval()`，`static_assert(false, ...)` 就会导致编译失败，并给出明确的错误信息。
