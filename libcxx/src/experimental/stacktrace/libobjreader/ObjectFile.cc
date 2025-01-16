#include "ObjectFile.h"

namespace llvm::objreader {

ObjectFile::ObjectFile(Memory &memory, std::filesystem::path path)
    : memory_(memory), path_(path) {}

} // namespace llvm::objreader
