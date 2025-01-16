#pragma once

#include <cassert>
#include <functional>

#include "Section.h"

namespace llvm::objreader {

struct SectionTable {
  virtual ~SectionTable() = default;
  virtual void getSections(std::function<void(Section &)> cb) = 0;
};

class ObjectFile;

struct MachO64SectionTable : SectionTable {
  virtual ~MachO64SectionTable() = default;

  ObjectFile *objectFile_{nullptr};

  void getSections(std::function<void(Section &)> cb) override {}
};

} // namespace llvm::objreader
