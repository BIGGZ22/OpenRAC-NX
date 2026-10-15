/* func_L00_0020E3B8 -- src/overlays/shared/help_0020CDF0.c (functional C for the port, not a match)
 * Per-tick update of the hero's two hand attachments (slots at D_0013F450 + 0xD00 / 0xD04), run
 * from the hero update (the level copy of func_001F7B40). For each slot i:
 * - a slot that is releasing (+0x34 set) fades out (func_00214D28 on +0x08 toward 0 at
 *   D_0015EE60 * 0.1) and is freed with func_L00_00250120 once its fade reaches 0;
 * - otherwise it is wanted when D_0013F450 + 0x20AB is set (slot 1 only when it is 2) and
 *   +0x20AF is clear (+0x20AF also marks a live slot as releasing). A wanted, empty slot spawns
 *   a moby through func_L00_00250060 with the class word D_L00_0015F780[i], fade 0, and +0x18
 *   from the table D_L00_00197680[D_L00_00197F40[i + 1]]; an unwanted live slot fades out and
 *   is freed as above;
 * - a live slot then fades in to 1 (or out to 0 while +0xD08 is set and +0xD14 clear), follows
 *   the hero moby's (+0x2080) two animation ids +0x52/+0x53 into its own +0x22/+0x23 with the
 *   blend state +0x20/+0x21, +0x24 (blend t), +0x28, +0x2C, +0x30..+0x33, takes a special pose
 *   from func_L00_0020DC68 (which also sets four floats of D_L00_0017A780), points its two
 *   sub-entries (+0x06/+0x07 enable, +0x38/+0x3C buffer in D_L00_0017CB80, 0x800 per slot and
 *   0x400 per entry) at ids below 0x17, and is posed with func_L00_002501C8.
 * Started from the attempt in build-sn/try/func_L00_0020E3B8/p8.c (same control flow as retail,
 * 1492 / 1596 bytes), checked against the retail assembly block by block; unlike p8, the id
 * refresh loop (func_L00_0020DC50) runs only while +0x30 is 0, as retail's does.
 * equiv: DIFFERENT by shape only (38 missing / 28 extra, 17 / 14 with the sub-entry loop's id
 * reads made volatile so gcc keeps retail's counting-up loop): every call with the same
 * arguments, every store and test; the rest is retail rebuilding D_0013F450's and D_L00_0017CB80's
 * addresses where gcc reuses a register (same addresses, other bases: 0x13F450 + 0x20AF against
 * 0x140000 + 0x14FF), retail's branch-likely copies (a second sh +0x30, a dead lbu +0x33), one
 * more store of 1.0 to +0x28 here, and equiv's argument tags on calls.
 */
extern char D_0013F450_20E3B8[] __asm__("D_0013F450");
extern char *D_00140150_20E3B8[] __asm__("D_00140150"); /* D_0013F450 + 0xD00: the two slots */
extern float D_0015EE60_20E3B8 __asm__("D_0015EE60");
extern float D_0015EE64_20E3B8 __asm__("D_0015EE64");
extern int D_L00_0015F780_20E3B8[] __asm__("D_L00_0015F780");
extern unsigned char D_L00_00197F40_20E3B8[] __asm__("D_L00_00197F40");
extern char *D_L00_00197680_20E3B8[] __asm__("D_L00_00197680");
extern char D_L00_0017A780_20E3B8[] __asm__("D_L00_0017A780");
extern char D_L00_0017CB80_20E3B8[] __asm__("D_L00_0017CB80");

extern float func_00214D28_20E3B8(float *, float, float) __asm__("func_00214D28");
extern void func_L00_00250120_20E3B8(void *, void *) __asm__("func_L00_00250120");
extern char *func_L00_00250060_20E3B8(void *, int) __asm__("func_L00_00250060");
extern int func_L00_002056D0_20E3B8(int) __asm__("func_L00_002056D0");
extern int func_L00_0020DC68_20E3B8(int, float *) __asm__("func_L00_0020DC68");
extern int func_L00_0020DC50_20E3B8(char *, int) __asm__("func_L00_0020DC50");
extern int func_001F9850_20E3B8(int) __asm__("func_001F9850");
extern void func_L00_002501C8_20E3B8(void *, unsigned char *) __asm__("func_L00_002501C8");

#define G_20E3B8 D_0013F450_20E3B8
#define HERO_20E3B8 (*(char **)(G_20E3B8 + 0x2080))
#define SLOT_20E3B8(i) (D_00140150_20E3B8[i])
/* The two sub-entries of slot i's object: enabled with their buffers for ids below 0x17. Retail
 * has this tail (and the func_L00_002501C8 call after it) once per branch. */
#define SUB_ENTRIES_20E3B8(o, i)                                                                  \
    do {                                                                                          \
        int j_;                                                                                   \
        for (j_ = 0; j_ < 2; j_++) {                                                              \
            if (*(unsigned char *)((o) + 0x22 + j_) < 0x17) {                                     \
                *(char *)((o) + 6 + j_) = 1;                                                      \
                *(char **)((o) + 0x38 + j_ * 4) = D_L00_0017CB80_20E3B8 + (i) * 0x800 + j_ * 0x400; \
            } else {                                                                              \
                *(char *)((o) + 6 + j_) = 0;                                                      \
            }                                                                                     \
        }                                                                                         \
    } while (0)

void func_L00_0020E3B8(void) {
    int i;

    for (i = 0; i < 2; i++) {
        char *o = SLOT_20E3B8(i);
        char *t;
        int diff;
        int w;
        int r;
        int k;
        float out;

        if (o != 0 && *(int *)(o + 0x34) != 0) {
            /* releasing: fade out, free at 0 */
            if (func_00214D28_20E3B8((float *)(o + 8), 0.0f, D_0015EE60_20E3B8 * 0.1f) == 0.0f) {
                func_L00_00250120_20E3B8(HERO_20E3B8, &SLOT_20E3B8(i));
            }
        } else {
            int c = *(unsigned char *)(G_20E3B8 + 0x20AB);
            int want = 0;

            if (c != 0 && (i == 0 || c == 2)) {
                want = 1;
            }
            if (*(unsigned char *)(G_20E3B8 + 0x20AF) != 0) {
                want = 0;
                t = SLOT_20E3B8(i);
                if (t != 0) {
                    *(int *)(t + 0x34) = 1;
                }
            }
            if (want) {
                if (SLOT_20E3B8(i) == 0) {
                    t = func_L00_00250060_20E3B8(HERO_20E3B8, D_L00_0015F780_20E3B8[i]);
                    SLOT_20E3B8(i) = t;
                    *(float *)(t + 8) = 0.0f;
                    *(char **)(SLOT_20E3B8(i) + 0x18) =
                        D_L00_00197680_20E3B8[D_L00_00197F40_20E3B8[i + 1]];
                }
            } else {
                t = SLOT_20E3B8(i);
                if (t == 0) {
                    continue;
                }
                if (func_00214D28_20E3B8((float *)(t + 8), 0.0f, D_0015EE60_20E3B8 * 0.1f) == 0.0f) {
                    func_L00_00250120_20E3B8(HERO_20E3B8, &SLOT_20E3B8(i));
                }
            }
        }

        o = SLOT_20E3B8(i);
        if (o == 0) {
            continue;
        }
        t = HERO_20E3B8;
        diff = *(unsigned char *)(t + 0x52) != *(unsigned char *)(t + 0x53);
        if (*(int *)(G_20E3B8 + 0xD08) != 0 && *(int *)(G_20E3B8 + 0xD14) == 0) {
            func_00214D28_20E3B8((float *)(o + 8), 0.0f, D_0015EE60_20E3B8 * 0.1f);
        } else if (*(int *)(o + 0x34) == 0) {
            func_00214D28_20E3B8((float *)(o + 8), 1.0f, D_0015EE60_20E3B8 * 0.1f);
        }

        w = 0;
        if (diff && *(int *)(G_20E3B8 + 0xA9C) != 0) {
            int a = func_L00_002056D0_20E3B8(*(unsigned char *)(o + 0x23));
            int b = func_L00_002056D0_20E3B8(*(unsigned char *)(HERO_20E3B8 + 0x53));
            if (a != b) {
                w = *(unsigned char *)(o + 0x22) != *(unsigned char *)(o + 0x23);
            }
        }
        *(short *)(o + 0x30) = w;

        r = func_L00_0020DC68_20E3B8(*(unsigned char *)(HERO_20E3B8 + 0x53), &out);
        if (r != 0 || *(unsigned char *)(o + 0x32) != 0) {
            if (out != 0.0f) {
                float x = D_0015EE64_20E3B8;
                char *tb = D_L00_0017A780_20E3B8;
                *(float *)(tb + 0x5E4) = out;
                *(float *)(tb + 0x694) = out;
                *(float *)(tb + 0x624) = x * 0.05f;
                *(float *)(tb + 0x628) = x * 0.3f;
                *(float *)(tb + 0x6D4) = x * 0.05f;
                *(float *)(tb + 0x6D8) = x * 0.3f;
            }
            if (r != 0) {
                if (*(unsigned char *)(o + 0x32) == 0 || *(unsigned char *)(o + 0x23) != 0) {
                    if (*(unsigned char *)(o + 0x22) == *(unsigned char *)(o + 0x23) ||
                        0.5f < *(float *)(o + 0x24)) {
                        *(char *)(o + 0x22) = *(char *)(o + 0x23);
                        *(char *)(o + 0x20) = *(char *)(o + 0x21);
                    }
                    *(float *)(o + 0x24) = 0.0f;
                    *(char *)(o + 0x23) = 0;
                    *(char *)(o + 0x21) = 0;
                    *(float *)(o + 0x28) = 1.0f;
                    *(float *)(o + 0x2C) = 1.0f / (float)func_001F9850_20E3B8(0xF);
                    *(char *)(o + 0x32) = 1;
                    *(char *)(o + 0x33) = 0;
                }
                *(float *)(o + 0x28) = 1.0f;
            } else if (*(unsigned char *)(o + 0x33) == 0) {
                t = HERO_20E3B8;
                *(char *)(o + 0x23) = func_L00_0020DC50_20E3B8(t, *(unsigned char *)(t + 0x53));
                *(char *)(o + 0x21) = 0;
                *(char *)(o + 0x33) = 1;
                *(float *)(o + 0x24) = 0.0f;
                *(float *)(o + 0x28) = 0.0f;
            } else {
                float f = *(float *)(o + 0x24) + D_0015EE60_20E3B8 * 0.1f;
                *(float *)(o + 0x24) = f;
                if (1.0f < f) {
                    *(float *)(o + 0x24) = 1.0f;
                }
                if (*(float *)(o + 0x24) == 1.0f) {
                    *(char *)(o + 0x32) = 0;
                }
            }
            SUB_ENTRIES_20E3B8(o, i);
            func_L00_002501C8_20E3B8(HERO_20E3B8, (unsigned char *)o);
        } else {
            if (*(short *)(o + 0x30) == 0) {
                for (k = 0; k < 2; k++) {
                    if (!diff || k == 1) {
                        t = HERO_20E3B8;
                        *(char *)(o + 0x22 + k) =
                            func_L00_0020DC50_20E3B8(t, *(unsigned char *)(t + 0x52 + k));
                    }
                }
            }
            if (*(short *)(o + 0x30) != 0) {
                float f = *(float *)(o + 0x24) + D_0015EE60_20E3B8 * 0.15f;
                *(float *)(o + 0x24) = f;
                if (1.0f <= f) {
                    *(char *)(o + 0x22) = *(char *)(o + 0x23);
                    *(char *)(o + 0x20) = *(char *)(o + 0x21);
                    *(short *)(o + 0x30) = 0;
                }
            } else {
                if (!diff) {
                    *(char *)(o + 0x20) = *(unsigned char *)(HERO_20E3B8 + 0x50);
                }
                *(char *)(o + 0x21) = *(unsigned char *)(HERO_20E3B8 + 0x51);
                *(float *)(o + 0x24) = *(float *)(HERO_20E3B8 + 0x54);
            }
            SUB_ENTRIES_20E3B8(o, i);
            func_L00_002501C8_20E3B8(HERO_20E3B8, (unsigned char *)o);
        }
    }
}
