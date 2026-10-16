/* Freeze screen draw (mode 4): the dialog for the record at D_00193400. Word 0
   is the kind; each kind lays out its gauge and its lines of text. Text is
   staged in the scratchpad at 0x70000000 as lines split by bytes 0 and 1. */

extern char D_00193400[];
extern unsigned char D_001E7BF8[];
extern unsigned char D_001E7C10[];
extern char D_0013E600[];
extern char D_0013F450[];
extern char D_0015F648[];
extern char D_0015F650[];
extern char D_0015F660[];
extern char D_0015F668[];
extern char D_0015F670[];
extern char D_0015F680[];
extern int D_0013D440;
extern int D_0015EF58[];
extern int D_0015EF68[];
extern int D_0015EE80;
extern int D_0015EE84;
extern int D_0015EFB0;
extern int D_0015F538;
extern int D_0015F5E8;
extern int D_0015F5EC;
extern int D_0015F5F0;
extern int D_0015F5F4;
extern int D_0015F5F8;
extern int D_0015F5FC;
extern int D_0015F600;
extern int D_0015F604;
extern int D_0015F608;
extern int D_0015F60C;
extern int D_0015F610;
extern int D_0015F614;
extern int D_0015F618;
extern int D_0015F61C;
extern int D_0015F620;
extern int D_0015F624;
extern int D_0015F628;
extern int D_0015F62C;
extern int D_0015F630;
extern int D_0015F634;
extern int D_0015F638;
extern int D_0015F63C;
extern int D_0015F640;
extern int D_0015F6C8;
extern int D_0015F6CC;
extern int D_001D60C0;

extern void func_001F55C0(int, int, int, int);
extern void func_001F4630(int);
extern void func_001F4748(void);
extern void func_001F62C8(int, int, int, int, int);
extern int func_001FA8A8(int, int, float);
extern int func_001FA898(float);
extern float func_001FA888(int);
extern float func_001F9FA8(float);
extern int func_001FE540(int);
extern void *func_00116B00(void *, const void *, int);
extern void func_001153FC(void *, int, int);
extern void func_001F7560(void *, int, void *, int);
extern void func_00234C98(int, long);
extern int func_00200198(int, int);
extern void func_00200468(int, int, int, int, int, int);
extern int func_00200248(int);
extern void func_00200E38(int, int, int, float, float, float, float, float);
extern int func_001F6EA8(int, int, int, int, int);
extern void func_001F6410(int, int, int, int, int);
extern int func_001F98C0(int);
extern long func_001F4868(int); /* GetEffectTex: the 64-bit TEX0 of an effect texture */
extern void func_001F7070(void *, void *, void *, void *, long, unsigned char *);
extern void fmt1(void *, const char *, int) __asm__("func_00116248");
extern void fmt2(void *, const char *, int, int) __asm__("func_00116248");
extern void fmt3(void *, const char *, int, int, int) __asm__("func_00116248");
extern void fmt5(void *, const char *, int, int, int, int) __asm__("func_00116248");

#define FRZ(off) (*(int *)(D_00193400 + (off)))
#define GP(a) (*(int *)(a))
#define SCRATCH ((unsigned char *)0x70000000)

void func_001FBE80(void) {
    unsigned char frame[0x40];
    unsigned char txt[0x200];
    unsigned char tmp[0x18];
    unsigned char *buf = frame;
    unsigned char *scr = SCRATCH;
    unsigned char *p;
    unsigned char *tx;
    int i, t, t2, c1, c2, c3, s, s1, s2, h, x, sel, idx, V, T, q1, rem1, q2, rem2, frac;
    int kind, v2, v16, v17, v18, v19, v20, v21, v22, v30, v23, arg, code, y, r1, r2, r3, r4;
    int dummy;
    float f0, f1, f2, f12, f20, f21, f22;

    func_001F55C0(0, 0, 0, 0x30);
    if (FRZ(0x2C) != 0) {
        return;
    }
    func_001F4630(0);

    kind = FRZ(0);
    if (kind == 0) {
        goto kind0;
    } else if (kind == 2) {
        goto kind2;
    } else if (kind == 3) {
        goto kind3;
    } else if (kind == 5) {
        goto kind5;
    } else if (kind == 6) {
        goto kind6;
    } else if (kind == 7) {
        goto kind7;
    } else if (kind == 8) {
        goto kind8;
    }
    goto kind14;

    /* ---- kind 5: a gauge and one line, from the timer ---- */
kind5:
    t = func_001F98C0(0x1E);
    f2 = (float)t;
    f0 = (float)FRZ(4);
    f0 = f0 / f2;
    f20 = 1.0f;
    f20 = f20 - f0;
    f1 = 80.0f;
    f1 = f20 * f1;
    func_001F62C8(0x50, 0x154, 0x60, 0x1A0, (int)f1);
    for (i = 0; i < 24; i++) {
        buf[i] = D_001E7BF8[i];
    }
    c1 = func_001FA8A8(GP(&D_0015F5F0), GP(&D_0015F5F4), f20);
    s = func_001FE540(0x4E2B);
    func_00116B00(scr, (void *)s, 0x400);

    p = scr;
    if (*p >= 2) {
        p = scr + 1;
        while (*p >= 2) {
            p++;
        }
    }
    if (*p == 1) {
        *p = 0;
        for (;;) {
            p++;
            if (*p != 1) {
                break;
            }
            *p = 0;
        }
    }
    v19 = c1;
    func_001F7560(buf, v19, scr, -1);
    f22 = 272.0f;
    x = *(short *)(buf + 0x0A) + *(short *)(buf + 0x0E);
    *(unsigned short *)(buf + 0x12) |= 4;
    func_001F7560(buf, v19, p, -1);
    *(unsigned short *)(buf + 0x12) ^= 4;
    *(short *)(buf + 0x0A) = (short)(0x136 - *(unsigned short *)(buf + 0x0E));
    func_001F7560(buf, v19, p, -1);
    t = func_001F98C0(0x1E);
    f1 = (float)t;
    f12 = (float)FRZ(0x24);
    f0 = 1.0f;
    f12 = f12 / f1;
    f12 = f0 - f12;
    c2 = func_001FA8A8(0x0020FFFF, 0x8020FFFF, f12);
    s = func_001FE540(0x524F);
    func_001F6EA8(0x100, 0x140, c2, s, -1);
    x = (x + *(short *)(buf + 0x0A)) >> 1;
    func_00234C98(0x47, 0x3004B);
    v16 = x - 0x20;
    h = func_00200198(0x755D, 0);
    func_00200468(h, 0xE0, v16, 0x40, 0x40, 0x80);
    v2 = D_0015F538;
    x = x << 4;
    f1 = -6.2831855f;
    f0 = 55.0f;
    f21 = (float)x;
    f20 = (float)(v2 % 0x37);
    f20 = f20 * f1;
    f20 = f20 / f0;
    h = func_00200198(0x755D, 1);
    h = func_00200248(h);
    func_00200E38(0x40, 0x40, h, 4096.0f, f21, f22, f22, f20);
    goto done;

    /* ---- kinds 1, 4 and the rest: a gauge and up to three lines ---- */
kind14:
    f0 = func_001FA888(D_0015F620);
    {
        int m = D_0015F538 % D_0015F620;
        f12 = (float)m;
        f12 = f12 / f0;
        f2 = 6.28318f;
        f12 = f12 * f2;
        f1 = 3.14159f;
        f12 = f12 - f1;
        f0 = func_001F9FA8(f12);
        f12 = 0.5f;
        f0 = f0 * f12;
        f12 = f0 + f12;
        v20 = func_001FA8A8(GP(&D_0015F624), GP(&D_0015F628), f12);
    }
    func_001F6410(0x50, GP(&D_0015F5EC) + 0x1A, 0xB0, 0x150, v20);
    if (FRZ(8) != 0) {
        func_001F6EA8(0x100, 0x5A, (int)0x8000C0C0, FRZ(8), -1);
    }
    if (FRZ(0xC) != 0) {
        func_001F6EA8(0x100, GP(&D_0015F5E8), (int)0x80FFA888, FRZ(0xC), -1);
    }
    if (FRZ(0x10) != 0) {
        func_001F6EA8(0x100, GP(&D_0015F5EC), (int)0x80FFA888, FRZ(0x10), -1);
    }
    goto done;

    /* ---- kind 2: a gauge and one line ---- */
kind2:
    f0 = func_001FA888(D_0015F620);
    {
        int m = D_0015F538 % D_0015F620;
        f12 = (float)m;
        f12 = f12 / f0;
        f2 = 6.28318f;
        f12 = f12 * f2;
        f1 = 3.14159f;
        f12 = f12 - f1;
        f0 = func_001F9FA8(f12);
        f12 = 0.5f;
        f0 = f0 * f12;
        f12 = f0 + f12;
        v20 = func_001FA8A8(GP(&D_0015F624), GP(&D_0015F628), f12);
    }
    func_001F6410(0x64, 0xA0, 0xB0, 0x150, v20);
    func_001F6EA8(0x100, 0x7A, (int)0x8000C0C0, FRZ(8), -1);
    goto done;

    /* ---- kind 0: the timed countdown, two lines of text ---- */
kind0:
    f0 = func_001FA888(D_0015F620);
    {
        int m = D_0015F538 % D_0015F620;
        f12 = (float)m;
        f12 = f12 / f0;
        f2 = 6.28318f;
        f12 = f12 * f2;
        f1 = 3.14159f;
        f12 = f12 - f1;
        f0 = func_001F9FA8(f12);
        f12 = 0.5f;
        f0 = f0 * f12;
        f12 = f0 + f12;
        v20 = func_001FA8A8(GP(&D_0015F624), GP(&D_0015F628), f12);
    }
    f1 = 0.125f;
    f2 = 1.0f;
    f0 = (float)FRZ(0x20);
    f21 = f0 * f1;
    if (f2 < f21) {
        f21 = f2;
    } else if (f21 < 0.1f) {
        f21 = 0.1f;
    }
    v19 = GP(&D_0015F62C);
    v23 = GP(&D_0015F634);
    v30 = GP(&D_0015F63C);
    if (FRZ(0x24) != 0) {
        f1 = (float)FRZ(0x24);
        f0 = 0.125f;
        f2 = 1.0f;
        f20 = f1 * f0;
        if (f2 < f20) {
            f20 = f2;
        } else if (f20 < 0.0f) {
            f20 = 0.0f;
        }
        r1 = func_001FA8A8(GP(&D_0015F62C), GP(&D_0015F630), f20);
        v19 = r1;
        r2 = func_001FA8A8(GP(&D_0015F634), GP(&D_0015F638), f20);
        v23 = r2;
        r3 = func_001FA8A8(GP(&D_0015F63C), GP(&D_0015F640), f20);
        v30 = r3;
    }
    if (*(short *)(D_0013F450 + 0x89A) < 3) {
        /* branch A: the four corners of the gauge, from the centre */
        f12 = (float)GP(&D_0015F604);
        f12 = f12 * f21;
        r1 = func_001FA898(f12);
        v18 = GP(&D_0015F5FC) - r1;
        f12 = (float)GP(&D_0015F604);
        f12 = f12 * f21;
        r2 = func_001FA898(f12);
        v17 = GP(&D_0015F5FC) + r2;
        f12 = (float)GP(&D_0015F600);
        f12 = f12 * f21;
        r3 = func_001FA898(f12);
        v16 = GP(&D_0015F5F8) - r3;
        f12 = (float)GP(&D_0015F600);
        f12 = f12 * f21;
        r4 = func_001FA898(f12);
        func_001F6410(v18, v17, v16, GP(&D_0015F5F8) + r4, v20);
        s = func_001FE540(0x4F6E);
        func_001F6EA8(0x100, GP(&D_0015F608), v19, s, -1);
        s = func_001FE540(0x524E);
        func_001F6EA8(0x100, GP(&D_0015F608) + 0x18, v19, s, -1);
        s = func_001FE540(0x524D);
        func_001F6EA8(0x100, GP(&D_0015F608) + 0x30, v19, s, -1);
        goto done;
    }
    /* branch B: the same four corners from the other globals, then the
       countdown lines */
    f12 = (float)GP(&D_0015F618);
    f12 = f12 * f21;
    r1 = func_001FA898(f12);
    v18 = GP(&D_0015F610) - r1;
    f12 = (float)GP(&D_0015F618);
    f12 = f12 * f21;
    r2 = func_001FA898(f12);
    v17 = GP(&D_0015F610) + r2;
    f12 = (float)GP(&D_0015F614);
    f12 = f12 * f21;
    r3 = func_001FA898(f12);
    v16 = GP(&D_0015F60C) - r3;
    f12 = (float)GP(&D_0015F614);
    f12 = f12 * f21;
    r4 = func_001FA898(f12);
    func_001F6410(v18, v17, v16, GP(&D_0015F60C) + r4, v20);

    x = *(int *)(D_0013F450 + 0x8C0);
    sel = x - 1;
    if (!(sel < 5)) {
        sel = 4;
    }
    s = func_001FE540(sel + 0x5229);
    s2 = func_001FE540(0x5245);
    fmt2(buf, D_0015F660, s, s2);
    x = *(int *)(D_0013F450 + 0x8C0);
    if (x == 1) {
        func_001F6EA8(0x100, GP(&D_0015F61C), v23, (int)buf, -1);
    } else {
        func_001F6EA8(0x100, GP(&D_0015F61C), v30, (int)buf, -1);
    }
    v2 = D_0015EE84;
    idx = ((v2 ^ 0x10) != 0) ? 0 : 4;
    V = *(int *)((char *)D_0015EF58 + idx);
    if (V != *(int *)(D_0013F450 + 0x894)) {
        s = func_001FE540(0x50A2);
        fmt1(buf, D_0015F668, s);
        func_001F6EA8(0x100, GP(&D_0015F61C) + 0x30, v30, (int)buf, -1);
    } else {
        T = (D_0015EE80 != 0) ? 3000 : 3600;
        q1 = V / T;
        rem1 = V - q1 * T;
        q2 = rem1 / (T / 60);
        rem2 = rem1 - q2 * (T / 60);
        frac = (rem2 * 100) / (T / 60);
        s = func_001FE540(0x50A3);
        fmt1(buf, D_0015F668, s);
        func_001F6EA8(0x100, GP(&D_0015F61C) + 0x26, v23, (int)buf, -1);
        fmt3(buf, D_0015F670, q1, q2, frac);
        func_001F6EA8(0x100, GP(&D_0015F61C) + 0x3A, v23, (int)buf, -1);
    }

    v2 = D_0015EE84;
    idx = ((v2 ^ 0x10) != 0) ? 0 : 4;
    v2 = *(int *)((char *)D_0015EF68 + idx);
    if (v2 != 0 && v2 == *(int *)(D_0013F450 + 0x8A8)) {
        s = func_001FE540(0x50A5);
        fmt1(buf, D_0015F668, s);
        func_001F6EA8(0x100, GP(&D_0015F61C) + 0x56, v23, (int)buf, -1);
        fmt1(buf, D_0015F680, *(int *)(D_0013F450 + 0x8A8));
        func_001F6EA8(0x100, GP(&D_0015F61C) + 0x6A, v23, (int)buf, -1);
    } else {
        s = func_001FE540(0x50A4);
        fmt1(buf, D_0015F668, s);
        func_001F6EA8(0x100, GP(&D_0015F61C) + 0x60, v30, (int)buf, -1);
    }
    s = func_001FE540(0x524E);
    func_001F6EA8(0x100, GP(&D_0015F61C) + 0x90, v19, s, -1);
    s = func_001FE540(0x524D);
    func_001F6EA8(0x100, GP(&D_0015F61C) + 0xA8, v19, s, -1);
    goto done;

    /* ---- kind 3: a text line chosen by the state at D_0015EFB0 ---- */
kind3:
    if (D_0013D440 != 0) {
        goto done;
    }
    v18 = 1;
    if (FRZ(0x20) <= 0) {
        goto done;
    }
    v2 = D_0015EFB0;
    tx = D_0015F648;
    v22 = 0;
    v20 = 0;
    v21 = 0;
    idx = v2 - 2;
    if ((unsigned)idx >= 23) {
        goto common;
    }
    if (idx == 0) {
        arg = 0x4FA6;
        s = func_001FE540(arg);
        tx = (unsigned char *)s;
        v20 = (FRZ(4) != 0) ? 0 : 0x524F;
        goto common;
    }
    if (idx == 1 || idx == 2) {
        v2 = GP(&D_0015F6CC);
        if (GP(&D_0015F6C8) != 0) {
            if (GP(&D_001D60C0) != 0) {
                goto k3ac;
            }
            arg = 0x4FAB;
            v22 = 0x5253;
            goto k3b8;
        }
        v2 = GP(&D_0015F6CC);
    k3ac:
        if (v2 == 0) {
            goto k3c8;
        }
        arg = 0x4FAB;
        v22 = 0x5254;
        goto k3b8;
    }
    if (idx == 3) {
        arg = 0x4FA7;
        v20 = 0x4FA8;
        goto k3e0;
    }
    if (idx == 4) {
        arg = 0x4FAF;
        v22 = 0x5254;
        goto k3b8;
    }
    if (idx == 5 || idx == 6) {
        arg = 0x4FB7;
        goto k3e0;
    }
    if (idx == 7 || idx == 8 || idx == 9 || idx == 14) {
        goto done;
    }
    if (idx == 10) {
        arg = 0x4FA7;
        if (GP(&D_001D60C0) != 0) {
            v20 = 0x4FA8;
            goto k3e0;
        }
        arg = 0x4FAD;
        v22 = 0x5254;
        goto k3b8;
    }
    if (idx == 11) {
        arg = 0x4FAD;
        v22 = 0x5254;
        goto k3b8;
    }
    if (idx == 12 || idx == 13) {
        arg = 0x4FB8;
        goto k3e0;
    }
    if (idx == 15) {
        arg = 0x4FBA;
        v20 = 0x524F;
        goto k3e0;
    }
    if (idx == 16) {
        arg = 0x4FBC;
        v20 = 0x524F;
        goto k3e0;
    }
    if (idx == 17) {
        arg = 0x4FA7;
        if (GP(&D_001D60C0) != 0) {
            v20 = 0x4FA8;
            goto k3e0;
        }
        if (GP(&D_0015F6C8) == 0) {
            arg = 0x4FA9;
            v20 = 0x4FA8;
            goto k3e0;
        }
        arg = 0x4FA9;
        goto k3l730;
    }
    if (idx == 18) {
        arg = 0x4FBB;
        v20 = 0x524F;
        goto k3e0;
    }
    if (idx == 19) {
        arg = 0x4FBD;
        v20 = 0x524F;
        goto k3e0;
    }
    if (idx == 20) {
        goto common;
    }
    if (idx == 21) {
        arg = 0x4FB1;
        v22 = 0x5253;
        goto k3b8;
    }
    arg = 0x4FAE;
    goto k3l730;

k3c8:
    arg = (GP(&D_001D60C0) == 0) ? 0x4FAC : 0x4FA7;
    v20 = 0x4FA8;
    goto k3e0;

k3b8:
    tx = (unsigned char *)func_001FE540(arg);
    v21 = 0x5250;
    goto common;

k3e0:
    tx = (unsigned char *)func_001FE540(arg);
    goto common;

k3l730:
    /* the "%s%c%c%s" line of two strings */
    v22 = 0x5253;
    s1 = func_001FE540(arg);
    v21 = 0x5250;
    s2 = func_001FE540(0x4FAA);
    fmt5(txt, D_0015F650, s1, 1, 1, s2);
    tx = txt;
    goto common;

    /* ---- kind 8: the same gauge and text, then one line ---- */
kind8:
    if (FRZ(0x20) <= 0) {
        goto done;
    }
    s1 = func_001FE540((FRZ(0x30) == 0) ? 0x4FBC : 0x4FBD);
    tx = (unsigned char *)s1;
    v20 = (FRZ(4) != 0) ? 0 : 0x524F;
    v21 = 0;
    v22 = 0;
    goto common;

    /* ---- kind 6: a two-part line and a bar, drawn in FontPrintWindow ---- */
kind6:
    for (i = 0; i < 24; i++) {
        buf[i] = D_001E7C10[i];
    }
    t = func_001F98C0(0x1E);
    f2 = (float)t;
    f20 = 1.0f;
    f0 = (float)FRZ(4);
    f1 = 80.0f;
    f0 = f0 / f2;
    f20 = f20 - f0;
    f1 = f20 * f1;
    func_001F62C8(0x64, 0x12C, 0x60, 0x1A0, (int)f1);
    c1 = func_001FA8A8(GP(&D_0015F5F0), GP(&D_0015F5F4), f20);
    c2 = func_001FA8A8(0x0020FFFF, 0x8020FFFF, f20);
    code = FRZ(0x1C);
    if (code == 2) {
        goto k6a;
    }
    if (code < 3) {
        if (code < 0) {
            code = 0;
            goto k6end;
        }
        goto k6a;
    }
    if (code == 3) {
        s = func_001FE540(0x524F);
        func_001F6EA8(0x100, 0x118, c2, s, -1);
        code = 0x5231;
        goto k6end;
    }
    code = 0;
    goto k6end;

k6a:
    s = func_001FE540(0x5253);
    func_001F6EA8(0xCA, 0x118, c2, s, -1);
    s = func_001FE540(0x5250);
    func_001F6EA8(0x135, 0x118, c2, s, -1);
    code = 0x5230;

k6end:
    if (code == 0) {
        goto done;
    }
    s = func_001FE540(code);
    func_001F7070(buf, (void *)c1, (void *)s, (void *)-1, func_001F4868(1), D_001DF3D0);
    goto done;

    /* ---- kind 7: a gauge and one line, from the countdown ---- */
kind7:
    if (FRZ(0x20) <= 0) {
        goto done;
    }
    s = func_001FE540(0x4FA6);
    func_00116B00(scr, (void *)s, 0x400);
    p = scr;
    if (*p >= 2) {
        p = scr + 1;
        while (*p >= 2) {
            p++;
        }
    }
    if (*p == 1) {
        *p = 0;
        for (;;) {
            p++;
            if (*p != 1) {
                break;
            }
            *p = 0;
        }
    }
    tx = p;
    v20 = (FRZ(4) != 0) ? 0 : 0x524F;
    v21 = 0;
    v22 = 0;
    goto common;

    /* ---- the shared text block: the gauge, then up to three lines ---- */
common:
    func_001153FC(tmp, 0, 0x18);
    f21 = 1.0f;
    *(unsigned short *)(tmp + 2) = (unsigned short)*(unsigned short *)(D_0013E600 + 4);
    *(unsigned short *)(tmp + 4) = 0x60;
    *(unsigned short *)(tmp + 6) = 0x1A0;
    *(unsigned short *)(tmp + 8) = 0x100;
    *(unsigned short *)(tmp + 0xA) = 0x68;
    *(unsigned short *)(tmp + 0x10) = 0x10;
    *(unsigned short *)(tmp + 0x12) = 5;
    for (i = 0; i < 24; i++) {
        buf[i] = tmp[i];
    }
    func_001F7560(buf, 0, tx, -1);
    x = *(short *)(buf + 0x0E) + 0x28;
    s = (*(int *)(D_0013E600 + 4) - x) >> 1;
    *(short *)(buf + 0x0A) = (short)(s + 4);
    *(short *)(buf + 0x02) = (short)(s + x);
    *(short *)(buf + 0x00) = (short)s;
    t = func_001F98C0(0x1E);
    f1 = (float)t;
    f0 = 80.0f;
    f20 = (float)FRZ(4);
    f20 = f20 / f1;
    f20 = f21 - f20;
    f0 = f20 * f0;
    func_001F62C8(*(short *)(buf + 0x00), *(short *)(buf + 0x02), 0x60, 0x1A0, (int)f0);
    *(unsigned short *)(buf + 0x12) ^= 4;
    c2 = func_001FA8A8(GP(&D_0015F5F0), GP(&D_0015F5F4), f20);
    func_001F7560(buf, c2, tx, -1);
    t2 = func_001F98C0(0x1E);
    f12 = (float)FRZ(0x24);
    f0 = (float)t2;
    f12 = f12 / f0;
    f12 = f21 - f12;
    c3 = func_001FA8A8(0x0020FFFF, 0x8020FFFF, f12);
    if (v22 != 0) {
        y = *(short *)(buf + 0x02) - 0x14;
        s = func_001FE540(v22);
        func_001F6EA8(0xCA, y, c3, s, -1);
    }
    if (v21 != 0) {
        y = *(short *)(buf + 0x02) - 0x14;
        s = func_001FE540(v21);
        func_001F6EA8(0x135, y, c3, s, -1);
    }
    if (v20 != 0) {
        y = *(short *)(buf + 0x02) - 0x14;
        s = func_001FE540(v20);
        func_001F6EA8(0x100, y, c3, s, -1);
    }
    goto done;

done:
    func_001F4748();
}
