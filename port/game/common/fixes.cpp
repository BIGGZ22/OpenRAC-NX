// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "fixes.h"

#include <cstring>

#include "common/log.h"

namespace openrac::fixes {

// The generated table (cmake/Differences.cmake): kTableCount entries; kTable has at least one, a
// blank one when the game has no fixes.
extern const Fix kTable[];
extern const int kTableCount;

namespace {

unsigned char g_on[kMax];
bool g_ready = false;

// The defaults, the first time anything asks (before settings::load, or without settings at all:
// a program without a window).
void ready() {
    if (!g_ready) {
        g_ready = true;
        reset();
    }
}

}  // namespace

int count() {
    return kTableCount < kMax ? kTableCount : kMax;
}

const Fix& fix(int index) {
    return kTable[index >= 0 && index < count() ? index : 0];
}

int find(const char* key) {
    if (key == nullptr) {
        return -1;
    }
    for (int i = 0; i < count(); ++i) {
        if (std::strcmp(kTable[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

bool on(int index) {
    ready();
    return index >= 0 && index < count() && g_on[index] != 0;
}

void set(int index, bool on) {
    ready();
    if (index >= 0 && index < count()) {
        g_on[index] = on ? 1 : 0;
    }
}

void reset() {
    g_ready = true;
    for (int i = 0; i < kMax; ++i) {
        g_on[i] = i < count() && kTable[i].on ? 1 : 0;
    }
}

const unsigned char* flag(int index) {
    ready();
    return index >= 0 && index < count() ? &g_on[index] : nullptr;
}

}  // namespace openrac::fixes

// game_host.h: a fix's flag by its key. A key the table does not have (a fix of another disc, or a
// typing mistake) is the port's behaviour, always on, and said once in the log.
extern "C" const unsigned char* openrac_game_fix(const char* key) {
    static const unsigned char kAlwaysOn = 1;
    const int index = openrac::fixes::find(key);
    if (index < 0) {
        openrac::log::warn("game fix {} is not in this disc's list of differences: always on", key ? key : "(none)");
        return &kAlwaysOn;
    }
    return openrac::fixes::flag(index);
}
