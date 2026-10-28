// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A shared library opened at run time (mod_library.h).

#include "mod_library.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#ifndef __SWITCH__
#include <dlfcn.h>
#endif
#endif

namespace openrac::mods::library {

#if defined(_WIN32)

void* open(const std::filesystem::path& path, std::string& error) {
    // The mod's own folder is searched for the libraries it needs, not the program's working one.
    HMODULE module = LoadLibraryExW(path.wstring().c_str(), nullptr,
                                    LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (module == nullptr) {
        const DWORD code = GetLastError();
        char text[256] = {};
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code, 0, text,
                           sizeof(text), nullptr) == 0) {
            error = "Windows error " + std::to_string(code);
        } else {
            error = text;
            while (!error.empty() && (error.back() == '\n' || error.back() == '\r' || error.back() == '.')) {
                error.pop_back();
            }
        }
        return nullptr;
    }
    return module;
}

void* symbol(void* library, const char* name) {
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(library), name));
}

void close(void* library) {
    FreeLibrary(static_cast<HMODULE>(library));
}

const void* base_of(const void* address) {
    HMODULE module = nullptr;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           static_cast<LPCSTR>(address), &module) != 0) {
        return module;
    }
    return nullptr;
}

#else
#ifndef __SWITCH__
void* open(const std::filesystem::path& path, std::string& error) {
    void* library = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (library == nullptr) {
        const char* why = dlerror();
        error = why != nullptr ? why : "cannot be opened";
    }
    return library;
}

void* symbol(void* library, const char* name) {
    return dlsym(library, name);
}

void close(void* library) {
    dlclose(library);
}

const void* base_of(const void* address) {
    Dl_info info{};
    return dladdr(address, &info) != 0 ? info.dli_fbase : nullptr;
}
#else
// Nintendo Switch stub implementations
void* open(const std::filesystem::path& path, std::string& error) {
    // No dynamic loading on Nintendo Switch - just return null to indicate failure
    error = "Dynamic library loading not supported on Nintendo Switch";
    return nullptr;
}

void* symbol(void* library, const char* name) {
    // No symbol lookup on Nintendo Switch
    return nullptr;
}

void close(void* library) {
    // Nothing to do on Nintendo Switch
}

const void* base_of(const void* address) {
    // No dynamic loading on Nintendo Switch - no base address available
    return nullptr;
}
#endif
#endif

}  // namespace openrac::mods::library