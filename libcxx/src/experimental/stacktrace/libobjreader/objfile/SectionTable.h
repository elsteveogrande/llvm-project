#pragma once

#include <cassert>

#include "../ObjectReader.h"
#include "../objfile/ObjectFile.h"
#include "../objfile/Section.h"
#include "../util/Memory.h"

namespace llvm::objreader {

struct SectionTable final {
  ObjectReader &reader_;
  Memory::List<Section> sections_;
  Memory::Vec<Section *> byIndex_;
  Memory::Map<uintptr_t, Section *> byAddr_;
  Memory::Map<std::string_view, Section *> byName_;

  explicit SectionTable(ObjectReader &reader)
      : reader_(reader), sections_(reader_.mem_), byIndex_(reader_.mem_),
        byAddr_(reader_.mem_), byName_(reader_.mem_) {
    // Dummy entry at index 0 so things like `NO_SECT` work
    byIndex_.push_back(nullptr);
    // Low and high sentinels in address map; use dummy entry
    byAddr_.emplace(0, nullptr);
    byAddr_.emplace(~0, nullptr);
  }

  void add(Section &&s) {
    if (!s.virt_size || !s.virt_addr ||
        (byAddr_.find(s.virt_addr) != byAddr_.end())) {
      return;
    }
    sections_.emplace_back(std::move(s));
    auto *sec = &sections_.back();
    byIndex_.push_back(sec);
    byAddr_[s.virt_addr] = sec;
    byName_[s.name] = sec;
  }

  void eachSection(std::function<void(Section &)> cb) {
    for (auto &sec : sections_) {
      cb(sec);
    }
  }

  Section *atIndex(uint32_t index) { return byIndex_.at(index); }

  Section *atAddress(uintptr_t addr) {
    auto it = byAddr_.upper_bound(addr);
    --it;
    return it->second;
  }

  Section *byName(std::string_view name) {
    auto it = byName_.find(name);
    return (it != byName_.end()) ? it->second : nullptr;
  }
};

} // namespace llvm::objreader
