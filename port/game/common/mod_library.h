// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A shared library opened at run time, the same on each system (dlopen, LoadLibraryW), for the mod
// loader (mods.cpp). Kept apart so that windows.h, which mod_library.cpp includes on Windows, meets
// none of the port's own names.

#pragma once

#include <filesystem>
#include <string>

namespace openrac::mods::library {

// The library at `path`, its symbols bound now; null, and the system's reason, when it cannot be.
void* open(const std::filesystem::path& path, std::string& error);
// An exported symbol, or null.
void* symbol(void* library, const char* name);
void close(void* library);
// Where the system loaded the library holding `address` (a function of it), or null: what a
// crash report compares a backtrace's frames with (common/crash.h, ModImage).
const void* base_of(const void* address);

}  // namespace openrac::mods::library
