# Placement New

简单来说，`Placement New` 的作用是：在一块已经分配好的、现成的内存地址上构造一个对象，但它本身不分配内存。

## 01 普通 new 与 Placement new
为了理解 `Placement New`，先看看熟悉的 `new` 做了什么。

```cpp
MyClass* p = new MyClass();
```
`new` 操作符实际上执行了两个步骤：

1. 分配内存：调用 `::operator new()` 在堆上分配一块足以容纳 `MyClass` 对象的内存。
2. 构造对象：在那块刚分配的内存上，调用 `MyClass` 的构造函数来初始化对象。
而 Placement New 将这两个步骤分开了。它只负责第 2 步。

它的语法是这样的：
```cpp
// 假设 buffer 是一块已经存在的、足够大的内存
MyClass* p = new (buffer) MyClass();
```


这行代码告诉编译器：“请不要分配新内存了，直接在 buffer 这块地址上调用 MyClass 的构造函数，然后把指向这个新构造对象的指针返回给我。”

## 02 STL 容器的核心需求
这正是实现像 `vector` 这样的容器所必需的。

例如：`vector` 的 `reserve()` 函数。当你调用 `vec.reserve(100)` 时，`vector` 会预先分配能容纳 100 个元素的连续内存块。

如果用普通 `new`：它会立即在这 100 个位置上都构造出对象。但此时 `vector` 的大小`（size()）`还是 0，这些提前构造的对象是多余且浪费资源的。
正确的做法（使用 Placement New）：
`vector` 先用内存分配器分配一块原始的、未初始化的内存。
当调用 `push_back()` 添加一个新元素时，`vector` 才在下一个可用的内存位置上，使用 `Placement New` 来构造这一个新对象。
这样就完美地实现了内存分配和对象构造的分离，大大提高了效率。
## 03 使用和销毁
使用 `Placement New` 有一个非常重要的规则：谁构造，谁析构。

因为 `Placement New` 没有分配内存，所以你绝对不能对它返回的指针使用 `delete`。`delete p`; 会尝试释放内存，但这块内存不是它分配的，会导致程序崩溃。

正确的销毁步骤是：

1. 显式调用析构函数：手动调用对象的析构函数来释放对象自身占用的资源（比如对象内部的指针）。

```cpp
p->~MyClass(); // 直接调用析-构函数
```

2. 释放原始内存：用当初分配这块内存的方式来释放它。

```cpp
// 如果 buffer 是用 ::operator new 分配的
::operator delete(buffer);
```