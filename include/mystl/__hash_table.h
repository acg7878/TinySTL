#ifndef TINYSTL___HASH_TABLE_H_
#define TINYSTL___HASH_TABLE_H_

#include <cstddef>          // for size_t, ptrdiff_t
#include <limits>           // for numeric_limits
#include <memory>           // for allocator_traits, unique_ptr, addressof, pointer_traits
#include <iterator>         // for forward_iterator_tag
#include <utility>          // for pair, move, forward
#include <algorithm>        // for max, min
#include <cmath>            // for ceil

namespace mystl {

// ============================================================================
// 前向声明
// ============================================================================
template <class _NodePtr>
class __hash_iterator;
template <class _ConstNodePtr>
class __hash_const_iterator;
template <class _NodePtr>
class __hash_local_iterator;
template <class _ConstNodePtr>
class __hash_const_local_iterator;

template <class _Tp, class _Hash, class _Equal, class _Alloc>
class __hash_table;

// ============================================================================
// 辅助函数：计算下一个质数（简化版本）
// ============================================================================
inline size_t __next_prime(size_t n) {
    static const size_t __small_primes[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71
    };
    
    // 小质数直接查找
    for (size_t prime : __small_primes) {
        if (prime >= n) {
            return prime;
        }
    }
    
    // 简单查找下一个质数（从 n 开始）
    for (size_t i = (n % 2 == 0 ? n + 1 : n); ; i += 2) {
        bool is_prime = true;
        for (size_t j = 3; j * j <= i; j += 2) {
            if (i % j == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            return i;
        }
    }
}

// ============================================================================
// 辅助函数：判断是否为 2 的幂
// ============================================================================
inline bool __is_hash_power2(size_t n) {
    return n > 2 && (n & (n - 1)) == 0;
}

// ============================================================================
// 辅助函数：将哈希值约束到桶索引
// ============================================================================
inline size_t __constrain_hash(size_t h, size_t bc) {
    // 如果桶数是 2 的幂，使用位运算（更快）
    if (__is_hash_power2(bc)) {
        return h & (bc - 1);
    }
    // 否则使用模运算
    return h < bc ? h : h % bc;
}

// ============================================================================
// 辅助函数：计算下一个 2 的幂
// ============================================================================
inline size_t __next_hash_pow2(size_t n) {
    if (n < 2) return n;
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    if (sizeof(size_t) > 4) {
        n |= n >> 32;
    }
    return n + 1;
}

// ============================================================================
// 节点基类
// ============================================================================
template <class _NodePtr>
struct __hash_node_base {
    using __node_type = typename std::pointer_traits<_NodePtr>::element_type;
    using __node_base_pointer = typename std::pointer_traits<_NodePtr>::template rebind<__hash_node_base>;
    using __next_pointer = __node_base_pointer;
    
    __next_pointer __next_;
    
    __hash_node_base() noexcept : __next_(nullptr) {}
    explicit __hash_node_base(__next_pointer __next) noexcept : __next_(__next) {}
    
    __next_pointer __ptr() noexcept {
        return std::pointer_traits<__node_base_pointer>::pointer_to(*this);
    }
    
    _NodePtr __upcast() noexcept {
        return std::pointer_traits<_NodePtr>::pointer_to(
            static_cast<__node_type&>(*this));
    }
    
    size_t __hash() const noexcept {
        return static_cast<__node_type const&>(*this).__hash_;
    }
};

// ============================================================================
// 完整节点类
// ============================================================================
template <class _Tp, class _VoidPtr>
struct __hash_node : public __hash_node_base<
    typename std::pointer_traits<_VoidPtr>::template rebind<__hash_node<_Tp, _VoidPtr>>
> {
    using __node_value_type = _Tp;
    using _Base = __hash_node_base<
        typename std::pointer_traits<_VoidPtr>::template rebind<__hash_node<_Tp, _VoidPtr>>
    >;
    using __next_pointer = typename _Base::__next_pointer;
    
    size_t __hash_;
    
private:
    union {
        _Tp __value_;
    };
    
public:
    explicit __hash_node(__next_pointer __next, size_t __hash)
        : _Base(__next), __hash_(__hash) {}
    
    ~__hash_node() {}
    
    _Tp& __get_value() { return __value_; }
    const _Tp& __get_value() const { return __value_; }
};

// ============================================================================
// 类型萃取：区分 set 和 map
// ============================================================================
template <class _Tp>
struct __hash_key_value_types {
    using key_type = _Tp;
    using __node_value_type = _Tp;
    using __container_value_type = _Tp;
    static const bool __is_map = false;
    
    static const key_type& __get_key(const _Tp& v) { return v; }
    static const __container_value_type& __get_value(const __node_value_type& v) {
        return v;
    }
};

// ============================================================================
// 节点类型萃取
// ============================================================================
template <class _NodePtr>
struct __hash_node_types {
    using difference_type = ptrdiff_t;
    using size_type = size_t;
    using __node_type = typename std::pointer_traits<_NodePtr>::element_type;
    using __node_pointer = _NodePtr;
    using __node_base_type = __hash_node_base<__node_pointer>;
    using __node_base_pointer = typename std::pointer_traits<_NodePtr>::template rebind<__node_base_type>;
    using __next_pointer = typename __node_base_type::__next_pointer;
    using __node_value_type = typename __node_type::__node_value_type;
};

// ============================================================================
// 迭代器：从迭代器类型萃取节点类型
// ============================================================================
template <class _HashIterator>
struct __hash_node_types_from_iterator;
template <class _NodePtr>
struct __hash_node_types_from_iterator<__hash_iterator<_NodePtr>>
    : __hash_node_types<_NodePtr> {};
template <class _NodePtr>
struct __hash_node_types_from_iterator<__hash_const_iterator<_NodePtr>>
    : __hash_node_types<_NodePtr> {};

// ============================================================================
// 全局迭代器
// ============================================================================
template <class _NodePtr>
class __hash_iterator {
    using _NodeTypes = __hash_node_types<_NodePtr>;
    using __node_pointer = _NodePtr;
    using __next_pointer = typename _NodeTypes::__next_pointer;
    
    __next_pointer __node_;
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename _NodeTypes::__node_value_type;
    using difference_type = typename _NodeTypes::difference_type;
    using reference = value_type&;
    using pointer = value_type*;
    
    __hash_iterator() noexcept : __node_(nullptr) {}
    
    explicit __hash_iterator(__next_pointer __node) noexcept : __node_(__node) {}
    
    reference operator*() const {
        return __node_->__upcast()->__get_value();
    }
    
    pointer operator->() const {
        return std::addressof(__node_->__upcast()->__get_value());
    }
    
    __hash_iterator& operator++() {
        __node_ = __node_->__next_;
        return *this;
    }
    
    __hash_iterator operator++(int) {
        __hash_iterator __tmp(*this);
        ++(*this);
        return __tmp;
    }
    
    friend bool operator==(const __hash_iterator& x, const __hash_iterator& y) {
        return x.__node_ == y.__node_;
    }
    
    friend bool operator!=(const __hash_iterator& x, const __hash_iterator& y) {
        return !(x == y);
    }
    
    template <class, class, class, class>
    friend class __hash_table;
    template <class>
    friend class __hash_const_iterator;
};

// ============================================================================
// Const 全局迭代器
// ============================================================================
template <class _NodePtr>
class __hash_const_iterator {
    using _NodeTypes = __hash_node_types<_NodePtr>;
    using __node_pointer = _NodePtr;
    using __next_pointer = typename _NodeTypes::__next_pointer;
    
    __next_pointer __node_;
    
public:
    using __non_const_iterator = __hash_iterator<_NodePtr>;
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename _NodeTypes::__node_value_type;
    using difference_type = typename _NodeTypes::difference_type;
    using reference = const value_type&;
    using pointer = const value_type*;
    
    __hash_const_iterator() noexcept : __node_(nullptr) {}
    
    __hash_const_iterator(const __non_const_iterator& x) noexcept
        : __node_(x.__node_) {}
    
    explicit __hash_const_iterator(__next_pointer __node) noexcept
        : __node_(__node) {}
    
    reference operator*() const {
        return __node_->__upcast()->__get_value();
    }
    
    pointer operator->() const {
        return std::addressof(__node_->__upcast()->__get_value());
    }
    
    __hash_const_iterator& operator++() {
        __node_ = __node_->__next_;
        return *this;
    }
    
    __hash_const_iterator operator++(int) {
        __hash_const_iterator __tmp(*this);
        ++(*this);
        return __tmp;
    }
    
    friend bool operator==(const __hash_const_iterator& x, const __hash_const_iterator& y) {
        return x.__node_ == y.__node_;
    }
    
    friend bool operator!=(const __hash_const_iterator& x, const __hash_const_iterator& y) {
        return !(x == y);
    }
    
    template <class, class, class, class>
    friend class __hash_table;
};

// ============================================================================
// 局部（桶）迭代器
// ============================================================================
template <class _NodePtr>
class __hash_local_iterator {
    using _NodeTypes = __hash_node_types<_NodePtr>;
    using __node_pointer = _NodePtr;
    using __next_pointer = typename _NodeTypes::__next_pointer;
    
    __next_pointer __node_;
    size_t __bucket_;
    size_t __bucket_count_;
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename _NodeTypes::__node_value_type;
    using difference_type = typename _NodeTypes::difference_type;
    using reference = value_type&;
    using pointer = value_type*;
    
    __hash_local_iterator() noexcept : __node_(nullptr) {}
    
    explicit __hash_local_iterator(__next_pointer __node, size_t __bucket,
                                  size_t __bucket_count) noexcept
        : __node_(__node), __bucket_(__bucket), __bucket_count_(__bucket_count) {
        if (__node_ != nullptr) {
            __node_ = __node_->__next_;
        }
    }
    
    reference operator*() const {
        return __node_->__upcast()->__get_value();
    }
    
    pointer operator->() const {
        return std::addressof(__node_->__upcast()->__get_value());
    }
    
    __hash_local_iterator& operator++() {
        __node_ = __node_->__next_;
        if (__node_ != nullptr &&
            __constrain_hash(__node_->__hash(), __bucket_count_) != __bucket_) {
            __node_ = nullptr;
        }
        return *this;
    }
    
    __hash_local_iterator operator++(int) {
        __hash_local_iterator __tmp(*this);
        ++(*this);
        return __tmp;
    }
    
    friend bool operator==(const __hash_local_iterator& x, const __hash_local_iterator& y) {
        return x.__node_ == y.__node_;
    }
    
    friend bool operator!=(const __hash_local_iterator& x, const __hash_local_iterator& y) {
        return !(x == y);
    }
    
    template <class, class, class, class>
    friend class __hash_table;
};

// ============================================================================
// Const 局部迭代器
// ============================================================================
template <class _ConstNodePtr>
class __hash_const_local_iterator {
    using _NodeTypes = __hash_node_types<_ConstNodePtr>;
    using __node_pointer = _ConstNodePtr;
    using __next_pointer = typename _NodeTypes::__next_pointer;
    
    __next_pointer __node_;
    size_t __bucket_;
    size_t __bucket_count_;
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename _NodeTypes::__node_value_type;
    using difference_type = typename _NodeTypes::difference_type;
    using reference = const value_type&;
    using pointer = const value_type*;
    
    __hash_const_local_iterator() noexcept : __node_(nullptr) {}
    
    explicit __hash_const_local_iterator(__next_pointer __node_ptr, size_t __bucket,
                                        size_t __bucket_count) noexcept
        : __node_(__node_ptr), __bucket_(__bucket), __bucket_count_(__bucket_count) {
        if (__node_ != nullptr) {
            __node_ = __node_->__next_;
        }
    }
    
    reference operator*() const {
        return __node_->__upcast()->__get_value();
    }
    
    pointer operator->() const {
        return std::addressof(__node_->__upcast()->__get_value());
    }
    
    __hash_const_local_iterator& operator++() {
        __node_ = __node_->__next_;
        if (__node_ != nullptr &&
            __constrain_hash(__node_->__hash(), __bucket_count_) != __bucket_) {
            __node_ = nullptr;
        }
        return *this;
    }
    
    __hash_const_local_iterator operator++(int) {
        __hash_const_local_iterator __tmp(*this);
        ++(*this);
        return __tmp;
    }
    
    friend bool operator==(const __hash_const_local_iterator& x,
                          const __hash_const_local_iterator& y) {
        return x.__node_ == y.__node_;
    }
    
    friend bool operator!=(const __hash_const_local_iterator& x,
                          const __hash_const_local_iterator& y) {
        return !(x == y);
    }
    
    template <class, class, class, class>
    friend class __hash_table;
};

// ============================================================================
// 桶数组删除器
// ============================================================================
template <class _Alloc>
class __bucket_list_deallocator {
    using allocator_type = _Alloc;
    using __alloc_traits = std::allocator_traits<allocator_type>;
    using size_type = typename __alloc_traits::size_type;
    
    size_type __size_;
    allocator_type __alloc_;
    
public:
    using pointer = typename __alloc_traits::pointer;
    __bucket_list_deallocator() noexcept : __size_(0) {}
    
    __bucket_list_deallocator(const allocator_type& a, size_type size) noexcept
        : __size_(size), __alloc_(a) {}
    
    size_type& size() noexcept { return __size_; }
    size_type size() const noexcept { return __size_; }
    
    allocator_type& __alloc() noexcept { return __alloc_; }
    const allocator_type& __alloc() const noexcept { return __alloc_; }
    
    void operator()(pointer p) noexcept {
        if (p != nullptr) {
            __alloc_traits::deallocate(__alloc_, p, size());
        }
    }
};

// ============================================================================
// 节点删除器
// ============================================================================
template <class _Alloc>
class __hash_node_destructor {
    using allocator_type = _Alloc;
    using __alloc_traits = std::allocator_traits<allocator_type>;
    using pointer = typename __alloc_traits::pointer;
    
    allocator_type& __na_;
    
public:
    bool __value_constructed;
    
    explicit __hash_node_destructor(allocator_type& na, bool constructed = false) noexcept
        : __na_(na), __value_constructed(constructed) {}
    
    void operator()(pointer p) noexcept {
        if (p) {
            if (__value_constructed) {
                __alloc_traits::destroy(__na_, &p->__get_value());
            }
            __alloc_traits::deallocate(__na_, p, 1);
        }
    }
};

// ============================================================================
// 主哈希表类
// ============================================================================
template <class _Tp, class _Hash, class _Equal, class _Alloc>
class __hash_table {
public:
    using value_type = _Tp;
    using hasher = _Hash;
    using key_equal = _Equal;
    using allocator_type = _Alloc;
    
private:
    using __alloc_traits = std::allocator_traits<allocator_type>;
    using __void_pointer = typename __alloc_traits::void_pointer;
    
    // 节点类型
    using __node = __hash_node<_Tp, __void_pointer>;
    using __node_pointer = typename std::pointer_traits<__void_pointer>::template rebind<__node>;
    using __node_base_type = __hash_node_base<__node_pointer>;
    using __node_base_pointer = typename std::pointer_traits<__void_pointer>::template rebind<__node_base_type>;
    using __next_pointer = typename __node_base_type::__next_pointer;
    
    // 节点分配器
    using __node_allocator = typename __alloc_traits::template rebind_alloc<__node>;
    using __node_traits = std::allocator_traits<__node_allocator>;
    using __node_base_allocator = typename __node_traits::template rebind_alloc<__node_base_type>;
    using __node_base_traits = std::allocator_traits<__node_base_allocator>;
    
    // 桶数组分配器
    using __pointer_allocator = typename __node_traits::template rebind_alloc<__next_pointer>;
    using __pointer_alloc_traits = std::allocator_traits<__pointer_allocator>;
    using __bucket_list_deleter = __bucket_list_deallocator<__pointer_allocator>;
    using __bucket_list = std::unique_ptr<__next_pointer[], __bucket_list_deleter>;
    using __node_pointer_pointer = typename __pointer_alloc_traits::pointer;
    
    // 删除器类型
    using _Dp = __hash_node_destructor<__node_allocator>;
    using __node_holder = std::unique_ptr<__node, _Dp>;
    
    // 成员变量
    __bucket_list __bucket_list_;
    __node_base_type __first_node_;
    __node_allocator __node_alloc_;
    size_t __size_;
    hasher __hasher_;
    float __max_load_factor_;
    key_equal __key_eq_;
    
    size_t& size() noexcept { return __size_; }
    
public:
    using size_type = typename __alloc_traits::size_type;
    using difference_type = typename __alloc_traits::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename __alloc_traits::pointer;
    using const_pointer = typename __alloc_traits::const_pointer;
    
    using iterator = __hash_iterator<__node_pointer>;
    using const_iterator = __hash_const_iterator<__node_pointer>;
    using local_iterator = __hash_local_iterator<__node_pointer>;
    using const_local_iterator = __hash_const_local_iterator<__node_pointer>;
    
    // 构造函数
    __hash_table() noexcept
        : __bucket_list_(nullptr, __bucket_list_deleter()),
          __first_node_(),
          __node_alloc_(),
          __size_(0),
          __hasher_(),
          __max_load_factor_(1.0f),
          __key_eq_() {}
    
    __hash_table(const hasher& hf, const key_equal& eql)
        : __bucket_list_(nullptr, __bucket_list_deleter()),
          __first_node_(),
          __node_alloc_(),
          __size_(0),
          __hasher_(hf),
          __max_load_factor_(1.0f),
          __key_eq_(eql) {}
    
    __hash_table(const hasher& hf, const key_equal& eql, const allocator_type& a)
        : __bucket_list_(nullptr, __bucket_list_deleter(__pointer_allocator(a), 0)),
          __node_alloc_(a),
          __size_(0),
          __hasher_(hf),
          __max_load_factor_(1.0f),
          __key_eq_(eql) {}
    
    explicit __hash_table(const allocator_type& a)
        : __bucket_list_(nullptr, __bucket_list_deleter(__pointer_allocator(a), 0)),
          __node_alloc_(a),
          __size_(0),
          __max_load_factor_(1.0f) {}
    
    // 拷贝构造
    __hash_table(const __hash_table& other)
        : __bucket_list_(nullptr, __bucket_list_deleter(
              __pointer_allocator(
                  std::allocator_traits<__pointer_allocator>::select_on_container_copy_construction(
                      other.__bucket_list_.get_deleter().__alloc())),
              0)),
          __node_alloc_(std::allocator_traits<__node_allocator>::select_on_container_copy_construction(
              other.__node_alloc())),
          __size_(0),
          __hasher_(other.hash_function()),
          __max_load_factor_(other.max_load_factor()),
          __key_eq_(other.key_eq()) {
        // 深拷贝：插入所有元素
        for (const_iterator it = other.begin(); it != other.end(); ++it) {
            __insert_unique(*it);
        }
    }
    
    // 移动构造
    __hash_table(__hash_table&& other) noexcept
        : __bucket_list_(std::move(other.__bucket_list_)),
          __first_node_(std::move(other.__first_node_)),
          __node_alloc_(std::move(other.__node_alloc_)),
          __size_(other.__size_),
          __hasher_(std::move(other.__hasher_)),
          __max_load_factor_(other.__max_load_factor_),
          __key_eq_(std::move(other.__key_eq_)) {
        if (size() > 0) {
            __bucket_list_[__constrain_hash(__first_node_.__next_->__hash(), bucket_count())] =
                __first_node_.__ptr();
        }
        other.__first_node_.__next_ = nullptr;
        other.__size_ = 0;
    }
    
    // 析构函数
    ~__hash_table() {
        clear();
    }
    
    // 拷贝赋值
    __hash_table& operator=(const __hash_table& other) {
        if (this != &other) {
            clear();
            hash_function() = other.hash_function();
            key_eq() = other.key_eq();
            max_load_factor() = other.max_load_factor();
            
            // 拷贝分配器（如果需要）
            if (std::allocator_traits<__node_allocator>::propagate_on_container_copy_assignment::value) {
                __node_alloc() = other.__node_alloc();
            }
            
            // 插入所有元素
            for (const_iterator it = other.begin(); it != other.end(); ++it) {
                __insert_unique(*it);
            }
        }
        return *this;
    }
    
    // 移动赋值
    __hash_table& operator=(__hash_table&& other) noexcept {
        if (this != &other) {
            clear();
            
            __bucket_list_ = std::move(other.__bucket_list_);
            __first_node_.__next_ = other.__first_node_.__next_;
            __size_ = other.__size_;
            __hasher_ = std::move(other.__hasher_);
            __max_load_factor_ = other.__max_load_factor_;
            __key_eq_ = std::move(other.__key_eq_);
            
            if (std::allocator_traits<__node_allocator>::propagate_on_container_move_assignment::value) {
                __node_alloc() = std::move(other.__node_alloc());
            }
            
            if (size() > 0) {
                __bucket_list_[__constrain_hash(__first_node_.__next_->__hash(), bucket_count())] =
                    __first_node_.__ptr();
            }
            
            other.__first_node_.__next_ = nullptr;
            other.__size_ = 0;
        }
        return *this;
    }
    
    // 访问器
    size_type size() const noexcept { return __size_; }
    
    hasher& hash_function() noexcept { return __hasher_; }
    const hasher& hash_function() const noexcept { return __hasher_; }
    
    float& max_load_factor() noexcept { return __max_load_factor_; }
    float max_load_factor() const noexcept { return __max_load_factor_; }
    
    void max_load_factor(float mlf) noexcept {
        __max_load_factor_ = std::max(mlf, load_factor());
    }
    
    key_equal& key_eq() noexcept { return __key_eq_; }
    const key_equal& key_eq() const noexcept { return __key_eq_; }
    
    __node_allocator& __node_alloc() noexcept { return __node_alloc_; }
    const __node_allocator& __node_alloc() const noexcept { return __node_alloc_; }
    
    size_type max_size() const noexcept {
        return std::min<size_type>(
            __node_traits::max_size(__node_alloc()),
            std::numeric_limits<difference_type>::max());
    }
    
    size_type bucket_count() const noexcept {
        return __bucket_list_.get_deleter().size();
    }
    
    float load_factor() const noexcept {
        size_type bc = bucket_count();
        return bc != 0 ? static_cast<float>(size()) / bc : 0.0f;
    }
    
    // 迭代器
    iterator begin() noexcept {
        return iterator(__first_node_.__next_);
    }
    
    iterator end() noexcept {
        return iterator(nullptr);
    }
    
    const_iterator begin() const noexcept {
        return const_iterator(__first_node_.__next_);
    }
    
    const_iterator end() const noexcept {
        return const_iterator(nullptr);
    }
    
    // 桶迭代器
    local_iterator begin(size_type n) {
        return local_iterator(__bucket_list_[n], n, bucket_count());
    }
    
    local_iterator end(size_type n) {
        return local_iterator(nullptr, n, bucket_count());
    }
    
    const_local_iterator cbegin(size_type n) const {
        return const_local_iterator(__bucket_list_[n], n, bucket_count());
    }
    
    const_local_iterator cend(size_type n) const {
        return const_local_iterator(nullptr, n, bucket_count());
    }
    
    // 查找
    template <class _Key>
    iterator find(const _Key& k) {
        size_t hash = hash_function()(k);
        size_type bc = bucket_count();
        if (bc != 0) {
            size_t chash = __constrain_hash(hash, bc);
            __next_pointer nd = __bucket_list_[chash];
            if (nd != nullptr) {
                for (nd = nd->__next_;
                     nd != nullptr &&
                     (nd->__hash() == hash || __constrain_hash(nd->__hash(), bc) == chash);
                     nd = nd->__next_) {
                    if (nd->__hash() == hash && key_eq()(nd->__upcast()->__get_value(), k)) {
                        return iterator(nd);
                    }
                }
            }
        }
        return end();
    }
    
    template <class _Key>
    const_iterator find(const _Key& k) const {
        size_t hash = hash_function()(k);
        size_type bc = bucket_count();
        if (bc != 0) {
            size_t chash = __constrain_hash(hash, bc);
            __next_pointer nd = __bucket_list_[chash];
            if (nd != nullptr) {
                for (nd = nd->__next_;
                     nd != nullptr &&
                     (hash == nd->__hash() || __constrain_hash(nd->__hash(), bc) == chash);
                     nd = nd->__next_) {
                    if (nd->__hash() == hash && key_eq()(nd->__upcast()->__get_value(), k)) {
                        return const_iterator(nd);
                    }
                }
            }
        }
        return end();
    }
    
    template <class _Key>
    size_type bucket(const _Key& k) const {
        size_type bc = bucket_count();
        if (bc == 0) return 0;
        return __constrain_hash(hash_function()(k), bc);
    }
    
    size_type bucket_size(size_type n) const {
        size_type bc = bucket_count();
        if (n >= bc) return 0;
        __next_pointer np = __bucket_list_[n];
        size_type r = 0;
        if (np != nullptr) {
            for (np = np->__next_;
                 np != nullptr && __constrain_hash(np->__hash(), bc) == n;
                 np = np->__next_, ++r)
                ;
        }
        return r;
    }
    
    // 计数
    template <class _Key>
    size_type __count_unique(const _Key& k) const {
        return static_cast<size_type>(find(k) != end());
    }
    
    // 清空
    void clear() noexcept {
        if (size() > 0) {
            __deallocate_node(__first_node_.__next_);
            __first_node_.__next_ = nullptr;
            size_type bc = bucket_count();
            for (size_type i = 0; i < bc; ++i) {
                __bucket_list_[i] = nullptr;
            }
            size() = 0;
        }
    }
    
    // 删除
    iterator erase(const_iterator p) {
        __next_pointer np = p.__node_;
        iterator r(np);
        ++r;
        remove(p);
        return r;
    }
    
    iterator erase(const_iterator first, const_iterator last) {
        for (const_iterator p = first; first != last; p = first) {
            ++first;
            erase(p);
        }
        __next_pointer np = last.__node_;
        return iterator(np);
    }
    
    template <class _Key>
    size_type __erase_unique(const _Key& k) {
        iterator i = find(k);
        if (i == end()) {
            return 0;
        }
        erase(i);
        return 1;
    }
    
    // 交换
    void swap(__hash_table& u) noexcept {
        using std::swap;
        __bucket_list_.swap(u.__bucket_list_);
        swap(__first_node_.__next_, u.__first_node_.__next_);
        swap(__node_alloc_, u.__node_alloc_);
        swap(__size_, u.__size_);
        swap(__hasher_, u.__hasher_);
        swap(__max_load_factor_, u.__max_load_factor_);
        swap(__key_eq_, u.__key_eq_);
        
        // 更新桶索引
        if (size() > 0) {
            __bucket_list_[__constrain_hash(__first_node_.__next_->__hash(), bucket_count())] =
                __first_node_.__ptr();
        }
        if (u.size() > 0) {
            u.__bucket_list_[__constrain_hash(u.__first_node_.__next_->__hash(), u.bucket_count())] =
                u.__first_node_.__ptr();
        }
    }
    
private:
    // 删除节点
    void __deallocate_node(__next_pointer np) noexcept {
        __node_allocator& na = __node_alloc();
        while (np != nullptr) {
            __next_pointer next = np->__next_;
            __node_pointer real_np = np->__upcast();
            __node_traits::destroy(na, &real_np->__get_value());
            __node_traits::deallocate(na, real_np, 1);
            np = next;
        }
    }
    
    // 移除节点（返回节点句柄）
    __node_holder remove(const_iterator p) noexcept {
        __next_pointer cn = p.__node_;
        size_type bc = bucket_count();
        size_t chash = __constrain_hash(cn->__hash(), bc);
        
        // 查找前驱节点
        __next_pointer pn = __bucket_list_[chash];
        for (; pn->__next_ != cn; pn = pn->__next_)
            ;
        
        // 更新桶索引
        if (pn == __first_node_.__ptr() || __constrain_hash(pn->__hash(), bc) != chash) {
            if (cn->__next_ == nullptr || __constrain_hash(cn->__next_->__hash(), bc) != chash) {
                __bucket_list_[chash] = nullptr;
            }
        }
        
        if (cn->__next_ != nullptr) {
            size_t nhash = __constrain_hash(cn->__next_->__hash(), bc);
            if (nhash != chash) {
                __bucket_list_[nhash] = pn;
            }
        }
        
        // 从链表中移除
        pn->__next_ = cn->__next_;
        cn->__next_ = nullptr;
        --size();
        
        return __node_holder(cn->__upcast(), _Dp(__node_alloc(), true));
    }
    
    // 构造节点
    template <class... Args>
    __node_holder __construct_node(Args&&... args) {
        __node_allocator& na = __node_alloc();
        __node_holder h(__node_traits::allocate(na, 1), _Dp(na));
        
        // 构造节点结构体
        new (std::addressof(*h.get())) __node(nullptr, 0);
        
        // 构造值类型
        __node_traits::construct(na, &h->__get_value(), std::forward<Args>(args)...);
        h.get_deleter().__value_constructed = true;
        
        // 计算并缓存哈希值
        h->__hash_ = hash_function()(h->__get_value());
        
        return h;
    }
    
    template <class _First, class... _Rest>
    __node_holder __construct_node_hash(size_t hash, _First&& f, _Rest&&... rest) {
        __node_allocator& na = __node_alloc();
        __node_holder h(__node_traits::allocate(na, 1), _Dp(na));
        
        new (std::addressof(*h.get())) __node(nullptr, hash);
        __node_traits::construct(na, &h->__get_value(),
                                 std::forward<_First>(f), std::forward<_Rest>(rest)...);
        h.get_deleter().__value_constructed = true;
        
        return h;
    }
    
    // 插入准备（unique keys）
    __next_pointer __node_insert_unique_prepare(size_t hash, value_type& value) {
        size_type bc = bucket_count();
        
        if (bc != 0) {
            size_t chash = __constrain_hash(hash, bc);
            __next_pointer ndptr = __bucket_list_[chash];
            if (ndptr != nullptr) {
                for (ndptr = ndptr->__next_;
                     ndptr != nullptr &&
                     (ndptr->__hash() == hash || __constrain_hash(ndptr->__hash(), bc) == chash);
                     ndptr = ndptr->__next_) {
                    if (ndptr->__hash() == hash &&
                        key_eq()(ndptr->__upcast()->__get_value(), value)) {
                        return ndptr;
                    }
                }
            }
        }
        
        // 检查是否需要扩容
        if (size() + 1 > bc * max_load_factor() || bc == 0) {
            __rehash_unique(std::max<size_type>(
                2 * bc + !__is_hash_power2(bc),
                static_cast<size_type>(std::ceil(static_cast<float>(size() + 1) / max_load_factor()))));
        }
        
        return nullptr;
    }
    
    // 执行插入（unique keys）
    void __node_insert_unique_perform(__node_pointer nd) noexcept {
        size_type bc = bucket_count();
        size_t chash = __constrain_hash(nd->__hash_, bc);
        
        __next_pointer pn = __bucket_list_[chash];
        if (pn == nullptr) {
            pn = __first_node_.__ptr();
            nd->__next_ = pn->__next_;
            pn->__next_ = nd->__ptr();
            __bucket_list_[chash] = pn;
            if (nd->__next_ != nullptr) {
                __bucket_list_[__constrain_hash(nd->__next_->__hash(), bc)] = nd->__ptr();
            }
        } else {
            nd->__next_ = pn->__next_;
            pn->__next_ = nd->__ptr();
        }
        ++size();
    }
    
    // 节点插入（unique keys）
    std::pair<iterator, bool> __node_insert_unique(__node_pointer nd) {
        nd->__hash_ = hash_function()(nd->__get_value());
        __next_pointer existing_node = __node_insert_unique_prepare(nd->__hash_, nd->__get_value());
        
        bool inserted = false;
        if (existing_node == nullptr) {
            __node_insert_unique_perform(nd);
            existing_node = nd->__ptr();
            inserted = true;
        }
        return std::pair<iterator, bool>(iterator(existing_node), inserted);
    }
    
public:
    // Rehash（需要 public，因为 unordered_map 需要访问）
    void __rehash_unique(size_type n) {
        if (n == 1) {
            n = 2;
        } else if (!__is_hash_power2(n)) {
            n = __next_prime(n);
        }
        
        size_type bc = bucket_count();
        if (n > bc || (n < bc && n >= static_cast<size_type>(std::ceil(static_cast<float>(size()) / max_load_factor())))) {
            __do_rehash_unique(n);
        }
    }
    
    void __do_rehash_unique(size_type nbc) {
        __pointer_allocator& npa = __bucket_list_.get_deleter().__alloc();
        
        // 分配新桶数组
        __next_pointer* new_buckets = nbc > 0 ? __pointer_alloc_traits::allocate(npa, nbc) : nullptr;
        __bucket_list_.reset(new_buckets);
        __bucket_list_.get_deleter().size() = nbc;
        
        if (nbc > 0) {
            // 初始化新桶数组
            for (size_type i = 0; i < nbc; ++i) {
                __bucket_list_[i] = nullptr;
            }
            
            // 重新分布节点
            __next_pointer pp = __first_node_.__ptr();
            __next_pointer cp = pp->__next_;
            
            if (cp != nullptr) {
                size_type chash = __constrain_hash(cp->__hash(), nbc);
                __bucket_list_[chash] = pp;
                size_type phash = chash;
                
                for (pp = cp, cp = cp->__next_; cp != nullptr; cp = pp->__next_) {
                    chash = __constrain_hash(cp->__hash(), nbc);
                    if (chash == phash) {
                        pp = cp;
                    } else {
                        if (__bucket_list_[chash] == nullptr) {
                            __bucket_list_[chash] = pp;
                            pp = cp;
                            phash = chash;
                        } else {
                            __next_pointer np = cp;
                            pp->__next_ = np->__next_;
                            np->__next_ = __bucket_list_[chash]->__next_;
                            __bucket_list_[chash]->__next_ = cp;
                        }
                    }
                }
            }
        }
    }
    
    void __reserve_unique(size_type n) {
        __rehash_unique(static_cast<size_type>(std::ceil(static_cast<float>(n) / max_load_factor())));
    }
    
public:
    // 插入接口
    std::pair<iterator, bool> __insert_unique(const value_type& x) {
        __node_holder h = __construct_node(x);
        std::pair<iterator, bool> r = __node_insert_unique(h.get());
        if (r.second) {
            h.release();
        }
        return r;
    }
    
    std::pair<iterator, bool> __insert_unique(value_type&& x) {
        __node_holder h = __construct_node(std::move(x));
        std::pair<iterator, bool> r = __node_insert_unique(h.get());
        if (r.second) {
            h.release();
        }
        return r;
    }
    
    template <class... Args>
    std::pair<iterator, bool> __emplace_unique(Args&&... args) {
        __node_holder h = __construct_node(std::forward<Args>(args)...);
        std::pair<iterator, bool> r = __node_insert_unique(h.get());
        if (r.second) {
            h.release();
        }
        return r;
    }
    
    template <class _Key, class... Args>
    std::pair<iterator, bool> __emplace_unique_key_args(const _Key& k, Args&&... args) {
        size_t hash = hash_function()(k);
        size_type bc = bucket_count();
        bool inserted = false;
        __next_pointer nd;
        size_t chash;
        
        if (bc != 0) {
            chash = __constrain_hash(hash, bc);
            nd = __bucket_list_[chash];
            if (nd != nullptr) {
                for (nd = nd->__next_;
                     nd != nullptr &&
                     (nd->__hash() == hash || __constrain_hash(nd->__hash(), bc) == chash);
                     nd = nd->__next_) {
                    if (nd->__hash() == hash && key_eq()(nd->__upcast()->__get_value(), k)) {
                        goto __done;
                    }
                }
            }
        }
        
        {
            __node_holder h = __construct_node_hash(hash, std::forward<Args>(args)...);
            if (size() + 1 > bc * max_load_factor() || bc == 0) {
                __rehash_unique(std::max<size_type>(
                    2 * bc + !__is_hash_power2(bc),
                    static_cast<size_type>(std::ceil(static_cast<float>(size() + 1) / max_load_factor()))));
                bc = bucket_count();
                chash = __constrain_hash(hash, bc);
            }
            
            __next_pointer pn = __bucket_list_[chash];
            if (pn == nullptr) {
                pn = __first_node_.__ptr();
                h->__next_ = pn->__next_;
                pn->__next_ = h.get()->__ptr();
                __bucket_list_[chash] = pn;
                if (h->__next_ != nullptr) {
                    __bucket_list_[__constrain_hash(h->__next_->__hash(), bc)] = h.get()->__ptr();
                }
            } else {
                h->__next_ = pn->__next_;
                pn->__next_ = static_cast<__next_pointer>(h.get());
            }
            nd = static_cast<__next_pointer>(h.release());
            ++size();
            inserted = true;
        }
    __done:
        return std::pair<iterator, bool>(iterator(nd), inserted);
    }
};

}  // namespace mystl

#endif  // TINYSTL___HASH_TABLE_H_

