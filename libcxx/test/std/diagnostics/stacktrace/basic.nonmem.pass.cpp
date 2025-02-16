//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20

#include <experimental/stacktrace>

#include <cassert>

/*
  (19.6.4.6) Non-member functions

  template<class Allocator>
    void swap(basic_stacktrace<Allocator>& a, basic_stacktrace<Allocator>& b)
      noexcept(noexcept(a.swap(b)));

  string to_string(const stacktrace_entry& f);

  template<class Allocator>
    string to_string(const basic_stacktrace<Allocator>& st);

  ostream& operator<<(ostream& os, const stacktrace_entry& f);
  template<class Allocator>
    ostream& operator<<(ostream& os, const basic_stacktrace<Allocator>& st);
*/
int main(int, char**) {
  /*
  template<class Allocator>
  void swap(basic_stacktrace<Allocator>& a, basic_stacktrace<Allocator>& b)
    noexcept(noexcept(a.swap(b)));
  Effects: Equivalent to a.swap(b).
  */
  std::stacktrace empty;
  auto current = std::stacktrace::current();

  std::stacktrace a(empty);
  std::stacktrace b(current);
  assert(a == empty);
  assert(b == current);

  std::swap(a, b);
  assert(a == current);
  assert(b == empty);

  /*
  string to_string(const stacktrace_entry& f);
  Returns: A string with a description of f.
  Recommended practice: The description should provide information about the contained evaluation,
  including information from f.source_file() and f.source_line().
  */
  // TODO

  /*
  template<class Allocator>
  string to_string(const basic_stacktrace<Allocator>& st);
  Returns: A string with a description of st.
  [Note 1: The number of lines is not guaranteed to be equal to st.size(). — end note]
  */
  // TODO

  /*
  ostream& operator<<(ostream& os, const stacktrace_entry& f);
  Effects: Equivalent to: return os << to_string(f);
  */
  // TODO

  /*
  template<class Allocator>
    ostream& operator<<(ostream& os, const basic_stacktrace<Allocator>& st);
  Effects: Equivalent to: return os << to_string(st);
  */
  // TODO

  return 0;
}
