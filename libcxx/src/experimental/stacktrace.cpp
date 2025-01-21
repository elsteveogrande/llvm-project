//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <__config>
#include <cstddef>
#include <functional>

#include <experimental/__stacktrace/entry.h>
#include <experimental/stacktrace>

#include "stacktrace/binary.h"
#include "stacktrace/module.h"
#include "stacktrace/process.h"
#include "stacktrace/trace.h"

_LIBCPP_BEGIN_NAMESPACE_STD

stacktrace_entry::~stacktrace_entry() noexcept = default;

_LIBCPP_HIDE_FROM_ABI void stacktrace_entry::__current_entries(
    function<void(size_t)> __resize_func,
    function<void(size_t, stacktrace_entry&&)> __assign_func,
    size_t __skip,
    size_t __max_depth) {
  // TODO!  This is a temporary impl, we'll use an actual library
  // like execinfo, dbghlp.h, ...

  void** fp = (void**)__builtin_frame_address(0);
  while (fp && __skip--) {
    fp = (void**)fp[0];
  }
  if (!fp) {
    return;
  }

  if (!__max_depth) {
    auto tmp_fp = fp;
    while (tmp_fp) {
      tmp_fp = (void**)tmp_fp[0];
      ++__max_depth;
    }
  }

  __resize_func(__max_depth);

  size_t __index = 0;
  while (fp && __max_depth--) {
    // Address of the instruction that called us.
    // We get that by obtaining the return address and backing up one byte
    // so we're on the last byte of the previous insn.
    auto* ptr = fp[1];
    if (!ptr) {
      break;
    }
    auto insn = ((uintptr_t)ptr) - 1;
    std::stacktrace_entry entry{insn, {}};
    __assign_func(__index++, std::move(entry));
    fp = (void**)fp[0];
  }
}

namespace {}

_LIBCPP_END_NAMESPACE_STD
