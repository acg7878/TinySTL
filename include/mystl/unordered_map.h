#ifndef TINYSTL_UNORDERED_MAP_H_
#define TINYSTL_UNORDERED_MAP_H_

// 引入我们自己的 hash_table 实现
#include "__hash_table.h"
#include <cstddef>           // for size_t, ptrdiff_t
#include <functional>        // for hash, equal_to
#include <initializer_list>  // for initializer_list
#include <iterator>          // for forward_iterator_tag
#include <memory>            // for allocator, addressof
#include <stdexcept>         // for out_of_range
#include <type_traits>       // for remove_reference_t, declval
#include <utility>           // for pair, move, forward

namespace mystl {

// 1. 前置声明：哈希表适配器（适配 hash_table 所需的哈希函数、相等性比较）
// 作用：将 unordered_map 的 Key 哈希/比较，适配为 hash_table 对 "键值对节点" 的操作
template <class Key, class ValueType, class Hash, class KeyEqual>
class unordered_map_hasher;

template <class Key, class ValueType, class Hash, class KeyEqual>
class unordered_map_key_equal;

// 2. 迭代器适配器（将 hash_table 迭代器转换为返回 value_type 的迭代器）
// 参考标准库的 hash_map_iterator 实现
template <class HashIterator>
class hash_map_iterator {
 private:
  HashIterator i_;

  // 从 get_value() 的返回类型提取 value_type
  using node_value_type = typename HashIterator::value_type;
  using value_type = std::remove_reference_t<
      decltype(std::declval<node_value_type>().get_value())>;

 public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = typename HashIterator::difference_type;
  using reference = value_type&;
  using pointer = value_type*;

  hash_map_iterator() noexcept = default;

  explicit hash_map_iterator(HashIterator i) noexcept : i_(i) {}

  reference operator*() const { return i_->get_value(); }

  pointer operator->() const { return std::addressof(i_->get_value()); }

  hash_map_iterator& operator++() {
    ++i_;
    return *this;
  }

  hash_map_iterator operator++(int) {
    hash_map_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  bool operator==(const hash_map_iterator& other) const {
    return i_ == other.i_;
  }

  bool operator!=(const hash_map_iterator& other) const {
    return i_ != other.i_;
  }

  // 允许 unordered_map 访问底层迭代器
  template <class, class, class, class, class>
  friend class unordered_map;

  // 获取底层迭代器（供 erase 等方法使用）
  HashIterator get_iterator() const { return i_; }
};

// const 迭代器适配器
template <class HashIterator>
class hash_map_const_iterator {
 private:
  HashIterator i_;

  // 从 get_value() 的返回类型提取 value_type
  using node_value_type = typename HashIterator::value_type;
  using value_type = std::remove_reference_t<
      decltype(std::declval<const node_value_type>().get_value())>;

 public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = typename HashIterator::difference_type;
  using reference = const value_type&;
  using pointer = const value_type*;

  hash_map_const_iterator() noexcept = default;

  explicit hash_map_const_iterator(HashIterator i) noexcept : i_(i) {}

  // 从非 const 迭代器构造（如果底层迭代器支持）
  template <class NonConstIterator>
  hash_map_const_iterator(hash_map_iterator<NonConstIterator> i) noexcept
      : i_(i.get_iterator()) {}

  reference operator*() const { return i_->get_value(); }

  pointer operator->() const { return std::addressof(i_->get_value()); }

  hash_map_const_iterator& operator++() {
    ++i_;
    return *this;
  }

  hash_map_const_iterator operator++(int) {
    hash_map_const_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  bool operator==(const hash_map_const_iterator& other) const {
    return i_ == other.i_;
  }

  bool operator!=(const hash_map_const_iterator& other) const {
    return i_ != other.i_;
  }

  template <class, class, class, class, class>
  friend class unordered_map;

  HashIterator get_iterator() const { return i_; }
};

// 2. unordered_map 主类（模板参数对齐 C++ 标准）
template <class Key, class T,
          class Hash = std::hash<Key>,          // 默认哈希函数（标准库）
          class KeyEqual = std::equal_to<Key>,  // 默认键相等性比较（标准库）
          class Allocator =
              std::allocator<std::pair<const Key, T>>  // 默认分配器（标准库）
          >
class unordered_map {
 public:
  // -------------------------- 类型别名（严格对齐 C++ 标准）--------------------------
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<const Key, T>;  // 键不可修改，故 first 为 const
  using hasher = Hash;
  using key_equal = KeyEqual;
  using allocator_type = Allocator;
  using size_type = typename std::allocator_traits<Allocator>::size_type;
  using difference_type =
      typename std::allocator_traits<Allocator>::difference_type;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer =
      typename std::allocator_traits<Allocator>::const_pointer;

 private:
  // -------------------------- 底层哈希表相关类型（适配 hash_table）--------------------------
  // （1）hash_table 存储的 "节点值类型"：封装 value_type，供适配器提取键
  struct hash_node_value {
    value_type data_;

    // 构造函数（支持拷贝、移动）
    hash_node_value(const value_type& val) : data_(val) {}
    hash_node_value(value_type&& val) : data_(std::move(val)) {}

    // 显式声明拷贝和移动构造函数（因为定义了赋值操作符）
    hash_node_value(const hash_node_value& other) : data_(other.data_) {}
    hash_node_value(hash_node_value&& other)
        : data_(std::move(other.data_)) {}

    // 支持 emplace：可变参数构造函数，用于直接构造 pair
    template <class... Args>
    hash_node_value(Args&&... args) : data_(std::forward<Args>(args)...) {}

    // 供适配器提取键的接口（hash_table 需要通过节点值获取 Key）
    const key_type& get_key() const noexcept { return data_.first; }

    // 提供获取 value_type 的接口（供迭代器适配器使用）
    value_type& get_value() noexcept { return data_; }
    const value_type& get_value() const noexcept { return data_; }

    // 支持赋值（因为 pair<const Key, T> 的赋值操作符被删除）
    hash_node_value& operator=(const hash_node_value& other) {
      // 通过修改非 const 引用来赋值（类似标准库的 ref() 方法）
      const_cast<key_type&>(data_.first) = other.data_.first;
      data_.second = other.data_.second;
      return *this;
    }

    hash_node_value& operator=(hash_node_value&& other) {
      const_cast<key_type&>(data_.first) =
          std::move(const_cast<key_type&>(other.data_.first));
      data_.second = std::move(other.data_.second);
      return *this;
    }
  };

  // （2）适配 hash_table 的哈希函数（由 unordered_map_hasher 实现）
  using hasher_adapter =
      unordered_map_hasher<key_type, hash_node_value, hasher, key_equal>;

  // （3）适配 hash_table 的键相等性比较（由 unordered_map_key_equal 实现）
  using key_equal_adapter =
      unordered_map_key_equal<key_type, hash_node_value, key_equal, hasher>;

  // （4）重新绑定分配器：将 value_type 的分配器，绑定为 hash_node_value 的分配器
  using node_allocator = typename std::allocator_traits<
      Allocator>::template rebind_alloc<hash_node_value>;

  // （5）底层哈希表的类型（使用我们自己的 hash_table 实现）
  using base_hash_table =
      mystl::hash_table<hash_node_value,    // 哈希表存储的节点值类型
                          hasher_adapter,     // 哈希函数适配器
                          key_equal_adapter,  // 键相等性比较适配器
                          node_allocator      // 节点分配器
                          >;

  // 底层哈希表实例（核心依赖）
  base_hash_table table_;

 public:
  // -------------------------- 迭代器类型（使用迭代器适配器，返回 value_type）--------------------------
  // 普通迭代器（前向迭代器，对齐标准）
  using iterator = hash_map_iterator<typename base_hash_table::iterator>;
  // const 迭代器
  using const_iterator =
      hash_map_const_iterator<typename base_hash_table::const_iterator>;
  // 桶内迭代器（遍历单个桶的元素）
  using local_iterator =
      hash_map_iterator<typename base_hash_table::local_iterator>;
  using const_local_iterator = hash_map_const_iterator<
      typename base_hash_table::const_local_iterator>;

  // -------------------------- 构造函数（覆盖标准核心接口）--------------------------
  // （1）默认构造
  unordered_map() noexcept(
      std::is_nothrow_default_constructible_v<hasher_adapter> &&
      std::is_nothrow_default_constructible_v<key_equal_adapter> &&
      std::is_nothrow_default_constructible_v<node_allocator>)
      : table_() {}

  // （2）指定桶数 + 哈希函数 + 比较函数 + 分配器
  explicit unordered_map(size_type bucket_count, const hasher& hf = hasher(),
                         const key_equal& ke = key_equal(),
                         const allocator_type& alloc = allocator_type())
      : table_(hf, ke, node_allocator(alloc)) {
    table_.rehash_unique(bucket_count);  // 初始化桶数
  }

  // （3）迭代器范围构造
  template <class InputIterator>
  unordered_map(InputIterator first, InputIterator last,
                size_type bucket_count = 0, const hasher& hf = hasher(),
                const key_equal& ke = key_equal(),
                const allocator_type& alloc = allocator_type())
      : table_(hf, ke, node_allocator(alloc)) {
    if (bucket_count > 0) {
      table_.rehash_unique(bucket_count);
    }
    insert(first, last);  // 插入迭代器范围内的元素
  }

  // （4）初始化列表构造
  unordered_map(std::initializer_list<value_type> il,
                size_type bucket_count = 0, const hasher& hf = hasher(),
                const key_equal& ke = key_equal(),
                const allocator_type& alloc = allocator_type())
      : unordered_map(il.begin(), il.end(), bucket_count, hf, ke, alloc) {}

  // （5）拷贝构造
  unordered_map(const unordered_map& other) : table_(other.table_) {
    // 重新设置桶数并插入元素（标准库的实现方式）
    table_.rehash_unique(other.bucket_count());
    insert(other.begin(), other.end());
  }

  // （6）移动构造
  unordered_map(unordered_map&& other) noexcept(
      std::is_nothrow_move_constructible_v<base_hash_table>)
      : table_(std::move(other.table_)) {}

  // （7）分配器扩展构造（拷贝 + 自定义分配器）
  unordered_map(const unordered_map& other, const allocator_type& alloc)
      : table_(other.table_, node_allocator(alloc)) {
    // 重新设置桶数并插入元素
    table_.rehash_unique(other.bucket_count());
    insert(other.begin(), other.end());
  }

  // （8）分配器扩展构造（移动 + 自定义分配器）
  unordered_map(unordered_map&& other, const allocator_type& alloc)
      : table_(std::move(other.table_), node_allocator(alloc)) {}

  // -------------------------- 析构函数（默认即可，依赖 hash_table 的析构）--------------------------
  ~unordered_map() = default;

  // -------------------------- 赋值运算符（覆盖标准核心接口）--------------------------
  // （1）拷贝赋值
  unordered_map& operator=(const unordered_map& other) {
    table_ = other.table_;
    return *this;
  }

  // （2）移动赋值
  unordered_map& operator=(unordered_map&& other) noexcept(
      std::is_nothrow_move_assignable_v<base_hash_table>) {
    table_ = std::move(other.table_);
    return *this;
  }

  // （3）初始化列表赋值
  unordered_map& operator=(std::initializer_list<value_type> il) {
    table_.clear();              // 先清空
    insert(il.begin(), il.end());  // 再插入新元素
    return *this;
  }

  // -------------------------- 核心访问接口（operator[]、at）--------------------------
  // （1）operator[]：访问或插入键值对（不存在则默认构造 mapped_type）
  mapped_type& operator[](const key_type& key) {
    // 使用 emplace_unique_key_args 来支持 piecewise_construct
    auto [base_it, inserted] = table_.emplace_unique_key_args(
        key, std::piecewise_construct, std::forward_as_tuple(key),
        std::forward_as_tuple());
    iterator it(base_it);
    return it->second;
  }

  mapped_type& operator[](key_type&& key) {
    auto [base_it, inserted] = table_.emplace_unique_key_args(
        key, std::piecewise_construct, std::forward_as_tuple(std::move(key)),
        std::forward_as_tuple());
    iterator it(base_it);
    return it->second;
  }

  // （2）at()：访问已存在的键值对（不存在则抛异常，对齐标准）
  mapped_type& at(const key_type& key) {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range("tinystl::unordered_map::at: key not found");
    }
    return it->second;
  }

  const mapped_type& at(const key_type& key) const {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range("tinystl::unordered_map::at: key not found");
    }
    return it->second;
  }

  // -------------------------- 插入接口（覆盖标准核心接口）--------------------------
  // （1）插入单个 value_type（返回 pair<iterator, bool>，bool 表示是否插入成功）
  std::pair<iterator, bool> insert(const value_type& val) {
    auto [base_it, inserted] = table_.insert_unique(val);
    return {iterator(base_it), inserted};
  }

  std::pair<iterator, bool> insert(value_type&& val) {
    auto [base_it, inserted] = table_.insert_unique(std::move(val));
    return {iterator(base_it), inserted};
  }

  // （2）插入迭代器范围
  template <class InputIterator>
  void insert(InputIterator first, InputIterator last) {
    for (; first != last; ++first) {
      table_.insert_unique(*first);
    }
  }

  // （3）插入初始化列表
  void insert(std::initializer_list<value_type> il) {
    insert(il.begin(), il.end());
  }

  // （4）emplace：直接构造 value_type（避免额外拷贝，C++11+）
  template <class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    // 完美转发参数，在哈希表中直接构造 hash_node_value（内部包含 value_type）
    auto [base_it, inserted] =
        table_.emplace_unique(std::forward<Args>(args)...);
    return {iterator(base_it), inserted};
  }

  // -------------------------- 查找与计数接口（覆盖标准核心接口）--------------------------
  // （1）find()：按键查找（返回迭代器）
  iterator find(const key_type& key) { return iterator(table_.find(key)); }

  const_iterator find(const key_type& key) const {
    return const_iterator(table_.find(key));
  }

  // （2）count()：统计键的个数（unordered_map 中只能是 0 或 1）
  size_type count(const key_type& key) const {
    return table_.count_unique(key);
  }

  // （3）contains()：判断键是否存在（C++20+，可选实现）
  bool contains(const key_type& key) const { return find(key) != end(); }

  // -------------------------- 删除接口（覆盖标准核心接口）--------------------------
  // （1）按迭代器删除
  iterator erase(iterator pos) {
    auto base_it = table_.erase(pos.get_iterator());
    return iterator(base_it);
  }

  // 支持 const_iterator 版本（标准库兼容）
  iterator erase(const_iterator pos) {
    auto base_it = table_.erase(pos.get_iterator());
    return iterator(base_it);
  }

  // （2）按键删除（返回删除的元素个数，0 或 1）
  size_type erase(const key_type& key) { return table_.erase_unique(key); }

  // （3）按迭代器范围删除
  iterator erase(iterator first, iterator last) {
    auto base_it =
        table_.erase(first.get_iterator(), last.get_iterator());
    return iterator(base_it);
  }

  // 支持 const_iterator 版本的范围删除
  iterator erase(const_iterator first, const_iterator last) {
    auto base_it =
        table_.erase(first.get_iterator(), last.get_iterator());
    return iterator(base_it);
  }

  // （4）清空容器
  void clear() noexcept { table_.clear(); }

  // -------------------------- 迭代器接口（覆盖标准核心接口）--------------------------
  iterator begin() noexcept { return iterator(table_.begin()); }
  iterator end() noexcept { return iterator(table_.end()); }
  const_iterator begin() const noexcept {
    return const_iterator(table_.begin());
  }
  const_iterator end() const noexcept { return const_iterator(table_.end()); }
  const_iterator cbegin() const noexcept {
    return const_iterator(table_.begin());
  }
  const_iterator cend() const noexcept {
    return const_iterator(table_.end());
  }

  // -------------------------- 桶相关接口（覆盖标准核心接口）--------------------------
  // （1）获取当前桶数
  size_type bucket_count() const noexcept { return table_.bucket_count(); }

  // （2）获取键对应的桶索引
  size_type bucket(const key_type& key) const { return table_.bucket(key); }

  // （3）获取指定桶的大小（元素个数）
  size_type bucket_size(size_type n) const { return table_.bucket_size(n); }

  // （4）桶内迭代器（遍历单个桶）
  local_iterator begin(size_type n) {
    return local_iterator(table_.begin(n));
  }
  local_iterator end(size_type n) { return local_iterator(table_.end(n)); }
  const_local_iterator begin(size_type n) const {
    return const_local_iterator(table_.cbegin(n));
  }
  const_local_iterator end(size_type n) const {
    return const_local_iterator(table_.cend(n));
  }

  // -------------------------- 负载因子与扩容接口（覆盖标准核心接口）--------------------------
  // （1）当前负载因子（元素个数 / 桶数）
  float load_factor() const noexcept { return table_.load_factor(); }

  // （2）获取/设置最大负载因子（超过则触发扩容）
  float max_load_factor() const noexcept { return table_.max_load_factor(); }

  void max_load_factor(float mlf) { table_.max_load_factor(mlf); }

  // （3）主动扩容桶数（确保桶数 >= n）
  void rehash(size_type n) { table_.rehash_unique(n); }

  // （4）预留元素空间（确保能容纳 n 个元素而不扩容）
  void reserve(size_type n) { table_.reserve_unique(n); }

  // -------------------------- 其他辅助接口 --------------------------
  // （1）获取分配器
  allocator_type get_allocator() const noexcept {
    return allocator_type(table_.node_alloc());
  }

  // （2）判断容器是否为空
  bool empty() const noexcept { return table_.size() == 0; }

  // （3）获取元素个数
  size_type size() const noexcept { return table_.size(); }

  // （4）获取最大可容纳元素个数
  size_type max_size() const noexcept { return table_.max_size(); }

  // （5）交换两个容器
  void swap(unordered_map& other) noexcept(
      std::is_nothrow_swappable_v<base_hash_table>) {
    table_.swap(other.table_);
  }

  // （6）获取哈希函数和键比较函数
  hasher hash_function() const {
    return table_.hash_function().hash_function();
  }

  key_equal key_eq() const { return table_.key_eq().key_eq(); }
};

// -------------------------- 哈希函数适配器实现（unordered_map_hasher）--------------------------
// 作用：将标准库的 hasher（针对 Key）适配为 hash_table 所需的 hasher（针对 hash_node_value）
template <class Key, class ValueType, class Hash, class KeyEqual>
class unordered_map_hasher {
 private:
  Hash hash_;  // 存储用户传入的哈希函数

 public:
  // 构造函数（默认、拷贝）
  unordered_map_hasher() noexcept(
      std::is_nothrow_default_constructible_v<Hash>)
      : hash_() {}

  unordered_map_hasher(const Hash& hf) noexcept(
      std::is_nothrow_copy_constructible_v<Hash>)
      : hash_(hf) {}

  // 核心：计算 hash_node_value 的哈希值（提取 Key 后调用 hash_）
  size_t operator()(const ValueType& val) const {
    return hash_(val.get_key());
  }

  // 核心：计算 Key 的哈希值（供 hash_table 查找时使用）
  size_t operator()(const Key& key) const { return hash_(key); }

  // 暴露原始哈希函数（供 unordered_map::hash_function() 使用）
  const Hash& hash_function() const noexcept { return hash_; }

  // 交换两个适配器
  void swap(unordered_map_hasher& other) noexcept(
      std::is_nothrow_swappable_v<Hash>) {
    using std::swap;
    swap(hash_, other.hash_);
  }
};

// -------------------------- 键相等性比较适配器实现（unordered_map_key_equal）--------------------------
// 作用：将标准库的 key_equal（针对 Key）适配为 hash_table 所需的比较函数（针对 hash_node_value）
template <class Key, class ValueType, class KeyEqual, class Hash>
class unordered_map_key_equal {
 private:
  KeyEqual key_eq_;  // 存储用户传入的键比较函数

 public:
  // 构造函数（默认、拷贝）
  unordered_map_key_equal() noexcept(
      std::is_nothrow_default_constructible_v<KeyEqual>)
      : key_eq_() {}

  unordered_map_key_equal(const KeyEqual& ke) noexcept(
      std::is_nothrow_copy_constructible_v<KeyEqual>)
      : key_eq_(ke) {}

  // 核心：比较两个 hash_node_value 的键是否相等
  bool operator()(const ValueType& lhs, const ValueType& rhs) const {
    return key_eq_(lhs.get_key(), rhs.get_key());
  }

  // 核心：比较 hash_node_value 的键与指定 Key 是否相等
  bool operator()(const ValueType& val, const Key& key) const {
    return key_eq_(val.get_key(), key);
  }

  bool operator()(const Key& key, const ValueType& val) const {
    return key_eq_(key, val.get_key());
  }

  // 暴露原始键比较函数（供 unordered_map::key_eq() 使用）
  const KeyEqual& key_eq() const noexcept { return key_eq_; }

  // 交换两个适配器
  void swap(unordered_map_key_equal& other) noexcept(
      std::is_nothrow_swappable_v<KeyEqual>) {
    using std::swap;
    swap(key_eq_, other.key_eq_);
  }
};

// -------------------------- 非成员函数（覆盖标准核心接口）--------------------------
// （1）交换两个 unordered_map
template <class Key, class T, class Hash, class KeyEqual, class Allocator>
void swap(unordered_map<Key, T, Hash, KeyEqual, Allocator>& lhs,
          unordered_map<Key, T, Hash, KeyEqual, Allocator>&
              rhs) noexcept(noexcept(lhs.swap(rhs))) {
  lhs.swap(rhs);
}

// （2）相等性比较（==、!=）
template <class Key, class T, class Hash, class KeyEqual, class Allocator>
bool operator==(const unordered_map<Key, T, Hash, KeyEqual, Allocator>& lhs,
                const unordered_map<Key, T, Hash, KeyEqual, Allocator>& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  // 遍历 lhs 的每个元素，检查 rhs 是否存在相同的键值对
  for (const auto& elem : lhs) {
    auto it = rhs.find(elem.first);
    if (it == rhs.end() || it->second != elem.second) {
      return false;
    }
  }
  return true;
}

template <class Key, class T, class Hash, class KeyEqual, class Allocator>
bool operator!=(const unordered_map<Key, T, Hash, KeyEqual, Allocator>& lhs,
                const unordered_map<Key, T, Hash, KeyEqual, Allocator>& rhs) {
  return !(lhs == rhs);
}

}  // namespace mystl

#endif  // TINYSTL_UNORDERED_MAP_H_