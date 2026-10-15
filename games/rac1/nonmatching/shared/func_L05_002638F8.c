/* NON_MATCHING func_L05_002638F8 -- src/overlays/shared/hud_00263490.c
 * Best so far: SIZE ours 1668 / retail 1700, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   HUD element update (shared, levels 05 and 16), 1700 bytes: returns a->0x58 (int); when a[0x72] is set it draws
 *   Differences left: the prologue saves one more register in retail (sq $ra first, an extra $s7 holding a value f
 */
extern int func_00116248(char *str, const char *fmt, ...);
extern float func_001FA888(int);
extern int func_001F9850(int);
extern int func_001FA8A8(int, int, float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_001F6FD8(int a, int b, int c, int d, int e);
extern void *func_001FE540(int);
extern void func_00201A38(int, int, int, int);
extern int func_L00_00236400(char *rec, int *x, int *y);
extern int func_00200198(int, int);
extern void func_L00_0023C058(int, int, int, int, int, int);
extern void func_00200468(int, int, int, int, int, int);
extern float func_001F9FA8(float);
extern char D_0013E633[];
extern char D_L05_0015F940[];
extern char D_L05_0015F9B0[];
extern char D_L05_0015F9B8[];
extern char D_L05_0015F9C0[];
extern char D_L05_00216FC0[];
extern int D_L05_0017EA18[];
extern int D_L05_0015FBAC[];
extern int D_L05_0015FB98[];
extern int D_L05_0015FB90;
extern int D_L05_0015FBA4;
extern int D_L05_0015FBA0;
extern int D_L05_0015FB9C;
extern int D_L05_0015FBA8;
extern int D_L05_0015FBB8;
extern int D_L05_0015FB8C;
extern int D_L05_0015F6B0 MACRO_ADDR;
extern short D_L05_0015F95C;
extern short D_L05_0015F980;
extern short D_L05_0015F974;
extern short D_L05_0015F978;
extern short D_L05_0015F994;
extern short D_L05_0015F998;
extern short D_L05_0015F99C;
extern short D_L05_0015F9A0;
extern short D_L05_0015F9A4;
extern short D_L05_0015F9A8;
extern short D_L05_0015F984;
extern short D_L05_0015F988;
extern short D_L05_0015F98C;
extern short D_L05_0015F990;

/* HUD element update: draws the numeric readouts for the level 5 counter display. */
int func_L05_002638F8(char *a) {
    char *p70 = a + 0x70;
    unsigned char *u = (unsigned char *)p70;
    char buf[0x40];
    int out[4];
    int xa, xb;
    float f20, f21, f0, f12;
    int r, sel, cnt, j, v, k, idx, h, acc, id, id2, i, fb90, fb8c, s16, r3, mod;
    char *E;

    f21 = (float)(unsigned char)p70[0] / 10.0f;
    if (f21 > 1.0f) {
        f21 = 1.0f;
    }
    E = D_0013E633 + 0xE1D;
    if (*(int *)(E + 0x208C) == 0x16) {
        if (u[2] != 0) {
            f20 = func_001FA888(u[1]);
            r = func_001F9850(*(int *)&D_L05_0015F95C);
            f0 = (float)r;
            f12 = f20 / f0;
            if (u[2] == 1 || u[2] == 3) {
                acc = func_001FA8A8(*(int *)&D_L05_0015F974, *(int *)&D_L05_0015F978, f12);
            } else {
                acc = *(int *)&D_L05_0015F980;
            }
            h = func_001FA898_r(f21 * 128.0f) << 24;
            acc += h;
            if ((unsigned int)(u[2] - 1) < 2) {
                func_001F6FD8(*(int *)&D_L05_0015F994 + 1, *(int *)&D_L05_0015F998 + 1, h, (int)buf, -1);
                func_001F6FD8(*(int *)&D_L05_0015F994, *(int *)&D_L05_0015F998, acc, (int)buf, -1);
                id = (int)func_001FE540(D_L05_0015FB90 == 0 ? 0x517D : 0x517C);
                func_00116248(buf, D_L05_0015F9B0, D_L05_0015FB90, id);
                func_001F6FD8(*(int *)&D_L05_0015F99C + 1, *(int *)&D_L05_0015F9A0 + 1, h, (int)buf, -1);
                func_001F6FD8(*(int *)&D_L05_0015F99C, *(int *)&D_L05_0015F9A0, acc, (int)buf, -1);
            } else if (u[2] == 3) {
                fb90 = D_L05_0015FB90;
                sel = -1;
                if (fb90 == 3) {
                    if (D_L05_0015FBA4 == 0) {
                        sel = 0;
                    } else if (D_L05_0015FBA0 == 0) {
                        sel = 1;
                    } else {
                        sel = (D_L05_0015FB9C != 0) ? fb90 : 2;
                    }
                } else if (fb90 == 4) {
                    sel = 4;
                }
                if (sel >= 0) {
                    fb8c = D_L05_0015FB8C;
                    k = (fb8c >= 4) ? 2 : (fb8c >= 2);
                    idx = sel + k * 5;
                    D_L05_0015FBA8 = idx;
                    D_L05_0015FBB8 = D_L05_0017EA18[idx];
                    cnt = 1;
                    for (i = 0; i < 3; i++) {
                        if (D_L05_0015FBAC[i] == idx) {
                            cnt++;
                        }
                    }
                    D_L05_0015FBB8 = D_L05_0015FBB8 / cnt;
                    func_00116248(buf, D_L05_0015F940, D_L05_0015FBB8);
                    func_00201A38(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8 - 0x18, acc, (int)buf);
                    id = (int)func_001FE540(idx + 0x5092);
                    func_00116248(buf, D_L05_0015F9B8, id);
                    func_00201A38(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8, acc, (int)buf);
                } else if (fb90 > 0) {
                    cnt = 0;
                    j = 0;
                    for (;;) {
                        v = D_L05_0015FB98[j];
                        if (v == 0) {
                            j++;
                        } else {
                            out[cnt] = j;
                            cnt++;
                            j++;
                        }
                        if (!(j < 4)) {
                            break;
                        }
                        if (!(cnt < fb90)) {
                            break;
                        }
                    }
                    if (fb90 == 1) {
                        D_L05_0015FBB8 = D_L05_0015FB8C * 10;
                        func_00116248(buf, D_L05_0015F940, D_L05_0015FBB8);
                        func_00201A38(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8 - 0x18, acc, (int)buf);
                        id = (int)func_001FE540(out[0] + 0x5183);
                        id2 = (int)func_001FE540(D_L05_0015FB8C ^ 0x168 ? 0x50A8 : 0x50A7);
                        func_00116248(buf, D_L05_0015F9C0, id, D_L05_0015FB8C, id2);
                        func_001F6FD8(*(int *)&D_L05_0015F99C + 1, *(int *)&D_L05_0015F9A0 + 1, id, (int)buf, -1);
                        func_001F6FD8(*(int *)&D_L05_0015F99C, *(int *)&D_L05_0015F9A0, acc, (int)buf, -1);
                    } else {
                        D_L05_0015FBB8 = D_L05_0015FB8C * 20;
                        func_00116248(buf, D_L05_0015F940, D_L05_0015FBB8);
                        func_00201A38(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8 - 0x18, acc, (int)buf);
                        id = (int)func_001FE540(out[0] + 0x5183);
                        id2 = (int)func_001FE540(out[1] + 0x5183);
                        func_00116248(buf, D_L05_00216FC0, id, id2, D_L05_0015FB8C);
                        func_00201A38(*(int *)&D_L05_0015F9A4, *(int *)&D_L05_0015F9A8, acc, (int)buf);
                    }
                }
            }
        }

        xa = *(int *)(a + 0x50);
        xb = *(int *)(a + 0x54);
        *(int *)(a + 0x58) = 0x100;
        *(int *)(a + 0x5C) = 0x40;
        func_L00_00236400(a, &xa, &xb);
        s16 = (*(int *)(a + 0x74) * 0xDD) / *(int *)(a + 0x8) + 0x1B;
        id = func_00200198(0x7558, 1);
        func_L00_0023C058(id, xa, xb, s16, 0x40, 0x80);
        id2 = func_00200198(0x7558, 0);
        func_00200468(id2, xa, xb, 0x100, 0x40, 0x80);
        if (func_001F9850(0x78) < (unsigned char)E[0x8AE]) {
            r3 = func_001F9850(0x3C);
            mod = D_L05_0015F6B0 % r3;
            f0 = func_001FA888(mod) / (float)r3;
            f0 = func_001F9FA8(f0 * 6.283180236816406f - 3.141590118408203f);
            r3 = func_001FA8A8(*(int *)&D_L05_0015F98C, *(int *)&D_L05_0015F990, f0 * 0.5f + 0.5f);
            id = (int)func_001FE540(0x531F);
            func_00116248(buf, (char *)id);
            func_00201A38(*(int *)&D_L05_0015F984 + 1, *(int *)&D_L05_0015F988 + 1, 0x80000000, (int)buf);
            func_00201A38(*(int *)&D_L05_0015F984, *(int *)&D_L05_0015F988, r3, (int)buf);
        }
    }
    return *(int *)(a + 0x58);
}
