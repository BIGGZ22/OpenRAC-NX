// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// <execinfo.h> for Android below API 33, where bionic has no backtrace(): the port's crash report
// and its debugging aids (common/crash.cpp, runtime/guest.cpp) call these three. From API 33 on
// this is bionic's own header. On the include path of Android builds only (cmake/Android.cmake).

#pragma once

#if __ANDROID_API__ >= 33
#include_next <execinfo.h>
#else

#ifdef __cplusplus
extern "C" {
#endif

// The return addresses of the calling thread's frames, the innermost first (_Unwind_Backtrace).
int backtrace(void** buffer, int size);

// "module(symbol+offset) [address]" for each address (dladdr), in one block the caller frees.
char** backtrace_symbols(void* const* buffer, int size);

// The same lines, written to `fd`; allocates nothing, for a signal handler.
void backtrace_symbols_fd(void* const* buffer, int size, int fd);

#ifdef __cplusplus
}
#endif

#endif
