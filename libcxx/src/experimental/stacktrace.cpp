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
    size_t __max_depth) _LIBCPP_NOINLINE {
  ++__skip; // increment once more, to omit this `__current_entries` frame
  auto current_proc = std::__stacktrace_support::process::current_process();
  if (!__max_depth) {
    __max_depth = current_proc->call_stack_height(__skip);
  }
  __resize_func(__max_depth);

  size_t index  = 0;
  auto callback = [__assign_func, &index](uintptr_t insn_addr) {
    std::stacktrace_entry entry{insn_addr, {}};
    __assign_func(index++, std::move(entry));
  };
  current_proc->call_stack_addrs(callback, __skip, __max_depth);
  __resize_func(index);
}

namespace {}

_LIBCPP_END_NAMESPACE_STD
