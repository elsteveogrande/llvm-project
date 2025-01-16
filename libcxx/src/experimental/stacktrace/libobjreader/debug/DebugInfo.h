#pragma once

#include "../objfile/ObjectFile.h"
#include "SourceLoc.h"

namespace llvm::objreader {

struct DebugInfo {
  ObjectReader &reader_;
  ObjectFile &objFile_;

  DebugInfo(ObjectReader &reader, ObjectFile &objFile)
      : reader_(reader), objFile_(objFile) {}

  virtual ~DebugInfo() = default;

  virtual SourceLoc sourceLocAtAddr(uintptr_t addr) = 0;
};

} // namespace llvm::objreader
