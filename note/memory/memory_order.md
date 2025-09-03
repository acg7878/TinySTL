# memory_order

## 概述

`memory_order` 是 C++11 引入的原子操作中的一个枚举类型，用于指定原子操作的内存顺序。内存顺序指定了原子操作对其他线程的可见性，以及编译器和处理器可以对原子操作进行重排序的约束。

## 内存顺序类型

C++11 定义了以下几种内存顺序：

*   `memory_order_relaxed`: 最宽松的内存顺序，仅保证原子性，不保证线程间的同步。
*   `memory_order_consume`: 消费顺序，用于建立依赖关系。
*   `memory_order_acquire`: 获取顺序，用于同步数据。
*   `memory_order_release`: 释放顺序，用于同步数据。
*   `memory_order_acq_rel`: 获取释放顺序，同时具有获取和释放的特性。
*   `memory_order_seq_cst`: 顺序一致性顺序，最强的内存顺序，保证所有线程以相同的顺序看到所有原子操作。

## 内存顺序的含义

### `memory_order_relaxed`

`memory_order_relaxed` 是最宽松的内存顺序。使用 `memory_order_relaxed` 的原子操作只保证原子性，不保证线程间的同步。这意味着编译器和处理器可以自由地对 `memory_order_relaxed` 的原子操作进行重排序。

`memory_order_relaxed` 适用于不需要线程间同步的原子操作，例如计数器。

### `memory_order_consume`

`memory_order_consume` 用于建立依赖关系。如果线程 A 使用 `memory_order_release` 释放了变量 x，线程 B 使用 `memory_order_consume` 读取了 x，那么线程 A 对 x 的写入操作对线程 B 可见。此外，线程 A 对 x 依赖的变量的写入操作也对线程 B 可见。

`memory_order_consume` 的使用比较复杂，需要仔细考虑依赖关系。

### `memory_order_acquire`

`memory_order_acquire` 用于同步数据。如果线程 A 使用 `memory_order_release` 释放了变量 x，线程 B 使用 `memory_order_acquire` 读取了 x，那么线程 A 对 x 的写入操作对线程 B 可见。此外，线程 A 在释放 x 之前的所有写入操作也对线程 B 可见。

`memory_order_acquire` 通常用于保护临界区。

### `memory_order_release`

`memory_order_release` 用于同步数据。如果线程 A 使用 `memory_order_release` 释放了变量 x，线程 B 使用 `memory_order_acquire` 读取了 x，那么线程 A 对 x 的写入操作对线程 B 可见。此外，线程 A 在释放 x 之前的所有写入操作也对线程 B 可见。

`memory_order_release` 通常与 `memory_order_acquire` 配对使用。

### `memory_order_acq_rel`

`memory_order_acq_rel` 同时具有获取和释放的特性。如果线程 A 使用 `memory_order_acq_rel` 修改了变量 x，线程 B 使用 `memory_order_acquire` 读取了 x，那么线程 A 对 x 的写入操作对线程 B 可见。此外，线程 A 在修改 x 之前的所有写入操作也对线程 B 可见。如果线程 C 使用 `memory_order_release` 释放了变量 x，线程 A 使用 `memory_order_acq_rel` 修改了 x，那么线程 C 对 x 的写入操作对线程 A 可见。此外，线程 C 在释放 x 之前的所有写入操作也对线程 A 可见。

`memory_order_acq_rel` 通常用于修改共享变量。

### `memory_order_seq_cst`

`memory_order_seq_cst` 是最强的内存顺序。使用 `memory_order_seq_cst` 的原子操作保证所有线程以相同的顺序看到所有原子操作。这意味着编译器和处理器不能对 `memory_order_seq_cst` 的原子操作进行重排序。

`memory_order_seq_cst` 适用于需要保证所有线程以相同的顺序看到所有原子操作的场景。

## 总结

`memory_order` 是 C++11 引入的原子操作中的一个枚举类型，用于指定原子操作的内存顺序。内存顺序指定了原子操作对其他线程的可见性，以及编译器和处理器可以对原子操作进行重排序的约束。选择合适的内存顺序可以提高程序的性能，同时保证线程间的同步。