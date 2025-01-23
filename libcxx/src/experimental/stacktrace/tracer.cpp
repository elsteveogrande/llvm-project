//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#if __has_include(<windows.h>) && __has_include(<dbghelp.h>)
#include <windows.h>
// windows.h has to go first
#include <dbghelp.h>
#define _LIBCXX_STACKTRACE_TRACE_USING_DBGHELP

#elif __has_include(<unwind.h>)
#include <unwind.h>
#define _LIBCXX_STACKTRACE_TRACE_USING_UNWIND

// #elif __has_include(<libunwind.h>)
// #include <libunwind.h>
// #define _LIBCXX_STACKTRACE_TRACE_USING_LIBUNWIND

// #elif __has_include(<execinfo.h>)
// #include <execinfo.h>
// #define _LIBCXX_STACKTRACE_TRACE_USING_EXECINFO
#endif

#include "tracer.h"

namespace std::__stacktrace_support {

tracer::~tracer() = default;

#if defined(_LIBCXX_STACKTRACE_TRACE_USING_DBGHELP)
struct dbghelp_tracer : tracer {};
#endif

#if defined(_LIBCXX_STACKTRACE_TRACE_USING_UNWIND)
struct unwind_tracer final : tracer {

    unwind_tracer() {}
    ~unwind_tracer() override = default;

    struct trace_fn final {
        [[no_unique_address]] std::function<_Unwind_Reason_Code(_Unwind_Context*)> fn;
        static _Unwind_Reason_Code run(_Unwind_Context* cx, void* _this) {
            return ((trace_fn*) _this)->fn(cx);
        }
    };

    size_t call_stack_height(size_t skip) const override {
        size_t ret = 0;
        trace_fn fn {[&ret](_Unwind_Context*) {
            ++ret;
            return _URC_NO_REASON;
        }};
        _Unwind_Backtrace(trace_fn::run, &fn);
        return (ret > skip) ? ret - skip : 0;
    }

    void
    call_stack_addrs(std::function<void(uintptr_t)> callback, size_t skip, size_t max_depth)
            const override {
        size_t index = 0;
        skip += 2;
        trace_fn fn {[&](_Unwind_Context* cx) {
            if (index == max_depth) { return _URC_END_OF_STACK; }
            if (skip && skip--) { return _URC_NO_REASON; }
            int ip_before;
            auto addr = (uintptr_t) _Unwind_GetIPInfo(cx, &ip_before);
            if (!addr) { return _URC_END_OF_STACK; }
            if (ip_before) { --addr; }
            printf("@@@ %ld: %lx\n", index, addr);
            callback(addr);
            ++index;
            return _URC_NO_REASON;
        }};
        _Unwind_Backtrace(trace_fn::run, &fn);
    }
};
#endif

// #if defined(_LIBCXX_STACKTRACE_TRACE_USING_LIBUNWIND)
// struct libunwind_tracer : tracer {};
// #endif

// #if defined(_LIBCXX_STACKTRACE_TRACE_USING_EXECINFO)
// struct execinfo_tracer : tracer {};
// #endif

std::shared_ptr<tracer> tracer::get_tracer() {
#if defined(_LIBCXX_STACKTRACE_TRACE_USING_DBGHELP)
    return std::make_shared<dbghelp_tracer>();
#elif defined(_LIBCXX_STACKTRACE_TRACE_USING_UNWIND)
    return std::make_shared<unwind_tracer>();
#else
    return {nullptr};
#endif
}

}  // namespace std::__stacktrace_support
