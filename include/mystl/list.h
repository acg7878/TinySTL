#ifndef TINYSTL_LIST_H
#define TINYSTL_LIST_H

#include <cstddef>

namespace mystl {

// 节点基类只包含指针，这使得哨兵节点可以不包含数据成员，节约空间。
struct list_node_base {
  list_node_base* prev;
  list_node_base* next;
};

template <class T>
struct list_node : public list_node_base {
  T data;
};


template <class T, class Ref, class Ptr>
struct list_iterator {
  using iterator = list_iterator<T, T&, T*>;
  using const_iterator = list_iterator<T, const T&, const T*>;
  using self = list_iterator<T, Ref, Ptr>;

  using value_type = T;
  using pointer = Ptr;
  using reference = Ref;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  using base_ptr = list_node_base*;
  using node_ptr = list_node<T>*;

  base_ptr node;  // 迭代器内部指向节点基类指针

  // 构造函数
  list_iterator() : node(nullptr) {}
  list_iterator(base_ptr n) : node(n) {}

  reference operator*() const { return static_cast<node_ptr>(node)->data; }
  pointer operator->() const { return &(static_cast<node_ptr>(node)->data); }

  //前置递增 ++it： 没有参数
  self& operator++() {
    node = node->next;
    return *this;
  }
  //后置递增 it++： 接受一个 int 类型的参数
  self operator++(int) {
    self tmp = *this;
    node = node->next;
    return tmp;
    // 后置递增迭代器，返回tmp也就是递增前的状态
  }
  self& operator--() {
    node = node->prev;
    return *this;
  }
  self operator--(int) {
    self tmp = *this;
    node = node->prev;
    return tmp;
  }
  bool operator==(const self& other) const { return node == other.node; }
  bool operator!=(const self& other) const { return node != other.node; }
};

template <typename T>
class list {
 public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;

  using iterator = list_iterator<T, T&, T*>;
  using const_iterator = list_iterator<T, const T&, const T*>;

 private:
  size_type _size;
  list_node_base head;  // 哨兵节点

 public:
  list() : _size(0) {
    head.next = &head;
    head.prev = &head;
  }
  list(size_type n, const value_type& value) : _size(0) {
    head.next = &head;
    head.prev = &head;
    for (size_type i = 0; i < n; ++i) {
      push_back(value);
    }
  }

  // 拷贝构造函数
  list(const list& other) : _size(0) {
    // _size初始化为0，因为push_back会计数
    head.next = &head;
    head.prev = &head;
    for (const value_type& value : other) {
      push_back(value);
    }
  }

  // 移动构造
  list(list&& other)  noexcept : _size(other._size) {
    if (other._size == 0) {
      head.next = head.prev = &head;
    } else {
      head.next = other.head.next;
      head.prev = other.head.prev;
      head.next->prev = &head;
      head.prev->next = &head;

      other.head.next = other.head.prev = &other.head;
      other._size = 0;
    }
  }

  void push_back(const value_type& value) {
    list_node<T>* node = new list_node<T>;
    node->data = value;
    node->next = &head;
    node->prev = head.prev;
    head.prev->next =
        node;          // head.prev：最后一个节点，->:最后一个节点指向插入的节点
    head.prev = node;  // 最后一个节点是node
    ++_size;
  }

  ~list() {
    while (head.next != &head) {
      list_node<T>* node = static_cast<list_node<T>*>(head.next);
      head.next = node->next;
      delete node;
    }
  }

  iterator begin() { return iterator(head.next); }
  iterator end() { return iterator(&head); }

  // 粗暴写法，不够STL
  // 应该修改 list_iterator 支持 const 节点指针
  /*
    template <class T, class Ref, class Ptr>
    struct list_iterator {
    using base_ptr = typename std::conditional<
        std::is_const<Ref>::value,
        const list_node_base*,
        list_node_base*
    >::type;
    base_ptr node;
    };
  */
  const_iterator begin() const {
    return const_iterator(const_cast<list_node_base*>(head.next));
  }
  const_iterator end() const {
    return const_iterator(const_cast<list_node_base*>(&head));
  }

  size_type size() const { return _size; }
  bool empty() const { return _size == 0; }
};

}  // namespace mystl

#endif