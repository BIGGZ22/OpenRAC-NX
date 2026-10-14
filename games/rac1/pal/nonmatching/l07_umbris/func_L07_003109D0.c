/* NON_MATCHING func_L07_003109D0 -- src/overlays/l07_umbris/vendor_002CE470.c
 * Best so far: BYTES 20/6140 (99.7% of the bytes match), checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Same instructions, scheduler order only. The const aliases of D_0015EE70/D_0015EE6C at that
 *   call are needed for retail's loads to sit ahead of the 0x138 store (plain declarations
 *   reload them after it). Making every use const is worse (SIZE 6100).
 *   Port notes: func_L07_003106E8 and func_L00_002592B0 are called through aliases that only
 *   reorder floats against ints (same registers). func_L07_00310038 is called with (moby, data),
 *   as retail passes data in $a1. In the else branch of the inside test, tgt is filled by
 *   func_L00_00260D30(moby, &tgt, 30.0f) before. When data+0x110 != 0 and state >= 7, retail reads
 *   tgt as func_L00_00260D30 left it (the asm does the same).
 */
extern void func_L00_00264B40(void *, int, void *, float);
extern void func_L00_00260D30(void *, void *, float);
extern void func_L00_0025E590(void *, void *);
extern int func_L00_001F3958(void);
extern float func_00214440(void *, int);
extern unsigned char *func_L00_00272488(void *, int, int, float, float);
extern void func_L00_002592B0(char *moby, float *vel, float target, float k, float d, float max);
extern void func_L00_002592B0_t(char *moby, float target, float *vel, float k, float d, float max) __asm__("func_L00_002592B0");
extern void func_L07_003106E8_o(char *, float, char *, float, int, float, int, float, float, int, int, int, int) __asm__("func_L07_003106E8");
extern const float D_0015EE70_c __asm__("D_0015EE70") MACRO_ADDR;
extern const float D_0015EE6C_c __asm__("D_0015EE6C") MACRO_ADDR;
extern float func_L00_00259148(float *vel, float cur, float target, float k, float d, float max);
extern void func_001E9768(void *, void *);
extern int func_L00_0025D6F0(void *, void *);
extern char D_L07_00211AE8[];
extern char D_L07_00211B28[];
extern unsigned char D_L07_00161BD0[3][3] SDATA(D_L07_00161BD0);
extern unsigned char D_L07_00161BE0[3][3] SDATA(D_L07_00161BE0);

struct Tgt_3109D0 {
    float pos[4];
    u128 q10;
    u128 q20;
    u128 q30;
    int moby;
    int kind;
};

/* Update function of the sharkigator (class 1059): tracks the hero, swims its path, lunges and bites. */
void func_L07_003109D0(unsigned char *moby) {
    char *data;
    struct Tgt_3109D0 tgt;
    float pos[4];
    float off[4];
    float rel[4];
    float step[4];
    float old[4];
    int wander;
    int inside;
    int anim;
    int next;
    unsigned char prev;
    unsigned char cur;
    char *hero;

    if (moby == 0) {
        return;
    }
    data = *(char **)(moby + 0x78);
    if (data == 0) {
        return;
    }
    func_L00_00264B40(moby, 0, data + 0x150, 2.2f);
    if (moby[0x20] == 0) {
        if (func_L07_00310110((char *)moby, data)) {
            STUB_printf(D_L07_00211AE8, (int)((char *)moby - D_L07_00160058) >> 8);
            DeleteMoby(moby);
            return;
        }
        goto end;
    }
    func_L00_00260D30(moby, &tgt, 30.0f);
    func_L07_0030FD78((char *)moby, data, (float *)(data + 0x20));
    func_L00_0025E590(moby, data + 0x60);
    wander = func_001F9908_30D8E8((int *)(data + 0x134)) != 0;
    if (*(int *)(data + 0x110) != 0 && moby[0x20] >= 7) {
        char *p = *(char **)(data + 0x148);
        inside = func_L00_0025A778(&tgt, p + 0x10, *(int *)p) != 0;
    } else {
        char *p = *(char **)(data + 0x148);
        hero = D_0013E633 + 0xE9D;
        inside = func_L00_0025A778(hero, p + 0x10, *(int *)p) != 0;
        if (func_001F9D48(moby + 0x10, hero) < 30.0f) {
            tgt.moby = *(int *)(hero + 0x2000);
            qcopy(&tgt.pos, hero);
            qcopy(&tgt.q10, hero + 0x10);
            qcopy(&tgt.q20, hero + 0x40);
            qcopy(&tgt.q30, hero + 0x50);
            tgt.kind = 0;
        } else {
            tgt.moby = 0;
            qzero(&tgt.pos);
            qzero(&tgt.q10);
            qzero(&tgt.q30);
            qzero(&tgt.q20);
            tgt.kind = 2;
        }
    }
    if (*(int *)(data + 0x110) != 0) {
        char *p = *(char **)(data + 0x128);
        func_L00_0025A778(moby + 0x10, p + 0x10, *(int *)p);
    }
    prev = moby[0xBC];
    next = 0xFF;
    anim = 0xFF;
    moby[0xBC] = func_L07_0030FFA8((char *)moby, data);
    cur = moby[0xBC];
    if (prev != cur) {
        next = D_L07_00161BD0[prev][cur];
        anim = D_L07_00161BE0[prev][cur];
    }
    func_L00_00250800(moby, 0, pos);
    if (func_L00_001F10E0(0.8f, pos, 1, moby) != 0 && func_L00_001F3958() == 0) {
        float s;
        float f;
        s = **(float **)(data + 0x12C);
        s -= F(data, 0x130);
        s *= 0.5f;
        if (s < FastAbsF(F(moby, 0x18) - pos[2])) {
            s = (**(float **)(data + 0x12C) - F(data, 0x130)) * 0.5f;
        } else {
            s = FastAbsF(F(moby, 0x18) - pos[2]);
        }
        if (F(data, 0xD8) < 0.0f) {
            s = -s;
        }
        if (moby[0x20] == 0xE || moby[0x20] == 0xC) {
            s += s;
        }
        if (0.5f < func_001F9D48(moby + 0x10, &D_L07_00173F60)) {
            f = func_001F9D48(moby + 0x10, &D_L07_00173F60);
            f += f;
        } else {
            f = 1.0f;
        }
        if (D_L07_0015F6B0 % 45 == *(short *)(moby + 0xB2) && random_integer_below(2) == 0) {
            func_L00_002A5158_30D8E8(D_L07_00161350, D_L07_00161358, 1, F(moby, 0x10), F(moby, 0x14), f, s);
        }
        if (D_L07_0015F6B0 % ((func_001160D8() & 7) + 20) == 0) {
            float size = random_float_between(1.5f, 2.4f);
            pos[0] += random_float_between(-0.15f, 0.15f);
            pos[1] += random_float_between(-0.15f, 0.15f);
            pos[2] = func_00214440(&D_L07_00173F60, 0);
            func_L00_00272488(pos, 0, -1, size, 5250.0f);
        }
    }
    if (1.0f <= F(data, 0x14C)) {
        *(u128 *)(D_0013E633 + 0xE9D) = *(u128 *)(moby + 0x10);
    }

    switch (moby[0x20]) {
    case 0:
        STUB_printf(D_L07_00211B28);
        goto end;
    case 1:
        if ((moby[0x70] & 2) && moby[0x53] != 0) {
            func_00213DE0(moby, 0, 0, 10);
        }
        if (inside && tgt.moby != 0 && tgt.moby == *(int *)(D_0013E633 + 0x2E9D) &&
            !func_L07_00310920(moby, 4, 6)) {
            if (moby[0x53] != 1) {
                func_00213DE0(moby, 1, 0, 10);
            }
            moby[0x20] = 4;
        }
        if (tgt.moby != 0 && tgt.moby == *(int *)(D_0013E633 + 0x2E9D) &&
            func_L07_003108B0(moby, tgt.moby, 1.5f)) {
            if (moby[0x53] != 2) {
                func_00213DE0(moby, 2, 0, 2);
            }
            moby[0x20] = 5;
        } else if (wander) {
            if (moby[0x53] != 1) {
                func_00213DE0(moby, 1, 0, 10);
            }
            *(int *)(data + 0x134) = func_001160D8() % 200 + 300;
            F(data, 0x13C) = random_angle_radians();
            moby[0x20] = 2;
        }
        FastVecScale(data + 0xD0, data + 0xD0, D_0015EE60 * (0.95f - 1.0f) + 1.0f);
        func_L07_003104D8((char *)moby, data, D_0015EE70 * 15.0f, D_0015EE70 * 50.0f, D_0015EE6C * 8.0f);
        if (next != 0xFF) {
            if (moby[0x53] != anim) {
                func_00213DE0(moby, anim, 0, 6);
            }
            moby[0x20] = next;
        }
        goto end;
    case 2:
        if ((moby[0x70] & 2) && moby[0x53] != 1) {
            func_00213DE0(moby, 1, 0, 10);
        }
        if (inside && tgt.moby != 0 && tgt.moby == *(int *)(D_0013E633 + 0x2E9D)) {
            if (!func_L07_00310920(moby, 4, 6)) {
                if (moby[0x53] != 1) {
                    func_00213DE0(moby, 1, 0, 10);
                }
                moby[0x20] = 4;
            }
        } else {
            float a = 1.74532926f;
            float b = 4.5378561f;
            func_L00_002592B0((char *)moby, (float *)(data + 0x120), F(data, 0x13C),
                              D_0015EE70 * a, D_0015EE70 * b, D_0015EE6C * a);
            F(moby, 0x44) = func_L00_00259148((float *)(data + 0x124), F(moby, 0x44), 0.0f,
                                              D_0015EE70 * a, D_0015EE70 * b, D_0015EE6C * a);
            if (wander && func_00215570(moby + 0x10, *(int *)(data + 0x114))) {
                if (moby[0x53] != 0) {
                    func_00213DE0(moby, 0, 0, 10);
                }
                *(int *)(data + 0x134) = func_001160D8() % 120 + 60;
                moby[0x20] = 1;
            }
        }
        func_L07_003102C8((struct Moby *)moby, (struct WalkerData *)data);
        if (func_L07_003105C0((char *)moby, data, F(data, 0x13C), 0.209439516f, D_0015EE70 * 6.0f,
                              0.52359879f, D_0015EE70 * 4.0f, D_0015EE6C * 7.0f, D_0015EE6C * 3.0f) &&
            wander) {
            if (moby[0x53] != 0) {
                func_00213DE0(moby, 0, 0, 10);
            }
            *(int *)(data + 0x134) = func_001160D8() % 120 + 60;
            moby[0x20] = 1;
        }
        if (tgt.moby != 0 && tgt.moby == *(int *)(D_0013E633 + 0x2E9D) &&
            func_L07_003108B0(moby, tgt.moby, 1.5f)) {
            if (moby[0x53] != 2) {
                func_00213DE0(moby, 2, 0, 2);
            }
            moby[0x20] = 5;
        }
        func_L07_003104D8((char *)moby, data, D_0015EE70 * 15.0f, D_0015EE70 * 50.0f, D_0015EE6C * 8.0f);
        if (next != 0xFF) {
            if (moby[0x53] != anim) {
                func_00213DE0(moby, anim, 0, 6);
            }
            moby[0x20] = next;
        }
        goto end;
    case 4: {
        float ang;
        float d;
        int close = 1;
        float pi4 = 12.566371f;
        float three = 3.0f;
        ang = func_L00_001FF860(tgt.pos[0] - F(moby, 0x10), tgt.pos[1] - F(moby, 0x14));
        func_L00_002592B0((char *)moby, (float *)(data + 0x120), ang,
                          D_0015EE70 * pi4, D_0015EE70 * 0.0f, D_0015EE6C * pi4);
        d = func_001F9D48(moby + 0x10, &tgt);
        d = func_L00_001FF860(d, tgt.pos[2] - F(moby, 0x18));
        F(moby, 0x44) = func_L00_00259148((float *)(data + 0x124), F(moby, 0x44), -d,
                                          D_0015EE70 * pi4, D_0015EE70 * 0.0f, D_0015EE6C * pi4);
        if (!(func_001F9D48(moby + 0x10, D_0013E633 + 0xE9D) < three)) {
            close = 0;
        }
        if (close) {
            FastVecScale(data + 0xD0, data + 0xD0, D_0015EE60 * (0.9f - 1.0f) + 1.0f);
        }
        if (func_L07_003105C0((char *)moby, data, ang, 0.209439516f, D_0015EE70 * 20.0f,
                              0.52359879f, D_0015EE70 * three, D_0015EE6C * 20.0f, D_0015EE6C * 5.0f) &&
            close) {
            if (moby[0x53] != 2) {
                func_00213DE0(moby, 2, 4, 2);
            }
            *(int *)(moby + 0x94) = 0;
            moby[0x20] = 5;
            F(data, 0x14C) = 0.0f;
            qcopy(data + 0x100, moby + 0x10);
        }
        F(data, 0x138) = tgt.pos[2] - 0.25f;
        func_L07_003104D8((char *)moby, data, D_0015EE70_c * 38.0f, D_0015EE70_c * 0.0f, D_0015EE6C_c * 38.0f);
        if (!inside || *(short *)(D_0013E633 + 0x1129) == 0) {
            if (moby[0x53] != 1) {
                func_00213DE0(moby, 1, 0, 10);
            }
            moby[0x20] = 2;
        }
        if (next != 0xFF) {
            if (moby[0x53] != anim) {
                func_00213DE0(moby, anim, 0, 6);
            }
            moby[0x20] = next;
        }
        goto end;
    }
    case 5: {
        float ang;
        float d;
        char *mpos;
        char *start;
        float one;
        func_L00_00250800(moby, 0, pos);
        mpos = (char *)moby + 0x10;
        FastVecSub(off, pos, mpos);
        ang = func_L00_001FF860(tgt.pos[0] - pos[0], tgt.pos[1] - pos[1]);
        func_L00_002592B0_t((char *)moby, ang, (float *)(data + 0x120),
                          D_0015EE70 * 12.566371f, D_0015EE70 * 0.0f, D_0015EE6C * 12.566371f);
        F(moby, 0x44) *= D_0015EE60 * 0.7f;
        F(data, 0x14C) += 0.028f;
        qcopy(old, mpos);
        FastVecSub(rel, &tgt, off);
        start = data + 0x100;
        FastVecSub(step, rel, start);
        one = 1.0f;
        FastVecScale(step, step, F(data, 0x14C) < one ? F(data, 0x14C) : one);
        FastVecAdd(mpos, step, start);
        func_001E9768(old, mpos);
        func_001E9768(start, mpos);
        FastVecSub(data + 0xD0, mpos, old);
        if (one <= F(data, 0x14C)) {
            char *g = D_0013E633 + 0xE1D;
            *(int *)(g + 0x22A8) = 0;
            func_L00_00222B80(0x3D, 1);
            *(int *)(*(char **)(g + 0x2080) + 0x98) = -1;
            g[0x20A5] = 1;
            moby[0x20] = 6;
        } else {
            F(data, 0x13C) = ang;
        }
        if (inside || *(short *)(D_0013E633 + 0x1129) != 0) {
            goto end;
        }
        *(int *)(data + 0x14C) = 0;
        if (moby[0x53] != 1) {
            func_00213DE0(moby, 1, 0, 10);
        }
        moby[0x20] = 2;
        goto end;
    }
    case 6: {
        float seven = 7.0f;
        float pi = 3.14159274f;
        float pi2 = 1.57079637f;
        func_L07_003102C8((struct Moby *)moby, (struct WalkerData *)data);
        func_L07_003105C0((char *)moby, data, F(data, 0x13C), 0.209439516f, D_0015EE70 * 6.0f,
                          0.52359879f, D_0015EE70 * 4.0f, D_0015EE6C * seven, D_0015EE6C * 3.0f);
        func_L07_003104D8((char *)moby, data, D_0015EE70 * 12.0f, D_0015EE70 * 50.0f, D_0015EE6C * seven);
        func_L00_002592B0((char *)moby, (float *)(data + 0x120), F(data, 0x13C),
                          D_0015EE70 * pi2, D_0015EE70 * pi, D_0015EE6C * pi);
        F(moby, 0x44) = func_L00_00259148((float *)(data + 0x124), F(moby, 0x44), 0.0f,
                                          D_0015EE70 * pi2, D_0015EE70 * pi, D_0015EE6C * pi);
        if ((moby[0x70] & 2) && moby[0x53] != 1) {
            func_00213DE0(moby, 1, 0, 10);
        }
        if (next != 0xFF) {
            if (moby[0x53] != anim) {
                func_00213DE0(moby, anim, 0, 6);
            }
            moby[0x20] = next;
        }
        goto end;
    }
    case 7:
        if (inside) {
            if (moby[0x53] != 4) {
                func_00213DE0(moby, 4, 0, 10);
            }
            moby[0x20] = 9;
        } else if (wander) {
            if (moby[0x53] != 4) {
                func_00213DE0(moby, 4, 0, 10);
            }
            *(int *)(data + 0x134) = func_001160D8() % 200 + 300;
            F(data, 0x13C) = random_angle_radians();
            moby[0x20] = 8;
        }
        FastVecScale(data + 0xD0, data + 0xD0, D_0015EE60 * (0.95f - 1.0f) + 1.0f);
        F(data, 0xD8) -= D_0015EE6C * 9.8f;
        if (func_L00_00259B08((int)moby, (int)(data + 0xD0), 0x400, 0x10, 1.0f, 0.0f)) {
            FastVecScale(data + 0xD0, data + 0xD0, 0.5f);
        }
        if (next != 0xFF) {
            if (moby[0x53] != anim) {
                func_00213DE0(moby, anim, 0, 6);
            }
            moby[0x20] = next;
        }
        goto end;
    case 8:
        if (inside) {
            if (moby[0x53] != 4) {
                func_00213DE0(moby, 4, 0, 10);
            }
            moby[0x20] = 9;
        } else {
            float a = 0.785398185f;
            float b = 1.57079637f;
            func_L00_002592B0((char *)moby, (float *)(data + 0x120), F(data, 0x13C),
                              D_0015EE70 * a, D_0015EE70 * b, D_0015EE6C * a);
            F(moby, 0x44) = func_L00_00259148((float *)(data + 0x124), F(moby, 0x44), 0.0f,
                                              D_0015EE70 * a, D_0015EE70 * b, D_0015EE6C * a);
            if (wander && func_00215570(moby + 0x10, *(int *)(data + 0x114))) {
                if (moby[0x53] != 3) {
                    func_00213DE0(moby, 3, 0, 10);
                }
                *(int *)(data + 0x134) = func_001160D8() % 120 + 60;
                moby[0x20] = 7;
            }
        }
        func_L07_003102C8((struct Moby *)moby, (struct WalkerData *)data);
        if (func_L07_003105C0((char *)moby, data, F(data, 0x13C), 0.34906584f, D_0015EE70 * 3.0f,
                              1.04719758f, D_0015EE70 * 9.0f, D_0015EE6C, D_0015EE6C) &&
            wander) {
            if (moby[0x53] != 3) {
                func_00213DE0(moby, 3, 0, 10);
            }
            *(int *)(data + 0x134) = func_001160D8() % 120 + 60;
            moby[0x20] = 7;
        }
        F(data, 0xD8) -= D_0015EE6C * 9.8f;
        if (func_L00_00259B08((int)moby, (int)(data + 0xD0), 0x400, 0x10, 1.0f, 0.0f)) {
            FastVecScale(data + 0xD0, data + 0xD0, 0.5f);
        }
        if (next != 0xFF) {
            if (moby[0x53] != anim) {
                func_00213DE0(moby, anim, 0, 6);
            }
            moby[0x20] = next;
        }
        goto end;
    case 9:
    case 10: {
        float ang;
        float d;
        float pi2 = 1.57079637f;
        ang = func_L00_001FF860(tgt.pos[0] - F(moby, 0x10), tgt.pos[1] - F(moby, 0x14));
        func_L00_002592B0_t((char *)moby, ang, (float *)(data + 0x120),
                          D_0015EE70 * 0.785398185f, D_0015EE70 * pi2, D_0015EE6C * 0.785398185f);
        if (func_L07_003105C0((char *)moby, data, ang, 0.52359879f, D_0015EE70 * 4.0f,
                              pi2, D_0015EE70 * 9.0f, D_0015EE6C * 1.5f, D_0015EE6C) &&
            func_001F9D48(moby + 0x10, &tgt) < 1.9f && moby[0x20] != 10) {
            if (moby[0x53] != 5) {
                func_00213DE0(moby, 5, 0, 2);
            }
            moby[0x20] = 10;
        }
        F(data, 0xD8) -= D_0015EE6C * 9.8f;
        if (func_L00_00259B08((int)moby, (int)(data + 0xD0), 0x400, 0x10, 1.0f, 0.0f)) {
            FastVecScale(data + 0xD0, data + 0xD0, 0.5f);
        }
        data[0x58] = func_001FA898_r(8.0f);
        data[0x5A] = func_001FA898_r(9.6f);
        if (!inside) {
            if (moby[0x53] != 4) {
                func_00213DE0(moby, 4, 0, 10);
            }
            moby[0x20] = 8;
        }
        func_L07_003106E8_o((char *)moby, 17.0f, data, 20.0f, 10, 1.0f, 5, 1.0f, 1.0f, 4, 4, 11, tgt.moby);
        if (moby[0x70] & 2) {
            if (moby[0x53] != 4) {
                func_00213DE0(moby, 4, 0, 10);
            }
            *(int *)(data + 0x134) = func_001160D8() % 200 + 300;
            moby[0x20] = 8;
        }
        if (next != 0xFF) {
            if (moby[0x53] != anim) {
                func_00213DE0(moby, anim, 0, 6);
            }
            moby[0x20] = next;
        }
        goto end;
    }
    case 11:
        if (moby[0x70] & 2) {
            moby[0x20] = 7;
        }
        goto end;
    case 12:
        if (func_L00_0025D6F0(moby, data + 0x70) & 1) {
            ((void (*)(char *, char *))func_L07_00310038)((char *)moby, data);
        }
        goto end;
    case 13:
        if (moby[0x70] & 2) {
            ((void (*)(char *, char *))func_L07_00310038)((char *)moby, data);
        }
        goto end;
    case 14:
        if (func_L00_0025D6F0(moby, data + 0x70) & (moby[0x70] & 2)) {
            func_L00_0025F4A8(moby, data + 0xD0, 0, 0.0f, 0.0f, 3, 3, 5, 2.0f, 1.0f, 4.0f, 1.0f, -1, 7.0f,
                              0, 1, -1, 0);
            DeleteMoby(moby);
            return;
        }
        goto end;
    case 16:
        FastVecScale(data + 0xD0, data + 0xD0, D_0015EE60 * (0.95f - 1.0f) + 1.0f);
        F(data, 0xD8) -= D_0015EE6C * 9.8f;
        func_L00_00259B08((int)moby, (int)(data + 0xD0), 0x400, 0x10, 1.0f, 0.0f);
        if (moby[0x70] & 2) {
            ((void (*)(char *, char *))func_L07_00310038)((char *)moby, data);
        }
        goto end;
    case 15:
        FastVecScale(data + 0xD0, data + 0xD0, D_0015EE60 * (0.95f - 1.0f) + 1.0f);
        F(data, 0xD8) -= D_0015EE6C * 9.8f;
        func_L00_00259B08((int)moby, (int)(data + 0xD0), 0x400, 0x10, 1.0f, 0.0f);
        if (next != 0xFF) {
            if (moby[0x53] != anim) {
                func_00213DE0(moby, anim, 0, 6);
            }
            moby[0x20] = next;
        }
        goto end;
    case 17:
        FastVecScale(data + 0xD0, data + 0xD0, D_0015EE60 * (0.95f - 1.0f) + 1.0f);
        if (moby[0x70] & 2) {
            ((void (*)(char *, char *))func_L07_00310038)((char *)moby, data);
        }
        goto end;
    default:
        goto end;
    }
end:
    if (F(moby, 0x10) < 2.0f || 1021.0f < F(moby, 0x10) || F(moby, 0x14) < 2.0f ||
        1021.0f < F(moby, 0x14) || F(moby, 0x18) < 2.0f || 1021.0f < F(moby, 0x18)) {
        DeleteMoby(moby);
    }
}
