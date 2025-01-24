//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_EXPERIMENTAL_STACKTRACE_MACROS
#define _LIBCPP_EXPERIMENTAL_STACKTRACE_MACROS

#include <__config>

#if __has_include(<windows.h>)
#include <windows.h>
#define _LIBCXX_STACKTRACE_HAS_WINDOWS_H
#endif

#if __has_include(<dbghelp.h>)
#include <dbghelp.h>
#define _LIBCXX_STACKTRACE_HAS_DBGHELP_H
#endif

#if __has_include(<dlfcn.h>)
#include <dlfcn.h>
#define _LIBCXX_STACKTRACE_HAS_DLFCN_H
#endif

#if __has_include(<unwind.h>)
#include <unwind.h>
#define _LIBCXX_STACKTRACE_HAS_UNWIND_H
#endif

#if __has_include(<libunwind.h>)
#include <libunwind.h>
#define _LIBCXX_STACKTRACE_HAS_LIBUNWIND_H
#endif

#if defined(_LIBCXX_STACKTRACE_HAS_WINDOWS_H) && defined(_LIBCXX_STACKTRACE_HAS_DBGHELP_H)
#define _LIBCXX_STACKTRACE_USE_COLLECTOR_WIN
#elif defined(_LIBCXX_STACKTRACE_HAS_UNWIND_H)
#define _LIBCXX_STACKTRACE_USE_COLLECTOR_UNWIND
#elif defined(_LIBCXX_STACKTRACE_HAS_LIBUNWIND_H)
#define _LIBCXX_STACKTRACE_USE_COLLECTOR_LINUNWIND
#endif

#if defined(_LIBCXX_STACKTRACE_HAS_WINDOWS_H) && defined(_LIBCXX_STACKTRACE_HAS_DBGHELP_H)
#define _LIBCXX_STACKTRACE_USE_PROCMAP_WIN
#elif defined(_LIBCXX_STACKTRACE_HAS_DLFCN_H)
#define _LIBCXX_STACKTRACE_USE_PROCMAP_DLFCN
#endif

#endif
