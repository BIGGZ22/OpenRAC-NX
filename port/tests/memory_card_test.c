/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openrac/game_lib.h"
#include "openrac/game_host.h"
static unsigned char memory[8192];
uint8_t* openrac_guest_base = memory;
const char* openrac_game_card_dir;
int openrac_game_no_card;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%d: %s\n", __LINE__, #x); return 1; } } while (0)
static int result(void) {
    if (openrac_lib_sceMcSync(0, 4096, 4100) != 1) return -999;
    return GREF(int, 4100);
}
int main(int argc, char** argv) {
    openrac_game_card_dir = argc > 1 ? argv[1] : "memory-card-test-data/new/SCES_509.16";
    /* Use the game's state words, not only the standalone library fallback. */
    openrac_game_mc_state(6000, 6004);
    CHECK(openrac_lib_sceMcGetInfo(0, 0, 4200, 4204, 4208) == 0);
    CHECK(result() == 0);
    CHECK(GREF(int, 4200) == 2 && GREF(int, 4208) == 1);
    CHECK(openrac_lib_sceMcGetInfo(1, 0, 0, 0, 0) == 0);
    CHECK(result() == -10);
    strcpy((char*)G(256), "/BESCES-50916");
    if (argc < 3 || strcmp(argv[2], "read") != 0) {
        CHECK(openrac_lib_sceMcMkdir(0, 0, 256) == 0);
        int made = result(); CHECK(made == 0 || made == -4);
        strcpy((char*)G(256), "/BESCES-50916/save00");
        CHECK(openrac_lib_sceMcOpen(0, 0, 256, 0x203) == 0);
        int fd = result(); CHECK(fd >= 0);
        memcpy(G(1024), "persistent-progress", 20);
        CHECK(openrac_lib_sceMcWrite(fd, 1024, 20) == 0); CHECK(result() == 20);
        CHECK(openrac_lib_sceMcClose(fd) == 0); CHECK(result() == 0);
    }
    strcpy((char*)G(256), "/BESCES-*");
    CHECK(openrac_lib_sceMcGetDir(0, 0, 256, 0, -1, 0) == 0); CHECK(result() == 1);
    strcpy((char*)G(256), "/BESCES-50916/save00");
    CHECK(openrac_lib_sceMcOpen(0, 0, 256, 1) == 0);
    int fd = result(); CHECK(fd >= 0);
    CHECK(openrac_lib_sceMcRead(fd, 2048, 20) == 0); CHECK(result() == 20);
    CHECK(memcmp(G(2048), "persistent-progress", 20) == 0);
    CHECK(openrac_lib_sceMcClose(fd) == 0); CHECK(result() == 0);
    CHECK(openrac_lib_sceMcClose(fd) == 0); CHECK(result() == -5);
    /* A regular file used as a parent must never appear as an inserted card. */
    char blocked[1200];
    snprintf(blocked, sizeof blocked, "%s/BESCES-50916/save00/child", openrac_game_card_dir);
    openrac_game_card_dir = blocked;
    CHECK(openrac_lib_sceMcGetInfo(0, 0, 0, 0, 0) == 0); CHECK(result() == -10);
    openrac_game_no_card = 1;
    CHECK(openrac_lib_sceMcGetInfo(0, 0, 0, 0, 0) == 0); CHECK(result() == -10);
    puts("Memory card detection, save write, directory scan and reload passed");
    return 0;
}
