/* NON_MATCHING func_L15_002D2398 -- src/overlays/l15_quartu/vendor_0029C1D0.c
 * Best so far: SIZE ours 1196 / retail 1172, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 15 moby update (1172 bytes): finds the owning entry in the 64-entry table, runs a state switch (cases 0.
 *   Remaining differences: frame and saved-register layout (ours saves an extra register and the table base is hoi
 */
extern int func_L00_0025B4D0(void *, void *, void *, int, void *, void *, int, int);
extern int func_001FA898(float);
extern void func_L03_00251A58(float *, float, float);
extern float func_L00_001FF860(float, float);
extern void func_L00_0025BBA0(void *, float *, void *, void *);
extern void func_L00_0025D5B0(float, void *, void *, int, int, int);
extern int func_L00_0028EF68(int i, int a1, int v, int k);
extern void func_L00_00250800(void *, int, void *);
extern void func_L00_0025F4A8(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern void func_L00_002584A8(void *, int, int);
extern void func_L00_0025E4B0(void *m, short *p);
extern void func_L00_0025E590(void *, void *);
extern void func_L00_00260D30(float, void *, void *);
extern int func_L00_00260FB0(float, char *, void *, int, int, void *, int);
extern float func_001F9D10(void *, void *);
extern char D_L15_00178780[];
extern short D_L15_00161BB4;
extern short D_L15_00161B88;
extern short D_L15_00161B90;
extern short D_L15_00161B8C;
extern short D_L15_00161BB8;
extern int D_001414D0 MACRO_ADDR;

/* Level 15 moby update: finds the table entry that owns this moby, runs its state switch and refreshes its timer and target. */
void func_L15_002D2398(void *mobyp) {
    char *moby = (char *)mobyp;
    char *data = *(char **)(moby + 0x78);
    char *vec = data + 0x60;
    char *vec2 = data + 0x120;
    char *other = 0;
    int i = 0;
    int a40;
    int a44 = 0;
    u128 vA;
    u128 vB;
    float f48;
    int ret;
    int sel = 0;
    int r2;
    float f;

    *(float *)(moby + 0x2C) = *(float *)&D_L15_00161BB4 * *(float *)(*(char **)(moby + 0x24) + 0x24);
    *(int *)(data + 0x1C8) = 0;

    while (i < 0x40) {
        char *e = D_L15_00178780 + (i << 6);
        if (*(int *)(e + 0x34) != (int)moby) {
            i++;
            continue;
        }
        if ((*(int *)(e + 0x24) & 0xA0000) == 0) {
            i++;
            continue;
        }
        if (*(int *)(e + 0x20) != D_001414D0) {
            other = e;
            break;
        }
        if ((*(int *)(e + 0x24) & 0x80000) != 0) {
            other = e;
            break;
        }
        *(int *)(e + 0x34) = 0;
        i++;
    }

    if (other != 0) {
        ret = func_L00_0025B4D0(moby, other, data + 0x20, 0, &a40, &a44, 0, 4);
        if (a40 != 1 && moby[0x20] != 10 && moby[0x20] != 0x40
            && *(short *)(*(char **)(other + 0x20) + 0xA6) != 0x350) {
            float d;
            sel = 9;
            d = *(float *)(data + 0x20) - *(float *)&a44;
            *(float *)(data + 0x20) = d;
            if (*(unsigned short *)(other + 0x2A) != 0x100)
                sel = ret;
            if (moby[0x20] == 2)
                sel = 9;
            if (d <= 0.0f)
                sel = 1;

            r2 = func_001FA898(2560.0f);
            *(int *)(data + 0x90) = r2;
            *(float *)(data + 0x98) = 2.5f;
            *(float *)(data + 0x80) = *(float *)&D_L15_00161B88 * D_0015EE70;
            *(float *)(data + 0x84) = 0.000500000024f;
            *(int *)(data + 0x94) = 9;
            ((unsigned char *)data)[0xAD] = 0;

            switch (sel) {
            case 0:
                break;
            case 1:
            case 2: {
                u128 z = 0;
                *(u128 *)&vA = z;
                *(unsigned short *)(moby + 0x34) &= 0xEFFF;
                func_L00_00250800(moby, 7, &vB);
                func_L00_0025F4A8(moby, &vA, &vB, 0.0f, 0.0f, 20, 6, 0x20, 16.0f, 8.0f, 9.0f, 4.0f, -1, 32.0f, 1, 0x10, -1, 0);
                func_L00_0028EF68(0, 0, (int)moby, 0x79);
                moby[0x20] = 10;
                ((unsigned char *)data)[0x67] = 0xF0;
                func_L00_002584A8(moby, 0x200, -1);
                break;
            }
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8: {
                *(float *)(data + 0x88) = *(float *)&D_L15_00161B90 * D_0015EE6C;
                *(float *)(data + 0x8C) = *(float *)&D_L15_00161B8C * D_0015EE6C;
                *(int *)(data + 0x1C8) = 1;
                func_L03_00251A58((float *)vec, 4.0f, 1.0f);
                *(float *)(data + 0xC0) = 12.0f;
                *(float *)(data + 0xC4) = 16.0f;
                {
                    char *p20 = *(char **)(other + 0x20);
                    float fx = *(float *)(moby + 0x10) - *(float *)(p20 + 0x10);
                    float fy = *(float *)(moby + 0x14) - *(float *)(p20 + 0x14);
                    f48 = func_L00_001FF860(fx, fy);
                }
                *(u128 *)&vA = *(u128 *)(other + 0x10);
                func_L00_0025BBA0(&vA, &f48, data + 0x88, data + 0x8C);
                func_L00_0025D5B0(f48, moby, vec, 8, 1, 0);
                func_L00_0028EF68(0, 0, (int)moby, 0x79);
                moby[0x20] = 9;
                ((unsigned char *)data)[0x67] = 0x78;
                break;
            }
            case 9:
            case 10:
                *(int *)(data + 0x1C8) = 1;
                ((unsigned char *)data)[0x67] = 0xFA;
                break;
            case 11:
                break;
            }

            func_L00_0025E4B0(moby, (short *)vec);
        }
        *(int *)(other + 0x34) = 0;
    }

    ((unsigned char *)moby)[0xA4] = 0xFF;
    func_L00_0025E590(moby, vec);
    *(float *)(moby + 0x2C) = *(float *)(*(char **)(moby + 0x24) + 0x24) * *(float *)&D_L15_00161BB8;
    *(int *)(data + 0x164) = 0;

    if (moby[0x20] == 1) {
        f = *(float *)(data + 0x174);
        *(float *)(data + 0x1B8) = f;
        func_L00_00260D30(f, moby + 0, vec2);
    } else {
        f = *(float *)(data + 0x178);
        *(float *)(data + 0x1B8) = f;
        if (*(int *)(data + 0x1D0) == -1) {
            func_L00_00260D30(f, moby, vec2);
        } else {
            char *p = (char *)D_L15_001B0DB0[*(int *)(data + 0x1D0)];
            int n = *(int *)p;
            func_L00_00260FB0(f, moby, vec2, 0, 0, p + 0x10, n);
        }
    }

    f = func_001F9D10(moby + 0x10, vec2);
    if (*(float *)(data + 0x1B8) < f)
        *(int *)(data + 0x164) = 2;
    if (*(int *)(data + 0x160) == 0)
        *(int *)(data + 0x160) = D_001414D0;
}
