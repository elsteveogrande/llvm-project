#pragma once

#include "../ObjectReader.h"
#include "../util/Memory.h"
#include <cassert>
#include <cstddef>
#include <map>

namespace llvm::objreader {

struct Image {
  std::string_view name; // usually the path to binary file
  uintptr_t addr;
  intptr_t slide;
  bool isMainProgram{false};
};

struct Images {
  ObjectReader &reader_;
  Memory::List<Image> images_;
  Memory::Map<std::string_view, Image *> byName_;
  Memory::Map<uintptr_t, Image *> byAddr_;

  virtual ~Images() = default;
  Images(ObjectReader &reader)
      : reader_{reader}, images_{reader_.mem_}, byName_{reader_.mem_},
        byAddr_{reader_.mem_} {}

  void add(Image &&image) {
    auto &im = images_.emplace_back(std::move(image));
    byName_[im.name] = &im;
    byAddr_[im.addr] = &im;
  }

  Image *atAddr(uintptr_t addr) {
    auto it = byAddr_.upper_bound(addr);
    if (it == byAddr_.end()) {
      return nullptr;
    }
    --it;
    if (it == byAddr_.end()) {
      return nullptr;
    }
    return it->second;
  }

  Image *mainProg() {
    for (auto &im : images_) {
      // Linear search, but main prog is usually the first image
      if (im.isMainProgram) {
        return &im;
      }
    }
    return nullptr;
  }
};

struct OSXImages : public Images {
  virtual ~OSXImages() = default;
  OSXImages(ObjectReader &reader);
};

} // namespace llvm::objreader
