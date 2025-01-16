#pragma once

#include <cassert>
#include <functional>
#include <unistd.h>

#include "Memory.h"
#include "Symbol.h"

namespace llvm::objreader {

struct SymbolTable {
  virtual ~SymbolTable() = default;
  virtual void getSymbols(std::function<void(Symbol &)> cb) = 0;
};

struct CachedSymbolTable : SymbolTable {
  Memory::Map<uintptr_t, Symbol> byAddr;

  virtual ~CachedSymbolTable() = default;

  explicit CachedSymbolTable(SymbolTable &&wrapped) {
    wrapped.getSymbols([this](Symbol &s) { byAddr[s.addr] = s; });
  }

  void getSymbols(std::function<void(Symbol &)> cb) override {
    for (auto &[addr, sym] : byAddr) {
      cb(sym);
    }
  }
};

class ObjectFile;

struct MachO64SymbolTable : SymbolTable {
  virtual ~MachO64SymbolTable() = default;

  ObjectFile *objectFile_{nullptr};

  void getSymbols(std::function<void(Symbol &)> cb) override {}
};

} // namespace llvm::objreader
