//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_EXPERIMENTAL_STACKTRACE_COLLECTOR
#define _LIBCPP_EXPERIMENTAL_STACKTRACE_COLLECTOR

#include <__config>
#include <cstddef>
#include <functional>

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

#endif  // _LIBCPP_EXPERIMENTAL_STACKTRACE_COLLECTOR
