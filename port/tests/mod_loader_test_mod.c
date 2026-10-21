/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * A mod for mod_loader_test.cpp, built as mods are (a shared library against
 * sdk/include/openrac/openrac_mod.h alone), twice: as test-a and test-b (TEST_MOD_ID). Its
 * settings say what it does: "behaviour" (ok, fail, unknown_hook, bad_signature, input) and "tag", a
 * number it adds to what it returns and reports through the test's function "Test_Record". With
 * "input" it also takes the keyboard, asks for the cursor captured, and writes into its pad reports
 * whether W is held and how far the mouse moved (interface 1.2). */
#include <stddef.h>

#ifndef TEST_MOD_ID
#define TEST_MOD_ID "test-a"
#endif

#include "openrac/openrac_mod.h"

static const openrac_api* s_api;
static openrac_mod* s_mod;
static int (*s_add)(int);
static void (*s_record)(int);
static int s_tag;
static int s_frames;
static int s_input;

OPENRAC_MOD_EXPORT void openrac_mod_query(openrac_mod_info* info) {
    info->api_major = OPENRAC_MOD_API_MAJOR;
    info->api_minor = OPENRAC_MOD_API_MINOR;
    info->id = TEST_MOD_ID;
    info->version = "1.0.0";
}

static int add(int x) {
    return s_add(x) * 10 + s_tag;
}

static void on_frame(void* user) {
    (void)user;
    s_frames++;
    if (s_record != NULL) {
        s_record(s_tag);
        /* A hook point taken after load is refused. */
        if (s_frames == 1) {
            void* original = NULL;
            s_record(1000 + -s_api->hook(s_mod, "Test_Add", "i32(i32)", (void*)add, &original));
        }
    }
}

static void on_pad(void* user, int port, uint8_t* report, int size) {
    (void)user;
    if (port == 0 && size > 0) {
        report[0] = (uint8_t)(report[0] + s_tag);
    }
    if (s_input && size >= 3) {
        const openrac_input* in = s_api->input();
        report[1] = (uint8_t)((in->keys[26] & OPENRAC_KEY_HELD) ? 0x57 : 0); /* W */
        report[2] = (uint8_t)(in->mouse_dx + 0.5f);
        s_api->capture_mouse(s_mod, in->overlay ? 0 : 1);
    }
}

OPENRAC_MOD_EXPORT int openrac_mod_load(const openrac_api* api, openrac_mod* mod) {
    s_api = api;
    s_frames = 0;
    s_mod = mod;
    s_input = 0;
    s_tag = (int)api->setting_int(mod, "tag");
    s_record = (void (*)(int))api->function("Test_Record");
    const char* behaviour = api->setting_string(mod, "behaviour");
    api->log(mod, OPENRAC_LOG_INFO, behaviour);
    if (behaviour[0] == 'f') { /* fail */
        api->fail(mod, "this mod refuses on purpose");
        return 1;
    }
    if (behaviour[0] == 'u') { /* unknown_hook: carries on regardless; the loader does not */
        void* original = NULL;
        api->hook(mod, "Test_Nothing", "void()", (void*)add, &original);
        return 0;
    }
    if (behaviour[0] == 'b') { /* bad_signature */
        void* original = NULL;
        return api->hook(mod, "Test_Add", "void(i32)", (void*)add, &original) == OPENRAC_HOOK_SIGNATURE ? 2 : 0;
    }
    if (behaviour[0] == 'i') { /* input */
        s_input = 1;
        if (api->api_minor < 2 || api->take_input(mod, OPENRAC_INPUT_KEYBOARD) != 0) {
            return 4;
        }
        api->capture_mouse(mod, 1);
    }
    if (api->hook(mod, "Test_Add", "i32(i32)", (void*)add, (void**)&s_add) != OPENRAC_HOOK_OK) {
        return 3;
    }
    api->on_frame(mod, on_frame, NULL);
    api->on_pad(mod, on_pad, NULL);
    return 0;
}
