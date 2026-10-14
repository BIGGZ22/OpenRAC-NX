/* Carried from the PAL decompilation's nonmatching/functional/func_L15_0029B750.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
extern char *func_L00_0025B478_29B750(void *, int, int) __asm__("func_L00_0025A420");
extern int func_L00_0025B4D0_29B750(void *, void *, void *, int, int *, float *, int, int) __asm__("func_L00_0025A478");
extern void func_L15_0029B2B0_29B750(void *) __asm__("func_L15_0029A0D0");
extern int func_001FA898_29B750(float) __asm__("func_001FA6D0");
extern float func_L00_001FF860_29B750(float, float) __asm__("func_L00_001FF8B0");
extern void func_L00_0025BBA0_29B750(void *, float *, void *, void *) __asm__("func_L00_0025AB48");
extern void func_L00_0025D5B0_29B750(void *, void *, float, int, int, int) __asm__("func_L00_0025C558");
extern void func_L00_0025E4B0_29B750(void *m, short *p) __asm__("func_L00_0025D458");
extern void func_L00_0025E590_29B750(void *, void *) __asm__("func_L00_0025D538");
extern float func_002140F8_29B750(float, float) __asm__("func_002132A8");
extern float func_001F9878_29B750(float) __asm__("func_001F96B0");
extern void func_L08_00211A38_29B750(float *) __asm__("func_L08_00211B08");
extern float func_001F9D10_29B750(void *, void *) __asm__("func_001F9B48");
extern int func_L00_001EFFF0_29B750(void *, void *, int, void *, void *) __asm__("func_L00_001EFC70");
extern int func_L00_00260FB0_29B750(void *, void *, float, int, int, void *, int) __asm__("func_L00_0025FF38");
extern float func_001FA850_29B750(float, float) __asm__("func_001FA688");
extern float func_001F9B88_29B750(float) __asm__("func_001F99C0");
extern char D_0013E633_29B750[] __asm__("D_0013E533");
extern int *D_L15_001B0DB0_29B750[] __asm__("D_L15_001B0AB0");
extern int D_L15_0015F6A8_29B750 __asm__("D_L15_0015F5C4") MACRO_ADDR;
extern float D_0015EE6C_29B750 __asm__("D_0015ED6C") MACRO_ADDR;
extern float D_0015EE70_29B750 __asm__("D_0015ED70") MACRO_ADDR;
extern int D_L15_00160058_29B750 __asm__("D_L15_0015FFD8") MACRO_ADDR;
extern short D_L15_00161474_29B750 __asm__("D_L15_001613EC");
extern short D_L15_00161478_29B750 __asm__("D_L15_001613F0");
extern short D_L15_0016147C_29B750 __asm__("D_L15_001613F4");
extern short D_L15_00161480_29B750 __asm__("D_L15_001613F8");
extern short D_L15_00161484_29B750 __asm__("D_L15_001613FC");
extern short D_L15_00161488_29B750 __asm__("D_L15_00161400");
extern short D_L15_0016148C_29B750 __asm__("D_L15_00161404");
extern short D_L15_00161490_29B750 __asm__("D_L15_00161408");
extern short D_L15_00161494_29B750 __asm__("D_L15_0016140C");

struct HitQ_29B750_c29a570 { char pad[0x10]; unsigned int v __attribute__((mode(TI))); };

/* FUN_L15_0029a570 -- src/overlays/shared/vendor_00298BB8.c (functional C for the port, not a match)
 * Shared enemy update (levels 15 and 17): takes hits (func_L00_0025A420/0025B4D0, a class-0x47 hit
 * or no health left picks the heavy reaction), plays the hit reactions (knock-back states 0x15/0x17
 * with their spring and anim tables), steps the behaviour (func_L00_0025D458/0025E590), then unless
 * `flag` runs its perception: line of sight to Ratchet within 64, path to him (func_L00_0025FF38),
 * facing within 90 degrees, and the aggro bookkeeping at +0x114/+0x15A..+0x15E/+0x19F.
 * From the staged near miss (nonmatching/shared/FUN_L15_0029a570.c), its logic checked against
 * retail; fixed its signed byte loads (pvars bytes are unsigned), the hit vector copies and the
 * frame layout; made self-contained.
 * equiv: DIFFERENT by shape only: three equality tests have the other branch polarity, retail
 * fills two delay slots with copies of a +0x110 load and a +0x15B/+0x19F constant where this
 * duplicates a +0x15E load and the "+0x114 = 2, +0x158 = 1" block, and where retail stores the
 * loaded D_L15_0015F5C4 (known to be 2) into +0x114 this stores the constant 2. */
void FUN_L15_0029a570(void *mobyv, int flag)
{
    unsigned char *data;
    char *q, *e, *P;
    union { unsigned int q __attribute__((mode(TI))); float f[4]; } vv;
    int ii;
    float ff, dd, dd2;
    unsigned char *moby = mobyv;
    int r16, r2;
    float f0, f1, dx, dy, ang, r, ka, kb;

    if (moby[0x20] == 0) return;
    data = *(unsigned char **)(moby + 0x78);
    f0 = *(float *)(*(char **)(moby + 0x24) + 0x24) * *(float *)&D_L15_00161474_29B750;
    *(float *)(moby + 0x2C) = f0;
    q = func_L00_0025B478_29B750(moby, 0x330000, 0);
    ff = 0.0f;
    r16 = func_L00_0025B4D0_29B750(moby, q, data + 0x20, 0, &ii, &ff, 0, 4);
    if (ii == 1) goto ba60;
    if (moby[0x20] == 0x17) goto ba60;
    if (*(unsigned char *)(D_0013E633_29B750 + 0x2EC1) == 0) func_L15_0029B2B0_29B750(moby);
    f0 = *(float *)(data + 0x20) - ff;
    *(float *)(data + 0x20) = f0;
    e = *(char **)(q + 0x20);
    if (*(short *)(e + 0xA6) == 0x47) r16 = 3;
    if (f0 <= 0.0f) r16 = 1;
    *(int *)(data + 0x90) = func_001FA898_29B750(512.0f);
    *(float *)(data + 0x98) = 0.5f;
    *(int *)(data + 0x94) = 9;
    *(float *)(data + 0x80) = *(float *)&D_L15_00161478_29B750 * D_0015EE70_29B750;
    data[0xAD] = 0;
    switch (r16) {
    case 0:
    case 11:
    default:
        goto ba4c;
    case 1:
    case 2:
        kb = D_0015EE70_29B750;
        ka = D_0015EE6C_29B750;
        *(float *)(data + 0x80) = *(float *)&D_L15_0016147C_29B750 * kb;
        *(float *)(data + 0x88) = *(float *)&D_L15_00161490_29B750 * ka;
        *(float *)(data + 0x8C) = *(float *)&D_L15_0016148C_29B750 * ka;
        *(float *)(data + 0x84) = *(float *)&D_L15_00161494_29B750 * ka;
        *(unsigned short *)(moby + 0x34) &= 0xEFFF;
        *(float *)(data + 0xC0) = 8.0f;
        *(float *)(data + 0xC4) = 16.0f;
        e = *(char **)(q + 0x20);
        dx = *(float *)(moby + 0x10) - *(float *)(e + 0x10);
        dy = *(float *)(moby + 0x14) - *(float *)(e + 0x14);
        dd2 = func_L00_001FF860_29B750(dx, dy);
        vv.q = ((struct HitQ_29B750_c29a570 *)q)->v;
        func_L00_0025BBA0_29B750(vv.f, &dd2, data + 0x88, data + 0x8C);
        func_L00_0025D5B0_29B750(moby, data + 0x70, dd2, 8, 1, 0);
        moby[0x20] = 0x17;
        moby[0xBC] = 2;
        data[0x67] = 0xF0;
        goto ba44;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        kb = D_0015EE70_29B750;
        ka = D_0015EE6C_29B750;
        *(float *)(data + 0x80) = *(float *)&D_L15_00161478_29B750 * kb;
        *(float *)(data + 0x88) = *(float *)&D_L15_00161484_29B750 * ka;
        *(float *)(data + 0x8C) = *(float *)&D_L15_00161480_29B750 * ka;
        *(float *)(data + 0x84) = *(float *)&D_L15_00161488_29B750 * ka;
        *(float *)(data + 0xC0) = 5.0f;
        *(float *)(data + 0xC4) = 10.0f;
        e = *(char **)(q + 0x20);
        dx = *(float *)(moby + 0x10) - *(float *)(e + 0x10);
        dy = *(float *)(moby + 0x14) - *(float *)(e + 0x14);
        dd = func_L00_001FF860_29B750(dx, dy);
        vv.q = ((struct HitQ_29B750_c29a570 *)q)->v;
        func_L00_0025BBA0_29B750(vv.f, &dd, data + 0x88, data + 0x8C);
        func_L00_0025D5B0_29B750(moby, data + 0x70, dd, 7, 1, 0);
        moby[0x20] = 0x15;
        moby[0xBC] = 2;
        data[0x67] = 0x78;
        goto ba44;
    case 9:
    case 10:
        data[0x67] = 0xFA;
        goto ba4c;
    }
ba44:
ba4c:
    func_L00_0025E4B0_29B750(moby, (short *)(data + 0x60));
ba60:
    moby[0xA4] = 0xFF;
    func_L00_0025E590_29B750(moby, data + 0x60);
    if (flag) return;
    if (*(int *)(data + 0x38) != 0) {
        f0 = func_001F9878_29B750(func_002140F8_29B750(180.0f, 240.0f));
        r2 = func_001FA898_29B750(f0);
        *(int *)(data + 0x38) = 0;
        *(float *)(data + 0x130) = (float)r2;
    }
    func_L08_00211A38_29B750((float *)(data + 0x130));
    if (func_001F9D10_29B750(moby + 0x10, D_0013E633_29B750 + 0xEED) < 64.0f) {
        qcopy(vv.f, moby + 0x10);
        vv.f[2] = vv.f[2] + 1.0f;
        P = 0;
        if (*(int *)(data + 0x12C) != -1) P = (char *)D_L15_00160058_29B750 + (*(int *)(data + 0x12C) << 8);
        r2 = func_L00_001EFFF0_29B750(vv.f, D_0013E633_29B750 + 0xEED, 2, P, 0);
        data[0x15A] = (r2 == 0);
    } else {
        data[0x15A] = 0;
    }
    {
        int *Pp = D_L15_001B0DB0_29B750[*(int *)(data + 0x120)];
        r2 = func_L00_00260FB0_29B750(moby, data + 0xD0, 128.0f, 0, 0, (char *)Pp + 0x10, *(int *)Pp);
    }
    r16 = 0;
    if (r2 == 2) {
        data[0x19F] = 0;
        data[0x15B] = 1;
        *(short *)(data + 0x158) = 0;
        goto bd48;
    }
    dx = *(float *)(data + 0xD0) - *(float *)(moby + 0x10);
    dy = *(float *)(data + 0xD4) - *(float *)(moby + 0x14);
    ang = func_L00_001FF860_29B750(dx, dy);
    r = func_001FA850_29B750(*(float *)(moby + 0x48), ang);
    if (r < 1.5707964f) {
        if (*(short *)(D_0013E633_29B750 + 0x1125) == 0) {
            r16 = data[0x15A];
            goto bc3c;
        }
        if (func_001F9B88_29B750(*(float *)(moby + 0x18) - *(float *)(data + 0xD8)) < 4.0f) {
            r16 = data[0x15A];
            goto bc3c;
        }
    }
    if (moby[0x20] == 0x10) {
        r16 = data[0x15A];
    } else if (moby[0x20] == 0x11) {
        r16 = data[0x15A];
    }
bc3c:
    if (*(int *)(data + 0x114) == 1) {
        data[0x15E] = data[0x15B] + 1;
    } else if (*(unsigned char *)(D_0013E633_29B750 + 0x2EC1) == 3) {
        if (data[0x15E] >= 2) data[0x15B] = 1;
    } else {
        data[0x15E] = 0;
    }
    data[0x19F] = 1;
    if (r16 == 0) {
        if (data[0x15A] == 0) data[0x15B] = 1;
        data[0x19F] = 0;
        *(int *)(data + 0x114) = 2;
        *(short *)(data + 0x158) = 0;
    } else {
        if ((*(short *)(data + 0x156) == 0 || data[0x15B] != 0) && *(unsigned char *)(D_0013E633_29B750 + 0x2EC1) == 3) {
            *(int *)(data + 0x114) = 2;
            *(short *)(data + 0x158) = 1;
        } else if (data[0x15E] != 0 && *(int *)(data + 0x114) != 1) {
            *(int *)(data + 0x114) = 2;
            *(short *)(data + 0x158) = 1;
        } else if (*(unsigned char *)(D_0013E633_29B750 + 0x2EC1) != 3 && *(int *)(data + 0x114) != 2) {
            data[0x15B] = 0;
        }
    }
    if (D_L15_0015F6A8_29B750 == 2) *(int *)(data + 0x114) = D_L15_0015F6A8_29B750;
bd48:
    r2 = *(int *)(data + 0x110);
    if (r2 != 0) return;
    *(int *)(data + 0x110) = *(int *)(D_0013E633_29B750 + 0x2E9D);
}
