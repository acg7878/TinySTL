# C++ 关键字：`constexpr`

`constexpr` 是 C++11 引入的一个关键特性，其核心含义是“**常量表达式 (constant expression)**”。它允许将计算过程从程序的**运行时 (runtime)** 提前到**编译期 (compile-time)**，这对于性能优化和模板元编程至关重要。

`constexpr` 可以用来修饰变量、函数和构造函数。

## 01 `constexpr` 变量

当 `constexpr` 修饰变量时，它要求该变量的值必须在**编译期**就能确定。这比 `const` 关键字的约束更强。

```cpp
const int runtime_const = get_current_time(); // OK: const 变量可以在运行时初始化
constexpr int compile_time_const = 42;      // OK: 42 是编译期常量

// 错误！get_current_time() 的值在运行时才能确定
constexpr int error_const = get_current_time();
```

`constexpr` 变量最大的优势在于它们可以用在任何需要编译期常量的地方，例如数组大小、模板非类型参数、枚举值等。

```cpp
constexpr int array_size = 10;
int my_array[array_size]; // OK

template<int N> class MyClass {};
MyClass<array_size> obj; // OK
```

## 02 `constexpr` 函数

当 `constexpr` 修饰函数时，它向编译器承诺：如果该函数的所有参数都是编译期常量，那么其返回值也能在编译期计算出来。

如果满足这个条件，编译器会在编译阶段直接执行该函数，并将结果值替换掉函数调用。如果参数是运行时变量，该函数则会像普通函数一样在运行时执行。

```cpp
// 一个编译期的阶乘函数
constexpr int factorial(int n) {
    return n <= 1 ? 1 : (n * factorial(n - 1));
}

// 场景1: 编译期计算
// 编译器直接计算出 factorial(5) 的值为 120
int arr[factorial(5)]; // 等价于 int arr[120];

// 场景2: 运行时计算
int x = 10;
int result = factorial(x); // 像普通函数一样在运行时调用
```

一个函数要成为 `constexpr` 函数，其函数体必须满足一定的限制（例如，不能有 `static` 或 `thread_local` 变量，不能有 `goto` 语句等），这些限制在 C++14/17/20 中被逐步放宽。

## 03 `constexpr` vs. `const`

| 特性 | `const` | `constexpr` |
| :--- | :--- | :--- |
| **含义** | 常量 (Constant) | 常量表达式 (Constant Expression) |
| **核心约束** | 变量初始化后**不可被修改** | 变量的值必须在**编译期可知** |
| **求值时机** | 可以在运行时求值 | **必须**在编译期求值 |
| **用途** | 保证数据不被意外修改 | 性能优化（编译期计算），用于模板参数、数组大小等 |

在 `type_traits` 的实现中，`constexpr` 保证了 `integral_constant::value` 是一个真正的编译期常量，这是进行后续模板元编程的基础。

## 04 `constexpr` 构造函数

`constexpr` 也可以修饰类的构造函数。这意味着如果构造函数的所有参数都是编译期常量，那么就可以在编译期创建一个该类的**常量对象**。

```cpp
class Point {
public:
    constexpr Point(double x, double y) : x_(x), y_(y) {}
    constexpr double get_x() const { return x_; }
    constexpr double get_y() const { return y_; }
private:
    double x_, y_;
};

// 在编译期创建一个 Point 对象
constexpr Point p1(1.0, 2.0);

// 在编译期调用其成员函数
double x_val = p1.get_x(); // x_val 在编译期就被初始化为 1.0
```

这使得用户自定义的类型（字面量类型）也能像内置类型一样参与到编译期计算中，极大地扩展了元编程的能力。