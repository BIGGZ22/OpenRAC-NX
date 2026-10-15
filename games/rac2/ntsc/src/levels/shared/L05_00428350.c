#include "include_asm.h"

extern int D_001A7350 __attribute__((sda));

extern int D_001A7354 __attribute__((sda));

extern void func_2FC560(char *out, char *src);

extern int func_307778(float value);

extern void func_00282CF0_9(char *local, char *first, char *second);

extern void func_00282CC0_14(char *target, char *first, char *local);

extern char *D_L05_001B9710;

extern int func_00282978(void *);

extern void func_0029F338(void *);

/* The call target and the callback it registers are not the same function in
   every level this unit is placed in, so neither name may state an address. */
extern void ref_L05_0042E4E8_00(void);

extern int ref_L05_0042E4E8_01(void *, void *);

#ifndef RAC2_T_VEC16_FC936841D
#define RAC2_T_VEC16_FC936841D
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;
#endif

INCLUDE_ASM("asm/levels", func_L05_00428350);

INCLUDE_ASM("asm/levels", func_L05_00428518);

INCLUDE_ASM("asm/levels", func_L05_00428AC8);

INCLUDE_ASM("asm/levels", func_L05_00429530);

INCLUDE_ASM("asm/levels", func_L05_0042AC30);

INCLUDE_ASM("asm/levels", func_L05_0042AFE0);

INCLUDE_ASM("asm/levels", func_L05_0042B2C0);

INCLUDE_ASM("asm/levels", func_L05_0042BB10);

extern char *func_328508(char *);

extern void func_32AF08(char *);

extern void func_3073E0(char *, char *);

extern void func_3065F8(char *);

extern void func_3066A0(char *, char *, float);

extern void func_306630(char *, char *, char *);

extern int func_2F9888(char *, int, char *, int, float);

extern void func_2DCB60(int, int);

extern void func_2EB6E8(void);

extern int D_00221D94;

extern int D_001A79F0;

extern char *D_001B1EC4;

extern char *D_001B251C;

extern char D_00189E20[];

extern char D_0019B2E8[];

extern char D_001B3620[];

extern char D_001BFB30[];

#ifndef RAC2_T_TI128_15C4FA
#define RAC2_T_TI128_15C4FA
typedef int TI128 __attribute__((mode(TI)));
#endif

/* Creates the 0x1226 object, seeds its flags from the caller's two floats and copies the
   caller's quadword into the object's transform slot. */
INCLUDE_ASM("asm/levels", func_L05_0042BCE0);

INCLUDE_ASM("asm/levels", func_L05_0042BDB0);

INCLUDE_ASM("asm/levels", func_L05_0042C928);

/* Two bound helpers at the node's +0x68; the second call registers a callback
   and the context, the other branch calls a one-argument helper. */
void func_L05_0042E4E8(char *a0)
{
    char *context = a0;
    char *node = *(char **)(context + 0x68);

    if (func_00282978(node) == 0) {
        func_00282978(node + 4);
        ref_L05_0042E4E8_01(&ref_L05_0042E4E8_00, context);
    } else {
        func_0029F338(context);
    }
}

INCLUDE_ASM("asm/levels", func_L05_0042F4E8);

INCLUDE_ASM("asm/levels", func_L05_00430318);

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L05_00430BF0(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_14((char *)&object[index - 1], (char *)&object[index - 2], local);
}

INCLUDE_ASM("asm/levels", func_L05_00432078);

/* Creates the 0x10E3 object and copies the caller's transforms into it. */
INCLUDE_ASM("asm/levels", func_L05_00432080);

INCLUDE_ASM("asm/levels", func_L05_00432158);

INCLUDE_ASM("asm/levels", func_L05_004333F8);

INCLUDE_ASM("asm/levels", func_L05_00441D40);

INCLUDE_ASM("asm/levels", func_L05_00441EA0);

INCLUDE_ASM("asm/levels", func_L05_00441F30);

INCLUDE_ASM("asm/levels", func_L05_00442398);

/* Blends the negated angle into the object's transform through the level's matrix table. */
INCLUDE_ASM("asm/levels", func_L05_004424B8);

INCLUDE_ASM("asm/levels", func_L05_00442560);

INCLUDE_ASM("asm/levels", func_L05_00442720);

INCLUDE_ASM("asm/levels", func_L05_004427A8);

INCLUDE_ASM("asm/levels", func_L05_00442828);

INCLUDE_ASM("asm/levels", func_L05_00442880);

INCLUDE_ASM("asm/levels", func_L05_00442A58);

INCLUDE_ASM("asm/levels", func_L05_00442F90);

INCLUDE_ASM("asm/levels", func_L05_004431A0);

INCLUDE_ASM("asm/levels", func_L05_004432E0);

INCLUDE_ASM("asm/levels", func_L05_00443740);

/* Folds the object's two turn rates out of its parent's rotation into the moby, then
   rebuilds the three orientation blocks the driver reads. */
INCLUDE_ASM("asm/levels", func_L05_00443C18);

INCLUDE_ASM("asm/levels", func_L05_00443D08);

INCLUDE_ASM("asm/levels", func_L05_004445E8);

INCLUDE_ASM("asm/levels", func_L05_00444840);

INCLUDE_ASM("asm/levels", func_L05_00444B20);

/* Converts a two-float record to the coordinate pair the level's bias words expect. */
void func_L05_00444D60(char *a0, int *a1, int *a2, int a3)
{
    float local[2];
    float bias;

    func_2FC560((char *)local, a0);
    bias = (float)D_001A7350;
    *a1 = func_307778((local[0] - bias) * 0.0625f);
    bias = (float)D_001A7354;
    *a2 = func_307778((local[1] - bias) * 0.0625f) + a3;
}


INCLUDE_ASM("asm/levels", func_L05_00444DF8);

INCLUDE_ASM("asm/levels", func_L05_00444FD0);

INCLUDE_ASM("asm/levels", func_L05_00445338);

/* Reads a state word from a fixed global; unless its +0x86 halfword is 0x19 it
   returns 0, otherwise it reports whether the signed halfword reached through
   the pointer at +0x70, offset +0xD8, is non-zero. */
int func_L05_00445980(void)
{
    char *base = D_L05_001B9710;

    if (*(short *)(base + 0x86) != 0x19)
        return 0;
    return *(short *)(*(char **)(base + 0x70) + 0xD8) != 0;
}

INCLUDE_ASM("asm/levels", func_L05_004459B0);

INCLUDE_ASM("asm/levels", func_L05_00445A98);

INCLUDE_ASM("asm/levels", func_L05_00445B58);

INCLUDE_ASM("asm/levels", func_L05_00445C38);

INCLUDE_ASM("asm/levels", func_L05_00445D60);

/* While the level is idle, checks the object's table bit before starting the encounter. */
INCLUDE_ASM("asm/levels", func_L05_00445F90);
