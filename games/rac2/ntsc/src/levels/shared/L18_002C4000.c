#include "include_asm.h"
#include "common.h"

/* Frozen Batch A A18-void-store-16; acknowledged claim 4cf4f8b91176606a7af1fd853f7ccd89fa63964199d0883686e46fb6a47255a8. */
typedef unsigned char BatchA_A18_u8;

typedef int BatchA_A18_s32;

typedef short BatchA_A18_BatchAs16;

/* Frozen Batch A A09-cond-select-36; acknowledged claim 3dadb64036892dfb357f8b3f0f7b47fc608b60613dcd8c823ef8df1bc26f7e5b. */
typedef unsigned char BatchA_A09_u8;

typedef int BatchA_A09_s32;

extern void func_00335668(BatchA_A18_u8 *a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2);


extern BatchA_A18_u8 D_0018C0D4;

INCLUDE_ASM("asm/levels", func_L18_002C4000);

/* func_L18_002C41F0 is defined in L18_002C4000-g8.c (this unit has $gp access for it). */

INCLUDE_ASM("asm/levels", func_L18_002CFD68);

INCLUDE_ASM("asm/levels", func_L18_002CFF48);

INCLUDE_ASM("asm/levels", func_L18_002D0C00);

INCLUDE_ASM("asm/levels", func_L18_002D1998);

INCLUDE_ASM("asm/levels", func_L18_002DEFF8);

extern BatchA_A18_s32 ref_L18_002E4090_00(BatchA_A18_s32 a0, BatchA_A18_s32 a1);

void func_L18_002E4090(BatchA_A18_s32 a0) {
    BatchA_A18_s32 e = a0 ? 2 : 1;
    switch (D_0018C0D4) {
    case 0:
        ref_L18_002E4090_00(0, e);
        break;
    case 4:
        ref_L18_002E4090_00(0x8D, e);
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L18_002E42F8);

INCLUDE_ASM("asm/levels", func_L18_002F1098);

extern void func_0027F1D8(void *a0, void *a1, const char *a2);
extern char D_L18_00296510[];

void func_L18_00303A58(void *a0, void *a1) {
    func_0027F1D8(a0, a1, D_L18_00296510);
}

INCLUDE_ASM("asm/levels", func_L18_003044B8);

INCLUDE_ASM("asm/levels", func_L18_00307C60);

BatchA_A18_s32 func_L18_0032B768(BatchA_A18_u8 *s, BatchA_A18_s32 a1, BatchA_A18_s32 a2) {
    BatchA_A18_u8 *p = *(BatchA_A18_u8 **)(s + 0x98);
    BatchA_A18_u8 *q;
    if (p == 0) {
        return 0;
    }
    if (a1 >= (*(BatchA_A18_s32 *)(p + 0x4) >> 5)) {
        return 0;
    }
    p += ((a1 << 5) + 0x10);
    *(p + 2) = (BatchA_A18_u8)a2;
    return 1;
}

extern BatchA_A18_s32 func_002A8AF0(BatchA_A18_s32 a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2);
extern BatchA_A18_s32 D_001FE480[];

/* hands the slot's payload to its handler; the slot table entry holds count then payload */
BatchA_A18_s32 func_L18_0033B628(BatchA_A18_s32 a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2) {
    BatchA_A18_u8 *p;
    if (a1 != -1) {
        p = (BatchA_A18_u8 *)D_001FE480[a1];
        if (p != 0) {
            if (*(BatchA_A18_s32 *)p != 0) {
                return func_002A8AF0(a0, (BatchA_A18_s32)(p + 0x10), *(BatchA_A18_s32 *)p);
            }
        }
    }
    return a2;
}

INCLUDE_ASM("asm/levels", func_L18_0035D238);

extern float func_L18_0035D5F8(BatchA_A18_s32 a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2,
                               BatchA_A18_s32 a3);
extern BatchA_A18_s32 D_001FE480[];

float func_L18_0035D5A8(BatchA_A18_s32 a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2,
                        BatchA_A18_s32 a3) {
    if (a3 == -1) {
        return -1.0f;
    }
    a3 = D_001FE480[a3];
    if (a3 == 0) {
        return -1.0f;
    }
    return func_L18_0035D5F8(a0, a1, a2, a3);
}

INCLUDE_ASM("asm/levels", func_L18_0035D5F8);

/* func_L18_003F50D8 is matched in L18_003F50D8.c (built with -mno-split-addresses). */

INCLUDE_ASM("asm/levels", func_L18_003F5198);

INCLUDE_ASM("asm/levels", func_L18_003F53D8);

INCLUDE_ASM("asm/levels", func_L18_003F5650);

INCLUDE_ASM("asm/levels", func_L18_003F6CA8);

extern float func_00282E48(char *, void *);
extern void func_002A93B0(void *);
extern void func_L18_003F74B8(void *a0, void *a1);
extern void func_002B0990(char *a0, char *a1, BatchA_A18_s32 a2, BatchA_A18_s32 a3, float f12);
extern char D_L18_001B97C0[];

/* Advances the object's step counter, refreshes the emitter and asks the particle
   system for the new position; the distance test only runs when the flag is set. */
void func_L18_003F7198(void *a0) {
    BatchA_A18_u8 *s0 = (BatchA_A18_u8 *)a0;
    void *s1;
    unsigned short v1;

    v1 = *(unsigned short *)((char *)s0 + 0x96);
    if (v1 <= 0xFFFE) {
        s1 = *(void **)((char *)s0 + 0x68);
        *(unsigned short *)((char *)s0 + 0x96) = v1 + 1;
    } else {
        s1 = *(void **)((char *)s0 + 0x68);
    }
    *(unsigned short *)((char *)s1 + 0x290) = 0;
    if (s0[0x31] != 0) {
        if (func_00282E48((char *)s0 + 0x10, D_L18_001B97C0) < 24.0f) {
            func_002A93B0(s0);
            s0[0x6F] = 0x16;
        }
    }
    if (*(float *)((char *)s0 + 0x10) < 5.0f || *(float *)((char *)s0 + 0x10) > 1018.0f ||
        *(float *)((char *)s0 + 0x14) < 5.0f || *(float *)((char *)s0 + 0x14) > 1018.0f ||
        *(float *)((char *)s0 + 0x18) < 5.0f || *(float *)((char *)s0 + 0x18) > 1018.0f) {
        func_L18_003F74B8(s0, 0);
    }
    func_002B0990((char *)s0, (char *)s1 + 0x2B0, 2, 1, 1.8f);
}

extern float func_00283248(float a, float b);

void func_L18_003F72D8(BatchA_A18_u8 *a0, BatchA_A18_u8 *a1) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    *(unsigned short *)((char *)p + 0x290) = 1;
    *(float *)((char *)p + 0x264) =
        func_00283248(*(float *)(a1 + 0x0) - *(float *)(a0 + 0x10),
                      *(float *)(a1 + 0x4) - *(float *)(a0 + 0x14));
}

INCLUDE_ASM("asm/levels", func_L18_003F7320);

extern float func_00308AD0(BatchA_A18_u8 *a0, BatchA_A18_u8 *a1);
extern BatchA_A18_s32 func_L18_003F8E40(BatchA_A18_u8 *s);

BatchA_A18_s32 func_L18_003F7358(BatchA_A18_u8 *a0, BatchA_A18_u8 *a1) {
    BatchA_A18_s32 r = 0;
    if (func_00308AD0(a0 + 0x10, a1 + 0x10) < 5.0f) {
        r = func_L18_003F8E40(a1) != 0;
    }
    return r;
}

/* func_L18_003F73B8 */
extern float func_00282E80(char *, void *);
extern float func_00283248(float, float);
extern float func_00283C80(float, float);

/* The 1.5707964 spelling is load-bearing: this assembler under-rounds by an ulp
   (1.5707964f, 1.57079637f and even 1.5707963705062866f all encode 0x3FC90FDB,
   retail holds 0x3FC90FDC).  Measured by sweeping spellings, COMPILER-NOTES. */
int func_L18_003F73B8(char *a0, float *a1)
{
    char *s1 = a0;
    float *s0 = a1;
    int r = 0;

    if (*(unsigned char *)(s1 + 0x31) != 0) {
        if (func_00282E80(s1 + 0x10, a1) < 3.2f) {
            if (func_00283C80(*(float *)(s1 + 0xF8),
                             func_00283248(s0[0] - *(float *)(s1 + 0x10),
                                           s0[1] - *(float *)(s1 + 0x14))) < 1.5707965f)
                r = 1;
        }
    }
    return r;
}


extern BatchA_A18_s32 func_002A77E0(BatchA_A18_s32 a0);

extern BatchA_A18_s32 func_L18_003F75E8(BatchA_A18_s32 a0, BatchA_A18_u8 *a1);

/* runs the 0x1E gate and, when it clears, the record query into a local buffer;
   answers whether the query found anything */
BatchA_A18_s32 func_L18_003F7470(BatchA_A18_u8 *a0) {
    BatchA_A18_u8 buf[16];
    BatchA_A18_s32 p;

    if (func_002A77E0(0x1E) == 0) {
        p = *(BatchA_A18_s32 *)(a0 + 0x68);
        return func_L18_003F75E8(*(BatchA_A18_s32 *)((char *)p + 0x25C), buf) != 0;
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L18_003F74B8);

extern void func_002A7750(BatchA_A18_u8 *a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2);

void func_L18_003F7590(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    *(unsigned short *)((char *)p + 0x88) = 0;
    *(BatchA_A18_s32 *)(a0 + 0x98) = 0;
    *(unsigned short *)(a0 + 0x34) =
        (unsigned short)((*(unsigned short *)(a0 + 0x34) & 0xEFFF) | 0x43);
    func_002A7750(a0, 1, 0);
    *(BatchA_A18_s32 *)(a0 + 0x98) = 0;
    *(unsigned short *)(a0 + 0x34) =
        (unsigned short)(*(unsigned short *)(a0 + 0x34) & 0xEFFF);
}

/* The group id lists, the entity bank base and the resident group count. The two scalars are
   `sda` arrays of unknown size: cc1 emits the one-instruction load, which the assembler
   expands absolute (`lui`/`lw`) as retail has it in this -G0 unit. */
extern unsigned short *D_L18_001FB320[];
extern char *D_L18_001B281C[] __attribute__((sda));
extern int D_L18_001B2838[] __attribute__((sda));

/* Counts the entities of kind 0xB69 in state 1 that the group's id list names, stores the count
   through a1 and returns the last of them (0 when there is none). Retail compares the state with
   0xFE and 0xFD before testing it for 1. */
BatchA_A18_s32 func_L18_003F75E8(BatchA_A18_s32 a0, BatchA_A18_u8 *a1) {
    unsigned short *q;
    unsigned char *e;
    unsigned char *r;
    BatchA_A18_s32 *cnt = (BatchA_A18_s32 *)a1;

    *cnt = 0;
    if (a0 == -1)
        return 0;
    r = 0;
    for (q = D_L18_001FB320[a0];
         (a0 & 0xFF) != 0xFF && a0 <= D_L18_001B2838[0] && q != 0 &&
         ((short)q[-1] >= 0 || q == D_L18_001FB320[a0]);
         q++) {
        e = (unsigned char *)(D_L18_001B281C[0] + ((*q & 0x7FFF) << 8));
        if (e == 0)
            break;
        if (*(short *)(e + 0xAA) == 0xB69 && e[0x20] != 0xFE && e[0x20] != 0xFD && e[0x20] == 1) {
            (*cnt)++;
            r = e;
        }
    }
    return (BatchA_A18_s32)r;
}

INCLUDE_ASM("asm/levels", func_L18_003F76F0);

extern void func_00282F20(void *a0, void *a1, float f12);
extern void func_00282CC0(void *a0, void *a1, void *a2);

/* the object's state flags and aim angle are set, then the target is handed
   over to the object's controller */
void func_L18_003F7818(BatchA_A18_u8 *s2, BatchA_A18_u8 *s3, BatchA_A18_s32 s7) {
    float local[4];
    BatchA_A18_u8 *s4;
    BatchA_A18_s32 *p;
    BatchA_A18_s32 v1;
    BatchA_A18_u8 state;

    s4 = *(BatchA_A18_u8 **)(s2 + 0x68);
    state = s2[0x20];
    if (state == 1) {
        p = *(BatchA_A18_s32 **)(s2 + 0x24);
        *(unsigned short *)(s2 + 0x34) =
            (unsigned short)((*(unsigned short *)(s2 + 0x34) & 0xFFBE) | 0x1000);
        v1 = p[4];
        *(unsigned short *)(s2 + 0x34) =
            (unsigned short)(*(unsigned short *)(s2 + 0x34) & 0xFFFD);
        *(BatchA_A18_s32 *)(s2 + 0x98) = v1;
        s4[0x2E] = state;
        s2[0x7F] = 0;
        *(float *)(s4 + 0x20) = (float)*(short *)(s4 + 0x24);
        func_00282F20(local, s3 + 0xC0, -0.5f);
        func_00282CC0(s2 + 0x10, s3 + 0x10, local);
        if (s7 != 0) {
            func_00282F20(local, s3 + 0xD0, 0.5f);
        } else {
            func_00282F20(local, s3 + 0xD0, -0.5f);
        }
        func_00282CC0(s2 + 0x10, s3 + 0x10, local);
        *(float *)(s2 + 0xF8) =
            func_00283248(*(float *)(s3 + 0x10) - *(float *)(s2 + 0x10),
                          *(float *)(s3 + 0x14) - *(float *)(s2 + 0x14));
    }
    *(BatchA_A18_s32 *)(s4 + 0x258) = (BatchA_A18_s32)s3;
    func_002A7750(s2, 0xB, 2);
    func_L18_003F8E30((BatchA_A18_u8 *)*(BatchA_A18_s32 *)(s4 + 0x258));
}

INCLUDE_ASM("asm/levels", func_L18_003F7988);

void func_L18_003F7A08(BatchA_A18_u8 *a0) {
    BatchA_A18_u8 *s0 = a0;
    BatchA_A18_s32 p;
    if (s0[0x95] != 1) {
        return;
    }
    p = *(BatchA_A18_s32 *)(s0 + 0x68);
    if (*(BatchA_A18_s32 *)(p + 0x258) != 0) {
        func_00335668(s0, 0xB, 2);
    } else if (*(BatchA_A18_s32 *)(p + 0x278) != -1) {
        func_00335668(s0, 0xD, 2);
    } else {
        func_00335668(s0, 0xC, 2);
    }
    func_00335668(s0, 7, -1);
}

extern void func_L18_003F7A08(BatchA_A18_u8 *a0);
void func_L18_003F7A88(BatchA_A18_u8 *a0) {
    if (a0[0x95] == 1) {
        func_L18_003F7A08(a0);
    }
}

/* func_L18_003F7AB8 is defined in L18_002C4000-g8.c (this unit has $gp access for it). */

extern void func_L18_003FB5E8(void *a0, void *a1);
extern void func_00282F20(void *a0, void *a1, float f12);
extern void func_00282CC0(void *a0, void *a1, void *a2);
extern float func_002A89A8(void *a0, BatchA_A18_s32 a1, float f12);
extern void func_002833F0(char *a, char *b);

/* Moves the object's state word, refreshes the two transform matrices through the
   shared helpers and writes the measured distance back into the object. */
void func_L18_003F8D28(void *a0, void *a1, void *a2) {
    char scratch[16];
    char mat[16];
    unsigned short v1;
    void *p;
    void *s4;
    void *v;
    float f;

    v1 = *(unsigned short *)((char *)a0 + 0x34);
    p = *(void **)((char *)a0 + 0x24);
    v1 = (unsigned short)((v1 & 0xFFBE) | 0x1000);
    *(unsigned short *)((char *)a0 + 0x34) = v1;
    v = *(void **)((char *)p + 0x10);
    *(void **)((char *)a0 + 0x98) = v;
    func_002A7750((BatchA_A18_u8 *)a0, 5, 2);
    s4 = *(void **)((char *)a0 + 0x68);
    *(void **)((char *)s4 + 0x74C) = a2;
    if (a1 != 0) {
        func_L18_003FB5E8(a1, mat);
        func_00282F20(scratch, (char *)a1 + 0xD0, -0.3f);
        func_00282CC0((char *)a0 + 0x10, mat, scratch);
        *(float *)((char *)a0 + 0x18) = func_002A89A8((char *)a0 + 0x10, 0, 0.5f);
        f = func_00283248(*(float *)mat - *(float *)((char *)a0 + 0x10),
                           *(float *)(mat + 4) - *(float *)((char *)a0 + 0x14));
        *(float *)((char *)a0 + 0xF8) = f;
        func_002833F0((char *)a0 + 0xC0, (char *)a0 + 0xF0);
        *(void **)((char *)s4 + 0x750) = a1;
        *(BatchA_A18_u8 *)((char *)a0 + 0x7F) = 0;
    }
}

/* A18-void-store-16 freshly exact-verified under family reservation */
void func_L18_003F8E30(BatchA_A18_u8 *s) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(s + 0x68);
    *(BatchA_A18_BatchAs16 *)((char *)p + 0x768) = 1;
}

/* A18-cond-select twin of 003F8E80: two independent state gates */
BatchA_A18_s32 func_L18_003F8E40(BatchA_A18_u8 *s) {
    BatchA_A18_s32 r = 0;
    if ((s[0x20] == 5 && s[0xBC] == 1) || (s[0x20] == 3 && s[0x94] == 5)) {
        r = 1;
    }
    return r;
}

/* A09-cond-select-36 freshly exact-verified under family reservation */
BatchA_A09_s32 func_L18_003F8E80(BatchA_A09_u8 *s) {
    BatchA_A09_s32 r = 0;
    if (s[0x20] == 6) { r = (s[0xBC] == 2); }
    return r;
}

INCLUDE_ASM("asm/levels", func_L18_003F8EA8);

extern void func_00335388(BatchA_A18_u8 *a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2,
                          BatchA_A18_s32 a3);

void func_L18_003F8FA8(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    func_00335388(a0, 0, 0, 3);
    *(BatchA_A18_s32 *)(a0 + 0x48) = 0;
    *(unsigned short *)((char *)p + 0x98) = 0;
    *(BatchA_A18_s32 *)(a0 + 0x98) = 0;
    *(unsigned short *)(a0 + 0x34) =
        (unsigned short)((*(unsigned short *)(a0 + 0x34) | 0x41) & 0xEFFF);
    func_00335668(a0, 1, 1);
    *(BatchA_A18_s32 *)(a0 + 0x98) = 0;
}

INCLUDE_ASM("asm/levels", func_L18_003F9DA0);

INCLUDE_ASM("asm/levels", func_L18_003F9E50);

INCLUDE_ASM("asm/levels", func_L18_003F9EF0);

extern void func_L18_003FC9B8(unsigned char *a0);
extern void func_L18_003F7988(unsigned char *a0);

/* Slot i of the object's eight group ids (the words at +0x60). Retail rebuilds the slot address
   at every test of the walk, which is what an inline helper expanded in place does. */
static __inline__ int L18_GroupId(unsigned char *p, int i) {
    int off = i << 2;
    int *ids = (int *)(p + 0x60);
    return *(int *)((char *)ids + off);
}

/* Walks the id list of each of the object's eight groups and, for every entity in it, calls the
   handler of its kind (0xD5C, 0x11CC, 0xD9B, 0x11CF or 0xB69), clears the entity's word at +0x98
   and sets bits 0 and 1 of its flags at +0x34; then stores -1 at +0x88 of the object. */
void func_L18_003F9FB0(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)(a0 + 0x68);
    unsigned short *q;
    unsigned char *e;
    int i;

    for (i = 0; i < 8; i++) {
        for (q = D_L18_001FB320[L18_GroupId(p, i)];
             (L18_GroupId(p, i) & 0xFF) != 0xFF && L18_GroupId(p, i) <= D_L18_001B2838[0] &&
             q != 0 && ((short)q[-1] >= 0 || q == D_L18_001FB320[L18_GroupId(p, i)]);
             q++) {
            e = (unsigned char *)(D_L18_001B281C[0] + ((*q & 0x7FFF) << 8));
            if (e == 0)
                break;
            if (*(short *)(e + 0xAA) == 0xD5C || *(short *)(e + 0xAA) == 0x11CC ||
                *(short *)(e + 0xAA) == 0xD9B || *(short *)(e + 0xAA) == 0x11CF) {
                func_L18_003FC9B8(e);
            } else if (*(short *)(e + 0xAA) == 0xB69) {
                func_L18_003F7988(e);
            }
            *(int *)(e + 0x98) = 0;
            *(unsigned short *)(e + 0x34) |= 3;
        }
    }
    *(int *)(p + 0x88) = -1;
}

INCLUDE_ASM("asm/levels", func_L18_003FA180);

INCLUDE_ASM("asm/levels", func_L18_003FA7C0);

BatchA_A18_s32 func_L18_003FA8E0(BatchA_A18_u8 *s) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(s + 0x68);
    if (*(short *)((char *)p + 0xA8) > 0) {
        *(unsigned short *)((char *)p + 0xA8) =
            (unsigned short)(*(unsigned short *)((char *)p + 0xA8) - 1);
        return 1;
    }
    return 0;
}

void func_L18_003FA908(BatchA_A18_u8 *s) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(s + 0x68);
    *(unsigned short *)((char *)p + 0xA8) =
        (unsigned short)(*(unsigned short *)((char *)p + 0xA8) + 1);
}

INCLUDE_ASM("asm/levels", func_L18_003FA920);

INCLUDE_ASM("asm/levels", func_L18_003FAE50);

INCLUDE_ASM("asm/levels", func_L18_003FB500);

INCLUDE_ASM("asm/levels", func_L18_003FB5E8);

void func_L18_003FC958(BatchA_A18_u8 *a0, BatchA_A18_s32 a1) {
    if (a1 != 0) {
        func_00335668(a0, 3, -1);
    } else {
        func_00335668(a0, 2, -1);
    }
    *(BatchA_A18_s32 *)(a0 + 0x98) = 0;
    *(unsigned short *)(a0 + 0x34) =
        (unsigned short)((*(unsigned short *)(a0 + 0x34) | 0x41) & 0xEFFF);
}

INCLUDE_ASM("asm/levels", func_L18_003FC9B8);

INCLUDE_ASM("asm/levels", func_L18_003FCA38);

INCLUDE_ASM("asm/levels", func_L18_003FCC48);

INCLUDE_ASM("asm/levels", func_L18_003FCD70);

INCLUDE_ASM("asm/levels", func_L18_003FD630);

INCLUDE_ASM("asm/levels", func_L18_003FD730);

/* store -1 into field 0x74 of the object at s[0x68] */
void func_L18_003FE010(BatchA_A18_u8 *s) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(s + 0x68);
    *(BatchA_A18_s32 *)((char *)p + 0x74) = -1;
}

void func_L18_003FE020(BatchA_A18_u8 *s) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(s + 0x68);
    *(BatchA_A18_s32 *)((char *)p + 0x74) = 0;
}

INCLUDE_ASM("asm/levels", func_L18_00426AB0);

extern void func_0033A0B0(BatchA_A18_u8 *a0, BatchA_A18_u8 *a1);
extern void func_0032D118(BatchA_A18_u8 *a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2,
                          BatchA_A18_s32 a3);

void func_L18_00426D98(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    *(unsigned short *)((char *)p + 0x50) = 0;
    *(unsigned short *)((char *)p + 0x52) = 1;
    func_0033A0B0(a0, (BatchA_A18_u8 *)p + 0x50);
    *(BatchA_A18_u8 *)((char *)p + 0x54) = 0x80;
    *(BatchA_A18_u8 *)((char *)p + 0x55) = 0x80;
    *(BatchA_A18_u8 *)((char *)p + 0x56) = 0x80;
    func_0032D118(a0, 0x80, 0x80, 0x80);
}

INCLUDE_ASM("asm/levels", func_L18_00426E00);

INCLUDE_ASM("asm/levels", func_L18_00426E48);

extern void func_002EC9C8(BatchA_A18_u8 *a0, BatchA_A18_s32 a1);

void func_L18_00426F28(BatchA_A18_u8 *a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2, float f12) {
    if (a0[0xBC] == 0) {
        if (a0[0x43] != a1) {
            func_00335388(a0, a1, a2, (BatchA_A18_s32)f12);
        }
    } else {
        func_002EC9C8((BatchA_A18_u8 *)a1, a2);
    }
}

extern void func_002B60B0(float a0, float a1, float a2, float a3);
extern void func_00282948(void *a0);

/* the object's scale angles are re-submitted in degrees, then the matrix is rebuilt */
void func_L18_00426F78(BatchA_A18_u8 *a0) {
    BatchA_A18_u8 *p = *(BatchA_A18_u8 **)(a0 + 0x68);
    if (a0[0x21] != 0xFF) {
        if (*(BatchA_A18_u8 *)(p + 0x39F) != 0) {
            func_002B60B0(0.00027777778450399637f * *(float *)(p + 0x3C4),
                          0.01666666753590107f * *(float *)(p + 0x3C0),
                          4.848137450608192e-06f * *(float *)(p + 0x3D4),
                          0.0002908882452175021f * *(float *)(p + 0x3D0));
            func_00282948(*(void **)(p + 0x388));
        }
    }
    *(BatchA_A18_u8 *)(p + 0x39F) = 0;
}

extern void func_00282948(void *a0);

void func_L18_00427010(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    if (a0[0x21] != 0xFF && *(BatchA_A18_u8 *)(p + 0x3B0) != 0) {
        func_00282948(*(void **)(p + 0x3AC));
    }
    *(BatchA_A18_u8 *)(p + 0x3B0) = 0;
}

void func_L18_00427058(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    BatchA_A18_s32 q;
    if (a0[0x21] == 0xFF) {
        return;
    }
    if (*(BatchA_A18_u8 *)(p + 0x3B0) != 0) {
        return;
    }
    q = *(BatchA_A18_s32 *)(p + 0x3AC);
    (*(BatchA_A18_s32 *)q)++;
    *(BatchA_A18_u8 *)(p + 0x3B0) = 1;
}

extern BatchA_A18_s32 func_L09_0033E690(BatchA_A18_u8 *a0, BatchA_A18_s32 a1, float f12);
extern BatchA_A18_s32 func_L09_00329178(BatchA_A18_u8 *a0, BatchA_A18_s32 a1,
                                        BatchA_A18_s32 a2, BatchA_A18_s32 a3);

/* re-seats the object on a new animation; -1 means "leave it to the level" */
BatchA_A18_s32 func_L18_00427130(BatchA_A18_u8 *a0, BatchA_A18_s32 a1, BatchA_A18_s32 a2) {
    BatchA_A18_u8 *p = *(BatchA_A18_u8 **)(a0 + 0x68);
    BatchA_A18_s32 r;
    *(BatchA_A18_s32 *)(p + 0x38C) = a1;
    if (a1 == -1) {
        *(BatchA_A18_BatchAs16 *)(p + 0x1C8) = (BatchA_A18_BatchAs16)a1;
        return -1;
    }
    r = func_L09_0033E690(a0 + 0x10, a1, 0.0f);
    if (func_L09_00329178(a0, a1, a2, r) != 0) {
        return *(short *)(p + 0x1C8);
    }
    *(unsigned short *)(p + 0x1C8) = 0;
    return -1;
}

extern BatchA_A18_s32 func_L09_0033E658(void *a0);

BatchA_A18_s32 func_L18_004271C0(BatchA_A18_u8 *a0) {
    BatchA_A18_s32 p = *(BatchA_A18_s32 *)(a0 + 0x68);
    if (*(short *)((char *)p + 0x1CA) != 0) {
        *(BatchA_A18_BatchAs16 *)((char *)p + 0x1CA) = 0;
        return 0;
    }
    *(BatchA_A18_BatchAs16 *)((char *)p + 0x1CA) =
        (BatchA_A18_BatchAs16)func_L09_0033E658(*(void **)((char *)p + 0x38C));
    return 1;
}

INCLUDE_ASM("asm/levels", func_L18_00427208);

INCLUDE_ASM("asm/levels", func_L18_004273F0);

INCLUDE_ASM("asm/levels", func_L18_00427688);

/* func_L18_00427B48: 144 bytes against retail's 148; the chain pointer and the
   stored 1 take each other's register. Deposited in nonmatching/levels/. */
INCLUDE_ASM("asm/levels", func_L18_00427B48);
