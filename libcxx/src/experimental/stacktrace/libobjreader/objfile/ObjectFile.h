#pragma once

#include "../ObjectReader.h"
#include "../util/Bytes.h"
#include "../util/Memory.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <utility>

namespace llvm::objreader {

struct DebugInfo;
struct SectionTable;
struct Section;
struct SymbolTable;

struct ObjectFile : public Bytes {
  ObjectReader &reader_;
  std::string_view path_;
  int fd_;
  void *mmap_;
  size_t size_;

  // These are initially null, and lazily built when first requested.
  Memory::Unique<DebugInfo> debugInfo_;
  Memory::Unique<SectionTable> sectionTable_;
  Memory::Unique<SymbolTable> symbolTable_;

  /** The executable file, if `this` is a DWO, dSYM, PDB, ...; `nullptr`
   * otherwise */
  ObjectFile *mainObjectFile_;

  ObjectFile(ObjectReader &reader, std::string_view path, int fd, void *mmap,
             size_t size);

  virtual Memory::Unique<DebugInfo> genDebugInfo() = 0;
  virtual Memory::Unique<SectionTable> genSectionTable() = 0;
  virtual Memory::Unique<SymbolTable> genSymbolTable() = 0;

  enum class Type {
    kMachO32,
    kMachO64,
    kELF32,
    kELF64,
    kPE,
    kPDB,
  };
  static std::string_view typeString(Type t) {
    constexpr static std::array<std::pair<Type, std::string_view>, 6>
        kTypeNames{
            {{Type::kMachO32, "kMachO32"},
             {Type::kMachO64, "kMachO64"},
             {Type::kELF32, "kELF32"},
             {Type::kELF64, "kELF64"},
             {Type::kPE, "kPE"},
             {Type::kPDB, "kPDB"}},
        };
    for (auto const &[type, name] : kTypeNames) {
      if (type == t) {
        return name;
      }
    }
    __builtin_unreachable();
  }

  virtual ~ObjectFile();
  virtual Type type() = 0;

  virtual Bytes header() = 0;

  std::string_view path() const { return path_; }

  Bytes sectionContent(Section &sec);

  /** Get this file's section table; this is lazily computed. */
  SectionTable &sectionTable();

  /** Get this file's symbol table; this is lazily computed. */
  SymbolTable &symbolTable();

  /** Get this file's debug info; this is lazily computed. */
  DebugInfo &debugInfo();
};

class MachO64File : public ObjectFile {
protected:
  Memory::Unique<SectionTable> genSectionTable() override;
  Memory::Unique<SymbolTable> genSymbolTable() override;
  Memory::Unique<DebugInfo> genDebugInfo() override;

public:
  virtual ~MachO64File() = default;

  MachO64File(ObjectReader &reader, std::string_view path, int fd, void *mmap,
              size_t size);

  Type type() override { return Type::kMachO64; }
  Bytes header() override;
};

} // namespace llvm::objreader
