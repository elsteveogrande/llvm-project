#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <fcntl.h>
#include <iostream>
#include <string_view>
#include <sys/mman.h>
#include <unistd.h>

namespace llvm::objreader {

struct Section final {
  std::string_view name; // (un-owned) name string
  uintptr_t virt_addr;   // where this is loaded in the process's address space
  size_t virt_size;      // size in memory
  size_t binary_offset;  // where in the binary this lives
  size_t binary_size; // for loaded sections, bytes occupied in its binary file

  operator bool() { return !name.empty(); }

  friend std::ostream &operator<<(std::ostream &os, Section &s) {
    if (s) {
      os << '[' << typeid(s).name() << " name:" << s.name << ']';
    } else {
      os << '[' << typeid(s).name() << " (uninitialized)]";
    }
    return os;
  }
};

} // namespace llvm::objreader
