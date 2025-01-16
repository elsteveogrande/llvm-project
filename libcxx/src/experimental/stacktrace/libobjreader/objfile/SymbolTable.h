#pragma once

#include <cassert>

#include "../ObjectReader.h"
#include "../objfile/ObjectFile.h"
#include "../util/Memory.h"
#include "Symbol.h"

namespace llvm::objreader {

struct SymbolTable final {
  ObjectReader &reader_;
  Memory::Map<uintptr_t, Symbol> byAddr_;
  Memory::Map<std::string_view, uintptr_t> byName_;

  explicit SymbolTable(ObjectReader &reader)
      : reader_(reader), byAddr_(reader_.mem_), byName_(reader_.mem_) {
    byAddr_.emplace(0, Symbol{});  // low sentinel
    byAddr_.emplace(~0, Symbol{}); // high sentinel
  }

  void add(Symbol &&sym) {
    auto addr = sym.addr;
    auto name = sym.name;
    byAddr_.emplace(addr, std::move(sym));
    byName_.emplace(name, addr);
  }

  void eachSymbol(std::function<void(Symbol &)> cb) {
    for (auto &[addr, sym] : byAddr_) {
      cb(sym);
    }
  }

  Symbol *atAddress(uintptr_t addr) {
    auto it = byAddr_.upper_bound(addr);
    if (it == byAddr_.end()) {
      return nullptr;
    }
    --it;
    return &it->second;
  }
};

} // namespace llvm::objreader
