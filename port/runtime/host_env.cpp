// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The C entry point to the port's environment variables (openrac/host_env.h): a name C gives is
// looked up in common/env.h's table, and the value the table kept is handed back as a C string.

#include "openrac/host_env.h"

#include <functional>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <string_view>

#include "common/env.h"
#include "common/log.h"

namespace openrac::runtime {
namespace {

// Reports `name`, a variable the table does not have, the first time it is asked for: a caller
// that asks at each disc read must not fill the log.
void report_unknown(std::string_view name) {
    // The names already reported, for the life of the program; the lock lets any thread ask.
    static std::mutex lock;
    static std::set<std::string, std::less<>> reported;

    const std::scoped_lock held(lock);
    if (reported.contains(name)) {
        return;
    }
    reported.emplace(name);
    log::error("host_env: {} is not in common/env.h's table; it reads as unset", name);
}

}  // namespace
}  // namespace openrac::runtime

extern "C" const char* openrac_env_string(const char* name) {
    // A null name names no variable; it is the same programming error as an unknown one.
    if (name == nullptr) {
        openrac::runtime::report_unknown("(null)");
        return nullptr;
    }

    const std::optional<openrac::env::Var> var = openrac::env::find(name);
    if (!var) {
        openrac::runtime::report_unknown(name);
        return nullptr;
    }

    // The kept value ends in a NUL and lives as long as the program (env.h), so C can hold it.
    const std::optional<std::string_view> value = openrac::env::value(*var);
    return value ? value->data() : nullptr;
}
