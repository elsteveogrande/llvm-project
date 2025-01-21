// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_EXPERIMENTAL_STACKTRACE_ENTRY
#define _LIBCPP_EXPERIMENTAL_STACKTRACE_ENTRY

/*
  Header <stacktrace> synopsis
  (19.6.2)

#include <compare>  // see [compare.syn]

namespace std {
  // [stacktrace.entry], class stacktrace_entry
  class stacktrace_entry;

  // [stacktrace.basic], class template basic_stacktrace
  template<class Allocator>
    class basic_stacktrace;

  // basic_stacktrace typedef-names
  using stacktrace = basic_stacktrace<allocator<stacktrace_entry>>;

  // [stacktrace.basic.nonmem], non-member functions
  template<class Allocator>
    void swap(basic_stacktrace<Allocator>& a, basic_stacktrace<Allocator>& b)
      noexcept(noexcept(a.swap(b)));

  string to_string(const stacktrace_entry& f);

  template<class Allocator>
    string to_string(const basic_stacktrace<Allocator>& st);

  ostream& operator<<(ostream& os, const stacktrace_entry& f);
  template<class Allocator>
    ostream& operator<<(ostream& os, const basic_stacktrace<Allocator>& st);

  // [stacktrace.format], formatting support
  template<> struct formatter<stacktrace_entry>;
  template<class Allocator> struct formatter<basic_stacktrace<Allocator>>;

  namespace pmr {
    using stacktrace = basic_stacktrace<polymorphic_allocator<stacktrace_entry>>;
  }

  // [stacktrace.basic.hash], hash support
  template<class T> struct hash;
  template<> struct hash<stacktrace_entry>;
  template<class Allocator> struct hash<basic_stacktrace<Allocator>>;
}

// [stacktrace.entry]

namespace std {
  class stacktrace_entry {
  public:
    using native_handle_type = implementation-defined;

    // [stacktrace.entry.cons], constructors
    constexpr stacktrace_entry() noexcept;
    constexpr stacktrace_entry(const stacktrace_entry& other) noexcept;
    constexpr stacktrace_entry& operator=(const stacktrace_entry& other) noexcept;

    ~stacktrace_entry();

    // [stacktrace.entry.obs], observers
    constexpr native_handle_type native_handle() const noexcept;
    constexpr explicit operator bool() const noexcept;

    // [stacktrace.entry.query], query
    string description() const;
    string source_file() const;
    uint_least32_t source_line() const;

    // [stacktrace.entry.cmp], comparison
    friend constexpr bool operator==(const stacktrace_entry& x,
                                     const stacktrace_entry& y) noexcept;
    friend constexpr strong_ordering operator<=>(const stacktrace_entry& x,
                                                 const stacktrace_entry& y) noexcept;
  };
}

// [stacktrace.basic]

namespace std {
  template<class Allocator>
  class basic_stacktrace {
  public:
    using value_type = stacktrace_entry;
    using const_reference = const value_type&;
    using reference = value_type&;
    using const_iterator = implementation-defined;  // see [stacktrace.basic.obs]
    using iterator = const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type = implementation-defined;
    using size_type = implementation-defined;
    using allocator_type = Allocator;

    // [stacktrace.basic.cons], creation and assignment
    static basic_stacktrace current(const allocator_type& alloc = allocator_type()) noexcept;
    static basic_stacktrace current(size_type skip,
                                    const allocator_type& alloc = allocator_type()) noexcept;
    static basic_stacktrace current(size_type skip, size_type max_depth,
                                    const allocator_type& alloc = allocator_type()) noexcept;

    basic_stacktrace() noexcept(is_nothrow_default_constructible_v<allocator_type>);
    explicit basic_stacktrace(const allocator_type& alloc) noexcept;

    basic_stacktrace(const basic_stacktrace& other);
    basic_stacktrace(basic_stacktrace&& other) noexcept;
    basic_stacktrace(const basic_stacktrace& other, const allocator_type& alloc);
    basic_stacktrace(basic_stacktrace&& other, const allocator_type& alloc);
    basic_stacktrace& operator=(const basic_stacktrace& other);
    basic_stacktrace& operator=(basic_stacktrace&& other)
      noexcept(allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
        allocator_traits<Allocator>::is_always_equal::value);

    ~basic_stacktrace();

    // [stacktrace.basic.obs], observers
    allocator_type get_allocator() const noexcept;

    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator rend() const noexcept;

    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    const_reverse_iterator crend() const noexcept;

    bool empty() const noexcept;
    size_type size() const noexcept;
    size_type max_size() const noexcept;

    const_reference operator[](size_type) const;
    const_reference at(size_type) const;

    // [stacktrace.basic.cmp], comparisons
    template<class Allocator2>
    friend bool operator==(const basic_stacktrace& x,
                           const basic_stacktrace<Allocator2>& y) noexcept;
    template<class Allocator2>
    friend strong_ordering operator<=>(const basic_stacktrace& x,
                                       const basic_stacktrace<Allocator2>& y) noexcept;

    // [stacktrace.basic.mod], modifiers
    void swap(basic_stacktrace& other)
      noexcept(allocator_traits<Allocator>::propagate_on_container_swap::value ||
        allocator_traits<Allocator>::is_always_equal::value);

  private:
    vector<value_type, allocator_type> frames_;         // exposition only
  };
}

*/
#include <__config>
#include <__cstddef/ptrdiff_t.h>
#include <__cstddef/size_t.h>
#include <__format/formatter.h>
#include <__functional/function.h>
#include <__functional/hash.h>
#include <__fwd/format.h>
#include <__fwd/ostream.h>
#include <__fwd/vector.h>
#include <__iterator/iterator.h>
#include <__iterator/iterator_traits.h>
#include <__iterator/reverse_access.h>
#include <__iterator/reverse_iterator.h>
#include <__memory/allocator.h>
#include <__memory/allocator_traits.h>
#include <__utility/move.h>
#include <__vector/swap.h>
#include <__vector/vector.h>
#include <any>
#include <c++/v1/__config>
#include <cstdint>
#include <exception>
#include <source_location>
#include <string>
#include <version>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

class stacktrace_entry {
public:
  // (19.6.3.1) Overview [stacktrace.entry.overview]
  using native_handle_type = uintptr_t;

  // (19.6.3.2) [stacktrace.entry.cons], constructors
  ~stacktrace_entry() noexcept;
  constexpr stacktrace_entry() noexcept                                   = default;
  constexpr stacktrace_entry(const stacktrace_entry&) noexcept            = default;
  constexpr stacktrace_entry& operator=(const stacktrace_entry&) noexcept = default;

  // (19.6.3.3) [stacktrace.entry.obs], observers
  [[nodiscard]] constexpr native_handle_type native_handle() const noexcept { return __addr_; }
  [[nodiscard]] constexpr explicit operator bool() const noexcept { return __addr_ != 0; }

  // (19.6.3.4) [stacktrace.entry.query], query
  [[nodiscard]] string description() const { return __loc_.function_name(); }
  [[nodiscard]] string source_file() const { return __loc_.file_name(); }
  [[nodiscard]] uint_least32_t source_line() const { return __loc_.line(); }

  // (19.6.3.5) [stacktrace.entry.cmp], comparison
  [[nodiscard]] friend constexpr bool operator==(const stacktrace_entry& x, const stacktrace_entry& y) noexcept {
    return x.__addr_ == y.__addr_;
  }
  [[nodiscard]] friend constexpr strong_ordering
  operator<=>(const stacktrace_entry& x, const stacktrace_entry& y) noexcept {
    return x.__addr_ <=> y.__addr_;
  }

  _LIBCPP_HIDE_FROM_ABI static void __current_entries(
      function<void(size_t)> __resize_func,
      function<void(size_t, stacktrace_entry&&)> __assign_func,
      size_t __skip,
      size_t __max_depth);

private:
  friend struct _LIBCPP_HIDE_FROM_ABI __stacktrace_test_helper;

  native_handle_type __addr_{0};
  source_location __loc_{};

  stacktrace_entry(native_handle_type __addr, source_location __loc) : __addr_(__addr), __loc_(__loc) {}
};

struct _LIBCPP_HIDE_FROM_ABI __stacktrace_test_helper {
  static void __set_addr(stacktrace_entry& __entry, uintptr_t __addr) { __entry.__addr_ = __addr; }
};

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP_EXPERIMENTAL_STACKTRACE_ENTRY
