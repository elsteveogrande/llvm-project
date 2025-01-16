//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <__config>

#include <experimental/stacktrace>

namespace {}

_LIBCPP_BEGIN_NAMESPACE_STD

// TODO!  This is using a very simple and unportable method which walks the stack
// back from the current frame address.  This will eventually use a proper library
// instead: unwind, execinfo(?), and probably some parts stolen from llvm::Symbolizer.

_LIBCPP_EXPORTED_FROM_ABI // TODO Get this symbol to appear in libc++.a without this macro
void
__stacktrace_helper::__populate(std::function<void(size_t)> __resize_lam,
                                std::function<void(size_t, uintptr_t)> __set_addr_lam,
                                size_t __skip,
                                size_t __max_depth) {
  void** fp;

  if (!__max_depth) {
    // zero depth means "no limit".  We'll find the actual depth and use that.
    // Although this is doing a stack walk twice, it'll reduce vector allocations.
    auto skip = __skip;
    fp        = (void**)__builtin_frame_address(0);
    while (fp && skip--) {
      fp = (void**)fp[0];
    }
    while (fp) {
      fp = (void**)fp[0];
      ++__max_depth;
    }
  }

  __resize_lam(__max_depth);

  fp = (void**)__builtin_frame_address(0);
  while (fp && __skip--) {
    fp = (void**)fp[0];
  }
  size_t __index = 0;
  while (fp && __max_depth--) {
    // Address of the instruction that called us.
    // We get that by obtaining the return address and backing up one byte
    // so we're on the last byte of the previous insn.
    auto* ptr = fp[1];
    if (!ptr) {
      break;
    }
    uintptr_t __insn = ((uintptr_t)ptr) - 1;
    printf("@@@ %lu <- %lx\n", __index, __insn);
    __set_addr_lam(__index, __insn);
    ++__index;
    fp = (void**)fp[0];
  }
}

_LIBCPP_END_NAMESPACE_STD
