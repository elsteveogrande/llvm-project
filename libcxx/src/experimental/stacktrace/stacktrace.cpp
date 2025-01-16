//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <experimental/stacktrace>
#include <libunwind.h>
#include <sstream>

#include "libobjreader/Memory.h"
#include "libobjreader/ObjectReader.h"

_LIBCPP_BEGIN_NAMESPACE_STD

inline namespace __stacktrace {

_LIBCPP_NO_TAIL_CALLS _LIBCPP_TEMPLATE_VIS _LIBCPP_NOINLINE void
__build_entries(std::function<void(stacktrace_entry&&)> __append, size_t __skip, size_t __max_depth) {
  if (!__max_depth) [[unlikely]] {
    return;
  }
  unw_context_t __uc;                // Init a context which holds
  unw_getcontext(&__uc);             // current cpu state (incl. frame pointer)
  unw_cursor_t __cur;                // Init a cursor which contains CPU registers and other state;
  unw_init_local(&__cur, &__uc);     // initially its IP value will refer to this `__current` function.
  int __result = 1;                  // Success if > 0; end of stack if 0; negative if some other error
  while (__skip-- && __result > 0) { // While there are any to skip (this is > 0),
    __result = unw_step(&__cur);     // step right over these top `skip` frames
  }
  while (__max_depth-- && __result > 0) {      // Until we hit the max or get UNW_STEP_END or some error
    __result = unw_step(&__cur);               // Keep walking down the stack
    if (__result >= 0) {                       // Non-negative: no error
      stacktrace_entry __entry;                // Start building our new entry
      __stacktrace_access __accessor{__entry}; // (we need this helper-friend to access private state)
      unw_word_t __ip;                         // We'll fetch the calling instruction's address.
      unw_get_reg(&__cur, UNW_REG_IP, &__ip);  // Get the address of either call instruction,
      if (!unw_is_signal_frame(&__cur)) {      // ... or whatever instruction triggered a signal.
        --__ip;                                // If a call insn, ip is just *after* that insn; go to previous
      }
      __accessor.addr() = (uintptr_t)__ip; // We have an address, so we have a valid frame
      __append(std::move(__entry));
    }
  }
}

void __populate_entries(stacktrace_entry* entries,
                        size_t size,
                        std::function<std::byte*(size_t)> alloc_func,
                        std::function<void(std::byte*, size_t)> dealloc_func) {
  llvm::objreader::Memory memory{alloc_func, dealloc_func};
  llvm::objreader::ObjectReader objReader{memory};
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
   * 0xaaaabbbbcccc: _Z2ns10DemangledNameI_StvectorISt_allocatorXYZ
   * 0xaaaabbbbcccc: ns::DemangledName(int, std::vector<int, std::allocator<int>>&)
   * 0xaaaabbbbcccc: ns::DemangledName(int, std::vector<int, std::allocator<int>>&): /full/path/to/foo.cc:42
   */

  constexpr static int __k_addr_width = (sizeof(void*) > 4) ? 12 : 8;
  // Although 64-bit addresses are 16 nibbles long, they're often <= 0x7fff_ffff_ffff
  __ss << "0x" << std::hex << std::setfill('0') << std::setw(__k_addr_width) << __entry.native_handle();
  if (!__entry.description().empty()) {
    // The symbol name; ideally it has been demangled
    __ss << ": " << __entry.description();
  }
  if (__entry.source_line()) {
    __ss << ": " << __entry.source_file() << ":" << __entry.source_line();
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
