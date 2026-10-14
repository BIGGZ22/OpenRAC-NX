/* Carried from port/game/rac1-pal/hand/level_knockback.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native PAL knocked-back flight of a moby, FUN_L00_0025c698 (0x858 bytes), which the
 * decompilation still has as assembly: a creature that was hit flies off, bounces and
 * comes to rest, and its update reads the flags this returns (0x40 its animation
 * ended, 0x100 it left the world or should be removed) to finish dying. Not a PS2
 * match: this is the decompilation's own near miss for the function
 * (nonmatching/shared/FUN_L00_0025c698.c, 2144 bytes against retail's 2136, the same
 * calls in the same order), used here as it is until the function is matched. */
extern float D_0015EE60_m __asm__("D_0015ED60") MACRO_ADDR;
/* What the PAL file this was written in declares before it. */
typedef struct { float f[4]; } __attribute__((aligned(16))) KnockVec;
extern KnockVec D_L00_00173E60_knock __asm__("D_L00_00173E60");
extern KnockVec D_L00_00173E70_knock __asm__("D_L00_00173E70");
extern int *D_L00_00177F00[];
extern int func_L00_001F2868(float, void *, int, void *, void *);
extern char *func_L00_002D7E90(void *, float);

/* Steps moby MOBY's motion from state ST and returns the flag bits it set. */
int FUN_L00_0025c698(char *moby, char *st) {
    float v0[4];
    float v10[4];
    float v20[4];
    float v30[4];
    float v40[4];
    float v50[4];
    float f60, f64, f68, f6c;
    char *pos;
    char *g;
    char *q;
    int flags = 0;
    int two = 2;
    float f21, f20, f12, t, r, rr;
    int n, k;
    int **pp;

    *(float *)(st + 0x8) = *(float *)(st + 0x8) - *(float *)(st + 0x10);
    if (*(unsigned char *)(st + 0x2E) == 1) {
        if (*(unsigned char *)(st + 0x2F) != 0) {
            func_L00_0025C398(moby, st, (void *)(int)*(unsigned char *)(st + 0x2F));
        }
    }
    f60 = func_001F9B20(st);
    func_00213ED8(&f60, (*(short *)(st + 0x3E) == two) ? 0.0f : *(float *)(st + 0x4C), *(float *)(st + 0x14));
    func_L00_001FF550((float *)st, (float *)st, f60);
    if (*(float *)(st + 0x1C) < *(float *)(st + 0x8)) {
        *(float *)(st + 0x8) = *(float *)(st + 0x1C);
    }

    pos = moby + 0x10;
    qcopy(v10, pos);
    v10[2] = v10[2] + *(float *)(st + 0x28);
    f21 = func_00214358f(v10, 0, 0.5f);
    if (func_L00_001F35D8() == 0) {
        if (*(float *)(st + 0x5C) == 0.0f && f21 <= v10[2] && v10[2] + *(float *)(st + 0x8) < f21) {
            *(float *)(st + 0x5C) = 1.0f;
            v10[2] = f21;
            func_L00_002D7E90(v10, 3.0f);
        }
        f21 = func_L00_0025A748_f(pos);
    } else {
        if ((*(int *)(st + 0x24) & 0x10) || *(short *)(st + 0x3E) == two) {
            func_001F9A10(v20, pos, st);
            f12 = func_L00_0025A748_f(v20);
            if (f12 < f21 - 0.25f) {
                func_L00_001FF550((float *)st, (float *)st, 0.0f);
            }
        }
    }

    if (*(float *)(st + 0x5C) != 0.0f) {
        if (*(float *)(st + 0x5C) < 4.0f) {
            func_001F9A68(st, st, 0.7f);
        }
        if (*(float *)(st + 0x5C) < 7.0f) {
            *(float *)(st + 0x5C) = *(float *)(st + 0x5C) + 1.0f;
            *(float *)(st + 0x10) = *(float *)(st + 0x10) * 0.75f;
        }
    }
    if (*(float *)(moby + 0x10) < 2.0f || 1021.0f < *(float *)(moby + 0x10)
        || *(float *)(moby + 0x14) < 2.0f || 1021.0f < *(float *)(moby + 0x14)
        || *(float *)(moby + 0x18) < 2.0f || 1021.0f < *(float *)(moby + 0x18)) {
        flags = 0x100;
    }

    if (*(float *)(st + 0x54) != -1.0f && *(float *)(st + 0x50) != -1.0f) {
        if (*(short *)(st + 0x3E) != 1) {
            if (*(short *)(st + 0x3E) < 2) {
                if (*(short *)(st + 0x3E) == 0) {
                    f20 = func_001F99C0(*(float *)(st + 0x8) / *(float *)(st + 0x10));
                    if (2.0f < f20) {
                        t = func_0020C9E0(moby);
                        rr = (*(float *)(st + 0x50) - t) / f20;
                        if (0.0f < rr) {
                            func_00213ED8((float *)(moby + 0x58), rr, D_0015EE60_m * 0.1f);
                        }
                    }
                }
            } else {
                if (*(short *)(st + 0x3E) == two) {
                    *(float *)(moby + 0x58) = 1.0f;
                }
            }
        } else {
            t = func_0020C9E0(moby);
            if (*(float *)(st + 0x54) <= t) {
                *(int *)(moby + 0x58) = 0;
            } else {
                r = *(float *)(st + 0x10) * -0.5f;
                f20 = 60.0f;
                n = func_L00_002595A0(&f64, &f68, r, *(float *)(st + 0x8) - r, *(float *)(moby + 0x18) - f21);
                if (n > 0 && 0.0f < f64 && f64 < f20) {
                    f20 = f64;
                }
                if (f20 < 0.0001f) {
                    f20 = 0.0001f;
                }
                t = func_0020C9E0(moby);
                rr = (*(float *)(st + 0x54) - t) / f20;
                if (2.0f < rr) {
                    rr = 2.0f;
                } else if (rr < 0.0f) {
                    rr = 0.0f;
                }
                func_00213ED8((float *)(moby + 0x58), rr, D_0015EE60_m * 0.1f);
            }
        }
    }

    if (*(unsigned char *)(moby + 0x70) & 2) {
        flags |= 0x40;
    }
    if (!(*(int *)(st + 0x24) & 4)) {
        func_L00_00258278(moby, (float *)(st + 0x44), *(float *)(st + 0x40), 0.02f, 0.3f, *(float *)(st + 0x48));
    }

    qcopy(v0, pos);
    v0[2] = v0[2] + *(float *)(st + 0x28);
    func_001F9A10(pos, pos, st);
    qcopy(v20, pos);
    v20[2] = v20[2] + *(float *)(st + 0x28);
    func_001F9A10(v30, v20, st);
    f12 = func_001FA6C0(*(int *)(st + 0x20)) * 0.0010742187732830644f;
    n = func_L00_001F2868(f12, v30, 16, moby, 0);
    if (n > 0) {
        pp = D_L00_00177F00;
        k = n;
        do {
            if (func_L00_0025E3B8(*pp) != 0) {
                func_L00_00259BC8((char *)*pp, 1.0f, (int)moby, 0x10000, pos, st);
            } else if (*(int *)(st + 0x24) & 0x20) {
                if (*(short *)(st + 0x3E) == two) {
                    flags |= 0x100;
                }
            }
            pp++;
        } while (--k != 0);
    }

    f20 = 0.0009765625f;
    t = func_001F9AF0(st);
    if ((float)*(int *)(st + 0x20) * f20 < t) {
        func_L00_001EFFF0_p(v0, v20, 36, moby, 0);
        qcopy(v20, &D_L00_00173E60_knock);
    }
    f12 = func_001FA6C0(*(int *)(st + 0x20)) * f20;
    if (func_L00_001F10E0_l(v20, f12, 36, moby) != 0) {
        flags |= 2;
        g = (char *)&D_L00_00173E70_knock;
        qcopy(pos, g);
        *(float *)(moby + 0x18) = *(float *)(moby + 0x18) - *(float *)(st + 0x28);
        func_L00_001FF500(v40, (char *)g + 0x10, 1.0f);
        q = (char *)g - 0x30;
        if (0.7070000171661377f <= v40[2]) {
            f12 = func_001F9AB0(st, v40);
            if (f12 < 0.0f) {
                if (*(int *)(q + 0x18) == 0 || *(int *)(q + 0x1C) > 0) {
                    flags |= 1;
                    *(short *)(st + 0x3E) = 2;
                }
                func_L00_001FF500(v50, v40, f12);
                func_001F9A28(st, st, v50);
            }
        } else {
            r = func_001F9AB0(st, v40);
            if (r < 0.0f) {
                func_L00_001FF500(v50, v40, r * -(*(float *)(st + 0x30) + 1.0f));
                func_001F9A10(st, st, v50);
            }
        }
        if (*(short *)(st + 0x3E) == two) {
            f6c = func_001F9AF0(st);
            func_00213ED8(&f6c, 0.0f, *(float *)(st + 0x10));
            func_L00_001FF500(st, st, f6c);
        }
    }
    if (*(short *)(st + 0x3E) == 0) {
        if (*(float *)(st + 0x8) < 0.0f) {
            *(short *)(st + 0x3E) = 1;
        }
    }
    return flags;
}
