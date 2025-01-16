#pragma once

#include <iostream>
#include <string_view>

namespace llvm::objreader {

struct SourceLoc final {
  uintptr_t addr_;
  std::string_view dir_;
  std::string_view file_;
  unsigned line_;

  operator bool() { return addr_ || line_; }

  friend std::ostream &operator<<(std::ostream &os, SourceLoc &s) {
    if (s) {
      // TODO: path sep char
      os << "[SourceLoc " << s.dir_ << '/' << s.file_ << ':' << s.line_ << ']';
    } else {
      os << "[SourceLoc (empty)]";
    }
    return os;
  }
};

} // namespace llvm::objreader
