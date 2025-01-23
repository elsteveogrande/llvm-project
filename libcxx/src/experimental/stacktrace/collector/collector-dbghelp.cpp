//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "collector.h"

#ifdef _LIBCXX_STACKTRACE_USING_DBGHELP
namespace std::__stacktrace_support {

collector::collector() {}

collector::~collector() {}

}  // namespace std::__stacktrace_support
#endif
