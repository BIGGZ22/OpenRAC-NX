#include "include_asm.h"

/* Frozen Batch A A04-table-select-76; acknowledged claim 8c4d82cfbf43e138ed35e757fdaeee3e7c4651fa684549ba9d5e8ac22c56d111. */
typedef unsigned char BatchA_A04_u8;

typedef unsigned int BatchA_A04_u32;

extern BatchA_A04_u32 D_001A79F0[] __attribute__((sda));

extern BatchA_A04_u8 D_001395B8[];

INCLUDE_ASM("asm/levels", func_L04_00402D90);

INCLUDE_ASM("asm/levels", func_L04_00403530);

INCLUDE_ASM("asm/levels", func_L04_00404068);

INCLUDE_ASM("asm/levels", func_L04_00404100);

INCLUDE_ASM("asm/levels", func_L04_004045E0);

INCLUDE_ASM("asm/levels", func_L04_00407078);

INCLUDE_ASM("asm/levels", func_L04_004073E8);

INCLUDE_ASM("asm/levels", func_L04_004077A0);

INCLUDE_ASM("asm/levels", func_L04_0040A090);

/* A04-table-select-76 freshly exact-verified under family reservation */
BatchA_A04_u32 func_L04_0040A1D0(BatchA_A04_u8 *a0) {
    BatchA_A04_u32 v = D_001A79F0[0];
    BatchA_A04_u32 r = 0;
    if (v == 4) r = D_001395B8[(BatchA_A04_u32)a0 + 0x4D];
    else if (v == 0xB) r = D_001395B8[(BatchA_A04_u32)a0 + 0x57];
    return r;
}

/* A04-table-select family, incrementing form: bumps the state-selected byte */
void func_L04_0040A220(BatchA_A04_u8 *a0)
{
    BatchA_A04_u32 v = D_001A79F0[0];

    if (v == 4) D_001395B8[(BatchA_A04_u32)a0 + 0x4D]++;
    else if (v == 0xB) D_001395B8[(BatchA_A04_u32)a0 + 0x57]++;
}

/* shared native body of family A04-table-select: reads the state-selected byte */
unsigned int func_L04_0040A268(unsigned char *a0)
{
    unsigned int v = D_001A79F0[0];
    unsigned int r = 0;

    if (v == 4) r = D_001395B8[(unsigned int)a0 + 0x52];
    else if (v == 0xB) r = D_001395B8[(unsigned int)a0 + 0x5C];
    return r;
}

/* A04-table-select family, incrementing form (second slot pair) */
void func_L04_0040A2B8(BatchA_A04_u8 *a0)
{
    BatchA_A04_u32 v = D_001A79F0[0];

    if (v == 4) D_001395B8[(BatchA_A04_u32)a0 + 0x52]++;
    else if (v == 0xB) D_001395B8[(BatchA_A04_u32)a0 + 0x5C]++;
}

extern BatchA_A04_u8 *D_001B26F0 __attribute__((sda));

extern void func_002B9180(void *);

/* Sweeps the 0x800 packed 64-byte records, redrawing the four live kinds */
void func_L04_0040A310(void)
{
    int i;

    for (i = 0; i < 0x800; i++) {
        BatchA_A04_u8 b = *(BatchA_A04_u8 *)((i << 6) + (BatchA_A04_u32)D_001B26F0);

        if (b == 0x4A || b == 8 || b == 0xB || b == 0xF) {
            func_002B9180(D_001B26F0 + (i << 6));
        }
    }
}

INCLUDE_ASM("asm/levels", func_L04_0040A3B8);

INCLUDE_ASM("asm/levels", func_L04_0040A5C0);

INCLUDE_ASM("asm/levels", func_L04_0040A628);

INCLUDE_ASM("asm/levels", func_L04_0040A958);

extern signed char *D_L04_001B24DC __attribute__((sda));

extern void func_40E248(char *a0, char *a1);

/* Retail reaches its record base off $gp (a small-data symbol), and the three
   state words at 0x48 of the record and of the indexed record. */
void func_L04_0040AC30(char *a0, char *a1)
{
    char *s0 = a0;
    char *s1 = a1;

    func_40E248(s0, s1);
    *(int *)(s0 + 0x48) = 0;
    if (*(int *)(s1 + 0x1DC) != -1) {
        *(int *)((*(int *)(s1 + 0x1DC) << 8) + (int)D_L04_001B24DC + 0x48) = 0;
    }
}

/* func_L04_0040AC98 reaches its global off $gp, so its body lives in
   L04_00402D90-g8.c. */

/* func_L04_0040ACA8 reaches one of its globals absolutely and another off $gp,
   so its body lives in L04_00402D90-g8.c. */

INCLUDE_ASM("asm/levels", func_L04_0040ACC8);

INCLUDE_ASM("asm/levels", func_L04_0040AD28);

INCLUDE_ASM("asm/levels", func_L04_0040BA68);

INCLUDE_ASM("asm/levels", func_L04_0040BD50);

INCLUDE_ASM("asm/levels", func_L04_0040BED0);

INCLUDE_ASM("asm/levels", func_L04_0040C0C0);

INCLUDE_ASM("asm/levels", func_L04_0040C368);

INCLUDE_ASM("asm/levels", func_L04_0040C880);

extern void func_L04_0040D148();

extern void func_L04_0040CE30();

void func_L04_0040CDD0(unsigned char *a0)
{
    unsigned char *s = *(unsigned char **)(a0 + 0x68);

    if (*(unsigned int *)(s + 0x4D8) != 0) {
        func_L04_0040D148(a0, -1);
    } else if (*(unsigned int *)(s + 0x768) != 0) {
        func_L04_0040D148(a0, *(unsigned int *)(s + 0x764));
    } else if (*(unsigned char *)(a0 + 0xBC) != 6) {
        func_L04_0040CE30(a0);
    }
}

extern void func_0029FCC0(void *, int, void *);

extern void func_00282D30(void *, void *, float);

extern void func_00282CC0(void *, void *, void *);

extern float func_00283CE0(int);

/* The five floats come first and the two ints last: the floats then go to
   f12-f16 and the ints to a1 and a2, and the int argument is set up after
   the float ones, as the retail code has it. */
extern void func_L00_003F8280(void *, float, float, float, float, float, int, int);

/* The emitter table the object points at: the slot id at 0x644, then the
   four rows of four heights from 0x660 on. */
typedef struct {
    char pad_000[0x644];
    int slot;
    char pad_648[0x18];
    float row660[4];
    float row670[4];
    float row680[4];
    float row690[4];
} L04_0040CE30_OBJ;

/* Walks the four rows of the emitter table: every row's height is lowered by
   the shared floor, the three threshold vectors are nudged by the row's share
   and handed to the particle scheduler, and a row that sinks too far is lifted
   back to the floor. */
void func_L04_0040CE30(char *a0)
{
    L04_0040CE30_OBJ *s2 = *(L04_0040CE30_OBJ **)(a0 + 0x68);
    float vec3[4];
    float vec4[4];
    float vec2[4];
    float tmp[4];
    char *dir = a0 + 0xC0;
    int i;

    for (i = 0; i < 4; i++) {
        float f22;
        float f21;
        float f20;

        func_0029FCC0(a0, 2, vec2);
        func_0029FCC0(a0, 3, vec3);
        func_0029FCC0(a0, 4, vec4);

        s2->row670[i] = s2->row670[i] - s2->row660[i];
        func_00282D30(tmp, dir, s2->row670[i]);
        func_00282CC0(vec3, vec3, tmp);

        f22 = func_00283CE0(i) * 0.25f;
        f21 = f22 * 0.1f;
        vec3[2] = vec3[2] + f21;
        f20 = f22 * -0.025f + 0.05f;
        func_L00_003F8280(vec3, f20, 0.0f, 0.0f, 0.0f, 0.0025f, s2->slot, 1);
        if (s2->row670[i] < -0.2f) {
            s2->row670[i] = -0.05f;
        }

        s2->row680[i] = s2->row680[i] - s2->row660[i];
        func_00282D30(tmp, dir, s2->row680[i]);
        func_00282CC0(vec2, vec2, tmp);
        vec2[2] = vec2[2] + f21;
        func_L00_003F8280(vec2, f20, 0.0f, 0.0f, 0.0f, 0.0025f, s2->slot, 1);
        if (s2->row680[i] < -0.2f) {
            s2->row680[i] = -0.05f;
        }

        s2->row690[i] = s2->row690[i] - s2->row660[i];
        func_00282D30(tmp, dir, s2->row690[i]);
        func_00282CC0(vec4, vec4, tmp);
        vec4[2] = vec4[2] + f21;
        func_L00_003F8280(vec4, f22 * -0.050000004f + 0.075f, 0.0f, 0.0f, 0.0f, 0.0025f, s2->slot, 1);
        if (s2->row690[i] < -0.19f) {
            s2->row690[i] = -0.05f;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L04_0040D148);

INCLUDE_ASM("asm/levels", func_L04_0040DCA8);

INCLUDE_ASM("asm/levels", func_L04_0040DF90);

extern BatchA_A04_u8 D_00188660[];

extern void func_002E59F8(int handle);

/* Releases the callback records this moby still holds: every handle slot that is set
   names a record of the callback table, and the one that belongs to this moby is
   handed to the release helper. Each slot is cleared afterwards. */
void func_L04_0040E248(int a0, BatchA_A04_u8 *a1)
{
    int id = a0;
    BatchA_A04_u8 *p = a1;
    int v;
    BatchA_A04_u8 *rec;

    v = *(int *)(p + 0x748);
    if (v != -1) {
        rec = D_00188660 + v * 0x70;
        if (*(int *)(rec + 0x88) == id && rec[0x74] != 0) {
            func_002E59F8(v);
        }
    }
    *(int *)(p + 0x748) = -1;

    v = *(int *)(p + 0x74C);
    if (v != -1) {
        rec = D_00188660 + v * 0x70;
        if (*(int *)(rec + 0x88) == id && rec[0x74] != 0) {
            func_002E59F8(v);
        }
    }
    *(int *)(p + 0x74C) = -1;

    v = *(int *)(p + 0x714);
    if (v != -1) {
        rec = D_00188660 + v * 0x70;
        if (*(int *)(rec + 0x88) == id && rec[0x74] != 0) {
            func_002E59F8(v);
        }
    }
    *(int *)(p + 0x714) = -1;

    v = *(int *)(p + 0x3BC);
    if (v != -1) {
        rec = D_00188660 + v * 0x70;
        if (*(int *)(rec + 0x88) == id && rec[0x74] != 0) {
            func_002E59F8(v);
        }
    }
    *(int *)(p + 0x3BC) = -1;

    v = *(int *)(p + 0x328);
    if (v != -1) {
        rec = D_00188660 + v * 0x70;
        if (*(int *)(rec + 0x88) == id && rec[0x74] != 0) {
            func_002E59F8(v);
        }
    }
    *(int *)(p + 0x328) = -1;

    v = *(int *)(p + 0x330);
    if (v != -1) {
        rec = D_00188660 + v * 0x70;
        if (*(int *)(rec + 0x88) == id && rec[0x74] != 0) {
            func_002E59F8(v);
        }
    }
    *(int *)(p + 0x330) = -1;

    v = *(int *)(p + 0x334);
    if (v != -1) {
        rec = D_00188660 + v * 0x70;
        if (*(int *)(rec + 0x88) == id && rec[0x74] != 0) {
            func_002E59F8(v);
        }
    }
    *(int *)(p + 0x32C) = 0;
    *(int *)(p + 0x334) = -1;
    *(int *)(p + 0x324) = 0;
}

INCLUDE_ASM("asm/levels", func_L04_0040E460);

INCLUDE_ASM("asm/levels", func_L04_0040EC80);

INCLUDE_ASM("asm/levels", func_L04_0040EF50);

INCLUDE_ASM("asm/levels", func_L04_0040F5D0);
