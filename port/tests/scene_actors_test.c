/* SPDX-License-Identifier: GPL-3.0-or-later */
/* A scene's actors when the moby pool is full (rac1/host/scene_actors.c, PAL's table): the part
 * whose moby CreateMoby cannot make is skipped, nothing is written through the null moby, its slot
 * stays empty for the next run, and the rest is done as the decompilation's C does it. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../game/rac1/host/scene_actors.c"
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); exit(1); } } while (0)

static unsigned char ram[0x200000];
uint8_t* openrac_guest_base = ram;
gaddr openrac_relocate_low = 0, openrac_relocate_high = 0;
gaddr openrac_relocate_data(gaddr a) { return a; }

static gaddr created[4];
static int creates, next_create, warnings;
static char warning[128];

void func_00118D80(int a0) { (void)a0; }
int func_0020C468(int a0, int a1) { (void)a0; (void)a1; return 0; }
gaddr func_0020D348(int a0) {
    (void)a0;
    ++creates;
    return created[next_create++];
}
void openrac_game_warn(const char* message) {
    ++warnings;
    snprintf(warning, sizeof warning, "%s", message);
}

enum { REC = 0x100000, MOBY_A = 0x120000, PARTS_A = 0x121000, MOBY_B = 0x130000, PARTS_B = 0x131000 };
#define STATE 0x0018CC20u

static void part(gaddr at, int id, int end, int count, const int* words) {
    GREF(int, at) = id;
    GREF(int, at + 0xC) = end;
    GREF(unsigned char, at + 0x10 + 0x10) = (unsigned char)count;
    for (int k = 0; k < count; ++k) {
        GREF(int, at + 0x10 + 0x1C + 4u * (unsigned)k) = words[k];
    }
}

static void moby(gaddr m, gaddr parts, unsigned char used, unsigned char flag) {
    GREF(gaddr, m + 0x24) = parts;
    GREF(unsigned short, m + 0x34) = 0x100;
    GREF(unsigned char, parts + 0xC) = used;
    GREF(unsigned char, parts + 0x6) = flag;
}

int main(void) {
    unsigned char low[0x200];
    memset(ram, 0xEE, sizeof low);
    memcpy(low, ram, sizeof low);

    GREF(unsigned short, REC + 0x0) = 0x1234;
    GREF(int, REC + 0x4) = 0x10;
    GREF(unsigned short, REC + 0x8) = 0x55AA;
    GREF(unsigned short, REC + 0xC) = 2;
    GREF(int, REC + 0x10) = 0x40;
    GREF(int, REC + 0x14) = 0x100;
    GREF(int, REC + 0x18) = 0x200;
    const int words0[2] = {4, 8}, words1[1] = {0x20};
    part(REC + 0x100, 500, 0x300, 2, words0);
    part(REC + 0x200, 1290, 0x400, 1, words1);
    GREF(gaddr, STATE + 0x58) = REC;
    moby(MOBY_A, PARTS_A, 3, 1);
    moby(MOBY_B, PARTS_B, 0, 0);

    /* The pool has room for one: the first part gets a moby, the second (class 1290) is skipped. */
    created[0] = MOBY_A;
    created[1] = 0;
    func_00204FC0(0);
    CHECK(creates == 2 && warnings == 1);
    CHECK(strcmp(warning, "scene actor skipped: moby pool full (class 1290)") == 0);
    CHECK(memcmp(low, ram, sizeof low) == 0); /* nothing written through the null moby */
    CHECK(GREF(gaddr, STATE + 0x178) == MOBY_A && GREF(gaddr, STATE + 0x17C) == 0);
    CHECK(GREF(unsigned short, STATE + 0x40) == 0x1234 && GREF(unsigned short, STATE + 0x46) == 0x55AA);
    CHECK(GREF(short, STATE + 0x44) == 2 && GREF(int, STATE + 0x54) == REC + 0x40 && GREF(int, STATE + 0x4C) == 0);
    CHECK(GREF(unsigned char, PARTS_A + 0xC) == 4);
    CHECK(GREF(unsigned char, MOBY_A + 0x52) == 3 && GREF(unsigned char, MOBY_A + 0x53) == 3);
    CHECK(GREF(unsigned short, MOBY_A + 0x32) == 0x1FF && GREF(unsigned short, MOBY_A + 0x34) == 0x106);
    CHECK(GREF(unsigned char, MOBY_A + 0x72) == 0xFF && GREF(unsigned char, MOBY_A + 0x73) == 0x18);
    CHECK(GREF(unsigned long long, MOBY_A + 0x38) == 0x38383800000000ull);
    CHECK(GREF(int, MOBY_A + 0x78) == REC + 0x300);
    CHECK(GREF(int, PARTS_A + 0x48 + 3 * 4) == REC + 0x110);
    /* Each part's pointer table made absolute, the skipped one's too. */
    CHECK(GREF(int, REC + 0x110 + 0x1C) == REC + 0x110 + 4 && GREF(int, REC + 0x110 + 0x20) == REC + 0x110 + 8);
    CHECK(GREF(int, REC + 0x210 + 0x1C) == REC + 0x210 + 0x20);

    /* The next run tries again for the empty slot only. */
    created[2] = MOBY_B;
    func_00204FC0(0);
    CHECK(creates == 3 && warnings == 1);
    CHECK(GREF(gaddr, STATE + 0x178) == MOBY_A && GREF(gaddr, STATE + 0x17C) == MOBY_B);
    CHECK(GREF(unsigned char, PARTS_B + 0xC) == 1 && GREF(unsigned char, MOBY_B + 0x52) == 0);
    CHECK(GREF(unsigned char, MOBY_B + 0x73) == 0 && GREF(int, MOBY_B + 0x78) == REC + 0x400);
    CHECK(GREF(int, PARTS_B + 0x48) == REC + 0x210);
    CHECK(memcmp(low, ram, sizeof low) == 0);
    return 0;
}
