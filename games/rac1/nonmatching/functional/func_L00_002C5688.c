/* functional: medium. corrected from nonmatching/functional/func_L00_002C5688.c: the hero-state 0x31 pull passed moby+0x10 where retail passes moby (func_L00_0025AC00 reads target+0xA4), the 0x24-byte effect block handed to func_L00_001F2BE8 was split over separate locals, and the vars+0x68 mask used 32-bit long. Thrown-object moby update (the state byte at moby+0x20 picks: 0 init, 1 held (killed if the hero lets go), 2 flying (gravity, push-apart from other mobys, sphere collision with bounce/stick), 3 and 4 landed/rolling (hit detection against the nearest hurt, damage and explosion when the vars+0x48 health drops below 0; 4 times out into 5), 5/6 fade and kill). Written from the asm with the NTSC decomp draft as the guide; PAL adds the hero-state 0x31 pull toward the hero before the hit test. Not checkable by real calls (not called before level 1). */
extern int func_001F9850_C5688(int) __asm__("func_001F9850");
extern float func_001FA888_C5688(int) __asm__("func_001FA888");
extern void func_L00_00260878_C5688(void *, void *) __asm__("func_L00_00260878");
extern void func_0020D678_C5688(void *) __asm__("func_0020D678");
extern float func_001F9CB8_C5688(void *) __asm__("func_001F9CB8");
extern int func_L00_0025D390_C5688(void *) __asm__("func_L00_0025D390");
extern int func_L00_002C4E00_C5688(void *, void *, int) __asm__("func_L00_002C4E00");
extern void func_001F9BF0_C5688(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_001F9BD8_C5688(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9C30_C5688(void *, void *, float) __asm__("func_001F9C30");
extern int func_L00_00259868_C5688(void *, void *, float, float, float, int) __asm__("func_L00_00259868");
extern int func_L00_001F10E0_C5688(float, void *, int, void *) __asm__("func_L00_001F10E0");
extern void func_L00_001FF610_C5688(void *, void *, void *) __asm__("func_L00_001FF610");
extern float func_001F9CE8_C5688(void *) __asm__("func_001F9CE8");
extern float func_001F9D48_C5688(void *, void *) __asm__("func_001F9D48");
extern float func_L00_001FF860_C5688(float, float) __asm__("func_L00_001FF860");
extern void func_00213DE0_C5688(void *, int, int, int) __asm__("func_00213DE0");
extern float func_001F9F90_C5688(float) __asm__("func_001F9F90");
extern float func_001F9FA8_C5688(float) __asm__("func_001F9FA8");
extern float func_001F9B50_C5688(float) __asm__("func_001F9B50");
extern float func_001FA790_C5688(float, float) __asm__("func_001FA790");
extern float func_001FA748_C5688(float, float) __asm__("func_001FA748");
extern void func_L00_002C5630_C5688(void *) __asm__("func_L00_002C5630");
extern int func_L00_001FEF78_C5688(void *) __asm__("func_L00_001FEF78");
extern void func_0022ED80_C5688(int, int, void *) __asm__("func_0022ED80");
extern void func_L00_00260958_C5688(float, void *) __asm__("func_L00_00260958");
extern void func_L00_0025AD38_C5688(float, void *, void *, int, void *) __asm__("func_L00_0025AD38");
extern char *func_L00_0025B478_C5688(void *, int, int) __asm__("func_L00_0025B478");
extern void func_L00_002C53A8_C5688(void *) __asm__("func_L00_002C53A8");
extern int func_001F9908_C5688(void *) __asm__("func_001F9908");
extern int func_001F9938_C5688(void *) __asm__("func_001F9938");
extern void func_L00_0025E4B0_C5688(void *, void *) __asm__("func_L00_0025E4B0");
extern void func_L00_0025F4A8_C5688(void *, void *, int, int, int, int, int, int,
                                    float, float, float, float, float, float, float,
                                    int, int, int) __asm__("func_L00_0025F4A8");
extern void func_L00_0025A890_C5688(float, void *, void *, int) __asm__("func_L00_0025A890");
extern void func_L00_001F2BE8_C5688(float, void *, int, void *, void *) __asm__("func_L00_001F2BE8");
extern void func_L00_0025E590_C5688(void *, void *) __asm__("func_L00_0025E590");
extern float func_002140F8_C5688(float, float) __asm__("func_002140F8");
extern float func_00214358_C5688(void *, int, float) __asm__("func_00214358");
extern void func_L00_00261478_C5688(void *, void *, void *, void *, void *, void *) __asm__("func_L00_00261478");
extern char D_0013F450_C5688[] __asm__("D_0013F450");
extern unsigned char D_0013E633_C5688[] __asm__("D_0013E633");
extern short D_L00_001B0BB0_C5688[] __asm__("D_L00_001B0BB0");
extern char *D_L00_00160098_C5688 __asm__("D_L00_00160098");
extern char D_L00_00173F40_C5688[] __asm__("D_L00_00173F40");
extern float D_0015EE60_C5688 __asm__("D_0015EE60");
extern float D_0015EE6C_C5688 __asm__("D_0015EE6C");
extern float D_0015EE70_C5688 __asm__("D_0015EE70");

void func_L00_002C5688(char *moby) {
    char *hero = D_0013F450_C5688;
    char *hit_info = D_L00_00173F40_C5688;
    int active = D_0013E633_C5688[6];
    char *vars = *(char **)(moby + 0x78);
    char *target, *found, *ground;
    /* One block like retail's frame (sp+0x20 v, +0x30 wv, +0x40 xv, +0x50 cv):
       func_L00_001F2BE8 reads 0x24 bytes from wv, including the bytes set at xv+8..11. */
    float sb[16] __attribute__((aligned(16)));
    float *v = sb, *wv = sb + 4, *xv = sb + 8, *cv = sb + 12;
    float len, len2, angle, cos_a, sin_a, rate, effect, fx;
    int i, ok;
    short *ids;

    if (*(short *)(moby + 0xA6) == 0xCB) active = 0;
    effect = func_001FA888_C5688(active);
    if (*(float *)(moby + 0x10) < 2.0f || 1021.0f < *(float *)(moby + 0x10) ||
        *(float *)(moby + 0x14) < 2.0f || 1021.0f < *(float *)(moby + 0x14) ||
        *(float *)(moby + 0x18) < 2.0f || 1021.0f < *(float *)(moby + 0x18)) {
        func_L00_00260878_C5688(moby, D_L00_001B0BB0_C5688);
        func_0020D678_C5688(moby);
        return;
    }
    if ((*(char **)(hero + 0x1090) == 0 || *(short *)(*(char **)(hero + 0x1090) + 0xA6) != 0x232) &&
        *(unsigned char *)(moby + 0x20) == 1) {
        func_L00_00260878_C5688(moby, D_L00_001B0BB0_C5688);
        func_0020D678_C5688(moby);
        return;
    }
    if (*(unsigned char *)(hero + 0x20A4) != 0 && *(unsigned char *)(moby + 0x20) == 1) {
        func_L00_00260878_C5688(moby, D_L00_001B0BB0_C5688);
        func_0020D678_C5688(moby);
        return;
    }
    *(int *)(vars + 0x54) = *(int *)(vars + 0x54) + 1;
    if (*(unsigned char *)(hero + 0x20A5) != 0 && *(unsigned char *)(moby + 0x20) < 2)
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) | 0x41;
    else
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) & 0xFFBE;
    if (*(short *)(*(char **)(vars + 0x40) + 0xA6) == 0x232 && *(short *)(vars + 0x46) == 0 &&
        func_001F9CB8_C5688(vars + 0x10) > 0.0f) {
        ground = *(char **)(hero + 0x2FC);
        if (ground == 0 || func_L00_0025D390_C5688(ground) == 0) {
            func_L00_002C4E00_C5688(vars, moby, 1);
        } else if (func_L00_002C4E00_C5688(vars, moby, 0) == 0) {
            func_L00_002C4E00_C5688(vars, moby, 2);
        }
    }
    switch (*(unsigned char *)(moby + 0x20)) {
    case 0:
        *(unsigned char *)(moby + 0x20) = 1;
        *(short *)(vars + 0x44) = 0;
        *(int *)(moby + 0x98) = -1;
        *(unsigned char *)(vars + 0x6E) = 0;
        *(short *)(vars + 0x6C) = 0;
        break;
    case 1:
        *(short *)(vars + 0x44) = 0;
        target = *(char **)(vars + 0x40);
        if (target != 0 && (*(unsigned char *)(target + 0x20) & 0xF0) != 0xF0 &&
            *(char **)(hero + 0x1090) != 0 && *(short *)(*(char **)(hero + 0x1090) + 0xA6) == 0x232)
            return;
        func_L00_00260878_C5688(moby, D_L00_001B0BB0_C5688);
        func_0020D678_C5688(moby);
        return;
    case 2:
        if (*(float *)(moby + 0x2C) < *(float *)(*(char **)(moby + 0x24) + 0x24))
            *(float *)(moby + 0x2C) = *(float *)(moby + 0x2C) + D_0015EE60_C5688 * 0.02f;
        qcopy(v, moby + 0x10);
        *(float *)(vars + 0x18) = *(float *)(vars + 0x18) - D_0015EE70_C5688 * 9.0f;
        for (i = 1; i <= D_L00_001B0BB0_C5688[0]; i++) {
            ids = D_L00_001B0BB0_C5688 + i;
            target = D_L00_00160098_C5688 + *ids * 0x100;
            if (target != moby && *(unsigned char *)(target + 0x20) > 1 && target != 0 &&
                *(unsigned char *)(target + 0x20) != 0xFE && *(unsigned char *)(target + 0x20) != 0xFD) {
                func_001F9BF0_C5688(wv, moby + 0x10, target + 0x10);
                wv[2] = 0.0f;
                len = func_001F9CB8_C5688(wv);
                if (len < 1.8f) {
                    func_001F9C30_C5688(wv, wv, (1.8f - len) * 0.015f);
                    func_001F9BD8_C5688(vars + 0x10, vars + 0x10, wv);
                }
            }
        }
        if (*(int *)(hero + 0x2084) == 0x1E)
            *(int *)(*(char **)(hero + 0x2080) + 0x94) = 0;
        ok = func_L00_00259868_C5688(moby, vars + 0x10, 0.2f, 0.2f, 0.0f, 0);
        if (ok & 1) {
            qcopy(wv, moby + 0x10);
            wv[2] = wv[2] + 0.2f;
            if (func_L00_001F10E0_C5688(0.23f, wv, 0, moby) != 0) {
                if (*(float *)(vars + 0x18) < 0.0f) {
                    func_001F9BF0_C5688(xv, moby + 0x10, hero + 0x80);
                    xv[2] = 0.0f;
                    len = func_001F9CB8_C5688(xv);
                    if (len < 1.8f) {
                        func_001F9C30_C5688(xv, xv, (1.8f - len) * 0.015f);
                        func_001F9BD8_C5688(vars + 0x10, vars + 0x10, xv);
                    }
                }
                func_L00_001FF610_C5688(vars + 0x10, vars + 0x10, hit_info + 0x40);
                if (*(int *)(hit_info + 0x18) != 0 && *(int *)(hit_info + 0x1C) <= 0)
                    func_001F9C30_C5688(vars + 0x10, vars + 0x10, 1.05f);
                else
                    func_001F9C30_C5688(vars + 0x10, vars + 0x10, 0.2f);
                len = func_001F9CB8_C5688(vars + 0x10);
                ground = *(char **)(hero + 0x2FC);
                if (len < D_0015EE60_C5688 * 0.01f ||
                    (ground != 0 && func_L00_0025D390_C5688(ground) != 0 &&
                     func_001F9CB8_C5688(vars + 0x10) < D_0015EE60_C5688 * 0.01f + func_001F9CB8_C5688(hero + 0x100))) {
                    angle = func_L00_001FF860_C5688(*(float *)(hit_info + 0x48), func_001F9CE8_C5688(hit_info + 0x40));
                    if (angle < 0.61086524f) {
                        *(short *)(vars + 0x44) = (short)(*(unsigned short *)(vars + 0x44) + 1);
                        if (*(int *)(hit_info + 0x18) == 0 || *(int *)(hit_info + 0x1C) > 0 ||
                            func_001F9850_C5688(240) < *(short *)(vars + 0x44)) {
                            if (*(unsigned char *)(moby + 0x53) != 1)
                                func_00213DE0_C5688(moby, 1, 0, func_001F9850_C5688(10));
                            *(int *)(moby + 0x98) = 0;
                            *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
                            v[0] = 0.0f; v[1] = 0.0f; v[2] = 0.0f; v[3] = 0.0f;
                            qcopy(vars + 0x10, v);
                            *(unsigned char *)(moby + 0x20) = 3;
                            *(short *)(vars + 0x44) = (short)func_001F9850_C5688(600);
                            *(float *)(moby + 0x18) = *(float *)(hit_info + 0x28);
                            qcopy(xv, hit_info + 0x40);
                            cos_a = func_001F9F90_C5688(*(float *)(moby + 0x48));
                            sin_a = func_001F9FA8_C5688(*(float *)(moby + 0x48));
                            cv[0] = xv[0] * cos_a + xv[1] * sin_a;
                            cos_a = func_001F9F90_C5688(*(float *)(moby + 0x48));
                            sin_a = func_001F9FA8_C5688(*(float *)(moby + 0x48));
                            cv[1] = xv[1] * cos_a - xv[0] * sin_a;
                            cv[2] = xv[2];
                            *(float *)(vars + 0x4C) = -func_L00_001FF860_C5688(func_001F9B50_C5688(cv[0] * cv[0] + cv[2] * cv[2]), cv[1]);
                            *(float *)(vars + 0x50) = func_L00_001FF860_C5688(cv[2], cv[0]);
                            rate = 0.1f;
                            *(float *)(moby + 0x40) = func_001FA748_C5688(*(float *)(moby + 0x40),
                                func_001FA790_C5688(*(float *)(vars + 0x4C), *(float *)(moby + 0x40)) * (D_0015EE60_C5688 * rate));
                            *(float *)(moby + 0x44) = func_001FA748_C5688(*(float *)(moby + 0x44),
                                func_001FA790_C5688(*(float *)(vars + 0x50), *(float *)(moby + 0x44)) * (D_0015EE60_C5688 * rate));
                        }
                    }
                }
                func_L00_002C5630_C5688(moby);
                if (func_L00_001FEF78_C5688(vars + 0x6F) != 0) {
                    func_0022ED80_C5688(0, 0, moby);
                    *(unsigned char *)(vars + 0x6F) = (unsigned char)func_001F9850_C5688(20);
                } else {
                    func_L00_00260958_C5688(D_0015EE6C_C5688, vars + 0x10);
                }
            }
        }
        if (*(int *)(hero + 0x2084) == 0x1E) {
            target = *(char **)(hero + 0x2080);
            *(int *)(target + 0x94) = *(int *)(*(char **)(target + 0x24) + 0x10);
        }
        break;
    case 3:
    case 4:
        if (*(int *)(hero + 0x2084) == 0x31) {
            if (func_001F9D48_C5688(moby + 0x10, hero + 0x80) < 1.5f)
                func_L00_0025AD38_C5688(1000.0f, moby, moby, 0x10000, hero + 0x80);
        }
        found = func_L00_0025B478_C5688(moby, 0x30001, 0);
        *(float *)(moby + 0x40) = func_001FA748_C5688(*(float *)(moby + 0x40),
            func_001FA790_C5688(*(float *)(vars + 0x4C), *(float *)(moby + 0x40)) * (D_0015EE60_C5688 * 0.1f));
        *(float *)(moby + 0x44) = func_001FA748_C5688(*(float *)(moby + 0x44),
            func_001FA790_C5688(*(float *)(vars + 0x50), *(float *)(moby + 0x44)) * (D_0015EE60_C5688 * 0.1f));
        func_L00_002C53A8_C5688(moby);
        func_001F9908_C5688(vars + 0x5C);
        func_001F9938_C5688(vars + 0x6C);
        if (found != 0) {
            if (*(int *)(found + 0x20) != *(int *)(vars + 0x58) || *(int *)(vars + 0x5C) <= 0) {
                *(int *)(vars + 0x58) = *(int *)(found + 0x20);
                *(int *)(vars + 0x5C) = func_001F9850_C5688(30);
                if (*(short *)(vars + 0x6C) == 0) {
                    if (*(unsigned short *)(found + 0x28) == 0x101 &&
                        (*(unsigned long long *)(vars + 0x68) & 0xFFFFFF00000000ULL) == 0) {
                        *(short *)(vars + 0x6C) = (short)func_001F9850_C5688(100);
                        *(unsigned char *)(vars + 0x6E) = 1;
                    } else {
                        *(unsigned char *)(vars + 0x6E) = 0;
                    }
                    func_0022ED80_C5688(0, 0, moby);
                    fx = *(float *)(found + 0x2C);
                    *(unsigned char *)(vars + 7) = 0xFA;
                    *(float *)(vars + 0x48) = *(float *)(vars + 0x48) - fx;
                    func_L00_0025E4B0_C5688(moby, vars);
                    if (*(float *)(vars + 0x48) < 0.0f) {
                        float a = D_0015EE60_C5688 * 0.5f;
                        float b = D_0015EE60_C5688 * 0.7f;
                        wv[0] = 0.0f; wv[1] = 0.0f; wv[2] = 0.0f; wv[3] = 0.0f;
                        wv[0] = a;
                        wv[2] = b;
                        wv[1] = a;
                        qcopy(v, wv);
                        func_L00_00260958_C5688(a, v);
                        func_L00_0025F4A8_C5688(moby, v, 0, 0, 0, 0, 2, 1,
                                                0.0f, 0.0f, 4.0f, 3.0f, 9.0f, 1.0f, 12.0f,
                                                1, -1, active);
                        if (active != 0) {
                            func_L00_0025A890_C5688(effect + effect, wv, moby, 0x810000);
                            *(unsigned char *)((char *)xv + 8) = 2;
                            *(unsigned char *)((char *)xv + 9) = 1;
                            *(unsigned short *)((char *)xv + 10) = *(unsigned short *)(moby + 0xA6);
                            func_L00_001F2BE8_C5688(effect * 3.0f, moby + 0x10, 0x10, moby, wv);
                        }
                        func_L00_00260878_C5688(moby, D_L00_001B0BB0_C5688);
                        func_0020D678_C5688(moby);
                        return;
                    }
                }
            }
        }
        *(unsigned char *)(moby + 0xA4) = 0xFF;
        func_L00_0025E590_C5688(moby, vars);
        if ((*(unsigned char *)(moby + 0x70) & 2) != 0) {
            if (*(unsigned char *)(moby + 0x53) != 2)
                func_00213DE0_C5688(moby, 2, 0, func_001F9850_C5688(10));
            *(float *)(moby + 0x58) = func_002140F8_C5688(0.7f, 1.05f);
        }
        if (*(unsigned char *)(moby + 0x20) != 4) {
            len = func_00214358_C5688(moby + 0x10, 0, 0.5f);
            if (len == 0.0f) return;
            func_L00_002C5630_C5688(moby);
            if (len + 0.02f < *(float *)(moby + 0x18)) {
                *(float *)(vars + 0x18) = *(float *)(vars + 0x18) - D_0015EE70_C5688 * 9.8f;
                func_001F9BD8_C5688(moby + 0x10, moby + 0x10, vars + 0x10);
                len = func_00214358_C5688(moby + 0x10, 0, 0.5f);
                if (*(float *)(moby + 0x18) < len) {
                    *(float *)(moby + 0x18) = len;
                    v[0] = 0.0f; v[1] = 0.0f; v[2] = 0.0f; v[3] = 0.0f;
                    qcopy(vars + 0x10, v);
                    qcopy(wv, hit_info + 0x40);
                    cos_a = func_001F9F90_C5688(*(float *)(moby + 0x48));
                    sin_a = func_001F9FA8_C5688(*(float *)(moby + 0x48));
                    cv[0] = wv[0] * cos_a + wv[1] * sin_a;
                    cos_a = func_001F9F90_C5688(*(float *)(moby + 0x48));
                    sin_a = func_001F9FA8_C5688(*(float *)(moby + 0x48));
                    cv[1] = wv[1] * cos_a - wv[0] * sin_a;
                    cv[2] = wv[2];
                    *(float *)(vars + 0x4C) = -func_L00_001FF860_C5688(func_001F9B50_C5688(cv[0] * cv[0] + cv[2] * cv[2]), cv[1]);
                    *(float *)(vars + 0x50) = func_L00_001FF860_C5688(cv[2], cv[0]);
                }
            } else {
                v[0] = 0.0f; v[1] = 0.0f; v[2] = 0.0f; v[3] = 0.0f;
                qcopy(vars + 0x10, v);
            }
            if (*(int *)(hit_info + 0x18) != 0)
                func_L00_00261478_C5688(moby, *(char **)(hit_info + 0x18), moby + 0x10, moby + 0x40, moby + 0x10, moby + 0x40);
        } else {
            if (*(float *)(moby + 0x2C) < *(float *)(*(char **)(moby + 0x24) + 0x24))
                *(float *)(moby + 0x2C) = *(float *)(moby + 0x2C) + D_0015EE60_C5688 * 0.02f;
            if (func_001F9938_C5688(vars + 0x44) != 0) {
                if (*(unsigned char *)(moby + 0x53) != 0)
                    func_00213DE0_C5688(moby, 0, 0, func_001F9850_C5688(60));
                func_L00_00260878_C5688(moby, D_L00_001B0BB0_C5688);
                *(unsigned char *)(moby + 0x20) = 5;
            }
        }
        break;
    case 5:
        len = func_00214358_C5688(moby + 0x10, 0, 0.5f);
        if (len != 0.0f && *(int *)(hit_info + 0x18) != 0)
            func_L00_00261478_C5688(moby, *(char **)(hit_info + 0x18), moby + 0x10, moby + 0x40, moby + 0x10, moby + 0x40);
        if ((*(unsigned char *)(moby + 0x70) & 2) != 0)
            *(int *)(moby + 0x58) = 0;
        if (func_001F9938_C5688(vars + 0x44) != 0 || (*(unsigned char *)(moby + 0x70) & 2) != 0)
            *(unsigned char *)(moby + 0x20) = 6;
        break;
    case 6:
        len = func_00214358_C5688(moby + 0x10, 0, 0.5f);
        rate = D_0015EE60_C5688;
        if (len != 0.0f && *(int *)(hit_info + 0x18) != 0) {
            func_L00_00261478_C5688(moby, *(char **)(hit_info + 0x18), moby + 0x10, moby + 0x40, moby + 0x10, moby + 0x40);
            rate = D_0015EE60_C5688;
        }
        *(float *)(moby + 0x2C) = *(float *)(moby + 0x2C) - *(float *)(moby + 0x2C) * (rate * 0.02f);
        if (!(*(float *)(moby + 0x2C) < *(float *)(*(char **)(moby + 0x24) + 0x24) * 0.1f))
            return;
        func_0020D678_C5688(moby);
        break;
    default:
        *(unsigned char *)(moby + 0x20) = 0;
        break;
    }
}
