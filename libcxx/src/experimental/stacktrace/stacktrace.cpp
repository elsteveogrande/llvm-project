//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "collector/collector.h"

#include <__config>
#include <cstddef>
#include <experimental/__stacktrace/entry.h>
#include <experimental/stacktrace>

_LIBCPP_BEGIN_NAMESPACE_STD

stacktrace_entry::~stacktrace_entry() noexcept = default;

_LIBCPP_HIDE_FROM_ABI _LIBCPP_NOINLINE void stacktrace_entry::__current_entries(
        function<void(size_t)> __resize_func,
        function<void(size_t, stacktrace_entry&&)> __assign_func,
        size_t __skip,
        size_t __max_depth) {

    std::__stacktrace_support::collector coll;

    ++__skip;  // increment once more, to omit this `__current_entries` frame

    if (!__max_depth) { __max_depth = coll.height(__skip); }
    __resize_func(__max_depth);

    size_t index = 0;
    auto callback = [__assign_func, &index](uintptr_t insn_addr) {
        std::stacktrace_entry entry {insn_addr, {}};
        __assign_func(index++, std::move(entry));
    };
    coll.collect(callback, __skip, __max_depth);
    __resize_func(index);
}

namespace {}

_LIBCPP_END_NAMESPACE_STD
