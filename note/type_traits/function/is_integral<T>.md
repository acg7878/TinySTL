# `is_integral<T>`

## 定义

`is_integral<T>` 是 C++ `<type_traits>` 库中的一个模板类，用于检查类型 `T` 是否为整型。

```cpp
template <class T>
struct is_integral;
```

## 作用

`is_integral<T>` 提供了一种在编译时确定类型 `T` 是否为整型的方法。这在模板编程中非常有用，可以根据类型是否为整型来选择不同的代码路径。

## 用法

`is_integral<T>` 继承自 `std::true_type` 或 `std::false_type`。如果 `T` 是整型（例如 `int`、`char`、`bool` 等），则 `is_integral<T>::value` 为 `true`，否则为 `false`。

```cpp
#include <iostream>
#include <type_traits>

int main() {
    std::cout << std::boolalpha;
    std::cout << "int: " << std::is_integral<int>::value << std::endl;
    std::cout << "double: " << std::is_integral<double>::value << std::endl;
    std::cout << "bool: " << std::is_integral<bool>::value << std::endl;
    std::cout << "char: " << std::is_integral<char>::value << std::endl;
    return 0;
}
```

输出：

```
int: true
double: false
bool: true
char: true
```

## 注意事项

*   `is_integral<T>` 只能用于类型 `T`。
*   `is_integral<T>` 的结果在编译时确定，不会产生运行时开销。
*   `is_integral<T>` 可以用于任何整型类型，包括 `int`、`char`、`bool`、`short`、`long` 等。

## 示例

以下示例展示了如何在模板中使用 `is_integral<T>` 来选择不同的代码路径：

```cpp
#include <iostream>
#include <type_traits>

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
process_integral(T value) {
    std::cout << "Processing integral value: " << value << std::endl;
    return value * 2;
}

template <typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type
process_integral(T value) {
    std::cout << "Type is not integral." << std::endl;
    return value;
}

int main() {
    int i = 10;
    double d = 3.14;
    process_integral(i); // 输出：Processing integral value: 10
    process_integral(d); // 输出：Type is not integral.
    return 0;
}