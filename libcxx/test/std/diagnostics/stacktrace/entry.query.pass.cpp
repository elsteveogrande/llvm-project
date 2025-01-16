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
  std::stacktrace_entry entry0;
  auto const entry = entry0;

  // [T8]
  // string description() const;
  auto desc = entry.description();
  assert(desc.c_str()[0] == 0);
  assert(desc.size() == 0);

  // [T9]
  // string source_file() const;
  auto src = entry.source_file();
  assert(src.c_str()[0] == 0);
  assert(src.size() == 0);

  // [T10]
  // uint_least32_t source_line() const;
  assert(entry.source_line() == 0);

  return 0;
}
