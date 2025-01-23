//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_EXPERIMENTAL_STACKTRACE_RESOLVER
#define _LIBCPP_EXPERIMENTAL_STACKTRACE_RESOLVER

#include <__config>

namespace std::__stacktrace_support {

struct resolver {
    ~resolver();
};

}  // namespace std::__stacktrace_support

#endif
