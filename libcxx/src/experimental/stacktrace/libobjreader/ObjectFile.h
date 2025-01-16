#pragma once

#include <filesystem>

#include "ByteRange.h"
#include "Memory.h"
#include "SectionTable.h"
#include "SymbolTable.h"

namespace llvm::objreader {

class ObjectFile : public ByteRange {
  enum class Type {
    kInvalid,
    kMachO64,
  };

  Memory &memory_;
  std::filesystem::path path_;
  Type const type_{Type::kInvalid};
  Memory::Unique<SectionTable> sectionTable_;
  Memory::Unique<SymbolTable> symbolTable_;

public:
  virtual ~ObjectFile() = default;

  ObjectFile(Memory &memory, std::filesystem::path path);

  operator bool() const { return type_ != Type::kInvalid; }

  SectionTable &sectionTable() {
    if (!sectionTable_) {
      switch (type_) {
      case Type::kMachO64:
        sectionTable_ = memory_.makeUnique<MachO64SectionTable>();
        break;
      case Type::kInvalid:
      default:
        throw std::logic_error("not a valid ObjectFile");
      }
    }
    return *sectionTable_;
  }

  SymbolTable &symbolTable() {
    if (!symbolTable_) {
      switch (type_) {
      case Type::kMachO64:
        symbolTable_ = memory_.makeUnique<MachO64SymbolTable>();
        break;
      case Type::kInvalid:
      default:
        throw std::logic_error("not a valid ObjectFile");
      }
    }
    return *symbolTable_;
  }
};

} // namespace llvm::objreader
