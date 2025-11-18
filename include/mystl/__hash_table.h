#ifndef TINYSTL_HASH_TABLE_H_
#define TINYSTL_HASH_TABLE_H_

#include <algorithm>  // for max, min
#include <cmath>      // for ceil
#include <cstddef>    // for size_t, ptrdiff_t
#include <iterator>   // for forward_iterator_tag
#include <limits>     // for numeric_limits
#include <memory>  // for allocator_traits, unique_ptr, addressof, pointer_traits
#include <utility>  // for pair, move, forward

namespace mystl {

// ============================================================================
// 前向声明
// ============================================================================
template <class _NodePtr>
class hash_iterator;
template <class _ConstNodePtr>
class hash_const_iterator;
template <class _NodePtr>
class hash_local_iterator;
template <class _ConstNodePtr>
class hash_const_local_iterator;

template <class _Tp, class _Hash, class _Equal, class _Alloc>
class hash_table;

// ============================================================================
// 辅助函数：计算下一个质数（简化版本）
// ============================================================================
inline size_t next_prime(size_t n) {
  static const size_t small_primes[] = {2,  3,  5,  7,  11, 13, 17, 19, 23, 29,
                                        31, 37, 41, 43, 47, 53, 59, 61, 67, 71};

  // 小质数直接查找
  for (size_t prime : small_primes) {
    if (prime >= n) {
      return prime;
    }
  }

  // 简单查找下一个质数（从 n 开始）
  for (size_t i = (n % 2 == 0 ? n + 1 : n);; i += 2) {
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
inline bool is_hash_power2(size_t n) {
  return n > 2 && (n & (n - 1)) == 0;
}

// ============================================================================
// 辅助函数：将哈希值映射到桶索引
// ============================================================================
inline size_t constrain_hash(size_t h, size_t bc) {
  // 如果桶数是 2 的幂，使用位运算（更快）
  if (is_hash_power2(bc)) {
    return h & (bc - 1);
  }
  // 否则使用模运算
  return h < bc ? h : h % bc;
}

// ============================================================================
// 辅助函数：计算下一个 2 的幂
// ============================================================================
inline size_t next_hash_pow2(size_t n) {
  if (n < 2)
    return n;
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
struct hash_node_base {
  using node_type = typename std::pointer_traits<_NodePtr>::element_type;
  using node_base_pointer =
      typename std::pointer_traits<_NodePtr>::template rebind<hash_node_base>;
  using next_pointer = node_base_pointer;

  next_pointer next_;

  hash_node_base() noexcept : next_(nullptr) {}
  explicit hash_node_base(next_pointer next) noexcept : next_(next) {}

  next_pointer ptr() noexcept {
    // 获取一个指向当前 hash_node_base 对象的、类型为 next_pointer 的指针
    return std::pointer_traits<node_base_pointer>::pointer_to(*this);
  }

  // upcast()：将 node_base_pointer 转换为 _NodePtr
  _NodePtr upcast() noexcept {
    return std::pointer_traits<_NodePtr>::pointer_to(
        static_cast<node_type&>(*this));
  }

  size_t hash() const noexcept {
    return static_cast<node_type const&>(*this).hash_;
  }
};

// ============================================================================
// 完整节点类
// ============================================================================
template <class _Tp, class _VoidPtr>
struct hash_node : public hash_node_base<typename std::pointer_traits<
                       _VoidPtr>::template rebind<hash_node<_Tp, _VoidPtr>>> {
  using node_value_type = _Tp;
  using _Base = hash_node_base<typename std::pointer_traits<
      _VoidPtr>::template rebind<hash_node<_Tp, _VoidPtr>>>;
  using next_pointer = typename _Base::next_pointer;

  // hash_：存储哈希值
  size_t hash_;

 private:
  // value_：存储节点值
  union {
    _Tp value_;
  };

 public:
  explicit hash_node(next_pointer next, size_t hash)
      : _Base(next), hash_(hash) {}

  ~hash_node() {}

  _Tp& get_value() { return value_; }
  const _Tp& get_value() const { return value_; }
};

// ============================================================================
// ：区分 set 和 map
// ============================================================================
template <class _Tp>
struct hash_key_value_types {
  using key_type = _Tp;
  using node_value_type = _Tp;
  using container_value_type = _Tp;
  static const bool is_map = false;

  static const key_type& get_key(const _Tp& v) { return v; }
  static const container_value_type& get_value(const node_value_type& v) {
    return v;
  }
};

// ============================================================================
// 节点类型萃取
// ============================================================================
template <class _NodePtr>
struct hash_node_types {
  using difference_type = ptrdiff_t;  // 差值类型
  using size_type = size_t;           // 大小类型
  using node_type =
      typename std::pointer_traits<_NodePtr>::element_type;  // 节点类型
  using node_pointer = _NodePtr;                             // 节点指针
  using node_base_type = hash_node_base<node_pointer>;       // 节点基类
  using node_base_pointer = typename std::pointer_traits<
      _NodePtr>::template rebind<node_base_type>;              // 节点基类指针
  using next_pointer = typename node_base_type::next_pointer;  // 下一个节点指针
  using node_value_type = typename node_type::node_value_type;  // 节点值类型
};

// ============================================================================
// 迭代器：从迭代器类型萃取节点类型
// ============================================================================
template <class _HashIterator>
struct hash_node_types_from_iterator;
template <class _NodePtr>
struct hash_node_types_from_iterator<hash_iterator<_NodePtr>>
    : hash_node_types<_NodePtr> {};
template <class _NodePtr>
struct hash_node_types_from_iterator<hash_const_iterator<_NodePtr>>
    : hash_node_types<_NodePtr> {};

// ============================================================================
// 全局迭代器
// ============================================================================
template <class _NodePtr>
class hash_iterator {
  using _NodeTypes = hash_node_types<_NodePtr>;
  using node_pointer = _NodePtr;
  using next_pointer = typename _NodeTypes::next_pointer;

  next_pointer node_;

 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = typename _NodeTypes::node_value_type;
  using difference_type = typename _NodeTypes::difference_type;
  using reference = value_type&;
  using pointer = value_type*;

  hash_iterator() noexcept : node_(nullptr) {}

  explicit hash_iterator(next_pointer node) noexcept : node_(node) {}

  reference operator*() const { return node_->upcast()->get_value(); }

  pointer operator->() const {
    return std::addressof(node_->upcast()->get_value());
  }

  hash_iterator& operator++() {
    node_ = node_->next_;
    return *this;
  }

  hash_iterator operator++(int) {
    hash_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  friend bool operator==(const hash_iterator& x, const hash_iterator& y) {
    return x.node_ == y.node_;
  }

  friend bool operator!=(const hash_iterator& x, const hash_iterator& y) {
    return !(x == y);
  }

  template <class, class, class, class>
  friend class hash_table;
  template <class>
  friend class hash_const_iterator;
};

// ============================================================================
// Const 全局迭代器
// ============================================================================
template <class _NodePtr>
class hash_const_iterator {
  using _NodeTypes = hash_node_types<_NodePtr>;
  using node_pointer = _NodePtr;
  using next_pointer = typename _NodeTypes::
      next_pointer;  // 接口隔离，最终是hash_node_base，只需要知道有个next，不知道还有别的变量

  next_pointer node_;

 public:
  using non_const_iterator = hash_iterator<_NodePtr>;
  using iterator_category = std::forward_iterator_tag;
  using value_type = typename _NodeTypes::node_value_type;
  using difference_type = typename _NodeTypes::difference_type;
  using reference = const value_type&;
  using pointer = const value_type*;

  hash_const_iterator() noexcept : node_(nullptr) {}

  hash_const_iterator(const non_const_iterator& x) noexcept : node_(x.node_) {}

  explicit hash_const_iterator(next_pointer node) noexcept : node_(node) {}

  reference operator*() const { return node_->upcast()->get_value(); }

  pointer operator->() const {
    return std::addressof(node_->upcast()->get_value());
  }

  hash_const_iterator& operator++() {
    node_ = node_->next_;
    return *this;
  }

  hash_const_iterator operator++(int) {
    hash_const_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  friend bool operator==(const hash_const_iterator& x,
                         const hash_const_iterator& y) {
    return x.node_ == y.node_;
  }

  friend bool operator!=(const hash_const_iterator& x,
                         const hash_const_iterator& y) {
    return !(x == y);
  }

  template <class, class, class, class>
  friend class hash_table;
};

// ============================================================================
// 局部（桶）迭代器
// ============================================================================
template <class _NodePtr>
class hash_local_iterator {
  using _NodeTypes = hash_node_types<_NodePtr>;
  using node_pointer = _NodePtr;
  using next_pointer = typename _NodeTypes::next_pointer;

  next_pointer node_;
  size_t bucket_;
  size_t bucket_count_;

 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = typename _NodeTypes::node_value_type;
  using difference_type = typename _NodeTypes::difference_type;
  using reference = value_type&;
  using pointer = value_type*;

  hash_local_iterator() noexcept : node_(nullptr) {}

  explicit hash_local_iterator(next_pointer node, size_t bucket,
                               size_t bucket_count) noexcept
      : node_(node), bucket_(bucket), bucket_count_(bucket_count) {
    if (node_ != nullptr) {
      node_ = node_->next_;
    }
  }

  reference operator*() const { return node_->upcast()->get_value(); }

  pointer operator->() const {
    return std::addressof(node_->upcast()->get_value());
  }

  hash_local_iterator& operator++() {
    node_ = node_->next_;
    if (node_ != nullptr &&
        constrain_hash(node_->hash(), bucket_count_) != bucket_) {
      node_ = nullptr;
    }
    return *this;
  }

  hash_local_iterator operator++(int) {
    hash_local_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  friend bool operator==(const hash_local_iterator& x,
                         const hash_local_iterator& y) {
    return x.node_ == y.node_;
  }

  friend bool operator!=(const hash_local_iterator& x,
                         const hash_local_iterator& y) {
    return !(x == y);
  }

  template <class, class, class, class>
  friend class hash_table;
};

// ============================================================================
// Const 局部迭代器
// ============================================================================
template <class _ConstNodePtr>
class hash_const_local_iterator {
  using _NodeTypes = hash_node_types<_ConstNodePtr>;
  using node_pointer = _ConstNodePtr;
  using next_pointer = typename _NodeTypes::next_pointer;

  next_pointer node_;
  size_t bucket_;
  size_t bucket_count_;

 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = typename _NodeTypes::node_value_type;
  using difference_type = typename _NodeTypes::difference_type;
  using reference = const value_type&;
  using pointer = const value_type*;

  hash_const_local_iterator() noexcept : node_(nullptr) {}

  explicit hash_const_local_iterator(next_pointer node_ptr, size_t bucket,
                                     size_t bucket_count) noexcept
      : node_(node_ptr), bucket_(bucket), bucket_count_(bucket_count) {
    if (node_ != nullptr) {
      node_ = node_->next_;
    }
  }

  reference operator*() const { return node_->upcast()->get_value(); }

  pointer operator->() const {
    return std::addressof(node_->upcast()->get_value());
  }

  hash_const_local_iterator& operator++() {
    node_ = node_->next_;
    if (node_ != nullptr &&
        constrain_hash(node_->hash(), bucket_count_) != bucket_) {
      node_ = nullptr;
    }
    return *this;
  }

  hash_const_local_iterator operator++(int) {
    hash_const_local_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  friend bool operator==(const hash_const_local_iterator& x,
                         const hash_const_local_iterator& y) {
    return x.node_ == y.node_;
  }

  friend bool operator!=(const hash_const_local_iterator& x,
                         const hash_const_local_iterator& y) {
    return !(x == y);
  }

  template <class, class, class, class>
  friend class hash_table;
};

// ============================================================================
// 桶数组删除器
// ============================================================================
template <class _Alloc>
class bucket_list_deallocator {
  using allocator_type = _Alloc;
  using alloc_traits = std::allocator_traits<allocator_type>;
  using size_type = typename alloc_traits::size_type;

  size_type size_;
  allocator_type alloc_;

 public:
  using pointer = typename alloc_traits::pointer;
  bucket_list_deallocator() noexcept : size_(0) {}

  bucket_list_deallocator(const allocator_type& a, size_type size) noexcept
      : size_(size), alloc_(a) {}

  size_type& size() noexcept { return size_; }
  size_type size() const noexcept { return size_; }

  allocator_type& alloc() noexcept { return alloc_; }
  const allocator_type& alloc() const noexcept { return alloc_; }

  void operator()(pointer p) noexcept {
    if (p != nullptr) {
      alloc_traits::deallocate(alloc_, p, size());
    }
  }
};

// ============================================================================
// 节点删除器
// ============================================================================
template <class _Alloc>
class hash_node_destructor {
  using allocator_type = _Alloc;
  using alloc_traits = std::allocator_traits<allocator_type>;
  using pointer = typename alloc_traits::pointer;

  allocator_type& na_;

 public:
  bool value_constructed;

  explicit hash_node_destructor(allocator_type& na,
                                bool constructed = false) noexcept
      : na_(na), value_constructed(constructed) {}

  void operator()(pointer p) noexcept {
    if (p) {
      if (value_constructed) {
        alloc_traits::destroy(na_, &p->get_value());
      }
      alloc_traits::deallocate(na_, p, 1);
    }
  }
};

// ============================================================================
// 主哈希表类
// ============================================================================
template <class _Tp, class _Hash, class _Equal, class _Alloc>
class hash_table {
 public:
  using value_type = _Tp;
  using hasher = _Hash;
  using key_equal = _Equal;
  using allocator_type = _Alloc;

 private:
  using alloc_traits = std::allocator_traits<allocator_type>;
  using void_pointer = typename alloc_traits::void_pointer;

  // 节点类型
  using node = hash_node<_Tp, void_pointer>;
  using node_pointer =
      typename std::pointer_traits<void_pointer>::template rebind<node>;
  using node_base_type = hash_node_base<node_pointer>;
  using node_base_pointer = typename std::pointer_traits<
      void_pointer>::template rebind<node_base_type>;
  using next_pointer = typename node_base_type::next_pointer;

  // 节点分配器
  using node_allocator = typename alloc_traits::template rebind_alloc<node>;
  using node_traits = std::allocator_traits<node_allocator>;
  using node_base_allocator =
      typename node_traits::template rebind_alloc<node_base_type>;
  using node_base_traits = std::allocator_traits<node_base_allocator>;

  // 桶数组分配器
  using pointer_allocator =
      typename node_traits::template rebind_alloc<next_pointer>;
  using pointer_alloc_traits = std::allocator_traits<pointer_allocator>;
  using bucket_list_deleter = bucket_list_deallocator<pointer_allocator>;
  using bucket_list = std::unique_ptr<next_pointer[], bucket_list_deleter>;
  using node_pointer_pointer = typename pointer_alloc_traits::pointer;

  // 删除器类型
  using _Dp = hash_node_destructor<node_allocator>;
  using node_holder = std::unique_ptr<node, _Dp>;

  // 成员变量
  bucket_list bucket_list_;    //桶数组
  node_base_type first_node_;  // 第一个节点
  node_allocator node_alloc_;  // 节点分配器
  size_t size_;                // 元素个数
  hasher hasher_;              // 哈希函数
  float max_load_factor_;      // 最大负载因子
  key_equal key_eq_;           // 键相等性比较

 public:
  size_t& size() noexcept { return size_; }

  using size_type = typename alloc_traits::size_type;
  using difference_type = typename alloc_traits::difference_type;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename alloc_traits::pointer;
  using const_pointer = typename alloc_traits::const_pointer;

  using iterator = hash_iterator<node_pointer>;
  using const_iterator = hash_const_iterator<node_pointer>;
  using local_iterator = hash_local_iterator<node_pointer>;
  using const_local_iterator = hash_const_local_iterator<node_pointer>;

  // 构造函数
  hash_table() noexcept
      : bucket_list_(nullptr, bucket_list_deleter()),
        first_node_(),
        node_alloc_(),
        size_(0),
        hasher_(),
        max_load_factor_(1.0f),
        key_eq_() {}

  hash_table(const hasher& hf, const key_equal& eql)
      : bucket_list_(nullptr, bucket_list_deleter()),
        first_node_(),
        node_alloc_(),
        size_(0),
        hasher_(hf),
        max_load_factor_(1.0f),
        key_eq_(eql) {}

  hash_table(const hasher& hf, const key_equal& eql, const allocator_type& a)
      : bucket_list_(nullptr, bucket_list_deleter(pointer_allocator(a), 0)),
        node_alloc_(a),
        size_(0),
        hasher_(hf),
        max_load_factor_(1.0f),
        key_eq_(eql) {}

  explicit hash_table(const allocator_type& a)
      : bucket_list_(nullptr, bucket_list_deleter(pointer_allocator(a), 0)),
        node_alloc_(a),
        size_(0),
        max_load_factor_(1.0f) {}

  // 拷贝构造
  hash_table(const hash_table& other)
      : bucket_list_(nullptr,
                     bucket_list_deleter(
                         pointer_allocator(
                             std::allocator_traits<pointer_allocator>::
                                 select_on_container_copy_construction(
                                     other.bucket_list_.get_deleter().alloc())),
                         0)),
        node_alloc_(
            std::allocator_traits<node_allocator>::
                select_on_container_copy_construction(other.node_alloc())),
        size_(0),
        hasher_(other.hash_function()),
        max_load_factor_(other.max_load_factor()),
        key_eq_(other.key_eq()) {
    // 深拷贝：插入所有元素
    for (const_iterator it = other.begin(); it != other.end(); ++it) {
      insert_unique(*it);
    }
  }

  // 移动构造
  hash_table(hash_table&& other) noexcept
      : bucket_list_(std::move(other.bucket_list_)),
        first_node_(std::move(other.first_node_)),
        node_alloc_(std::move(other.node_alloc_)),
        size_(other.size_),
        hasher_(std::move(other.hasher_)),
        max_load_factor_(other.max_load_factor_),
        key_eq_(std::move(other.key_eq_)) {
    if (size() > 0) {
      bucket_list_[constrain_hash(first_node_.next_->hash(), bucket_count())] =
          first_node_.ptr();
    }
    other.first_node_.next_ = nullptr;
    other.size_ = 0;
  }

  // 析构函数
  ~hash_table() { clear(); }

  // 拷贝赋值
  hash_table& operator=(const hash_table& other) {
    if (this != &other) {
      clear();
      hash_function() = other.hash_function();
      key_eq() = other.key_eq();
      max_load_factor() = other.max_load_factor();

      // 拷贝分配器（如果需要）
      if (std::allocator_traits<
              node_allocator>::propagate_on_container_copy_assignment::value) {
        node_alloc() = other.node_alloc();
      }

      // 插入所有元素
      for (const_iterator it = other.begin(); it != other.end(); ++it) {
        insert_unique(*it);
      }
    }
    return *this;
  }

  // 移动赋值
  hash_table& operator=(hash_table&& other) noexcept {
    if (this != &other) {
      clear();

      bucket_list_ = std::move(other.bucket_list_);
      first_node_.next_ = other.first_node_.next_;
      size_ = other.size_;
      hasher_ = std::move(other.hasher_);
      max_load_factor_ = other.max_load_factor_;
      key_eq_ = std::move(other.key_eq_);

      if (std::allocator_traits<
              node_allocator>::propagate_on_container_move_assignment::value) {
        node_alloc() = std::move(other.node_alloc());
      }

      if (size() > 0) {
        bucket_list_[constrain_hash(first_node_.next_->hash(),
                                    bucket_count())] = first_node_.ptr();
      }

      other.first_node_.next_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }

  // 访问器
  size_type size() const noexcept { return size_; }

  hasher& hash_function() noexcept { return hasher_; }
  const hasher& hash_function() const noexcept { return hasher_; }

  float& max_load_factor() noexcept { return max_load_factor_; }
  float max_load_factor() const noexcept { return max_load_factor_; }

  void max_load_factor(float mlf) noexcept {
    max_load_factor_ = std::max(mlf, load_factor());
  }

  key_equal& key_eq() noexcept { return key_eq_; }
  const key_equal& key_eq() const noexcept { return key_eq_; }

  node_allocator& node_alloc() noexcept { return node_alloc_; }
  const node_allocator& node_alloc() const noexcept { return node_alloc_; }

  size_type max_size() const noexcept {
    return std::min<size_type>(node_traits::max_size(node_alloc()),
                               std::numeric_limits<difference_type>::max());
  }

  size_type bucket_count() const noexcept {
    return bucket_list_.get_deleter().size();
  }

  float load_factor() const noexcept {
    size_type bc = bucket_count();
    return bc != 0 ? static_cast<float>(size()) / bc : 0.0f;
  }

  // 迭代器
  iterator begin() noexcept { return iterator(first_node_.next_); }

  iterator end() noexcept { return iterator(nullptr); }

  const_iterator begin() const noexcept {
    return const_iterator(first_node_.next_);
  }

  const_iterator end() const noexcept { return const_iterator(nullptr); }

  // 桶迭代器
  local_iterator begin(size_type n) {
    return local_iterator(bucket_list_[n], n, bucket_count());
  }

  local_iterator end(size_type n) {
    return local_iterator(nullptr, n, bucket_count());
  }

  const_local_iterator cbegin(size_type n) const {
    return const_local_iterator(bucket_list_[n], n, bucket_count());
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
      size_t chash = constrain_hash(hash, bc);
      next_pointer nd = bucket_list_[chash];
      if (nd != nullptr) {
        for (nd = nd->next_;
             nd != nullptr &&
             (nd->hash() == hash || constrain_hash(nd->hash(), bc) == chash);
             nd = nd->next_) {
          if (nd->hash() == hash && key_eq()(nd->upcast()->get_value(), k)) {
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
      size_t chash = constrain_hash(hash, bc);
      next_pointer nd = bucket_list_[chash];
      if (nd != nullptr) {
        for (nd = nd->next_;
             nd != nullptr &&
             (hash == nd->hash() || constrain_hash(nd->hash(), bc) == chash);
             nd = nd->next_) {
          if (nd->hash() == hash && key_eq()(nd->upcast()->get_value(), k)) {
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
    if (bc == 0)
      return 0;
    return constrain_hash(hash_function()(k), bc);
  }

  size_type bucket_size(size_type n) const {
    size_type bc = bucket_count();
    if (n >= bc)
      return 0;
    next_pointer np = bucket_list_[n];
    size_type r = 0;
    if (np != nullptr) {
      for (np = np->next_; np != nullptr && constrain_hash(np->hash(), bc) == n;
           np = np->next_, ++r)
        ;
    }
    return r;
  }

  // 计数
  template <class _Key>
  size_type count_unique(const _Key& k) const {
    return static_cast<size_type>(find(k) != end());
  }

  // 清空
  void clear() noexcept {
    if (size() > 0) {
      deallocate_node(first_node_.next_);
      first_node_.next_ = nullptr;
      size_type bc = bucket_count();
      for (size_type i = 0; i < bc; ++i) {
        bucket_list_[i] = nullptr;
      }
      size() = 0;
    }
  }

  // 删除
  iterator erase(const_iterator p) {
    next_pointer np = p.node_;
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
    next_pointer np = last.node_;
    return iterator(np);
  }

  template <class _Key>
  size_type erase_unique(const _Key& k) {
    iterator i = find(k);
    if (i == end()) {
      return 0;
    }
    erase(i);
    return 1;
  }

  // 交换
  void swap(hash_table& u) noexcept {
    using std::swap;
    bucket_list_.swap(u.bucket_list_);
    swap(first_node_.next_, u.first_node_.next_);
    swap(node_alloc_, u.node_alloc_);
    swap(size_, u.size_);
    swap(hasher_, u.hasher_);
    swap(max_load_factor_, u.max_load_factor_);
    swap(key_eq_, u.key_eq_);

    // 更新桶索引
    if (size() > 0) {
      bucket_list_[constrain_hash(first_node_.next_->hash(), bucket_count())] =
          first_node_.ptr();
    }
    if (u.size() > 0) {
      u.bucket_list_[constrain_hash(u.first_node_.next_->hash(),
                                    u.bucket_count())] = u.first_node_.ptr();
    }
  }

 private:
  // 删除节点
  void deallocate_node(next_pointer np) noexcept {
    node_allocator& na = node_alloc();
    while (np != nullptr) {
      next_pointer next = np->next_;
      node_pointer real_np = np->upcast();
      node_traits::destroy(na, &real_np->get_value());
      node_traits::deallocate(na, real_np, 1);
      np = next;
    }
  }

  // 移除节点（返回节点句柄）
  node_holder remove(const_iterator p) noexcept {
    next_pointer cn = p.node_;
    size_type bc = bucket_count();
    size_t chash = constrain_hash(cn->hash(), bc);

    // 查找前驱节点
    next_pointer pn = bucket_list_[chash];
    for (; pn->next_ != cn; pn = pn->next_)
      ;

    // 更新桶索引
    if (pn == first_node_.ptr() || constrain_hash(pn->hash(), bc) != chash) {
      if (cn->next_ == nullptr ||
          constrain_hash(cn->next_->hash(), bc) != chash) {
        bucket_list_[chash] = nullptr;
      }
    }

    if (cn->next_ != nullptr) {
      size_t nhash = constrain_hash(cn->next_->hash(), bc);
      if (nhash != chash) {
        bucket_list_[nhash] = pn;
      }
    }

    // 从链表中移除
    pn->next_ = cn->next_;
    cn->next_ = nullptr;
    --size();

    return node_holder(cn->upcast(), _Dp(node_alloc(), true));
  }

  // 构造节点
  template <class... Args>
  node_holder construct_node(Args&&... args) {
    node_allocator& na = node_alloc();
    node_holder h(node_traits::allocate(na, 1), _Dp(na));

    // 构造节点结构体
    new (std::addressof(*h.get())) node(nullptr, 0);

    // 构造值类型
    node_traits::construct(na, &h->get_value(), std::forward<Args>(args)...);
    h.get_deleter().value_constructed = true;

    // 计算并缓存哈希值
    h->hash_ = hash_function()(h->get_value());

    return h;
  }

  template <class _First, class... _Rest>
  node_holder construct_node_hash(size_t hash, _First&& f, _Rest&&... rest) {
    node_allocator& na = node_alloc();
    node_holder h(node_traits::allocate(na, 1), _Dp(na));

    new (std::addressof(*h.get())) node(nullptr, hash);
    node_traits::construct(na, &h->get_value(), std::forward<_First>(f),
                           std::forward<_Rest>(rest)...);
    h.get_deleter().value_constructed = true;

    return h;
  }

  // 插入准备（unique keys）
  next_pointer node_insert_unique_prepare(size_t hash, value_type& value) {
    size_type bc = bucket_count();

    if (bc != 0) {
      size_t chash = constrain_hash(hash, bc);
      next_pointer ndptr = bucket_list_[chash];

      // ndptr != nullptr：链表走完了
      // ndptr->hash() == hash：完整哈希等于hash，无需再执行constrain_hash，已经能确定ndptr是当前桶
      // constrain_hash(ndptr->hash(), bc) == chash ： 是我们要找的桶
      if (ndptr != nullptr) {
        for (ndptr = ndptr->next_;
             ndptr != nullptr && (ndptr->hash() == hash ||
                                  constrain_hash(ndptr->hash(), bc) == chash);
             ndptr = ndptr->next_) {
          if (ndptr->hash() == hash &&
              key_eq()(ndptr->upcast()->get_value(), value)) {
            // 哈希比较 + 等价比较
            return ndptr;
          }
        }
      }
    }

    // 检查是否需要扩容
    if (size() + 1 > bc * max_load_factor() || bc == 0) {
      rehash_unique(std::max<size_type>(
          2 * bc + !is_hash_power2(bc),
          static_cast<size_type>(
              std::ceil(static_cast<float>(size() + 1) / max_load_factor()))));
    }

    return nullptr;
  }

  // 执行插入（unique keys）
  void node_insert_unique_perform(node_pointer nd) noexcept {
    size_type bc = bucket_count();
    size_t chash = constrain_hash(nd->hash_, bc);

    next_pointer pn = bucket_list_[chash];
    if (pn == nullptr) {
      pn = first_node_.ptr();    // 设置pn指向头哨兵节点
      nd->next_ = pn->next_;     // 将新节点插入到头哨兵节点的下一个位置
      pn->next_ = nd->ptr();     // 哨兵节点的下一个指向新节点
      bucket_list_[chash] = pn;  // 将桶的头部指向哨兵节点
      if (nd->next_ != nullptr) {
        bucket_list_[constrain_hash(nd->next_->hash(), bc)] = nd->ptr();
      }
    } else {
      nd->next_ = pn->next_;
      pn->next_ = nd->ptr();
    }
    ++size();
  }

  // 节点插入（unique keys）
  std::pair<iterator, bool> node_insert_unique(node_pointer nd) {
    nd->hash_ = hash_function()(nd->get_value());
    next_pointer existing_node =
        node_insert_unique_prepare(nd->hash_, nd->get_value());

    bool inserted = false;
    // 没找到相同键的节点，执行插入
    if (existing_node == nullptr) {
      node_insert_unique_perform(nd);
      existing_node = nd->ptr();
      inserted = true;
    }
    return std::pair<iterator, bool>(iterator(existing_node), inserted);
  }

 public:
  // Rehash（需要 public，因为 unordered_map 需要访问）
  void rehash_unique(size_type n) {
    if (n == 1) {
      n = 2;
    } else if (!is_hash_power2(n)) {
      n = next_prime(n);
    }

    size_type bc = bucket_count();
    if (n > bc ||
        (n < bc && n >= static_cast<size_type>(std::ceil(
                            static_cast<float>(size()) / max_load_factor())))) {
      do_rehash_unique(n);
    }
  }

  // do_rehash_unique：重新映射桶数组
  void do_rehash_unique(size_type nbc) {
    pointer_allocator& npa = bucket_list_.get_deleter().alloc();

    // 分配新桶数组
    next_pointer* new_buckets =
        nbc > 0 ? pointer_alloc_traits::allocate(npa, nbc) : nullptr;
    bucket_list_.reset(new_buckets);
    bucket_list_.get_deleter().size() = nbc;

    if (nbc > 0) {
      // 初始化新桶数组
      for (size_type i = 0; i < nbc; ++i) {
        bucket_list_[i] = nullptr;
      }

      // 重新分布节点
      next_pointer pp = first_node_.ptr();
      next_pointer cp = pp->next_;

      if (cp != nullptr) {
        size_type chash = constrain_hash(cp->hash(), nbc);
        bucket_list_[chash] = pp;
        size_type phash = chash;

        for (pp = cp, cp = cp->next_; cp != nullptr; cp = pp->next_) {
          chash = constrain_hash(cp->hash(), nbc);
          if (chash == phash) {
            pp = cp;
          } else {
            if (bucket_list_[chash] == nullptr) {
              bucket_list_[chash] = pp;
              pp = cp;
              phash = chash;
            } else {
              next_pointer np = cp;
              pp->next_ = np->next_;
              np->next_ = bucket_list_[chash]->next_;
              bucket_list_[chash]->next_ = cp;
            }
          }
        }
      }
    }
  }

  void reserve_unique(size_type n) {
    rehash_unique(static_cast<size_type>(
        std::ceil(static_cast<float>(n) / max_load_factor())));
  }

 public:
  // 插入接口
  std::pair<iterator, bool> insert_unique(const value_type& x) {
    node_holder h = construct_node(x);
    std::pair<iterator, bool> r = node_insert_unique(h.get());
    if (r.second) {
      h.release();
    }
    return r;
  }

  std::pair<iterator, bool> insert_unique(value_type&& x) {
    node_holder h = construct_node(std::move(x));
    std::pair<iterator, bool> r = node_insert_unique(h.get());
    if (r.second) {
      h.release();  // unique_str的release
    }
    return r;
  }

  template <class... Args>
  std::pair<iterator, bool> emplace_unique(Args&&... args) {
    node_holder h = construct_node(std::forward<Args>(args)...);
    std::pair<iterator, bool> r = node_insert_unique(h.get());
    if (r.second) {
      h.release();
    }
    return r;
  }

  template <class _Key, class... Args>
  std::pair<iterator, bool> emplace_unique_key_args(const _Key& k,
                                                    Args&&... args) {
    size_t hash = hash_function()(k);
    size_type bc = bucket_count();
    bool inserted = false;
    next_pointer nd;
    size_t chash;

    if (bc != 0) {
      chash = constrain_hash(hash, bc);
      nd = bucket_list_[chash];
      if (nd != nullptr) {
        for (nd = nd->next_;
             nd != nullptr &&
             (nd->hash() == hash || constrain_hash(nd->hash(), bc) == chash);
             nd = nd->next_) {
          if (nd->hash() == hash && key_eq()(nd->upcast()->get_value(), k)) {
            goto done;
          }
        }
      }
    }

    {
      node_holder h = construct_node_hash(hash, std::forward<Args>(args)...);
      if (size() + 1 > bc * max_load_factor() || bc == 0) {
        rehash_unique(std::max<size_type>(
            2 * bc + !is_hash_power2(bc),
            static_cast<size_type>(std::ceil(static_cast<float>(size() + 1) /
                                             max_load_factor()))));
        bc = bucket_count();
        chash = constrain_hash(hash, bc);
      }

      next_pointer pn = bucket_list_[chash];
      if (pn == nullptr) {
        pn = first_node_.ptr();
        h->next_ = pn->next_;
        pn->next_ = h.get()->ptr();
        bucket_list_[chash] = pn;
        if (h->next_ != nullptr) {
          bucket_list_[constrain_hash(h->next_->hash(), bc)] = h.get()->ptr();
        }
      } else {
        h->next_ = pn->next_;
        pn->next_ = static_cast<next_pointer>(h.get());
      }
      nd = static_cast<next_pointer>(h.release());
      ++size();
      inserted = true;
    }
  done:
    return std::pair<iterator, bool>(iterator(nd), inserted);
  }
};

}  // namespace mystl

#endif  // TINYSTL_HASH_TABLE_H_
