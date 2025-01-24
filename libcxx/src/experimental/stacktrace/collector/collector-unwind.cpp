//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../macros.h"
#include "collector.h"

#ifdef _LIBCXX_STACKTRACE_USE_COLLECTOR_UNWIND

#include <__config>
#include <cstddef>
#include <functional>

namespace std::__stacktrace_support {

collector::collector() {}

collector::~collector() {}

struct trace_fn final {
    [[no_unique_address]] std::function<_Unwind_Reason_Code(_Unwind_Context*)> fn;
    static _Unwind_Reason_Code run(_Unwind_Context* cx, void* _this) {
        return ((trace_fn*) _this)->fn(cx);
    }
};

size_t collector::height(size_t skip) {
    size_t ret = 0;
    trace_fn fn {[&ret](_Unwind_Context*) {
        ++ret;
        return _URC_NO_REASON;
    }};
    _Unwind_Backtrace(trace_fn::run, &fn);
    return (ret > skip) ? ret - skip : 0;
}

void collector::collect(addr_callback callback, size_t skip, size_t max_depth) {
    size_t index = 0;
    ++skip;  // omit the call to this function
    trace_fn fn {[&](_Unwind_Context* cx) {
        if (index == max_depth) { return _URC_END_OF_STACK; }
        if (skip && skip--) { return _URC_NO_REASON; }
        int ip_before;
        auto addr = (uintptr_t) _Unwind_GetIPInfo(cx, &ip_before);
        if (!addr) { return _URC_END_OF_STACK; }
        if (ip_before) { --addr; }
        callback(addr);
        ++index;
        return _URC_NO_REASON;
    }};
    _Unwind_Backtrace(trace_fn::run, &fn);
}

}  // namespace std::__stacktrace_support

#endif
