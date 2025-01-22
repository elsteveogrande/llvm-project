//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "process.h"

#include <iostream>

namespace std::__stacktrace_support {

std::shared_ptr<process> process::current_process() {
    return std::make_shared<process>();
}

size_t warn_once_no_trace_available() {
    std::cerr << "warning: cannot produce stacktrace" << std::endl;
    return 1;
}

[[gnu::noinline]] size_t process::call_stack_height(size_t skip) const {
    if (!tracer_) {
        static size_t fail = warn_once_no_trace_available();
        return fail;
    }
    return tracer_->call_stack_height(skip);
}

[[gnu::noinline]] void process::call_stack_addrs(
        std::function<void(uintptr_t)> callback, size_t skip, size_t max_depth) const {
    if (!tracer_) { return; }
    tracer_->call_stack_addrs(callback, skip, max_depth);
}

}  // namespace std::__stacktrace_support
