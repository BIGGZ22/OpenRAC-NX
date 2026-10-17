/* functional: medium. The debug menu update (nothing in the executable calls it, and its first 14 words are "addiu sp / nop" leftovers of stripped code, not a prologue; the real entry is the addiu sp,-0x150 after them): reads the pad (D_0013CA40+0x1A0/0x1A4) and moves a page/cursor (D_0018C418+0xC / +0x10) through six pages (menu state) and edits the debug values of the page under the cursor, then returns. Page 0 toggles bit flags in +8 (item 4 turns them into 0x10 or 0xF), page 1 has 11 items (the cursor mode, the item counts and the fog/LOD switch values, then a table of 16 flags at D_0014C150), page 2 toggles bits of +4, page 3 edits a counter per item with the ranges of D_001E02B0, page 4 edits the background/ambient values, and page 5 toggles two flags, appends an "occ_sample_deltas.dat" sample of the camera position to the level's folder and reads the samples back. The first branch (pressed 0x500) snapshots the camera into the menu state and returns. The 64-bit pad word at +0x1A0 is tested as two 32-bit halves; func_00120778 returns the double as a 64-bit value for the variadic call. Not run against retail (hand-checked against the assembly). */
extern void func_0022DD68(void);
extern void func_001FB448(int, int, int);
extern int func_00116248(char *str, const char *fmt, ...);
extern int func_00116810(const char *s);
extern int func_0011BF80(const char *path, int flags);
extern int func_0011C388(int fd, int off, int whence);
extern int func_0011C820(int fd, const void *buf, int n);
extern int func_0011C5C0(int fd, void *buf, int n);
extern int func_0011C208(int fd);
extern void func_001F9BF0(void *dst, void *a, void *b);
extern float func_001F9CE8(void *v);
extern float func_001FA058(float, float);
extern float func_001FA790(float, float);
extern float func_001FA888(int);
extern void func_001F3140(void);
extern void func_001F2930(void);
extern long func_00120778(float);

extern unsigned char D_0013CA40[];
extern unsigned char D_0018C418[];
extern unsigned char D_0018CE00[];
extern unsigned char D_00187040[];
extern unsigned char D_00187180[];
extern unsigned char D_0013F4D0[];
extern unsigned char D_00151880[];
extern unsigned char D_0014C150[];
extern unsigned char D_0013E620[];
extern unsigned char D_001864D0[];
extern unsigned char D_001E02B0[];
extern int D_0013D530[];
extern unsigned char D_0015FCC8[];
extern unsigned char D_0015F108[];
extern char D_0015F120[];
extern char D_0015F130[];
extern int D_0015EE84;
extern int D_0015EE98;
extern char D_001E7AC0[];
extern char D_001E7AD8[];
extern char D_001E7AF8[];
extern char D_001E7B10[];
extern char D_001E7B30[];
extern int D_001941E0;
extern int D_0015F720;
extern int D_0015F734;
extern int D_0015F6E8;
extern int D_001601BC;
extern int D_00160030;
extern float D_00160FC0;
extern float D_00161070;
extern float D_001604E4;
extern float D_0015F588;
extern float D_0015F58C;
extern float D_0015F590;
extern float D_0015F594;
extern unsigned char D_0015F584;
extern unsigned char D_0015F585;
extern unsigned char D_0015F586;

#define M D_0018C418
#define MI(o) (*(int *)(M + (o)))
#define PADP D_0013CA40
#define PRESSED (*(unsigned int *)(PADP + 0x1A4))
#define HELD_LO (*(unsigned int *)(PADP + 0x1A0))
/* (1 << 35 | 0x40) and (1 << 34 | 0x10) of the 64-bit pad word */
#define HELD_A ((HELD_LO & 0x40) || (PRESSED & 0x8))
#define HELD_B ((HELD_LO & 0x10) || (PRESSED & 0x4))
#define F4(p, o) (*(float *)((p) + (o)))

static void flag_toggle_1088(int off)
{
    if (PRESSED & 0x50)
        MI(off) = (MI(off) == 0);
}

/* Cycle a counter up on 0x40 and down on 0x10, wrapping at hi. */
static void cycle_1088(int off, int hi)
{
    if (PRESSED & 0x40) {
        MI(off) = MI(off) + 1;
        if (MI(off) >= hi)
            MI(off) = 0;
    }
    if (PRESSED & 0x10) {
        MI(off) = MI(off) - 1;
        if (MI(off) < 0)
            MI(off) = hi - 1;
    }
}

static void adjust_f_1088(float *v, float step, float top)
{
    if (HELD_A)
        *v = *v - step;
    if (HELD_B)
        *v = *v + step;
    if (top < *v)
        *v = top;
    else if (*v < 0.0f)
        *v = 0.0f;
}

static void adjust_byte_1088(unsigned char *v)
{
    unsigned char b = *v;
    if (b < 0xFF && HELD_A) {
        *v = b + 1;
        b = *v;
    }
    if (b != 0 && HELD_B)
        *v = b - 1;
}

void func_001F1088(void)
{
    int arr[6];
    char name[0x40];
    char msg[0x40];
    float v3[4];
    int i, s, fd, len, size, found, b;
    int *src = (int *)D_001E7AC0;

    func_0022DD68();

    if (PRESSED & 0x500) {
        D_0015F6E8 = MI(0xA4);
        func_001FB448(*(int *)(D_0018CE00 + 0x23C), *(int *)(D_0018CE00 + 0x240), *(int *)(D_0018CE00 + 0x244));
        M[0xF9] = 0x6E;
        M[0xF8] = 0x6E;
        MI(0x9C) = (int)F4(D_00187040, 0x148) >> 2;
        MI(0x94) = (int)F4(D_00187040, 0x140) >> 2;
        MI(0x98) = (int)F4(D_00187040, 0x144) >> 2;
        return;
    }

    for (i = 0; i < 6; i++)
        arr[i] = src[i];

    if (PRESSED & 0x2000) {
        s = MI(0xC) + 1;
        MI(0xC) = s;
        if (s == 3)
            MI(0xC) = 0;
        else if (s == 6)
            MI(0xC) = 3;
    } else if (PRESSED & 0x8000) {
        s = MI(0xC) - 1;
        MI(0xC) = s;
        if (s < 0)
            MI(0xC) = 2;
        else if (s == 2)
            MI(0xC) = 5;
    }

    if (PRESSED & 0x1000) {
        MI(0x10) = MI(0x10) - 1;
        if (MI(0x10) < 0) {
            s = MI(0xC);
            if (s >= 3)
                MI(0xC) = (s + 3) % 6;
            s = MI(0xC);
            MI(0x10) = arr[s] - 1;
            goto clamp;
        }
    }
    if (PRESSED & 0x4000) {
        s = MI(0xC);
        MI(0x10) = MI(0x10) + 1;
        if (MI(0x10) >= arr[s]) {
            if (s < 3)
                MI(0xC) = (s + 9) % 6;
            MI(0x10) = 0;
        }
    }
clamp:
    s = MI(0xC);
    if ((unsigned)s < 6 && MI(0x10) >= arr[s])
        MI(0x10) = arr[s] - 1;

    switch (MI(0xC)) {
    case 0:
        if (PRESSED & 0x40) {
            int bit = 1 << MI(0x10);
            MI(8) = MI(8) ^ bit;
            if (bit == 0x10) {
                if (MI(8) & 0x10)
                    MI(8) = bit;
                else
                    MI(8) = 0xF;
            }
        }
        return;

    case 1:
        switch (MI(0x10)) {
        case 0:
            if (!(PRESSED & 0x50))
                return;
            cycle_1088(0x14, 4);
            switch (MI(0x14)) {
            case 0:
                MI(8) = 0xF;
                return;
            case 1:
                MI(8) = 0;
                return;
            case 2:
                MI(8) = 0;
                func_001F9BF0(v3, D_0013F4D0, D_00187180);
                F4(M, 0x8C) = func_001F9CE8(v3);
                {
                    float r = func_001FA058(v3[0], v3[1]);
                    F4(M, 0x88) = func_001FA790(r, F4(D_00187040, 0x158));
                }
                F4(M, 0x90) = F4(D_0013F4D0, 0x8) - F4(D_00187040, 0x148);
                F4(M, 0x84) = func_001FA790(F4(D_0013F4D0, 0x18), F4(D_00187040, 0x158));
                return;
            case 3:
                MI(8) = 6;
                return;
            }
            return;
        case 1:
            cycle_1088(0x18, 8);
            return;
        case 2:
            if (D_0015F720 == 0) {
                MI(0x1C) = 0;
                return;
            }
            cycle_1088(0x1C, 3);
            return;
        case 3:
            flag_toggle_1088(0x20);
            return;
        case 4:
            flag_toggle_1088(0x24);
            return;
        case 5:
            if (PRESSED & 0x50) {
                int t = (MI(0x28) == 0);
                MI(0x28) = t;
                if (t) {
                    D_00160FC0 = 65536.0f;
                    D_00161070 = 64.0f;
                    D_001604E4 = 40.0f;
                    D_00160030 = 0x40;
                    D_001601BC = 0x40000;
                } else {
                    D_00160FC0 = 500000.0f;
                    D_00161070 = 720.0f;
                    D_001604E4 = 500.0f;
                    D_00160030 = 0x1F4;
                    D_001601BC = 0x1F4000;
                }
            }
            return;
        case 6:
            cycle_1088(0x2C, 3);
            return;
        case 7:
            flag_toggle_1088(0x30);
            return;
        case 8:
            if (PRESSED & 0x50) {
                float k;
                int t = (MI(0x34) == 0);
                MI(0x34) = t;
                k = t ? 0.125f : 0.5f;
                F4(D_0018CE00, 0x200) = func_001FA888(*(short *)(D_00151880 + 0x150)) * k;
                F4(D_0018CE00, 0x204) = func_001FA888(*(short *)(D_00151880 + 0x152)) * k;
                F4(D_0018CE00, 0x208) = F4(D_0018CE00, 0x200) * 4.0f;
                F4(D_0018CE00, 0x20C) = F4(D_0018CE00, 0x204) * 4.0f;
                func_001F3140();
            }
            return;
        case 9:
            if (PRESSED & 0x50) {
                if (PRESSED & 0x40) {
                    MI(0x38) = MI(0x38) + 1;
                    if (MI(0x38) == 0x24)
                        MI(0x38) = -1;
                } else {
                    MI(0x38) = MI(0x38) - 1;
                    if (MI(0x38) < -1)
                        MI(0x38) = 0x23;
                }
            }
            MI(0xA4) = (MI(0x38) < 0) ? 0 : 2;
            return;
        case 10:
            if (PRESSED & 0x50) {
                if (PRESSED & 0x40) {
                    MI(0x3C) = MI(0x3C) + 1;
                    if (MI(0x3C) == 0x10)
                        MI(0x3C) = 0;
                } else {
                    MI(0x3C) = MI(0x3C) - 1;
                    if (MI(0x3C) < 0)
                        MI(0x3C) = 0xF;
                }
            } else if (PRESSED & 0x20) {
                unsigned char *p = D_0014C150 + D_0015EE84 * 16 + MI(0x3C);
                unsigned char v = (*p == 0) ? 0xFF : 0;
                *p = v;
                D_0015FCC8[MI(0x3C)] = v;
            }
            return;
        }
        return;

    case 2:
        if (PRESSED & 0x40)
            MI(4) = MI(4) ^ (1 << MI(0x10));
        return;

    case 3: {
        int *ptr;
        int maxv;
        unsigned char *tbl = D_0015F108 + 8; /* 13 item ids at 0x15F110 */
        unsigned char *cnt;
        unsigned char *rec;

        if (MI(0x10) < 0xD) {
            b = tbl[MI(0x10)];
            maxv = *(unsigned short *)(D_001E02B0 + b * 0x18 + 0xE);
            ptr = &D_0013D530[b];
        } else {
            ptr = &D_0015EE98;
            maxv = 999999;
        }
        if (HELD_LO & 0x8) {
            if (PRESSED & 0x40)
                *ptr = *ptr + 0x3C;
            if (PRESSED & 0x20) {
                if (*ptr < 0x3D)
                    *ptr = 0;
                else
                    *ptr = *ptr - 0x3C;
            }
        } else if (HELD_LO & 0x2) {
            if (PRESSED & 0x40)
                *ptr = *ptr + 0x258;
            if (PRESSED & 0x20) {
                if (*ptr < 0x259)
                    *ptr = 0;
                else
                    *ptr = *ptr - 0x258;
            }
        } else {
            if (PRESSED & 0x40)
                *ptr = *ptr + 1;
            if (PRESSED & 0x20) {
                if (*ptr > 0)
                    *ptr = *ptr - 1;
            }
        }
        if (maxv < *ptr)
            *ptr = maxv;

        if (PRESSED & 0x10) {
            b = tbl[MI(0x10)];
            D_0013E620[b] = D_0013E620[b] + 1;
        }
        if (PRESSED & 0x80) {
            b = tbl[MI(0x10)];
            D_0013E620[b] = D_0013E620[b] - 1;
        }
        b = tbl[MI(0x10)];
        cnt = &D_0013E620[b];
        rec = D_001864D0 + b * 0x4C;
        if (*(short *)(rec + 0x3E) < (int)*cnt)
            *cnt = rec[0x3E];
        return;
    }

    case 4:
        if (MI(0x10) == 0) {
            if (HELD_A)
                D_0015F588 = D_0015F588 + 1024.0f;
            if (HELD_B)
                D_0015F588 = D_0015F588 - 1024.0f;
            if (D_0015F58C < D_0015F588)
                D_0015F588 = D_0015F58C;
            else if (D_0015F588 < 0.0f)
                D_0015F588 = 0.0f;
        }
        if (MI(0x10) == 2) {
            if (HELD_A)
                D_0015F58C = D_0015F58C + 1024.0f;
            if (HELD_B)
                D_0015F58C = D_0015F58C - 1024.0f;
            if (524288.0f < D_0015F58C)
                D_0015F58C = 524288.0f;
            else if (D_0015F58C < D_0015F588)
                D_0015F58C = D_0015F588;
        }
        if (MI(0x10) == 1) {
            adjust_f_1088(&D_0015F590, 2.55f, 255.0f);
        }
        if (MI(0x10) == 3) {
            adjust_f_1088(&D_0015F594, 2.55f, 255.0f);
        }
        if (MI(0x10) == 4)
            adjust_byte_1088(&D_0015F584);
        if (MI(0x10) == 5)
            adjust_byte_1088(&D_0015F585);
        if (MI(0x10) == 6)
            adjust_byte_1088(&D_0015F586);
        func_001F2930();
        return;

    case 5:
        if (MI(0x10) == 0 && (PRESSED & 0x50))
            MI(0xF0) = (MI(0xF0) == 0);
        if (MI(0x10) == 1 && (PRESSED & 0x50))
            MI(0xF4) = (MI(0xF4) == 0);

        if (MI(0x10) == 2 && (PRESSED & 0xF0)) {
            float *pt;
            long a, c, d;

            found = 0;
            pt = (float *)D_001941E0;
            for (i = 0; i < D_0015F734; i++) {
                float x = F4(D_00187040, 0x140), y = F4(D_00187040, 0x144), z = F4(D_00187040, 0x148);
                float *p = (float *)((char *)pt + i * 4);
                if (p[0] < x + 0.5f && x - 0.5f < p[0] &&
                    p[1] < y + 0.5f && y - 0.5f < p[1] &&
                    p[2] < z + 0.5f && z - 0.5f < p[2]) {
                    found = 1;
                    break;
                }
            }
            if (found)
                return;

            func_00116248(name, D_0015F120, D_001E7AD8, D_0015EE84, D_001E7AF8, D_0015F130);
            a = func_00120778(F4(D_00187040, 0x140));
            c = func_00120778(F4(D_00187040, 0x144));
            d = func_00120778(F4(D_00187040, 0x148));
            func_00116248(msg, D_001E7B10, a, c, d);
            fd = func_0011BF80(name, 0x303);
            if (fd >= 0) {
                func_0011C388(fd, 0, 2);
                len = func_00116810(msg);
                func_0011C820(fd, msg, len);
                func_0011C208(fd);
                M[0xF9] = 0x79;
            }

            pt = (float *)D_001941E0;
            func_00116248(name, D_0015F120, D_001E7AD8, D_0015EE84, D_001E7AF8, D_0015F130);
            fd = func_0011BF80(name, 0x303);
            if (fd >= 0) {
                func_0011C388(fd, 0, 2);
                func_0011C820(fd, D_00187040 + 0x140, 4);
                func_0011C820(fd, D_00187040 + 0x144, 4);
                func_0011C820(fd, D_00187040 + 0x148, 4);
                func_0011C388(fd, 0, 0);
                size = func_0011C388(fd, 0, 2);
                if (size > 0x100000) {
                    D_0015F734 = 0;
                } else {
                    func_0011C388(fd, 0, 0);
                    func_0011C5C0(fd, pt, size);
                    D_0015F734 = size / 12;
                }
                func_0011C208(fd);
            }
        }

        if (MI(0x10) == 3 && (PRESSED & 0xF0)) {
            long a, c, d;

            func_00116248(name, D_0015F120, D_001E7AD8, D_0015EE84, D_001E7AF8, D_0015F130);
            a = func_00120778(F4(D_00187040, 0x140));
            c = func_00120778(F4(D_00187040, 0x144));
            d = func_00120778(F4(D_00187040, 0x148));
            func_00116248(msg, D_001E7B30, a, c, d);
            fd = func_0011BF80(name, 0x303);
            if (fd >= 0) {
                func_0011C388(fd, 0, 2);
                len = func_00116810(msg);
                func_0011C820(fd, msg, len);
                func_0011C208(fd);
                M[0xF8] = 0x79;
            }
        }
        return;
    }
}
