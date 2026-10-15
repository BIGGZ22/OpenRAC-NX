/* NON_MATCHING func_L13_0030D880 -- src/overlays/l13_gemlik/vendor_0030CAE0.c
 * Best so far: SIZE ours 1596 / retail 1572, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Gemlik moby update, class 1634: if moby[0xBC]==1, two loops of 2 passes each (jitter the vectors at sp+0x20/0x
 *   Runs 1-2 (p0/p1, D_L13 globals as extern short read through casts): SIZE 1596 vs 1572 (24 bytes over). Retail 
 *   Run 4 (p2, the globals as MACRO_ADDR float/int): SIZE 1696, worse: the constants are rematerialized at each ca
 *   Left: the constants f20-f26 are not kept in registers across the calls (retail reloads nothing inside the loop
 */
extern short D_L13_00161FC0;
extern short D_L13_00161FC4;
extern short D_L13_00161FC8;
extern short D_L13_00161FCC;
extern short D_L13_00161FD4;
extern short D_L13_00161FD8;
extern short D_L13_00161FDC;
extern short D_L13_00161FE0;
extern short D_L13_00161FE4;
extern short D_L13_00161FE8;
extern short D_L13_00161FEC;
extern short D_L13_00161FF0;
extern short D_L13_00161FF4;
extern short D_L13_00161FF8;
extern short D_L13_00161F98;
extern short D_L13_00161F9C;
extern short D_L13_00161FA0;
extern short D_L13_00161FA4;
extern short D_L13_00161FA8;
extern short D_L13_00161FAC;
extern short D_L13_00161FB0;
extern short D_L13_00161FB4;
extern short D_L13_00161FB8;
extern char D_0013E633[];
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern float func_002140F8(float, float);
extern float func_L00_001FF860(float, float);
extern float func_001FA748(float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_001F9878(float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern char *func_00219780(void *, void *, void *, int, int, int, int, int, int);
extern void func_001F9BD8(void *, void *, void *);
extern float func_001FA888(int);
extern int func_001F9938(void *);
extern void func_0020D678(void *);
extern int func_L00_001EFFF0(void *, void *, int, int, int);
extern void func_L10_002E51A8(char *moby);

/* Gemlik moby update (class 1634): two jitter passes over the path vectors, then the moby's 0x2C-0x48 floats. */
void func_L13_0030D880(char *moby)
{
    char *data = *(char **)(moby + 0x78);
    char buf[16];
    float v20[4];
    float v30[4];
    float v40[4];
    float v50[4];
    float f20, f21, f22, f23, f24, f25, f26;
    float r, g;
    int k, ia, ib, ic;
    short v2, v5;

    qcopy(buf, moby + 0x10);
    if (((unsigned char *)moby)[0xBC] == 1) {
        float *p50 = v50;
        v30[3] = *(float *)&D_L13_00161FEC;
        v40[3] = *(float *)&D_L13_00161FF0;
        f22 = -0.0649999976f;
        f21 = 0.0649999976f;
        f26 = 0.899999976f;
        f25 = 1.10000002f;
        f24 = 1.20000005f;
        k = 1;
        do {
            qcopy(v20, moby + 0x10);
            r = func_002140F8(f22, f21);
            v20[0] = v20[0] + r;
            r = func_002140F8(f22, f21);
            v20[1] = v20[1] + r;
            r = func_002140F8(f22, f21);
            v20[2] = v20[2] + r;
            qzero(v30);
            k--;
            f23 = 0.0174532924f;
            g = *(float *)&D_L13_00161FC0 * D_0015EE6C;
            v30[2] = func_002140F8(g * f26, g * f25);
            f20 = func_002140F8(-*(float *)&D_L13_00161FE0, *(float *)&D_L13_00161FE0) * f23;
            r = func_001FA748(func_L00_001FF860(*(float *)(data + 0), *(float *)(data + 4)), f20);
            v40[0] = func_001F9F90(r) * (*(float *)&D_L13_00161FB4 * D_0015EE6C);
            r = func_001FA748(func_L00_001FF860(*(float *)(data + 0), *(float *)(data + 4)), f20);
            g = *(float *)&D_L13_00161FB8 * D_0015EE6C;
            *(int *)&v40[2] = 0;
            v40[1] = func_001F9FA8(r) * (*(float *)&D_L13_00161FB4 * D_0015EE6C);
            r = func_002140F8(g * f26, g * f25);
            v40[2] = r;
            r = func_002140F8(*(float *)&D_L13_00161FA8, *(float *)&D_L13_00161FA8 + *(float *)&D_L13_00161FA8);
            ia = func_001FA898_r(func_001F9878(r));
            r = func_002140F8(*(float *)&D_L13_00161FAC, *(float *)&D_L13_00161FAC * f24);
            ib = func_001FA898_r(func_001F9878(r));
            r = func_002140F8(*(float *)&D_L13_00161FB0, *(float *)&D_L13_00161FB0 * f24);
            ic = func_001FA898_r(func_001F9878(r));
            func_00219780(v20, v30, v40, *(int *)&D_L13_00161F98, *(int *)&D_L13_00161F9C, ia, ib, ic, -1);
            v30[3] = *(float *)&D_L13_00161FF4;
        } while (k >= 0);
        f26 = f23;
        k = 1;
        v30[3] = *(float *)&D_L13_00161FF4;
        v40[3] = *(float *)&D_L13_00161FF8;
        f22 = -0.100000001f;
        f21 = 0.100000001f;
        f25 = 0.699999988f;
        f24 = 1.5f;
        f23 = 1.20000005f;
        do {
            qcopy(p50, moby + 0x10);
            r = func_002140F8(f22, f21);
            v50[0] = v50[0] + r;
            r = func_002140F8(f22, f21);
            v50[1] = v50[1] + r;
            r = func_002140F8(-*(float *)&D_L13_00161FE8, *(float *)&D_L13_00161FE8);
            v50[2] = v50[2] + r;
            qzero(v30);
            k--;
            g = *(float *)&D_L13_00161FD4 * D_0015EE6C;
            v30[2] = func_002140F8(g * f25, g * f24);
            f20 = func_002140F8(-*(float *)&D_L13_00161FE4, *(float *)&D_L13_00161FE4) * f26;
            r = func_001FA748(func_L00_001FF860(*(float *)(data + 0), *(float *)(data + 4)), f20);
            v40[0] = func_001F9F90(r) * (*(float *)&D_L13_00161FD8 * D_0015EE6C);
            r = func_001FA748(func_L00_001FF860(*(float *)(data + 0), *(float *)(data + 4)), f20);
            g = *(float *)&D_L13_00161FDC * D_0015EE6C;
            *(int *)&v40[2] = 0;
            v40[1] = func_001F9FA8(r) * (*(float *)&D_L13_00161FD8 * D_0015EE6C);
            r = func_002140F8(g * f25, g * f24);
            v40[2] = r;
            r = func_002140F8(*(float *)&D_L13_00161FC4, *(float *)&D_L13_00161FC4 + *(float *)&D_L13_00161FC4);
            ia = func_001FA898_r(func_001F9878(r));
            r = func_002140F8(*(float *)&D_L13_00161FC8, *(float *)&D_L13_00161FC8 * f23);
            ib = func_001FA898_r(func_001F9878(r));
            r = func_002140F8(*(float *)&D_L13_00161FCC, *(float *)&D_L13_00161FCC * f23);
            ic = func_001FA898_r(func_001F9878(r));
            func_00219780(v50, v30, v40, *(int *)&D_L13_00161FA0, *(int *)&D_L13_00161FA4, ia, ib, ic, -1);
        } while (k >= 0);
    }

    *(float *)(moby + 0x40) = func_001FA748(*(float *)(moby + 0x40), *(float *)(data + 0x10));
    *(float *)(moby + 0x44) = func_001FA748(*(float *)(moby + 0x44), *(float *)(data + 0x14));
    func_001F9BD8(moby + 0x10, moby + 0x10, data);
    *(float *)(data + 0x8) = *(float *)(data + 0x8) - D_0015EE70 * 14.6000004f;
    if (*(float *)(moby + 0x10) < 0.0f || *(float *)(moby + 0x14) < 0.0f || *(float *)(moby + 0x18) < 0.0f) {
        func_0020D678(moby);
        return;
    }
    v5 = *(short *)(data + 0x18);
    v2 = *(short *)(data + 0x1A);
    if (v5 < v2 / 4) {
        f20 = func_001FA888(v5);
        *(float *)(moby + 0x2C) = *(float *)(data + 0x1C) * f20 / func_001FA888(v2 / 4);
    }
    if (func_001F9938(data + 0x18) != 0) {
        func_0020D678(moby);
    } else if (((unsigned char *)moby)[0xBC] == 1) {
        if (func_L00_001EFFF0(buf, moby + 0x10, 2, *(int *)(D_0013E633 + 0x2E9D), 0)) {
            func_L10_002E51A8(moby);
            func_0020D678(moby);
        }
    }
}
