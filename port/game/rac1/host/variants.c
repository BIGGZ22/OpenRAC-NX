/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: functions the decompilation's catalogue (config/overlays/functions.tsv)
 * folds into another by its fingerprint, although their code differs from it in a constant or a
 * field offset (split_places.tsv). Each is the function it was folded into with the words that
 * differ, read from the retail copies; the places are theirs alone (hostgen, read_split_places).
 */
#include "rac1.h"

#include <stdint.h>

/* The hero block, D_0013E633 + 0xE1D. */
#define HERO RAC1_ADDR(0x0013F450u)

#if !RAC1_OMIT(func_L00_00232A00)
/* func_L00_00211F68's other: clears the hero's +0x227C and +0x2278 (not the target speed and
 * speed). Ratchet's animation advance (func_L00_00232EF0) starts with it. */
void func_L00_00232A00(void) {
    GREF(int, HERO + 0x227C) = 0;
    GREF(int, HERO + 0x2278) = 0;
}
#endif

#if !RAC1_OMIT(func_L00_0020A870)
/* func_L00_0020A858's three neighbours: the same two-float setter on D_L00_0017A780, at
 * +0x154/+0x158, +0x204/+0x208 and +0x2B4/+0x2B8 instead of +0xA4/+0xA8. */
void func_L00_0020A870(float a, float b) {
    const gaddr p = OPENRAC_LDATA(0, RAC1_LADDR(00, 0x0017A780u));
    GREF(float, p + 0x154) = a;
    GREF(float, p + 0x158) = b;
}

void func_L00_0020A888(float a, float b) {
    const gaddr p = OPENRAC_LDATA(0, RAC1_LADDR(00, 0x0017A780u));
    GREF(float, p + 0x204) = a;
    GREF(float, p + 0x208) = b;
}

void func_L00_0020A8A0(float a, float b) {
    const gaddr p = OPENRAC_LDATA(0, RAC1_LADDR(00, 0x0017A780u));
    GREF(float, p + 0x2B4) = a;
    GREF(float, p + 0x2B8) = b;
}
#endif

/* func_001EC270's level copy: the dispatch record's second hook (+0x10), not its first (+0x08). */
void func_L00_001EBD58(gaddr arg0) {
    const gaddr rec = OPENRAC_DATA(RAC1_ADDR(0x001E8F80u)) + (gaddr)GREF(short, arg0 + 0x8C) * 0x14u;
    const gaddr fn = GREF(gaddr, rec + 0x10);
    if (fn != 0) {
        GFN(void (*)(gaddr), fn)(arg0);
    }
}

/* func_001FB848's level copies: the same four-word packet (a DMA call of COUNT quadwords at DATA
 * and its return) with other counts and data. */
static void packet_call(uint32_t count, gaddr data) {
    const gaddr at = GREF(gaddr, OPENRAC_DATA(RAC1_ADDR(0x00161000u)));
    GREF(uint32_t, at + 0) = 0x30000000u | count;
    GREF(uint32_t, at + 4) = data;
    GREF(uint32_t, at + 8) = 0;
    GREF(uint32_t, at + 12) = 0x50000000u | count;
    GREF(gaddr, OPENRAC_DATA(RAC1_ADDR(0x00161000u))) = at + 16;
}

void func_L00_00201300(void) { packet_call(0x29, RAC1_ADDR(0x00151C60u)); }
/* Three neighbours in each level, each calling its own three quadwords: level 0's data addresses,
 * relocated to the loaded level's by pairing each with its copy there. */
void func_L00_002A2020(void) { packet_call(0x03, OPENRAC_LDATA(0, RAC1_LADDR(00, 0x001C3280u))); }
void func_L00_002A2080(void) { packet_call(0x03, OPENRAC_LDATA(0, RAC1_LADDR(00, 0x001C2840u))); }
void func_L00_002A20E0(void) { packet_call(0x03, OPENRAC_LDATA(0, RAC1_LADDR(00, 0x001C2870u))); }
void func_L00_002A2148(void) { packet_call(0x0B, RAC1_ADDR(0x0013D010u)); }

/* func_0020CC88's level copy: flags +0x20 and +0x21 of D_0013D5C8, not +0x21 and +0x1F. */
int func_L00_0024F054(void) {
    const gaddr base = RAC1_ADDR(0x0013D5C8u);
    return GREF(uint8_t, base + 0x20) != 0 && GREF(uint8_t, base + 0x21) != 0;
}

/* func_00217860's level copy: the track words at +0x38/+0x3C/+0x3A of D_001517D0, not
 * +0x54/+0x58/+0x56. */
void func_L00_00267080(int arg0, long long arg1) {
    const gaddr p = (gaddr)(int)arg1;
    if (p == 0) {
        return;
    }
    GREF(int, p) = arg0;
    if (arg0 != 0) {
        if (GREF(short, p + 10) == 1) {
            GREF(short, p + 10) = 2;
        }
    } else {
        const gaddr b = RAC1_ADDR(0x001517D0u);
        func_002167C0(GREF(short, b + 0x38), GREF(short, b + 0x3C), GREF(short, b + 0x3A));
    }
}

/* func_00229C08's level copy: words 6 and 5 of D_0018A3B0 decide the shrub textures, not 8 and 7. */
void func_L00_00296CD0(void) {
    const gaddr out = OPENRAC_DATA(RAC1_ADDR(0x00161000u));
    const gaddr list = OPENRAC_DATA(RAC1_ADDR(0x001604F0u));
    const gaddr p = GREF(gaddr, out);
    GREF(gaddr, out) += 16;
    const gaddr l = GREF(gaddr, list);
    GREF(int, l + 0) = 0x20000000;
    GREF(int, l + 4) = (int)GREF(gaddr, out);
    GREF(int, l + 8) = 0;
    GREF(int, l + 12) = 0;
    const gaddr flags = OPENRAC_DATA(RAC1_ADDR(0x0018A3B0u));
    if (GREF(int, flags + 6 * 4) != 0 && GREF(int, flags + 5 * 4) != 0) {
        const int size = func_0022B648(GREF(int, RAC1_ADDR(0x0015EF74u)));
        func_00234E80();
        if (GREF(int, OPENRAC_DATA(RAC1_ADDR(0x001604F8u))) < size) {
            GREF(int, OPENRAC_DATA(RAC1_ADDR(0x001604F8u))) = size;
        }
    }
    const gaddr q = GREF(gaddr, out);
    GREF(int, q + 0) = 0x20000000;
    GREF(int, q + 4) = (int)(GREF(gaddr, list) + 16);
    GREF(int, q + 8) = 0;
    GREF(int, q + 12) = 0;
    GREF(gaddr, out) += 16;
    GREF(int, p + 0) = 0x20000000;
    GREF(int, p + 4) = (int)GREF(gaddr, out);
    GREF(int, p + 8) = 0;
    GREF(int, p + 12) = 0;
}

#if !RAC1_OMIT(func_L01_002A2B68)
/* func_L00_0028F210's copy in some levels: the sound slot's +0x80, not +0x84. */
int func_L01_002A2B68(int i, int v) {
    GREF(int, RAC1_ADDR(0x0013E650u) + (gaddr)(i * 0x70) + 0x80) = v;
    return 1;
}
#endif

#if !RAC1_OMIT(func_L00_002E9A88)
/* func_L00_002E9A40's neighbour: +0xE4/+0xE8 of the record, not +0xDC/+0xE0. */
void func_L00_002E9A88(float a, float b) {
    const gaddr p = GREF(gaddr, OPENRAC_LDATA(0, RAC1_LADDR(00, 0x00166F00u)));
    if (GREF(short, p + 0x86) == 0) {
        const gaddr q = GREF(gaddr, p + 0x70) + 0x40;
        if (a != 0.0f) {
            GREF(float, q + 0xE4) = a;
        }
        if (b != 0.0f) {
            GREF(float, q + 0xE8) = b;
        }
    }
}
#endif

#if !RAC1_OMIT(func_L05_0031AAA8)
/* func_L05_0031AAA8 (levels 5 and 16; no C in the decompilation): the moby of the class list
 * D_L05_001AC040[moby +0x21] (moby indices, the last with its top bit set) whose variables
 * (+0x78) hold ID at +0xB4, or 0. Level 16's copy compares +0xAC. */
static gaddr moby_with_id(gaddr moby, int id, uint32_t field) {
    const gaddr list = GREF(gaddr, OPENRAC_LDATA(5, RAC1_LADDR(05, 0x001AC040u)) + 4u * GREF(uint8_t, moby + 0x21));
    if (list == 0) {
        return 0;
    }
    const gaddr base = GREF(gaddr, OPENRAC_LDATA(5, RAC1_LADDR(05, 0x00160098u)));
    for (gaddr p = list;; p += 2) {
        const uint16_t w = GREF(uint16_t, p);
        const gaddr m = base + (gaddr)(w & 0x7FFF) * 0x100u;
        if (GREF(int16_t, GREF(gaddr, m + 0x78) + field) == id) {
            return m;
        }
        if (w & 0x8000) {
            return 0;
        }
    }
}

gaddr func_L05_0031AAA8(gaddr moby, int id) { return moby_with_id(moby, id, 0xB4); }
gaddr func_L16_002E6048(gaddr moby, int id) { return moby_with_id(moby, id, 0xAC); }
#endif

#if !RAC1_OMIT(func_L12_002C08A0)
/* func_L11_0030FB58's level 12 copy: the record index at +0xC of the variables, not +0x4. */
void func_L12_002C08A0(gaddr moby) {
    gaddr p = GREF(gaddr, OPENRAC_LDATA(11, RAC1_LADDR(11, 0x001AC540u)) + 4u * GREF(uint8_t, moby + 0x21));
    const gaddr base = GREF(gaddr, OPENRAC_LDATA(11, RAC1_LADDR(11, 0x00160058u)));
    int16_t w;
    do {
        w = GREF(int16_t, p);
        p += 2;
        const gaddr e = base + (gaddr)(w & 0x7FFF) * 256u;
        GREF(float, e + 0x18) = GREF(float, moby + 0x18);
        const gaddr s = OPENRAC_LDATA(11, RAC1_LADDR(11, 0x001DAB40u)) + (gaddr)(GREF(int, GREF(gaddr, e + 0x78) + 0xC) * 0x1190);
        GREF(float, s + 8) = GREF(float, moby + 0x18);
    } while (w >= 0);
}
#endif

#if !RAC1_OMIT(func_L13_0030AD88)
/* func_L06_00300AB0's level 13 copy: the effect's data 0x20 lower (+0x210 points, +0x1F0 centre,
 * +0x21C fades). Fifteen strips of quads between the points and a sag toward the centre, each
 * drawn twice, 0.25 apart. Its globals are level 13's own (the copy's $gp
 * and lui addresses): read at level 6's addresses, the texture index was garbage and the draw
 * faulted. QuadPacket: pos[4][4] +0, col[4] +0x40, uv[4][2] +0x50, tag[4] +0x70. */
void func_L13_0030AD88(gaddr moby) {
    enum { PK = 0x90, MAT = 2 * PK, TMP = MAT + 0x40, SIZE = TMP + 0x10 };
    GFRAME(frame_, SIZE);
    const gaddr data = GREF(gaddr, moby + 0x78);
    const gaddr uvs = OPENRAC_LDATA(13, RAC1_LADDR(13, 0x001D9E40u));
    func_00234C98(71, 342027);
    for (int k = 0; k < 2; ++k) {
        GREF(uint64_t, frame_ + k * PK + 0x78) = (uint64_t)func_001F4868(GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F2Cu))));
    }
    uint64_t regs = (uint64_t)(uint32_t)GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F18u)));
    regs |= (uint64_t)(uint32_t)GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F1Cu))) << 2;
    regs |= (uint64_t)(uint32_t)GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F20u))) << 4;
    regs |= (uint64_t)(uint32_t)GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F24u))) << 6;
    regs |= (uint64_t)(uint32_t)GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F28u))) << 32;
    for (int k = 0; k < 2; ++k) {
        GREF(uint64_t, frame_ + k * PK + 0x80) = 0x0000FF9000000260ULL;
        GREF(uint64_t, frame_ + k * PK + 0x70) = 0;
        GREF(uint64_t, frame_ + k * PK + 0x88) = regs;
    }
    func_001FA190(frame_ + MAT);
    for (int i = 0; i < 15; ++i) {
        GREF(float, uvs + 1 * 8 + 4) = GREF(float, uvs + 3 * 8 + 4);
        GREF(float, uvs + 3 * 8 + 4) = func_002140F8(0.0f, 0.2f);
        for (int j = 0; j < 4; ++j) {
            const gaddr v = frame_ + 0x10u * j;
            const int off = (i + j / 2) * 16;
            openrac_qcopy(v, data + off + 0x210);
            if (j & 1) {
                func_001F9BF0(frame_ + TMP, v, data + 0x1F0);
                func_L00_001FF4B0(frame_ + TMP, frame_ + TMP, 0.5f);
                func_001F9BF0(v, v, frame_ + TMP);
                GREF(float, v + 8) -= 0.125f;
                GREF(uint32_t, frame_ + 0x40 + 4u * j) = func_001FA8A8(
                    GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F38u))), GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F3Cu))),
                    GREF(float, data + off + 0x21C));
            } else {
                GREF(uint32_t, frame_ + 0x40 + 4u * j) = func_001FA8A8(
                    GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F30u))), GREF(int, OPENRAC_LDATA(13, RAC1_LADDR(13, 0x00161F34u))),
                    GREF(float, data + off + 0x21C));
            }
            for (int k = 0; k < 2; ++k) {
                GREF(float, frame_ + k * PK + 0x50 + 8u * j) = GREF(float, uvs + 8u * j);
                GREF(float, frame_ + k * PK + 0x54 + 8u * j) = GREF(float, uvs + 8u * j + 4);
            }
        }
        func_L00_001FD1D8(frame_, frame_ + MAT, 0);
        for (int j = 1; j < 4; j += 2) {
            GREF(float, frame_ + 0x10u * j + 8) += 0.25f;
        }
        func_L00_001FD1D8(frame_, frame_ + MAT, 0);
    }
    func_00234C98(71, 341515);
}
#endif

/* func_L00_0025D390's two neighbours. All three give the block a moby with mode bit 0x20 keeps a
 * pointer to in its variables, or 0 for no moby or one without the bit; func_L00_0025D390 reads the
 * pointer at +0 (the block a hit is resolved in), these two the ones at +0x10 and +8. The one at +8
 * is the platform block: the turn and the move a moby made this tick, which it carries whatever
 * stands on it with (func_L00_002617B0 writes it, the hero's func_L00_00234800 reads it). Called
 * as func_L00_0025D390, the carry read a crate's hit block instead, whose first words are its
 * health 1.0 and a 0.5: a turn of one radian and a move of half a unit a tick, which threw the hero
 * off any crate he landed on. */
static int moby_block(gaddr moby, gaddr at) {
    if (moby == 0 || (GREF(uint16_t, moby + 0x34) & 0x20) == 0) {
        return 0;
    }
    return GREF(int, GREF(gaddr, moby + 0x78) + at);
}

int func_L00_0025D3C0(gaddr moby) { return moby_block(moby, 0x10); }
#if !RAC1_OMIT(func_L00_00261448)
int func_L00_00261448(gaddr moby) { return moby_block(moby, 8); }
#endif

/* func_00216270's level places (32 bytes: a0 = an address, then one call) whose call is not
 * func_0012F068(func_002177F0). The checkpoint restore (func_L00_002862E0) ends with the first:
 * its level's checkpoint block after the snapshot, given to func_L00_002864E0. */
void func_L00_00286598(void) { func_L00_002864E0(OPENRAC_LDATA(0, RAC1_LADDR(00, 0x001BB610u))); }

/* The others, one level each: func_L03_00292AC0's copies and func_L18_00282F48 on that level's
 * data (the address in the place's own level; the call reaches the loaded level's copy). */
void func_L03_002DDD58(void) { func_L03_00292AC0(RAC1_LADDR(03, 0x001DC460u)); }
void func_L07_002F9418(void) { func_L03_00292AC0(RAC1_LADDR(07, 0x001D36C0u)); }
void func_L07_0030E800(void) { func_L03_00292AC0(RAC1_LADDR(07, 0x00208820u)); }
void func_L08_002DB448(void) { func_L03_00292AC0(RAC1_LADDR(08, 0x001D7320u)); }
void func_L14_003047D8(void) { func_L03_00292AC0(RAC1_LADDR(14, 0x001E9FA0u)); }
void func_L18_002FC168(void) { func_L18_00282F48(RAC1_LADDR(18, 0x001DFED0u)); }
