//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <cxxabi.h>
#include <experimental/stacktrace>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

#if __has_include(<unwind.h>)
#  define _LIBCXX_STACKTRACE_USE_UNWIND
#  include <unwind.h>
#endif

#include "libobjreader/ObjectReader.h"
#include "libobjreader/debug/DebugInfo.h"
#include "libobjreader/image/Image.h"
#include "libobjreader/objfile/SymbolTable.h"
#include "libobjreader/util/Memory.h"

_LIBCPP_BEGIN_NAMESPACE_STD

inline namespace __stacktrace {

/** Build stacktrace_entry vector and populate only instruction addresses. */
_LIBCPP_NO_TAIL_CALLS _LIBCPP_TEMPLATE_VIS _LIBCPP_NOINLINE void
__build_entries(std::function<void(stacktrace_entry&&)> __append, size_t __skip, size_t __max_depth) {
  if (!__max_depth) [[unlikely]] {
    return;
  }

#ifdef _LIBCXX_STACKTRACE_USE_UNWIND
  struct trace_fn {
    std::function<void(stacktrace_entry&&)> append;
    size_t skip;
    size_t max_depth;
    static _Unwind_Reason_Code operator()(_Unwind_Context* context, void* _self) {
      auto* self = (trace_fn*)_self;
      if (self->skip) {
        --self->skip;
        return _URC_NO_REASON;
      }
      if (!self->max_depth) {
        return _URC_END_OF_STACK;
      }
      --self->max_depth;

      int ipBefore;
      uintptr_t addr = _Unwind_GetIPInfo(context, &ipBefore);
      std::stacktrace_entry entry;
      __stacktrace_access(entry).addr() = addr;
      self->append(std::move(entry));
      return _URC_NO_REASON;
    }
  };

  trace_fn tf{__append, 1 + __skip, __max_depth};
  _Unwind_Backtrace(trace_fn::operator(), &tf);
#endif
}

/** Using entries from `build_entries` (and their addresses), resolve symbol names and source locations. */
void __populate_entries(stacktrace_entry* entries,
                        size_t size,
                        std::function<std::byte*(size_t)> alloc_func,
                        std::function<void(std::byte*, size_t)> dealloc_func) {
  llvm::objreader::Memory memory{alloc_func, dealloc_func};
  llvm::objreader::ObjectReader objReader{memory};
  auto procImages = objReader.images();

  for (size_t i = 0; i < size; i++) {
    auto& e = entries[i];
    if (!e) {
      continue;
    }
    auto image = procImages->atAddr(e.native_handle());
    if (!image) {
      continue;
    }
    auto unslid  = e.native_handle() - image->slide;
    auto objFile = objReader.open(image->name);
    if (!objFile) {
      continue;
    }
    auto& symTable = (*objFile)->symbolTable();
    auto sym       = symTable.atAddress(unslid);
    if (!sym) {
      continue;
    }
    if (sym->name.empty()) {
      continue;
    }

    auto access = __stacktrace_access(e);

    // We were able to find the symbol which contains this address.
    // Try to demangle it, and use the demangled name in the stacktrace_entry.
    // If that fails (or if it's not mangled) use the original symbol string.
    char* buffer  = (char*)alloc_func(512);
    size_t length = 512;
    int status    = 0;
    __cxxabiv1::__cxa_demangle(sym->name.data(), buffer, &length, &status);
    access.symbol() = status ? sym->name : buffer;

    // Now we have to find debugging (DWARF or PDB) data.
    auto& di = (*objFile)->debugInfo();
    auto loc = di.sourceLocAtAddr(unslid);
    if (loc) {
      access.file() = loc.file_;
      access.line() = loc.line_;
    }
  }
}

/*
 * `to_string` Helpers
 */

void __stacktrace_to_string::operator()(stringstream& __ss, stacktrace_entry const& __entry) {
  /*
   * Return a single line similar to one of the following, depending on what information
   * we were able to collect (accounting for failures in demangling and in looking up source locations).
   * This will not append a newline character.
   *
   * 0xaaaabbbbcccc
   * 0xaaaabbbbcccc: _Z2ns10MangledNameI_StvectorISt_allocatorXYZ
   * 0xaaaabbbbcccc: ns::DemangledName(int, std::vector<int, std::allocator<int>>&)
   * 0xaaaabbbbcccc: ns::DemangledName(int, std::vector<int, std::allocator<int>>&): /full/path/to/foo.cc:42
   */

  // Although 64-bit addresses are 16 nibbles long, they're often <= 0x7fff_ffff_ffff
  constexpr static int __k_addr_width = (sizeof(void*) > 4) ? 12 : 8;

  __ss << "0x" << std::hex << std::setfill('0') << std::setw(__k_addr_width) << __entry.native_handle();
  if (!__entry.description().empty()) {
    // The symbol name; ideally it has been demangled
    __ss << ": " << __entry.description();
  }
  if (__entry.source_line()) {
    __ss << ": " << __entry.source_file() << ":" << std::dec << __entry.source_line();
  }
}

void __stacktrace_to_string::operator()(stringstream& __ss, stacktrace_entry const* __entries, size_t __count) {
  if (!__count) {
    __ss << "(empty stacktrace)";
  } else {
    for (size_t __i = 0; __i < __count; __i++) {
      if (__i) {
        // Insert newlines between entries (but not before the first or after the last)
        __ss << std::endl;
      }
      (*this)(__ss, __entries[__i]);
    }
  }
}

string __stacktrace_to_string::operator()(stacktrace_entry const& __entry) {
  stringstream __ss;
  (*this)(__ss, __entry);
  return __ss.str();
}

string __stacktrace_to_string::operator()(stacktrace_entry const* __entries, size_t __count) {
  stringstream __ss;
  (*this)(__ss, __entries, __count);
  return __ss.str();
}

uintptr_t& __stacktrace_access::addr() { return __entry.__addr_; }
string& __stacktrace_access::symbol() { return __entry.__symbol_; }
string& __stacktrace_access::file() { return __entry.__file_; }
uint32_t& __stacktrace_access::line() { return __entry.__line_; }

} // namespace __stacktrace

_LIBCPP_END_NAMESPACE_STD
