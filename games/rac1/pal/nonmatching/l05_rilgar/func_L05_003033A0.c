/* NON_MATCHING func_L05_003033A0 -- src/overlays/l05_rilgar/vendor_002D28D0.c
 * Best so far: BYTES 49/5188 (99.1% of the bytes match), checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Left (scheduling only, same instructions, same registers):
 *   - case 11 (+0x1288/+0x1294): func_L00_00260108 arg moves, a1 set before a0 (retail a0 first).
 *   - case 14 (+0x1370..+0x13b0): 2nd func_L00_00265050 call puts `sw $s0,0($sp)` in the delay slot
 *   (retail stores it first); 3rd call sets a0 after the stack store (retail right after t2).
 *   Tried without effect: float-first 260108 prototype, block-scoped p/r/z locals, void* return, float[] F660 alia
 *   Moby* param with a u8* copy.
 *   Declarations of note: D_L05_00161B58 as `const float` SDATA and a `const` alias of D_0015EE6C for the case-0
 *   F4 store: retail hoists those loads above the preceding pointer stores (d's fields were probably a struct).
 */
extern void func_L00_00264BB0(void *, float);
extern void func_L00_0025B178(void *);
void func_L05_003047E8(Moby *m);
void func_L05_00304F60(struct Moby *m, s32 node, f32 side);
extern void func_L05_0028CEE8(int *, int, int);
extern s32 func_L01_00277FD8(void *, s32, s32, void *, s32, void *, f32);
extern void func_L00_00262DF0_b91A8(int, void *, void *, float) __asm__("func_L00_00262DF0");
extern int func_L00_00262BC0(int, void *, void *, void *);
extern int func_00215570(void *arg0, int arg1);
extern f32 func_001FA850(f32, f32);
extern f32 func_00214D28(f32 *, f32, f32);
extern int func_001F9938(void *);
extern float func_0020D830(void *);
extern float func_001F9B88(float);
extern void func_L00_0025AC00(void *, int, int, void *, void *, float);
extern int func_L00_0025D6F0(void *, void *);
extern void func_L00_00265050(void *, int, void *, void *, int, int, float, void *, void *, void *);
extern void func_L00_00260108(void *, void *, int, float, float);
extern int D_L05_0015F6A8 MACRO_ADDR;
extern char D_L05_001672C0[];
extern unsigned char *D_L05_00160098_c __asm__("D_L05_00160098") MACRO_ADDR;
extern const float D_0015EE6C_k __asm__("D_0015EE6C") MACRO_ADDR;
extern const float D_L05_00161B58_f SDATA(D_L05_00161B58);
extern short D_L05_00161B88;
extern short D_L05_00161B78;
extern short D_L05_00161B80;
extern short D_L05_00161B84;
extern short D_L05_00161B7C;
extern short D_L05_00161B6C;
extern short D_L05_00161B70;
extern short D_L05_00161B74;
extern short D_L05_00161B5C;
extern short D_L05_00161B60;
extern short D_L05_00161B64;
extern short D_L05_00161B68;
extern short D_L05_00161B44;

/* Extermibot update: wakes from its dormant pose, walks its path, closes in on the target and strikes it, bursts into sparks between strikes, and dies on a hit. */
void func_L05_003033A0(Moby *mo) {
    unsigned char *m = (unsigned char *)mo;
    char *d = *(char **)(m + 0x78);
    char *t;
    float dir[4];
    float out[4];
    float hit[4];
    float pos[4];
    int i;
    int j;
    float *rot;
    float yaw;
    unsigned char *pos0;

    func_L00_00264BB0(d + 0x170, 2.5f);
    if (D_L05_0015F6A8 == 2 && ((*(unsigned short *)(m + 0x34) ^ 1) & 1)) {
        *(int *)(d + 0x2DC) = 1;
        *(unsigned short *)(m + 0x34) |= 0x41;
    } else if (*(int *)(d + 0x2DC) != 0 && D_L05_0015F6A8 == 0) {
        *(int *)(d + 0x2DC) = 0;
        *(unsigned short *)(m + 0x34) &= 0xFFBE;
    }
    if (m[0x31] != 0 && func_001F9D10_2D8388((float *)(m + 0x10), (float *)D_L05_001672C0) < 28.0f) {
        func_L00_0025B178(m);
        m[0x7F] = 0x16;
    }
    func_L05_003047E8((Moby *)m);
    t = *(char **)(d + 0x160);
    *(float *)(m + 0x2C) = *(float *)(*(char **)(m + 0x24) + 0x24) * D_L05_00161B58_f;
    switch (m[0x20]) {
    case 0:
        *(unsigned short *)(m + 0x34) |= 0x1000;
        qcopy(d + 0x270, m + 0x10);
        *(float *)(d + 0x20) = 2.0f;
        d[0x29] = 0;
        *(int *)(d + 0x2B0) = 0;
        d[0x28] = 1;
        *(float *)(d + 0x30) = 1.0f;
        *(short *)(d + 0x24) = 2;
        d[0x58] = func_001FA898(14.0f);
        d[0x5A] = func_001FA898(11.2f);
        func_L01_0026E8E0(d + 0xD0);
        *(float *)(d + 0xDC) = 2.0f;
        *(float *)(d + 0xD8) = 2.0f;
        *(int *)(d + 0xD0) = func_001FA898(D_L05_00161B58_f * 409.6f);
        *(float *)(d + 0xF4) = D_0015EE6C_k * 6.0f;
        *(float *)(d + 0x2D8) = func_00214158();
        if (func_002140B0(2) != 0) {
            *(unsigned short *)(m + 0x34) |= 0x8000;
        } else {
            *(unsigned short *)(m + 0x34) &= 0x7FFF;
        }
        if (*(int *)(d + 0x2B4) != 0) {
            m[0x20] = 0xC;
            m[0x31] = 0;
            *(int *)(m + 0x94) = 0;
            *(unsigned short *)(m + 0x34) &= 0xEFFF;
            *(float *)(m + 0x18) -= 20.0f;
            *(short *)(m + 0x32) = 0;
        } else if (*(long *)(d + 0x2B8) != -1) {
            *(unsigned short *)(m + 0x34) &= 0xEFFF;
            m[0x20] = 0xF;
            if (m[0x53] != 0) func_00213DE0(m, 0, 0, 0);
        } else {
            m[0x20] = 1;
            if (m[0x53] != 0) func_00213DE0(m, 0, 0, 0);
        }
        func_L05_0028CEE8((int *)(d + 0x2C0), 1, *(int *)(d + 0x2C4));
        break;
    case 1:
        if (*(int *)(d + 0x164) != 2 && func_002140B0(9) == 0) {
            if (func_L01_00277FD8(d + 0x2C0, 1, *(int *)(d + 0x2C4), m + 0x10, (s32)(t + 0x10), dir,
                                  D_L05_00161B58_f * 0.4f) != 0) {
                if (func_002140B0(0x100) & 1) {
                    *(float *)(d + 0x298) = -*(float *)(d + 0x298);
                }
                *(float *)(d + 0x294) = *(float *)(d + 0x298) * 0.017453292f;
                *(int *)(d + 0x2AC) = func_001F9850((int)func_002140F8(30.0f, 90.0f));
                m[0x20] = 4;
                if (m[0x53] != 1) func_00213DE0(m, 1, 0, func_001F9850(10));
            } else {
                *(int *)(d + 0x164) = 2;
            }
        }
        if (*(int *)(d + 0x164) == 2 && *(int *)(d + 0x2B0) != 0) {
            m[0x20] = 6;
            if (m[0x53] != 1) func_00213DE0(m, 1, 0, func_001F9850(10));
        } else if (m[0x70] & 2) {
            if (m[0x31] != 0 && (func_002140B0(0xFF) & 1)) {
                func_L00_00258DB0((float *)(d + 0x280), 0.0f, 4.0f);
                *(float *)(d + 0x288) = 0.0f;
                func_001F9BD8(d + 0x280, d + 0x280, d + 0x270);
                *(short *)(d + 0x2D2) = 0;
                m[0xBC] = m[0x20];
                m[0x20] = 2;
                if (m[0x53] != 1) func_00213DE0(m, 1, 0, func_001F9850(10));
            } else if (m[0x53] != 0) {
                func_00213DE0(m, 0, 0, func_001F9850(10));
            }
        }
        break;
    case 2:
        rot = (float *)(m + 0x48);
        pos0 = m + 0x10;
        yaw = func_L00_001FF860(*(float *)(d + 0x280) - *(float *)(m + 0x10),
                                *(float *)(d + 0x284) - *(float *)(m + 0x14));
        func_L00_0025CE58(rot, (float *)(d + 0x290), yaw,
                          D_0015EE70 * 6.2831855f, D_0015EE70 * 6.2831855f, D_0015EE6C * 6.2831855f);
        if (func_001FA850(*(float *)(m + 0x48),
                          func_L00_001FF860(*(float *)(d + 0x280) - *(float *)(m + 0x10),
                                            *(float *)(d + 0x284) - *(float *)(m + 0x14))) < 1.5707964f) {
            dir[0] = func_001F9F90(*(float *)(m + 0x48)) * 2.0f;
            dir[1] = func_001F9FA8(*(float *)(m + 0x48)) * 2.0f;
            dir[2] = 0.0f;
            func_L00_00259B88(m, d + 0xD0, dir, out, 1.0f);
            func_L00_00262DF0_b91A8(*(int *)(d + 0x2C0), pos0, pos0, D_L05_00161B58_f * 0.4f);
        }
        (*(short *)(d + 0x2D2))++;
        if ((func_001F9D48(pos0, d + 0x280) < 1.0f && *(short *)(d + 0x2D2) > func_001F9850(0x3C))
            || *(short *)(d + 0x2D2) > func_001F9850(0xB4)) {
            m[0x20] = m[0xBC];
            if (m[0x53] != 0) func_00213DE0(m, 0, 0, func_001F9850(20));
        }
        break;
    case 6: {
        int near;
        rot = (float *)(m + 0x48);
        pos0 = m + 0x10;
        func_00214D28((float *)(d + 0xF4), D_0015EE6C * 6.0f, D_0015EE70 * 6.0f);
        yaw = func_L00_001FF860(*(float *)(t + 0x10) - *(float *)(m + 0x10),
                                *(float *)(t + 0x14) - *(float *)(m + 0x14));
        func_L00_0025CE58(rot, (float *)(d + 0x290), yaw,
                          D_0015EE70 * 6.2831855f, D_0015EE70 * 6.2831855f, D_0015EE6C * 6.2831855f);
        if (func_001FA850(*(float *)(m + 0x48),
                          func_L00_001FF860(*(float *)(t + 0x10) - *(float *)(m + 0x10),
                                            *(float *)(t + 0x14) - *(float *)(m + 0x14))) < 1.5707964f) {
            dir[0] = func_001F9F90(*(float *)(m + 0x48)) * 2.0f;
            dir[1] = func_001F9FA8(*(float *)(m + 0x48)) * 2.0f;
            dir[2] = 0.0f;
            func_L00_00259B88(m, d + 0xD0, dir, out, 1.0f);
            func_L00_00262DF0_b91A8(*(int *)(d + 0x2C0), pos0, pos0, D_L05_00161B58_f * 0.4f);
        }
        near = 0;
        if (func_L00_00262BC0(*(int *)(d + 0x2C0), pos0, t + 0x10, hit) != 0
            && func_001F9D10_2D8388((float *)pos0, hit) < 1.5f) {
            near = 1;
        }
        if (near) {
            m[0x20] = 5;
            if (m[0x53] != 4) func_00213DE0(m, 4, 0, func_001F9850(20));
        } else if (*(int *)(d + 0x2B0) != 0) {
            if (func_001F9D48(pos0, t + 0x10) < 3.0f) {
                m[0x20] = 7;
                *(short *)(d + 0x2D0) = func_001F9850(*(int *)&D_L05_00161B88);
                if (m[0x53] != 2) func_00213DE0(m, 2, 0, func_001F9850(10));
            }
        } else {
            m[0x20] = 10;
        }
        break;
    }
    case 5:
        if (*(int *)(d + 0x2B0) == 0) {
            *(float *)(d + 0xF4) = 0.0f;
            m[0x20] = 10;
            if (m[0x53] != 1) func_00213DE0(m, 1, 0, func_001F9850(20));
        } else if (func_001F9D48(m + 0x10, t + 0x10) < 3.0f) {
            m[0x20] = 7;
            *(short *)(d + 0x2D0) = func_001F9850(*(int *)&D_L05_00161B88);
            if (m[0x53] != 2) func_00213DE0(m, 2, 0, func_001F9850(10));
        }
        break;
    case 15:
        if ((*(int *)(d + 0x2B8) != -1 && D_L05_00160098_c[*(int *)(d + 0x2B8) * 256 + 0xBC] != 0)
            || (*(int *)(d + 0x2BC) != -1 && func_00215570(D_0013E633 + 0xE9D, *(int *)(d + 0x2BC)) != 0)) {
            *(short *)(m + 0x32) = 0x40;
            *(unsigned short *)(m + 0x34) |= 0x1000;
            m[0x20] = 1;
        } else if (m[0x70] & 2) {
            if (func_002140B0(0xFF) & 1) {
                func_L00_00258DB0((float *)(d + 0x280), 0.0f, 4.0f);
                *(float *)(d + 0x288) = 0.0f;
                func_001F9BD8(d + 0x280, d + 0x280, d + 0x270);
                *(short *)(d + 0x2D2) = func_001F9850(0x5A);
                m[0xBC] = m[0x20];
                m[0x20] = 2;
                if (m[0x53] != 1) func_00213DE0(m, 1, 0, func_001F9850(10));
            } else if (m[0x53] != 0) {
                func_00213DE0(m, 0, 0, func_001F9850(10));
            }
        }
        break;
    case 4: {
        float dist = func_001F9D48(m + 0x10, t + 0x10);
        func_L05_00304F60((struct Moby *)m, (s32)(t + 0x10), *(float *)(d + 0x294));
        if (dist < 3.0f
            && func_001FA850(*(float *)(m + 0x48),
                             func_L00_001FF860(*(float *)(t + 0x10) - *(float *)(m + 0x10),
                                               *(float *)(t + 0x14) - *(float *)(m + 0x14))) < 0.7853982f) {
            m[0x20] = 7;
            *(short *)(d + 0x2D0) = func_001F9850(*(int *)&D_L05_00161B88);
            if (m[0x53] != 2) func_00213DE0(m, 2, 0, func_001F9850(10));
        }
        if (*(int *)(d + 0x164) == 2) {
            m[0x20] = 10;
        }
        break;
    }
    case 7:
        rot = (float *)(m + 0x48);
        yaw = func_L00_001FF860(*(float *)(t + 0x10) - *(float *)(m + 0x10),
                                *(float *)(t + 0x14) - *(float *)(m + 0x14));
        func_L00_0025CE58(rot, (float *)(d + 0x290), yaw,
                          D_0015EE70 * 12.566371f, D_0015EE70 * 12.566371f, D_0015EE6C * 25.132742f);
        if ((m[0x70] & 2) && m[0x53] == 2) {
            func_00213DE0(m, 7, 0, func_001F9850(5));
        }
        if (func_001F9938(d + 0x2D0) != 0) {
            float a;
            m[0x20] = 8;
            *(short *)(d + 0x2D0) = 0;
            if (m[0x53] != 3) func_00213DE0(m, 3, 0, func_001F9850(1));
            a = func_001FA748(*(float *)(m + 0x48), 0.0f);
            dir[0] = func_001F9F90(a) * (*(float *)&D_L05_00161B78 * D_0015EE6C);
            dir[1] = func_001F9FA8(a) * (*(float *)&D_L05_00161B78 * D_0015EE6C);
            dir[2] = 0.0f;
            dir[3] = *(float *)&D_L05_00161B80;
            out[2] = 0.0f;
            out[3] = *(float *)&D_L05_00161B84;
            func_L00_00250800(m, 0, hit);
            for (i = 0x1D; i >= 0; i--) {
                int c1, c2;
                qcopy(pos, hit);
                pos[0] += func_002140F8(-0.065f, 0.065f);
                pos[1] += func_002140F8(-0.065f, 0.065f);
                pos[2] += func_002140F8(-0.065f, 0.065f);
                out[0] = func_002140F8(-D_0015EE6C, D_0015EE6C);
                out[1] = func_002140F8(-D_0015EE6C, D_0015EE6C);
                out[2] = func_002140F8(-(*(float *)&D_L05_00161B7C * D_0015EE6C), *(float *)&D_L05_00161B7C * D_0015EE6C);
                c1 = func_001FA898_r(func_001F9878(func_002140F8(*(float *)&D_L05_00161B6C, *(float *)&D_L05_00161B6C * 2.0f)));
                c2 = func_001FA898_r(func_001F9878(func_002140F8(*(float *)&D_L05_00161B70, *(float *)&D_L05_00161B70 * 1.2f)));
                func_00219780(pos, dir, out, *(int *)&D_L05_00161B5C, *(int *)&D_L05_00161B60, c1, c2,
                              func_001FA898_r(func_001F9878(func_002140F8(*(float *)&D_L05_00161B74, *(float *)&D_L05_00161B74 * 1.2f))), -1);
            }
            dir[3] = *(float *)&D_L05_00161B80 * 0.1f;
            out[3] = *(float *)&D_L05_00161B84 * 0.1f;
            for (j = 0x1D; j >= 0; j--) {
                int c1, c2;
                qcopy(pos, hit);
                pos[0] += func_002140F8(-0.065f, 0.065f);
                pos[1] += func_002140F8(-0.065f, 0.065f);
                pos[2] += func_002140F8(-0.065f, 0.065f);
                out[0] = func_002140F8(-D_0015EE6C, D_0015EE6C);
                out[1] = func_002140F8(-D_0015EE6C, D_0015EE6C);
                out[2] = func_002140F8(-(*(float *)&D_L05_00161B7C * D_0015EE6C), *(float *)&D_L05_00161B7C * D_0015EE6C);
                c1 = func_001FA898_r(func_001F9878(func_002140F8(*(float *)&D_L05_00161B6C, *(float *)&D_L05_00161B6C * 2.0f)));
                c2 = func_001FA898_r(func_001F9878(func_002140F8(*(float *)&D_L05_00161B70, *(float *)&D_L05_00161B70 * 1.2f)));
                func_00219780(pos, dir, out, *(int *)&D_L05_00161B64, *(int *)&D_L05_00161B68, c1, c2,
                              func_001FA898_r(func_001F9878(func_002140F8(*(float *)&D_L05_00161B74, *(float *)&D_L05_00161B74 * 1.2f))), -1);
            }
        }
        break;
    case 8: {
        int fl;
        float a = func_001FA748(*(float *)(m + 0x48), 0.0f);
        if (func_0020D830(m) < 1.5f
            && func_001FA850(a, func_L00_001FF860(*(float *)(t + 0x10) - *(float *)(m + 0x10),
                                                  *(float *)(t + 0x14) - *(float *)(m + 0x14))) < 0.2617994f
            && func_001F9D48(m + 0x10, t + 0x10) < 3.0f
            && func_001F9B88(*(float *)(t + 0x18) - *(float *)(m + 0x18)) < 2.0f) {
            dir[0] = func_001F9F90(*(float *)(m + 0x48));
            dir[1] = func_001F9FA8(*(float *)(m + 0x48));
            dir[2] = 0.0f;
            dir[3] = 5627.9248f;
            qcopy(out, t + 0x10);
            out[2] += 0.75f;
            func_L00_0025AC00(t, (int)m, *(int *)(D_0013E633 + 0x1F75) != 6 ? 0x10001 : 0x10000, out, dir, 1.0f);
        }
        if (m[0x70] & 2) {
            m[0x20] = 9;
            if (m[0x53] != 4) func_00213DE0(m, 4, 0, func_001F9850(10));
        }
        break;
    }
    case 9:
        if (m[0x70] & 2) {
            if (func_001F9D48(m + 0x10, t + 0x10) < 3.0f
                && func_001FA850(*(float *)(m + 0x48),
                                 func_L00_001FF860(*(float *)(t + 0x10) - *(float *)(m + 0x10),
                                                   *(float *)(t + 0x14) - *(float *)(m + 0x14))) < 0.2617994f) {
                m[0x20] = 7;
                *(short *)(d + 0x2D0) = func_001F9850(*(int *)&D_L05_00161B88);
                if (m[0x53] != 7) func_00213DE0(m, 7, 0, func_001F9850(5));
            } else {
                *(float *)(d + 0xF4) = 0.0f;
                m[0x20] = 4;
                if (m[0x53] != 1) func_00213DE0(m, 1, 0, func_001F9850(10));
            }
        }
        break;
    case 10:
        func_L05_00304F60((struct Moby *)m, (s32)(d + 0x270), 0.0f);
        if (func_001F9D48(m + 0x10, d + 0x270) < 1.0f) {
            m[0x20] = 1;
            if (m[0x53] != 0) func_00213DE0(m, 0, 0, func_001F9850(3));
        } else if (*(int *)(d + 0x164) != 2) {
            m[0x20] = 4;
            if (m[0x53] != 1) func_00213DE0(m, 1, 0, func_001F9850(3));
        } else if (*(int *)(d + 0x2B0) != 0) {
            m[0x20] = 6;
        }
        break;
    case 11:
        if (func_L00_0025D6F0(m, d + 0x70) & 0x60) {
            if (func_L01_00277FD8(d + 0x2C0, 1, *(int *)(d + 0x2C4), m + 0x10, (s32)(d + 0x270), dir,
                                  D_L05_00161B58_f * 0.4f) != 0) {
                m[0x20] = 4;
                if (m[0x53] != 1) func_00213DE0(m, 1, 0, func_001F9850(10));
                break;
            }
            func_L00_00260108(m, m + 0x10, -1, 0.5f, 13.0f);
        } else if (!(*(float *)(m + 0x18) < 2.0f)) {
            break;
        }
        func_0020D678(mo);
        return;
    case 13:
        if (m[0x70] & 2) {
            m[0x20] = 1;
            if (m[0x53] != 0) func_00213DE0(m, 0, 0, func_001F9850(10));
        }
        break;
    case 14:
        if (func_L00_0025D6F0(m, d + 0x70) & 0x60) {
            func_L00_00260108(m, m + 0x10, -1, 0.5f, 13.0f);
            func_L00_00265050(m, 0x6DA, m + 0x10, m + 0x40, 0, 0, 0.0f, D_L05_0015F660, D_L05_0015F660, D_L05_0015F660);
            func_L00_00265050(m, 0x6DB, m + 0x10, m + 0x40, 0, 0, 0.0f, D_L05_0015F660, D_L05_0015F660, D_L05_0015F660);
            func_L00_00265050(m, 0x6DC, m + 0x10, m + 0x40, 0, 0, 0.0f, D_L05_0015F660, D_L05_0015F660, D_L05_0015F660);
        } else if (!(*(float *)(m + 0x18) < 0.0f)) {
            break;
        }
        func_0020D678(mo);
        return;
    }
    *(int *)&D_L05_00161B44 = *(volatile int *)0x10000000;
}
