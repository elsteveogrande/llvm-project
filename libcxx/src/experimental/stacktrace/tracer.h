//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_EXPERIMENTAL_STACKTRACE_TRACER
#define _LIBCPP_EXPERIMENTAL_STACKTRACE_TRACER

#include <__config>
#include <cstddef>
#include <functional>
#include <memory>

namespace std::__stacktrace_support {
struct binary;
struct module;
struct process;

struct tracer {
    virtual ~tracer();
    static std::shared_ptr<tracer> get_tracer();

    // Some environment-dependent tracer implementation should support these:

    virtual size_t call_stack_height(size_t skip) const = 0;

    virtual void
    call_stack_addrs(std::function<void(uintptr_t)> callback, size_t skip, size_t max_depth)
            const = 0;
};

}  // namespace std::__stacktrace_support

#endif
