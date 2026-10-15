/* NON_MATCHING func_L00_0025D6F0 -- src/overlays/shared/mobyutil_00258BC8.c
 * Best so far: SIZE ours 2144 / retail 2136, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Moby motion step: eases the moby's position (moby+0x10) and velocity-like state fields toward targets, picks a
 *   Structure still open after p1-p3: the st3E 0/1/2 blocks (LA54/LAB0/LBC8) are laid out differently from retail,
 *   Unblock: a way to pin st to $18 without a new variable (or the allocator order for two saved-register pseudos 
 */
extern float D_0015EE60_m __asm__("D_0015EE60") MACRO_ADDR;
extern int *D_L00_00178000[];
extern int func_L00_001F2BE8(float, void *, int, void *, void *);
extern char *func_L00_002D9340(void *, float);

/* Steps moby MOBY's motion from state ST and returns the flag bits it set. */
int func_L00_0025D6F0(char *moby, char *st) {
    float v0[4];
    float v10[4];
    float v20[4];
    float v30[4];
    float v40[4];
    float v50[4];
    float f60, f64, f68, f6c;
    char *pos;
    V_94C8 *g;
    char *q;
    int flags = 0;
    int two = 2;
    float f21, f20, f12, t, r, rr;
    int n, k;
    int **pp;

    *(float *)(st + 0x8) = *(float *)(st + 0x8) - *(float *)(st + 0x10);
    if (*(unsigned char *)(st + 0x2E) == 1) {
        if (*(unsigned char *)(st + 0x2F) != 0) {
            func_L00_0025D3F0(moby, st, (void *)(int)*(unsigned char *)(st + 0x2F));
        }
    }
    f60 = func_001F9CE8(st);
    func_00214D28(&f60, (*(short *)(st + 0x3E) == two) ? 0.0f : *(float *)(st + 0x4C), *(float *)(st + 0x14));
    func_L00_001FF500((float *)st, (float *)st, f60);
    if (*(float *)(st + 0x1C) < *(float *)(st + 0x8)) {
        *(float *)(st + 0x8) = *(float *)(st + 0x1C);
    }

    pos = moby + 0x10;
    qcopy(v10, pos);
    v10[2] = v10[2] + *(float *)(st + 0x28);
    f21 = func_00214358f(v10, 0, 0.5f);
    if (func_L00_001F3958() == 0) {
        if (*(float *)(st + 0x5C) == 0.0f && f21 <= v10[2] && v10[2] + *(float *)(st + 0x8) < f21) {
            *(float *)(st + 0x5C) = 1.0f;
            v10[2] = f21;
            func_L00_002D9340(v10, 3.0f);
        }
        f21 = func_L00_0025A748_f(pos);
    } else {
        if ((*(int *)(st + 0x24) & 0x10) || *(short *)(st + 0x3E) == two) {
            func_001F9BD8(v20, pos, st);
            f12 = func_L00_0025A748_f(v20);
            if (f12 < f21 - 0.25f) {
                func_L00_001FF500((float *)st, (float *)st, 0.0f);
            }
        }
    }

    if (*(float *)(st + 0x5C) != 0.0f) {
        if (*(float *)(st + 0x5C) < 4.0f) {
            func_001F9C30(st, st, 0.7f);
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
                    f20 = func_001F9B88(*(float *)(st + 0x8) / *(float *)(st + 0x10));
                    if (2.0f < f20) {
                        t = func_0020D830(moby);
                        rr = (*(float *)(st + 0x50) - t) / f20;
                        if (0.0f < rr) {
                            func_00214D28((float *)(moby + 0x58), rr, *(float *)&D_0015EE60 * 0.1f);
                        }
                    }
                }
            } else {
                if (*(short *)(st + 0x3E) == two) {
                    *(float *)(moby + 0x58) = 1.0f;
                }
            }
        } else {
            t = func_0020D830(moby);
            if (*(float *)(st + 0x54) <= t) {
                *(int *)(moby + 0x58) = 0;
            } else {
                r = *(float *)(st + 0x10) * -0.5f;
                f20 = 60.0f;
                n = func_L00_0025A5D8(&f64, &f68, r, *(float *)(st + 0x8) - r, *(float *)(moby + 0x18) - f21);
                if (n > 0 && 0.0f < f64 && f64 < f20) {
                    f20 = f64;
                }
                if (f20 < 0.0001f) {
                    f20 = 0.0001f;
                }
                t = func_0020D830(moby);
                rr = (*(float *)(st + 0x54) - t) / f20;
                if (2.0f < rr) {
                    rr = 2.0f;
                } else if (rr < 0.0f) {
                    rr = 0.0f;
                }
                func_00214D28((float *)(moby + 0x58), rr, D_0015EE60_m * 0.1f);
            }
        }
    }

    if (*(unsigned char *)(moby + 0x70) & 2) {
        flags |= 0x40;
    }
    if (!(*(int *)(st + 0x24) & 4)) {
        func_L00_002592B0(moby, (float *)(st + 0x44), *(float *)(st + 0x40), 0.02f, 0.3f, *(float *)(st + 0x48));
    }

    qcopy(v0, pos);
    v0[2] = v0[2] + *(float *)(st + 0x28);
    func_001F9BD8(pos, pos, st);
    qcopy(v20, pos);
    v20[2] = v20[2] + *(float *)(st + 0x28);
    func_001F9BD8(v30, v20, st);
    f12 = func_001FA888(*(int *)(st + 0x20)) * 0.0010742187732830644f;
    n = func_L00_001F2BE8(f12, v30, 16, moby, 0);
    if (n > 0) {
        pp = D_L00_00178000;
        k = n;
        do {
            if (func_L00_0025F410(*pp) != 0) {
                func_L00_0025AC00((char *)*pp, 1.0f, (int)moby, 0x10000, pos, st);
            } else if (*(int *)(st + 0x24) & 0x20) {
                if (*(short *)(st + 0x3E) == two) {
                    flags |= 0x100;
                }
            }
            pp++;
        } while (--k != 0);
    }

    f20 = 0.0009765625f;
    t = func_001F9CB8(st);
    if ((float)*(int *)(st + 0x20) * f20 < t) {
        func_L00_001EFFF0_p(v0, v20, 36, moby, 0);
        qcopy(v20, &D_94C8_60);
    }
    f12 = func_001FA888(*(int *)(st + 0x20)) * f20;
    if (func_L00_001F10E0_l(v20, f12, 36, moby) != 0) {
        flags |= 2;
        g = &D_L00_00173F70;
        qcopy(pos, g);
        *(float *)(moby + 0x18) = *(float *)(moby + 0x18) - *(float *)(st + 0x28);
        func_L00_001FF4B0(v40, (char *)g + 0x10, 1.0f);
        q = (char *)g - 0x30;
        if (0.7070000171661377f <= v40[2]) {
            f12 = func_001F9C78(st, v40);
            if (f12 < 0.0f) {
                if (*(int *)(q + 0x18) == 0 || *(int *)(q + 0x1C) > 0) {
                    flags |= 1;
                    *(short *)(st + 0x3E) = 2;
                }
                func_L00_001FF4B0(v50, v40, f12);
                func_001F9BF0(st, st, v50);
            }
        } else {
            r = func_001F9C78(st, v40);
            if (r < 0.0f) {
                func_L00_001FF4B0(v50, v40, r * -(*(float *)(st + 0x30) + 1.0f));
                func_001F9BD8(st, st, v50);
            }
        }
        if (*(short *)(st + 0x3E) == two) {
            f6c = func_001F9CB8(st);
            func_00214D28(&f6c, 0.0f, *(float *)(st + 0x10));
            func_L00_001FF4B0(st, st, f6c);
        }
    }
    if (*(short *)(st + 0x3E) == 0) {
        if (*(float *)(st + 0x8) < 0.0f) {
            *(short *)(st + 0x3E) = 1;
        }
    }
    return flags;
}
