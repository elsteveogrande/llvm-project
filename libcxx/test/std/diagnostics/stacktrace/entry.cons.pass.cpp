//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20

#include <cassert>
#include <experimental/stacktrace>

/*
  (19.6.3.2) Constructors [stacktrace.entry.cons]

namespace std {
  class stacktrace_entry {
  public:
    // [stacktrace.entry.cons], constructors
    constexpr stacktrace_entry() noexcept;                                          // [T2]
    constexpr stacktrace_entry(const stacktrace_entry& other) noexcept;             // [T3]
    constexpr stacktrace_entry& operator=(const stacktrace_entry& other) noexcept;  // [T4]
    ~stacktrace_entry();                                                            // [T5]

    [. . .]
  };
}
*/

int main(int, char**) {
  // [T2]
  // constexpr stacktrace_entry() noexcept;
  // "Postconditions: *this is empty."
  static_assert(std::is_default_constructible_v<std::stacktrace_entry>);
  static_assert(std::is_nothrow_default_constructible_v<std::stacktrace_entry>);
  std::stacktrace_entry entryT2;
  assert(!entryT2);

  // [T3]
  // constexpr stacktrace_entry(const stacktrace_entry& other) noexcept;
  static_assert(std::is_nothrow_copy_constructible_v<std::stacktrace_entry>);
  std::stacktrace_entry entryT3(entryT2);

  // [T4]
  // constexpr stacktrace_entry& operator=(const stacktrace_entry& other) noexcept;
  static_assert(std::is_nothrow_copy_assignable_v<std::stacktrace_entry>);
  std::stacktrace_entry entryT4;
  entryT4 = entryT2;

  // [T5]
  // ~stacktrace_entry();
  std::stacktrace_entry* entryPtr{nullptr};
  delete entryPtr;
  {
    auto entryT5(entryT4); /* construct and immediately let it go out of scope */
  }

  return 0;
}
