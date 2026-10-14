/* functional: medium. Same reads, writes, calls and arguments in the same order as retail, read side by side with the compiled output; differs only in compiler choices (registers, branch threading, the two flag bytes at 0x13DE48 read once instead of re-read, the seen-bit test as a shift right instead of a mask), and the globals are named by their addresses (_gp = 0x166D00). */
/* func_L01_0030C090 -- src/overlays/l01_novalis/vendor_002FABE8.c (functional C for the port, not a match)
 * Novalis tutorial-hint moby. m = the moby, d = m[0x78] its data (+0x18 a countdown, +0x20..+0x3C trigger ids,
 * +0x28 a done flag, +0x2C a frame counter, +0x34 a hit count).
 *  State 0: clears d+0x14 and d+0x28, m[0x30] = 0xFF, state 1.
 *  State 1: each frame asks Help_Request (func_L00_00203F20) for the level's hints when their conditions hold:
 *    the two timed hints 0x4E27/0x4E28 from the stats at 0x141948; the three pad hints 0x3E8..0x3EA from the
 *    bytes at 0x13DE48 while D_L01_0015F6A8 is 0; trigger d+0x3C (0x3EF), d+0x38 (0x3EC, 0x3ED); trigger d+0x30
 *    with the hero in state 0x3B counts nearby class 0x118 mobies (0x3EE at 3 and 6); while the hero is in state
 *    0x12 and inside trigger d+0x20 or d+0x24, counts frames to hint 0x3F0 and records it in the stats; state 0x11
 *    marks hint 0x44 seen; then every class 0x326 moby within 10 units gives hint 1 (recording it in the stats)
 *    or, past the D_0015EEA0 threshold, hint 2 when func_L01_00276680(moby, 255) says so.
 * equiv: DIFFERENT by shape only: retail keeps the upper halves of 0x13F4D0 and 0x141A68 in saved registers and
 * forms each address from them (the tool cannot resolve those, so every hero/stats access shows on both sides),
 * and reorg's delay-slot copies of the trigger-id loads count as extra; read side by side, every call, argument,
 * load and store matches. */

extern char D_0014171B_30C090[] __asm__("D_0014171B");
extern char D_0013E633_30C090[] __asm__("D_0013E633");
extern char D_0013D605_30C090[] __asm__("D_0013D605");
extern char D_L01_00179910_30C090[] __asm__("D_L01_00179910");
extern int D_L01_0015F6B0_30C090 __asm__("D_L01_0015F6B0");
extern int D_L01_0015F6A8_30C090 __asm__("D_L01_0015F6A8") MACRO_ADDR;
extern int D_L01_0015F76C_30C090 __asm__("D_L01_0015F76C") MACRO_ADDR;
extern int D_L01_0015F768_30C090 __asm__("D_L01_0015F768");
extern char *D_L01_00160064_30C090 __asm__("D_L01_00160064") MACRO_ADDR;
extern int D_0015EFA4_30C090 __asm__("D_0015EFA4") MACRO_ADDR;
extern int D_0015EE84_30C090 __asm__("D_0015EE84") MACRO_ADDR;
extern int D_0015EEA0_30C090 __asm__("D_0015EEA0");

extern int func_001F9908_30C090(void *) __asm__("func_001F9908");
extern int func_001F9850_30C090(int) __asm__("func_001F9850");
extern int func_L00_00203F20_30C090(int, int) __asm__("func_L00_00203F20");
extern int func_00215570_30C090(void *, int) __asm__("func_00215570");
extern float func_001F9D10_30C090(void *, void *) __asm__("func_001F9D10");
extern int func_L00_002D95D8_30C090(void *) __asm__("func_L00_002D95D8");
extern int func_L01_00276680_30C090(void *, float) __asm__("func_L01_00276680");

/* Record a hint in its stats entry e: count (+0, saturating), latest minute (+2), level bit and seen flag (+4). */
static inline __attribute__((always_inline)) void hint_stat_30C090(unsigned short *e) {
    unsigned short c = e[0];
    int x = D_0015EFA4_30C090;
    if (c <= 0xFFFE) {
        e[0] = c + 1;
        x = D_0015EFA4_30C090;
    }
    if (e[1] < func_001F9850_30C090(x) / 600) {
        e[1] = func_001F9850_30C090(D_0015EFA4_30C090) / 600;
    }
    *(int *)(e + 2) = *(int *)(e + 2) | (1 << D_0015EE84_30C090) | 0x80000000;
}

void func_L01_0030C090(unsigned char *m) {
    char *d = *(char **)(m + 0x78);
    char *G = D_0014171B_30C090 + 0x34D;   /* 0x141A68, Help_Request's seen table */
    char *H = D_0014171B_30C090 + 0x22D;   /* 0x141948, hint stats */
    char *P = D_0013E633_30C090 + 0xE9D;   /* 0x13F4D0, the hero */
    unsigned char *S = (unsigned char *)D_0013D605_30C090 + 0x843;   /* 0x13DE48 */
    int *hq = (int *)D_L01_00179910_30C090;   /* the help queue: +0 busy, +0x24 the message shown or -1 */
    char *mb;
    int st, t, n;

    func_001F9908_30C090(d + 0x18);

    if (m[0x20] == 0) {
        *(int *)(d + 0x14) = 0;
        m[0x30] = 0xFF;
        m[0x20] = 1;
        *(int *)(d + 0x28) = 0;
        return;
    }
    if (m[0x20] != 1) {
        return;
    }

    if (D_L01_0015F6B0_30C090 < func_001F9850_30C090(400)) {
        if (*(unsigned short *)(H + 0x60) != 0 && *(unsigned short *)(H + 0x58) == 0) {
            t = func_001F9850_30C090(D_0015EFA4_30C090) - *(unsigned short *)(H + 0x62) * 600;
            if (t < func_001F9850_30C090(0x44C)) {
                if (*(int *)(G + 0x28C) >= 0) {
                    func_L00_00203F20_30C090(0x4E27, 0x51);
                }
            }
        }
    }
    if (D_L01_0015F6B0_30C090 < func_001F9850_30C090(400)) {
        if (*(unsigned short *)(H + 0xE8) != 0) {
            t = func_001F9850_30C090(D_0015EFA4_30C090) - *(unsigned short *)(H + 0xEA) * 600;
            if (t < func_001F9850_30C090(0x44C)) {
                if (*(int *)(G + 0x394) >= 0) {
                    func_L00_00203F20_30C090(0x4E28, 0x72);
                }
            }
        }
    }

    if (D_L01_0015F6A8_30C090 == 0) {
        if (S[3] != 0 && S[2] == 0 && hq[0] == 0 && hq[9] == -1 && *(unsigned short *)(G + 0x20) == 0) {
            func_L00_00203F20_30C090(0x3E8, 4);
        } else if (S[3] == 0 && S[2] != 0 && hq[0] == 0 && hq[9] == -1 && *(unsigned short *)(G + 0x28) == 0) {
            func_L00_00203F20_30C090(0x3E9, 5);
        } else if (S[3] != 0 && S[2] != 0 && hq[0] == 0 && hq[9] == -1 && *(unsigned short *)(G + 0x30) == 0 &&
                   *(unsigned short *)(G + 0x20) == 0) {
            func_L00_00203F20_30C090(0x3EA, 6);
        }
    }

    if (func_00215570_30C090(P, *(int *)(d + 0x3C)) != 0) {
        st = *(int *)(P + 0x200C);
        if ((unsigned)st < 2 || st == 9) {
            if (hq[0] == 0 && hq[9] == -1) {
                t = func_001F9850_30C090(D_0015EFA4_30C090) - *(unsigned short *)(H + 0x4A) * 600;
                if (func_001F9850_30C090(0x34BC0) < t) {
                    if ((*(int *)(G + 0x21C) & (1 << D_0015EE84_30C090)) == 0) {
                        func_L00_00203F20_30C090(0x3EF, 0x43);
                    }
                }
            }
        }
    }

    if (func_00215570_30C090(P, *(int *)(d + 0x38)) != 0) {
        st = *(int *)(P + 0x200C);
        if ((unsigned)st < 2 || st == 9) {
            if (D_L01_0015F76C_30C090 < 3 && hq[0] == 0 && hq[9] == -1) {
                if (*(unsigned short *)(G + 0x200) == 0) {
                    func_L00_00203F20_30C090(0x3EC, 0x40);
                }
            }
        }
    }

    if (func_00215570_30C090(P, *(int *)(d + 0x38)) != 0) {
        st = *(int *)(P + 0x200C);
        if ((unsigned)st < 2 || st == 9) {
            if (*(int *)(P + 0x2004) == 1 && D_L01_0015F768_30C090 == -1 && hq[0] == 0 && hq[9] == -1) {
                if (*(unsigned short *)(G + 0x200) != 0 && *(unsigned short *)(G + 0x208) == 0) {
                    func_L00_00203F20_30C090(0x3ED, 0x41);
                }
            }
        }
    }

    if (*(unsigned short *)(G + 0x210) < 2) {
        if (func_00215570_30C090(P, *(int *)(d + 0x30)) != 0) {
            if (*(int *)(P + 0x2010) == 0x3B && *(int *)(P + 0x118) == 0) {
                for (mb = D_L01_00160064_30C090; mb != 0; mb = *(char **)(mb + 0x28)) {
                    if (*(short *)(mb + 0xA6) != 0x118) continue;
                    if (!(func_001F9D10_30C090(mb + 0x10, P) < 3.0f)) continue;
                    if (**(float **)(mb + 0x78) == 1.0f) continue;
                    n = *(int *)(d + 0x34) + 1;
                    *(int *)(d + 0x34) = n;
                    if (n == 3 || n == 6) {
                        func_L00_00203F20_30C090(0x3EE, 0x42);
                    }
                }
            }
        }
    }

    if (*(int *)(P + 0x200C) == 0x12 && *(int *)(d + 0x28) == 0 && hq[0] == 0 && hq[9] == -1 &&
        (func_00215570_30C090(P, *(int *)(d + 0x20)) != 0 || func_00215570_30C090(P, *(int *)(d + 0x24)) != 0)) {
        *(int *)(d + 0x2C) = *(int *)(d + 0x2C) + 1;
        if (func_00215570_30C090(P, *(int *)(d + 0x20)) != 0 && func_001F9850_30C090(0xF0) < *(int *)(d + 0x2C) &&
            *(unsigned short *)(H + 0xB0) == 0) {
            func_L00_00203F20_30C090(0x3F0, 0x44);
            hint_stat_30C090((unsigned short *)(H + 0xB0));
        } else if (func_00215570_30C090(P, *(int *)(d + 0x24)) != 0) {
            if (func_001F9850_30C090(0x168) < *(int *)(d + 0x2C) && *(unsigned short *)(H + 0xB8) == 0) {
                func_L00_00203F20_30C090(0x3F0, 0x44);
                hint_stat_30C090((unsigned short *)(H + 0xB8));
            }
        }
    } else if (*(int *)(P + 0x200C) == 0x11) {
        *(unsigned short *)(G + 0x220) = 0xFFFF;
    } else {
        *(int *)(d + 0x2C) = 0;
    }

    for (mb = D_L01_00160064_30C090; mb != 0; mb = *(char **)(mb + 0x28)) {
        if (*(short *)(mb + 0xA6) != 0x326) continue;
        if (!(func_001F9D10_30C090(mb + 0x10, P) < 10.0f)) continue;
        if (hq[0] != 0 || hq[9] != -1) continue;
        if (*(unsigned short *)(G + 0x8) != 0 || *(unsigned short *)(G + 0x10) != 0) continue;
        if (*(unsigned short *)(H + 0x68) != 0) continue;
        if (func_L00_002D95D8_30C090(mb) != 0) continue;
        if (*(int *)(P + 0x2228) < D_0015EEA0_30C090) {
            hint_stat_30C090((unsigned short *)(H + 0x68));
            func_L00_00203F20_30C090(1, 1);
        } else if (func_L01_00276680_30C090(mb, 255.0f) != 0) {
            func_L00_00203F20_30C090(2, 2);
        }
    }
}
