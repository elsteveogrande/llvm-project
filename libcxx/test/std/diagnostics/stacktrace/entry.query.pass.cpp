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
#include <iostream>

/*
    (19.6.3.4) Query [stacktrace.entry.query]

namespace std {
  class stacktrace_entry {
  public:
    // [stacktrace.entry.query], query
    string description() const;                                                     // [T8]
    string source_file() const;                                                     // [T9]
    uint_least32_t source_line() const;                                             // [T10]

    [. . .]
  };
}
*/

int main(int, char**) {
  // empty trace entry
  std::stacktrace_entry e;

  // [T8]
  // string description() const;
  auto desc = e.description();
  assert(desc.c_str()[0] == 0);
  assert(desc.size() == 0);

  // [T9]
  // string source_file() const;
  auto src = e.source_file();
  assert(src.c_str()[0] == 0);
  assert(src.size() == 0);

  // [T10]
  // uint_least32_t source_line() const;
  assert(e.source_line() == 0);

  // Get the current trace.
  uint32_t line_number = __LINE__ + 1; // record where `current` is being called
  auto trace           = std::stacktrace::current();

  // First entry of this should be `main`.
  e = trace[0];
  assert(e);
  assert(e.native_handle());
  assert(e.native_handle() >= (uintptr_t)&main);
  assert(e.description() == "main");

  std::cout << "e.description: " << e.description() << '\n';
  std::cout << "e.source_file: " << e.source_file() << '\n';
  std::cout << "e.source_line: " << e.source_line() << '\n';

  return 0;
}
