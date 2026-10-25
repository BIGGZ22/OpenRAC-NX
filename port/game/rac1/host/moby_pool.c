/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: more room for spawned mobys than on a PS2. A documented difference
 * (differences.json, "bigger-moby-pool"), not a fix the player switches back.
 *
 * The level loader LoadLevelGameplay (func_L00_002422D8; the US disc's FUN_L00_00241940) sizes the
 * level's moby array from its gameplay file: the header's moby section (header +0x44) gives the
 * placed mobys' count, then the spare count n2. The array is (placed + n2) * 0x100 bytes, the
 * spawned mobys' pvars n2 * 0x80 after it, and the pool CreateMoby hands slots out of ends at
 * base + (placed + n2 - 1) * 0x100, its last slot holding the free mark. Everything the loader
 * allocates after them moves with n2; nothing sits at a fixed address. Every level ships 256 spare
 * slots (255 usable) but Drek's fleet (level 17), which ships 400.
 *
 * Busy effects fill 255: on Gaspar (level 9) the falling rocks' lava sparks come in bursts of 100
 * to 150, and fcheck (docs/port/BACKFILL.md) saw retail's pool reach 254 of 255 there. A full
 * pool makes CreateMoby answer 0, and a caller that does not check reads through the null moby: on
 * a console into low memory, in the port a crash (docs/port/KNOWN_ISSUES.md; scene_actors.c guards
 * the one call site seen).
 *
 * Here the loader sees at least SPARE spare slots where the level heap has room for them. Each
 * spare slot takes 0x180 bytes (the moby and its pvars), so 768 take 192 KiB more than 256. The
 * heap runs from the level's data up to 0x1FF8000 (the console's malloc arena and stack sit above,
 * set up by crt0); nothing checks it, and in the port memory ends at 0x2000000. Replaying the
 * loader's allocations on both discs (every placed moby kept, the largest ship and text) leaves
 * Kalebo III 829 KiB, Kerwan 287 KiB, Gemlik 278 KiB and every other level more, but Rilgar (5),
 * Blarg Station (6) and Quartu (15) end within 0 to 65 KiB of the top: those three keep the disc's
 * count. Every other level has room for 768 with at least 64 KiB to spare.
 *
 * The loader decompresses the gameplay file's
 * header itself (WadDecompress, func_0020C468, into the load state's header pointer, D_L00_00173F00
 * + 4) and reads it at once, so the count is raised as that decompression ends (hand.c calls
 * rac1_wad_decompressed), while a run of the loader is under way and the destination is the header
 * it named at its start. The array, the pvars, the pool's end and what follows all follow; indices
 * stay far below the 15 bits a moby's index has. The limits on the way (the level heap's headroom
 * on the levels with the most placed mobys, the moby grid's blocks, the renderer) are in
 * docs/port/KNOWN_ISSUES.md.
 *
 * LoadLevelGameplay is a hook point for mods too (rac1.level.load_gameplay; the difficulty mod
 * takes it): the hook sits under this, as func_L00_002422D8__hooked (hostgen's hooks.c: the mod's
 * replacement, or the game's C), so a mod's run of the original sees the bigger pool, and so does
 * what it reads of the pool afterwards.
 */
#include "rac1.h"

/* The level load's state (Load244110 in loaders_00240398.c): +4 the gameplay file's header. */
#define LOAD RAC1_LADDR(00, 0x00173F00u)

enum {
    LOAD_HEADER = 0x4,
    HEADER_MOBYS = 0x44, /* the moby section's offset in the header */
    MOBYS_SPARE = 0x4,   /* in the section: the placed count, then the spare count n2 */
    SPARE = 768          /* spare slots a level with room gets at least (767 usable) */
};

#define CURRENT_LEVEL RAC1_ADDR(0x0015EE84u)

/* The levels whose heap has no room for more: Rilgar, Blarg Station, Quartu. */
static int heap_full(int level) {
    return level == 5 || level == 6 || level == 15;
}

/* Where the running loader decompresses its header; 0 while no loader runs. */
static gaddr g_header;

/* The gameplay header decompressed to `to`, while a loader that named it runs: its spare count raised
 * to SPARE (once a run; a level that ships more keeps its own, and so does one without room). */
void rac1_wad_decompressed(gaddr to) {
    /* Only the header the running loader named: every other WAD decompresses as it is. */
    if (g_header == 0 || to != g_header) {
        return;
    }
    g_header = 0;
    /* Rilgar, Blarg Station and Quartu: 192 KiB more would run past the heap's top. */
    if (heap_full(GREF(int, OPENRAC_DATA(CURRENT_LEVEL)))) {
        return;
    }
    const gaddr spare = to + (gaddr)GREF(int, to + HEADER_MOBYS) + MOBYS_SPARE;
    /* A level that ships more (none does yet) keeps its own. */
    if (GREF(int, spare) < SPARE) {
        GREF(int, spare) = SPARE;
    }
}

/* LoadLevelGameplay: the game's (or a mod's) loader, with the header it names marked for the raise.
 * Returns the level heap's cursor after what it loaded. */
gaddr func_L00_002422D8(int skip_read) {
    const gaddr outer = g_header;
    g_header = GREF(gaddr, OPENRAC_LDATA(0, LOAD) + LOAD_HEADER);
    const gaddr cursor = func_L00_002422D8__hooked(skip_read);
    g_header = outer;
    return cursor;
}

/*
 * The run list of a tick, with room for the bigger pool: build_resident_visibility_list
 * (func_0020E6B8; the US disc's FUN_0020d868), which UpdateMobys runs first (crash_state.c). It
 * links every active moby into its class slot's run list and writes each with mode bit 0x1000 to
 * the target list in the scratchpad (0x70000800, 0x400 bytes: 512 indices), then copies that list
 * to the pointer table the targeting code reads (D_001C7C80, 0x800 bytes: 512 pointers, the last
 * NULL). Retail counts on fewer than 512: nothing bounds it, and past 511 the table's NULL lands in
 * the next table (D_001C8480) and the scratchpad list runs into the group flags (0x70000C80), which
 * the same run reads. Placed mobys alone come close on some levels (803 of classes with the bit on
 * Rilgar, kept down by the distance test), and a bigger pool lets spawned enemies add to them.
 *
 * Here the list holds at most 511, the rest left out of it (and so out of what targeting looks
 * through that tick) but still run; one warning in the log says it happened. Under 512 it is the
 * decompilation's C as it is (rac1-decomp nonmatching/functional/func_0020E6B8.c, fcheck SAME;
 * the group pass advances the list for mode & 0x1000, as retail does); its own copy stays as
 * func_0020E6B8__game, unused (libraries.tsv, wrap).
 */
#define MOBY_ARRAY RAC1_ADDR(0x00160018u)
#define CAMERA_BLOCK RAC1_ADDR(0x00187040u)
#define GROUP_MEMBERS                                                                              \
    RAC1_ADDR(0x001C7AC0u) /* u16 member lists, one per group, the last with bit 15 */
#define TARGETS RAC1_ADDR(0x001C7C80u)

enum {
    SLOTS = 0x70000000u, /* 224 class slots: {head, tail} */
    SLOTS_END = 0x70000700u,
    TARGET_LIST = 0x70000800u,
    TARGET_LIST_END = 0x70000C00u,
    GROUP_FLAGS = 0x70000C80u,
    GROUP_FLAGS_END = 0x70000D00u,
    GROUPS = 0x70,
    TARGETS_MAX = 511,
    MOBY_POS = 0x10,
    MOBY_STATE = 0x20,
    MOBY_GROUP = 0x21,
    MOBY_SLOT = 0x22,
    MOBY_NEXT = 0x28,
    MOBY_DIST = 0x30,
    MOBY_DRAWN = 0x31,
    MOBY_MODE = 0x34
};

static int g_targets_capped;

/* The moby `m` appended to its class slot's run list. */
static void run_list_append(gaddr m) {
    const gaddr sl = SLOTS + (gaddr)GREF(unsigned char, m + MOBY_SLOT) * 8u;
    const gaddr tail = GREF(gaddr, sl + 4);
    GREF(gaddr, sl + 4) = m;
    /* The slot's first moby is its head; a later one hangs off the last. */
    if (tail == 0) {
        GREF(gaddr, sl) = m;
    } else {
        GREF(gaddr, tail + MOBY_NEXT) = m;
    }
}

/* Index `idx` written at the target list's end, kept when `mode` has 0x1000 and there is room. */
static gaddr target_add(gaddr tgt, int idx, unsigned mode) {
    GREF(short, tgt) = (short)idx;
    /* Without 0x1000 the next moby writes over it, as retail's list does. */
    if (!(mode & 0x1000)) {
        return tgt;
    }
    /* Two bytes an index: room while fewer than TARGETS_MAX are kept. */
    if (tgt < TARGET_LIST + TARGETS_MAX * 2u) {
        return tgt + 2;
    }
    /* One warning a session is enough for a report. */
    if (!g_targets_capped) {
        g_targets_capped = 1;
        openrac_game_warn(
            "moby target list full: more than 511 active mobys to target, the rest left out"
        );
    }
    return tgt;
}

/* build_resident_visibility_list: the tick's run lists and target list built; the first run list's
 * head, every slot's list chained after it. */
gaddr func_0020E6B8(void) {
    for (gaddr q = SLOTS; q != SLOTS_END; q += 4) {
        GREF(int, q) = 0;
    }
    for (gaddr q = TARGET_LIST; q != TARGET_LIST_END; q += 4) {
        GREF(int, q) = 0;
    }
    for (gaddr q = GROUP_FLAGS; q != GROUP_FLAGS_END; q += 4) {
        GREF(int, q) = 0;
    }

    const gaddr base = (gaddr)GREF(int, OPENRAC_DATA(MOBY_ARRAY));
    const gaddr cam = OPENRAC_DATA(CAMERA_BLOCK) + 0x140;
    gaddr tgt = TARGET_LIST;
    gaddr cur = base - 0x100;
    int idx = -1;
    for (;;) {
        const int st = GREF(signed char, cur + 0x100 + MOBY_STATE);
        cur += 0x100;
        idx++;
        /* A negative state is a free or dead slot; 0xFF marks the array's end. */
        if (st < 0) {
            if (st == -1) {
                break;
            }
            continue;
        }
        const gaddr m = cur;
        const unsigned mode = GREF(unsigned short, m + MOBY_MODE);
        /* Mode 2: the moby is not run. */
        if (mode & 2) {
            continue;
        }
        const unsigned drawn = GREF(unsigned char, m + MOBY_DRAWN);
        const unsigned dist = GREF(unsigned char, m + MOBY_DIST);
        /* Not drawn last tick and with an update distance (0xFF: always): run only within it. */
        if (drawn == 0 && dist != 0xFF) {
            const float dx = GREF(float, m + MOBY_POS) - GREF(float, cam);
            const float dy = GREF(float, m + MOBY_POS + 4) - GREF(float, cam + 4);
            const float dz = GREF(float, m + MOBY_POS + 8) - GREF(float, cam + 8);
            const float d = (float)dist;
            float t = d * d;
            t = t - dx * dx;
            t = t - dy * dy;
            t = t - dz * dz;
            /* Farther than its update distance from the camera. */
            if (t < 0.0f) {
                continue;
            }
        }
        /* Every active moby flags its group (group -1 writes 0x70000C7F, as retail's delay slot
         * does); a group's members are added by the group pass. */
        const int group = GREF(signed char, m + MOBY_GROUP);
        GREF(char, GROUP_FLAGS + (gaddr)group) = 1;
        /* A group's member waits for the group pass. */
        if (group >= 0) {
            continue;
        }
        tgt = target_add(tgt, idx, mode);
        run_list_append(m);
    }

    /* The group pass: each flagged group's live members. */
    for (int gi = 0; gi != GROUPS; gi++) {
        /* No member of this group was active. */
        if (GREF(unsigned char, GROUP_FLAGS + (gaddr)gi) == 0) {
            continue;
        }
        gaddr lp = GREF(gaddr, OPENRAC_DATA(GROUP_MEMBERS) + 4u * (gaddr)gi);
        for (int r5 = 0; r5 >= 0;) {
            r5 = GREF(short, lp);
            lp += 2;
            const int member = r5 & 0x7FFF;
            const gaddr m = base + ((gaddr)member << 8);
            const unsigned mode = GREF(unsigned short, m + MOBY_MODE);
            /* A dead member is written but not kept, as retail does. */
            if (GREF(signed char, m + MOBY_STATE) < 0) {
                GREF(short, tgt) = (short)member;
                continue;
            }
            tgt = target_add(tgt, member, mode);
            run_list_append(m);
        }
    }

    /* The target list as the NULL-terminated pointer table. */
    gaddr out = OPENRAC_DATA(TARGETS);
    for (gaddr rp = TARGET_LIST; rp != tgt; rp += 2) {
        GREF(gaddr, out) = base + ((gaddr)GREF(unsigned short, rp) << 8);
        out += 4;
    }
    GREF(gaddr, out) = 0;

    /* The class slots' lists chained in slot order; the first head is the result. */
    gaddr first = 0, last = 0;
    for (gaddr sl = SLOTS; sl != SLOTS_END; sl += 8) {
        const gaddr head = GREF(gaddr, sl);
        /* An empty slot adds nothing. */
        if (head == 0) {
            continue;
        }
        const gaddr tail = GREF(gaddr, sl + 4);
        /* The first slot with mobys gives the result; later ones hang off the last tail. */
        if (first == 0) {
            first = head;
        } else {
            GREF(gaddr, last + MOBY_NEXT) = head;
        }
        last = tail;
        GREF(gaddr, tail + MOBY_NEXT) = 0;
    }
    return first;
}
