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
  (19.6.3.3) Observers [stacktrace.entry.obs]

namespace std {
  class stacktrace_entry {
    // [stacktrace.entry.obs], observers
    constexpr native_handle_type native_handle() const noexcept;                    // [T6]
    constexpr explicit operator bool() const noexcept;                              // [T7]
    
    [. . .]
  };
}
*/

_LIBCPP_BEGIN_NAMESPACE_STD
inline namespace __stacktrace {}
_LIBCPP_END_NAMESPACE_STD

int main(int, char**) {
  // [T6]
  std::stacktrace_entry entry_t6;

  // constexpr native_handle_type native_handle() const noexcept;
  assert(entry_t6.native_handle() == 0);

  // [T7]
  // constexpr explicit operator bool() const noexcept;
  // "Returns: false if and only if *this is empty."
  assert(!entry_t6);

  // Now set addr to something nonzero (using non-public helper API)
  std::__stacktrace_access access_t6{entry_t6};
  access_t6.addr() = (uintptr_t)&main;
  assert(entry_t6.native_handle() == (uintptr_t)&main);
  assert(entry_t6);

  return 0;
}
