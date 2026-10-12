/* NON_MATCHING func_L06_003010E0 -- src/overlays/l06_blarg/vendor_002FE5D0.c
 * Best so far: SIZE ours 1540 / retail 1492, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 06 moby update (class 1083): a five-state machine on moby[0x20] (switch with cases 0..4, case 3 falls in
 *   Best is p5.c (1476 bytes vs 1492). Left: case 1 reuses a constant 2 in a branch delay slot retail computes bef
 *   Unblock: try_func resolves the call to func_L06_00235E08 as func_000000 in our object (retail jal func_235E08)
 */
extern void func_001F9850_unused_guard(void);
extern int func_001F9850(int);
extern int func_001F9908(void *);
extern float func_001F9D48_07408(void *, void *) __asm__("func_001F9D48");
extern void func_0022ED80(int, int, void *);
extern void func_L06_003016B8(char *, int);
extern void func_L00_002EBF50(void *, void *, int, int, int);
extern void func_L02_002F9ED8(float, float, float, float, float, float);
extern void func_L00_002EBE88(void *);
extern void func_L00_002EBEE0(void *);
extern float func_L00_001FF860(float, float);
extern void func_L00_00217718(void *, void *, int, int);
extern float func_001FA888(int);
extern void func_L06_00235E08(int, int);
extern void func_L00_002EC0C8(int);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9C08(void *, void *, void *, float);
extern void func_001F49B0(void (*)(void), void *);
extern void func_L06_00301B78(void);
extern void func_L06_00301848(void);
extern char D_0013E633[];
extern short D_L06_001620D8;
extern short D_L06_001620D4;
extern short D_L06_001620E0;
extern int D_L06_0015F508;
extern int D_L06_0015F504 MACRO_ADDR;
extern float D_L06_0015F4FC MACRO_ADDR;
extern char *D_L06_0016016C MACRO_ADDR;
extern char *D_L06_00160058 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;

// Moby update for class 1083 on level 06: a five-state machine on moby[0x20] that steers and fires.
void func_L06_003010E0(char *moby) {
    char a[16];
    char b[16];
    char c[16];
    char dd[16];
    char e[16];
    char *d = *(char **)(moby + 0x78);
    char *x = D_0013E633 + 0xE9D;
    char *p;
    float f0;
    int idx;
    int r;
    int f3 = 0;

    *(float *)(moby + 0x2C) = *(float *)(*(char **)(moby + 0x24) + 0x24) * *(float *)&D_L06_001620D8;

    switch ((unsigned char)moby[0x20]) {
    case 0:
        *(int *)(d + 4) = 1;
        moby[0x20] = 1;
        break;
    case 1:
        if (!(*(int *)(d + 8) >= 0 && *(unsigned char *)(D_L06_00160058 + (*(int *)(d + 8) << 8) + 0xBC) != 0)
            && *(int *)(d + 0x20) != -1) {
            break;
        }
        f0 = func_001F9D48_07408(moby + 0x10, x);
        if (f0 < 32.0f) {
            moby[0x20] = 2;
            func_001F9850(30);
            *(int *)(d + 0x24) = 2;
        } else {
            func_L06_003016B8(moby, *(int *)(d + 0x20) != -1);
            moby[0x20] = 4;
        }
        break;
    case 2:
        if (func_001F9908(d + 0x24)) {
            moby[0x20] = 3;
            D_L06_0015F504 = 1;
            D_L06_0015F508 = 0x12;
            idx = *(int *)(d + 0x14) << 7;
            qcopy(a, D_L06_0016016C + idx + 0x30);
            qcopy(b, D_L06_0016016C + idx + 0x70);
            r = func_001F9850(0x12C);
            func_L00_002EBF50(a, b, 1, r, 0);
            func_L02_002F9ED8(0.0010000000474974513f, 1.0f, 1.0f, 0.0010000000474974513f, 1.0f, 1.0f);
            func_L00_002EBE88(a);
            func_L00_002EBEE0(b);
            *(int *)(d + 0x18) = 0;
            p = D_L06_0016016C + (*(int *)(d + 0x10) << 7);
            f0 = func_L00_001FF860(*(float *)(moby + 0x10) - *(float *)(p + 0x30), *(float *)(moby + 0x14) - *(float *)(p + 0x34));
            *(float *)(p + 0x78) = f0;
            func_L00_00217718(p + 0x30, p + 0x70, 0x72, 0);
        }
        if (*(int *)(d + 0x24) < *(int *)&D_L06_001620D4) {
            f0 = func_001FA888(*(int *)(d + 0x24));
            D_L06_0015F4FC = 1.0f - f0 / (float)*(int *)&D_L06_001620D4;
        }
        break;
    case 3:
        if (*(int *)(d + 0x24) < *(int *)&D_L06_001620D4) {
            idx = *(int *)(d + 0x24) + 1;
            *(int *)(d + 0x24) = idx;
            f0 = func_001FA888(idx);
            D_L06_0015F4FC = 1.0f - f0 / (float)*(int *)&D_L06_001620D4;
        }
        f3 = 1;
        break;
    case 4:
        f0 = func_001F9D48_07408(moby + 0x10, x);
        if (f0 < 16.0f) {
            idx = *(int *)(d + 0xC) << 7;
            qcopy(D_L06_0016016C + idx + 0x30, x);
            p = D_L06_0016016C + idx;
            *(float *)(p + 0x38) = *(float *)(p + 0x38) - 0.5f;
            moby[0x20] = 5;
            *(int *)(d + 0xC) = -1;
        }
        break;
    }

    if (f3) {
        if (0.9999899864196777f <= *(float *)(d + 0x18)) {
            moby[0x20] = 5;
            func_L06_00235E08(0, 0);
            func_L00_002EC0C8(2);
            D_L06_0015F504 = 0;
        } else {
            if (0.05000000074505806f <= *(float *)(d + 0x18)) {
                if (*(int *)(d + 0xC) >= 0) {
                    idx = *(int *)(d + 0xC) << 7;
                    qcopy(D_L06_0016016C + idx + 0x30, x);
                    p = D_L06_0016016C + idx;
                    *(float *)(p + 0x38) = *(float *)(p + 0x38) - 0.5f;
                    *(int *)(d + 0xC) = -1;
                }
            }
            if (*(int *)(d + 4) == 1) {
                f0 = func_001F9D48_07408(moby + 0x10, D_L06_00160058 + (*(int *)(d + 0x20) << 8) + 0x10);
                if (0.10000000149011612f < f0) {
                    func_0022ED80(0, 0, moby);
                    func_L06_003016B8(moby, 1);
                }
            }
            f0 = D_0015EE6C;
            func_00214D88((float *)(d + 0x18), (float *)(d + 0x1C), 1.0f, D_0015EE70, D_0015EE70, f0 * 1.5f);
            idx = *(int *)(d + 0x14) << 7;
            qcopy(a, D_L06_0016016C + idx + 0x30);
            qcopy(c, D_L06_0016016C + idx + 0x70);
            *(float *)c = *(float *)&D_L06_001620E0 * 0.01745329238474369f;
            qcopy(b, x);
            qcopy(dd, x + 0x10);
            *(float *)(dd + 4) = *(float *)(c + 4);
            func_L00_001FF4B0(e, x - 0x80, -2.0f);
            func_001F9BD8(b, b, e);
            *(float *)(b + 8) = *(float *)(b + 8) + 0.8500000238418579f;
            func_001F9C08(a, a, b, *(float *)(d + 0x18));
            func_001F9C08(c, c, dd, *(float *)(d + 0x18));
            func_L00_002EBE88(a);
            func_L00_002EBEE0(c);
        }
    }

    if (*(int *)(d + 4) != 0) {
        if (*(int *)d != 0) {
            func_001F49B0(func_L06_00301B78, moby);
        } else {
            func_001F49B0(func_L06_00301B78, moby);
        }
    } else {
        if (*(int *)d != 0) {
            func_001F49B0(func_L06_00301848, moby);
        } else {
            func_001F49B0(func_L06_00301848, moby);
        }
    }
}
