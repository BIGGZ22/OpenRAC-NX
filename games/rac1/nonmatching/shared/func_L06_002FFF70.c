/* NON_MATCHING func_L06_002FFF70 -- src/overlays/shared/vendor_002FF000.c
 * Best so far: SIZE ours 1480 / retail 1472, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Shared level-06 update: a switch on a 0..11 result of func_L00_0025B4D0 (cases 1-2 and 3-10 each run a timed e
 *   Differences left: the switch as a jump table (merging case 0 and 11 into default makes it bigger, 1496), the p
 *   Unblock: a form of the 0/11/default cases that keeps the 12-entry jump table, and the register order of the si
 */
extern void func_L06_003006F8(void *);
extern float func_002140F8(float, float);
extern float func_001F9878(float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern char *func_L00_0025B478(void *, int, int);
extern int func_L00_0025B4D0(void *, void *, void *, int, int *, float *, int, int);
extern int func_001F9850(int);
extern float func_L00_001FF860(float, float);
extern void func_0022EE28(int, int, int);
extern void func_L00_00264DB8(int, int);
extern void func_L00_0025D5B0(void *, void *, float, int, int, int);
extern float func_L00_00258C80(float lo, float hi);
extern float func_00214158(void);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern char *func_L06_003020B8(int unused, char *pa, char *pb);
extern void func_L00_0025E4B0(void *m, short *p);
extern void func_L00_002584A8(void *, int, int);
extern void func_L00_0025E590(void *, void *);
extern int func_L00_00260FB0(float, char *, void *, int, int, void *, int);
extern float func_001F9D48(void *, void *);
extern float func_001F9B88(float);
extern int *D_L06_001B0FB0[];
extern float D_0015EE6C MACRO_ADDR;
extern short D_0015EE84;
extern short D_L06_00162038;
extern short D_L06_00162040;
extern short D_L06_00162044;
extern short D_L06_00162048;
extern short D_L06_0016204C;
extern short D_L06_0016203C;
extern char D_0013D50F[];
extern char D_0013E633[];

// Level 06 update for a moby with a 12-state machine: aims at a target, runs timed effects, and sets its rotation.
void func_L06_002FFF70(char *m) {
    char *d;
    char *r19;
    char *p;
    char *a;
    int t20;
    float t24;
    int r16;
    int r2;
    int k;
    float vv[4];
    float vec2[4];
    float f0;
    float f1;
    float f2;
    float f3;
    float f4;
    float f12;
    float f20;
    float f21;
    float f22;
    float f23;
    float f24;
    float f25;
    f0 = *(float *)(*(char **)(m + 0x24) + 0x24) * *(float *)&D_L06_00162038;
    *(float *)(m + 0x2C) = f0;
    func_L06_003006F8(m);
    d = *(char **)(m + 0x78);
    a = m;
    if (*(int *)(d + 0x38) != 0) {
        f0 = func_002140F8(180.0f, 240.0f);
        *(int *)(d + 0x188) = func_001FA898_r(func_001F9878(f0));
        *(int *)(d + 0x38) = 0;
        a = d + 0x188;
    }
    func_001F9908((int *)a);
    t24 = 0.0f;
    *(int *)&t24 = 0;
    r19 = func_L00_0025B478(m, 0x330000, 0);
    r16 = func_L00_0025B4D0(m, r19, d + 0x20, 0, &t20, &t24, 0, 4);
    if (t20 != 1 && (unsigned char)m[0x20] != 0x10) {
        if (*(int *)&D_0015EE84 == 0xA && r19 != 0 && *(char **)(r19 + 0x20) != 0 &&
            *(short *)(*(char **)(r19 + 0x20) + 0xA6) == 0x31A &&
            *(unsigned char *)(D_0013D50F + 0x12) == 0) {
            *(unsigned char *)(D_0013D50F + 0x12) = 1;
            func_0022EE28(1, 0, 0);
            func_L00_00264DB8(0x53DB, -1);
        }
        f0 = *(float *)(d + 0x20) - t24;
        *(float *)(d + 0x20) = f0;
        if (f0 <= 0.0f) r16 = 1;
        *(int *)(d + 0x80) = 0x1F4;
        *(float *)(d + 0x88) = 0.5f;
        switch (r16) {
        case 1:
        case 2:
            *(unsigned char *)(m + 0x20) = 0x10;
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
            f4 = 9.0f;
            f2 = D_0015EE6C;
            f0 = *(float *)&D_L06_00162048 * f2;
            f3 = 18.0f;
            f1 = *(float *)&D_L06_0016204C * f2;
            *(float *)(d + 0xB0) = f4;
            *(float *)(d + 0xB4) = f3;
            *(float *)(d + 0x7C) = f1;
            *(float *)(d + 0x78) = f0;
            f22 = 0.0f;
            f23 = 0.2f;
            f0 = func_L00_001FF860(*(float *)(r19 + 0x10), *(float *)(r19 + 0x14));
            f25 = 3.0f;
            func_L00_0025D5B0(m, d + 0x60, f0, 9, 1, 0);
            f24 = 7.0f;
            for (k = 9; k >= 0; k--) {
                qcopy(vv, m + 0x10);
                vv[2] = vv[2] + 2.5f;
                f0 = func_L00_00258C80(f22, f23);
                vv[0] = vv[0] + f0;
                f0 = func_L00_00258C80(f22, f23);
                vv[1] = vv[1] + f0;
                f0 = func_L00_00258C80(f22, f23);
                vv[2] = vv[2] + f0;
                f21 = func_00214158();
                f0 = func_002140F8(f25, 5.7f);
                f1 = D_0015EE6C;
                f20 = f0 * f1;
                f0 = func_001F9F90(f21);
                vec2[0] = f0 * f20;
                f0 = func_001F9FA8(f21);
                vec2[1] = f0 * f20;
                vec2[2] = f22;
                f0 = func_L00_001FF860(*(float *)(r19 + 0x10), *(float *)(r19 + 0x14));
                f0 = func_001F9F90(f0);
                f1 = D_0015EE6C * f24;
                f0 = f0 * f1;
                vec2[0] = vec2[0] + f0;
                f0 = func_L00_001FF860(*(float *)(r19 + 0x10), *(float *)(r19 + 0x14));
                f0 = func_001F9FA8(f0);
                f1 = D_0015EE6C * f24;
                f0 = f0 * f1;
                vec2[1] = vec2[1] + f0;
                f0 = func_002140F8(f25, 8.5f);
                f1 = D_0015EE6C;
                f0 = f0 * f1;
                vec2[2] = f0;
                p = func_L06_003020B8((int)m, (char *)vv, (char *)vec2);
                if (p != 0) {
                    *(float *)(p + 0x2C) = *(float *)(p + 0x2C) * 0.4f;
                }
            }
            *(unsigned char *)(d + 0x117) = 0xFA;
            func_L00_002584A8(m, 0, -1);
            func_L00_0025E4B0(m, (short *)(d + 0x110));
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            *(unsigned char *)(m + 0x20) = 0xF;
            r2 = func_001F9850(0x3C);
            f1 = *(float *)&D_L06_00162040 * D_0015EE6C;
            f0 = *(float *)&D_L06_00162044 * D_0015EE6C;
            *(short *)(d + 0x26) = r2;
            *(float *)(d + 0x78) = f1;
            *(float *)(d + 0xB0) = 8.0f;
            *(float *)(d + 0xB4) = 15.0f;
            *(float *)(d + 0x7C) = f0;
            f0 = func_L00_001FF860(*(float *)(r19 + 0x10), *(float *)(r19 + 0x14));
            func_L00_0025D5B0(m, d + 0x60, f0, 7, 1, 0);
            *(unsigned char *)(d + 0x117) = 0xFA;
            func_L00_0025E4B0(m, (short *)(d + 0x110));
            break;
        default:
            func_L00_0025E4B0(m, (short *)(d + 0x110));
            break;
        }
    }
    *(unsigned char *)(m + 0xA4) = 0xFF;
    func_L00_0025E590(m, d + 0x110);
    f0 = *(float *)&D_L06_0016203C;
    if (*(int *)(d + 0x188) != 0) {
        f1 = 6.0f;
        f0 = f0 + f1;
    }
    *(float *)(d + 0x198) = f0;
    p = (char *)D_L06_001B0FB0[*(int *)(d + 0x180)];
    r2 = func_L00_00260FB0(*(float *)(d + 0x198), m, d + 0x120, 0, 0, (char *)p + 0x10, *(int *)p);
    if (r2 != 2) {
        f0 = func_001F9D48(d + 0x170, d + 0x120);
        if (*(float *)(d + 0x198) < f0) {
            *(int *)(d + 0x164) = 2;
        } else {
            f0 = func_001F9B88(*(float *)(m + 0x18) - *(float *)(d + 0x128));
            f1 = 3.0f;
            if (f1 < f0) *(int *)(d + 0x164) = 2;
        }
    }
    if (*(int *)(d + 0x160) == 0) {
        *(int *)(d + 0x160) = *(int *)(D_0013E633 + 0x2E9D);
    }
}
