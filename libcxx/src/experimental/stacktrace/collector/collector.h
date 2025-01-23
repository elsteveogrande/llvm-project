//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_EXPERIMENTAL_STACKTRACE_TRACE
#define _LIBCPP_EXPERIMENTAL_STACKTRACE_TRACE

#include <__config>
#include <cstddef>
#include <functional>

#if __has_include(<windows.h>) && __has_include(<dbghelp.h>)
#include <windows.h>
// windows.h has to go first
#include <dbghelp.h>
#define _LIBCXX_STACKTRACE_USING_DBGHELP

#elif __has_include(<unwind.h>)
#include <unwind.h>
#define _LIBCXX_STACKTRACE_USING_UNWIND

#elif __has_include(<libunwind.h>)
#include <libunwind.h>
#define _LIBCXX_STACKTRACE_USING_LIBUNWIND

#else
#error Cannot support stacktrace, need one of: unwind, libunwind, dbghelp
#endif

namespace std::__stacktrace_support {

struct collector final {
    collector();
    ~collector();
    collector(collector const&) = delete;

    size_t height(size_t skip);

    using addr_callback = std::function<void(uintptr_t)>;
    void collect(addr_callback cb, size_t skip, size_t max_depth);
};

}  // namespace std::__stacktrace_support

#endif  // _LIBCPP_EXPERIMENTAL_STACKTRACE_TRACE
