/* functional: high. Same reads, writes, calls and arguments as retail (tools/equiv.py: only register, branch-polarity and constant-sharing differences, e.g. (d->D4 == -1) ? 2 : 5 computed with nor/movz instead of a branch). */
/* Blarg space fighter moby update (class 101, level 13 / Gemlik), per the near miss's notes: calls func_L13_002C3FB8,
 * then a state machine on moby+0x20: 0 initialises and spawns the two projectile mobies (d+0xFC, d+0xF8); 2 turns toward
 * the target and goes to 4 once d+0xB4 != 2; 4 turns toward the target with an animation sub-state on moby+0xBC that
 * fires the projectiles alternately on a timer; 5 waits for d+0xB4 != 2, then 6 moves toward path point d+0xD4 of
 * D_L13_0016016C and back to 2 when there; 7 waits for the animation flag and returns to 4; 8 spawns an explosion and
 * deletes the projectiles and itself. */
extern void func_L13_002C3FB8_3840(void *) __asm__("func_L13_002C3FB8");
extern int func_L13_00306D40_3840(void *, int) __asm__("func_L13_00306D40");
extern void func_L13_00306E20_3840(char *, char *, char *, float) __asm__("func_L13_00306E20");
extern void func_00213DE0_3840(void *, int, int, int) __asm__("func_00213DE0");
extern float func_00214158_3840(void) __asm__("func_00214158");
extern float func_001F9D10_3840(void *, void *) __asm__("func_001F9D10");
extern void func_001F9BF0_3840(void *, void *, void *) __asm__("func_001F9BF0");
extern float func_00214D88_3840(float *, float *, float, float, float, float) __asm__("func_00214D88");
extern void func_L00_001FF4B0_3840(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void func_001F9BD8_3840(void *, void *, void *) __asm__("func_001F9BD8");
extern float func_L00_001FF860_3840(float, float) __asm__("func_L00_001FF860");
extern float func_L00_0025CE58_3840(void *, void *, float, float, float, float) __asm__("func_L00_0025CE58");
extern int func_001F9850_3840(int) __asm__("func_001F9850");
extern int func_001F9908_3840(void *) __asm__("func_001F9908");
extern void func_L00_00250800_3840(void *, int, void *) __asm__("func_L00_00250800");
extern int func_0022ED80_3840(int, int, void *) __asm__("func_0022ED80");
extern float func_001F9D48_3840(void *, void *) __asm__("func_001F9D48");
extern void func_001F9BC0_3840(void *) __asm__("func_001F9BC0");
extern void func_0020D678_3840(void *) __asm__("func_0020D678");
extern void func_L00_0025F4A8_3840(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern char *D_L13_0016016C_3840 __asm__("D_L13_0016016C") MACRO_ADDR;
extern float D_L13_0016152C_3840 __asm__("D_L13_0016152C") MACRO_ADDR;
extern float D_0015EE70_3840 __asm__("D_0015EE70") MACRO_ADDR;
extern float D_0015EE6C_3840 __asm__("D_0015EE6C") MACRO_ADDR;

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
    func_L13_002C3FB8_3840(m);
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
            func_00213DE0_3840(m, 0, 0, 10);
        }
        *(unsigned char *)(m + 0xBC) = 0;
        *(int *)(d + 0xFC) = func_L13_00306D40_3840(m, 0);
        *(int *)(d + 0xF8) = func_L13_00306D40_3840(m, 1);
        k = (*(int *)(d + 0xD4) == -1) ? 2 : 5;
        *(unsigned char *)(m + 0x20) = k;
        *(float *)(d + 0x100) = func_00214158_3840();
        *(float *)(d + 0x108) = func_00214158_3840();
        *(float *)(d + 0x10C) = func_00214158_3840();
        *(int *)(d + 0x110) = -1;
        return;
    case 1:
    case 3:
        return;
    case 2:
        r = func_L00_001FF860_3840(*(float *)(q + 0x10) - *(float *)(m + 0x10), *(float *)(q + 0x14) - *(float *)(m + 0x14));
        fa = D_0015EE70_3840 * 4.71238899230957f;
        func_L00_0025CE58_3840(m + 0x48, d + 0xDC, r, fa, fa, D_0015EE6C_3840 * 10.122909545898438f);
        if (*(int *)(d + 0xB4) == 2) {
            return;
        }
        *(float *)(d + 0xEC) = 15.0f;
        *(unsigned char *)(m + 0xBC) = 1;
        if (*(unsigned char *)(m + 0x53) != 1) {
            func_00213DE0_3840(m, 1, 0, 0);
        }
        *(unsigned char *)(m + 0x20) = 4;
        return;
    case 4:
        r = func_L00_001FF860_3840(*(float *)(q + 0x10) - *(float *)(m + 0x10), *(float *)(q + 0x14) - *(float *)(m + 0x14));
        func_L00_0025CE58_3840(m + 0x48, d + 0xDC, r, D_0015EE70_3840 * 12.566370964050293f, D_0015EE70_3840 * 25.132741928100586f, D_0015EE6C_3840 * 6.2831854820251465f);
        switch (*(unsigned char *)(m + 0xBC)) {
        case 0:
            *(unsigned char *)(m + 0xBC) = 1;
            if (*(unsigned char *)(m + 0x53) != 1) {
                func_00213DE0_3840(m, 1, 0, 2);
            }
            return;
        case 1:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            *(unsigned char *)(m + 0xBC) = 2;
            if (*(unsigned char *)(m + 0x53) != 2) {
                func_00213DE0_3840(m, 2, 0, 2);
            }
            *(int *)(d + 0xE8) = 0;
            return;
        case 2:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            *(unsigned char *)(m + 0xBC) = 3;
            if (*(unsigned char *)(m + 0x53) != 3) {
                func_00213DE0_3840(m, 3, 0, 2);
            }
            return;
        case 3:
            if (func_001F9908_3840(d + 0xE8) != 0) {
                *(int *)(d + 0xE8) = func_001F9850_3840(60);
                if (*(int *)(d + 0xF4)) {
                    func_L00_00250800_3840(m, 1, v20);
                } else {
                    func_L00_00250800_3840(m, 0, v20);
                }
                func_001F9BF0_3840(v30, q + 0x10, v20);
                func_L00_001FF4B0_3840(v30, v30, D_L13_0016152C_3840 * D_0015EE6C_3840);
                if (*(int *)(d + 0xF4) == 0) {
                    if (*(int *)(d + 0xFC)) {
                        func_L13_00306E20_3840(*(char **)(d + 0xFC), v30, q, *(float *)(d + 0xEC));
                        func_0022ED80_3840(0, 0, m);
                    }
                    *(int *)(d + 0xFC) = func_L13_00306D40_3840(m, 0);
                    *(int *)(d + 0xF4) = 1;
                } else {
                    if (*(int *)(d + 0xF8)) {
                        func_L13_00306E20_3840(*(char **)(d + 0xF8), v30, q, *(float *)(d + 0xEC));
                        func_0022ED80_3840(0, 0, m);
                    }
                    *(int *)(d + 0xF8) = func_L13_00306D40_3840(m, 1);
                    *(int *)(d + 0xF4) = 0;
                }
                *(float *)(d + 0xEC) = *(float *)(d + 0xEC) + 5.0f;
                r = func_001F9D48_3840(m + 0x10, q + 0x10);
                if (r < *(float *)(d + 0xEC)) {
                    *(float *)(d + 0xEC) = func_001F9D48_3840(m + 0x10, q + 0x10);
                }
            }
            if (*(int *)(d + 0xB4) != 2) {
                return;
            }
            if (*(unsigned char *)(m + 0x53) != 4) {
                func_00213DE0_3840(m, 4, 0, 2);
            }
            *(unsigned char *)(m + 0xBC) = 4;
            return;
        case 4:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            *(unsigned char *)(m + 0xBC) = 5;
            if (*(unsigned char *)(m + 0x53) != 5) {
                func_00213DE0_3840(m, 5, 0, 2);
            }
            return;
        case 5:
            if (*(int *)(d + 0xB4) == 2) {
                if (*(unsigned char *)(m + 0x53) != 6) {
                    func_00213DE0_3840(m, 6, 0, 2);
                }
                *(unsigned char *)(m + 0xBC) = 6;
                return;
            }
            *(unsigned char *)(m + 0xBC) = 2;
            if (*(unsigned char *)(m + 0x53) != 2) {
                func_00213DE0_3840(m, 2, 0, 2);
            }
            return;
        case 6:
            if (!(*(unsigned char *)(m + 0x70) & 2)) {
                return;
            }
            if (*(int *)(d + 0xB4) != 2) {
                *(unsigned char *)(m + 0xBC) = 1;
                if (*(unsigned char *)(m + 0x53) != 1) {
                    func_00213DE0_3840(m, 1, 0, 2);
                }
                return;
            }
            if (*(unsigned char *)(m + 0x53)) {
                func_00213DE0_3840(m, 0, 0, 2);
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
        f20 = func_001F9D10_3840(m + 0x10, D_L13_0016016C_3840 + (*(int *)(d + 0xD4) << 7) + 0x30);
        func_001F9BF0_3840(v20, D_L13_0016016C_3840 + (*(int *)(d + 0xD4) << 7) + 0x30, m + 0x10);
        fa = D_0015EE70_3840 * 10.0f;
        func_00214D88_3840(&z40, (float *)(d + 0xF0), f20, fa, fa, D_0015EE6C_3840 * 20.0f);
        func_L00_001FF4B0_3840(v20, v20, *(float *)(d + 0xF0));
        func_001F9BD8_3840(m + 0x10, m + 0x10, v20);
        if (!(f20 < 0.1f)) {
            return;
        }
        if (!(*(float *)(d + 0xF0) < 0.01f)) {
            return;
        }
        if (*(unsigned char *)(m + 0x53)) {
            func_00213DE0_3840(m, 0, 0, 10);
        }
        *(unsigned char *)(m + 0xBC) = 0;
        *(unsigned char *)(m + 0x20) = 2;
        return;
    case 7:
        if (!(*(unsigned char *)(m + 0x70) & 2)) {
            return;
        }
        if (*(unsigned char *)(m + 0x53) != *(unsigned char *)(m + 0xBC)) {
            func_00213DE0_3840(m, *(unsigned char *)(m + 0xBC), 0, 10);
        }
        *(unsigned char *)(m + 0x20) = 4;
        return;
    case 8:
        func_001F9BC0_3840(v20);
        func_L00_0025F4A8_3840(m, v20, m + 0x10, 0.0f, 0.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, 2, 15.0f, 1, 1, -1, 0);
        if (*(int *)(d + 0xF8)) {
            func_0020D678_3840((void *)*(int *)(d + 0xF8));
        }
        if (*(int *)(d + 0xFC)) {
            func_0020D678_3840((void *)*(int *)(d + 0xFC));
        }
        func_0020D678_3840(m);
        return;
    default:
        return;
    }
}
