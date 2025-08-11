# C++ 类型转换：`static_cast`

`static_cast` 是 C++ 提供的四种类型转换操作符之一（另外三种是 `dynamic_cast`, `const_cast`, `reinterpret_cast`）。它用于在**编译期**执行**相对安全**的类型转换。

其基本语法为：
`static_cast<new_type>(expression)`

## 01 `static_cast` 的主要用途

`static_cast` 主要用于处理 C++ 中大部分良性、有明确定义的类型转换。

### a) 相关类型指针之间的转换

`static_cast` 可以在有继承关系的类指针之间进行转换：
-   **上行转换 (Up-casting)**：将派生类指针转换为基类指针。这是绝对安全的，因为派生类对象本身就是一个基类对象。
-   **下行转换 (Down-casting)**：将基类指针转换为派生类指针。这**不安全**，`static_cast` 不会进行运行期类型检查。如果基类指针实际指向的不是派生类对象，转换后的指针将是无效的，解引用它会导致未定义行为。

```cpp
class Base {};
class Derived : public Base {};

Derived* d = new Derived();
Base* b = static_cast<Base*>(d); // 安全的上行转换

// 不安全的下行转换
Base* b2 = new Base();
Derived* d2 = static_cast<Derived*>(b2); // 编译通过，但运行时危险
```

### b) `void*` 指针的转换

`static_cast` 是将 `void*`（通用指针）转换为具体类型指针的标准方式。

在 `allocator` 的实现中，全局 `::operator new` 返回的是一个 `void*`，因为它不知道这块内存将被用来存储什么类型的对象。我们需要使用 `static_cast` 将其转换为我们需要的 `T*` 类型。

```cpp
// from allocator.h
T* ptr = static_cast<T*>(::operator new(n * sizeof(T)));
```
这个转换是开发者在向编译器保证：“我知道这块内存将被用于存储 `T` 类型的对象，请相信我并完成转换。”

### c) 数值类型之间的转换

`static_cast` 可以在各种数值类型之间进行转换，如 `int` 到 `float`，`enum` 到 `int` 等。

```cpp
int i = 10;
double d = static_cast<double>(i); // int -> double

enum Color { RED, GREEN, BLUE };
int color_val = static_cast<int>(RED); // enum -> int
```

## 02 `static_cast` vs. C 风格转换

在 C++ 中，应始终优先使用 `static_cast` 而不是 C 风格的强制类型转换 `(new_type)expression`。

-   **可读性与可搜索性**：`static_cast` 在代码中非常显眼，可以轻松地在项目中搜索所有的类型转换点，便于代码审查和维护。
-   **更严格的编译期检查**：`static_cast` 只能执行有意义的、编译器认可的转换。它不允许在不相关的指针类型之间进行转换（例如 `int*` 到 `MyClass*`），而 C 风格转换则会粗暴地允许这种危险操作（其行为类似于 `reinterpret_cast`）。

```cpp
int* p_int = ...;
// 错误！编译器会阻止这种不相关的指针转换
MyClass* p_mc = static_cast<MyClass*>(p_int);

// C 风格转换则会允许，埋下安全隐患
MyClass* p_mc_c = (MyClass*)p_int;
```

## 03 总结

`static_cast` 是 C++ 中用于处理大多数常规类型转换的首选工具。它在编译期进行检查，提供了比 C 风格转换更高的类型安全性，同时代码意图也更加明确。在编写 C++ 代码时，应养成使用 C++ 风格类型转换的习惯。