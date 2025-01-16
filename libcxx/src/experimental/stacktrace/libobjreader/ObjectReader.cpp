#include "ObjectReader.h"

#include "image/Image.h"
#include "objfile/ObjectFile.h"
#include "util/Memory.h"
#include <cerrno>
#include <iostream>

namespace llvm::objreader {

std::optional<ObjectFile *> ObjectReader::open(std::string_view path) {
  auto it = openFiles_.find(path);
  if (it != openFiles_.end()) {
    return it->second.get();
  }

  std::optional<ObjectFile *> ret{};

  int fd = ::open(path.data(), O_RDONLY);
  if (fd == -1) {
    // std::cerr << "open failed: " << path << ": " << strerror(errno)
    //           << std::endl;
    return ret;
  }
  auto size = size_t(lseek(fd, 0, SEEK_END));
  if (size < 64) {
    close(fd);
    std::cerr << "too small: " << path << std::endl;
    return ret;
  }
  auto *mmap = ::mmap(nullptr, size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
  if (!mmap) {
    close(fd);
    std::cerr << "mmap failed: " << path << ": " << strerror(errno)
              << std::endl;
    return ret;
  }

  Memory::Unique<ObjectFile> ref;

  uint32_t magic32 = *(uint32_t *)mmap;
  if (magic32 == 0xfeedfacf) {
    ref = mem_.makeUnique<MachO64File>(*this, path, fd, mmap, size);
  } else {
    ::munmap(mmap, size);
    close(fd);
    std::cerr << "could not identify: " << path << std::endl;
  }

  assert(ref);
  ret = ref.get();
  openFiles_.emplace(path, std::move(ref));
  return ret;
}

Memory::Unique<Images> ObjectReader::images() {
  return mem_.makeUnique<OSXImages>(*this);
}

} // namespace llvm::objreader
