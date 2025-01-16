#include "ObjectFile.h"

#include "../ObjectReader.h"
#include "Section.h"
#include "SectionTable.h"
#include "SymbolTable.h"

namespace llvm::objreader {

ObjectFile::ObjectFile(ObjectReader &reader, std::string_view path, int fd,
                       void *mmap, size_t size)
    : Bytes((std::byte *)mmap, size), reader_(reader), path_(path), fd_(fd),
      mmap_(mmap), size_(size) {}

ObjectFile::~ObjectFile() {
  if (fd_ > 0) {
    if (mmap_) {
      munmap(mmap_, size_);
    }
    close(fd_);
  }
}

Bytes ObjectFile::header() { return slice(0, 64); }

Bytes ObjectFile::sectionContent(Section &sec) {
  return slice(sec.binary_offset, sec.binary_size);
}

SectionTable &ObjectFile::sectionTable() {
  if (!sectionTable_) {
    sectionTable_ = genSectionTable();
  }
  return *sectionTable_;
}

SymbolTable &ObjectFile::symbolTable() {
  if (!symbolTable_) {
    symbolTable_ = genSymbolTable();
  }
  return *symbolTable_;
}

DebugInfo &ObjectFile::debugInfo() {
  if (!debugInfo_) {
    debugInfo_ = genDebugInfo();
  }
  return *debugInfo_;
}

} // namespace llvm::objreader
