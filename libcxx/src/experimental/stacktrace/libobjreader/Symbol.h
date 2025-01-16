#pragma once

#include "Section.h"
#include <cstdint>
#include <string_view>

namespace llvm::objreader {

struct Symbol final {
  uint64_t addr{};
  std::string_view name{};
  Section section{};

  operator bool() { return !name.empty(); }

  friend std::ostream &operator<<(std::ostream &os, Symbol &s) {
    if (s) {
      os << "[Symbol"                         //
         << ' ' << (void *)s.addr             //
         << " [sec " << s.section.name << "]" //
         << ' ' << s.name                     //
         << ']';
    } else {
      os << '[' << typeid(s).name() << " (empty)]";
    }
    return os;
  }
};

} // namespace llvm::objreader
