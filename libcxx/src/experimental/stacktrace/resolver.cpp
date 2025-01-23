//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "resolver.h"

#if __has_include(<windows.h>) && __has_include(<dbghelp.h>)
#include <windows.h>
// windows.h has to go first
#include <dbghelp.h>
#define _LIBCXX_STACKTRACE_RESOLVE_USING_DBGHELP
#endif

#if __has_include(<dlfcn.h>)
#include <dlfcn.h>
#define _LIBCXX_STACKTRACE_RESOLVE_USING_DLFCN
#endif

namespace std::__stacktrace_support {}
