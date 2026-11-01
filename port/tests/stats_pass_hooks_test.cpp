// SPDX-License-Identifier: GPL-3.0-or-later
// stats::PassHooks (common/stats.h): kGpu sections reach an installed backend's pass marks whether
// or not stats are enabled, in nesting order; other sections and an uninstalled hook cost nothing.
#include "common/stats.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
using namespace openrac;
static std::vector<std::string> events;
int main() {
    { const stats::Scope plain("no hooks", stats::kGpu); }
    assert(events.empty());
    stats::set_pass_hooks({[](std::string_view n) { events.emplace_back("begin " + std::string(n)); },
                           [] { events.emplace_back("end"); }});
    assert(!stats::enabled());
    {
        const stats::Scope outer("outer", stats::kGpu);
        { const stats::Scope cpu_only("cpu only"); }
        { const stats::Scope inner("inner", stats::kGpu); }
    }
    const std::vector<std::string> want{"begin outer", "begin inner", "end", "end"};
    assert(events == want);
    stats::set_pass_hooks({});
    events.clear();
    { const stats::Scope after("after removal", stats::kGpu); }
    assert(events.empty());
    std::puts("PASS: kGpu sections forward begin/end to installed pass hooks, nested, independent of enabled()");
}
