/* NON_MATCHING func_L14_002B2A28 -- src/overlays/shared/vendor_002B2A28.c
 * Best so far: BYTES 50/3620 (98.6% of the bytes match), checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   n = --data[0x24C] lands in $v0 in ours, $a1 in retail (same ops); 3 rewrites left it in place.
 *   equiv.py p21.c --show: DIFFERENT (6 missing, 3 extra). Every item read against the assembly, none is a wrong v
 *   - andi r 0xFF x2: retail's redundant zero-extension of an lbu'd byte (no value change).
 *   - lbu r+20: retail fills a beql delay slot with a copy of the common tail's state reload (duplicate).
 *   - beq r #7 vs ours beq r r: the same m[0x52]==7 test; ours sets the 7 in a register reached from two paths, wh
 *   - lw r+0 / sll r 4 "arga1": case 4's path entry pointer and n*16; whichever lands in $a1 is still live at the
 *   following func_L00_002E34F0(m) call, which takes one argument (matched C in this file), so not a real argument
 *   All calls, arguments, constants, conditions and store values otherwise match retail byte for byte.
 */
extern char D_0013E633[];
extern int *D_L14_001B0F30[];
extern int D_L14_0015F6A8 MACRO_ADDR;
extern char D_L14_001FBED0[];
extern char D_L14_001FBF10[];
extern char D_L14_001FBF68[];
extern char D_L14_001FBFA8[];
extern char D_L14_001FC000[];
extern float D_0015EE70 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern u8 D_L14_00174660_2B2A28[] __asm__("D_L14_00174660");
extern short D_L14_001615B8;
extern short D_L14_001615B4;
extern short D_L14_001615B0;
extern short D_L14_001615AC;
extern short D_L14_001615BC;

extern void func_L00_00264B40_2B2A28(float, char *, int, char *) __asm__("func_L00_00264B40");
extern int func_L00_002E34F0(char *);
extern void func_L00_002584A8(void *, int, int);
void func_L14_002B41B0(char *m);
void func_L14_002B44B8(char *moby);
void func_L14_002B4128(struct Moby *m);
extern void func_001E9730_2B2A28(char *, ...) __asm__("func_001E9730");
extern int func_002140B0(int);
void func_L14_002B45A8(char *moby);
extern int func_001FA898(float);
extern void func_0020D960_2B2A28(char *, int, char *) __asm__("func_0020D960");
void func_L14_002B4568(char *moby);
extern float func_00214358(float, void *, int);
extern int func_001F9938(void *);
int func_L14_002DAC00(int i);
extern int func_L14_002B4BF0(char *);
extern int func_001F9850(int);
void func_L14_002B4580(char *moby);
extern void func_00213D28(void *, int, int);
extern float func_L00_001FF860(float, float);
extern float func_0020D830_2B2A28(char *) __asm__("func_0020D830");
int func_L14_002B4668(char *moby);
extern void func_00213DE0_2B2A28(char *, int, int, int) __asm__("func_00213DE0");
extern float func_L00_0025C918(float *p, float *v, float t, float u1, float u2, float eps);
extern float func_L00_0025CE58_2B2A28(float *, float, float *, float, float, float) __asm__("func_L00_0025CE58");
void func_L14_002B47C8(char *moby);
float func_L14_002B4828(char *moby, int flag, float x);
void func_L14_002B4A00(char *a, float x);
int func_L14_002B4B70(int index);
extern int func_L00_002E33D8(void *, int, int);
extern float func_001FA850(float, float);
extern int func_L00_002629E0(int, void *, void *);
void func_L14_002B4AE0(int idx);
extern float func_001FA790(float, float);
extern float func_001FA888_2B2A28(int) __asm__("func_001FA888");
extern void func_0020DAF8_2B2A28(char *, int, void *) __asm__("func_0020DAF8");
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern float func_001F9F90_2B2A28(float) __asm__("func_001F9F90");
extern float func_001F9FA8(float);
extern int func_L00_001EFFF0_2B2A28(void *, void *, int, void *, void *) __asm__("func_L00_001EFFF0");
extern int func_L00_002E3640(void *, void *, void *);
extern int func_L00_0025D6F0(void *, void *);
extern void func_L00_00250800_2B2A28(char *, int, void *) __asm__("func_L00_00250800");
unsigned char *func_L14_002DFE98(char *owner, char *pos);
extern void func_L00_0025F4A8_2B2A28(char *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern void func_L14_002E1570(char *, int *, int);
extern void func_0020D678_2B2A28(char *) __asm__("func_0020D678");
void func_L14_002B3850(struct Moby *m);

typedef int q128_2B2A28 __attribute__((mode(TI)));
typedef struct {
    float a;
    float b;
    float c;
    float d;
    char *moby;
    int flags;
    unsigned char b98;
    unsigned char b99;
    unsigned short cls;
    float e;
    int f;
} Ray_2B2A28;

/* Plasmabot update: validates its path and target, walks the path, turns to aim at its owner's target and fires, dies or despawns. */
void func_L14_002B2A28(char *m) {
    char *data;
    char *owner;
    int st;
    float mat[4][4];
    float v[4];
    float hit[4];
    Ray_2B2A28 ray;
    int x;

    data = *(char **)(m + 0x78);
    owner = 0;
    func_L00_00264B40_2B2A28(2.5f, m, 2, data + 0x120);
    if (*(int *)(data + 0x288) != 0) {
        if (((unsigned char *)D_0013E633)[0x2EC1] != 2) {
            *(unsigned short *)(m + 0x34) |= 0x41;
            *(int *)(m + 0x94) = 0;
            if (((unsigned char *)m)[0x53] == 5) {
                func_L00_002E34F0(m);
                func_L00_002584A8(m, 0, -1);
                { func_0020D678_2B2A28(m); return; }
            }
            ((unsigned char *)m)[0x30] = 0xFF;
            *(unsigned short *)(m + 0x34) &= 0xEFFF;
            return;
        }
        *(unsigned short *)(m + 0x34) &= 0xFFBE;
        if (((unsigned char *)m)[0x20] != 0xB) {
            *(int *)(m + 0x94) = *(int *)(*(char **)(m + 0x24) + 0x10);
        }
        ((unsigned char *)m)[0x30] = 0x40;
        *(unsigned short *)(m + 0x34) |= 0x1000;
    }
    if (((unsigned char *)m)[0x20] != 0) {
        func_L14_002B41B0(m);
        func_L14_002B44B8(m);
        func_L14_002B4128((struct Moby *)m);
        owner = *(char **)(data + 0x110);
    }
    if (D_L14_0015F6A8 == 2) {
        ((unsigned char *)m)[0x31] = 0;
        *(unsigned short *)(m + 0x34) |= 1;
    } else if (((unsigned char *)m)[0x20] != 1) {
        ((unsigned char *)m)[0x31] = 1;
        *(unsigned short *)(m + 0x34) &= 0xFFFE;
    }
    switch (((unsigned char *)m)[0x20]) {
    case 0: {
        if (((unsigned char *)data)[0x24A] != 0) {
            if (*(int *)(data + 0x240) == -1) {
                func_001E9730_2B2A28(D_L14_001FBED0, *(short *)(m + 0xB2));
                { func_0020D678_2B2A28(m); return; }
            }
            if (*D_L14_001B0F30[*(int *)(data + 0x240)] == 0) {
                func_001E9730_2B2A28(D_L14_001FBF10, *(short *)(m + 0xB2));
                { func_0020D678_2B2A28(m); return; }
            }
        }
        if (*(int *)(data + 0x244) == -1) {
            func_001E9730_2B2A28(D_L14_001FBF68, *(short *)(m + 0xB2));
            { func_0020D678_2B2A28(m); return; }
        }
        if (*D_L14_001B0F30[*(int *)(data + 0x244)] == 0) {
            func_001E9730_2B2A28(D_L14_001FBFA8, *(short *)(m + 0xB2));
            { func_0020D678_2B2A28(m); return; }
        }
        if (((unsigned char *)data)[0x24A] != 0 && *(int *)(data + 0x264) < 0) {
            func_001E9730_2B2A28(D_L14_001FC000, *(short *)(m + 0xB2));
            { func_0020D678_2B2A28(m); return; }
        }
        if (func_002140B0(2) != 0) {
            *(unsigned short *)(m + 0x34) |= 0x8000;
        }
        func_L14_002B45A8(m);
        if (*(int *)(data + 0x240) != -1) {
            qcopy(m + 0x10, (char *)D_L14_001B0F30[*(int *)(data + 0x240)] + 0x10);
        }
        data[0x28] = 1;
        data[0x29] = 0;
        *(float *)(data + 0x20) = 2.0f;
        *(float *)(data + 0x30) = 1.25f;
        *(short *)(data + 0x24) = 2;
        data[0x58] = func_001FA898(15.0f);
        data[0x5A] = func_001FA898(16.0f);
        *(float *)(data + 0x27C) = 1.0f;
        *(short *)(data + 0x248) = 0;
        *(int *)(data + 0x23C) = 0;
        *(short *)(data + 0x260) = 0;
        *(int *)(data + 0x268) = 0;
        *(int *)(data + 0x26C) = 0;
        *(int *)(data + 0x280) = 0;
        *(int *)(data + 0x284) = 0;
        *(int *)(data + 0x270) = 0;
        *(int *)(data + 0x274) = 0;
        if (((unsigned char *)data)[0x121] == 0) {
            func_0020D960_2B2A28(m, 2, data + 0x120);
        }
        if (((unsigned char *)data)[0x161] == 0) {
            func_0020D960_2B2A28(m, 3, data + 0x160);
        }
        if (((unsigned char *)data)[0x24A] != 0) {
            func_L14_002B4568(m);
            ((unsigned char *)m)[0x20] = 1;
            return;
        }
        *(float *)(m + 0x18) = func_00214358(0.5f, m + 0x10, 0);
        if (*(int *)(data + 0x240) == -1) {
            ((unsigned char *)m)[0x20] = 4;
        } else {
            ((unsigned char *)m)[0x20] = 0xD;
        }
        return;
    }
    case 1: {
        float *p;
        if (*(int *)(data + 0x114) == 2) return;
        if (func_001F9938(data + 0x248) == 0) return;
        if (func_L14_002DAC00(*(int *)(data + 0x264)) == 0) return;
        if (func_L14_002B4BF0(m) == 0) return;
        *(short *)(data + 0x248) = func_001F9850(*(int *)&D_L14_001615B8);
        func_L14_002B4580(m);
        ((unsigned char *)m)[0x20] = 2;
        func_00213D28(m, 1, 0);
        p = (float *)D_L14_001B0F30[*(int *)(data + 0x240)];
        *(float *)(m + 0x48) = func_L00_001FF860(p[8] - p[4], p[9] - p[5]);
        *(int *)(data + 0x268) = 0;
        *(float *)(data + 0x27C) = 1.0f;
        *(int *)(data + 0x26C) = 0;
        *(int *)(data + 0x270) = 0;
        *(int *)(data + 0x274) = 0;
        *(int *)(data + 0x24C) = 0;
        *(int *)(data + 0x250) = 0;
        *(float *)(data + 0x25C) = *(float *)(data + 0x268);
        *(float *)(data + 0x258) = *(float *)(data + 0x268);
        *(float *)(data + 0x254) = *(float *)(data + 0x268);
        return;
    }
    case 2:
        if (func_0020D830_2B2A28(m) >= 3.0f) {
            func_L14_002B4668(m);
        }
        if ((m[0x70] & 2) && ((unsigned char *)m)[0x52] == ((unsigned char *)m)[0x53]) {
            ((unsigned char *)m)[0x20] = 3;
            func_00213DE0_2B2A28(m, 3, 0, func_001F9850(10));
            *(float *)(m + 0x58) = 1.0f;
        }
        break;
    case 9: {
        float *p = (float *)D_L14_001B0F30[*(int *)(data + 0x240)];
        func_L00_0025C918((float *)(m + 0x10), (float *)(data + 0x254), p[4], 0.01f, 0.2f, 0.0f);
        func_L00_0025C918((float *)(m + 0x14), (float *)(data + 0x258), p[5], 0.01f, 0.2f, 0.0f);
        if ((m[0x70] & 2) && ((unsigned char *)m)[0x52] == ((unsigned char *)m)[0x53]) {
            func_00213DE0_2B2A28(m, 0, 0, func_001F9850(10));
            ((unsigned char *)m)[0x20] = 1;
            *(float *)(m + 0x58) = 1.0f;
            func_L14_002B4568(m);
            return;
        }
        break;
    }
    case 13: {
        float ang;
        float k;
        ang = func_L00_001FF860(*(float *)(owner + 0x10) - *(float *)(m + 0x10),
                                *(float *)(owner + 0x14) - *(float *)(m + 0x14));
        k = D_0015EE70 * 12.566371f;
        func_L00_0025CE58_2B2A28((float *)(m + 0x48), ang, (float *)(data + 0x23C), k, k, D_0015EE6C * 6.2831855f);
        if (*(int *)(data + 0x114) == 2) break;
        if (func_001F9938(data + 0x248) == 0) break;
        ((unsigned char *)m)[0x20] = 3;
        func_00213DE0_2B2A28(m, 3, 0, func_001F9850(10));
        *(float *)(m + 0x58) = 1.0f;
        *(int *)(data + 0x250) = 0;
        *(float *)(data + 0x27C) = 1.0f;
        *(int *)(data + 0x24C) = 0;
        *(float *)(data + 0x25C) = *(float *)(data + 0x250);
        *(float *)(data + 0x258) = *(float *)(data + 0x250);
        *(float *)(data + 0x254) = *(float *)(data + 0x250);
        break;
    }
    case 3:
        if (func_L14_002B4668(m) != 0) {
            func_00213DE0_2B2A28(m, 0, 0, func_001F9850(10));
            ((unsigned char *)m)[0x20] = 4;
        }
        break;
    case 8:
        if (((unsigned char *)data)[0x24A] != 0) {
            func_L14_002DAC00(*(int *)(data + 0x264));
        }
        if (func_L14_002B4668(m) != 0) {
            if (((unsigned char *)data)[0x24A] != 0) {
                func_00213DE0_2B2A28(m, 2, 0, func_001F9850(10));
                ((unsigned char *)m)[0x20] = 9;
            } else {
                *(short *)(data + 0x248) = func_001F9850(*(int *)&D_L14_001615B8);
                ((unsigned char *)m)[0x20] = 0xD;
            }
        }
        break;
    case 4: {
        float ang;
        float k;
        float aim;
        func_L14_002B47C8(m);
        ang = func_L00_001FF860(*(float *)(owner + 0x10) - *(float *)(m + 0x10),
                                *(float *)(owner + 0x14) - *(float *)(m + 0x14));
        k = D_0015EE70 * 12.566371f;
        func_L00_0025CE58_2B2A28((float *)(m + 0x48), ang, (float *)(data + 0x23C), k, k, D_0015EE6C * 6.2831855f);
        aim = func_L14_002B4828(m, 0, *(float *)(m + 0x48));
        func_L14_002B4A00(m, ang);
        if (*(int *)(data + 0x114) == 2) {
            if (*(int *)(data + 0x240) == -1) break;
            ((unsigned char *)m)[0x20] = 8;
            if (((unsigned char *)m)[0x53] != 3) {
                func_00213DE0_2B2A28(m, 3, 0, func_001F9850(0x14));
            }
            *(float *)(data + 0x250) = *(float *)((char *)D_L14_001B0F30[*(int *)(data + 0x240)] + --*(int *)(data + 0x24C) * 16 + 0x1C);
            *(float *)(data + 0x27C) = -1.0f;
            *(float *)(data + 0x254) = *(float *)(data + 0x258) = *(float *)(data + 0x25C) = 0.0f;
            func_L00_002E34F0(m);
            break;
        }
        if (func_001F9938(data + 0x260) == 0) break;
        if (func_L14_002B4B70(((unsigned char *)m)[0x21]) >= 2) break;
        if (func_L00_002E33D8(m, 1, 0) == 0) break;
        if (func_002140B0(4) != 0) break;
        if (!(func_001FA850(*(float *)(m + 0x48), ang) < 0.017453292f)) break;
        if (!(aim < 0.017453292f)) break;
        if (func_L00_002629E0(*(int *)(data + 0x244), m + 0x10, owner + 0x10) != 0) break;
        func_L14_002B4AE0(((unsigned char *)m)[0x21]);
        *(float *)(data + 0x278) = ang;
        ((unsigned char *)m)[0x20] = 5;
        func_00213DE0_2B2A28(m, 4, 0, func_001F9850(5));
        *(short *)(data + 0x262) = func_001F9850(*(int *)&D_L14_001615B4);
        break;
    }
    case 5: {
        float ang;
        ang = func_L00_001FF860(*(float *)(owner + 0x10) - *(float *)(m + 0x10),
                                *(float *)(owner + 0x14) - *(float *)(m + 0x14));
        func_L14_002B4828(m, 0, func_001FA790(ang, *(float *)&D_L14_001615B0));
        func_L14_002B4A00(m, ang);
        if ((m[0x70] & 2) && ((unsigned char *)m)[0x52] == ((unsigned char *)m)[0x53]) {
            func_00213DE0_2B2A28(m, 5, 0, func_001F9850(10));
            ((unsigned char *)m)[0x20] = 6;
        }
        break;
    }
    case 6: {
        int flag;
        float ang;
        float t;
        float c;
        float f;
        flag = 0;
        ang = func_L00_001FF860(*(float *)(owner + 0x10) - *(float *)(m + 0x10),
                                *(float *)(owner + 0x14) - *(float *)(m + 0x14));
        t = func_001FA888_2B2A28(*(short *)(data + 0x262));
        t = t / func_001FA888_2B2A28(func_001F9850(*(int *)&D_L14_001615B4));
        c = *(float *)&D_L14_001615B0;
        f = func_001FA790(*(float *)(data + 0x278), -c + (c + c) * t);
        if (((unsigned char *)data)[0x24B] != 0) {
            if (*(float *)(owner + 0x18) + 0.1f - *(float *)(m + 0x18) < 0.0f) {
                flag = 1;
            }
        }
        func_L14_002B4828(m, flag, f);
        func_L14_002B4A00(m, ang);
        func_0020DAF8_2B2A28(m, 0, mat);
        qcopy(v, mat[1]);
        if (flag == 0) {
            v[2] = 0.0f;
        }
        func_L00_001FF4B0(v, v, *(float *)&D_L14_001615AC);
        func_001F9BD8(hit, mat[3], v);
        func_L00_001FF4B0(v, v, *(float *)&D_L14_001615BC);
        func_001F9BD8(mat[3], mat[3], v);
        ray.moby = m;
        ray.flags = 0x10001;
        ray.f = 1;
        ray.e = 1.0f;
        ray.a = func_001F9F90_2B2A28(*(float *)(m + 0x48));
        ray.b = func_001F9FA8(*(float *)(m + 0x48));
        ray.c = 1.0f;
        ray.d = 5627.9248f;
        ray.b99 = 1;
        ray.cls = *(unsigned short *)(m + 0xA6);
        ray.b98 = 0;
        if (func_L00_001EFFF0_2B2A28(mat[3], hit, 0, m, &ray) != 0) {
            qcopy(hit, D_L14_00174660_2B2A28);
        }
        func_L00_002E3640(m, mat, hit);
        if (func_001F9938(data + 0x262) != 0) {
            func_00213DE0_2B2A28(m, 6, 0, func_001F9850(10));
            ((unsigned char *)m)[0x20] = 7;
            func_L00_002E34F0(m);
        }
        break;
    }
    case 7:
        if ((m[0x70] & 2) && ((unsigned char *)m)[0x52] == ((unsigned char *)m)[0x53]) {
            func_00213DE0_2B2A28(m, 0, 0, func_001F9850(10));
            ((unsigned char *)m)[0x20] = 4;
        }
        break;
    case 10: {
        if (((m[0x70] & 2) && m[0x52] == m[0x53] && m[0x52] == 7)
            || (m[0x52] != 7 && m[0x53] != 7)) {
            func_00213DE0_2B2A28(m, 0, 0, 10);
            ((unsigned char *)m)[0x20] = 4;
        }
        break;
    }
    case 11: {
        if ((func_L00_0025D6F0(m, data + 0x60) & 0x160) == 0) return;
        func_L00_002E34F0(m);
        func_L00_00250800_2B2A28(m, 2, mat);
        func_L14_002DFE98(m, (char *)mat);
        func_L00_0025F4A8_2B2A28(m, data + 0x40, m + 0x10, 0.0f, 0.0f, 5, 2, 4, 2.0f, 1.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
        if (((unsigned char *)D_0013E633)[0x2EC1] == 2) {
            func_L00_002584A8(m, 0x200, -1);
            { func_0020D678_2B2A28(m); return; }
        }
        func_L00_002584A8(m, 0, -1);
        { func_0020D678_2B2A28(m); return; }
    }
    case 12: {
        func_L00_00250800_2B2A28(m, 2, mat);
        func_L14_002DFE98(m, (char *)mat);
        x = 0;
        func_L14_002E1570(m, &x, 1);
        func_L00_002E34F0(m);
        func_0020D678_2B2A28(m);
        return;
    }
    }
    st = ((unsigned char *)m)[0x20];
    if (!(st >= 4 && st <= 7) && st != 1) {
        func_L14_002B4828(m, 0, *(float *)(m + 0x48));
        func_L14_002B4A00(m, *(float *)(m + 0x48));
    }
    func_L14_002B3850((struct Moby *)m);
}
