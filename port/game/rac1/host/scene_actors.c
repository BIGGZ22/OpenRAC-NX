/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: a scene's actors when the moby pool is full. A documented divergence from the
 * console: the port skips the actor where the console would read through a null moby.
 *
 * UpdateWorldObjectAnimation (func_00204FC0; the US disc's FUN_00204790) relocates the model
 * record a scene chunk brought in (D_0018CC20 + 0x58) and hooks each of its parts to a moby: for a
 * part whose slot (D_0018CC20 + 0x178 + 4 * i) is still empty it creates one (CreateMoby,
 * func_0020D348) and at once reads the new moby's part list (moby +0x24, then its +0xC). The
 * in-game cutscene's start runs it (func_L00_00299B68, rac1-pal/hand/level_scene_start.c), and
 * the space scenes' chunks (ParseSpaceSceneChunk).
 *
 * CreateMoby answers 0 when the level's 255 spawned mobys are all in use. On Gaspar (level 9) the
 * lava's sparks fill the pool in bursts, and picking up the Pilot's Helmet (class 1290, its update
 * func_L00_002E1850) starts a scene that then wants mobys: the read through 0 crashed the port.
 * The original game has the same latent bug: fcheck (docs/workflow/BACKFILL.md) saw retail's pool
 * reach 254 of 255 on Gaspar from the sparks' bursts, so the console is one burst away from the
 * same read (where it reads low memory and goes on with garbage, or stops).
 *
 * Here, a part whose moby cannot be made is skipped: no read through 0, its slot stays 0, and
 * everything else the routine does happens as before (the record's fields, the other parts, the
 * part's own pointer table made absolute). The scene lacks that copy of the actor; the next run
 * over a chunk (the next scene, the next space chunk) tries again for an empty slot, as the
 * routine does anyway. One warning in the log says it happened, so a report shows it. The proper
 * fix, a bigger pool, is tracked in docs/port/KNOWN_ISSUES.md.
 *
 * The rest is the decompilation's C as it is (games/rac1 src/game/loaders.c, matched; the NTSC
 * decomp's src/world/animation/update_world_object_animation.c, the same routine), compared line
 * by line with what hostgen makes of each for its disc; the decompilation's own copy stays as
 * func_00204FC0__game, unused (libraries.tsv, wrap). The two discs differ in two places: where the
 * record's word at +8 goes (RAC1_TRANSFER_PAD08: PAL +0x46, US +0x48) and the parameter (PAL's
 * C takes the chunk's slot and does not read it; the US C takes none: RAC1_TYPE_..._params).
 */
#include "rac1.h"

#include <stdio.h>

/* The scene chunk's transfer state (TransferState in loaders.c). */
#define TRANSFER RAC1_ADDR(0x0018CC20u)
enum {
    TRANSFER_UNK38 = 0x38,
    TRANSFER_FLAGS = 0x40,     /* u16, the record's +0 */
    TRANSFER_PARTS = 0x44,     /* s16, the record's part count (+0xC) */
    TRANSFER_UNK4C = 0x4C,
    TRANSFER_END = 0x54,
    TRANSFER_REC = 0x58,       /* the model record */
    TRANSFER_UNK5C = 0x5C,
    TRANSFER_SLOTS = 0x178     /* struct Moby *slots[], one per part */
};
/* The model record (ModelRec). */
enum {
    REC_FLAGS = 0x0,
    REC_PART_COUNT = 0x4,
    REC_PAD08 = 0x8,
    REC_NUM_PARTS = 0xC,
    REC_END_OFF = 0x10,
    REC_PART_OFF = 0x14
};
/* A moby (moby.h), and its class's part list (PartList). */
enum {
    MOBY_PARTS = 0x24,
    MOBY_UNK32 = 0x32,
    MOBY_UNK34 = 0x34,
    MOBY_COLOR = 0x38,
    MOBY_IDX = 0x52,
    MOBY_SLOT = 0x53,
    MOBY_FLAG72 = 0x72,
    MOBY_FLAG73 = 0x73,
    MOBY_MODEL = 0x78,
    MOBY_UNK94 = 0x94,
    PARTS_FLAG = 0x6,
    PARTS_USED = 0xC,
    PARTS_ENTRIES = 0x48
};
/* The game mode word (6: the class 0x215 part takes the hero's variant), the variant table, the
 * variant's index (D_0013E130 + 0x26), and the hero's moby whose colour the new moby takes
 * (D_0013F450 + 0x2080: Hero.moby; its +0x38). */
#define GAME_MODE RAC1_ADDR(0x0015F6E8u)
#define VARIANT_TABLE RAC1_ADDR(0x00160588u)
#define VARIANT_INDEX (RAC1_ADDR(0x0013E130u) + 0x26u)
#define HERO_MOBY (RAC1_ADDR(0x0013F450u) + 0x2080u)

void func_00204FC0(RAC1_TYPE_func_00204FC0_params) {
    const gaddr state = OPENRAC_DATA(TRANSFER);
    func_00118D80(0);
    func_0020C468(GREF(int, state + TRANSFER_UNK5C), (int)GREF(gaddr, state + TRANSFER_REC));
    func_00118D80(0);

    const gaddr rec = GREF(gaddr, state + TRANSFER_REC);
    GREF(int, state + TRANSFER_UNK38) = 0;
    gaddr cp = rec + REC_PART_OFF;
    GREF(unsigned short, state + TRANSFER_FLAGS) = GREF(unsigned short, rec + REC_FLAGS);
    const int pc = GREF(int, rec + REC_PART_COUNT);
    GREF(unsigned short, state + RAC1_TRANSFER_PAD08) = GREF(unsigned short, rec + REC_PAD08);
    GREF(short, state + TRANSFER_PARTS) = (short)GREF(unsigned short, rec + REC_NUM_PARTS);
    GREF(int, state + TRANSFER_END) = (int)rec + GREF(int, rec + REC_END_OFF);
    GREF(int, state + TRANSFER_UNK4C) = pc < 0x400 ? 0 : (int)rec + pc;

    for (int i = 0; i < (int)GREF(short, state + TRANSFER_PARTS); i++) {
        const int off = GREF(int, cp);
        cp += 4;
        gaddr sp = rec + (gaddr)off;
        int id = GREF(int, sp);
        sp += 0xC;
        const int endp = (int)rec + GREF(int, sp);
        sp += 4;
        if (GREF(int, OPENRAC_DATA(GAME_MODE)) == 6 && i == 0 && id == 0x215) {
            id = GREF(int, OPENRAC_DATA(VARIANT_TABLE) + (gaddr)GREF(short, VARIANT_INDEX) * 4u);
        }
        const gaddr slot = state + TRANSFER_SLOTS + (gaddr)i * 4u;
        gaddr mob = GREF(gaddr, slot);
        if (mob == 0) {
            mob = func_0020D348(id);
            /* The guard (the divergence above): the pool is full, so this part gets no moby now. */
            if (mob == 0) {
                char line[96];
                snprintf(line, sizeof line, "scene actor skipped: moby pool full (class %d)", id);
                openrac_game_warn(line);
            } else {
                const gaddr parts = GREF(gaddr, mob + MOBY_PARTS);
                const int idx = GREF(unsigned char, parts + PARTS_USED);
                GREF(unsigned char, parts + PARTS_USED) = (unsigned char)(idx + 1);
                GREF(unsigned char, mob + MOBY_IDX) = (unsigned char)idx;
                GREF(unsigned char, mob + MOBY_SLOT) = (unsigned char)idx;
                GREF(unsigned short, mob + MOBY_UNK32) = 0x1FF;
                GREF(unsigned short, mob + MOBY_UNK34) |= 6;
                GREF(unsigned char, mob + MOBY_FLAG72) = 0xFF;
                GREF(int, mob + MOBY_UNK94) = 0;
                const gaddr hero_moby = GREF(gaddr, HERO_MOBY);
                GREF(unsigned long long, mob + MOBY_COLOR) =
                    hero_moby == 0 ? 0x38383800000000ull : GREF(unsigned long long, hero_moby + 0x38);
                if (GREF(unsigned char, GREF(gaddr, mob + MOBY_PARTS) + PARTS_FLAG)) {
                    GREF(unsigned char, mob + MOBY_FLAG73) = 0x18;
                }
                GREF(gaddr, slot) = mob;
            }
        }
        if (mob != 0) {
            GREF(int, mob + MOBY_MODEL) = endp;
            GREF(int, GREF(gaddr, mob + MOBY_PARTS) + PARTS_ENTRIES +
                          (gaddr)GREF(unsigned char, mob + MOBY_IDX) * 4u) = (int)sp;
        }
        for (int k = 0; k < (int)GREF(unsigned char, sp + 0x10); k++) {
            const gaddr w = sp + 0x1C + (gaddr)k * 4u;
            GREF(int, w) = (int)sp + GREF(int, w);
        }
    }
}
