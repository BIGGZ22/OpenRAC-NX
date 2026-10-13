/* Carried from the PAL decompilation's nonmatching/l15_quartu/func_L15_002C3BD8.c (games/rac1/pal) through the version
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
/* What the PAL file this was written in declares before it. */
extern char D_L15_001673C0[];

/* NON_MATCHING FUN_L15_002c2938 -- src/overlays/l15_quartu/vendor_0029C1D0.c
 * Best so far: SIZE ours 1672 / retail 1684, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped at budget (10 runs), 1672 vs 1684 bytes, not EXACT. Best: p8.c (p9 differs only by an int index). Matc
 *   Still differs: retail copies the list index into $v0 before the sll (`daddu $v0,$v1`) and copies the lhu id in
 */
extern char *func_L00_0025A420(void *, int, int);
extern void func_L00_002628D8(char *, char *, int, float, float);
extern float func_001F96B0(float);
extern float func_002132A8(float, float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern int func_001F9770(void *);
extern float func_001F9B48(void *, void *);
extern int func_001F9688(int);
extern int func_L00_00257B90(int, int);
extern float func_001FA688(float, float);
extern void func_00212F90(void *, int, int, int);
extern float func_L00_00258110(float *vel, float cur, float target, float k, float d, float max);
extern float func_00213308(void);
extern int func_00213260(int);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_L00_001FF290(void *, void *, void *);
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_L00_0026CBB0(void *, void *, int, int, int, int, float);
extern int func_0022DA68(int, int, int);
extern void func_L00_0025F090(void *, void *, int, float, float);
extern void func_L01_002787A0(void *);
extern void *func_L00_00263FD8(void *, int, void *, void *, int, int, float, void *, void *, void *);
extern void func_0020C828(void *);

extern int D_L15_001ABE40[];
extern int D_L15_00160058_m __asm__("D_L15_0015FFD8") MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern float D_L15_0015F580[] MACRO_ADDR;
extern short D_0015EE64_s __asm__("D_0015ED64");
extern short D_L15_00161A00;
extern short D_L15_00161A04;
extern short D_L15_001619FC;
extern short D_L15_001619F4;
extern short D_L15_001619F8;
extern short D_L15_001619EC;

/* Construction arm update (moby class 221, level 15): a state machine on moby[0x20]. */
void FUN_L15_002c2938(char *moby) {
    char *data = *(char **)(moby + 0x78);
    unsigned char *um = (unsigned char *)moby;
    int flag = 0;
    if (um[0x20] != 6) {
        char *r = func_L00_0025A420(moby, 0x10000, 0);
        if (r != 0 && 0.0f < *(float *)(r + 0x2C)) flag = 1;
        if (flag) moby[0x20] = 6;
    }
    if (um[0x20] != 0) {
        float g = *(float *)&D_0015EE64_s;
        float v = *(float *)(data + 0x84);
        float a = g * 0.03f;
        float b = g * 0.3f;
        *(float *)(data + 0x68) = v;
        func_L00_002628D8(moby, data, 0, a, b);
    }
    switch (um[0x20]) {
    case 0: {
        float a = func_001F96B0(*(float *)&D_L15_00161A00 * 60.0f);
        float b = func_001F96B0(*(float *)&D_L15_00161A04 * 60.0f);
        int d = func_001FA898_r(func_002132A8(a, b));
        *(short *)(data + 0x80) = d;
        *(int *)(data + 0x84) = 0;
        *(short *)(data + 0x82) = 0;
        *(int *)(data + 0x88) = 0;
        moby[0x20] = 1;
        moby[0x30] = 0xFF;
        break;
    }
    case 1: {
        int count;
        int ok;
        int id;
        if (!func_001F9770(data + 0x80)) break;
        if (!(func_001F9B48(D_L15_001673C0, moby + 0x10) < 42.0f)) break;
        {
            int k = um[0x21];
            ok = 1;
            if (k != 0xFF) {
                int lst = D_L15_001ABE40[k];
                count = 0;
                if (lst == 0) break;
                {
                    int base = D_L15_00160058_m;
                    unsigned short *p = (unsigned short *)lst;
                    do {
                        char *e;
                        id = *p;
                        e = (char *)(base + ((id & 0x7FFF) << 8));
                        if (e[0x20] >= 0 && *(short *)(e + 0xA6) == *(short *)(moby + 0xA6) && (unsigned char)e[0x20] != 1)
                            count++;
                        p++;
                    } while ((id << 16) >= 0);
                }
                if (count != 0) {
                    *(short *)(data + 0x80) = func_001F9688(0x3C);
                    ok = 0;
                }
            }
        }
        if (!ok) break;
        *(float *)(data + 0x8C) = func_002132A8(-0.209439516f, 0.209439516f);
        moby[0x20] = 2;
        break;
    }
    case 2: {
        int r = func_L00_00257B90(0, 1);
        *(short *)(data + 0x82) = r;
        if (func_001FA688(*(float *)(data + 0x84), *(float *)(data + 0x8C)) < 0.00872664619f) {
            int k = *(short *)(data + 0x82) * 2 + 1;
            int q = func_001F9688(6);
            func_00212F90(moby, k, 0, q);
            moby[0x20] = 3;
        }
        {
            float s = D_0015ED70;
            float m = D_0015ED6C;
            *(float *)(data + 0x84) = func_L00_00258110((float *)(data + 0x88), *(float *)(data + 0x84), *(float *)(data + 0x8C), s * 1.57079637f, s * 0.785398185f, m * 0.785398185f);
        }
        break;
    }
    case 3: {
        if (um[0x53] != *(short *)(data + 0x82) * 2 + 1) break;
        if (!(um[0x70] & 2)) break;
        *(short *)(data + 0x80) = func_001F9688(func_L00_00257B90(0xF, 0x2D));
        {
            int k = *(short *)(data + 0x82) * 2 + 2;
            int q = func_001F9688(3);
            func_00212F90(moby, k, 0, q);
        }
        *(float *)(data + 0x90) = func_00213308();
        moby[0x20] = 4;
        break;
    }
    case 4: {
        if (func_001F9B48(moby + 0x10, D_L15_001673C0) < 20.0f && func_00213260(2) == 0 && um[0x31] != 0) {
            float A = *(float *)&D_L15_001619FC;
            float t1 = func_002132A8(A / 5.0f, A);
            float f22 = t1 * D_0015ED6C;
            float t2 = func_002132A8(0.0f, *(float *)&D_L15_001619FC / 5.0f);
            float f23 = t2 * D_0015ED6C;
            float t3 = func_00213308();
            float vP[4];
            float vA[4];
            float vB[4];
            float vO[4];
            float u1;
            float u2;
            float u3;
            float u4;
            u1 = func_001F9DC8(*(float *)(data + 0x90));
            vA[0] = u1 * f22;
            u2 = func_001F9DE0(*(float *)(data + 0x90));
            vA[1] = u2 * f22;
            vA[2] = 0.0f;
            u3 = func_001F9DC8(t3);
            vB[0] = u3 * f23;
            u4 = func_001F9DE0(t3);
            vB[1] = u4 * f23;
            vB[2] = 0.0f;
            func_L00_001FF290(vO, vA, vB);
            func_L00_0024F7C8(moby, 1, vP);
            {
                int ra = func_001F9688(*(int *)&D_L15_001619F4);
                int rb = func_001F9688(*(int *)&D_L15_001619F8);
                int c = func_L00_00257B90(ra, rb);
                func_L00_0026CBB0(vP, vA, *(int *)&D_L15_001619EC, *(int *)&D_L15_001619EC, c, 1, 40000.0f);
            }
        }
        if (!func_001F9770(data + 0x80)) break;
        {
            int k = *(short *)(data + 0x82) * 2 + 3;
            int q = func_001F9688(6);
            func_00212F90(moby, k, 0, q);
        }
        moby[0x20] = 5;
        break;
    }
    case 5: {
        float a;
        float b;
        int d;
        int r;
        if (um[0x53] != *(short *)(data + 0x82) * 2 + 3) break;
        if (!(um[0x70] & 2)) break;
        a = func_001F96B0(*(float *)&D_L15_00161A00 * 60.0f);
        b = func_001F96B0(*(float *)&D_L15_00161A04 * 60.0f);
        d = func_001FA898_r(func_002132A8(a, b));
        *(short *)(data + 0x80) = d;
        r = func_001F9688(6);
        func_00212F90(moby, 0, 0, r);
        moby[0x20] = 1;
        break;
    }
    case 6:
        func_0022DA68(0, 0, (int)moby);
        func_L00_0025F090(moby, moby + 0x10, -1, 1.5f, 13.0f);
        func_L01_002787A0(moby);
        func_L00_00263FD8(moby, 0x74B, moby + 0x10, moby + 0x40, 0, 0, 0.0f, D_L15_0015F580, D_L15_0015F580, D_L15_0015F580);
        func_L00_00263FD8(moby, 0x74C, moby + 0x10, moby + 0x40, 0, 0, 0.0f, D_L15_0015F580, D_L15_0015F580, D_L15_0015F580);
        func_L00_00263FD8(moby, 0x74D, moby + 0x10, moby + 0x40, 0, 0, 0.0f, D_L15_0015F580, D_L15_0015F580, D_L15_0015F580);
        func_0020C828(moby);
        break;
    }
}
