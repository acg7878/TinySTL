#ifndef TINYSTL_STRING_H
#define TINYSTL_STRING_H

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace mystl {

// char_traits
template <class CharT>
struct char_traits {
  using char_type = CharT;
  using int_type = int;
  using off_type = std::streamoff;
  using pos_type = std::streampos;
  using state_type = std::mbstate_t;  // 多字节转换状态

  static void assign(char_type& c1, const char_type& c2) noexcept { c1 = c2; }
  static bool eq(char_type c1, char_type c2) noexcept { return c1 == c2; }
  static bool lt(char_type c1, char_type c2) noexcept { return c1 < c2; }

  static int compare(const char_type* s1, const char_type* s2, size_t n) {
    for (size_t i = 0; i < n; ++i) {
      if (lt(s1[i], s2[i]))
        return -1;
      if (lt(s2[i], s1[i]))
        return 1;
    }
    return 0;
  }

  static size_t length(const char_type* s) {
    size_t len = 0;
    while (!eq(s[len], char_type()))
      ++len;
    return len;
  }

  static char_type* copy(char_type* dest, const char_type* src, size_t n) {
    for (size_t i = 0; i < n; ++i)
      dest[i] = src[i];
    return dest;
  }

  static char_type* move(char_type* dest, const char_type* src, size_t n) {
    if (dest < src) {
      for (size_t i = 0; i < n; ++i)
        dest[i] = src[i];
    } else if (dest > src) {
      for (size_t i = n; i > 0; --i)
        dest[i - 1] = src[i - 1];
    }
    return dest;
  }

  static char_type* assign(char_type* s, size_t n, char_type a) {
    for (size_t i = 0; i < n; ++i)
      s[i] = a;
    return s;
  }

  static constexpr char_type to_char_type(int_type c) noexcept {
    return static_cast<char_type>(c);
  }
  static constexpr int_type to_int_type(char_type c) noexcept {
    return static_cast<int_type>(c);
  }
  static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept {
    return c1 == c2;
  }
  static constexpr int_type eof() noexcept { return -1; }
};

// char特化
template <>
struct char_traits<char> {
  using char_type = char;
  using int_type = int;
  using off_type = std::streamoff;
  using pos_type = std::streampos;
  using state_type = std::mbstate_t;

  static void assign(char_type& c1, const char_type& c2) noexcept { c1 = c2; }
  static bool eq(char_type c1, char_type c2) noexcept { return c1 == c2; }
  static bool lt(char_type c1, char_type c2) noexcept { return c1 < c2; }

  static int compare(const char_type* s1, const char_type* s2, size_t n) {
    return std::memcmp(s1, s2, n);
  }
  static size_t length(const char_type* s) { return std::strlen(s); }

  static const char_type* find(const char_type* s, size_t n,
                               const char_type& a) {
    return static_cast<const char_type*>(std::memchr(s, a, n));
  }

  static char_type* move(char_type* s1, const char_type* s2, size_t n) {
    return static_cast<char_type*>(std::memmove(s1, s2, n));
  }

  static char_type* copy(char_type* s1, const char_type* s2, size_t n) {
    return static_cast<char_type*>(std::memcpy(s1, s2, n));
  }

  static char_type* assign(char_type* s, size_t n, char_type a) {
    return static_cast<char_type*>(std::memset(s, a, n));
  }

  static constexpr int_type not_eof(int_type c) noexcept {
    return c != eof() ? c : 0;
  }
  static constexpr char_type to_char_type(int_type c) noexcept {
    return static_cast<char_type>(c);
  }
  static constexpr int_type to_int_type(char_type c) noexcept {
    return static_cast<unsigned char>(c);
  }
  static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept {
    return c1 == c2;
  }
  static constexpr int_type eof() noexcept { return -1; }
};

// basic_string 实现
template <class CharT, class Traits = char_traits<CharT>,
          class Allocator = std::allocator<CharT>>
class basic_string {
 public:
  using traits_type = Traits;
  using value_type = CharT;
  using allocator_type = Allocator;
  using size_type = typename Allocator::size_type;
  using difference_type = typename Allocator::difference_type;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename Allocator::pointer;
  using const_pointer = typename Allocator::const_pointer;

  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  static inline const size_type npos = -1;

 private:
  struct long_rep {
    size_type is_long : 1;
    size_type cap : sizeof(size_type) * 8 - 1;
    size_type size;
    pointer data;
  };

  static constexpr size_type min_cap =
      (sizeof(long_rep) - 1) / sizeof(value_type);

  struct short_rep {
    unsigned char is_long : 1;
    unsigned char size : 7;
    value_type data[min_cap];
  };

  union rep {
    long_rep l;
    short_rep s;
  };

  rep r_;
  allocator_type alloc_;

  // Helper functions
  bool is_long() const noexcept { return r_.s.is_long; }

  void set_short_size(size_type s) noexcept {
    r_.s.size = static_cast<unsigned char>(s);
    r_.s.is_long = 0;
  }

  size_type get_short_size() const noexcept { return r_.s.size; }

  void set_long_cap(size_type c) noexcept {
    r_.l.cap = c;
    r_.l.is_long = 1;
  }

  size_type get_long_cap() const noexcept { return r_.l.cap; }

  void set_long_size(size_type s) noexcept { r_.l.size = s; }
  size_type get_long_size() const noexcept { return r_.l.size; }

  void set_long_pointer(pointer p) noexcept { r_.l.data = p; }
  pointer get_long_pointer() const noexcept { return r_.l.data; }

 public:
  // Constructors
  basic_string() noexcept : alloc_() {
    set_short_size(0);
    r_.s.data[0] = value_type();
  }

  basic_string(const CharT* s) : alloc_() {
    size_type len = Traits::length(s);
    init(s, len);
  }

  basic_string(const CharT* s, size_type count) : alloc_() { init(s, count); }

  basic_string(const basic_string& other) : alloc_(other.alloc_) {
    init(other.data(), other.size());
  }

  basic_string(basic_string&& other) noexcept
      : alloc_(std::move(other.alloc_)) {
    if (other.is_long()) {
      r_.l = other.r_.l;
      other.set_short_size(0);
      other.r_.s.data[0] = value_type();
    } else {
      r_.s = other.r_.s;
    }
  }

  basic_string(size_type count, CharT ch) : alloc_() {
    // Simple initialization
    if (count < min_cap) {
      set_short_size(count);
      Traits::assign(r_.s.data, count, ch);
      r_.s.data[count] = value_type();
    } else {
      init_long(count);
      Traits::assign(get_long_pointer(), count, ch);
      get_long_pointer()[count] = value_type();
      set_long_size(count);
    }
  }

  // Assignment
  basic_string& operator=(const basic_string& other) {
    if (this != &other) {
      assign(other.data(), other.size());
    }
    return *this;
  }

  basic_string& operator=(basic_string&& other) noexcept {
    if (this != &other) {
      clear();
      if (is_long()) {
        alloc_.deallocate(get_long_pointer(), get_long_cap() + 1);
      }
      if (other.is_long()) {
        r_.l = other.r_.l;
        other.set_short_size(0);
        other.r_.s.data[0] = value_type();
      } else {
        r_.s = other.r_.s;
      }
    }
    return *this;
  }

  basic_string& operator=(const CharT* s) {
    assign(s, Traits::length(s));
    return *this;
  }

  // Destructor
  ~basic_string() {
    if (is_long()) {
      alloc_.deallocate(get_long_pointer(), get_long_cap() + 1);
    }
  }

  // Iterators
  iterator begin() noexcept {
    return is_long() ? get_long_pointer() : r_.s.data;
  }
  const_iterator begin() const noexcept {
    return is_long() ? get_long_pointer() : r_.s.data;
  }
  iterator end() noexcept { return begin() + size(); }
  const_iterator end() const noexcept { return begin() + size(); }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  const_iterator cbegin() const noexcept { return begin(); }
  const_iterator cend() const noexcept { return end(); }
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  const_reverse_iterator crend() const noexcept { return rend(); }

  // Capacity
  size_type size() const noexcept {
    return is_long() ? get_long_size() : get_short_size();
  }
  size_type length() const noexcept { return size(); }
  size_type max_size() const noexcept {
    return static_cast<size_type>(-1) / sizeof(CharT) - 1;
  }

  size_type capacity() const noexcept {
    return is_long() ? get_long_cap() : min_cap - 1;
  }

  bool empty() const noexcept { return size() == 0; }

  void reserve(size_type new_cap = 0) {
    if (new_cap <= capacity())
      return;

    pointer new_data = alloc_.allocate(new_cap + 1);
    size_type s = size();

    Traits::copy(new_data, data(), s);
    new_data[s] = value_type();

    if (is_long()) {
      alloc_.deallocate(get_long_pointer(), get_long_cap() + 1);
    }

    set_long_cap(new_cap);
    set_long_size(s);
    set_long_pointer(new_data);
  }

  void resize(size_type count, CharT ch = CharT()) {
    size_type s = size();
    if (count > s) {
      reserve(count);
      Traits::assign(begin() + s, count - s, ch);
    } else if (count < s) {
      // just change size
    }

    if (is_long()) {
      set_long_size(count);
      get_long_pointer()[count] = value_type();
    } else {
      set_short_size(count);
      r_.s.data[count] = value_type();
    }
  }

  void shrink_to_fit() {
    if (is_long() && size() < min_cap) {
      // Downgrade to short string if possible
      size_type s = size();
      short_rep temp_short;
      temp_short.is_long = 0;
      temp_short.size = static_cast<unsigned char>(s);
      Traits::copy(temp_short.data, get_long_pointer(), s);
      temp_short.data[s] = value_type();

      alloc_.deallocate(get_long_pointer(), get_long_cap() + 1);
      r_.s = temp_short;
    }
  }

  // Element access
  reference operator[](size_type pos) { return *(begin() + pos); }
  const_reference operator[](size_type pos) const { return *(begin() + pos); }

  reference at(size_type pos) {
    if (pos >= size())
      throw std::out_of_range("basic_string::at");
    return operator[](pos);
  }
  const_reference at(size_type pos) const {
    if (pos >= size())
      throw std::out_of_range("basic_string::at");
    return operator[](pos);
  }

  reference front() { return *begin(); }
  const_reference front() const { return *begin(); }
  reference back() { return *(end() - 1); }
  const_reference back() const { return *(end() - 1); }

  const CharT* c_str() const noexcept { return data(); }
  const CharT* data() const noexcept {
    return is_long() ? get_long_pointer() : r_.s.data;
  }

  // Modifiers
  void push_back(CharT ch) {
    size_type s = size();
    size_type cap = capacity();
    if (s == cap) {
      reserve(cap == 0 ? min_cap : cap * 2);
    }

    if (is_long()) {
      get_long_pointer()[s] = ch;
      get_long_pointer()[s + 1] = value_type();
      set_long_size(s + 1);
    } else {
      r_.s.data[s] = ch;
      r_.s.data[s + 1] = value_type();
      set_short_size(s + 1);
    }
  }

  void pop_back() {
    if (!empty()) {
      resize(size() - 1);
    }
  }

  basic_string& append(const CharT* s, size_type count) {
    size_type curr_size = size();
    if (curr_size + count > capacity()) {
      reserve(std::max(curr_size + count,
                       capacity() * 2));  // Simplified growth strategy
    }

    CharT* p = is_long() ? get_long_pointer() : r_.s.data;
    Traits::copy(p + curr_size, s, count);

    size_type new_size = curr_size + count;
    p[new_size] = value_type();

    if (is_long())
      set_long_size(new_size);
    else
      set_short_size(new_size);

    return *this;
  }

  basic_string& append(const basic_string& str) {
    return append(str.data(), str.size());
  }

  basic_string& operator+=(const basic_string& str) { return append(str); }
  basic_string& operator+=(CharT ch) {
    push_back(ch);
    return *this;
  }
  basic_string& operator+=(const CharT* s) {
    return append(s, Traits::length(s));
  }

  void clear() noexcept {
    if (is_long()) {
      set_long_size(0);
      get_long_pointer()[0] = value_type();
    } else {
      set_short_size(0);
      r_.s.data[0] = value_type();
    }
  }

  void swap(basic_string& other) noexcept {
    // Simple swap of the union representation
    // (Assuming allocators are swappable or same)
    rep temp = r_;
    r_ = other.r_;
    other.r_ = temp;
  }

  // Search
  size_type find(const CharT* s, size_type pos, size_type count) const {
    size_type sz = size();
    if (pos > sz)
      return npos;
    if (count == 0)
      return pos;

    // Brute force for simplicity, replace with KMP or Boyer-Moore for real usage
    // or just use Traits::find if available (but Traits::find is usually single char)
    const CharT* data_ptr = data();
    for (size_type i = pos; i <= sz - count; ++i) {
      if (Traits::compare(data_ptr + i, s, count) == 0) {
        return i;
      }
    }
    return npos;
  }

  size_type find(const basic_string& str, size_type pos = 0) const {
    return find(str.data(), pos, str.size());
  }

 private:
  void init(const CharT* s, size_type n) {
    if (n < min_cap) {
      set_short_size(n);
      Traits::copy(r_.s.data, s, n);
      r_.s.data[n] = value_type();
    } else {
      init_long(n);
      Traits::copy(get_long_pointer(), s, n);
      get_long_pointer()[n] = value_type();
      set_long_size(n);
    }
  }

  void init_long(size_type n) {
    size_type cap = n;
    pointer p = alloc_.allocate(cap + 1);
    set_long_cap(cap);
    set_long_size(0);  // Set later
    set_long_pointer(p);
  }

  void assign(const CharT* s, size_type n) {
    if (n <= capacity()) {
      CharT* p = is_long() ? get_long_pointer() : r_.s.data;
      Traits::move(p, s, n);
      p[n] = value_type();
      if (is_long())
        set_long_size(n);
      else
        set_short_size(n);
    } else {
      // Reallocate
      pointer new_data = alloc_.allocate(n + 1);
      Traits::copy(new_data, s, n);
      new_data[n] = value_type();

      if (is_long()) {
        alloc_.deallocate(get_long_pointer(), get_long_cap() + 1);
      }

      set_long_cap(n);
      set_long_size(n);
      set_long_pointer(new_data);
    }
  }
};

template <class CharT, class Traits, class Allocator>
void swap(basic_string<CharT, Traits, Allocator>& lhs,
          basic_string<CharT, Traits, Allocator>& rhs) noexcept {
  lhs.swap(rhs);
}

template <class CharT, class Traits, class Allocator>
bool operator==(const basic_string<CharT, Traits, Allocator>& lhs,
                const basic_string<CharT, Traits, Allocator>& rhs) {
  return lhs.size() == rhs.size() &&
         Traits::compare(lhs.data(), rhs.data(), lhs.size()) == 0;
}

template <class CharT, class Traits, class Allocator>
bool operator!=(const basic_string<CharT, Traits, Allocator>& lhs,
                const basic_string<CharT, Traits, Allocator>& rhs) {
  return !(lhs == rhs);
}

template <class CharT, class Traits, class Allocator>
bool operator<(const basic_string<CharT, Traits, Allocator>& lhs,
               const basic_string<CharT, Traits, Allocator>& rhs) {
  return Traits::compare(lhs.data(), rhs.data(),
                         std::min(lhs.size(), rhs.size())) < 0 ||
         (lhs.size() < rhs.size() &&
          Traits::compare(lhs.data(), rhs.data(), lhs.size()) == 0);
}

template <class CharT, class Traits, class Allocator>
std::ostream& operator<<(std::ostream& os,
                         const basic_string<CharT, Traits, Allocator>& str) {
  return os.write(str.data(), str.size());
}

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;

}  // namespace mystl

#endif  // MYSTL_STRING_H
