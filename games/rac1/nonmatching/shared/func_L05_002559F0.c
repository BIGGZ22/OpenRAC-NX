/* NON_MATCHING func_L05_002559F0 -- src/overlays/shared/help_00237B00.c
 * Best so far: SIZE ours 1672 / retail 1664, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Shared help-mode picker (1664 bytes): reads the help state at D_0013E633+0xE1D, picks a mode (0x6D/0x6E/0x6F) 
 *   Best candidate p4.c (p5.c is the same with the mode and store swapped, same size): 1644 vs 1664, 20 bytes shor
 */
extern char D_0013D355[];
extern char D_0013D50F[];
extern char D_L05_00174340[];
extern int D_0015EE84 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern int D_0015F778[];
extern void func_L00_00233EE0(float *, float, float, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern int func_L00_001EFFF0(void *, void *, int, int, int);
extern float func_001F9CE8(void *);
extern float func_L00_001FF860(float, float);
extern float func_001FA748(float, float);
extern float func_001FA850(float, float);
extern int func_L01_0028C2D8(void *, void *, float);
extern int func_L00_0025E7F8(void *, int, int, int);
extern int func_L05_002559A0(void);
extern int func_001F9850(int);
extern void func_L00_00232C10(int, int, float);
extern float func_001F9B88(float);
extern void func_L05_0024D680(int, int);
extern void func_0022EE28(int, int, int);
extern void func_L00_00264DB8(int, int);

/* Shared help code: picks the help mode from the player's state and the nearby moby, then shows the matching hint. */
int func_L05_002559F0(void)
{
    unsigned char *g;
    char *h;
    unsigned char *pp;
    unsigned char *s5;
    float v1[4];
    float v2[4];
    float v3[4];
    float f0;
    float f1;
    float f2;
    float f20;
    int mode;
    int r16;
    int r4;
    int ret;
    int ee;
    int t;
    unsigned char c;

    g = D_0013E633 + 0xE1D;
    if (!(D_0015EE6C * 12.0f < *(float *)(g + 0x164))) goto L_BA8;
    f20 = 0.0f;
    func_L00_00233EE0(v1, 0.0f, 0.0f, 0.75f);
    qcopy(v2, v1);
    qcopy(v3, g + 0xE0);
    func_L00_001FF4B0(v3, v3, 0.55f);
    func_001F9BD8(v3, v3, g + 0xE0);
    v3[2] = 0.0f;
    func_001F9BD8(v2, v1, v3);
    if (!func_L00_001EFFF0(v1, v2, 2, 0, 0)) goto L_BA8;
    h = D_L05_00174340;
    if (*(int *)(h + 0x18) != 0) goto L_BAC;
    if (*(int *)(h + 0x1C) == 0) goto L_BAC;
    f20 = func_001F9CE8(h + 0x40);
    f0 = func_L00_001FF860(*(float *)(h + 0x48), f20);
    if (!(1.2217305f < f0)) goto L_BAC;
    f0 = func_L00_001FF860(*(float *)(h + 0x40), *(float *)(h + 0x44));
    f0 = func_001FA748(f0, 3.1415927f);
    *(float *)(g + 0x8A4) = f0;
    f0 = func_L00_001FF860(*(float *)(g + 0xE0), *(float *)(g + 0xE4));
    f0 = func_001FA850(f0, *(float *)(g + 0x8A4));
    if (!(f0 < 0.7853982f)) goto L_BAC;
    mode = (*(float *)(g + 0x2DC) < 1.0f) ? 0x6F : 0x6D;
    goto L_F08;

L_BA8:
L_BAC:
    if ((D_0013E633 + 0xE1D)[0x257] != 0) {
        f20 = func_001F9CE8((D_0013E633 + 0xE1D) + 0x100);
        f0 = func_001F9CE8((D_0013E633 + 0xE1D) + 0xE0) * 0.25f;
        if (f20 < f0) {
            mode = 0x6D;
            goto L_F08;
        }
    }
    ee = D_0015EE84;
    if (ee == 5) {
        if (*(float *)((D_0013E633 + 0xE1D) + 0x88) < 61.4f) {
            *(float *)((D_0013E633 + 0xE1D) + 0x8A0) = 61.4f;
            mode = 0x6E;
            goto L_F08;
        }
    }
    if (ee == 0x10) {
        if (*(float *)((D_0013E633 + 0xE1D) + 0x88) < 76.0f) {
            *(float *)((D_0013E633 + 0xE1D) + 0x8A0) = 76.0f;
            mode = 0x6E;
            goto L_F08;
        }
        if ((D_0013E633 + 0xE1D)[0x12EA] != 0) {
            if (*(float *)((D_0013E633 + 0xE1D) + 0x2DC) < 0.1f) {
                mode = 0x6D;
                goto L_F08;
            }
        }
    }
    if (*(short *)((D_0013E633 + 0xE1D) + 0x30C) != 0) goto L_DEC;
    pp = *(unsigned char **)((D_0013E633 + 0xE1D) + 0x8B4);
    if (pp == 0) goto L_D68;
    r16 = func_L01_0028C2D8((D_0013E633 + 0xE1D) + 0x80, pp, 0.0f);
    r4 = func_L00_0025E7F8(*(unsigned char **)((D_0013E633 + 0xE1D) + 0x8B4), r16, -1, 1);
    *(short *)((D_0013E633 + 0xE1D) + 0x8C8) = r16;
    ee = D_0015EE84;
    if (ee != 5) goto L_D68;
    pp = *(unsigned char **)((D_0013E633 + 0xE1D) + 0x8B4);
    f0 = *(float *)(pp + (r4 << 4) + 0x18);
    f1 = *(float *)(pp + (r16 << 4) + 0x18);
    f20 = f0;
    if (f1 < f0) f20 = f1;
    f2 = func_001F9D48((float *)((D_0013E633 + 0xE1D) + 0x80), (float *)(pp + (r16 << 4) + 0x10));
    if (0.5f < f20 - *(float *)((D_0013E633 + 0xE1D) + 0x88)) {
        if (4.0f < f2) {
            mode = 0x6D;
            goto L_F08;
        }
    }

L_D68:
    if (*(short *)((D_0013E633 + 0xE1D) + 0x30C) != 0) goto L_DEC;
    if (*(short *)((D_0013E633 + 0xE1D) + 0x8BC) == 0) goto L_DF0;
    if (*(short *)((D_0013E633 + 0xE1D) + 0x89C) != 0) goto L_DF0;
    pp = *(unsigned char **)((D_0013E633 + 0xE1D) + 0x2080);
    if (pp[0x53] == 0x55) goto L_DAC;
    if (pp[0x53] != 0x7F) goto L_DEC;

L_DAC:
    if ((unsigned char)pp[0x51] < 0xD) {
        r16 = func_L05_002559A0();
        t = func_001F9850(7);
        func_L00_00232C10(r16, 14, (float)t);
    }

L_DEC:
L_DF0:
    r16 = (*(short *)((D_0013E633 + 0xE1D) + 0x30C) == 0);
    if ((D_0013E633 + 0xE1D)[0x257] != 0) {
        f0 = func_001F9B88(*(float *)((D_0013E633 + 0xE1D) + 0x218) - *(float *)((D_0013E633 + 0xE1D) + 0x2D8));
        if (f0 < 0.05f) r16 = 1;
    }
    if (r16 == 0) {
        ret = 0;
        goto L_END;
    }
    if (*(short *)((D_0013E633 + 0xE1D) + 0x89C) == 0) {
        ret = 0;
        goto L_END;
    }
    if (!(*(float *)((D_0013E633 + 0xE1D) + 0xE8) < 0.0f)) {
        ret = 0;
        goto L_END;
    }
    f20 = 1.0471976f;
    f0 = func_001F9B88(*(float *)((D_0013E633 + 0xE1D) + 0x94));
    if (f20 < f0) {
        mode = 0x6D;
        goto L_F08;
    }
    f0 = func_001F9B88(*(float *)((D_0013E633 + 0xE1D) + 0x90));
    if (f20 < f0) {
        mode = 0x6D;
        goto L_F08;
    }
    f0 = func_001FA850(*(float *)((D_0013E633 + 0xE1D) + 0x98), *(float *)((D_0013E633 + 0xE1D) + 0x874));
    if (1.3962634f < f0) {
        mode = 0x6D;
        goto L_F08;
    }
    pp = *(unsigned char **)((D_0013E633 + 0xE1D) + 0x2080);
    c = pp[0x53];
    if ((unsigned int)(c - 0x69) >= 4) goto L_F18;
    if (*(float *)((D_0013E633 + 0xE1D) + 0xAA8) < (float)D_0015F778[(D_0013E633 + 0xE1D)[0x88F]]) {
        mode = 0x6D;
        goto L_F08;
    }

L_F18:
    if (D_0013D355[0x13B] == 0) goto L_F94;
    ee = D_0015EE84;
    if (ee != 5) goto L_F90;
    if (*(int *)((D_0013E633 + 0xE1D) + 0x6F0) != 4) goto L_F90;
    if (*(int *)((D_0013E633 + 0xE1D) + 0x70C) < 4) goto L_F90;
    s5 = (unsigned char *)D_0013D50F + 1;
    if (s5[7] != 0) goto L_F90;
    s5[7] = 1;
    func_0022EE28(1, 0, 0);
    func_L00_00264DB8(0x53DB, -1);

L_F90:
    *(int *)((D_0013E633 + 0xE1D) + 0x714) = 0;

L_F94:
    if (*(int *)((D_0013E633 + 0xE1D) + 0x2084) != 0x6C) goto L_6004;
    func_L05_0024D680(0x6B, 0);
    c = (*(unsigned char **)((D_0013E633 + 0xE1D) + 0x2080))[0x53];
    if (c == 0x55) {
        ret = 1;
        goto L_END;
    }
    if (c == 0x7F) {
        ret = 1;
        goto L_END;
    }
    r16 = func_L05_002559A0();
    t = func_001F9850(7);
    func_L00_00232C10(r16, 14, (float)t);
    ret = 1;
    goto L_END;

L_6004:
    pp = *(unsigned char **)((D_0013E633 + 0xE1D) + 0x2080);
    c = pp[0x53];
    if (c == 0x55) {
        ret = 0;
        goto L_END;
    }
    if (c == 0x7F) {
        ret = 0;
        goto L_END;
    }
    r16 = func_L05_002559A0();
    t = func_001F9850(7);
    func_L00_00232C10(r16, 14, (float)t);
    ret = 0;
    goto L_END;

L_F08:
    func_L05_0024D680(mode, 1);
    ret = 1;

L_END:
    return ret;
}
