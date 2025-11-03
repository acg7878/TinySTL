#ifndef TINYSTL_UNORDERED_MAP_H_
#define TINYSTL_UNORDERED_MAP_H_

// 引入我们自己的 __hash_table 实现
#include <mystl/__hash_table.h>
// 引入必要的辅助头文件（键值对、哈希函数、相等性比较、分配器等）
#include <utility>      // for pair, move, forward
#include <tuple>        // for piecewise_construct, forward_as_tuple
#include <functional>   // for hash, equal_to
#include <memory>       // for allocator, addressof
#include <iterator>     // for forward_iterator_tag
#include <type_traits>  // for remove_reference_t, declval
#include <initializer_list>  // for initializer_list
#include <stdexcept>    // for out_of_range
#include <cstddef>      // for size_t, ptrdiff_t


namespace mystl {

// 1. 前置声明：哈希表适配器（适配 __hash_table 所需的哈希函数、相等性比较）
// 作用：将 unordered_map 的 Key 哈希/比较，适配为 __hash_table 对 "键值对节点" 的操作
template <class Key, class ValueType, class Hash, class KeyEqual>
class __unordered_map_hasher;

template <class Key, class ValueType, class Hash, class KeyEqual>
class __unordered_map_key_equal;

// 2. 迭代器适配器（将 __hash_table 迭代器转换为返回 value_type 的迭代器）
// 参考标准库的 __hash_map_iterator 实现
template <class HashIterator>
class __hash_map_iterator {
private:
    HashIterator __i_;

    // 从 __get_value() 的返回类型提取 value_type
    using __node_value_type = typename HashIterator::value_type;
    using value_type = std::remove_reference_t<decltype(std::declval<__node_value_type>().__get_value())>;

public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = typename HashIterator::difference_type;
    using reference = value_type&;
    using pointer = value_type*;

    __hash_map_iterator() noexcept = default;

    explicit __hash_map_iterator(HashIterator __i) noexcept : __i_(__i) {}

    reference operator*() const {
        return __i_->__get_value();
    }

    pointer operator->() const {
        return std::addressof(__i_->__get_value());
    }

    __hash_map_iterator& operator++() {
        ++__i_;
        return *this;
    }

    __hash_map_iterator operator++(int) {
        __hash_map_iterator __tmp(*this);
        ++(*this);
        return __tmp;
    }

    bool operator==(const __hash_map_iterator& other) const {
        return __i_ == other.__i_;
    }

    bool operator!=(const __hash_map_iterator& other) const {
        return __i_ != other.__i_;
    }

    // 允许 unordered_map 访问底层迭代器
    template <class, class, class, class, class>
    friend class unordered_map;

    // 获取底层迭代器（供 erase 等方法使用）
    HashIterator __get_iterator() const { return __i_; }
};

// const 迭代器适配器
template <class HashIterator>
class __hash_map_const_iterator {
private:
    HashIterator __i_;

    // 从 __get_value() 的返回类型提取 value_type
    using __node_value_type = typename HashIterator::value_type;
    using value_type = std::remove_reference_t<decltype(std::declval<const __node_value_type>().__get_value())>;

public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = typename HashIterator::difference_type;
    using reference = const value_type&;
    using pointer = const value_type*;

    __hash_map_const_iterator() noexcept = default;

    explicit __hash_map_const_iterator(HashIterator __i) noexcept : __i_(__i) {}

    // 从非 const 迭代器构造（如果底层迭代器支持）
    template <class NonConstIterator>
    __hash_map_const_iterator(__hash_map_iterator<NonConstIterator> __i) noexcept
        : __i_(__i.__get_iterator()) {}

    reference operator*() const {
        return __i_->__get_value();
    }

    pointer operator->() const {
        return std::addressof(__i_->__get_value());
    }

    __hash_map_const_iterator& operator++() {
        ++__i_;
        return *this;
    }

    __hash_map_const_iterator operator++(int) {
        __hash_map_const_iterator __tmp(*this);
        ++(*this);
        return __tmp;
    }

    bool operator==(const __hash_map_const_iterator& other) const {
        return __i_ == other.__i_;
    }

    bool operator!=(const __hash_map_const_iterator& other) const {
        return __i_ != other.__i_;
    }

    template <class, class, class, class, class>
    friend class unordered_map;

    HashIterator __get_iterator() const { return __i_; }
};

// 2. unordered_map 主类（模板参数对齐 C++ 标准）
template <
    class Key,
    class T,
    class Hash = std::hash<Key>,          // 默认哈希函数（标准库）
    class KeyEqual = std::equal_to<Key>,  // 默认键相等性比较（标准库）
    class Allocator = std::allocator<std::pair<const Key, T>>  // 默认分配器（标准库）
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
    using difference_type = typename std::allocator_traits<Allocator>::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

private:
    // -------------------------- 底层哈希表相关类型（适配 __hash_table）--------------------------
    // （1）__hash_table 存储的 "节点值类型"：封装 value_type，供适配器提取键
    struct __hash_node_value {
        value_type __data;
        
        // 构造函数（支持拷贝、移动）
        __hash_node_value(const value_type& val) : __data(val) {}
        __hash_node_value(value_type&& val) : __data(std::move(val)) {}
        
        // 显式声明拷贝和移动构造函数（因为定义了赋值操作符）
        __hash_node_value(const __hash_node_value& other) : __data(other.__data) {}
        __hash_node_value(__hash_node_value&& other) : __data(std::move(other.__data)) {}
        
        // 支持 emplace：可变参数构造函数，用于直接构造 pair
        template <class... Args>
        __hash_node_value(Args&&... args) : __data(std::forward<Args>(args)...) {}
        
        // 供适配器提取键的接口（__hash_table 需要通过节点值获取 Key）
        const key_type& __get_key() const noexcept { return __data.first; }
        
        // 提供获取 value_type 的接口（供迭代器适配器使用）
        value_type& __get_value() noexcept { return __data; }
        const value_type& __get_value() const noexcept { return __data; }
        
        // 支持赋值（因为 pair<const Key, T> 的赋值操作符被删除）
        __hash_node_value& operator=(const __hash_node_value& other) {
            // 通过修改非 const 引用来赋值（类似标准库的 __ref() 方法）
            const_cast<key_type&>(__data.first) = other.__data.first;
            __data.second = other.__data.second;
            return *this;
        }
        
        __hash_node_value& operator=(__hash_node_value&& other) {
            const_cast<key_type&>(__data.first) = std::move(const_cast<key_type&>(other.__data.first));
            __data.second = std::move(other.__data.second);
            return *this;
        }
    };

    // （2）适配 __hash_table 的哈希函数（由 __unordered_map_hasher 实现）
    using __hasher_adapter = __unordered_map_hasher<
        key_type,
        __hash_node_value,
        hasher,
        key_equal
    >;

    // （3）适配 __hash_table 的键相等性比较（由 __unordered_map_key_equal 实现）
    using __key_equal_adapter = __unordered_map_key_equal<
        key_type,
        __hash_node_value,
        key_equal,
        hasher
    >;

    // （4）重新绑定分配器：将 value_type 的分配器，绑定为 __hash_node_value 的分配器
    using __node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<__hash_node_value>;

    // （5）底层哈希表的类型（使用我们自己的 __hash_table 实现）
    using __base_hash_table = mystl::__hash_table<
        __hash_node_value,        // 哈希表存储的节点值类型
        __hasher_adapter,         // 哈希函数适配器
        __key_equal_adapter,      // 键相等性比较适配器
        __node_allocator          // 节点分配器
    >;

    // 底层哈希表实例（核心依赖）
    __base_hash_table __table_;

public:
    // -------------------------- 迭代器类型（使用迭代器适配器，返回 value_type）--------------------------
    // 普通迭代器（前向迭代器，对齐标准）
    using iterator = __hash_map_iterator<typename __base_hash_table::iterator>;
    // const 迭代器
    using const_iterator = __hash_map_const_iterator<typename __base_hash_table::const_iterator>;
    // 桶内迭代器（遍历单个桶的元素）
    using local_iterator = __hash_map_iterator<typename __base_hash_table::local_iterator>;
    using const_local_iterator = __hash_map_const_iterator<typename __base_hash_table::const_local_iterator>;

    // -------------------------- 构造函数（覆盖标准核心接口）--------------------------
    // （1）默认构造
    unordered_map() noexcept(
        std::is_nothrow_default_constructible_v<__hasher_adapter> &&
        std::is_nothrow_default_constructible_v<__key_equal_adapter> &&
        std::is_nothrow_default_constructible_v<__node_allocator>
    ) : __table_() {}

    // （2）指定桶数 + 哈希函数 + 比较函数 + 分配器
    explicit unordered_map(size_type bucket_count,
                          const hasher& hf = hasher(),
                          const key_equal& ke = key_equal(),
                          const allocator_type& alloc = allocator_type())
        : __table_(hf, ke, __node_allocator(alloc)) {
        __table_.__rehash_unique(bucket_count);  // 初始化桶数
    }

    // （3）迭代器范围构造
    template <class InputIterator>
    unordered_map(InputIterator first, InputIterator last,
                  size_type bucket_count = 0,
                  const hasher& hf = hasher(),
                  const key_equal& ke = key_equal(),
                  const allocator_type& alloc = allocator_type())
        : __table_(hf, ke, __node_allocator(alloc)) {
        if (bucket_count > 0) {
            __table_.__rehash_unique(bucket_count);
        }
        insert(first, last);  // 插入迭代器范围内的元素
    }

    // （4）初始化列表构造
    unordered_map(std::initializer_list<value_type> il,
                  size_type bucket_count = 0,
                  const hasher& hf = hasher(),
                  const key_equal& ke = key_equal(),
                  const allocator_type& alloc = allocator_type())
        : unordered_map(il.begin(), il.end(), bucket_count, hf, ke, alloc) {}

    // （5）拷贝构造
    unordered_map(const unordered_map& other)
        : __table_(other.__table_) {
        // 重新设置桶数并插入元素（标准库的实现方式）
        __table_.__rehash_unique(other.bucket_count());
        insert(other.begin(), other.end());
    }

    // （6）移动构造
    unordered_map(unordered_map&& other) noexcept(
        std::is_nothrow_move_constructible_v<__base_hash_table>
    ) : __table_(std::move(other.__table_)) {}

    // （7）分配器扩展构造（拷贝 + 自定义分配器）
    unordered_map(const unordered_map& other, const allocator_type& alloc)
        : __table_(other.__table_, __node_allocator(alloc)) {
        // 重新设置桶数并插入元素
        __table_.__rehash_unique(other.bucket_count());
        insert(other.begin(), other.end());
    }

    // （8）分配器扩展构造（移动 + 自定义分配器）
    unordered_map(unordered_map&& other, const allocator_type& alloc)
        : __table_(std::move(other.__table_), __node_allocator(alloc)) {}

    // -------------------------- 析构函数（默认即可，依赖 __hash_table 的析构）--------------------------
    ~unordered_map() = default;

    // -------------------------- 赋值运算符（覆盖标准核心接口）--------------------------
    // （1）拷贝赋值
    unordered_map& operator=(const unordered_map& other) {
        __table_ = other.__table_;
        return *this;
    }

    // （2）移动赋值
    unordered_map& operator=(unordered_map&& other) noexcept(
        std::is_nothrow_move_assignable_v<__base_hash_table>
    ) {
        __table_ = std::move(other.__table_);
        return *this;
    }

    // （3）初始化列表赋值
    unordered_map& operator=(std::initializer_list<value_type> il) {
        __table_.clear();  // 先清空
        insert(il.begin(), il.end());  // 再插入新元素
        return *this;
    }

    // -------------------------- 核心访问接口（operator[]、at）--------------------------
    // （1）operator[]：访问或插入键值对（不存在则默认构造 mapped_type）
    mapped_type& operator[](const key_type& key) {
        // 使用 __emplace_unique_key_args 来支持 piecewise_construct
        auto [base_it, inserted] = __table_.__emplace_unique_key_args(
            key, std::piecewise_construct,
            std::forward_as_tuple(key),
            std::forward_as_tuple());
        iterator it(base_it);
        return it->second;
    }

    mapped_type& operator[](key_type&& key) {
        auto [base_it, inserted] = __table_.__emplace_unique_key_args(
            key, std::piecewise_construct,
            std::forward_as_tuple(std::move(key)),
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
        auto [base_it, inserted] = __table_.__insert_unique(val);
        return {iterator(base_it), inserted};
    }

    std::pair<iterator, bool> insert(value_type&& val) {
        auto [base_it, inserted] = __table_.__insert_unique(std::move(val));
        return {iterator(base_it), inserted};
    }

    // （2）插入迭代器范围
    template <class InputIterator>
    void insert(InputIterator first, InputIterator last) {
        for (; first != last; ++first) {
            __table_.__insert_unique(*first);
        }
    }

    // （3）插入初始化列表
    void insert(std::initializer_list<value_type> il) {
        insert(il.begin(), il.end());
    }

    // （4）emplace：直接构造 value_type（避免额外拷贝，C++11+）
    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        // 完美转发参数，在哈希表中直接构造 __hash_node_value（内部包含 value_type）
        auto [base_it, inserted] = __table_.__emplace_unique(std::forward<Args>(args)...);
        return {iterator(base_it), inserted};
    }

    // -------------------------- 查找与计数接口（覆盖标准核心接口）--------------------------
    // （1）find()：按键查找（返回迭代器）
    iterator find(const key_type& key) {
        return iterator(__table_.find(key));
    }

    const_iterator find(const key_type& key) const {
        return const_iterator(__table_.find(key));
    }

    // （2）count()：统计键的个数（unordered_map 中只能是 0 或 1）
    size_type count(const key_type& key) const {
        return __table_.__count_unique(key);
    }

    // （3）contains()：判断键是否存在（C++20+，可选实现）
    bool contains(const key_type& key) const {
        return find(key) != end();
    }

    // -------------------------- 删除接口（覆盖标准核心接口）--------------------------
    // （1）按迭代器删除
    iterator erase(iterator pos) {
        auto base_it = __table_.erase(pos.__get_iterator());
        return iterator(base_it);
    }

    // 支持 const_iterator 版本（标准库兼容）
    iterator erase(const_iterator pos) {
        auto base_it = __table_.erase(pos.__get_iterator());
        return iterator(base_it);
    }

    // （2）按键删除（返回删除的元素个数，0 或 1）
    size_type erase(const key_type& key) {
        return __table_.__erase_unique(key);
    }

    // （3）按迭代器范围删除
    iterator erase(iterator first, iterator last) {
        auto base_it = __table_.erase(first.__get_iterator(), last.__get_iterator());
        return iterator(base_it);
    }

    // 支持 const_iterator 版本的范围删除
    iterator erase(const_iterator first, const_iterator last) {
        auto base_it = __table_.erase(first.__get_iterator(), last.__get_iterator());
        return iterator(base_it);
    }

    // （4）清空容器
    void clear() noexcept {
        __table_.clear();
    }

    // -------------------------- 迭代器接口（覆盖标准核心接口）--------------------------
    iterator begin() noexcept { return iterator(__table_.begin()); }
    iterator end() noexcept { return iterator(__table_.end()); }
    const_iterator begin() const noexcept { return const_iterator(__table_.begin()); }
    const_iterator end() const noexcept { return const_iterator(__table_.end()); }
    const_iterator cbegin() const noexcept { return const_iterator(__table_.begin()); }
    const_iterator cend() const noexcept { return const_iterator(__table_.end()); }

    // -------------------------- 桶相关接口（覆盖标准核心接口）--------------------------
    // （1）获取当前桶数
    size_type bucket_count() const noexcept {
        return __table_.bucket_count();
    }

    // （2）获取键对应的桶索引
    size_type bucket(const key_type& key) const {
        return __table_.bucket(key);
    }

    // （3）获取指定桶的大小（元素个数）
    size_type bucket_size(size_type n) const {
        return __table_.bucket_size(n);
    }

    // （4）桶内迭代器（遍历单个桶）
    local_iterator begin(size_type n) { return local_iterator(__table_.begin(n)); }
    local_iterator end(size_type n) { return local_iterator(__table_.end(n)); }
    const_local_iterator begin(size_type n) const { return const_local_iterator(__table_.cbegin(n)); }
    const_local_iterator end(size_type n) const { return const_local_iterator(__table_.cend(n)); }

    // -------------------------- 负载因子与扩容接口（覆盖标准核心接口）--------------------------
    // （1）当前负载因子（元素个数 / 桶数）
    float load_factor() const noexcept {
        return __table_.load_factor();
    }

    // （2）获取/设置最大负载因子（超过则触发扩容）
    float max_load_factor() const noexcept {
        return __table_.max_load_factor();
    }

    void max_load_factor(float mlf) {
        __table_.max_load_factor(mlf);
    }

    // （3）主动扩容桶数（确保桶数 >= n）
    void rehash(size_type n) {
        __table_.__rehash_unique(n);
    }

    // （4）预留元素空间（确保能容纳 n 个元素而不扩容）
    void reserve(size_type n) {
        __table_.__reserve_unique(n);
    }

    // -------------------------- 其他辅助接口 --------------------------
    // （1）获取分配器
    allocator_type get_allocator() const noexcept {
        return allocator_type(__table_.__node_alloc());
    }

    // （2）判断容器是否为空
    bool empty() const noexcept {
        return __table_.size() == 0;
    }

    // （3）获取元素个数
    size_type size() const noexcept {
        return __table_.size();
    }

    // （4）获取最大可容纳元素个数
    size_type max_size() const noexcept {
        return __table_.max_size();
    }

    // （5）交换两个容器
    void swap(unordered_map& other) noexcept(
        std::is_nothrow_swappable_v<__base_hash_table>
    ) {
        __table_.swap(other.__table_);
    }

    // （6）获取哈希函数和键比较函数
    hasher hash_function() const {
        return __table_.hash_function().hash_function();
    }

    key_equal key_eq() const {
        return __table_.key_eq().key_eq();
    }
};

// -------------------------- 哈希函数适配器实现（__unordered_map_hasher）--------------------------
// 作用：将标准库的 hasher（针对 Key）适配为 __hash_table 所需的 hasher（针对 __hash_node_value）
template <class Key, class ValueType, class Hash, class KeyEqual>
class __unordered_map_hasher {
private:
    Hash __hash_;  // 存储用户传入的哈希函数

public:
    // 构造函数（默认、拷贝）
    __unordered_map_hasher() noexcept(std::is_nothrow_default_constructible_v<Hash>)
        : __hash_() {}

    __unordered_map_hasher(const Hash& hf) noexcept(std::is_nothrow_copy_constructible_v<Hash>)
        : __hash_(hf) {}

    // 核心：计算 __hash_node_value 的哈希值（提取 Key 后调用 __hash_）
    size_t operator()(const ValueType& val) const {
        return __hash_(val.__get_key());
    }

    // 核心：计算 Key 的哈希值（供 __hash_table 查找时使用）
    size_t operator()(const Key& key) const {
        return __hash_(key);
    }

    // 暴露原始哈希函数（供 unordered_map::hash_function() 使用）
    const Hash& hash_function() const noexcept {
        return __hash_;
    }

    // 交换两个适配器
    void swap(__unordered_map_hasher& other) noexcept(std::is_nothrow_swappable_v<Hash>) {
        using std::swap;
        swap(__hash_, other.__hash_);
    }
};

// -------------------------- 键相等性比较适配器实现（__unordered_map_key_equal）--------------------------
// 作用：将标准库的 key_equal（针对 Key）适配为 __hash_table 所需的比较函数（针对 __hash_node_value）
template <class Key, class ValueType, class KeyEqual, class Hash>
class __unordered_map_key_equal {
private:
    KeyEqual __key_eq_;  // 存储用户传入的键比较函数

public:
    // 构造函数（默认、拷贝）
    __unordered_map_key_equal() noexcept(std::is_nothrow_default_constructible_v<KeyEqual>)
        : __key_eq_() {}

    __unordered_map_key_equal(const KeyEqual& ke) noexcept(std::is_nothrow_copy_constructible_v<KeyEqual>)
        : __key_eq_(ke) {}

    // 核心：比较两个 __hash_node_value 的键是否相等
    bool operator()(const ValueType& lhs, const ValueType& rhs) const {
        return __key_eq_(lhs.__get_key(), rhs.__get_key());
    }

    // 核心：比较 __hash_node_value 的键与指定 Key 是否相等
    bool operator()(const ValueType& val, const Key& key) const {
        return __key_eq_(val.__get_key(), key);
    }

    bool operator()(const Key& key, const ValueType& val) const {
        return __key_eq_(key, val.__get_key());
    }

    // 暴露原始键比较函数（供 unordered_map::key_eq() 使用）
    const KeyEqual& key_eq() const noexcept {
        return __key_eq_;
    }

    // 交换两个适配器
    void swap(__unordered_map_key_equal& other) noexcept(std::is_nothrow_swappable_v<KeyEqual>) {
        using std::swap;
        swap(__key_eq_, other.__key_eq_);
    }
};

// -------------------------- 非成员函数（覆盖标准核心接口）--------------------------
// （1）交换两个 unordered_map
template <class Key, class T, class Hash, class KeyEqual, class Allocator>
void swap(unordered_map<Key, T, Hash, KeyEqual, Allocator>& lhs,
          unordered_map<Key, T, Hash, KeyEqual, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
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

}  // namespace tinystl

#endif  // TINYSTL_UNORDERED_MAP_H_