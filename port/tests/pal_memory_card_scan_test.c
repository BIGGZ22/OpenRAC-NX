/* SPDX-License-Identifier: GPL-3.0-or-later
 * Compile after generating rac1-pal: see run_pal_memory_card_scan_test.py.
 * Runs the real translated PAL directory-validation state, with synthetic
 * save payloads. Other gameplay services abort if unexpectedly called. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openrac/game_host.h"
#include "openrac/game_lib.h"
#include OPENRAC_PAL_MEMCARD_SOURCE

uint8_t* openrac_guest_base;
gaddr openrac_guest_sp = GUEST_STACK_TOP;
gaddr openrac_relocate_low, openrac_relocate_high;
const char* openrac_game_card_dir;
int openrac_game_no_card;
gaddr openrac_guest_frame(uint32_t size) {
    return openrac_guest_sp -= (size + 15u) & ~15u;
}
#define UNEXPECTED abort()
gaddr openrac_relocate_data(gaddr a) { UNEXPECTED; }
openrac_host_fn openrac_guest_function(gaddr a) { UNEXPECTED; }
gaddr openrac_guest_nearest(const uint32_t* a, int b, gaddr c) { UNEXPECTED; }
gaddr openrac_folded_call(const uint32_t* a, int b, gaddr c, uint32_t d, int e) { UNEXPECTED; }
int func_00121A80(gaddr a) { UNEXPECTED; }
int func_00122598(int a) { UNEXPECTED; }
void func_0012D818(gaddr a) { UNEXPECTED; }
void func_001E9730(int a, ...) { UNEXPECTED; }
void func_001F9A00(gaddr a, gaddr b, int c) { UNEXPECTED; }
int func_001FDF10(unsigned int a, gaddr b, gaddr c) { UNEXPECTED; }
void func_00208338(int a) { UNEXPECTED; }
void func_00208FA0(void) { UNEXPECTED; }
void func_00209CE8(int a) { UNEXPECTED; }
int func_00217628(int a, int b, int c) { UNEXPECTED; }
int func_002176C8(int a, int b, int c) { UNEXPECTED; }
short func_00217748(int a) { UNEXPECTED; }
int func_00116248(gaddr a, gaddr b, ...) { UNEXPECTED; }
int func_001151B4(gaddr a, gaddr b, unsigned int n) { return memcmp(G(a), G(b), n); }
gaddr func_001153FC(gaddr a, int b, unsigned int n) { memset(G(a), b, n); return a; }
int func_001165B8(gaddr a, gaddr b) { return strcmp(G(a), G(b)); }
gaddr func_001166FC(gaddr a, gaddr b) { strcpy(G(a), G(b)); return a; }
int func_00116948(gaddr a, gaddr b, int n) { return strncmp(G(a), G(b), n); }
gaddr func_00116B00(gaddr a, gaddr b, int n) { strncpy(G(a), G(b), n); return a; }
/* Same PAL address bindings as libraries.tsv; file operations use real libmc. */
int func_001236F0(void) { return openrac_lib_sceMcInit(); }
int func_001238B0(int p, int s, gaddr n, int m) { return openrac_lib_sceMcOpen(p, s, n, m); }
int func_001239D8(int p, int s, gaddr n) { return openrac_lib_sceMcMkdir(p, s, n); }
int func_00123A10(int f) { return openrac_lib_sceMcClose(f); }
int func_00123AC8(int f, int o, int w) { return openrac_lib_sceMcSeek(f, o, w); }
int func_00123C30(int f, gaddr b, int n) { return openrac_lib_sceMcRead(f, b, n); }
int func_00123D48(int f, gaddr b, int n) { return openrac_lib_sceMcWrite(f, b, n); }
int func_00123F30(int m, gaddr c, gaddr r) { return openrac_lib_sceMcSync(m, c, r); }
int func_00124068(int p, int s, gaddr t, gaddr f, gaddr m) { return openrac_lib_sceMcGetInfo(p, s, t, f, m); }
int func_001241F0(int p, int s, gaddr n, unsigned int m, int max, gaddr t) { return openrac_lib_sceMcGetDir(p, s, n, m, max, t); }
int func_00124340(int p, int s) { return openrac_lib_sceMcFormat(p, s); }
int func_00124410(int p, int s, gaddr n) { return openrac_lib_sceMcDelete(p, s, n); }
int func_00124528(int p, int s) { return openrac_lib_sceMcUnformat(p, s); }

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%d: %s\n", __LINE__, #x); exit(1); } } while (0)
static int result(void) {
    CHECK(openrac_lib_sceMcSync(0, 0x2000, 0x2004) == 1);
    return GREF(int, 0x2004);
}
static void scan(int expected_slot) {
    McState_209E68* state = G(0x0013D390);
    memset(state, 0, sizeof *state);
    state->state = 7;
    func_00209E68(); /* GetDir */
    CHECK(state->state == 8 && state->busy);
    func_00209E68(); /* Sync */
    CHECK(!state->busy);
    func_00209E68(); /* Retail game's exact entry/count/block validation */
    CHECK(state->card[0].x14 == expected_slot);
    if (expected_slot == -1) CHECK(state->sub == 1 && !state->busy);
}
int main(int argc, char** argv) {
    CHECK(argc == 3);
    openrac_game_card_dir = argv[1];
    openrac_guest_base = calloc(1, OPENRAC_COMPACT_SIZE);
    CHECK(openrac_guest_base);
    openrac_game_mc_state(0x00132EA8, 0x0015B0C0);
    /* Retail string locations, synthetic names: no disc data required. */
    strcpy(G(0x0013D2E8), "/PALTEST/*");
    strcpy(G(0x0013D315), "icon.sys");
    strcpy(G(0x0013D335), "icon.icn");
    strcpy(G(0x0013D355), "extras");
    strcpy(G(0x0015FF58), "save");
    strcpy(G(0x0015FF60), ".bin");
    if (strcmp(argv[2], "write") == 0) {
        strcpy(G(0x1000), "/PALTEST");
        CHECK(openrac_lib_sceMcMkdir(0, 0, 0x1000) == 0);
        int made = result(); CHECK(made == 0 || made == -4);
        /* 344 payload clusters + six directory clusters = 350. */
        const char* names[] = {"icon.sys", "icon.icn", "extras", "save0.bin", "save1.bin", "save2.bin", "save3.bin", "save4.bin"};
        const int sizes[] = {1, 3, 15, 65, 65, 65, 65, 65};
        for (int i = 0; i < 8; ++i) {
            snprintf(G(0x1000), 128, "/PALTEST/%s", names[i]);
            CHECK(openrac_lib_sceMcOpen(0, 0, 0x1000, 0x203) == 0);
            int fd = result(); CHECK(fd >= 0);
            memset(G(0x400000), i + 1, sizes[i] * 1024);
            CHECK(openrac_lib_sceMcWrite(fd, 0x400000, sizes[i] * 1024) == 0);
            CHECK(result() == sizes[i] * 1024);
            CHECK(openrac_lib_sceMcClose(fd) == 0 && result() == 0);
        }
    }
    scan(-1); /* Existing card accepted, proceeds to reading slots. */
    /* Simulate the old Switch listing: eight real files, no dot entries.
     * The SAME game validator must reject this, proving the regression. */
    McState_209E68* state = G(0x0013D390);
    state->state = 8; state->sub = 0; state->busy = 0; state->result = 8;
    memmove(G(0x001A0980), G(0x001A0980 + 128), 8 * 64);
    func_00209E68();
    CHECK(state->card[0].x14 == -2);
    scan(-1);
    /* Read back a distinctive payload after a fresh process, too. */
    strcpy(G(0x1000), "/PALTEST/save3.bin");
    CHECK(openrac_lib_sceMcOpen(0, 0, 0x1000, 1) == 0);
    int fd = result(); CHECK(fd >= 0);
    CHECK(openrac_lib_sceMcRead(fd, 0x400000, 65 * 1024) == 0);
    CHECK(result() == 65 * 1024);
    for (int i = 0; i < 65 * 1024; ++i) CHECK(GREF(unsigned char, 0x400000 + i) == 7);
    CHECK(openrac_lib_sceMcClose(fd) == 0 && result() == 0);
    puts("PAL retail save directory accepted; missing-dot regression rejected; payload reloaded");
    free(openrac_guest_base);
    return 0;
}
