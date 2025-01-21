//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <__config>
#include <cstddef>
#include <functional>
#include <memory>

namespace std::__stacktrace_support {

struct binary;
struct module;
struct trace;

struct process {
    process() = default;
    ~process() = default;
    process(process const&) = delete;

    static std::shared_ptr<process> current_process() {
        return std::make_shared<process>();
    }

    [[gnu::noinline]] size_t call_stack_height(size_t skip) const {
        void** fp = (void**) __builtin_frame_address(0);
        while (fp && skip--) { fp = (void**) fp[0]; }
        size_t ret = 0;
        while (fp) {
            fp = (void**) fp[0];
            ++ret;
        }
        return ret;
    }

    [[gnu::noinline]] void call_stack_addrs(
            std::function<void(uintptr_t)> callback, size_t skip, size_t max_depth) const {
        void** fp = (void**) __builtin_frame_address(0);
        while (fp && skip--) { fp = (void**) fp[0]; }
        while (fp && max_depth--) {
            auto insn_addr = ((uintptr_t) fp[1]) - 1;
            callback(insn_addr);
            fp = (void**) fp[0];
        }
    }
};

}  // namespace std::__stacktrace_support
