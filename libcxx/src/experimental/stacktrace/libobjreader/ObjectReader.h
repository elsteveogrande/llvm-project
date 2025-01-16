#pragma once

#include "Memory.h"
#include "ObjectFile.h"
#include <string_view>

namespace llvm::objreader {

class ObjectReader {
  Memory &memory_;

public:
  explicit ObjectReader(Memory &memory) : memory_(memory) {}
  ObjectFile open(std::string_view path) { return ObjectFile{memory_, path}; }
};

} // namespace llvm::objreader
