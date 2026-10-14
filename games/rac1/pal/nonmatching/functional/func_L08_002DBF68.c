/* functional: medium. Per-tick state machine (states in moby+0x20) of a Batalia enemy: approaches the target, attacks, throws a projectile at state 5, is killed in states 0x5D-0x63 and flees in 0x64. Not run against retail; the called helpers are declared with the argument types the matched code uses. */
extern void f264B40_DBF68(float, void *, int, void *) __asm__("func_L00_00264B40");
extern float f1F9D10_DBF68(void *, void *) __asm__("func_001F9D10");
extern void f25B178_DBF68(void *) __asm__("func_L00_0025B178");
extern float f1FA888_DBF68(int) __asm__("func_001FA888");
extern int f1FA898_DBF68(float) __asm__("func_001FA898");
extern void f2C5A50_DBF68(void *) __asm__("func_L04_002C5A50");
extern char *f25B478_DBF68(void *, int, int) __asm__("func_L00_0025B478");
extern int f25B4D0_DBF68(void *, void *, void *, int, int *, float *, int, int) __asm__("func_L00_0025B4D0");
extern float f1FF860_DBF68(float, float) __asm__("func_L00_001FF860");
extern void f25D5B0_DBF68(float, void *, void *, int, int, int) __asm__("func_L00_0025D5B0");
extern void f2584A8_DBF68(void *, int, int) __asm__("func_L00_002584A8");
extern void f25E4B0_DBF68(void *, void *) __asm__("func_L00_0025E4B0");
extern int f260D30_DBF68(void *, void *, float) __asm__("func_L00_00260D30");
extern void f26E8E0_DBF68(char *) __asm__("func_L01_0026E8E0");
extern void f1F9BC0_DBF68(void *) __asm__("func_001F9BC0");
extern int f1F9850_DBF68(int) __asm__("func_001F9850");
extern void f213DE0_DBF68(void *, int, int, int) __asm__("func_00213DE0");
extern void f25A060_DBF68(void *, void *, void *, void *) __asm__("func_L00_0025A060");
extern float f1F9D48_DBF68(void *, void *) __asm__("func_001F9D48");
extern float f1FA850_DBF68(float, float) __asm__("func_001FA850");
extern int f1F9908_DBF68(void *) __asm__("func_001F9908");
extern void f2592B0_DBF68(void *, void *, float, float, float, float) __asm__("func_L00_002592B0");
extern float f1F9F90_DBF68(float) __asm__("func_001F9F90");
extern float f1F9FA8_DBF68(float) __asm__("func_001F9FA8");
extern void f1F9BD8_DBF68(void *, void *, void *) __asm__("func_001F9BD8");
extern float f20D830_DBF68(void *) __asm__("func_0020D830");
extern void f1F9BF0_DBF68(void *, void *, void *) __asm__("func_001F9BF0");
extern float f1F9CB8_DBF68(void *) __asm__("func_001F9CB8");
extern float f1F9878_DBF68(float) __asm__("func_001F9878");
extern void f2607A8_DBF68(void *, float) __asm__("func_L00_002607A8");
extern void f250800_DBF68(void *, int, void *) __asm__("func_L00_00250800");
extern void f1FF500_DBF68(void *, void *, float) __asm__("func_L00_001FF500");
extern float f25BC48_DBF68(void *, void *, void *, float, float) __asm__("func_L00_0025BC48");
extern int f25D6F0_DBF68(void *, void *) __asm__("func_L00_0025D6F0");
extern void f20D678_DBF68(void *) __asm__("func_0020D678");
extern void f25E590_DBF68(void *, void *) __asm__("func_L00_0025E590");
extern int D_L08_00161A3C_DBF68 SDATA(D_L08_00161A3C);
extern int D_L08_00161A60_DBF68 SDATA(D_L08_00161A60);
extern float D_L08_00161A1C_DBF68 SDATA(D_L08_00161A1C);
extern float D_L08_00161A20_DBF68 SDATA(D_L08_00161A20);
extern float D_L08_00161A08_DBF68 SDATA(D_L08_00161A08);
extern float D_L08_00161A0C_DBF68 SDATA(D_L08_00161A0C);
extern float D_L08_00161A18_DBF68 SDATA(D_L08_00161A18);
extern float D_0015EE6C_DBF68 __asm__("D_0015EE6C") MACRO_ADDR;
extern float D_0015EE70_DBF68 __asm__("D_0015EE70") MACRO_ADDR;
extern char D_0013F450_DBF68[] __asm__("D_0013F450");
extern char *D_001414D0_DBF68 __asm__("D_001414D0");
extern char D_L08_00167640_DBF68[] __asm__("D_L08_00167640");
extern char D_L08_00167500_DBF68[] __asm__("D_L08_00167500");

void func_L08_002DBF68(char *m)
{
    char *d = *(char **)(m + 0x78);
    char buf[0x48];
    float v50[4], v60[4], v70[4], o88[4];
    int loc80;
    float loc84;
    char *tgt, *old, *nw, *r;
    float f, dist, a, b;
    int st, t;

    loc84 = 0.0f;
    f264B40_DBF68(3.0f, m, D_L08_00161A3C_DBF68, d + 0x160);
    if (*(unsigned char *)(m + 0x31) != 0) {
        if (f1F9D10_DBF68(m + 0x10, D_L08_00167640_DBF68) < 29.0f) {
            f25B178_DBF68(m);
            *(unsigned char *)(m + 0x7F) = 0x17;
        }
    }
    if (*(unsigned char *)(m + 0x20) == 3)
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xFBFF;
    else
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x400;

    if (*(unsigned char *)(m + 0x20) == 0x64) {
        /* fleeing: turn the heading toward the stored one over time */
        f = f1FA888_DBF68(D_L08_00161A60_DBF68);
        a = (float)(D_L08_00161A60_DBF68 - *(short *)(d + 0x130));
        b = (float)(-*(short *)(d + 0x132));
        a = a / f;
        b = b * a;
        b = (float)*(short *)(d + 0x132) + b;
        *(unsigned char *)(m + 0x23) = (unsigned char)f1FA898_DBF68(b);
        f2C5A50_DBF68(m);
        return;
    }

    r = f25B478_DBF68(m, 0x330000, 0);
    f25B4D0_DBF68(m, r, d + 0x20, 0, &loc80, &loc84, 0, 4);
    if (loc80 != 1 && *(unsigned char *)(m + 0x20) != 0x63) {
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x1000;
        f = *(float *)(d + 0x20) - loc84;
        *(float *)(d + 0x20) = f;
        if (0.0f < f) {
            if (loc84 < 1.0f) {
                *(unsigned char *)(d + 0x67) = 0x64;
            } else {
                *(int *)(d + 0x94) = 9;
                *(float *)(d + 0x88) = D_0015EE6C_DBF68 * 5.0f;
                *(float *)(d + 0x8C) = D_0015EE6C_DBF68 * 7.0f;
                *(float *)(d + 0x80) = D_0015EE70_DBF68 * 30.0f;
                *(float *)(d + 0x84) = 0.0005f;
                *(unsigned char *)(d + 0xAD) = 0;
                a = f1FF860_DBF68(*(float *)(m + 0x10) - *(float *)(D_0013F450_DBF68 + 0x80),
                                  *(float *)(m + 0x14) - *(float *)(D_0013F450_DBF68 + 0x84));
                f25D5B0_DBF68(a, m, d + 0x70, 6, 1, 0);
                *(float *)(d + 0xC0) = 6.0f;
                *(float *)(d + 0xC4) = 10.0f;
                *(unsigned char *)(d + 0x67) = 0x78;
                *(unsigned char *)(m + 0x20) = 0x5A;
            }
        } else {
            *(int *)(d + 0x94) = 9;
            *(float *)(d + 0x84) = 0.0005f;
            *(float *)(d + 0x88) = D_L08_00161A1C_DBF68 * D_0015EE6C_DBF68;
            *(float *)(d + 0x80) = D_0015EE70_DBF68 * 30.0f;
            *(float *)(d + 0x8C) = D_L08_00161A20_DBF68 * D_0015EE6C_DBF68;
            *(unsigned char *)(d + 0xAD) = 0;
            a = f1FF860_DBF68(*(float *)(m + 0x10) - *(float *)(D_0013F450_DBF68 + 0x80),
                              *(float *)(m + 0x14) - *(float *)(D_0013F450_DBF68 + 0x84));
            f25D5B0_DBF68(a, m, d + 0x70, 9, 1, 0);
            *(float *)(d + 0xC0) = 6.0f;
            *(float *)(d + 0xC4) = 10.0f;
            f2584A8_DBF68(m, 0, -1);
            *(unsigned char *)(d + 0x67) = 0xFA;
            *(unsigned char *)(m + 0x20) = 0x63;
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
            *(unsigned char *)(*(char **)(d + 0x120) + 0xBC) = 0;
        }
        f25E4B0_DBF68(m, d + 0x60);
    }

    *(unsigned char *)(m + 0xA4) = 0xFF;
    *(int *)(buf + 0x44) = f260D30_DBF68(m, buf, 16.0f);
    if (*(int *)(buf + 0x40) == 0)
        *(char **)(buf + 0x40) = D_001414D0_DBF68;
    st = *(unsigned char *)(m + 0x20);
    tgt = *(char **)(buf + 0x40);

    switch (st) {
    case 0:
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x1000;
        *(unsigned char *)(d + 0x29) = 1;
        *(float *)(d + 0x20) = 2.0f;
        f26E8E0_DBF68(d + 0xD0);
        *(float *)(d + 0xF4) = D_L08_00161A08_DBF68 * D_0015EE6C_DBF68;
        *(float *)(d + 0xD8) = 2.0f;
        *(float *)(d + 0xDC) = 2.0f;
        *(float *)(d + 0x10C) = 0.03f;
        *(float *)(d + 0x110) = 0.3f;
        *(float *)(d + 0x114) = 0.25f;
        *(unsigned char *)(d + 0x58) = (unsigned char)f1FA898_DBF68(17.0f);
        *(unsigned char *)(d + 0x5A) = (unsigned char)f1FA898_DBF68(10.4f);
        f1F9BC0_DBF68(d + 0x40);
        nw = func_L08_002DBD88(m);
        *(char **)(d + 0x120) = nw;
        nw[0xBC] = 1;
        *(int *)(d + 0x128) = f1F9850_DBF68(0xB4);
        if (*(unsigned char *)(m + 0x53) != 4)
            f213DE0_DBF68(m, 4, 0, 0);
        *(unsigned char *)(m + 0x20) = 1;
        goto tail1;
    case 1:
        f25A060_DBF68(m, d + 0xD0, *(char **)(d + 0x120) + 0x10, d + 0x40);
        dist = f1F9D10_DBF68(m + 0x10, *(char **)(d + 0x120) + 0x10);
        if (dist < 3.0f) {
            tgt = *(char **)(d + 0x120);
            a = f1FF860_DBF68(*(float *)(tgt + 0x10) - *(float *)(m + 0x10),
                              *(float *)(tgt + 0x14) - *(float *)(m + 0x14));
            b = f1FA850_DBF68(*(float *)(m + 0x48), a);
            if (b < 0.2617994f) {
                dist = f1F9D48_DBF68(m + 0x10, *(char **)(buf + 0x40) + 0x10);
                if (6.0f < dist) {
                    if (*(unsigned char *)(m + 0x53) != 5)
                        f213DE0_DBF68(m, 5, 0, 10);
                    *(unsigned char *)(m + 0x20) = 6;
                    goto done;
                }
                t = f1F9850_DBF68(0xF0);
                *(int *)(d + 0x134) = t;
                old = *(char **)(d + 0x120);
                nw = func_L08_002DBD88(m);
                *(char **)(d + 0x120) = nw;
                if (nw == 0)
                    *(char **)(d + 0x120) = old;
                old[0xBC] = 0;
                (*(char **)(d + 0x120))[0xBC] = 1;
                goto done;
            }
        }
        if (f1F9908_DBF68(d + 0x134) == 0)
            goto done;
        old = *(char **)(d + 0x120);
        nw = func_L08_002DBD88(m);
        *(char **)(d + 0x120) = nw;
        if (nw == 0)
            *(char **)(d + 0x120) = old;
        old[0xBC] = 0;
        (*(char **)(d + 0x120))[0xBC] = 1;
        *(int *)(d + 0x134) = f1F9850_DBF68(0xF0);
        goto done;
    case 6:
        f25A060_DBF68(m, d + 0xD0, *(char **)(d + 0x120) + 0x10, d + 0x40);
        if ((*(unsigned char *)(m + 0x70) & 2) == 0)
            goto done;
        t = f1F9850_DBF68(6);
        f213DE0_DBF68(*(char **)(d + 0x120), 1, 0, t);
        {
            char *src = *(char **)(d + 0x120) + 0x10;
            *(long *)(m + 0x10) = *(long *)(src + 0);
            *(long *)(m + 0x18) = *(long *)(src + 8);
        }
        *(float *)(m + 0x18) = *(float *)(m + 0x18) - 0.1f;
        if (*(unsigned char *)(m + 0x53) != 1)
            f213DE0_DBF68(m, 1, 0, 10);
        *(unsigned char *)(m + 0x20) = 3;
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
        *(float *)(m + 0x48) = f1FF860_DBF68(*(float *)(tgt + 0x10) - *(float *)(m + 0x10),
                                              *(float *)(tgt + 0x14) - *(float *)(m + 0x14));
        *(int *)(d + 0x128) = f1F9850_DBF68(0x78);
        *(int *)(m + 0x94) = 0;
        goto done;
    case 3:
        dist = f1F9D48_DBF68(m + 0x10, tgt + 0x10);
        a = f1FF860_DBF68(*(float *)(tgt + 0x10) - *(float *)(m + 0x10),
                          *(float *)(tgt + 0x14) - *(float *)(m + 0x14));
        f2592B0_DBF68(m, d + 0x12C, a, 0.02f, 0.3f, 0.2f);
        t = 0;
        if (dist < 20.0f && 8.0f < dist) {
            if (f1F9908_DBF68(d + 0x128) != 0) {
                a = f1FF860_DBF68(*(float *)(m + 0x10) - *(float *)(D_L08_00167500_DBF68 + 0x140),
                                  *(float *)(m + 0x14) - *(float *)(D_L08_00167500_DBF68 + 0x144));
                b = f1FA850_DBF68(*(float *)(D_L08_00167500_DBF68 + 0x158), a);
                if (b < 0.55850536f)
                    t = 1;
            }
        }
        if (t == 0) {
            if (*(int *)(d + 0x38) == 0)
                goto done;
            if (!(0.5f < dist))
                goto done;
        }
        t = f1F9850_DBF68(6);
        f213DE0_DBF68(*(char **)(d + 0x120), 1, 0, t);
        if (*(unsigned char *)(m + 0x53) != 2) {
            t = f1F9850_DBF68(7);
            f213DE0_DBF68(m, 2, 0, t);
        }
        *(unsigned char *)(m + 0x20) = 4;
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x1000;
        *(int *)(m + 0x94) = *(int *)(*(char **)(m + 0x24) + 0x10);
        goto done;
    case 4:
        f = f1F9F90_DBF68(*(float *)(m + 0x48));
        v50[0] = f + f;
        f = f1F9FA8_DBF68(*(float *)(m + 0x48));
        v50[1] = f + f;
        v50[2] = 0.0f;
        f1F9BD8_DBF68(v50, v50, *(char **)(d + 0x120) + 0x10);
        if (0.2f < f1F9D48_DBF68(m + 0x10, v50))
            f25A060_DBF68(m, d + 0xD0, v50, d + 0x40);
        if ((*(unsigned char *)(m + 0x70) & 2) == 0)
            goto done;
        dist = f1F9D48_DBF68(m + 0x10, tgt + 0x10);
        t = 0;
        if (!(dist < 6.0f)) {
            a = f1FF860_DBF68(*(float *)(m + 0x10) - *(float *)(D_L08_00167500_DBF68 + 0x140),
                              *(float *)(m + 0x14) - *(float *)(D_L08_00167500_DBF68 + 0x144));
            b = f1FA850_DBF68(*(float *)(D_L08_00167500_DBF68 + 0x158), a);
            if (!(0.55850536f < b)) {
                if (*(unsigned char *)(m + 0x53) != 3)
                    f213DE0_DBF68(m, 3, 0, f1F9850_DBF68(6));
                *(unsigned char *)(m + 0x20) = 5;
                goto done;
            }
        }
        old = *(char **)(d + 0x120);
        nw = func_L08_002DBD88(m);
        *(char **)(d + 0x120) = nw;
        if (nw == 0)
            *(char **)(d + 0x120) = old;
        old[0xBC] = 0;
        (*(char **)(d + 0x120))[0xBC] = 1;
        if (*(unsigned char *)(m + 0x53) == 4) {
            *(unsigned char *)(m + 0x20) = 1;
            goto tail1;
        }
        t = f1F9850_DBF68(7);
        f213DE0_DBF68(m, 4, 0, t);
        *(unsigned char *)(m + 0x20) = 1;
        goto tail1;
    case 5:
        a = f1FF860_DBF68(*(float *)(tgt + 0x10) - *(float *)(m + 0x10),
                          *(float *)(tgt + 0x14) - *(float *)(m + 0x14));
        f2592B0_DBF68(m, d + 0x12C, a, 0.02f, 0.3f, 0.2f);
        if (*(unsigned char *)(m + 0x52) == *(unsigned char *)(m + 0x53)) {
            if (f20D830_DBF68(m) == 19.0f) {
                float len, k;
                f1F9BF0_DBF68(v70, tgt + 0x10, m + 0x10);
                v70[2] = 0.0f;
                len = f1F9CB8_DBF68(v70);
                k = len / f1F9878_DBF68(D_L08_00161A0C_DBF68);
                f = len - (D_L08_00161A18_DBF68 * k + 2.0f);
                if (10.0f < f)
                    f = 10.0f;
                else if (f < 3.0f)
                    f = 3.0f;
                f2607A8_DBF68(v70, f);
                f1F9BD8_DBF68(v70, v70, m + 0x10);
                f250800_DBF68(m, 9, v50);
                f1F9BF0_DBF68(v60, tgt + 0x10, v50);
                f1FF500_DBF68(v60, v60, k);
                v60[2] = f25BC48_DBF68(v50, v70, o88, k, -(D_0015EE70_DBF68 * 15.0f));
                nw = (char *)func_L08_002D35E8((int)m, v50, v60);
                *(char **)(d + 0x124) = nw;
                if (nw == 0)
                    goto done;
                {
                    char *p = *(char **)(nw + 0x78);
                    *(int *)(p + 0x20) = f1F9850_DBF68(0xB4);
                    *(short *)(p + 0xC) = (short)f1F9850_DBF68(0xA);
                    *(short *)(p + 0xE) = (short)f1F9850_DBF68(0xA);
                }
                (*(char **)(d + 0x124))[0x20] = 1;
                nw = *(char **)(d + 0x124);
                nw[0xBC] = nw[0xBC] | 3;
                *(int *)(d + 0x128) = f1F9850_DBF68(0xB4);
                goto done;
            }
        }
        if ((*(unsigned char *)(m + 0x70) & 2) == 0)
            goto done;
        *(unsigned char *)(m + 0x20) = 1;
        if (*(unsigned char *)(m + 0x53) != 4)
            f213DE0_DBF68(m, 4, 0, 3);
        goto tail1;
    case 91:
        if ((*(unsigned char *)(m + 0x70) & 2) == 0)
            goto done;
        if (*(unsigned char *)(m + 0x53) != 7)
            f213DE0_DBF68(m, 7, 0, 3);
        *(unsigned char *)(m + 0x20) = 0x5D;
        goto done;
    case 90:
    case 92:
    case 93:
        if ((f25D6F0_DBF68(m, d + 0x70) & 0x40) != 0) {
            old = *(char **)(d + 0x120);
            nw = func_L08_002DBD88(m);
            *(char **)(d + 0x120) = nw;
            if (nw != 0) {
                old[0xBC] = 0;
                (*(char **)(d + 0x120))[0xBC] = 1;
            } else {
                *(char **)(d + 0x120) = old;
            }
            if (*(unsigned char *)(m + 0x53) != 4)
                f213DE0_DBF68(m, 4, 0, 10);
            *(unsigned char *)(m + 0x20) = 1;
            *(int *)(d + 0x134) = f1F9850_DBF68(0xF0);
        }
        if (*(float *)(m + 0x18) < 5.0f) {
            f20D678_DBF68(m);
            return;
        }
        goto done_c8;
    case 99:
        if ((f25D6F0_DBF68(m, d + 0x70) & 0x40) == 0)
            goto done;
        *(int *)(m + 0x58) = 0;
        *(unsigned char *)(m + 0x20) = 0x64;
        *(int *)(m + 0x94) = 0;
        *(short *)(d + 0x132) = 0x28;
        *(short *)(d + 0x130) = (short)(unsigned short)D_L08_00161A60_DBF68;
        f2C5A50_DBF68(m);
        f20D678_DBF68(m);
        return;
    default:
        goto done;
    }

tail1:
    *(int *)(d + 0x134) = f1F9850_DBF68(0xF0);
done:
    f25E590_DBF68(m, d + 0x60);
    *(int *)(d + 0x38) = 0;
    return;
done_c8:
    f25E590_DBF68(m, d + 0x60);
    *(int *)(d + 0x38) = 0;
}
