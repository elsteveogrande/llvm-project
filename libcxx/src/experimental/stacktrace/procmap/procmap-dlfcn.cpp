//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../macros.h"

#ifdef _LIBCXX_STACKTRACE_USE_PROCMAP_DLFCN

#include "procmap.h"

#include <__config>
#include <cstddef>
#include <cstdlib>
#include <cxxabi.h>
#include <dlfcn.h>
#include <string>

namespace std::__stacktrace_support {

procmap::procmap() {}

procmap::~procmap() {}

std::string procmap::resolve_symbol(uintptr_t addr, bool demangle) {
    Dl_info info;
    dladdr((void*) addr, &info);
    if (dlerror()) { return ""; }
    if (!info.dli_sname) { return ""; }

    char const* symbol = info.dli_sname;
    if (!symbol) { return ""; }

    if (demangle) {
        int st = 0;
        auto* demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, &st);
        if (demangled) {
            std::string ret(demangled);
            free(demangled);
            return ret;
        }
    }

    return symbol;
}

}  // namespace std::__stacktrace_support

#endif
