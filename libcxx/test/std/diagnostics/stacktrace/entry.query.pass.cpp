//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20
// ADDITIONAL_COMPILE_FLAGS: -O0 -g -gsplit-dwarf

#include <experimental/stacktrace>

#include <cassert>
#include <iostream>
#include <string>

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
  uint32_t line_number = __LINE__ + 1; // record where `current` is being called:
  auto trace           = std::stacktrace::current();

  // First entry of this should be `main`.
  e = trace.at(0);
  assert(e);

  std::cout << "main is at:      " << (void*)&main << std::endl;
  std::cout << "e.native_handle: " << (void*)e.native_handle() << std::endl;
  std::cout << "e.description:   " << e.description() << std::endl;
  std::cout << "e.source_file:   " << e.source_file() << std::endl;
  std::cout << "e.source_line:   " << e.source_line() << std::endl;

  std::cout << trace << std::endl;

  assert(e.native_handle());
  assert(e.native_handle() >= (uintptr_t)&main);
  assert(e.description() == "_main");
  assert(e.source_file().contains("entry.query.pass.cpp"));
  assert(e.source_line() == line_number);

  return 0;
}
