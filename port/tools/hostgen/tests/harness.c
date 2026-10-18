/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The smallest runtime that runs hostgen's output, for its tests: game
 * memory as port/runtime lays it out (POSIX only), and the guest.h calls.
 * The port itself links port/runtime instead. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openrac/guest.h"

#include <sys/mman.h>

uint8_t* openrac_guest_base;
gaddr openrac_guest_sp = GUEST_STACK_TOP;

static gaddr strings_next = GUEST_STRINGS;
static gaddr statics_next = GUEST_STATICS;

gaddr openrac_guest_frame(uint32_t size) {
    openrac_guest_sp = (openrac_guest_sp - size) & ~0xFu;
    if (openrac_guest_sp < GUEST_STACK_BOTTOM) {
        fprintf(stderr, "game stack full\n");
        exit(2);
    }
    return openrac_guest_sp;
}

gaddr openrac_guest_static(uint32_t size) {
    gaddr at = statics_next;
    statics_next = (at + size + 15) & ~15u;
    return at;
}

gaddr openrac_guest_string(const char* s, size_t size) {
    gaddr at = strings_next;
    memcpy(G(at), s, size);
    strings_next = (at + (gaddr)size + 3) & ~3u;
    return at;
}

#define MAX_FUNCTIONS 4096

static struct {
    int overlay;
    gaddr address;
    openrac_host_fn fn;
} table[MAX_FUNCTIONS];

static size_t table_count;
static int current_overlay = OPENRAC_OVERLAY_EXE;

void openrac_guest_register(int overlay, const openrac_fn_entry* entries, size_t count) {
    for (size_t i = 0; i < count && table_count < MAX_FUNCTIONS; i++) {
        table[table_count].overlay = overlay;
        table[table_count].address = entries[i].address;
        table[table_count].fn = entries[i].fn;
        table_count++;
    }
}

void openrac_guest_set_overlay(int overlay) {
    current_overlay = overlay;
}

int openrac_guest_overlay(void) {
    return current_overlay;
}

openrac_host_fn openrac_guest_function(gaddr address) {
    for (size_t i = 0; i < table_count; i++) {
        if (table[i].address == address && table[i].overlay == current_overlay) {
            return table[i].fn;
        }
    }
    for (size_t i = 0; i < table_count; i++) {
        if (table[i].address == address && table[i].overlay == OPENRAC_OVERLAY_EXE) {
            return table[i].fn;
        }
    }
    fprintf(stderr, "no function at %08x\n", address);
    exit(3);
}

const char* openrac_guest_function_name(gaddr address) {
    (void)address;
    return NULL;
}

void openrac_guest_missing(const char* name) {
    printf("missing %s\n", name);
}

void openrac_guest_report_missing(void) {}

/* A site's first call (guest.h): a stub's is a missing function; a zero-filled call prints its
 * site, which the tests read. */
void openrac_guest_reach_first(openrac_reach_site* site) {
    if (strcmp(site->kind, "zero-args") == 0) {
        printf("reach %s\n", site->name);
    } else {
        openrac_guest_missing(site->name);
    }
}

/* What the tests' C calls to print (host functions; names other than func_
 * get the game_ prefix in the port, port/tools/hostgen/program.py). */
void game_test_print(int v) {
    printf("%d\n", v);
}

void game_test_print_float(float v) {
    printf("%.3f\n", v);
}

void game_test_print_str(gaddr s) {
    printf("%s\n", (const char*)G(s));
}

void openrac_game_register_functions(void);
int game_test_main(void);

int main(void) {
    void* base =
        mmap(NULL, 0x100000000ull, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (base == MAP_FAILED) {
        return 4;
    }
    openrac_guest_base = base;
    mprotect(openrac_guest_base, GUEST_RAM_SIZE, PROT_READ | PROT_WRITE);
    mprotect(openrac_guest_base + GUEST_PORT, GUEST_PORT_SIZE, PROT_READ | PROT_WRITE);
    openrac_game_register_functions();
    return game_test_main();
}
