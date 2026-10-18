// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// backtrace() and its two neighbours for Android below API 33 (execinfo.h beside this file), over
// the unwinder every NDK program links (_Unwind_Backtrace) and the dynamic linker's dladdr.

#include "execinfo.h"

#if __ANDROID_API__ < 33

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unwind.h>

struct walk {
    void** buffer;
    int size;
    int count;
};

static _Unwind_Reason_Code step(struct _Unwind_Context* context, void* arg) {
    struct walk* w = (struct walk*)arg;
    const uintptr_t pc = _Unwind_GetIP(context);
    if (pc == 0) {
        return _URC_NO_REASON;
    }
    if (w->count >= w->size) {
        return _URC_END_OF_STACK;
    }
    w->buffer[w->count++] = (void*)pc;
    return _URC_NO_REASON;
}

int backtrace(void** buffer, int size) {
    if (buffer == NULL || size <= 0) {
        return 0;
    }
    struct walk w = {buffer, size, 0};
    _Unwind_Backtrace(step, &w);
    return w.count;
}

// One frame's line into `out` (at most `room` bytes, terminated); its length.
static int describe(void* address, char* out, size_t room) {
    Dl_info info;
    memset(&info, 0, sizeof info);
    if (dladdr(address, &info) == 0 || info.dli_fname == NULL) {
        return snprintf(out, room, "[%p]", address);
    }
    if (info.dli_sname != NULL) {
        const uintptr_t offset = (uintptr_t)address - (uintptr_t)info.dli_saddr;
        return snprintf(out, room, "%s(%s+0x%lx) [%p]", info.dli_fname, info.dli_sname,
                        (unsigned long)offset, address);
    }
    const uintptr_t offset = (uintptr_t)address - (uintptr_t)info.dli_fbase;
    return snprintf(out, room, "%s(+0x%lx) [%p]", info.dli_fname, (unsigned long)offset, address);
}

enum { kLine = 512 };

char** backtrace_symbols(void* const* buffer, int size) {
    if (buffer == NULL || size <= 0) {
        return NULL;
    }
    // glibc's shape: the pointers, then the strings they point at, in one allocation.
    const size_t table = (size_t)size * sizeof(char*);
    char** out = (char**)malloc(table + (size_t)size * kLine);
    if (out == NULL) {
        return NULL;
    }
    char* text = (char*)out + table;
    for (int i = 0; i < size; ++i) {
        out[i] = text + (size_t)i * kLine;
        describe(buffer[i], out[i], kLine);
    }
    return out;
}

void backtrace_symbols_fd(void* const* buffer, int size, int fd) {
    char line[kLine];
    for (int i = 0; i < size; ++i) {
        int n = describe(buffer[i], line, sizeof line - 1);
        if (n < 0) {
            continue;
        }
        if (n > (int)sizeof line - 2) {
            n = (int)sizeof line - 2;
        }
        line[n] = '\n';
        if (write(fd, line, (size_t)n + 1) < 0) {
            return;
        }
    }
}

#endif
