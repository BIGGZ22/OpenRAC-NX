/* NON_MATCHING func_L13_002C3840 -- src/overlays/l13_gemlik/vendor_002C2638.c
 * Best so far: SIZE ours 1900 / retail 1912, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Blarg space fighter moby update (class 101, level 13): state machine on moby+0x20 (9 cases, jtbl_L13_001F4920)
 *   Still differs: retail rematerialises -1 for the D4 compare and the store (ours keeps one li $16,-1), and case 
 */
extern void func_L13_002C3FB8(void *);
extern int func_L13_00306D40(void *, int);
extern void func_L13_00306E20(char *, char *, int, float);
extern void func_00213DE0(void *, int, int, int);
extern float func_00214158(void);
extern float func_001F9D10(void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern float func_00214D88(float *, float *, float, float, float, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern float func_L00_001FF860(float, float);
extern int func_001F9850(int);
extern void func_L00_00250800(void *, int, void *);
extern void func_0022ED80(int, int, int);
extern float func_001F9D48(void *, void *);
extern void func_001F9BC0(void *);
extern void func_0020D678(void *);
extern void func_L00_0025F4A8_s(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern void *D_L13_0016016C MACRO_ADDR;
extern short D_L13_0016152C;
extern float D_0015EE70 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;

/* Moby update for class 101 (blarg space fighter) on level 13: a state machine on moby+0x20, with a second switch on moby+0xBC. */
void func_L13_002C3840(char *m) {
    char *d;
    char *q;
    char v20[16];
    char v30[16];
    float z40;
    float f20;
    float r;
    float fa;
    int k;

    d = *(char **)(m + 0x78);
    func_L13_002C3FB8(m);
    q = *(char **)(d + 0xB0);
    k = *(unsigned char *)(m + 0x20);
    if ((unsigned)k >= 9) {
        return;
    }
    switch (k) {
    case 0:
        qcopy(d + 0xC0, m + 0x10);
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x5000;
        *(float *)(d + 0xEC) = 15.0f;
        *(float *)(d + 0x20) = 4.0f;
        *(short *)(d + 0x24) = 4;
        *(unsigned char *)(d + 0x28) = 3;
        *(unsigned char *)(d + 0x58) = 0x32;
        *(unsigned char *)(d + 0x5A) = 0;
        *(unsigned short *)(m + 0x32) = 0xFF;
        *(float *)(m + 0x18) = *(float *)(m + 0x18) + 1.0f;
        if (*(unsigned char *)(m + 0x53)) {
            func_00213DE0(m, 0, 0, 10);
        }
        *(unsigned char *)(m + 0xBC) = 0;
        *(int *)(d + 0xFC) = func_L13_00306D40(m, 0);
        *(int *)(d + 0xF8) = func_L13_00306D40(m, 1);
        k = (*(int *)(d + 0xD4) == -1) ? 2 : 5;
        *(unsigned char *)(m + 0x20) = k;
        *(float *)(d + 0x100) = func_00214158();
        *(float *)(d + 0x108) = func_00214158();
        *(float *)(d + 0x10C) = func_00214158();
        *(int *)(d + 0x110) = -1;
        return;
    case 1:
    case 3:
        return;
    case 2:
        r = func_L00_001FF860(*(float *)(q + 0x10) - *(float *)(m + 0x10), *(float *)(q + 0x14) - *(float *)(m + 0x14));
        fa = D_0015EE70 * 4.71238899230957f;
        func_L00_0025CE58(m + 0x48, r, d + 0xDC, fa, fa, D_0015EE6C * 10.122909545898438f);
        if (*(int *)(d + 0xB4) == 2) {
            return;
        }
        *(float *)(d + 0xEC) = 15.0f;
        *(unsigned char *)(m + 0xBC) = 1;
        if (*(unsigned char *)(m + 0x53) != 1) {
            func_00213DE0(m, 1, 0, 0);
        }
        *(unsigned char *)(m + 0x20) = 4;
        return;
    case 4:
        r = func_L00_001FF860(*(float *)(q + 0x10) - *(float *)(m + 0x10), *(float *)(q + 0x14) - *(float *)(m + 0x14));
        func_L00_0025CE58(m + 0x48, r, d + 0xDC, D_0015EE70 * 12.566370964050293f, D_0015EE70 * 25.132741928100586f, D_0015EE6C * 6.2831854820251465f);
        switch (*(unsigned char *)(m + 0xBC)) {
        case 0:
            *(unsigned char *)(m + 0xBC) = 1;
            if (*(unsigned char *)(m + 0x53) != 1) {
                func_00213DE0(m, 1, 0, 2);
            }
            return;
        case 1:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            *(unsigned char *)(m + 0xBC) = 2;
            if (*(unsigned char *)(m + 0x53) != 2) {
                func_00213DE0(m, 2, 0, 2);
            }
            *(int *)(d + 0xE8) = 0;
            return;
        case 2:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            *(unsigned char *)(m + 0xBC) = 3;
            if (*(unsigned char *)(m + 0x53) != 3) {
                func_00213DE0(m, 3, 0, 2);
            }
            return;
        case 3:
            if (func_001F9908((int *)(d + 0xE8)) != 0) {
                *(int *)(d + 0xE8) = func_001F9850(60);
                if (*(int *)(d + 0xF4)) {
                    func_L00_00250800(m, 1, v20);
                } else {
                    func_L00_00250800(m, 0, v20);
                }
                func_001F9BF0(v30, q + 0x10, v20);
                func_L00_001FF4B0(v30, v30, *(float *)&D_L13_0016152C * D_0015EE6C);
                if (*(int *)(d + 0xF4) == 0) {
                    if (*(int *)(d + 0xFC)) {
                        func_L13_00306E20(*(char **)(d + 0xFC), v30, (int)q, *(float *)(d + 0xEC));
                        func_0022ED80(0, 0, (int)m);
                    }
                    *(int *)(d + 0xFC) = func_L13_00306D40(m, 0);
                    *(int *)(d + 0xF4) = 1;
                } else {
                    if (*(int *)(d + 0xF8)) {
                        func_L13_00306E20(*(char **)(d + 0xF8), v30, (int)q, *(float *)(d + 0xEC));
                        func_0022ED80(0, 0, (int)m);
                    }
                    *(int *)(d + 0xF8) = func_L13_00306D40(m, 1);
                    *(int *)(d + 0xF4) = 0;
                }
                *(float *)(d + 0xEC) = *(float *)(d + 0xEC) + 5.0f;
                r = func_001F9D48(m + 0x10, q + 0x10);
                if (r < *(float *)(d + 0xEC)) {
                    *(float *)(d + 0xEC) = func_001F9D48(m + 0x10, q + 0x10);
                }
            }
            if (*(int *)(d + 0xB4) != 2) {
                return;
            }
            if (*(unsigned char *)(m + 0x53) != 4) {
                func_00213DE0(m, 4, 0, 2);
            }
            *(unsigned char *)(m + 0xBC) = 4;
            return;
        case 4:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            *(unsigned char *)(m + 0xBC) = 5;
            if (*(unsigned char *)(m + 0x53) != 5) {
                func_00213DE0(m, 5, 0, 2);
            }
            return;
        case 5:
            if (*(int *)(d + 0xB4) == 2) {
                if (*(unsigned char *)(m + 0x53) != 6) {
                    func_00213DE0(m, 6, 0, 2);
                }
                *(unsigned char *)(m + 0xBC) = 6;
                return;
            }
            *(unsigned char *)(m + 0xBC) = 2;
            if (*(unsigned char *)(m + 0x53) != 2) {
                func_00213DE0(m, 2, 0, 2);
            }
            return;
        case 6:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            if (*(int *)(d + 0xB4) != 2) {
                *(unsigned char *)(m + 0xBC) = 1;
                if (*(unsigned char *)(m + 0x53) != 1) {
                    func_00213DE0(m, 1, 0, 2);
                }
                return;
            }
            if (*(unsigned char *)(m + 0x53)) {
                func_00213DE0(m, 0, 0, 2);
            }
            *(unsigned char *)(m + 0x20) = 2;
            *(unsigned char *)(m + 0xBC) = 0;
            return;
        default:
            return;
        }
    case 5:
        if (*(int *)(d + 0xB4) == 2) {
            return;
        }
        *(unsigned char *)(m + 0xBC) = 0;
        *(unsigned char *)(m + 0x20) = 6;
        return;
    case 6:
        z40 = 0.0f;
        f20 = func_001F9D10(m + 0x10, (char *)D_L13_0016016C + (*(int *)(d + 0xD4) << 7) + 0x30);
        func_001F9BF0(v20, (char *)D_L13_0016016C + (*(int *)(d + 0xD4) << 7) + 0x30, m + 0x10);
        fa = D_0015EE70 * 10.0f;
        func_00214D88(&z40, (float *)(d + 0xF0), f20, fa, fa, D_0015EE6C * 20.0f);
        func_L00_001FF4B0(v20, v20, *(float *)(d + 0xF0));
        func_001F9BD8(m + 0x10, m + 0x10, v20);
        if (!(f20 < 0.1f)) {
            return;
        }
        if (!(*(float *)(d + 0xF0) < 0.01f)) {
            return;
        }
        if (*(unsigned char *)(m + 0x53)) {
            func_00213DE0(m, 0, 0, 10);
        }
        *(unsigned char *)(m + 0xBC) = 0;
        *(unsigned char *)(m + 0x20) = 2;
        return;
    case 7:
        if (!(*(unsigned char *)(m + 0x70) & 2)) {
            return;
        }
        if (*(unsigned char *)(m + 0x53) != *(unsigned char *)(m + 0xBC)) {
            func_00213DE0(m, *(unsigned char *)(m + 0xBC), 0, 10);
        }
        *(unsigned char *)(m + 0x20) = 4;
        return;
    case 8:
        func_001F9BC0(v20);
        func_L00_0025F4A8_s(m, v20, m + 0x10, 0.0f, 0.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, 2, 15.0f, 1, 1, -1, 0);
        if (*(int *)(d + 0xF8)) {
            func_0020D678((void *)*(int *)(d + 0xF8));
        }
        if (*(int *)(d + 0xFC)) {
            func_0020D678((void *)*(int *)(d + 0xFC));
        }
        func_0020D678(m);
        return;
    default:
        return;
    }
}
