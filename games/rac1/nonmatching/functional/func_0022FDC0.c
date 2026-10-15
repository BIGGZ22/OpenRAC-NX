/* Port copy of nonmatching/game/func_0022FDC0.c: the two bone matrices are 4x4 locals (retail's
   frame has them at sp+0x20 and sp+0x60) and the trail rings take their translation rows (sp+0x50
   and sp+0x90); the near miss's 16-byte locals let each matrix run over its neighbours, so the
   rings got a rotation row and the ship's engine trails were drawn around the origin.
   One pass of the space loader (text 0x22FDC0). Steps the fade in D_0015F53C,
   counts frames and steps in D_0018CC20, copies the object list's frame
   vectors from the D_0013E130 slots, and calls the loader helpers. */
extern void sl_E97B0(void) __asm__("func_001E97B0");
extern void sl_E97A8(void) __asm__("func_001E97A8");
extern void sl_E448(int, int, int, int, int, int, int, void *) __asm__("func_0012E448");
extern void sl_0520(int) __asm__("func_00205220");
extern void sl_9C30(void *, void *, float) __asm__("func_001F9C30");
extern void sl_9BD8(void *, void *, void *) __asm__("func_001F9BD8");
extern float sl_FA888(int) __asm__("func_001FA888");
extern void sl_D678(void *) __asm__("func_0020D678");
extern void sl_D6D0(void *) __asm__("func_0020D6D0");
extern void sl_DAF8(void *, int, void *) __asm__("func_0020DAF8");
extern void sl_ED48(void *) __asm__("func_0020ED48");
extern float sl_F53C __asm__("D_0015F53C");
extern float sl_67FC __asm__("D_00160504");
extern int sl_EE84 __asm__("D_0015EE84");
extern int sl_EE5C __asm__("D_0015EE5C");
extern short sl_EF4A __asm__("D_0015EF4A");
extern short sl_7DB8 __asm__("D_0015EF48");
extern unsigned char sl_DE4B[] __asm__("D_0013DE4B");
extern unsigned char sl_DE48[] __asm__("D_0013DE48");
extern char sl_A[] __asm__("D_0018CC20");
extern char sl_B[] __asm__("D_0013E130");
extern char sl_C[] __asm__("D_0018CE00");
extern char sl_CD98[] __asm__("D_0018CD98");
extern char sl_E70[] __asm__("D_001D9E70");
extern char sl_ED8[] __asm__("D_001D9ED8");
extern char sl_EC0[] __asm__("D_001D9EC0");
extern char sl_DC0[] __asm__("D_001D9DC0");
extern char sl_E6C0[] __asm__("D_0013E6C0");
extern char sl_1941C0[] __asm__("D_001941C0");
extern char sl_560[] __asm__("D_00160560");
extern char sl_6700[] __asm__("D_00160600");
extern char sl_6710[] __asm__("D_001605F0");
extern char sl_6720[] __asm__("D_001605E0");

void func_0022FDC0(void) {
    float v0[4], v1[4];
    float bone1[4][4], bone2[4][4];
    char *A = sl_A;
    char *B = sl_B;
    char *C = sl_C;
    char *obj, *P, *W, *W0, *W1, *base, *p5, *p16, *X, *cur, *ent, *Qp, *P2;
    char *p19, *p20, *p;
    int *tbl;
    int t, v, k, i, idx, idx1, idx16, n34, a34, a40, a44, r, j, c, par, e, A38;
    unsigned int dbits;
    union { unsigned int u; float f; } cv;
    float f0, f1, f2, f12, f20;

    sl_E97B0();
    f0 = sl_F53C - 0.25f;
    sl_F53C = f0;
    if (f0 < 0.0f) {
        sl_F53C = 0.0f;
    }
    *(int *)(A + 0x38) += 1;
    *(int *)(A + 0x34) += 1;
    n34 = *(int *)(A + 0x34);

    if (n34 == 1) {
        e = sl_EE84;
        if (e != 0 && (e != 1 || sl_DE4B[0] != 0)) {
            func_0012EC30();
            sl_E448(sl_EE5C, *(int *)(B + 0x58), 0x400, 0, 0, 0, 0, sl_E6C0);
            func_0012EC40();
            func_0012DDC0();
        }
    }

    a40 = *(short *)(A + 0x40);
    a34 = *(int *)(A + 0x34);
    if (a34 < a40) {
        if (*(int *)(A + 0x38) >= 0x60) {
            *(int *)(A + 0x3C) += 1;
            sl_0520(*(int *)(A + 0x3C));
        }
        goto L_FC;
    }

    a44 = *(short *)(A + 0x44);
    if (a44 > 0) {
        i = 0;
        do {
            obj = *(char **)(A + 0x178 + 4 * i);
            if (obj != 0) {
                P = *(char **)(obj + 0x24);
                ((unsigned char *)P)[0xC] = (unsigned char)(((unsigned char *)P)[0xC] - 1);
                P = *(char **)(obj + 0x24);
                *(int *)(P + 0x48 + 4 * ((unsigned char *)P)[0xC]) = 0;
                sl_D678(obj);
            }
            i++;
        } while (i < *(short *)(A + 0x44));
    }

    /* L_FF38 */
    e = sl_EE84;
    if (e != 0 && (e != 1 || sl_DE4B[0] != 0)) {
        if (sl_7DB8 < 3) {
            *(int *)(B + 0x5C) = 0;
        }
    }

    /* L_FF70 */
    if (*(int *)(B + 0x5C) >= 2) {
        goto L_B4;
    }
    if (*(int *)(B + 0x5C) == 0) {
        r = func_001160D8();
        *(int *)(B + 0x58) = (*(int *)(B + 0x58) + ((r >> 16) % 3) + 1) & 3;
    } else {
        *(int *)(B + 0x58) = 4;
    }
    /* L_FFBC */
    *(int *)(B + 0x50) = 0;
    *(int *)(B + 0x54) = 0;
    *(int *)(B + 0x5C) = *(int *)(B + 0x5C) + 1;
    {
        int *s = (int *)sl_6720;
        int *d = (int *)sl_6710;
        d[0] = s[0];
        d[1] = s[1];
        d[2] = s[2];
        d[3] = s[3];
    }
    func_001F99B0(A, 0, 0x1C0);

    P2 = sl_1941C0;
    idx = *(int *)(B + 0x58);
    Qp = *(char **)(P2 + 0x14);
    *(int *)(A + 0x58) = *(int *)(P2 + 0x4) + D_0016100C;
    *(int *)(A + 0x5C) = *(int *)(P2 + 0x8) + D_0016100C;
    ent = Qp + 0x50 + 4 * idx;
    t = *(int *)ent;
    base = Qp + *(int *)(Qp + 0x4);
    X = base + t;
    if (*(int *)(X + 4) != 0) {
        tbl = (int *)(A + 0x60);
        cur = X;
        v = *(int *)X;
        k = 0;
        for (;;) {
            k++;
            cur += 8;
            v += 0x800;
            *(char **)(A + 0x60 + 4 * (k - 1)) = X + v;
            if (k >= 0x46) {
                break;
            }
            if (*(int *)(cur + 4) == 0) {
                break;
            }
            v = *(int *)cur;
        }
        (void)tbl;
    }
    /* L_A4 */
    sl_0520(0);

L_FC:
    func_0022F128();
    idx = *(int *)(B + 0x58);
    f1 = ((float *)sl_ED8)[idx];
    f0 = *(float *)(C + 0xB0);
    if (f0 < f1) {
        *(float *)(C + 0xB0) = f1;
    }
    func_001F3140();
    f20 = 1.0f;
    if (*(int *)(B + 0x58) == 4) {
        a40 = *(short *)(A + 0x40);
        a34 = *(int *)(A + 0x34);
        f1 = (float)a40;
        f0 = (float)(a40 - a34);
        f20 = f0 / f1;
        sl_9C30(v0, sl_6700, f20);
        sl_9BD8(sl_6710, sl_6710, v0);
    }
    idx = *(int *)(B + 0x58);
    p16 = sl_E70 + (idx << 4);
    r = func_001F98C0(0x78);
    a34 = *(int *)(A + 0x34);
    f12 = (float)(a34 - r) * 20.0f;
    f12 = f12 * f20;
    sl_9C30(sl_560, p16, f12);

    idx = *(int *)(B + 0x58);
    f0 = ((float *)sl_EC0)[idx];
    a44 = *(short *)(A + 0x44);
    sl_67FC = f0;
    if (a44 <= 0) {
        goto L_560;
    }

    i = 0;
    do {
        obj = *(char **)(sl_CD98 + 4 * i);
        k = 0;
        do {
            P = *(char **)(obj + 0x24);
            A38 = *(int *)(A + 0x38);
            idx = A38 >> 1;
            idx1 = idx + 1;
            c = ((unsigned char *)P)[0xC] - 1;
            p5 = P + 4 * c;
            W = *(char **)(p5 + 0x48);
            par = A38 & 1;
            W1 = *(char **)(W + 0x1C + 4 * idx1);
            W0 = *(char **)(W + 0x1C + 4 * idx);
            p20 = W1 + 0x10;
            p19 = W0 + 0x10;
            f0 = sl_FA888(par);
            f2 = (float)k;
            idx16 = idx << 4;
            f12 = 1.0f;
            f0 = f0 * 0.5f;
            f2 = f2 * 0.25f;
            base = *(char **)(obj + 0x78);
            f0 = f0 + f2;
            f12 = f12 - f0;
            *(float *)(obj + 0x54) = f0;
            sl_9C30(v0, base + idx16, f12);
            sl_9C30(v1, base + 16 * idx1, *(float *)(obj + 0x54));
            sl_9BD8(obj + 0x10, v0, v1);
            ((unsigned char *)obj)[0x52] = 2;
            ((unsigned char *)obj)[0x53] = 2;
            ((unsigned char *)obj)[0x50] = 0;
            sl_D6D0(obj);
            func_001F9A98(*(char **)(obj + 0x68) + 0x10, p19, 0x20);
            func_001F9A98(*(char **)(obj + 0x6C) + 0x10, p20, 0x20);
            ((unsigned char *)obj)[0x72] = 0xFF;
            ((unsigned char *)obj)[0x71] = 0xFF;
            ((unsigned char *)obj)[0x51] = 0;
            *(short *)(obj + 0x32) = 0x1FF;
            sl_ED48(obj);
            P = *(char **)(obj + 0x24);
            ((unsigned char *)obj)[0x52] = (unsigned char)(((unsigned char *)P)[0xC] - 1);
            ((unsigned char *)obj)[0x53] = (unsigned char)(((unsigned char *)P)[0xC] - 1);
            sl_DAF8(obj, 1, bone1);
            sl_DAF8(obj, 2, bone2);
            {
                int b50 = (*(int *)(B + 0x50) + 1) & 0x1F;
                int b54 = *(int *)(B + 0x54);
                *(int *)(B + 0x50) = b50;
                if (b54 < 0x20) {
                    *(int *)(B + 0x54) = b54 + 1;
                }
            }
            j = *(int *)(B + 0x50);
            {
                int *s = (int *)bone1[3];
                int *d = (int *)(B + 0xC0 + 16 * j);
                d[0] = s[0];
                d[1] = s[1];
                d[2] = s[2];
                d[3] = s[3];
            }
            {
                int *s = (int *)bone2[3];
                int *d = (int *)(B + 0x2C0 + 16 * j);
                d[0] = s[0];
                d[1] = s[1];
                d[2] = s[2];
                d[3] = s[3];
            }
            if (*(int *)(B + 0x58) == 4) {
                e = sl_EE84;
                if (e == 0 || (e == 1 && sl_DE48[3] == 0)) {
                    *(unsigned short *)(obj + 0x34) = (unsigned short)(*(unsigned short *)(obj + 0x34) | 1);
                    *(int *)(B + 0x54) = 0;
                }
                a40 = *(short *)(A + 0x40);
                a34 = *(int *)(A + 0x34);
                if (a40 - 0x38 < a34) {
                    cv.u = 0x3C924925u;
                    f2 = cv.f;
                    f1 = (float)(a40 - a34);
                    f1 = f1 * f2;
                    f0 = *(float *)(*(char **)(obj + 0x24) + 0x24);
                    f0 = f0 * f1;
                    *(float *)(obj + 0x2C) = f0;
                    dbits = ((unsigned int)(a40 - a34)) << 24;
                    p = sl_DC0;
                    j = 2;
                    do {
                        *(unsigned int *)p = (*(unsigned int *)p & 0x00FFFFFFu) | dbits;
                        j--;
                        p += 8;
                    } while (j >= 0);
                } else {
                    p = sl_DC0;
                    j = 2;
                    do {
                        *(unsigned int *)p = (*(unsigned int *)p & 0x00FFFFFFu) | 0x38000000u;
                        j--;
                        p += 8;
                    } while (j >= 0);
                }
            }
            k++;
        } while (k < 2);
        i++;
    } while (i < *(short *)(A + 0x44));

L_560:
    sl_E97A8();
    return;

L_B4:
    if (sl_EF4A != 0) {
        sl_EF4A = 0;
    }
    D_0015F6FC = 1;
    return;
}
