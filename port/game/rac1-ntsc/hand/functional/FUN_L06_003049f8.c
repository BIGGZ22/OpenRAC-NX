/* Carried from the PAL decompilation's nonmatching/l06_blarg/func_L06_00305E38.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L06_003049f8 -- src/overlays/l06_blarg/vendor_002FE5D0.c
 * Best so far: SIZE ours 1532 / retail 1544, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 06 moby update (class 0x463): switch on moby[0x20] (0 init, 1 check/return, 2 spawn loops, 3 vector loop
 *   Best by size: p1.c (1532 vs 1544, 12 short; state-1 test first, plain if chain). p4.c is the switch form with 
 *   Unblock: a read of the state-2 spawn loops and the state-3 vector code against retail register allocation (reg
 */
extern void func_L00_0028D918(int);
extern int func_L00_0028D8C0(void *, int);
extern void func_0022DA68(int, int, void *);
extern int func_0022ED80_0B248(int, int, int) __asm__("func_0022DA68");
extern void func_L06_00305000(char *m);
extern void func_L06_00217300(void *);
extern void func_L00_00257D78(float *, float, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern f32 func_001F9D10_1edff8(void *, void *) __asm__("func_001F9B48");
extern void func_001F4600(void (*)(char *), void *);
extern void func_L06_00305170(char *m);
extern void func_001FA050(float *, float *);
extern void func_001F9FF8(float *, float);
extern void func_001FA378(void *, void *, void *);
extern float func_L00_00257C48(float lo, float hi);
extern void func_L00_001FF500(void *, void *, float);
extern float func_002132A8(float, float);
extern int func_001FA6E0(int, int, float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern int func_001F9688(int);
extern int func_00213260(int);
extern unsigned char *func_L00_002730E0(float *pos, char *vel, int color, unsigned char life, unsigned char b, int mode, float scale);
extern int D_L06_00162238 MACRO_ADDR;
extern int D_L06_00162234 MACRO_ADDR;
extern char *D_L06_0015FFD8 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern char D_L06_00167540[];
extern float D_L06_001FE890[];
extern char D_0013E533[];
extern short D_L06_001621F8;
extern short D_L06_001621FC;
extern short D_L06_00162200;
extern short D_L06_00162204;
extern short D_L06_00162208;
extern short D_L06_00162210;
extern short D_L06_00162214;
extern short D_L06_00162218;
extern short D_L06_0016221C;
extern short D_L06_00162220;
extern short D_L06_00162224;
extern short D_L06_00162228;
extern short D_L06_0016222C;
extern short D_L06_00162230;

/* Level 06 update for moby class 1123: state machine on moby[0x20] that drives effect objects. */
void FUN_L06_003049f8(char *moby)
{
    char *data = *(char **)(moby + 0x78);
    unsigned char state = (unsigned char)moby[0x20];
    int *slot = (int *)(data + 0xF00);
    int n;

    if (state == 1) {
        char *e = D_L06_0015FFD8 + ((*(int *)(data + 0xF10)) << 8);
        if (*(short *)(e + 0xA6) == 0x463) {
            if ((unsigned char)e[0x20] < 3) return;
        }
        moby[0x20] = 2;
        *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
        return;
    }

    if (state == 0) {
        int f10;
        *(float *)(moby + 0x2C) = *(float *)(*(char **)(moby + 0x24) + 0x24) * *(float *)&D_L06_00162210;
        D_L06_00162238 = 0x16;
        D_L06_00162234 = 0x2D;
        slot[1] = 0xB;
        slot[2] = 0x17;
        slot[3] = 0x22;
        slot[0] = 0;
        f10 = *(int *)(data + 0xF10);
        *(int *)(data + 0xF18) = -1;
        if (f10 != -1) {
            moby[0x20] = 1;
            *(int *)(moby + 0x94) = 0;
        } else {
            moby[0x20] = 2;
        }
        return;
    }

    if (state == 2) {
        float tmp[120];
        float f20;
        float f21;
        int i;
        float v;

        if (func_L00_0028D8C0(moby, *(int *)(data + 0xF18)) == 0) {
            *(int *)(data + 0xF18) = func_0022ED80_0B248(0, 4, (int)moby);
        }
        func_L06_00305000(moby);

        v = *(float *)&D_L06_001621F8 + *(float *)&D_L06_001621FC * D_0015ED6C;
        *(float *)&D_L06_001621F8 = v;
        if (1.0f < v) {
            v = v - 1.0f;
            *(float *)&D_L06_001621F8 = v;
        }

        f21 = *(float *)&D_L06_00162204 / 30.0f;
        f20 = -(*(float *)&D_L06_00162204 * 0.5f);

        i = 0;
        do {
            if (slot[i] == 0) {
                char *base = data + i * 0x1E0;
                char *p16 = base;
                char *p17 = base;
                char *v18 = (char *)tmp;
                for (n = 0x1D; n >= 0; n--) {
                    func_L06_00217300(p16);
                    p16 += 0x10;
                    *(float *)p17 = f20;
                    p17 += 0x10;
                    func_L00_00257D78((float *)v18, 0.0f, *(float *)&D_L06_00162200 * D_0015ED6C);
                    v18 += 0x10;
                    f20 = f20 + f21;
                }
                p16 = base + 0x790;
                {
                    char *a17 = (char *)tmp;
                    char *a18 = (char *)tmp + 0x10;
                    char *a19 = (char *)tmp + 0x20;
                    for (n = 0x1B; n >= 0; n--) {
                        func_001F9A10(p16, a17, a18);
                        a17 += 0x10;
                        a18 += 0x10;
                        func_001F9A10(p16, p16, a19);
                        a19 += 0x10;
                        func_001F9A68(p16, p16, 0.333000004f);
                        p16 += 0x10;
                    }
                }
                slot[i] = D_L06_00162234;
            } else {
                int sv = slot[i];
                char *q = data + i * sv;
                char *p16 = q + 0x10;
                char *p17 = q + 0x790;
                for (n = 0x1B; n >= 0; n--) {
                    func_001F9A10(p16, p16, p17);
                    p16 += 0x10;
                    p17 += 0x10;
                }
                slot[i] = slot[i] - 1;
            }
            i = i + 1;
        } while (i < 4);

        if (func_001F9D10_1edff8(moby + 0x10, D_L06_00167540) < 48.0f) {
            func_001F4600(func_L06_00305170, moby);
        }

        {
            int f14 = *(int *)(data + 0xF14);
            if (f14 >= 0) {
                char *blk = D_L06_0015FFD8 + (f14 << 8);
                if (blk[0xBC]) {
                    int f18;
                    *(int *)(moby + 0x94) = 0;
                    moby[0x20] = 3;
                    f18 = *(int *)(data + 0xF18);
                    if (f18 != -1) {
                        char *e = (D_0013E533 + 0x1D) + f18 * 0x70;
                        if (*(char **)(e + 0x88) == moby && e[0x74] != 0) {
                            func_L00_0028D918(f18);
                        }
                    }
                    *(int *)(data + 0xF18) = -1;
                    func_0022DA68(1, 0, moby);
                }
            }
        }
        return;
    }

    if (state == 3) {
        float A[4];
        float V[4];
        float P70[4];
        float P80[4];
        float P90[4];
        float f21;
        float f20;
        int i;

        func_001FA050(A, (float *)(moby + 0x40));
        f21 = 0.0f;
        func_001F9FF8(V, *(float *)&D_L06_00162208);
        func_001FA378(A, A, V);

        i = 0;
        do {
            func_001FA050(V, &D_L06_001FE890[i * 4]);
            func_001FA378(V, A, V);
            func_001F9A10(P70, P70, moby + 0x10);
            P70[3] = 1.0f;
            P70[2] = P70[2] + ((float)i * 1.700000048f + 0.600000024f);
            f20 = 0.0f;
            n = 0x31;
            do {
                float f0 = *(float *)&D_L06_00162204;
                float g;
                float h;
                int r17;
                int s;
                int t;
                unsigned char life;
                int u;
                f0 = func_L00_00257C48(f21, f0 * 0.75f);
                func_L00_001FF500(P80, V, f0);
                func_001F9A10(P80, P80, P70);
                f0 = func_002132A8(f21, 1.0f);
                r17 = func_001FA6E0(*(int *)&D_L06_00162214, *(int *)&D_L06_00162218, f0);
                g = func_002132A8(*(float *)&D_L06_0016221C, *(float *)&D_L06_00162220);
                f20 = g;
                h = func_002132A8((float)*(int *)&D_L06_00162224, (float)*(int *)&D_L06_00162228);
                s = func_001FA898_r(h);
                func_L00_00257D78(P90, *(float *)&D_L06_0016222C * D_0015ED6C,
                                  *(float *)&D_L06_00162230 * D_0015ED6C);
                t = func_001F9688(s);
                life = (unsigned char)t;
                u = func_00213260(0xFF);
                func_L00_002730E0((float *)P80, (char *)P90, r17, life, (unsigned char)u, 0, f20);
                n--;
            } while (n >= 0);
            i = i + 1;
        } while (i < 4);
        moby[0x20] = 4;
        return;
    }
}
