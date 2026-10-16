/* Carried from the PAL decompilation's src/overlays/shared/vendor_002A5138.c (matched C there) (games/rac1) through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern void *func_0020C4F8(int oClass);
extern void func_L00_00250DF8(void *);
extern char D_0013F350[];
extern float D_0015ED6C MACRO_ADDR;
extern int func_001F9688(int);
extern char *func_L00_0025C338(void *);
extern void func_00214DB0(void *, float, float, float);
typedef float V_c2a96f8[4] __attribute__((aligned(16)));
extern V_c2a96f8 D_L00_00173E60;
extern char D_0013E533[] NOT_SDA;
extern float func_001F9AF0(float *);
typedef int Q_2aa998_c2a96f8 __attribute__((mode(TI)));
typedef union { Q_2aa998_c2a96f8 q; float f[4]; } BoltVector_2AA998_c2a96f8;
typedef struct { char pad[0x10]; BoltVector_2AA998_c2a96f8 position; } BoltOwner_2AA998_c2a96f8;
extern short D_L00_001613C4;
extern short D_L00_001B0770[];
extern char *func_L00_002630C8(void *, void *, void *, float *, float *, float *, float);
extern int func_L00_001EFC70(void *, void *, int, int, int);
extern void func_L00_0025F780(void *, short *, int);



/* Spawns a homing bolt (class 0x100) from owner at pos, aimed at tgt or at whatever lies along its heading. */
char *FUN_L00_002a96f8(void *owner, void *pos, float a, float b, float c, char *tgt, float rz, float ry) {
    char *m;
    char *d;
    BoltVector_2AA998_c2a96f8 t;
    BoltVector_2AA998_c2a96f8 p;
    m = (char *)func_0020C4F8(0x100);
    if (m != 0) {
        ((unsigned char *)m)[0x30] = 0xFF;
        m[0x31] = 1;
        m[0x20] = 0;
        *(float *)(m + 0x2C) *= *(float *)&D_L00_001613C4;
        *(short *)(m + 0x32) = 0xFF;
        d = *(char **)(m + 0x78);
        *(float *)(m + 0x48) = rz;
        *(float *)(m + 0x44) = ry;
        if (tgt == 0) {
            func_00214DB0(&t, D_0015ED6C * 20.0f, rz, ry);
            p.q = *(Q_2aa998_c2a96f8 *)pos;
            tgt = func_L00_002630C8(owner, &p, &t, &a, &b, &c, 0.34906584f);
        }
        *(void **)(d + 0x10) = owner;
        qcopy(m + 0x10, pos);
        *(float *)(d + 0x34) = D_0015ED6C * 100.0f;
        *(int *)(d + 0x38) = 1;
        *(int *)(d + 0x24) = 0;
        *(int *)(d + 0x28) = 0;
        *(int *)(d + 0x20) = 0;
        *(char **)(d + 0x18) = tgt;
        if (tgt != 0) {
            unsigned char *e = (unsigned char *)func_L00_0025C338(tgt);
            if (e != 0) {
                *(unsigned short *)(e + 0x1E) |= 0x80;
                if (e[0xB] != 0) *(int *)(d + 0x38) = 0;
                if (e[0xC] != 0) *(float *)(d + 0x34) = (float)e[0xC] * D_0015ED6C;
            }
        }
        *(float *)(d + 0x2C) = a;
        *(float *)(d + 0x30) = b;
        *(int *)(d + 0x14) = func_001F9688(0x96);
        if (tgt != 0) {
            qcopy(d, tgt + 0x10);
            *(float *)(d + 8) += c;
            *(float *)(d + 0x1C) = c;
        }
        {
            int r;
            if (func_001F9AF0((float *)(D_0013F350 + 0x140)) > D_0015ED6C * 0.1f) {
                r = func_001F9688(0x3C);
                *(short *)(d + 0x3E) = r;
                *(short *)(d + 0x3C) = r;
            } else {
                r = func_001F9688(1);
                *(short *)(d + 0x3E) = r;
                *(short *)(d + 0x3C) = r;
            }
        }
        t.q = ((BoltOwner_2AA998_c2a96f8 *)owner)->position.q;
        t.f[2] = ((float *)pos)[2];
        if (func_L00_001EFC70(&t, pos, 0, *(int *)(D_0013E533 + 0x2E9D), 0) != 0) {
            m[0xBC] = 1;
            qcopy(m + 0x10, &D_L00_00173E60);
        }
        func_L00_00250DF8(m);
        func_L00_0025F780(m, D_L00_001B0770, 0x1F);
        *(float *)(m + 0x2C) += *(float *)(m + 0x2C);
    }
    return m;
}
