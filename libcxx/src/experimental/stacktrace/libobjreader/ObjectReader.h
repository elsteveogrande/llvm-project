#pragma once

#include "util/Memory.h"
#include <string_view>

namespace llvm::objreader {

struct ObjectFile;
struct Images;

struct ObjectReader {
  Memory &mem_;
  Memory::Map<std::string_view, Memory::Unique<ObjectFile>> openFiles_;

  explicit ObjectReader(Memory &mem) : mem_(mem), openFiles_(mem_) {}

  // TODO: use <filesystem>?  It's available with C++17 but I'm not sure
  // the library will be linked into libc++
  std::optional<ObjectFile *> open(std::string_view path);

  Memory::Unique<Images> images();
};

} // namespace llvm::objreader
