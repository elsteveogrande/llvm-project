//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_EXPERIMENTAL_STACKTRACE_PROCESS
#define _LIBCPP_EXPERIMENTAL_STACKTRACE_PROCESS

#include "resolver.h"
#include "tracer.h"

#include <__config>
#include <cstddef>
#include <memory>

namespace std::__stacktrace_support {

struct binary;
struct module;

struct process {
    std::shared_ptr<tracer> tracer_;
    std::shared_ptr<resolver> resolver_;

    process();

    static std::shared_ptr<process> current_process();
};

}  // namespace std::__stacktrace_support

#endif
